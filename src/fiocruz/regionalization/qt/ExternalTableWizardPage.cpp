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
\file fiocruz/src/fiocruz/regionalization/ExternalTableWizardPage.h

\brief This file defines the Regionalization External Table Wizard Page class
*/

#include "../Regionalization.h"
#include "ExternalTableWizardPage.h"
#include "ui_ExternalTableWizardPageForm.h"

// Terralib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/maptools/DataSetLayer.h>

// Qt
#include <QMessageBox>

Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::fiocruz::ExternalTableWizardPage::ExternalTableWizardPage(QWidget* parent)
  : QWizardPage(parent),
  m_ui(new Ui::ExternalTableWizardPageForm)
{
  //setup controls
  m_ui->setupUi(this);

  //configure page
  this->setTitle(tr("Regionalization Data"));
  this->setSubTitle(tr("Selects layers and properties that defines the regionalization objects."));

  //connects
  connect(m_ui->m_spatialLayerComboBox, SIGNAL(activated(int)), this, SLOT(onSpatialLayerComboBoxActivated(int)));
  connect(m_ui->m_tabularLayerComboBox, SIGNAL(activated(int)), this, SLOT(onTabularLayerComboBoxActivated(int)));
}

te::qt::plugins::fiocruz::ExternalTableWizardPage::~ExternalTableWizardPage()
{

}

bool te::qt::plugins::fiocruz::ExternalTableWizardPage::isComplete() const
{
  return true;
}

void te::qt::plugins::fiocruz::ExternalTableWizardPage::setList(std::list<te::map::AbstractLayerPtr>& layerList)
{
  m_ui->m_spatialLayerComboBox->clear();
  m_ui->m_tabularLayerComboBox->clear();

  //set layers into combo box
  std::list<te::map::AbstractLayerPtr>::iterator it = layerList.begin();

  while (it != layerList.end())
  {
    te::map::AbstractLayerPtr l = *it;

    std::auto_ptr<te::da::DataSetType> dsType = l->getSchema();

    if (dsType->hasGeom())
      m_ui->m_spatialLayerComboBox->addItem(l->getTitle().c_str(), QVariant::fromValue(l));
    else if (!dsType->hasGeom() && !dsType->hasRaster())
      m_ui->m_tabularLayerComboBox->addItem(l->getTitle().c_str(), QVariant::fromValue(l));

    ++it;
  }

  if (m_ui->m_spatialLayerComboBox->count() > 0)
    onSpatialLayerComboBoxActivated(0);

  if (m_ui->m_tabularLayerComboBox->count() > 0)
    onTabularLayerComboBoxActivated(0);
}

te::qt::plugins::fiocruz::VecStringPair te::qt::plugins::fiocruz::ExternalTableWizardPage::getUniqueObjects()
{
  te::qt::plugins::fiocruz::VecStringPair values;

  QVariant varLayer = m_ui->m_tabularLayerComboBox->currentData(Qt::UserRole);

  if (varLayer.isValid())
  {
    te::map::AbstractLayerPtr l = varLayer.value<te::map::AbstractLayerPtr>();

    std::auto_ptr<te::da::DataSetType> dsType = l->getSchema();

    std::string dataSetName = dsType->getName();
    std::string columnName = m_ui->m_tabularObjIdComboBox->currentText().toStdString();
    std::string aliasColumnName = m_ui->m_tabularObjNameComboBox->currentText().toStdString();

    te::da::DataSourcePtr ds = te::da::GetDataSource(l->getDataSourceId());

    te::qt::plugins::fiocruz::Regionalization reg;

    reg.getDistinctObjects(ds, dataSetName, columnName, aliasColumnName, values);
  }

  return values;
}

te::qt::plugins::fiocruz::RegionalizationInputParams* te::qt::plugins::fiocruz::ExternalTableWizardPage::getRegionalizationInputParameters()
{
  //get vectorial layer
  QVariant varVecLayer = m_ui->m_spatialLayerComboBox->currentData(Qt::UserRole);

  te::map::AbstractLayerPtr dsVecLayer = varVecLayer.value<te::map::AbstractLayerPtr>();

  if (!dsVecLayer)
    throw;

  std::auto_ptr<te::da::DataSetType> dsVecType = dsVecLayer->getSchema();

  //get tabular layer
  QVariant varTabLayer = m_ui->m_tabularLayerComboBox->currentData(Qt::UserRole);

  te::map::AbstractLayerPtr dsTabLayer = varTabLayer.value<te::map::AbstractLayerPtr>();

  if (!dsTabLayer)
    throw;

  std::auto_ptr<te::da::DataSetType> dsTabType = dsTabLayer->getSchema();

  //set parameters
  te::qt::plugins::fiocruz::RegionalizationInputParams* params = new te::qt::plugins::fiocruz::RegionalizationInputParams();

  //input vector data
  params->m_iVectorDataSource = te::da::GetDataSource(dsVecLayer->getDataSourceId());
  params->m_iVectorDataSet = dsVecLayer->getData();
  params->m_iVectorDataSetName = dsVecType->getTitle();

  params->m_iVectorColumnOriginId = m_ui->m_spatialPropertyComboBox->currentText().toStdString();

  //input tabular data
  params->m_iTabularDataSource = te::da::GetDataSource(dsTabLayer->getDataSourceId());
  params->m_iTabularDataSet = dsTabLayer->getData();
  params->m_iTabularDataSetName = dsTabType->getTitle();

  params->m_iTabularColumnOriginId = m_ui->m_tabularLinkPropComboBox->currentText().toStdString();
  params->m_iTabularColumnDestinyId = m_ui->m_tabularObjIdComboBox->currentText().toStdString();
  params->m_iTabularColumnDestinyAlias = m_ui->m_tabularObjNameComboBox->currentText().toStdString();

  //dominance params
  

  return params;
}

std::vector<std::string> te::qt::plugins::fiocruz::ExternalTableWizardPage::getTabularAttributes()
{
  QVariant varLayer = m_ui->m_tabularLayerComboBox->itemData(m_ui->m_tabularLayerComboBox->currentIndex(), Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  std::vector<std::string> values;

  //set properties from tabular layer into vector
  std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();

  for (std::size_t t = 0; t < dsType->getProperties().size(); ++t)
  {
    te::dt::Property* prop = dsType->getProperties()[t];

    if (prop->getType() == te::dt::INT32_TYPE || prop->getType() == te::dt::STRING_TYPE)
    {
      values.push_back(dsType->getProperties()[t]->getName());
    }
  }

  return values;
}

void te::qt::plugins::fiocruz::ExternalTableWizardPage::getExtentInfo(te::gm::Envelope& env, int& srid)
{
  //get vectorial layer
  QVariant varVecLayer = m_ui->m_spatialLayerComboBox->currentData(Qt::UserRole);

  te::map::AbstractLayerPtr dsVecLayer = varVecLayer.value<te::map::AbstractLayerPtr>();

  srid = dsVecLayer->getSRID();

  env = dsVecLayer->getExtent();
}

void te::qt::plugins::fiocruz::ExternalTableWizardPage::onSpatialLayerComboBoxActivated(int index)
{
  QVariant varLayer = m_ui->m_spatialLayerComboBox->itemData(index, Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  m_ui->m_spatialPropertyComboBox->clear();

  //set properties from spatial layer into referency property combo
  std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();

  for (std::size_t t = 0; t < dsType->getProperties().size(); ++t)
  {
    te::dt::Property* prop = dsType->getProperties()[t];

    if (prop->getType() == te::dt::INT32_TYPE || prop->getType() == te::dt::STRING_TYPE)
      m_ui->m_spatialPropertyComboBox->addItem(dsType->getProperties()[t]->getName().c_str());
  }
}

void te::qt::plugins::fiocruz::ExternalTableWizardPage::onTabularLayerComboBoxActivated(int index)
{
  QVariant varLayer = m_ui->m_tabularLayerComboBox->itemData(index, Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  m_ui->m_tabularLinkPropComboBox->clear();
  m_ui->m_tabularObjIdComboBox->clear();
  m_ui->m_tabularObjNameComboBox->clear();

  //set properties from tabular layer into combos
  std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();

  for (std::size_t t = 0; t < dsType->getProperties().size(); ++t)
  {
    te::dt::Property* prop = dsType->getProperties()[t];

    if (prop->getType() == te::dt::INT32_TYPE || prop->getType() == te::dt::STRING_TYPE)
    {
      m_ui->m_tabularLinkPropComboBox->addItem(dsType->getProperties()[t]->getName().c_str());
      m_ui->m_tabularObjIdComboBox->addItem(dsType->getProperties()[t]->getName().c_str());
      m_ui->m_tabularObjNameComboBox->addItem(dsType->getProperties()[t]->getName().c_str());
    }
  }
}
