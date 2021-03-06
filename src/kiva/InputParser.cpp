/* Copyright (c) 2012-2016 Big Ladder Software. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#include "InputParser.hpp"

Input inputParser(std::string inputFile)
{

  Input input;
  SimulationControl simulationControl;
  Foundation foundation;
  Boundaries boundaries;
  Initialization initialization;
  Output output;

  boost::filesystem::path inputPath(inputFile);

  YAML::Node yamlInput = YAML::LoadFile(inputPath.string());

  // SIMULATION CONTROL
  simulationControl.startDate =
      boost::gregorian::from_string(yamlInput["Simulation Control"]["Start Date"].as<std::string>());
  simulationControl.endDate =
      boost::gregorian::from_string(yamlInput["Simulation Control"]["End Date"].as<std::string>());
  simulationControl.timestep =
      boost::posix_time::minutes(yamlInput["Simulation Control"]["Timestep"].as<long>());

  // MATERIALS
  std::map<std::string, Material> materials;

  for(YAML::const_iterator it=yamlInput["Materials"].begin();it!=yamlInput["Materials"].end();++it)
  {
    Material tempMaterial;
    tempMaterial.conductivity = it->second["Conductivity"].as<double>();
    tempMaterial.density = it->second["Density"].as<double>();
    tempMaterial.specificHeat = it->second["Specific Heat"].as<double>();

    materials.insert(std::pair<std::string,Material>(it->first.as<std::string>(),tempMaterial));
  }

  // FOUNDATION

  // Soil
  foundation.soil = materials[yamlInput["Foundation"]["Soil"].as<std::string>()];

  // Grade
  if (yamlInput["Foundation"]["Soil Absorptivity"].IsDefined()) {
    foundation.soilAbsorptivity = yamlInput["Foundation"]["Soil Absorptivity"].as<double>();
  }
  else {
    foundation.soilAbsorptivity = 0.8;
  }

  if (yamlInput["Foundation"]["Soil Emissivity"].IsDefined()) {
    foundation.soilEmissivity = yamlInput["Foundation"]["Soil Emissivity"].as<double>();
  }
  else {
    foundation.soilEmissivity = 0.8;
  }

  if (yamlInput["Foundation"]["Surface Roughness"].IsDefined()) {
    foundation.surfaceRoughness = yamlInput["Foundation"]["Surface Roughness"].as<double>();
  }
  else {
    foundation.surfaceRoughness = 0.03;
  }

  // Slab
  if  (yamlInput["Foundation"]["Slab"].IsDefined())
  {
    foundation.hasSlab = true;

    for (size_t i=0;i<yamlInput["Foundation"]["Slab"]["Layers"].size();i++)
    {

      Layer tempLayer;
      tempLayer.thickness = yamlInput["Foundation"]["Slab"]["Layers"][i]["Thickness"].as<double>();
      tempLayer.material = materials[yamlInput["Foundation"]["Slab"]["Layers"][i]["Material"].as<std::string>()];

      foundation.slab.layers.push_back(tempLayer);

    }

    if (yamlInput["Foundation"]["Slab"]["Emissivity"].IsDefined()) {
      foundation.slab.emissivity = yamlInput["Foundation"]["Slab"]["Emissivity"].as<double>();
    }
    else {
      foundation.slab.emissivity = 0.8;
    }
  }
  else
  {
    foundation.hasSlab = false;
  }

  // Wall
  if  (yamlInput["Foundation"]["Wall"].IsDefined())
  {
    foundation.hasWall = true;

    for (size_t i=0;i<yamlInput["Foundation"]["Wall"]["Layers"].size();i++)
    {

      Layer tempLayer;
      tempLayer.thickness = yamlInput["Foundation"]["Wall"]["Layers"][i]["Thickness"].as<double>();
      tempLayer.material = materials[yamlInput["Foundation"]["Wall"]["Layers"][i]["Material"].as<std::string>()];

      foundation.wall.layers.push_back(tempLayer);

    }

    foundation.wall.heightAboveGrade = yamlInput["Foundation"]["Wall"]["Height Above Grade"].as<double>();
    foundation.wall.height = yamlInput["Foundation"]["Wall"]["Height"].as<double>();

    if (yamlInput["Foundation"]["Wall"]["Interior Emissivity"].IsDefined()) {
      foundation.wall.interiorEmissivity = yamlInput["Foundation"]["Wall"]["Interior Emissivity"].as<double>();
    }
    else {
      foundation.wall.interiorEmissivity = 0.8;
    }

    if (yamlInput["Foundation"]["Wall"]["Exterior Emissivity"].IsDefined()) {
      foundation.wall.exteriorEmissivity = yamlInput["Foundation"]["Wall"]["Exterior Emissivity"].as<double>();
    }
    else {
      foundation.wall.exteriorEmissivity = 0.8;
    }

    if (yamlInput["Foundation"]["Wall"]["Exterior Absorptivity"].IsDefined()) {
      foundation.wall.exteriorAbsorptivity = yamlInput["Foundation"]["Wall"]["Exterior Absorptivity"].as<double>();
    }
    else {
      foundation.wall.exteriorAbsorptivity = 0.8;
    }

  }
  else
  {
    foundation.hasWall = false;
  }

  // Interior Horizontal Insulation
  if  (yamlInput["Foundation"]["Interior Horizontal Insulation"].IsDefined())
  {
    foundation.hasInteriorHorizontalInsulation = true;

    foundation.interiorHorizontalInsulation.layer.thickness = yamlInput["Foundation"]["Interior Horizontal Insulation"]["Thickness"].as<double>();
    foundation.interiorHorizontalInsulation.layer.material = materials[yamlInput["Foundation"]["Interior Horizontal Insulation"]["Material"].as<std::string>()];
    foundation.interiorHorizontalInsulation.depth = yamlInput["Foundation"]["Interior Horizontal Insulation"]["Depth"].as<double>();
    foundation.interiorHorizontalInsulation.width = yamlInput["Foundation"]["Interior Horizontal Insulation"]["Width"].as<double>();

  }
  else
  {
    foundation.hasInteriorHorizontalInsulation = false;
  }

  // Interior Vertical Insulation
  if  (yamlInput["Foundation"]["Interior Vertical Insulation"].IsDefined())
  {
    foundation.hasInteriorVerticalInsulation = true;

    foundation.interiorVerticalInsulation.layer.thickness = yamlInput["Foundation"]["Interior Vertical Insulation"]["Thickness"].as<double>();
    foundation.interiorVerticalInsulation.layer.material = materials[yamlInput["Foundation"]["Interior Vertical Insulation"]["Material"].as<std::string>()];
    foundation.interiorVerticalInsulation.depth = yamlInput["Foundation"]["Interior Vertical Insulation"]["Depth"].as<double>();

  }
  else
  {
    foundation.hasInteriorVerticalInsulation = false;
  }

  // Exterior Horizontal Insulation
  if  (yamlInput["Foundation"]["Exterior Horizontal Insulation"].IsDefined())
  {
    foundation.hasExteriorHorizontalInsulation = true;

    foundation.exteriorHorizontalInsulation.layer.thickness = yamlInput["Foundation"]["Exterior Horizontal Insulation"]["Thickness"].as<double>();
    foundation.exteriorHorizontalInsulation.layer.material = materials[yamlInput["Foundation"]["Exterior Horizontal Insulation"]["Material"].as<std::string>()];
    foundation.exteriorHorizontalInsulation.depth = yamlInput["Foundation"]["Exterior Horizontal Insulation"]["Depth"].as<double>();
    foundation.exteriorHorizontalInsulation.width = yamlInput["Foundation"]["Exterior Horizontal Insulation"]["Width"].as<double>();

  }
  else
  {
    foundation.hasExteriorHorizontalInsulation = false;
  }

  // Exterior Vertical Insulation
  if  (yamlInput["Foundation"]["Exterior Vertical Insulation"].IsDefined())
  {
    foundation.hasExteriorVerticalInsulation = true;

    foundation.exteriorVerticalInsulation.layer.thickness = yamlInput["Foundation"]["Exterior Vertical Insulation"]["Thickness"].as<double>();
    foundation.exteriorVerticalInsulation.layer.material = materials[yamlInput["Foundation"]["Exterior Vertical Insulation"]["Material"].as<std::string>()];
    foundation.exteriorVerticalInsulation.depth = yamlInput["Foundation"]["Exterior Vertical Insulation"]["Depth"].as<double>();

  }
  else
  {
    foundation.hasExteriorVerticalInsulation = false;
  }


  // Site

  foundation.foundationDepth = yamlInput["Foundation"]["Foundation Depth"].as<double>();

  if  (yamlInput["Foundation"]["Orientation"].IsDefined()) {
    foundation.orientation = yamlInput["Foundation"]["Orientation"].as<double>();
  }
  else {
    foundation.orientation = 0.0;
  }

  // Geometry
  if (yamlInput["Foundation"]["Coordinate System"].IsDefined()) {
    if (yamlInput["Foundation"]["Coordinate System"].as<std::string>() == "CARTESIAN")
      foundation.coordinateSystem = Foundation::CS_CARTESIAN;
    else if (yamlInput["Foundation"]["Coordinate System"].as<std::string>() == "CYLINDRICAL")
      foundation.coordinateSystem = Foundation::CS_CYLINDRICAL;
  }
  else {
    foundation.coordinateSystem = Foundation::CS_CARTESIAN;
  }

  if (yamlInput["Foundation"]["Two-Dimensional Approximation"].IsDefined()) {
    if (yamlInput["Foundation"]["Two-Dimensional Approximation"].as<std::string>() == "AP")
      foundation.reductionStrategy = Foundation::RS_AP;
    else if (yamlInput["Foundation"]["Two-Dimensional Approximation"].as<std::string>() == "RR")
      foundation.reductionStrategy = Foundation::RS_RR;
    else if (yamlInput["Foundation"]["Two-Dimensional Approximation"].as<std::string>() == "BOUNDARY")
      foundation.reductionStrategy = Foundation::RS_BOUNDARY;
    else if (yamlInput["Foundation"]["Two-Dimensional Approximation"].as<std::string>() == "CUSTOM")
    {
      foundation.reductionStrategy = Foundation::RS_CUSTOM;
      if (yamlInput["Foundation"]["Length 1"].IsDefined())
      {
        foundation.twoParameters = true;
        foundation.reductionLength1 = yamlInput["Foundation"]["Length 1"].as<double>();
      }
      else
      {
        foundation.twoParameters = false;
      }
      foundation.reductionLength2 = yamlInput["Foundation"]["Length 2"].as<double>();
    }
  }
  else {
    foundation.reductionStrategy = Foundation::RS_BOUNDARY;
  }

  if (yamlInput["Foundation"]["Number of Dimensions"].IsDefined())
    foundation.numberOfDimensions = yamlInput["Foundation"]["Number of Dimensions"].as<int>();
  else
    foundation.numberOfDimensions = 2;

  if  (yamlInput["Foundation"]["Use Symmetry"].IsDefined())
    foundation.useSymmetry = yamlInput["Foundation"]["Use Symmetry"].as<bool>();
  else
    foundation.useSymmetry = true;

  for (size_t i=0;i<yamlInput["Foundation"]["Polygon"].size();i++)
  {
    foundation.polygon.outer().push_back(Point(
        yamlInput["Foundation"]["Polygon"][i][0].as<double>(),
        yamlInput["Foundation"]["Polygon"][i][1].as<double>()));
  }

  if  (yamlInput["Foundation"]["Building Height"].IsDefined())
  {
    foundation.buildingHeight = yamlInput["Foundation"]["Building Height"].as<double>();
  }
  else
  {
    foundation.buildingHeight = 0.0;
  }

  if  (yamlInput["Foundation"]["Perimeter Surface Width"].IsDefined())
  {
    foundation.hasPerimeterSurface = true;
    foundation.perimeterSurfaceWidth = yamlInput["Foundation"]["Perimeter Surface Width"].as<double>();
  }
  else
  {
    foundation.hasPerimeterSurface = false;
    foundation.perimeterSurfaceWidth = 0.0;
  }

  // Meshing
  if  (yamlInput["Foundation"]["Mesh"].IsDefined())
  {

    if  (yamlInput["Foundation"]["Mesh"]["Minimum Cell Dimension"].IsDefined()) {
      foundation.mesh.minCellDim = yamlInput["Foundation"]["Mesh"]["Minimum Cell Dimension"].as<double>();
    }
    else {
      foundation.mesh.minCellDim = 0.02;
    }

    if  (yamlInput["Foundation"]["Mesh"]["Maximum Near-Field Growth Coefficient"].IsDefined()) {
      foundation.mesh.maxNearGrowthCoeff = yamlInput["Foundation"]["Mesh"]["Maximum Near-Field Growth Coefficient"].as<double>();
    }
    else {
      foundation.mesh.maxNearGrowthCoeff = 1.5;
    }

    if  (yamlInput["Foundation"]["Mesh"]["Maximum Deep-Field Growth Coefficient"].IsDefined()) {
      foundation.mesh.maxDepthGrowthCoeff = yamlInput["Foundation"]["Mesh"]["Maximum Deep-Field Growth Coefficient"].as<double>();
    }
    else {
      foundation.mesh.maxDepthGrowthCoeff = 1.5;
    }

    if  (yamlInput["Foundation"]["Mesh"]["Maximum Interior-Field Growth Coefficient"].IsDefined()) {
      foundation.mesh.maxInteriorGrowthCoeff = yamlInput["Foundation"]["Mesh"]["Maximum Interior-Field Growth Coefficient"].as<double>();
    }
    else {
      foundation.mesh.maxInteriorGrowthCoeff = 1.5;
    }

    if  (yamlInput["Foundation"]["Mesh"]["Maximum Far-Field Growth Coefficient"].IsDefined()) {
      foundation.mesh.maxExteriorGrowthCoeff = yamlInput["Foundation"]["Mesh"]["Maximum Far-Field Growth Coefficient"].as<double>();
    }
    else {
      foundation.mesh.maxExteriorGrowthCoeff = 1.5;
    }
  }
  else
  {
    foundation.mesh.minCellDim = 0.05;
    foundation.mesh.maxNearGrowthCoeff = 1.5;
    foundation.mesh.maxDepthGrowthCoeff = 1.5;
    foundation.mesh.maxInteriorGrowthCoeff = 1.5;
    foundation.mesh.maxExteriorGrowthCoeff = 1.5;
  }

  // Simulation Control
  if  (yamlInput["Foundation"]["Numerical Scheme"].IsDefined())
  {
    if (yamlInput["Foundation"]["Numerical Scheme"].as<std::string>() == "ADE")
      foundation.numericalScheme = Foundation::NS_ADE;
    else if (yamlInput["Foundation"]["Numerical Scheme"].as<std::string>() == "EXPLICIT")
      foundation.numericalScheme = Foundation::NS_EXPLICIT;
    else if (yamlInput["Foundation"]["Numerical Scheme"].as<std::string>() == "ADI")
    {
      foundation.numericalScheme = Foundation::NS_ADI;
      if  (yamlInput["Foundation"]["f-ADI"].IsDefined())
        foundation.fADI = yamlInput["Foundation"]["f-ADI"].as<double>();
      else
        foundation.fADI = 0.00001;
    }
    else if (yamlInput["Foundation"]["Numerical Scheme"].as<std::string>() == "IMPLICIT")
      foundation.numericalScheme = Foundation::NS_IMPLICIT;
    else if (yamlInput["Foundation"]["Numerical Scheme"].as<std::string>() == "CRANK-NICOLSON")
      foundation.numericalScheme = Foundation::NS_CRANK_NICOLSON;
    else if (yamlInput["Foundation"]["Numerical Scheme"].as<std::string>() == "STEADY-STATE")
      foundation.numericalScheme = Foundation::NS_STEADY_STATE;
  }
  else
  {
    foundation.numericalScheme = Foundation::NS_ADI;
    foundation.fADI = 0.00001;
}

  if  (yamlInput["Foundation"]["Solver"].IsDefined())
  {
    foundation.solver = yamlInput["Foundation"]["Solver"].as<std::string>();
  }
  else
  {
    foundation.solver = "bicgstab";
  }

  if  (yamlInput["Foundation"]["Preconditioner"].IsDefined())
  {
    foundation.preconditioner = yamlInput["Foundation"]["Preconditioner"].as<std::string>();
  }
  else
  {
    foundation.preconditioner = "ilu";
  }

  if  (yamlInput["Foundation"]["Maximum Iterations"].IsDefined())
  {
    foundation.maxIterations = yamlInput["Foundation"]["Maximum Iterations"].as<int>();
  }
  else
  {
    foundation.maxIterations = 100000;
  }

  if  (yamlInput["Foundation"]["Tolerance"].IsDefined())
  {
    foundation.tolerance = yamlInput["Foundation"]["Tolerance"].as<double>();
  }
  else
  {
    foundation.tolerance = 1.0e-6;
  }

  // BOUNDARIES
  if  (yamlInput["Boundaries"]["Far-Field Width"].IsDefined()) {
    foundation.farFieldWidth = yamlInput["Boundaries"]["Far-Field Width"].as<double>();
  }
  else {
    foundation.farFieldWidth = 40;
  }

  if  (yamlInput["Boundaries"]["Deep-Ground Depth"].IsDefined()) {
    foundation.deepGroundDepth = yamlInput["Boundaries"]["Deep-Ground Depth"].as<double>();
  }
  else {
    foundation.deepGroundDepth = 40;
  }

  if (yamlInput["Boundaries"]["Deep-Ground Boundary Condition"].IsDefined()) {
    if (yamlInput["Boundaries"]["Deep-Ground Boundary Condition"].as<std::string>() == "AUTO")
    {
      foundation.deepGroundBoundary = Foundation::DGB_AUTO;
    }
    else if (yamlInput["Boundaries"]["Deep-Ground Boundary Condition"].as<std::string>() == "CONSTANT-TEMP")
    {
      foundation.deepGroundBoundary = Foundation::DGB_CONSTANT_TEMPERATURE;
      foundation.deepGroundTemperature = yamlInput["Boundaries"]["Deep-Ground Temperature"].as<double>();
    }
    else if (yamlInput["Boundaries"]["Deep-Ground Boundary Condition"].as<std::string>() == "ZERO-FLUX")
    {
      foundation.deepGroundBoundary = Foundation::DGB_ZERO_FLUX;
    }
  }
  else {
    foundation.deepGroundBoundary = Foundation::DGB_ZERO_FLUX;
  }

  if  (yamlInput["Boundaries"]["Indoor Air Temperature Method"].IsDefined())
  {
    if (yamlInput["Boundaries"]["Indoor Air Temperature Method"].as<std::string>() == "FILE")
    {
      boundaries.indoorTemperatureMethod = Boundaries::ITM_FILE;
      boundaries.indoorAirTemperatureFile.fileName = yamlInput["Boundaries"]["Indoor Air Temperature File"]["Name"].as<std::string>();
      boundaries.indoorAirTemperatureFile.firstIndex.first = yamlInput["Boundaries"]["Indoor Air Temperature File"]["Index"][0].as<int>();
      boundaries.indoorAirTemperatureFile.firstIndex.second = yamlInput["Boundaries"]["Indoor Air Temperature File"]["Index"][1].as<int>();
      boundaries.indoorAirTemperatureFile.searchDir = inputPath.parent_path();
      boundaries.indoorAirTemperatureFile.readData();
    }
    else if (yamlInput["Boundaries"]["Indoor Air Temperature Method"].as<std::string>() == "CONSTANT")
    {
      boundaries.indoorTemperatureMethod = Boundaries::ITM_CONSTANT_TEMPERATURE;
      boundaries.indoorAirTemperature = yamlInput["Boundaries"]["Indoor Air Temperature"].as<double>();
    }
  }
  else
  {
    boundaries.indoorTemperatureMethod = Boundaries::ITM_CONSTANT_TEMPERATURE;
    boundaries.indoorAirTemperature = yamlInput["Boundaries"]["Indoor Air Temperature"].as<double>();
  }

  if (yamlInput["Boundaries"]["Local Boundary Layer Thickness"].IsDefined()) {
    boundaries.deltaLocal = yamlInput["Boundaries"]["Local Boundary Layer Thickness"].as<double>();
  }
  else {
    boundaries.deltaLocal = 370;
  }

  if (yamlInput["Boundaries"]["Local Terrain Exponent"].IsDefined()) {
    boundaries.alphaLocal = yamlInput["Boundaries"]["Local Terrain Exponent"].as<double>();
  }
  else {
    boundaries.alphaLocal = 0.22;
  }

  if  (yamlInput["Boundaries"]["Convection Calculation Method"].IsDefined())
  {
    if (yamlInput["Boundaries"]["Convection Calculation Method"].as<std::string>() == "AUTO")
      foundation.convectionCalculationMethod = Foundation::CCM_AUTO;
    else if (yamlInput["Boundaries"]["Convection Calculation Method"].as<std::string>() == "CONSTANT")
    {
      foundation.convectionCalculationMethod = Foundation::CCM_CONSTANT_COEFFICIENT;
      foundation.interiorConvectiveCoefficient = yamlInput["Boundaries"]["Interior Convective Coefficient"].as<double>();
      foundation.exteriorConvectiveCoefficient = yamlInput["Boundaries"]["Exterior Convective Coefficient"].as<double>();
    }
  }
  else
  {
    foundation.convectionCalculationMethod = Foundation::CCM_AUTO;
  }

  if  (yamlInput["Boundaries"]["Outdoor Air Temperature Method"].IsDefined())
  {
    if (yamlInput["Boundaries"]["Outdoor Air Temperature Method"].as<std::string>() == "WEATHER-FILE")
      boundaries.outdoorTemperatureMethod = Boundaries::OTM_WEATHER_FILE;
    else if (yamlInput["Boundaries"]["Outdoor Air Temperature Method"].as<std::string>() == "CONSTANT")
    {
      boundaries.outdoorTemperatureMethod = Boundaries::OTM_CONSTANT_TEMPERATURE;
      boundaries.outdoorDryBulbTemperature = yamlInput["Boundaries"]["Outdoor Air Temperature"].as<double>();
    }
  }
  else
  {
    boundaries.outdoorTemperatureMethod = Boundaries::OTM_WEATHER_FILE;
  }

  if  (yamlInput["Boundaries"]["Wall Top Boundary Condition"].IsDefined())
  {
    if (yamlInput["Boundaries"]["Wall Top Boundary Condition"].as<std::string>() == "ZERO-FLUX")
      foundation.wallTopBoundary = Foundation::WTB_ZERO_FLUX;
    else if (yamlInput["Boundaries"]["Wall Top Boundary Condition"].as<std::string>() == "LINEAR-DT")
    {
      foundation.wallTopBoundary = Foundation::WTB_LINEAR_DT;
      foundation.wallTopInteriorTemperature = yamlInput["Boundaries"]["Wall Top Interior Temperature"].as<double>();
      foundation.wallTopExteriorTemperature = yamlInput["Boundaries"]["Wall Top Exterior Temperature"].as<double>();
    }
  }
  else
  {
    foundation.wallTopBoundary = Foundation::WTB_ZERO_FLUX;
  }

  // INITIALIZATION
  if  (yamlInput["Initialization"]["Initialization Method"].IsDefined())
  {
    if (yamlInput["Initialization"]["Initialization Method"].as<std::string>() == "KUSUDA")
      initialization.initializationMethod = Initialization::IM_KUSUDA;
    else if (yamlInput["Initialization"]["Initialization Method"].as<std::string>() == "STEADY-STATE")
      initialization.initializationMethod = Initialization::IM_STEADY_STATE;
    else if (yamlInput["Initialization"]["Initialization Method"].as<std::string>() == "CONSTANT")
    {
      initialization.initializationMethod = Initialization::IM_CONSTANT_TEMPERATURE;
      initialization.initialTemperature = yamlInput["Initialization"]["Initial Temperature"].as<double>();
    }
  }
  else
  {
    initialization.initializationMethod = Initialization::IM_STEADY_STATE;
  }

  if  (yamlInput["Initialization"]["Accelerated Initialization Timestep"].IsDefined())
  {
    initialization.implicitAccelTimestep = yamlInput["Initialization"]["Accelerated Initialization Timestep"].as<long>();
  }
  else
  {
    initialization.implicitAccelTimestep = 168;
  }

  if  (yamlInput["Initialization"]["Number of Accelearted Initialization Timesteps"].IsDefined())
  {
    initialization.implicitAccelPeriods = yamlInput["Initialization"]["Number of Accelearted Initialization Timesteps"].as<long>();
  }
  else
  {
    initialization.implicitAccelPeriods = 12;
  }

  if  (yamlInput["Initialization"]["Number of Warmup Days in Initialization"].IsDefined())
  {
    initialization.warmupDays = yamlInput["Initialization"]["Number of Warmup Days in Initialization"].as<long>();
  }
  else
  {
    initialization.warmupDays = 365;
  }

  // OUTPUT

  // CSV Reports
  if  (yamlInput["Output"]["Output Report"].IsDefined())
  {
    for(size_t i=0;i<yamlInput["Output"]["Output Report"]["Reports"].size();i++)
    {
      OutputVariable temp(yamlInput["Output"]["Output Report"]["Reports"][i].as<int>());
      output.outputReport.push_back(temp);
    }

    if  (yamlInput["Output"]["Output Report"]["Minimum Reporting Frequency"].IsDefined())
    {
      output.outputReport.minFrequency =
          boost::posix_time::minutes(
              yamlInput["Output"]["Output Report"]["Minimum Reporting Frequency"].as<long>());
    }
    else
    {
      output.outputReport.minFrequency = boost::posix_time::minutes(60);
    }
    output.outputReport.setOutputMap();
  }

  // Animations/Plots
  for(size_t i=0;i<yamlInput["Output"]["Output Snapshots"].size();i++)
  {
    OutputAnimation temp;
    temp.dir = yamlInput["Output"]["Output Snapshots"][i]["Directory"].as<std::string>();

    if (yamlInput["Output"]["Output Snapshots"][i]["Size"].IsDefined()) {
      temp.size = yamlInput["Output"]["Output Snapshots"][i]["Size"].as<int>();
    }
    else {
      temp.size = 800;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Frequency"].IsDefined()) {
      temp.frequency = boost::posix_time::hours(yamlInput["Output"]["Output Snapshots"][i]["Frequency"].as<long>());
    }
    else {
      temp.frequency = boost::posix_time::hours(36);
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Mesh"].IsDefined()) {
      temp.grid = yamlInput["Output"]["Output Snapshots"][i]["Mesh"].as<bool>();
    }
    else {
      temp.grid = false;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Gradients"].IsDefined()) {
      temp.gradients = yamlInput["Output"]["Output Snapshots"][i]["Gradients"].as<bool>();
    }
    else {
      temp.gradients = false;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Contours"].IsDefined()) {
      temp.contours = yamlInput["Output"]["Output Snapshots"][i]["Contours"].as<bool>();
    }
    else {
      temp.contours = true;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Contour Labels"].IsDefined()) {
      temp.contourLabels = yamlInput["Output"]["Output Snapshots"][i]["Contour Labels"].as<bool>();
    }
    else {
      temp.contourLabels = false;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Axes"].IsDefined()) {
      temp.axes = yamlInput["Output"]["Output Snapshots"][i]["Axes"].as<bool>();
    }
    else {
      temp.axes = true;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Timestamp"].IsDefined()) {
      temp.timestamp = yamlInput["Output"]["Output Snapshots"][i]["Timestamp"].as<bool>();
    }
    else {
      temp.timestamp = true;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Plot Type"].IsDefined())
    {
      if (yamlInput["Output"]["Output Snapshots"][i]["Plot Type"].as<std::string>() == "TEMPERATURE")
        temp.plotType = OutputAnimation::P_TEMP;
      else if (yamlInput["Output"]["Output Snapshots"][i]["Plot Type"].as<std::string>() == "HEAT-FLUX")
        temp.plotType = OutputAnimation::P_FLUX;
    }
    else
      temp.plotType = OutputAnimation::P_TEMP;

    if (yamlInput["Output"]["Output Snapshots"][i]["Flux Direction"].IsDefined())
    {
      if (yamlInput["Output"]["Output Snapshots"][i]["Flux Direction"].as<std::string>() == "MAG")
        temp.fluxDir = OutputAnimation::D_M;
      else if (yamlInput["Output"]["Output Snapshots"][i]["Flux Direction"].as<std::string>() == "X")
        temp.fluxDir = OutputAnimation::D_X;
      else if (yamlInput["Output"]["Output Snapshots"][i]["Flux Direction"].as<std::string>() == "Y")
        temp.fluxDir = OutputAnimation::D_Y;
      else if (yamlInput["Output"]["Output Snapshots"][i]["Flux Direction"].as<std::string>() == "Z")
        temp.fluxDir = OutputAnimation::D_Z;
    }
    else
      temp.fluxDir = OutputAnimation::D_M;

    if (yamlInput["Output"]["Output Snapshots"][i]["Color Scheme"].IsDefined())
    {
      if (yamlInput["Output"]["Output Snapshots"][i]["Color Scheme"].as<std::string>() == "CMR")
        temp.colorScheme = OutputAnimation::C_CMR;
      else if (yamlInput["Output"]["Output Snapshots"][i]["Color Scheme"].as<std::string>() == "JET")
        temp.colorScheme = OutputAnimation::C_JET;
      else if (yamlInput["Output"]["Output Snapshots"][i]["Color Scheme"].as<std::string>() == "NONE")
        temp.colorScheme = OutputAnimation::C_NONE;
    }
    else
      temp.colorScheme = OutputAnimation::C_CMR;

    if (yamlInput["Output"]["Output Snapshots"][i]["File Format"].IsDefined())
    {
      if (yamlInput["Output"]["Output Snapshots"][i]["File Format"].as<std::string>() == "PNG")
        temp.format = OutputAnimation::F_PNG;
      else if (yamlInput["Output"]["Output Snapshots"][i]["File Format"].as<std::string>() == "TEX")
        temp.format = OutputAnimation::F_TEX;
    }
    else
      temp.format = OutputAnimation::F_PNG;

    if (yamlInput["Output"]["Output Snapshots"][i]["Unit System"].IsDefined())
    {
      if (yamlInput["Output"]["Output Snapshots"][i]["Unit System"].as<std::string>() == "IP")
        temp.outputUnits = OutputAnimation::IP;
      else if (yamlInput["Output"]["Output Snapshots"][i]["Unit System"].as<std::string>() == "SI")
        temp.outputUnits = OutputAnimation::SI;
    }
    else
      temp.outputUnits = OutputAnimation::SI;

    if (yamlInput["Output"]["Output Snapshots"][i]["Output Range"].IsDefined())
    {
      temp.minimumTemperature = yamlInput["Output"]["Output Snapshots"][i]["Output Range"][0].as<double>();
      temp.maximumTemperature = yamlInput["Output"]["Output Snapshots"][i]["Output Range"][1].as<double>();
    }
    else
    {
      temp.minimumTemperature = -20;
      temp.maximumTemperature = 40;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Number of Contours"].IsDefined())
    {
      temp.numberOfContours = yamlInput["Output"]["Output Snapshots"][i]["Number of Contours"].as<int>();
    }
    else
    {
      temp.numberOfContours = 13;
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Contour Color"].IsDefined())
    {
      temp.contourColor = yamlInput["Output"]["Output Snapshots"][i]["Contour Color"].as<std::string>();
    }
    else
    {
      temp.contourColor = "H";
    }

    if (yamlInput["Output"]["Output Snapshots"][i]["Start Date"].IsDefined())
    {
      temp.startDate = boost::gregorian::from_string(yamlInput["Output"]["Output Snapshots"][i]["Start Date"].as<std::string>());
      temp.startDateSet = true;
    }
    else
      temp.startDateSet = false;

    if (yamlInput["Output"]["Output Snapshots"][i]["End Date"].IsDefined())
    {
      temp.endDate = boost::gregorian::from_string(yamlInput["Output"]["Output Snapshots"][i]["End Date"].as<std::string>());
      temp.endDateSet = true;
    }
    else
      temp.endDateSet = false;

    if (yamlInput["Output"]["Output Snapshots"][i]["X Range"].IsDefined())
    {
      temp.xRange.first = yamlInput["Output"]["Output Snapshots"][i]["X Range"][0].as<double>();
      temp.xRange.second = yamlInput["Output"]["Output Snapshots"][i]["X Range"][1].as<double>();
      temp.xRangeSet = true;
    }
    else
      temp.xRangeSet = false;

    if (yamlInput["Output"]["Output Snapshots"][i]["Y Range"].IsDefined())
    {
      temp.yRange.first = yamlInput["Output"]["Output Snapshots"][i]["Y Range"][0].as<double>();
      temp.yRange.second = yamlInput["Output"]["Output Snapshots"][i]["Y Range"][1].as<double>();
      temp.yRangeSet = true;
    }
    else
      temp.yRangeSet = false;

    if (yamlInput["Output"]["Output Snapshots"][i]["Z Range"].IsDefined())
    {

      temp.zRange.first = yamlInput["Output"]["Output Snapshots"][i]["Z Range"][0].as<double>();
      temp.zRange.second = yamlInput["Output"]["Output Snapshots"][i]["Z Range"][1].as<double>();
      temp.zRangeSet = true;
    }
    else
      temp.zRangeSet = false;

    output.outputAnimations.push_back(temp);
  }

  // Full Input
  input.simulationControl = simulationControl;
  input.foundation = foundation;
  input.boundaries = boundaries;
  input.initialization = initialization;
  input.output = output;


  return input;

}
