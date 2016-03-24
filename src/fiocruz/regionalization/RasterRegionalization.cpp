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
\file fiocruz/src/fiocruz/RasterRegionalization.cpp

\brief This file defines the algorithm for raster regionalization
*/

#include "RasterRegionalization.h"

#include "RasterInterpolate.h"
#include "SimpleMemDataSet.h"
#include "Utils.h"

#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleData.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/memory/DataSet.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/raster/Utils.h>
#include <terralib/vp/Utils.h>



te::qt::plugins::fiocruz::RasterRegionalization::RasterRegionalization()
{
}

te::qt::plugins::fiocruz::RasterRegionalization::~RasterRegionalization()
{

}

void te::qt::plugins::fiocruz::RasterRegionalization::setInputParameters(RegionalizationInputParams* inParams)
{
  m_inputParams.reset(inParams);
}

void te::qt::plugins::fiocruz::RasterRegionalization::setOutputParameters(RasterRegionalizationOutputParams* outParams)
{
  m_outputParams.reset(outParams);
}

bool te::qt::plugins::fiocruz::RasterRegionalization::generate(std::vector<std::string>& rastersPath, std::vector<te::rst::Raster*>& rasters)
{
  //for vector data
  te::da::DataSetPtr vecDataSet = m_inputParams->m_iVectorDataSet;
  std::string vecColumnOriginId = m_inputParams->m_iVectorColumnOriginId;
  std::auto_ptr<te::da::DataSetType> vecDataSetType = m_inputParams->m_iVectorDataSource->getDataSetType(m_inputParams->m_iVectorDataSetName);
  std::size_t geomColumnPos = te::da::GetFirstPropertyPos(vecDataSet.get(), te::dt::GEOMETRY_TYPE);
  te::gm::GeometryProperty* geomProperty = te::da::GetFirstGeomProperty(vecDataSetType.get());
  int srid = geomProperty->getSRID();

  ComplexDataSet vecDataDriver(vecDataSet.get(), vecDataSetType.get());

  //for tabular data
  te::da::DataSetPtr tabDataSet = m_inputParams->m_iTabularDataSet;
  std::string tabColumnOriginId = m_inputParams->m_iTabularColumnOriginId;
  std::string tabColumnDestinyId = m_inputParams->m_iTabularColumnDestinyId;
  std::auto_ptr<te::da::DataSetType> tabDataSetType = m_inputParams->m_iTabularDataSource->getDataSetType(m_inputParams->m_iTabularDataSetName);
  ComplexDataSet tabDataDriver(tabDataSet.get(), tabDataSetType.get());

  //raster parameters
  bool hasSpatialInformation = m_inputParams->m_hasSpatialInformation;
  te::sa::KernelFunctionType kernelFunction = m_inputParams->m_kernelFunction;
  size_t numberOfNeighbours = m_inputParams->m_numberOfNeighbours;
  double boxRatio = m_inputParams->m_boxRatio;

  std::string xAttrName = m_inputParams->m_xAttrName;
  std::string yAttrName = m_inputParams->m_yAttrName;

  double resX = m_inputParams->m_resX;
  double resY = m_inputParams->m_resY;

  std::string path = m_outputParams->m_path;
  std::string baseName = m_outputParams->m_baseName;

  std::auto_ptr<te::gm::Geometry> geometry(unitePolygonsFromDataSet(vecDataDriver));
  te::gm::MultiPolygon* multiPolygon = dynamic_cast<te::gm::MultiPolygon*>(geometry.get());
  if (multiPolygon == 0)
  {
    return false;
  }

  for (size_t i = 0; i < m_inputParams->m_objects.size(); ++i)
  {
    std::string currentDestiny = m_inputParams->m_objects[i];

    std::string fileName = path + "/" + baseName + "_" + currentDestiny + ".tif";
    std::string tempFileName = path + "/" + baseName + "_" + currentDestiny + "_temp_file.tif";

    //read the ocurrencies
    Ocurrencies ocurrencies;
    if (hasSpatialInformation == true)
    {
      ocurrencies = GetOcurrencies(tabDataDriver, tabColumnOriginId, xAttrName, yAttrName, currentDestiny);
    }
    else
    {
      ocurrencies = GetOcurrencies(tabDataDriver, tabColumnDestinyId, tabColumnOriginId, vecDataDriver, vecColumnOriginId, currentDestiny);
    }
    KernelInterpolationAlgorithm algorithm = m_inputParams->m_algorithm;

    //criar raster
    te::gm::Envelope* envelope = m_inputParams->m_iVectorDataSet->getExtent(geomColumnPos).release();

    std::auto_ptr<te::rst::Raster> outputRaster(te::qt::plugins::fiocruz::CreateRaster(tempFileName, envelope, resX, resY, srid));

    int band = 0;
    RasterInterpolate(ocurrencies, outputRaster.get(), band, algorithm, kernelFunction, numberOfNeighbours, boxRatio);

    outputRaster.reset(te::qt::plugins::fiocruz::ClipRaster(outputRaster.get(), multiPolygon, fileName));

    rastersPath.push_back(fileName);
    rasters.push_back(outputRaster.release());
  }

  return true;
}

