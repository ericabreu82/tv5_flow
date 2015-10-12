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
\file fiocruz/src/fiocruz/regionalization/RegionalizationVectorWizard.cpp

\brief This file defines the Regionalization Vector Wizard class
*/

#include "RegionalizationVectorWizard.h"

#include "ExternalTableWizardPage.h"
#include "LegendWizardPage.h"
#include "MapWizardPage.h"
#include "RegionalizationVectorWizardPage.h"


te::qt::plugins::fiocruz::RegionalizationVectorWizard::RegionalizationVectorWizard(QWidget* parent) : QWizard(parent)
{
  //configure the wizard
  this->setWizardStyle(QWizard::ModernStyle);
  this->setWindowTitle(tr("Reginalization Maps using vectorial representation."));
  this->setFixedSize(QSize(740, 580));

  addPages();
}

te::qt::plugins::fiocruz::RegionalizationVectorWizard::~RegionalizationVectorWizard()
{

}

bool te::qt::plugins::fiocruz::RegionalizationVectorWizard::validateCurrentPage()
{
  if (currentPage() == m_externalTablePage.get())
  {
    bool res = m_externalTablePage->isComplete();

    std::vector<std::string> objects = m_externalTablePage->getUniqueObjects();

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
  else if (currentPage() == m_regionalizationPage.get())
  {
    return m_regionalizationPage->isComplete();
  }

  return false;
}

void te::qt::plugins::fiocruz::RegionalizationVectorWizard::setList(std::list<te::map::AbstractLayerPtr>& layerList)
{
  m_externalTablePage->setList(layerList);
}

void te::qt::plugins::fiocruz::RegionalizationVectorWizard::addPages()
{
  m_externalTablePage.reset(new te::qt::plugins::fiocruz::ExternalTableWizardPage(this));
  m_legendPage.reset(new te::qt::plugins::fiocruz::LegendWizardPage(this));
  m_regionalizationPage.reset(new te::qt::plugins::fiocruz::RegionalizationVectorWizardPage(this));
  m_mapPage.reset(new te::qt::plugins::fiocruz::MapWizardPage(this));

  addPage(m_externalTablePage.get());
  addPage(m_legendPage.get());
  addPage(m_mapPage.get());
  addPage(m_regionalizationPage.get());
}
