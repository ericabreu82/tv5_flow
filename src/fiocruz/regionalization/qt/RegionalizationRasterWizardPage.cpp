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
\file fiocruz/src/fiocruz/regionalization/RegionalizationRasterWizardPage.cpp

\brief This file defines the Regionalization Raster Wizard Page class
*/

#include "RegionalizationRasterWizardPage.h"
#include "ui_RegionalizationRasterWizardPageForm.h"

// Terralib
#include <terralib/sa/Enums.h>

// Qt
#include <QMessageBox>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::fiocruz::RegionalizationRasterWizardPage::RegionalizationRasterWizardPage(QWidget* parent)
: QWizardPage(parent),
m_ui(new Ui::RegionalizationRasterWizardPageForm)
{
  //setup controls
  m_ui->setupUi(this);

  //configure page
  this->setTitle(tr("Regionalization Raster"));
  this->setSubTitle(tr("Reginalization Maps using raster representation."));

  //fill interface
  fillKernelParameters();

  m_srid = 0;
}

te::qt::plugins::fiocruz::RegionalizationRasterWizardPage::~RegionalizationRasterWizardPage()
{

}

bool te::qt::plugins::fiocruz::RegionalizationRasterWizardPage::isComplete() const
{
  return true;
}

void te::qt::plugins::fiocruz::RegionalizationRasterWizardPage::setAttrbutes(std::vector<std::string>& attrs)
{
  m_ui->m_xAttrComboBox->clear();
  m_ui->m_yAttrComboBox->clear();

  //set properties from tabular layer into combos
  for (std::size_t t = 0; t < attrs.size(); ++t)
  {
    m_ui->m_xAttrComboBox->addItem(attrs[t].c_str());
    m_ui->m_yAttrComboBox->addItem(attrs[t].c_str());
  }
}

bool te::qt::plugins::fiocruz::RegionalizationRasterWizardPage::hasSpatialInformation()
{
  return m_ui->m_spatialInfoGroupBox->isChecked();
}

void te::qt::plugins::fiocruz::RegionalizationRasterWizardPage::getSpatialAttributesNames(std::string& xAttr, std::string& yAttr)
{
  xAttr = m_ui->m_xAttrComboBox->currentText().toStdString();
  yAttr = m_ui->m_yAttrComboBox->currentText().toStdString();
}

void te::qt::plugins::fiocruz::RegionalizationRasterWizardPage::setExtent(te::gm::Envelope env, int srid)
{
  m_envelope = env;

  m_srid = srid;

  double val = std::max(env.getHeight(), env.getWidth());

  m_ui->m_radiusHorizontalSlider->setMinimum(0);
  m_ui->m_radiusHorizontalSlider->setMaximum(int(val));
}

void te::qt::plugins::fiocruz::RegionalizationRasterWizardPage::fillKernelParameters()
{
  //function
  m_ui->m_functionComboBox->clear();

  m_ui->m_functionComboBox->addItem("Quartic", QVariant(te::sa::Quartic));
  m_ui->m_functionComboBox->addItem("Normal", QVariant(te::sa::Normal));
  m_ui->m_functionComboBox->addItem("Triangular", QVariant(te::sa::Triangular));
  m_ui->m_functionComboBox->addItem("Uniform", QVariant(te::sa::Uniform));
  m_ui->m_functionComboBox->addItem("Negative Exponential", QVariant(te::sa::Negative_Exp));

  //estimation
  m_ui->m_estimationComboBox->clear();

  m_ui->m_estimationComboBox->addItem("Density", QVariant(te::sa::Density));
  m_ui->m_estimationComboBox->addItem("Spatial Moving Average", QVariant(te::sa::Spatial_Moving_Average));
  m_ui->m_estimationComboBox->addItem("Probability", QVariant(te::sa::Probability));
}
