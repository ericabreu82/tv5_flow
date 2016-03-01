/*  Copyright (C) 2011-2012 National Institute For Space Research (INPE) - Brazil.

This file is part of the TerraLib - a Framework for building GIS enabled applications.

TerraLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

TerraLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with TerraLib. See COPYING. If not, write to
TerraLib Team at <terralib-team@terralib.org>.
*/

/*!
\file fiocruz/src/fiocruz/RasterInterpolate.cpp

\brief This class represents a raster interpolation
*/

#include "RasterInterpolate.h"
#include "SimpleMemDataSet.h"
#include "Utils.h"

#include "terralib/common/STLUtils.h"
#include "terralib/dataaccess/dataset/DataSet.h"
#include "terralib/dataaccess/dataset/DataSetType.h"
#include "terralib/dataaccess/utils/Utils.h"
#include "terralib/geometry/GeometryProperty.h"
#include "terralib/geometry/MultiPolygon.h"
#include "terralib/geometry/PointM.h"
#include "terralib/geometry/Polygon.h"
#include "terralib/raster/Band.h"
#include "terralib/raster/BandProperty.h"
#include "terralib/raster/Grid.h"
#include "terralib/raster/Raster.h"

#include <boost/lexical_cast.hpp>

te::qt::plugins::fiocruz::Ocurrencies te::qt::plugins::fiocruz::GetOcurrencies(const ComplexDataSet& ocurrenciesDataDriver, const std::string& destinyIdColumnName, const std::string& xColumnName, const std::string& yColumnName, const std::string& destinyIdFilter)
{
  Ocurrencies ocurrencies;

  te::da::DataSet* dataSet = ocurrenciesDataDriver.getDataSet();
  te::da::DataSetType* dataSetType = ocurrenciesDataDriver.getDataSetType();

  int typeX = dataSetType->getProperty(xColumnName)->getType();
  int typeY = dataSetType->getProperty(yColumnName)->getType();
  size_t indexX = dataSetType->getPropertyPosition(xColumnName);
  size_t indexY = dataSetType->getPropertyPosition(yColumnName);

  dataSet->moveBeforeFirst();
  while (dataSet->moveNext())
  {
    std::string destinyId = dataSet->getString(destinyIdColumnName);
    if (destinyIdFilter.empty() == false && destinyIdFilter != destinyId)
    {
      continue;
    }

    double x = 0.;
    double y = 0.;

    if (typeX == te::dt::DOUBLE_TYPE)
    {
      x = dataSet->getDouble(xColumnName);
    }
    else if (typeX == te::dt::STRING_TYPE)
    {
      std::string strValue = dataSet->getString(xColumnName);
      x = boost::lexical_cast<double>(strValue);
    }
    else
    {
      continue;
    }
    if (typeY == te::dt::DOUBLE_TYPE)
    {
      y = dataSet->getDouble(yColumnName);
    }
    else if (typeY == te::dt::STRING_TYPE)
    {
      std::string strValue = dataSet->getString(yColumnName);
      y = boost::lexical_cast<double>(strValue);
    }
    else
    {
      continue;
    }

    te::gm::Coord2D coord(x, y);

    Ocurrencies::iterator it = ocurrencies.find(destinyId);
    if (it != ocurrencies.end())
    {
      it->second.push_back(coord);
    }
    else
    {
      ocurrencies[destinyId].push_back(coord);
    }
  }

  return ocurrencies;
}
te::qt::plugins::fiocruz::Ocurrencies te::qt::plugins::fiocruz::GetOcurrencies(const ComplexDataSet& ocurrenciesDataDriver, const std::string& destinyIdColumnName, const std::string& originLinkColumnName, const ComplexDataSet& originDataDriver, const std::string& originIdColumnName, const std::string& destinyIdFilter)
{
  Ocurrencies ocurrencies;
  //we first get the centroids from the polygons of the originDataSet and create a map
  te::da::DataSet* ocurrenciesDataSet = ocurrenciesDataDriver.getDataSet();
  te::da::DataSetType* ocurrenciesDataSetType = ocurrenciesDataDriver.getDataSetType();

  te::da::DataSet* originDataSet = originDataDriver.getDataSet();
  te::da::DataSetType* originDataSetType = originDataDriver.getDataSetType();
  te::gm::GeometryProperty* originGeometryProperty = te::da::GetFirstGeomProperty(originDataSetType);
  te::gm::GeomType geomType = originGeometryProperty->getGeometryType();

  std::map<std::string, te::gm::Coord2D> mapOriginCentroids;

  originDataSet->moveBeforeFirst();
  while (originDataSet->moveNext())
  {
    std::string originId = originDataSet->getString(originIdColumnName);

    std::auto_ptr<te::gm::Geometry> geometry = originDataSet->getGeometry(originGeometryProperty->getName());
    if (geomType == te::gm::GeomType::PolygonType)
    {
      te::gm::Polygon* polygon = dynamic_cast<te::gm::Polygon*>(geometry.get());
      if (polygon == 0)
      {
        continue;
      }

      std::auto_ptr<te::gm::Coord2D> centroid(polygon->getCentroidCoord());
      te::gm::Coord2D coord = te::gm::Coord2D(centroid->getX(), centroid->getY());
      mapOriginCentroids[originId] = coord;
    }
    else if (geomType == te::gm::GeomType::MultiPolygonType)
    {
      te::gm::MultiPolygon* polygon = dynamic_cast<te::gm::MultiPolygon*>(geometry.get());
      if (polygon == 0)
      {
        continue;
      }

      std::auto_ptr<te::gm::Coord2D> centroid(polygon->getCentroidCoord());
      te::gm::Coord2D coord = te::gm::Coord2D(centroid->getX(), centroid->getY());
      mapOriginCentroids[originId] = coord;
    }
  }

  //now we create the ocurrencies map
  ocurrenciesDataSet->moveBeforeFirst();
  while (ocurrenciesDataSet->moveNext())
  {
    std::string destinyId = ocurrenciesDataSet->getString(destinyIdColumnName);
    if (destinyIdFilter.empty() == false && destinyIdFilter != destinyId)
    {
      continue;
    }

    std::string originId = ocurrenciesDataSet->getString(originLinkColumnName);
    std::map<std::string, te::gm::Coord2D>::iterator itOrigin = mapOriginCentroids.find(originId);
    if (itOrigin == mapOriginCentroids.end())
    {
      continue;
    }

    double x = itOrigin->second.getX();
    double y = itOrigin->second.getY();

    te::gm::Coord2D coord(x, y);

    Ocurrencies::iterator it = ocurrencies.find(destinyId);
    if (it != ocurrencies.end())
    {
      it->second.push_back(coord);
    }
    else
    {
      ocurrencies[destinyId].push_back(coord);
    }
  }

  return ocurrencies;
}

bool te::qt::plugins::fiocruz::BuildKDTree(const Ocurrencies& ocurrencies, KernelInterpolationAlgorithms::KD_ADAPTATIVE_TREE& tree)
{
  if (ocurrencies.empty() == true)
  {
    return false;
  }

  std::vector< std::pair< te::gm::Coord2D, te::gm::PointM > > inputVector;

  Ocurrencies::const_iterator it = ocurrencies.begin();
  while (it != ocurrencies.end())
  {
    const CoordVector& vecCoords = it->second;

    std::map<te::gm::Coord2D, int> mapCount;

    for (size_t i = 0; i < vecCoords.size(); ++i)
    {
      std::map<te::gm::Coord2D, int>::iterator itCount = mapCount.find(vecCoords[i]);
      if (itCount == mapCount.end())
      {
        mapCount[vecCoords[i]] = 1;
      }
      else
      {
        itCount->second = itCount->second + 1;
      }
    }

    std::map<te::gm::Coord2D, int>::iterator itCount = mapCount.begin();
    while (itCount != mapCount.end())
    {
      te::gm::Coord2D coord = itCount->first;
      
      te::gm::PointM point(coord.getX(), coord.getY(), itCount->second);

      inputVector.push_back(std::pair<te::gm::Coord2D, te::gm::PointM>(coord, point));

      ++itCount;
    }

    ++it;
  }

  tree.build(inputVector);

  return true;
}

bool te::qt::plugins::fiocruz::RasterInterpolate(const Ocurrencies& ocurrencies,
  te::rst::Raster* outputRaster, const int& band,
  const KernelInterpolationAlgorithm& algorithm,
  const te::sa::KernelFunctionType& method,
  const size_t& numberOfNeighbors, const double& boxRatio)
{
  if ((ocurrencies.empty() == true) || (outputRaster == 0))
  {
    return false;
  }

  // A minimum of MINBUCKETSIZE elements in each bucket
  const te::rst::Grid* grid = outputRaster->getGrid();
  const te::gm::Envelope* mbr = outputRaster->getExtent();
  int rasterSRID = outputRaster->getSRID();
  size_t bucketSize = 2 * numberOfNeighbors;

  // load tree with samaples
  KernelInterpolationAlgorithms::KD_ADAPTATIVE_TREE tree(*mbr, bucketSize);

  if (BuildKDTree(ocurrencies, tree) == false)
  {
    return false;
  }

  if (tree.size() == 0)
  {
    return false;
  }

  KernelInterpolationAlgorithms interpolationObj(tree);

  int nLines = outputRaster->getNumberOfRows();
  int nCols = outputRaster->getNumberOfColumns();

  double value = outputRaster->getBand(0)->getProperty()->m_noDataValue;

  if (algorithm == TeDistWeightAvgInterpolation)
  {
    for (int i = 0; i < nLines; ++i)
    {
      for (int j = 0; j < nCols; ++j)
      {
        te::gm::Coord2D coord = grid->gridToGeo(j, i);
        value = interpolationObj.distWeightAvgNearestNeighbor(coord, numberOfNeighbors, method);
        outputRaster->setValue(j, i, value, band);
      }
    }
  }
  else if (algorithm == TeDistWeightAvgInBoxInterpolation)
  {
    for (int i = 0; i < nLines; ++i)
    {
      for (int j = 0; j < nCols; ++j)
      {
        te::gm::Coord2D coord = grid->gridToGeo(j, i);
        te::gm::Envelope box(coord.getX() - boxRatio, coord.getY() - boxRatio, coord.getX() + boxRatio, coord.getY() + boxRatio);

        value = interpolationObj.boxDistWeightAvg(coord, box, method);

        outputRaster->setValue(j, i, value, band);
      }
    }
  }
  else
  {
    return false;
  }

  return true;
}

std::vector<std::string> te::qt::plugins::fiocruz::CreateDominancesMaps(std::string path, std::string baseName, std::vector<te::rst::Raster*> rasters, std::vector<te::qt::plugins::fiocruz::DominanceParams> dpVec)
{
  std::vector<std::string> paths;

  std::vector<te::rst::Raster*> outRasters;

  if (rasters.empty())
    return paths;

  //build output rasters
  te::rst::Raster* refRaster = rasters[0];

  for (size_t i = 0; i < dpVec.size(); ++i)
  {
    std::string domTitle = dpVec[i].m_propertyName;

    std::string fileName = path + "/" + baseName + "_" + domTitle + ".tif";

    te::gm::Envelope* env = new te::gm::Envelope(*refRaster->getExtent());

    te::rst::Raster* outputRaster = te::qt::plugins::fiocruz::CreateRaster(fileName, env, refRaster->getGrid()->getResolutionX(), refRaster->getGrid()->getResolutionY(), refRaster->getSRID(), te::dt::UCHAR_TYPE);

    outRasters.push_back(outputRaster);

    paths.push_back(fileName);
  }

  //calculate dominances
  for (size_t i = 0; i < dpVec.size(); ++i)
  {
    double levMin = (double)dpVec[i].m_minLevel / 100.;
    double levMax = (double)dpVec[i].m_maxLevel / 100.;

    for (int lin = 0; lin < refRaster->getNumberOfRows(); lin++)
    {
      for (int col = 0; col < refRaster->getNumberOfColumns(); col++)
      {
        std::vector<double> vecValues;
        double val;
        double total = 0.;

        for (std::size_t vecPos = 0; vecPos < rasters.size(); vecPos++)
        {
          rasters[vecPos]->getValue(col, lin, val);
          vecValues.push_back(val);
          total += val;
        }

        if (total == 0.)
        {
          outRasters[i]->setValue(col, lin, 0);
          continue;
        }

        //generate individual kernel maps -> each raster / total
        std::vector<double>::iterator itValues = vecValues.begin();
        int count = 0;
        bool check = false;
        double secundary = 0.;

        while (itValues != vecValues.end())
        {
          double result = *itValues / total;

          //evitar a interseccao entre os mapas de mercado
          if (result > levMax)
          {
            check = false;
            break;
          }

          if (result > levMin && result <= levMax && result > secundary)
          {
            secundary = result;

            outRasters[i]->setValue(col, lin, count + 1);

            check = true;
          }

          if (result == levMax && levMax == 1)
          {
            secundary = result;

            outRasters[i]->setValue(col, lin, count + 1);

            check = true;
          }

          ++itValues;
          ++count;
        }

        if (!check)
        {
          outRasters[i]->setValue(col, lin, 0);
        }
      }
    }
  }

  te::common::FreeContents(outRasters);

  return paths;
}

std::vector<std::string> te::qt::plugins::fiocruz::CreateIndividualRegionalization(std::string path, std::string baseName, std::vector<std::string> objs, std::vector<te::rst::Raster*> rasters)
{
  std::vector<std::string> paths;

  std::vector<te::rst::Raster*> outRasters;

  if (rasters.empty())
    return paths;

  //build output rasters
  te::rst::Raster* refRaster = rasters[0];

  for (size_t i = 0; i < objs.size(); ++i)
  {
    std::string currentDestiny = objs[i];

    std::string fileName = path + "/" + baseName + "_" + currentDestiny + "_reg.tif";

    te::gm::Envelope* env = new te::gm::Envelope(*refRaster->getExtent());

    te::rst::Raster* outputRaster = te::qt::plugins::fiocruz::CreateRaster(fileName, env, refRaster->getGrid()->getResolutionX(), refRaster->getGrid()->getResolutionY(), refRaster->getSRID());

    outRasters.push_back(outputRaster);

    paths.push_back(fileName);
  }

  //fill output rasters
  for (unsigned int lin = 0; lin < refRaster->getNumberOfRows(); lin++)
  {
    for (unsigned int col = 0; col < refRaster->getNumberOfColumns(); col++)
    {
      std::vector<double> vecValues;
      double val;
      double total = 0.;

      for (std::size_t vecPos = 0; vecPos < rasters.size(); vecPos++)
      {
        rasters[vecPos]->getValue(col, lin, val);
        vecValues.push_back(val);
        total += val;
      }

      for (std::size_t vecPos = 0; vecPos < vecValues.size(); vecPos++)
      {
        double value = 0.;

        if (total != 0.)
        {
          value = vecValues[vecPos] / total;
        }

        outRasters[vecPos]->setValue(col, lin, value);
      }
    }
  }

  te::common::FreeContents(outRasters);

  return paths;
}