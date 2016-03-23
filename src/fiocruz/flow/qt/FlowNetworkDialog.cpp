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
\file fiocruz/src/fiocruz/flow/qt/FlowNetworkDialog.cpp

\brief This file defines the Flow Network dialog class
*/

#include "../FlowGraphImport.h"
#include "../FlowDominance.h"
#include "FlowNetworkDialog.h"
#include "ui_FlowNetworkDialogForm.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/qt/widgets/datasource/selector/DataSourceSelectorDialog.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>

// Boost
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::fiocruz::FlowNetworkDialog::FlowNetworkDialog(QWidget* parent, Qt::WindowFlags f)
: QDialog(parent, f),
m_ui(new Ui::FlowNetworkDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_targetDatasourceToolButton->setIcon(QIcon::fromTheme("datasource"));

  //connects
  connect(m_ui->m_flowLayerComboBox, SIGNAL(activated(int)), this, SLOT(onFlowLayerComboBoxActivated(int)));
  connect(m_ui->m_domLayerComboBox, SIGNAL(activated(int)), this, SLOT(onDomLayerComboBoxActivated(int)));
  connect(m_ui->m_targetDatasourceToolButton, SIGNAL(pressed()), this, SLOT(onTargetDatasourceToolButtonPressed()));
  connect(m_ui->m_targetFileToolButton, SIGNAL(pressed()), this, SLOT(onTargetFileToolButtonPressed()));
  connect(m_ui->m_okPushButton, SIGNAL(released()), this, SLOT(onOkPushButtonClicked()));

}

te::qt::plugins::fiocruz::FlowNetworkDialog::~FlowNetworkDialog()
{

}

void te::qt::plugins::fiocruz::FlowNetworkDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  m_ui->m_flowLayerComboBox->clear();
  m_ui->m_domLayerComboBox->clear();
  
  //set layers into combo box
  std::list<te::map::AbstractLayerPtr>::iterator it = list.begin();

  while (it != list.end())
  {
    te::map::AbstractLayerPtr l = *it;

    std::auto_ptr<te::da::DataSetType> dsType = l->getSchema();

    if (!dsType->hasRaster())
    {
      m_ui->m_flowLayerComboBox->addItem(l->getTitle().c_str(), QVariant::fromValue(l));
      m_ui->m_domLayerComboBox->addItem(l->getTitle().c_str(), QVariant::fromValue(l));
    }

    ++it;
  }

  if (m_ui->m_flowLayerComboBox->count() > 0)
    onFlowLayerComboBoxActivated(0);

  if (m_ui->m_domLayerComboBox->count() > 0)
    onDomLayerComboBoxActivated(0);
}

te::map::AbstractLayerPtr te::qt::plugins::fiocruz::FlowNetworkDialog::getOutputLayer()
{
  return m_outputLayer;
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::onOkPushButtonClicked()
{
  //check input parameters
  if (m_ui->m_flowLayerComboBox->currentText().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Flow Layer not selected."));
    return;
  }

  //check input dominance parameters
  if (m_ui->m_domLayerRadioButton->isChecked())
  {
    if (m_ui->m_domLayerComboBox->currentText().isEmpty())
    {
      QMessageBox::warning(this, tr("Warning"), tr("Dominance Layer not selected."));
      return;
    }
  }
  else if (!m_ui->m_domCalcRadioButton->isChecked() && !m_ui->m_domLayerRadioButton->isChecked())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Dominance type not defined."));
    return;
  }

  //load graph
  QVariant flowVarLayer = m_ui->m_flowLayerComboBox->currentData(Qt::UserRole);
  te::map::AbstractLayerPtr flowLayer = flowVarLayer.value<te::map::AbstractLayerPtr>();
  std::auto_ptr<te::da::DataSet> flowDataSet = flowLayer->getData();

  int flowOriginColumnIdx = m_ui->m_flowOriginComboBox->currentData().toInt();
  int flowDestinyColumnIdx = m_ui->m_flowDestinyComboBox->currentData().toInt();
  int flowWeightColumnIdx = m_ui->m_flowWeightComboBox->currentData().toInt();
  int flowGeomColumnIdx = te::da::GetFirstSpatialPropertyPos(flowDataSet.get());

  te::qt::plugins::fiocruz::FlowGraphImport fgi;

  te::graph::AbstractGraph* graph = 0;

  try
  {
    graph = fgi.importGraph(flowDataSet, flowOriginColumnIdx, flowDestinyColumnIdx, flowWeightColumnIdx, flowGeomColumnIdx);
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal error loading graph from Layer."));
    return;
  }

  //calculate dominance
  te::qt::plugins::fiocruz::FlowDominance fd;

  if (m_ui->m_domLayerRadioButton->isChecked())
  {
    QVariant varLayer = m_ui->m_domLayerComboBox->currentData(Qt::UserRole);
    te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();
    std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();
    te::da::DataSourcePtr ds = te::da::GetDataSource(layer->getDataSourceId());
    std::string dataSetName = dsType->getName();

    int linkColumnIdx = m_ui->m_domPropertyIdxComboBox->currentData().toInt();
    int domColumnIdx = m_ui->m_domPropertyNameComboBox->currentData().toInt();

    fd.associate(graph, ds, dataSetName, linkColumnIdx, domColumnIdx);
  }
  else if (m_ui->m_domCalcRadioButton->isChecked())
  {
    te::qt::plugins::fiocruz::DominanceType dt;

    if (m_ui->m_calculateOutputRadioButton->isChecked())
      dt = te::qt::plugins::fiocruz::DOMINANCE_OUTPUTFLOW;
    else if (m_ui->m_calculateInputRadioButton->isChecked())
      dt = te::qt::plugins::fiocruz::DOMINANCE_INPUTFLOW;

    fd.calculate(graph, dt);
  }


  int a = 0;
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::onFlowLayerComboBoxActivated(int index)
{
  QVariant varLayer = m_ui->m_flowLayerComboBox->itemData(index, Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  m_ui->m_flowOriginComboBox->clear();
  m_ui->m_flowDestinyComboBox->clear();
  m_ui->m_flowWeightComboBox->clear();

  //set properties from spatial layer into referency property combo
  std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();

  for (std::size_t t = 0; t < dsType->getProperties().size(); ++t)
  {
    te::dt::Property* prop = dsType->getProperties()[t];

    if (prop->getType() == te::dt::INT32_TYPE || prop->getType() == te::dt::INT64_TYPE || prop->getType() == te::dt::STRING_TYPE)
    {
      m_ui->m_flowOriginComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
      m_ui->m_flowDestinyComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
      m_ui->m_flowWeightComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
    }
  }
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::onDomLayerComboBoxActivated(int index)
{
  QVariant varLayer = m_ui->m_domLayerComboBox->itemData(index, Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  m_ui->m_domPropertyIdxComboBox->clear();
  m_ui->m_domPropertyNameComboBox->clear();

  //set properties from spatial layer into referency property combo
  std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();

  for (std::size_t t = 0; t < dsType->getProperties().size(); ++t)
  {
    te::dt::Property* prop = dsType->getProperties()[t];

    if (prop->getType() == te::dt::INT32_TYPE || prop->getType() == te::dt::INT64_TYPE || prop->getType() == te::dt::STRING_TYPE)
    {
      m_ui->m_domPropertyIdxComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
      m_ui->m_domPropertyNameComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
    }
  }
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::onTargetDatasourceToolButtonPressed()
{
  m_ui->m_newLayerNameLineEdit->clear();
  m_ui->m_newLayerNameLineEdit->setEnabled(true);

  te::qt::widgets::DataSourceSelectorDialog dlg(this);
  dlg.exec();

  std::list<te::da::DataSourceInfoPtr> dsPtrList = dlg.getSelecteds();

  if (dsPtrList.size() <= 0)
    return;

  std::list<te::da::DataSourceInfoPtr>::iterator it = dsPtrList.begin();

  m_ui->m_repositoryLineEdit->setText(QString(it->get()->getTitle().c_str()));

  m_outputDatasource = *it;
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::onTargetFileToolButtonPressed()
{
  m_ui->m_newLayerNameLineEdit->clear();
  m_ui->m_repositoryLineEdit->clear();

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(), tr("Shapefile (*.shp *.SHP);;"), 0, QFileDialog::DontConfirmOverwrite);

  if (fileName.isEmpty())
    return;

  boost::filesystem::path outfile(fileName.toStdString());

  m_ui->m_repositoryLineEdit->setText(outfile.string().c_str());

  m_ui->m_newLayerNameLineEdit->setText(outfile.leaf().string().c_str());

  m_ui->m_newLayerNameLineEdit->setEnabled(false);

  //create new data source
  boost::filesystem::path uri(m_ui->m_repositoryLineEdit->text().toStdString());

  std::map<std::string, std::string> dsInfo;
  dsInfo["URI"] = uri.string();

  boost::uuids::basic_random_generator<boost::mt19937> gen;
  boost::uuids::uuid u = gen();
  std::string id_ds = boost::uuids::to_string(u);

  te::da::DataSourceInfoPtr dsInfoPtr(new te::da::DataSourceInfo);
  dsInfoPtr->setConnInfo(dsInfo);
  dsInfoPtr->setTitle(uri.stem().string());
  dsInfoPtr->setAccessDriver("OGR");
  dsInfoPtr->setType("OGR");
  dsInfoPtr->setDescription(uri.string());
  dsInfoPtr->setId(id_ds);

  te::da::DataSourceInfoManager::getInstance().add(dsInfoPtr);

  m_outputDatasource = dsInfoPtr;
}

