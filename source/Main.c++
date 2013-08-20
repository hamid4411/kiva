/* Main.c++ is part of Kiva (Written by Neal Kruis)
 * Copyright (C) 2012-2013 Big Ladder Software <info@bigladdersoftware.com>
 * All rights reserved.
 *
 * Kiva is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kiva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kiva.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "InputParser.h"
#include "WeatherData.h"
#include "Input.h"
#include "Ground.h"
#include <cmath>
#include <boost/program_options.hpp>


using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;
namespace po = boost::program_options;

int main(int argc, char *argv[])
{

	string versionInfo = "kiva 0.0.1";
	string copyrightInfo = "Copyright (C) 2012-2013 Big Ladder Software\n"
			               "Web: www.bigladdersoftware.com";

	try {

		po::options_description generic("Options");
		generic.add_options()
				("help,h", "Produce this message")
				("version,v", "Display version information");

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("input-file", po::value<string>(), "input file");

        po::options_description cmdLine;
        cmdLine.add(generic).add(hidden);

		po::positional_options_description p;
		p.add("input-file", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
				options(cmdLine).positional(p).run(), vm);
		po::notify(vm);

		if (vm.count("help"))
		{
			cout << versionInfo << "\n";
			cout << copyrightInfo << "\n";
			cout << "Usage: kiva [Input File]\n"
					"   Input format: yaml\n";
			cout << generic;
			return 0;
		}
		if (vm.count("version"))
		{
			cout << versionInfo << "\n";
			cout << copyrightInfo << "\n";
			return 0;
		}
		if (vm.count("input-file"))
		{
			ptime beginCalc = second_clock::local_time();
			cout << "Starting Program: " << beginCalc << endl;

			// parse input
			Input input = inputParser(vm["input-file"].as<string>());

			// parse weather
			WeatherData weather(input.simulationControl.weatherFile);

			// simulation
			input.simulationControl.setStartTime();
			ptime simStart = input.simulationControl.startTime;
			ptime simEnd(input.simulationControl.endDate + days(1));
			time_duration simDuration =  simEnd - simStart;


			double tstart = 0.0; // [s] Simulation start time
			double tend = simDuration.total_seconds(); // [s] Simulation end time
			double timestep = input.simulationControl.timestep.total_seconds();

			// set up output file
			ofstream output;
			output.open("Output.csv");
			output << "Time Stamp, Heat Flux [W/m2]" << endl;

			// initialize
			Ground ground(weather,input.foundations[0],input.simulationControl);

			// loop

			ptime prevTime = second_clock::local_time();

			for (double t = tstart; t < tend; t = t + timestep)
			{
				ptime currentTime = second_clock::local_time();
				ptime simTime(input.simulationControl.startDate,seconds(t));
				date today = simTime.date();

				ground.calculate(t);

				if (currentTime - prevTime > milliseconds(500))
				{

					double percentComplete = round(t/tend*1000)/10.0;

					cout << percentComplete << "% (" << today << ")\n";

					prevTime = currentTime;
				}

				output << to_simple_string(simTime) << ", " <<
						  ground.QSlabTotal << endl;

			}

			output.close();

			ptime simTime(input.simulationControl.startDate,seconds(tend));
			date today = simTime.date();
			cout << "100% (" << today << ")\n";

			// process output
			//cout << "Total Heat Flux: "<< ground.QSlabTotal << endl;


			ptime finishCalc = second_clock::local_time();
			cout << "Finished Program: " << finishCalc << endl;

			time_duration totalCalc = finishCalc - beginCalc;
			cout << "Elapsed Time: " << totalCalc << endl;

			return 0;

		}

	}
    catch(exception& e)
    {
        cout << e.what() << endl;
        return 1;
    }

}


