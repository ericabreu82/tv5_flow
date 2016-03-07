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
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/maptools/GroupingItem.h>

// Plugin
#include "../Utils.h"
#include "../RasterRegionalization.h"
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
  this->setFixedSize(QSize(740, 600));

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
    if (!m_regionalizationRasterPage->isComplete())
      return false;

    return executeRasterRegionalization();
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
  RegionalizationInputParams* inParams = m_externalTablePage->getRegionalizationInputParameters();

  inParams->m_vecDominance = m_mapPage->getDominances();
  inParams->m_objects = m_legendPage->getObjects();

  RegionalizationOutputParams* outParams = m_regionalizationVectorPage->getRegionalizationOutputParameters();

  te::qt::plugins::fiocruz::Regionalization reg;
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

    std::string dsId = outParams->m_oDataSource->getId();

    std::vector<te::map::AbstractLayerPtr> layers = te::qt::plugins::fiocruz::CreateVecDominanceMaps(dsId, dpVec, legMap);

    for (std::size_t t = 0; t < layers.size(); ++t)
    {
      m_outputLayers.push_back(layers[t]);
    }

    te::common::FreeContents(legMap);

    //create layers for each object calculated
    if (m_mapPage->createIndividualMaps())
    {
      int slices = m_mapPage->getNSlices();

      std::auto_ptr<te::color::ColorBar> cb = m_mapPage->getColorBar();

      std::vector<te::map::AbstractLayerPtr> layers = te::qt::plugins::fiocruz::CreateVecIndividualMaps(outParams->m_oDataSource->getId(), outParams->m_propNames, cb, slices);

      //add layer to output layers
      for (std::size_t t = 0; t < layers.size(); ++t)
      {
        m_outputLayers.push_back(layers[t]);
      }
    }
  }

  return res;
}


bool te::qt::plugins::fiocruz::RegionalizationWizard::executeRasterRegionalization()
{
  //we first get the parameters from the interface
  RegionalizationInputParams* inParams = m_externalTablePage->getRegionalizationInputParameters();
  inParams->m_vecDominance = m_mapPage->getDominances();
  inParams->m_objects = m_legendPage->getObjects();

  //raster input parameters
  inParams->m_hasSpatialInformation = m_regionalizationRasterPage->hasSpatialInformation();
  inParams->m_kernelFunction = m_regionalizationRasterPage->getKernelFunctionType();
  inParams->m_algorithm = m_regionalizationRasterPage->getKernelInterpolationAlgorithm();
  inParams->m_numberOfNeighbours = m_regionalizationRasterPage->getNumberOfNeighbours();
  inParams->m_boxRatio = m_regionalizationRasterPage->getRadius();
  m_regionalizationRasterPage->getSpatialAttributesNames(inParams->m_xAttrName, inParams->m_yAttrName);
  m_regionalizationRasterPage->getResolution(inParams->m_resX, inParams->m_resY);

  std::string path = m_regionalizationRasterPage->getPath();
  std::string baseName = m_regionalizationRasterPage->getBaseName();

  RasterRegionalizationOutputParams* outParams = new RasterRegionalizationOutputParams();
  outParams->m_path = path;
  outParams->m_baseName = baseName;

  te::qt::plugins::fiocruz::RasterRegionalization reg;
  reg.setInputParameters(inParams);
  reg.setOutputParameters(outParams);

  bool res = false;
  std::vector<std::string> rastersPath;
  std::vector<te::rst::Raster*> rasters;

  try
  {
    res = reg.generate(rastersPath, rasters);
  }
  catch (const te::common::Exception& e)
  {
    QMessageBox::warning(this, tr("Raster Regionalization"), e.what());
    return false;
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Raster Regionalization"), e.what());
    return false;
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Raster Regionalization"), tr("Internal Error."));
    return false;
  }


  //create dominances
  std::vector<te::qt::plugins::fiocruz::DominanceParams> dpVec = m_mapPage->getDominances();

  std::vector<std::string> domMapsPaths = te::qt::plugins::fiocruz::CreateDominancesMaps(path, baseName, rasters, dpVec);

  if (!domMapsPaths.empty())
  {
    //create layer and legend for each dominance map
    std::vector<std::pair<std::string, std::string> > legVec = m_legendPage->getLegendVector();
    std::map<std::string, te::map::GroupingItem*> legMap = m_legendPage->getLegendMap();

    std::vector<te::map::AbstractLayerPtr> layers = te::qt::plugins::fiocruz::CreateRasterDominanceMaps(domMapsPaths, legVec);

    for (std::size_t t = 0; t < layers.size(); ++t)
    {
      m_outputLayers.push_back(layers[t]);
    }
  }


  //create individual layers
  if (m_mapPage->createIndividualMaps())
  {
    //create regionalization maps
    std::vector<std::string> regMaps = te::qt::plugins::fiocruz::CreateIndividualRegionalization(path, baseName, inParams->m_objects, rasters);

    te::common::FreeContents(rasters);

    //create layers and legend
    std::auto_ptr<te::color::ColorBar> cb = m_mapPage->getColorBar();

    std::vector<te::map::AbstractLayerPtr> layers = te::qt::plugins::fiocruz::CreateRasterIndividualMaps(rastersPath, cb);

    for (std::size_t t = 0; t < layers.size(); ++t)
    {
      m_outputLayers.push_back(layers[t]);
    }

    cb = m_mapPage->getColorBar();

    std::vector<te::map::AbstractLayerPtr> regLayers = te::qt::plugins::fiocruz::CreateRasterIndividualMaps(regMaps, cb);

    for (std::size_t t = 0; t < regLayers.size(); ++t)
    {
      m_outputLayers.push_back(regLayers[t]);
    }
  }
  else
  {
    te::common::FreeContents(rasters);
  }

  return true;
}
