/* Copyright (c) 2012-2016 Big Ladder Software. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GroundPlot_CPP
#define GroundPlot_CPP

#include "GroundPlot.hpp"

static const double EPSILON = 1E-5;

namespace Kiva {

SnapshotSettings::SnapshotSettings() :
  size(800),
  frequency(129600.0),
  grid(false),
  gradients(false),
  contours(true),
  contourLabels(false),
  axes(true),
  timestamp(true),
  plotType(SnapshotSettings::P_TEMP),
  fluxDir(SnapshotSettings::D_M),
  colorScheme(SnapshotSettings::C_CMR),
  format(SnapshotSettings::F_PNG),
  outputUnits(SnapshotSettings::SI),
  minValue(-20.0),
  maxValue(40.0),
  numberOfContours(13),
  contourColor("H")
{}



GroundPlot::GroundPlot(SnapshotSettings &snapshotSettings, Domain &domain, std::vector<Block> &blocks) :
  snapshotSettings(snapshotSettings), blocks(blocks)
{

  boost::filesystem::remove_all(snapshotSettings.dir);
  boost::filesystem::create_directory(snapshotSettings.dir);

  frameNumber = 0;

  if (snapshotSettings.outputUnits == SnapshotSettings::IP)
    distanceUnitConversion = 3.28084;
  else
    distanceUnitConversion = 1;

  std::size_t contourLevels = snapshotSettings.numberOfContours;

  if (isEqual(snapshotSettings.xRange.first,snapshotSettings.xRange.second))
  {
    iMin = domain.meshX.getNearestIndex(snapshotSettings.xRange.first);
    iMax = domain.meshX.getNearestIndex(snapshotSettings.xRange.second);
  }
  else
  {
    iMin = domain.meshX.getPreviousIndex(snapshotSettings.xRange.first);
    iMax = domain.meshX.getNextIndex(snapshotSettings.xRange.second);

    // Check for exact match
    if (isEqual(domain.meshX.centers[iMin + 1],snapshotSettings.xRange.first))
      iMin += 1;
    if (isEqual(domain.meshX.centers[iMax - 1],snapshotSettings.xRange.second))
      iMax -= 1;
  }

  if (isEqual(snapshotSettings.yRange.first,snapshotSettings.yRange.second))
  {
    jMin = domain.meshY.getNearestIndex(snapshotSettings.yRange.first);
    jMax = domain.meshY.getNearestIndex(snapshotSettings.yRange.second);
  }
  else
  {
    jMin = domain.meshY.getPreviousIndex(snapshotSettings.yRange.first);
    jMax = domain.meshY.getNextIndex(snapshotSettings.yRange.second);

    // Check for exact match
    if (isEqual(domain.meshY.centers[jMin + 1],snapshotSettings.yRange.first))
      jMin += 1;
    if (isEqual(domain.meshY.centers[jMax - 1],snapshotSettings.yRange.second))
      jMax -= 1;
  }

  if (isEqual(snapshotSettings.zRange.first,snapshotSettings.zRange.second))
  {
    kMin = domain.meshZ.getNearestIndex(snapshotSettings.zRange.first);
    kMax = domain.meshZ.getNearestIndex(snapshotSettings.zRange.second);
  }
  else
  {
    kMin = domain.meshZ.getPreviousIndex(snapshotSettings.zRange.first);
    kMax = domain.meshZ.getNextIndex(snapshotSettings.zRange.second);

    // Check for exact match
    if (isEqual(domain.meshZ.centers[kMin + 1],snapshotSettings.zRange.first))
      kMin += 1;
    if (isEqual(domain.meshZ.centers[kMax - 1],snapshotSettings.zRange.second))
      kMax -= 1;
  }

  std::size_t nI = iMax - iMin + 1;
  std::size_t nJ = jMax - jMin + 1;
  std::size_t nK = kMax - kMin + 1;

  xMin = domain.meshX.dividers[0];
  xMax = domain.meshX.dividers[domain.nX];

  yMin = domain.meshY.dividers[0];
  yMax = domain.meshY.dividers[domain.nY];


  if (nI == 1)
  {
    sliceType = YZ;

    hAxis.nN = nJ;
    hAxis.nMin = jMin;
    hAxis.nMax = jMax;
    hAxis.mesh = domain.meshY;

    vAxis.nN = nK;
    vAxis.nMin = kMin;
    vAxis.nMax = kMax;
    vAxis.mesh = domain.meshZ;

    slice = domain.meshX.centers[iMin];
  }
  else if (nJ == 1)
  {
    if (domain.meshY.centers.size() == 1)
      sliceType = XZ_2D;
    else
      sliceType = XZ;

    hAxis.nN = nI;
    hAxis.nMin = iMin;
    hAxis.nMax = iMax;
    hAxis.mesh = domain.meshX;

    vAxis.nN = nK;
    vAxis.nMin = kMin;
    vAxis.nMax = kMax;
    vAxis.mesh = domain.meshZ;

    slice = domain.meshY.centers[jMin];
  }
  else if (nK == 1)
  {
    sliceType = XY;

    hAxis.nN = nI;
    hAxis.nMin = iMin;
    hAxis.nMax = iMax;
    hAxis.mesh = domain.meshX;

    vAxis.nN = nJ;
    vAxis.nMin = jMin;
    vAxis.nMax = jMax;
    vAxis.mesh = domain.meshY;

    slice = domain.meshZ.centers[kMin];
  }

  mglData TDatRef(hAxis.nN,vAxis.nN),
      hDatRef(hAxis.nN),
      vDatRef(vAxis.nN),
      hGridRef(hAxis.nN + 1),
      vGridRef(vAxis.nN + 1),
      TGridRef(hAxis.nN + 1, vAxis.nN + 1),
      cDatRef(contourLevels);

  TDat = TDatRef;
  hDat = hDatRef;
  vDat = vDatRef;
  hGrid = hGridRef;
  vGrid = vGridRef;
  TGrid = TGridRef;
  cDat = cDatRef;

  hGrid.a[0] = hAxis.mesh.centers[hAxis.nMin]*distanceUnitConversion;

  for(size_t i = 0; i < hAxis.nN - 1; i++)
  {
    hDat.a[i] = hAxis.mesh.centers[i + hAxis.nMin]*distanceUnitConversion;
    hGrid.a[i + 1] = hAxis.mesh.dividers[i + hAxis.nMin + 1]*distanceUnitConversion;
  }

  hDat.a[hAxis.nN - 1] = hAxis.mesh.centers[hAxis.nMax]*distanceUnitConversion;
  hGrid.a[hAxis.nN] = hAxis.mesh.centers[hAxis.nMax]*distanceUnitConversion;

  vGrid.a[0] = vAxis.mesh.centers[vAxis.nMin]*distanceUnitConversion;

  for(size_t j = 0; j < vAxis.nN - 1; j++)
  {
    vDat.a[j] = vAxis.mesh.centers[j + vAxis.nMin]*distanceUnitConversion;
    vGrid.a[j + 1] = vAxis.mesh.dividers[j + vAxis.nMin + 1]*distanceUnitConversion;
  }

  vDat.a[vAxis.nN - 1] = vAxis.mesh.centers[vAxis.nMax]*distanceUnitConversion;
  vGrid.a[vAxis.nN] = vAxis.mesh.centers[vAxis.nMax]*distanceUnitConversion;

  for(size_t j = 0; j <= vAxis.nN; j++)
  {
    for(size_t i = 0; i <= hAxis.nN; i++)
    {
      TGrid.a[i+hAxis.nN*j] = 200.0;
    }
  }

  double min = snapshotSettings.minValue;
  double max = snapshotSettings.maxValue;
  double step = (max - min) / (contourLevels - 1);

  for (size_t n = 0; n < contourLevels; n++)
      cDat.a[n] = min + double(n)*step;

}

void GroundPlot::createFrame(std::string timeStamp)
{


  std::string distanceUnit;
  std::string temperatureUnit;
  std::string fluxUnit;

  if (snapshotSettings.outputUnits == SnapshotSettings::IP)
  {
    distanceUnit = "ft";
    temperatureUnit = "\\textdegree F";
    fluxUnit = "W/ft^2";
  }
  else
  {
    distanceUnit = "m";
    temperatureUnit = "\\textdegree C";
    fluxUnit = "W/m^2";
  }

  double hMin = hGrid.a[0];
  double hMax = hGrid.a[hAxis.nN];
  double hRange = hMax - hMin;

  double vMin = vGrid.a[0];
  double vMax = vGrid.a[vAxis.nN];
  double vRange = vMax - vMin;

  int nT = cDat.GetNN();
  double Tmin = cDat.a[0];
  double Tmax = cDat.a[nT - 1];
  double Tstep = cDat.a[1] - cDat.a[0];

  // Text properties
  double hText = 0.05;
  double vText = 0.95;

  double vTextSpacing = 0.05;

  mglGraph gr;
  gr.LoadFont("heros");

  // Plot
  gr.Clf(1,1,1);
  double aspect = 1.0;
  int height = snapshotSettings.size;
  int width = height*aspect;


  gr.SetSize(width,height);

  gr.SetFontSize(2.0);
  gr.SetRange('x', hGrid);
  gr.SetRange('y', vGrid);
  gr.SetRange('c', Tmin, Tmax);
  gr.SetRange('z', Tmin, Tmax);
  gr.SetTicks('c', Tstep, nT, Tmin);
  gr.Aspect(hRange, vRange);


  // Timestamp

  if (snapshotSettings.axes)
  {
    if (snapshotSettings.colorScheme != SnapshotSettings::C_NONE)
    {
      if  (snapshotSettings.plotType == SnapshotSettings::P_TEMP)
        gr.Puts(0.9, 0.056, temperatureUnit.c_str(), ":AL");
      else
        gr.Puts(0.9, 0.056, fluxUnit.c_str(), ":AL");
    }
  }

  if (snapshotSettings.timestamp)
    gr.Puts(hText, vText, timeStamp.c_str(), ":AL");

  switch (sliceType)
  {
  case XZ_2D:
  {
  }
  break;
  case XY:
  {
    std::string sliceString = "Z = " + str(boost::format("%0.2f") % (slice*distanceUnitConversion)) + " " + distanceUnit;
    if (snapshotSettings.axes)
      gr.Puts(hText, vText - vTextSpacing, sliceString.c_str(), ":AL");
  }
  break;
  case XZ:
  {
    std::string sliceString = "Y = " + str(boost::format("%0.2f") % (slice*distanceUnitConversion)) + " " + distanceUnit;
    if (snapshotSettings.axes)
      gr.Puts(hText, vText - vTextSpacing, sliceString.c_str(), ":AL");
  }
  break;
  case YZ:
  {
    std::string sliceString = "X = " + str(boost::format("%0.2f") % (slice*distanceUnitConversion)) + " " + distanceUnit;
    if (snapshotSettings.axes)
      gr.Puts(hText, vText - vTextSpacing, sliceString.c_str(), ":AL");
  }
  }
  gr.SetPlotFactor(1.3);

  if (snapshotSettings.axes)
  {
    gr.Axis("yU");
    gr.Axis("x");
    if (snapshotSettings.colorScheme == SnapshotSettings::C_CMR)
    {
      gr.Colorbar("kUrqyw_");
    }
    else if (snapshotSettings.colorScheme == SnapshotSettings::C_JET)
    {
      gr.Colorbar("BbcyrR_");
    }
  }

  if (snapshotSettings.colorScheme == SnapshotSettings::C_CMR)
  {
    gr.Dens(hDat, vDat, TDat,"kUrqyw");
  }
  else if (snapshotSettings.colorScheme == SnapshotSettings::C_JET)
  {
    gr.Dens(hDat, vDat, TDat,"BbcyrR");
  }

  gr.Box("k",false);

  if (snapshotSettings.contours)
  {
    if (snapshotSettings.contourLabels)
      gr.Cont(cDat, hDat, vDat, TDat,(snapshotSettings.contourColor + "t").c_str());
    else
      gr.Cont(cDat, hDat, vDat, TDat,snapshotSettings.contourColor.c_str());
  }
  if (snapshotSettings.gradients)
    gr.Grad(hDat, vDat, TDat);
  if (snapshotSettings.grid)
    gr.Grid(hGrid, vGrid, TGrid, "W");


  // Draw blocks
  for (size_t b = 0; b < blocks.size(); b++)
  {
    switch (sliceType)
    {
    case XZ_2D:
    {
      mglPoint bl = mglPoint(std::min(std::max(blocks[b].xMin*distanceUnitConversion, hMin),hMax),
                   std::min(std::max(blocks[b].zMin*distanceUnitConversion, vMin),vMax),
                   210.0);
      mglPoint br = mglPoint(std::max(std::min(blocks[b].xMax*distanceUnitConversion, hMax),hMin),
                   std::min(std::max(blocks[b].zMin*distanceUnitConversion, vMin),vMax),
                   210.0);
      mglPoint tr = mglPoint(std::max(std::min(blocks[b].xMax*distanceUnitConversion, hMax),hMin),
                   std::max(std::min(blocks[b].zMax*distanceUnitConversion, vMax),vMin),
                   210.0);
      mglPoint tl = mglPoint(std::min(std::max(blocks[b].xMin*distanceUnitConversion, hMin),hMax),
                   std::max(std::min(blocks[b].zMax*distanceUnitConversion, vMax),vMin),
                   210.0);

      gr.Line(bl, br, "k");
      gr.Line(br, tr, "k");
      gr.Line(tr, tl, "k");
      gr.Line(tl, bl, "k");
    }
    break;
    case XY:
    {
      // Find intersection with viewing window
      Polygon view;
      view.outer().push_back(Point(hMin,vMin));
      view.outer().push_back(Point(hMin,vMax));
      view.outer().push_back(Point(hMax,vMax));
      view.outer().push_back(Point(hMax,vMin));

      MultiPolygon intersection;
      boost::geometry::intersection(view, blocks[b].polygon, intersection);

      // loop through each polygon in resulting multi_polygon
      for (std::size_t p = 0; p < intersection.size(); p++)
      {
        // loop through each vertex in each polygon and create a line
        std::size_t nV = intersection[p].outer().size();
        for (std::size_t v = 0; v < nV - 1; v++)
        {
          gr.Line(mglPoint(intersection[p].outer()[v].get<0>()*distanceUnitConversion,intersection[p].outer()[v].get<1>()*distanceUnitConversion,210.0),
              mglPoint(intersection[p].outer()[v+1].get<0>()*distanceUnitConversion,intersection[p].outer()[v+1].get<1>()*distanceUnitConversion,210.0),
              "k");
        }
        gr.Line(mglPoint(intersection[p].outer()[nV - 1].get<0>()*distanceUnitConversion,intersection[p].outer()[nV - 1].get<1>()*distanceUnitConversion,210.0),
            mglPoint(intersection[p].outer()[0].get<0>()*distanceUnitConversion,intersection[p].outer()[0].get<1>()*distanceUnitConversion,210.0),
            "k");
      }
    }
    break;
    case XZ:
      {
      // Find intersecting point pairs with slicing plane
      Line slicingPlane;
      slicingPlane.push_back(Point(xMin-EPSILON,slice));
      slicingPlane.push_back(Point(xMax+EPSILON,slice));

      MultiPoint intersection;
      boost::geometry::intersection(slicingPlane, blocks[b].polygon, intersection);

      // sort points in ascending order
      sort(intersection.begin(), intersection.end(), comparePointsX);

      for (std::size_t p = 0; p < intersection.size(); p++)
      {
        // Use point pairs and zmin/max to draw rectangles similar to 2D
        // case.
        double p1 = intersection[p].get<0>()*distanceUnitConversion;
        double p2 = intersection[p+1].get<0>()*distanceUnitConversion;


        mglPoint bl = mglPoint(std::min(std::max(p1, hMin),hMax),
                     std::min(std::max(blocks[b].zMin*distanceUnitConversion, vMin),vMax),
                     210.0);
        mglPoint br = mglPoint(std::max(std::min(p2, hMax),hMin),
                     std::min(std::max(blocks[b].zMin*distanceUnitConversion, vMin),vMax),
                     210.0);
        mglPoint tr = mglPoint(std::max(std::min(p2, hMax),hMin),
                     std::max(std::min(blocks[b].zMax*distanceUnitConversion, vMax),vMin),
                     210.0);
        mglPoint tl = mglPoint(std::min(std::max(p1, hMin),hMax),
                     std::max(std::min(blocks[b].zMax*distanceUnitConversion, vMax),vMin),
                     210.0);

        gr.Line(bl, br, "k");
        gr.Line(br, tr, "k");
        gr.Line(tr, tl, "k");
        gr.Line(tl, bl, "k");

        p += 1; // skip one point, on to the next pair
      }
      }
      break;
    case YZ:
      {
      // Find intersecting point pairs with slicing plane
      Line slicingPlane;
      slicingPlane.push_back(Point(slice,yMin-EPSILON));
      slicingPlane.push_back(Point(slice,yMax+EPSILON));

      MultiPoint intersection;
      boost::geometry::intersection(slicingPlane, blocks[b].polygon, intersection);

      // sort points in ascending order
      sort(intersection.begin(), intersection.end(), comparePointsY);

      for (std::size_t p = 0; p < intersection.size(); p++)
      {
        // Use point pairs and zmin/max to draw rectangles similar to 2D
        // case.
        double p1 = intersection[p].get<1>()*distanceUnitConversion;
        double p2 = intersection[p+1].get<1>()*distanceUnitConversion;


        mglPoint bl = mglPoint(std::min(std::max(p1, hMin),hMax),
                     std::min(std::max(blocks[b].zMin*distanceUnitConversion, vMin),vMax),
                     210.0);
        mglPoint br = mglPoint(std::max(std::min(p2, hMax),hMin),
                     std::min(std::max(blocks[b].zMin*distanceUnitConversion, vMin),vMax),
                     210.0);
        mglPoint tr = mglPoint(std::max(std::min(p2, hMax),hMin),
                     std::max(std::min(blocks[b].zMax*distanceUnitConversion, vMax),vMin),
                     210.0);
        mglPoint tl = mglPoint(std::min(std::max(p1, hMin),hMax),
                     std::max(std::min(blocks[b].zMax*distanceUnitConversion, vMax),vMin),
                     210.0);

        gr.Line(bl, br, "k");
        gr.Line(br, tr, "k");
        gr.Line(tr, tl, "k");
        gr.Line(tl, bl, "k");

        p += 1; // skip one point, on to the next pair
      }
      }
      break;
    }
  }

  if (snapshotSettings.format == SnapshotSettings::F_PNG)
    gr.WritePNG((snapshotSettings.dir + "/" + str(boost::format("%04d") % frameNumber) + ".png").c_str(),"",false);
  else if (snapshotSettings.format == SnapshotSettings::F_TEX)
    gr.WriteTEX((snapshotSettings.dir + "/" + str(boost::format("%04d") % frameNumber) + ".tex").c_str());

  frameNumber += 1;
  nextPlotTime += snapshotSettings.frequency;
}

bool GroundPlot::makeNewFrame(double t)
{
  if ((t >= nextPlotTime) && (t >= tStart) && (t <= tEnd))
    return true;
  else
    return false;
}

}

#endif