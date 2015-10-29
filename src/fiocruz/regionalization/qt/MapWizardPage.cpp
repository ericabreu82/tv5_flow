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
\file fiocruz/src/fiocruz/regionalization/MapWizardPage.h

\brief This file defines the Regionalization Map Wizard Page class
*/

#include "MapWizardPage.h"
#include "ui_MapWizardPageForm.h"

#include "../../regionalization/Regionalization.h"

// TerraLib
#include <terralib/qt/widgets/colorbar/ColorBar.h>
#include <terralib/qt/widgets/colorbar/ColorCatalogWidget.h>

// Qt
#include <QMessageBox>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::fiocruz::MapWizardPage::MapWizardPage(QWidget* parent)
: QWizardPage(parent),
m_ui(new Ui::MapWizardPageForm)
{
  //setup controls
  m_ui->setupUi(this);

  //add color catalog widget
  QGridLayout* l = new QGridLayout(m_ui->m_widget);
  l->setContentsMargins(0, 0, 0, 0);
  m_colorBar = new  te::qt::widgets::ColorCatalogWidget(m_ui->m_widget);
  l->addWidget(m_colorBar);

  m_colorBar->getColorBar()->setHeight(20);
  m_colorBar->getColorBar()->setScaleVisible(false);

  //configure page
  this->setTitle(tr("Map"));
  this->setSubTitle(tr("Define the maps properties to be used in regionalization."));
}

te::qt::plugins::fiocruz::MapWizardPage::~MapWizardPage()
{
}

bool te::qt::plugins::fiocruz::MapWizardPage::isComplete() const
{
  return true;
}

std::vector<te::qt::plugins::fiocruz::DominanceParams> te::qt::plugins::fiocruz::MapWizardPage::getDominances()
{
  std::vector<DominanceParams> dominances;

  if (m_ui->m_primaryGroupBox->isChecked())
  {
    DominanceParams dp;

    dp.m_minLevel = m_ui->m_minPrimarySpinBox->value();
    dp.m_maxLevel = m_ui->m_maxPrimarySpinBox->value();
    dp.m_propertyName = "Primary";
    dominances.push_back(dp);
  }

  if (m_ui->m_secondaryGroupBox->isChecked())
  {
    DominanceParams dp;

    dp.m_minLevel = m_ui->m_minSecondarySpinBox->value();
    dp.m_maxLevel = m_ui->m_maxSecondarySpinBox->value();
    dp.m_propertyName = "Secondary";
    dominances.push_back(dp);
  }

  if (m_ui->m_tertiaryGroupBox->isChecked())
  {
    DominanceParams dp;

    dp.m_minLevel = m_ui->m_minTertiarySpinBox->value();
    dp.m_maxLevel = m_ui->m_maxTertiarySpinBox->value();
    dp.m_propertyName = "Tertiary";
    dominances.push_back(dp);
  }

  return dominances;
}
