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
\file fiocruz/src/fiocruz/Utils.cpp

\brief This file defines common functions for regionalization plugin
*/

#include "Utils.h"

// TerraLib
#include <terralib/common/Globals.h>
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/geometry/Geometry.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Polygon.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/maptools/Grouping.h>
#include <terralib/maptools/GroupingAlgorithms.h>
#include <terralib/maptools/GroupingItem.h>
#include <terralib/maptools/Utils.h>
#include <terralib/raster/BandProperty.h>
#include <terralib/raster/Grid.h>
#include <terralib/raster/PositionIterator.h>
#include <terralib/raster/Raster.h>
#include <terralib/raster/RasterFactory.h>
#include <terralib/raster/RasterSummary.h>
#include <terralib/raster/RasterSummaryManager.h>
#include <terralib/raster/Utils.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/se/ColorMap.h>
#include <terralib/se/Enums.h>
#include <terralib/se/Interpolate.h>
#include <terralib/se/InterpolationPoint.h>
#include <terralib/se/MapItem.h>
#include <terralib/se/ParameterValue.h>
#include <terralib/se/RasterSymbolizer.h>
#include <terralib/se/Recode.h>
#include <terralib/se/Utils.h>

// Qt
#include <QColor>
#include <QString>

//Boost
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

te::rst::Raster* te::qt::plugins::fiocruz::CreateRaster(const std::string& fileName, te::gm::Envelope* envelope, double resX, double resY, int srid, int type)
{
  std::map<std::string, std::string> connInfo;
  connInfo["URI"] = fileName;

  te::rst::Grid* grid = new te::rst::Grid(resX, resY, envelope, srid);
  te::rst::BandProperty* bProp = new te::rst::BandProperty(0, type, "");
  bProp->m_noDataValue = 0.;

  std::vector<te::rst::BandProperty*> vecBandProp;
  vecBandProp.push_back(bProp);

  te::rst::Raster* raster = te::rst::RasterFactory::make("GDAL", grid, vecBandProp, connInfo);

  te::rst::FillRaster(raster, bProp->m_noDataValue);

  return raster;
}

te::rst::Raster* te::qt::plugins::fiocruz::ClipRaster(te::rst::Raster* inputRaster, te::gm::MultiPolygon* geom, const std::string& outputFileName)
{
  assert(inputRaster);
  assert(geom);

  int type = inputRaster->getBandDataType(0);
  int srid = inputRaster->getGrid()->getSRID();
  double resX = inputRaster->getGrid()->getResolutionX();
  double resY = inputRaster->getGrid()->getResolutionY();
  te::gm::Envelope* envelopeCopy = new te::gm::Envelope(*inputRaster->getGrid()->getExtent());

  //create the output raster
  te::rst::Raster* outputRaster = CreateRaster(outputFileName, envelopeCopy, resX, resY, srid, type);
  assert(outputRaster);

  te::gm::Polygon* polygon = 0;
  std::vector<double> doubleVec;
  te::gm::Coord2D inputCoord;
  te::gm::Coord2D outputCoord;

  for (std::size_t i = 0; i < geom->getNumGeometries(); ++i)
  {
    polygon = static_cast<te::gm::Polygon*> (geom->getGeometryN(i));

    te::rst::PolygonIterator<double> it = te::rst::PolygonIterator<double>::begin(inputRaster, polygon);
    te::rst::PolygonIterator<double> itend = te::rst::PolygonIterator<double>::end(inputRaster, polygon);

    while (it != itend)
    {
      inputRaster->getValues(it.getColumn(), it.getRow(), doubleVec);

      inputCoord = inputRaster->getGrid()->gridToGeo(it.getColumn(), it.getRow());
      outputCoord = outputRaster->getGrid()->geoToGrid(inputCoord.x, inputCoord.y);
      outputCoord.x = te::rst::Round(outputCoord.x);
      outputCoord.y = te::rst::Round(outputCoord.y);

      if (
        (
        (outputCoord.x >= 0)
        &&
        (outputCoord.x < (int)outputRaster->getNumberOfColumns())
        )
        &&
        (
        (outputCoord.y >= 0)
        &&
        (outputCoord.y < (int)outputRaster->getNumberOfRows())
        )
        )
      {
        outputRaster->setValues(outputCoord.x, outputCoord.y, doubleVec);
      }

      ++it;
    }
  }

  return outputRaster;
}

std::vector<te::map::AbstractLayerPtr> te::qt::plugins::fiocruz::CreateRasterIndividualMaps(std::vector<std::string> rastersPath, std::auto_ptr<te::color::ColorBar> cb, int slices, int prec)
{
  std::vector<te::map::AbstractLayerPtr> layers;

  for (std::size_t t = 0; t < rastersPath.size(); ++t)
  {
    //create data source
    boost::filesystem::path uri(rastersPath[t]);

    std::map<std::string, std::string> dsInfo;
    dsInfo["URI"] = uri.string();

    boost::uuids::basic_random_generator<boost::mt19937> gen;
    boost::uuids::uuid u = gen();
    std::string id_ds = boost::uuids::to_string(u);

    te::da::DataSourceInfoPtr dsInfoPtr(new te::da::DataSourceInfo);
    dsInfoPtr->setConnInfo(dsInfo);
    dsInfoPtr->setTitle(uri.stem().string());
    dsInfoPtr->setAccessDriver("GDAL");
    dsInfoPtr->setType("GDAL");
    dsInfoPtr->setDescription(uri.string());
    dsInfoPtr->setId(id_ds);

    te::da::DataSourceInfoManager::getInstance().add(dsInfoPtr);

    te::da::DataSourcePtr ds = te::da::DataSourceManager::getInstance().get(id_ds, "GDAL", dsInfoPtr->getConnInfo());

    //create layer
    te::qt::widgets::DataSet2Layer converter(ds->getId());

    te::da::DataSetTypePtr dt(ds->getDataSetType(ds->getDataSetNames()[0]).release());

    te::map::AbstractLayerPtr layer = converter(dt);

    //get raster info
    te::rst::Raster* raster = te::map::GetRaster((te::map::DataSetLayer*)layer.get());

    const te::rst::RasterSummary* rsMin = te::rst::RasterSummaryManager::getInstance().get(raster, te::rst::SUMMARY_MIN);
    const te::rst::RasterSummary* rsMax = te::rst::RasterSummaryManager::getInstance().get(raster, te::rst::SUMMARY_MAX);
    const std::complex<double>* cmin = rsMin->at(0).m_minVal;
    const std::complex<double>* cmax = rsMax->at(0).m_maxVal;
    double min = cmin->real();
    double max = cmax->real();

    delete raster;

    //create legend
    std::vector<te::map::GroupingItem*> legVec;

    std::vector<double> vec;
    vec.push_back(min);
    vec.push_back(max);

    //create grouping
    std::vector<te::color::RGBAColor> colorVec = cb->getSlices(slices + 1);

    te::map::GroupingByEqualSteps(vec.begin(), vec.end(), slices, legVec, prec);

    te::se::Interpolate* interpolate = new te::se::Interpolate();

    interpolate->setFallbackValue("#000000");
    interpolate->setLookupValue(new te::se::ParameterValue("Rasterdata"));
    interpolate->setMethodType(te::se::Interpolate::COLOR);

    for (std::size_t i = 0; i < colorVec.size(); ++i)
    {
      QColor color(colorVec[i].getRed(), colorVec[i].getGreen(), colorVec[i].getBlue(), colorVec[i].getAlpha());

      if (i == colorVec.size() - 1)
      {
        QString rangeStr = legVec[i - 1]->getUpperLimit().c_str();
        std::string colorStr = color.name().toLatin1().data();

        te::se::InterpolationPoint* ip = new te::se::InterpolationPoint();

        ip->setData(rangeStr.toDouble());
        ip->setValue(new te::se::ParameterValue(colorStr));

        interpolate->add(ip);
      }
      else
      {
        QString rangeStr = legVec[i]->getLowerLimit().c_str();
        std::string colorStr = color.name().toLatin1().data();

        te::se::InterpolationPoint* ip = new te::se::InterpolationPoint();

        ip->setData(rangeStr.toDouble());
        ip->setValue(new te::se::ParameterValue(colorStr));

        interpolate->add(ip);
      }
    }

    te::common::FreeContents(legVec);

    te::se::ColorMap* cm = new te::se::ColorMap();
    cm->setInterpolate(interpolate);

    te::se::RasterSymbolizer* rasterSymb = te::se::GetRasterSymbolizer(layer->getStyle());
    rasterSymb->setColorMap(cm);

    layers.push_back(layer);
  }

  return layers;
}

std::vector<te::map::AbstractLayerPtr> te::qt::plugins::fiocruz::CreateRasterDominanceMaps(std::vector<std::string> rastersPath, std::vector<std::pair<std::string, std::string> > legVec)
{
  std::vector<te::map::AbstractLayerPtr> layers;

  for (std::size_t t = 0; t < rastersPath.size(); ++t)
  {
    //create data source
    boost::filesystem::path uri(rastersPath[t]);

    std::map<std::string, std::string> dsInfo;
    dsInfo["URI"] = uri.string();

    boost::uuids::basic_random_generator<boost::mt19937> gen;
    boost::uuids::uuid u = gen();
    std::string id_ds = boost::uuids::to_string(u);

    te::da::DataSourceInfoPtr dsInfoPtr(new te::da::DataSourceInfo);
    dsInfoPtr->setConnInfo(dsInfo);
    dsInfoPtr->setTitle(uri.stem().string());
    dsInfoPtr->setAccessDriver("GDAL");
    dsInfoPtr->setType("GDAL");
    dsInfoPtr->setDescription(uri.string());
    dsInfoPtr->setId(id_ds);

    te::da::DataSourceInfoManager::getInstance().add(dsInfoPtr);

    te::da::DataSourcePtr ds = te::da::DataSourceManager::getInstance().get(id_ds, "GDAL", dsInfoPtr->getConnInfo());

    //create layer
    te::qt::widgets::DataSet2Layer converter(ds->getId());

    te::da::DataSetTypePtr dt(ds->getDataSetType(ds->getDataSetNames()[0]).release());

    te::map::AbstractLayerPtr layer = converter(dt);

    //create legend
    te::se::Recode* r = new te::se::Recode();

    r->setFallbackValue("#000000");
    r->setLookupValue(new te::se::ParameterValue("Rasterdata"));

    //set null value
    te::se::MapItem* m = new te::se::MapItem();
    m->setData(0.);
    m->setValue(new te::se::ParameterValue("#FFFFFF"));
    m->setTitle("No Value");
    r->add(m);

    for (std::size_t t = 0; t < legVec.size(); ++t)
    {
      te::se::MapItem* m = new te::se::MapItem();
      m->setData(t + 1);
      m->setValue(new te::se::ParameterValue(legVec[t].second));
      m->setTitle(legVec[t].first);
      r->add(m);
    }

    te::se::ColorMap* cm = new te::se::ColorMap();
    cm->setRecode(r);

    te::se::RasterSymbolizer* rasterSymb = te::se::GetRasterSymbolizer(layer->getStyle());
    rasterSymb->setColorMap(cm);

    layers.push_back(layer);
  }

  return layers;
}

std::vector<te::map::AbstractLayerPtr> te::qt::plugins::fiocruz::CreateVecDominanceMaps(std::string dsId, std::vector<te::qt::plugins::fiocruz::DominanceParams> dpVec, std::map<std::string, te::map::GroupingItem*> legMap)
{
  std::vector<te::map::AbstractLayerPtr> layers;

  for (std::size_t t = 0; t < dpVec.size(); ++t)
  {
    //create layer
    te::da::DataSourcePtr ds = te::da::GetDataSource(dsId);

    te::qt::widgets::DataSet2Layer converter(ds->getId());

    te::da::DataSetTypePtr dt(ds->getDataSetType(ds->getDataSetNames()[0]).release());

    te::map::AbstractLayerPtr layer = converter(dt);

    //create legend
    std::vector<te::map::GroupingItem*> legend;

    std::map<std::string, te::map::GroupingItem*>::iterator it;

    for (it = legMap.begin(); it != legMap.end(); ++it)
    {
      te::map::GroupingItem* gi = new te::map::GroupingItem(*it->second);

      legend.push_back(gi);
    }

    std::string groupingName = dpVec[t].m_propertyName;
    int attrType = te::dt::STRING_TYPE;
    int prec = 0;
    int geomType = te::map::GetGeomType(layer);

    //create grouping
    te::map::Grouping* group = new te::map::Grouping(groupingName, te::map::UNIQUE_VALUE);
    group->setPropertyType(attrType);
    group->setPrecision(prec);
    group->setStdDeviation(0.);
    group->setGroupingItems(legend);

    layer->setGrouping(group);

    layer->setTitle(layer->getTitle() + "_" + groupingName);

    layers.push_back(layer);
  }

  return layers;
}

std::vector<te::map::AbstractLayerPtr> te::qt::plugins::fiocruz::CreateVecIndividualMaps(std::string dsId, std::vector<std::string> propNames, std::auto_ptr<te::color::ColorBar> cb, int slices, int prec, int attrType)
{
  std::vector<te::map::AbstractLayerPtr> layers;

  //get data
  te::da::DataSourcePtr ds = te::da::GetDataSource(dsId);

  std::auto_ptr<te::da::DataSet> dataSet = ds->getDataSet(ds->getDataSetNames()[0]);

  //get info for each object
  for (std::size_t t = 0; t < propNames.size(); ++t)
  {
    int nullValues = 0;
    std::vector<int> values;

    dataSet->moveBeforeFirst();

    while (dataSet->moveNext())
    {
      if (dataSet->isNull(propNames[t]))
      {
        ++nullValues;
        continue;
      }

      values.push_back(dataSet->getInt32(propNames[t]));
    }

    std::vector<te::map::GroupingItem*> legend;

    te::map::GroupingByEqualSteps(values.begin(), values.end(), slices, legend, prec);

    std::vector<te::color::RGBAColor> colorVec = cb->getSlices(legend.size());

    //create layer
    te::qt::widgets::DataSet2Layer converter(ds->getId());

    te::da::DataSetTypePtr dt(ds->getDataSetType(ds->getDataSetNames()[0]).release());

    te::map::AbstractLayerPtr layer = converter(dt);

    //create symbolizer
    int geomType = te::map::GetGeomType(layer);

    for (size_t p = 0; p < colorVec.size(); ++p)
    {
      std::vector<te::se::Symbolizer*> symbVec;

      te::se::Symbolizer* s = te::se::CreateSymbolizer((te::gm::GeomType)geomType, colorVec[p].getColor());

      symbVec.push_back(s);

      legend[p]->setSymbolizers(symbVec);
    }

    //create null grouping item
    if (nullValues != 0)
    {
      te::map::GroupingItem* legendItem = new te::map::GroupingItem;
      legendItem->setLowerLimit(te::common::Globals::sm_nanStr);
      legendItem->setUpperLimit(te::common::Globals::sm_nanStr);
      legendItem->setTitle("No Value");
      legendItem->setCount(nullValues);

      std::vector<te::se::Symbolizer*> symbVec;
      te::se::Symbolizer* s = te::se::CreateSymbolizer((te::gm::GeomType)geomType, "#dddddd");
      symbVec.push_back(s);
      legendItem->setSymbolizers(symbVec);

      legend.push_back(legendItem);
    }

    //create grouping
    te::map::Grouping* group = new te::map::Grouping(propNames[t], te::map::EQUAL_STEPS);
    group->setPropertyType(attrType);
    group->setNumSlices(slices);
    group->setPrecision(prec);
    group->setStdDeviation(0.);
    group->setGroupingItems(legend);

    layer->setGrouping(group);

    layer->setTitle(layer->getTitle() + "_" + propNames[t]);

    layers.push_back(layer);
  }

  return layers;
}
