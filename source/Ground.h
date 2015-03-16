/* Ground.h is part of Kiva (Written by Neal Kruis)
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

#ifndef GROUND_HPP_
#define GROUND_HPP_

//#define USE_LIS_SOLVER

#include "Mesher.h"
#include "Domain.h"
#include "WeatherData.h"
#include "Input.h"
#include "Algorithms.h"
#include "GroundPlot.h"

#if defined(ENABLE_OPENGL)

#include "PixelCounter.h"

#endif

#include <cmath>
#include <vector>
#include <iostream>
#include <string>

#include <mgl2/mgl.h>

#include <boost/lexical_cast.hpp>
#include <boost/multi_array.hpp>

#if defined(USE_LIS_SOLVER)

#include "lis.h"

#else

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>

#include <boost/numeric/bindings/traits/ublas_vector.hpp>
#include <boost/numeric/bindings/traits/ublas_sparse.hpp>
#include <boost/numeric/bindings/umfpack/umfpack.hpp>

namespace umf = boost::numeric::bindings::umfpack;

#endif

class Ground
{
private:

  double timestep;
  SimulationControl &simulationControl;
  WeatherData &weatherData;

  bool preprocess;

  double tNow;
  double annualAverageDryBulbTemperature;

public:
  Domain domain;
  Foundation &foundation;

private:

  // Data structures

  // ADE
  boost::multi_array<double, 3> U; // ADE upper sweep, n+1
  boost::multi_array<double, 3> UOld; // ADE upper sweep, n
  boost::multi_array<double, 3> V; // ADE lower sweep, n+1
  boost::multi_array<double, 3> VOld; // ADE lower sweep, n

  // ADI
  std::vector<double> a1; // lower diagonal
  std::vector<double> a2; // main diagonal
  std::vector<double> a3; // upper diagonal
  std::vector<double> b_; // right-hand side
  std::vector<double> x_; // solution

  // Implicit
#if defined(USE_LIS_SOLVER)
  LIS_MATRIX Amat;
  LIS_VECTOR b, x;

  LIS_SOLVER solver;

  std::vector<char> solverOptions;
#else
    boost::numeric::ublas::compressed_matrix<double,
                    boost::numeric::ublas::column_major, 0,
                    boost::numeric::ublas::unbounded_array<int>,
                    boost::numeric::ublas::unbounded_array<double> > Amat;

    boost::numeric::ublas::vector<double> b, x;
#endif


  boost::multi_array<double, 3> TOld; // solution, n

  std::vector<std::pair<double,double>> boundaryLayer;

  std::vector<GroundPlot> plots;
  std::ofstream outputFile;


  boost::posix_time::ptime prevStatusUpdate;
  double prevOutputTime;
  bool initPeriod;


public:
  size_t nX, nY, nZ;

  boost::multi_array<double, 3> TNew; // solution, n+1

  double percentComplete;

  // Constructor
  Ground(WeatherData &weatherData,
      Foundation &foundation,
      SimulationControl &simulationControl,
      std::string outputFileName,
      bool preprocess=false);

  // Destructor
  virtual ~Ground();

  // Calculator
  void simulate();


private:

  // Initializers (Called from constructor)
  void buildDomain();

  void initializeConditions();

  void initializePlots();

  void calculate(double t);

  void printStatus(double t);

  std::string printOutputHeaders();
  std::string printOutputLine();

  // Calculators (Called from main calculator)
  void calculateADE();

  void calculateADEUpwardSweep();

  void calculateADEDownwardSweep();

  void calculateExplicit();

  void calculateMatrix(Foundation::NumericalScheme scheme);

  void calculateADI(int dim);

  void plot();

  // Misc. Functions
  void setAmatValue(const int i, const int j, const double val);
  void setbValue(const int i, const double val);
  void solveLinearSystem();
  void clearAmat();
  double getxValue(const int i);

  boost::posix_time::ptime getSimTime(double t);

  double getInitialTemperature(double t, double z);

  double getDeepGroundTemperature();

  double getConvectionCoeff(double Tsurf,
                double Tamb,
                double Vair,
                double roughness,
                bool isExterior,
                double tilt);

  double getOutdoorTemperature();

  double getIndoorTemperature();

  double getLocalWindSpeed();

  void setSolarBoundaryConditions();

  double getSurfaceAverageHeatFlux(std::string surfaceName);
  double getSurfaceAverageTemperature(std::string surfaceName);
  double getSurfaceArea(std::string surfaceName);

  double getSurfaceEffectiveTemperature(std::string surfaceName, double constructionRValue);

  std::vector<double> calculateHeatFlux(const size_t &i, const size_t &j, const size_t &k);
  void calculateBoundaryLayer();
  double getBoundaryValue(double dist);

  double getBoundaryDistance(double val);

  void setNewBoundaryGeometry();
};



#endif /* GROUND_HPP_ */
