/*  Copyright (C) 2008 National Institute For Space Research (INPE) - Brazil.

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
\file fiocruz/src/fiocruz/regionalization/RegionalizationWizard.cpp

\brief This file defines the Regionalization Wizard class
*/

// TerraLib
#include <terralib/common/Globals.h>
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/maptools/Grouping.h>
#include <terralib/maptools/GroupingAlgorithms.h>
#include <terralib/maptools/GroupingItem.h>
#include <terralib/maptools/Utils.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/se/Utils.h>

// Plugin
#include "../Regionalization.h"
#include "RegionalizationWizard.h"

#include "ExternalTableWizardPage.h"
#include "LegendWizardPage.h"
#include "MapWizardPage.h"
#include "RegionalizationRasterWizardPage.h"
#include "RegionalizationVectorWizardPage.h"

// Qt Includes
#include <QMessageBox>


te::qt::plugins::fiocruz::RegionalizationWizard::RegionalizationWizard(QWidget* parent, const RegionalizationType& type) : 
  QWizard(parent),
  m_regionalizationType(type)
{
  //configure the wizard
  this->setWizardStyle(QWizard::ModernStyle);
  this->setWindowTitle(tr("Reginalization Maps."));
  this->setFixedSize(QSize(740, 580));

  addPages();
}

te::qt::plugins::fiocruz::RegionalizationWizard::~RegionalizationWizard()
{

}

bool te::qt::plugins::fiocruz::RegionalizationWizard::validateCurrentPage()
{
  if (currentPage() == m_externalTablePage.get())
  {
    bool res = m_externalTablePage->isComplete();

    std::vector<std::string> attrs = m_externalTablePage->getTabularAttributes();

    te::gm::Envelope env;
    int srid = 0;

    m_externalTablePage->getExtentInfo(env, srid);

    if (m_regionalizationType == te::qt::plugins::fiocruz::Raster_Regionalization)
    {
      m_regionalizationRasterPage->setAttrbutes(attrs);

      m_regionalizationRasterPage->setExtent(env, srid);
    }

    te::qt::plugins::fiocruz::VecStringPair objects = m_externalTablePage->getUniqueObjects();

    if (res && !objects.empty())
    {
      m_legendPage->setList(objects);

      return true;
    }
  }
  else if (currentPage() == m_legendPage.get())
  {
    return m_legendPage->isComplete();
  }
  else if (currentPage() == m_mapPage.get())
  {
    return m_mapPage->isComplete();
  }
  else if (m_regionalizationType == te::qt::plugins::fiocruz::Vector_Regionalization && currentPage() == m_regionalizationVectorPage.get())
  {
    if (!m_regionalizationVectorPage->isComplete())
      return false;

    return executeVectorRegionalization();
  }
  else if (m_regionalizationType == te::qt::plugins::fiocruz::Raster_Regionalization && currentPage() == m_regionalizationRasterPage.get())
  {
    return m_regionalizationRasterPage->isComplete();
  }

  return false;
}

void te::qt::plugins::fiocruz::RegionalizationWizard::setList(std::list<te::map::AbstractLayerPtr>& layerList)
{
  m_externalTablePage->setList(layerList);
}

std::vector<te::map::AbstractLayerPtr>& te::qt::plugins::fiocruz::RegionalizationWizard::getOutputLayers()
{
  return m_outputLayers;
}

void te::qt::plugins::fiocruz::RegionalizationWizard::addPages()
{
  m_externalTablePage.reset(new te::qt::plugins::fiocruz::ExternalTableWizardPage(this));
  m_legendPage.reset(new te::qt::plugins::fiocruz::LegendWizardPage(this));
  m_mapPage.reset(new te::qt::plugins::fiocruz::MapWizardPage(this));

  addPage(m_externalTablePage.get());
  addPage(m_legendPage.get());
  addPage(m_mapPage.get());
  
  if (m_regionalizationType == te::qt::plugins::fiocruz::Vector_Regionalization)
  {
    m_regionalizationVectorPage.reset(new te::qt::plugins::fiocruz::RegionalizationVectorWizardPage(this));
    addPage(m_regionalizationVectorPage.get());
  }
  else if (m_regionalizationType == te::qt::plugins::fiocruz::Raster_Regionalization)
  {
    m_regionalizationRasterPage.reset(new te::qt::plugins::fiocruz::RegionalizationRasterWizardPage(this));
    addPage(m_regionalizationRasterPage.get());
  }
}

bool te::qt::plugins::fiocruz::RegionalizationWizard::executeVectorRegionalization()
{
  te::qt::plugins::fiocruz::Regionalization reg;

  RegionalizationInputParams* inParams = m_externalTablePage->getRegionalizationInputParameters();

  inParams->m_vecDominance = m_mapPage->getDominances();
  inParams->m_objects = m_legendPage->getObjects();

  RegionalizationOutputParams* outParams = m_regionalizationVectorPage->getRegionalizationOutputParameters();

  reg.setInputParameters(inParams);
  reg.setOutputParameters(outParams);

  bool res = false;

  try
  {
    res = reg.generate();
  }
  catch (const te::common::Exception& e)
  {
    QMessageBox::warning(this, tr("Regionalization"), e.what());
    return false;
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Regionalization"), e.what());
    return false;
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Regionalization"), tr("Internal Error."));
    return false;
  }

  //create layer
  if (res)
  {
    //create layer for each dominance and create legend for dominance
    std::vector<te::qt::plugins::fiocruz::DominanceParams> dpVec = m_mapPage->getDominances();

    std::map<std::string, te::map::GroupingItem*> legMap = m_legendPage->getLegendMap();

    for (std::size_t t = 0; t < dpVec.size(); ++t)
    {
      //create layer
      te::da::DataSourcePtr ds = te::da::GetDataSource(outParams->m_oDataSource->getId());

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

      //add layer to output layers
      m_outputLayers.push_back(layer);
    }

    te::common::FreeContents(legMap);

    //create layers for each object calculated
    if (m_mapPage->createIndividualMaps())
    {
      int attrType = te::dt::INT32_TYPE;
      int prec = 1;
      int slices = m_mapPage->getNSlices();
      
      std::auto_ptr<te::color::ColorBar> cb = m_mapPage->getColorBar();

      //get data
      te::da::DataSourcePtr ds = te::da::GetDataSource(outParams->m_oDataSource->getId());

      std::auto_ptr<te::da::DataSet> dataSet = ds->getDataSet(ds->getDataSetNames()[0]);

      //get info for each object
      for (std::size_t t = 0; t < outParams->m_propNames.size(); ++t)
      {
        int nullValues = 0;
        std::vector<int> values;

        dataSet->moveBeforeFirst();

        while (dataSet->moveNext())
        {
          if (dataSet->isNull(outParams->m_propNames[t]))
          {
            ++nullValues;
            continue;
          }

          values.push_back(dataSet->getInt32(outParams->m_propNames[t]));
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

        for (size_t t = 0; t < colorVec.size(); ++t)
        {
          std::vector<te::se::Symbolizer*> symbVec;

          te::se::Symbolizer* s = te::se::CreateSymbolizer((te::gm::GeomType)geomType, colorVec[t].getColor());

          symbVec.push_back(s);

          legend[t]->setSymbolizers(symbVec);
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
        te::map::Grouping* group = new te::map::Grouping(outParams->m_propNames[t], te::map::EQUAL_STEPS);
        group->setPropertyType(attrType);
        group->setNumSlices(slices);
        group->setPrecision(prec);
        group->setStdDeviation(0.);
        group->setGroupingItems(legend);

        layer->setGrouping(group);

        layer->setTitle(layer->getTitle() + "_" + outParams->m_propNames[t]);

        //add layer to output layers
        m_outputLayers.push_back(layer);
      }
    }
  }

  return res;
}
