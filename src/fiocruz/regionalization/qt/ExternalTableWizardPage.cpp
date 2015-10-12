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

std::vector<std::string> te::qt::plugins::fiocruz::ExternalTableWizardPage::getUniqueObjects()
{
  std::vector<std::string> values;

  QVariant varLayer = m_ui->m_tabularLayerComboBox->currentData(Qt::UserRole);

  if (varLayer.isValid())
  {
    te::map::AbstractLayerPtr l = varLayer.value<te::map::AbstractLayerPtr>();

    te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(l.get());

    if (dsLayer)
    {
      std::auto_ptr<te::da::DataSetType> dsType = dsLayer->getSchema();

      std::string dataSetName = dsType->getName();
      std::string columnName = m_ui->m_tabularObjIdComboBox->currentText().toStdString();

      te::da::DataSourcePtr ds = te::da::GetDataSource(dsLayer->getDataSourceId());

      te::qt::plugins::fiocruz::Regionalization reg;

      reg.getDistinctObjects(ds, dataSetName, columnName, values);
    }
  }

  return values;
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
