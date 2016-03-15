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
\file fiocruz/src/fiocruz/flow/qt/FlowDiagramDialog.cpp

\brief This file defines the Flow Diagram dialog class
*/

// TerraLib
#include <terralib/common/STLUtils.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/qt/widgets/datasource/selector/DataSourceSelectorDialog.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/graph/Globals.h>

#include "../FlowGraphDiagramBuilder.h"
#include "../FlowGraphExport.h"
#include "FlowDiagramDialog.h"
#include "ui_FlowDiagramDialogForm.h"
#include "FlowNetworkRenderer.h"

// Qt
#include <QMessageBox>
#include <QFileDialog>

// Boost
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


Q_DECLARE_METATYPE(te::map::AbstractLayerPtr);

te::qt::plugins::fiocruz::FlowDiagramDialog::FlowDiagramDialog(QWidget* parent, Qt::WindowFlags f)
: QDialog(parent, f),
m_ui(new Ui::FlowDiagramDialogForm)
{
  // add controls
  m_ui->setupUi(this);

  m_ui->m_targetDatasourceToolButton->setIcon(QIcon::fromTheme("datasource"));

  //connects
  connect(m_ui->m_spatialLayerComboBox, SIGNAL(activated(int)), this, SLOT(onSpatialLayerComboBoxActivated(int)));
  connect(m_ui->m_tabularLayerComboBox, SIGNAL(activated(int)), this, SLOT(onTabularLayerComboBoxActivated(int)));
  connect(m_ui->m_targetDatasourceToolButton, SIGNAL(pressed()), this, SLOT(onTargetDatasourceToolButtonPressed()));
  connect(m_ui->m_targetFileToolButton, SIGNAL(pressed()), this, SLOT(onTargetFileToolButtonPressed()));
  connect(m_ui->m_okPushButton, SIGNAL(released()), this, SLOT(onOkPushButtonClicked()));
}

te::qt::plugins::fiocruz::FlowDiagramDialog::~FlowDiagramDialog()
{

}

void te::qt::plugins::fiocruz::FlowDiagramDialog::setLayerList(std::list<te::map::AbstractLayerPtr> list)
{
  m_ui->m_spatialLayerComboBox->clear();
  m_ui->m_tabularLayerComboBox->clear();

  //set layers into combo box
  std::list<te::map::AbstractLayerPtr>::iterator it = list.begin();

  while (it != list.end())
  {
    te::map::AbstractLayerPtr l = *it;

    std::auto_ptr<te::da::DataSetType> dsType = l->getSchema();

    if (dsType->hasGeom())
      m_ui->m_spatialLayerComboBox->addItem(l->getTitle().c_str(), QVariant::fromValue(l));

    if (!dsType->hasRaster())
      m_ui->m_tabularLayerComboBox->addItem(l->getTitle().c_str(), QVariant::fromValue(l));

    ++it;
  }

  if (m_ui->m_spatialLayerComboBox->count() > 0)
    onSpatialLayerComboBoxActivated(0);

  if (m_ui->m_tabularLayerComboBox->count() > 0)
    onTabularLayerComboBoxActivated(0);
}

te::map::AbstractLayerPtr te::qt::plugins::fiocruz::FlowDiagramDialog::getOutputLayer()
{
  return m_outputLayer;
}

void te::qt::plugins::fiocruz::FlowDiagramDialog::onOkPushButtonClicked()
{
  //get input spatial info
  QVariant spatialVarLayer = m_ui->m_spatialLayerComboBox->currentData(Qt::UserRole);
  te::map::AbstractLayerPtr spatialLayer = spatialVarLayer.value<te::map::AbstractLayerPtr>();
  std::auto_ptr<te::da::DataSetType> spatialDsType = spatialLayer->getSchema();
  te::da::DataSourcePtr spatialDs = te::da::GetDataSource(spatialLayer->getDataSourceId());
  std::string spatialDataSetName = spatialDsType->getName();

  int linkColumnIdx = m_ui->m_spatialPropertyComboBox->currentData().toInt();
  int srid = spatialLayer->getSRID();

  //get get input tabular info
  QVariant tabularVarLayer = m_ui->m_tabularLayerComboBox->currentData(Qt::UserRole);
  te::map::AbstractLayerPtr tabularLayer = tabularVarLayer.value<te::map::AbstractLayerPtr>();
  std::auto_ptr<te::da::DataSetType> tabularDsType = tabularLayer->getSchema();
  te::da::DataSourcePtr tabularDs = te::da::GetDataSource(tabularLayer->getDataSourceId());
  std::string tabularDataSetName = tabularDsType->getName();

  int fromIdx = m_ui->m_tabularOriginComboBox->currentData().toInt();
  int toIdx = m_ui->m_tabularDestinyComboBox->currentData().toInt();;
  int weightIdx = m_ui->m_tabularWeightComboBox->currentData().toInt();;

  
  //output info
  std::string dataSetName = m_ui->m_newLayerNameLineEdit->text().toStdString();
  std::size_t idx = dataSetName.find(".");
  if (idx != std::string::npos)
    dataSetName = dataSetName.substr(0, idx);

  std::string graphName = dataSetName;

  // data source information
  std::map<std::string, std::string> connInfo;
  //connInfo["PG_HOST"] = "localhost";
  //connInfo["PG_USER"] = "postgres";
  //connInfo["PG_PASSWORD"] = "tdk696";
  //connInfo["PG_DB_NAME"] = "t5graph";
  //connInfo["PG_CONNECT_TIMEOUT"] = "4";
  //connInfo["PG_CLIENT_ENCODING"] = "LATIN1";

  // graph type
  std::string graphType = te::graph::Globals::sm_graphFactoryDefaultObject;

  // graph information
  std::map<std::string, std::string> graphInfo;
  //graphInfo["GRAPH_DATA_SOURCE_TYPE"] = "POSTGIS";
  //graphInfo["GRAPH_NAME"] = graphName;
  //graphInfo["GRAPH_DESCRIPTION"] = "Generated by Flow Builder.";
  //graphInfo["GRAPH_STORAGE_MODE"] = te::graph::Globals::sm_edgeStorageMode;
  //graphInfo["GRAPH_STRATEGY_LOADER"] = te::graph::Globals::sm_factoryLoaderStrategyTypeSequence;
  //graphInfo["GRAPH_CACHE_POLICY"] = "FIFO";
  graphInfo["GRAPH_DATA_SOURCE_TYPE"] = "MEM";
  graphInfo["GRAPH_NAME"] = graphName;
  graphInfo["GRAPH_DESCRIPTION"] = "Generated by Flow Builder.";

  boost::shared_ptr<te::graph::AbstractGraph> graph;

  try
  {
    te::qt::plugins::fiocruz::FlowGraphDiagramBuilder builder;

    if (!builder.build(spatialDs, spatialDataSetName, linkColumnIdx, srid, tabularDs, tabularDataSetName, fromIdx, toIdx, weightIdx, connInfo, graphType, graphInfo))
    {
      QMessageBox::warning(this, tr("Warning"), builder.getErrorMessage().c_str());
    }

    graph = builder.getGraph();
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error"));
  }

  //export
  te::da::DataSourcePtr outputDataSource = te::da::DataSourceManager::getInstance().get(m_outputDatasource->getId(), m_outputDatasource->getType(), m_outputDatasource->getConnInfo());

  te::qt::plugins::fiocruz::FlowGraphExport fge;

  try
  {
    fge.exportGraph(outputDataSource, dataSetName, graph.get());
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error"));
  }

  outputDataSource->close();

  graph->flush();

  //create layer
  te::da::DataSourcePtr ds = te::da::GetDataSource(m_outputDatasource->getId());

  te::qt::widgets::DataSet2Layer converter(m_outputDatasource->getId());

  te::da::DataSetTypePtr dt(ds->getDataSetType(ds->getDataSetNames()[0]).release());

  m_outputLayer = converter(dt);

  FlowNetworkRenderer* renderer = new FlowNetworkRenderer();

  accept();
}

void te::qt::plugins::fiocruz::FlowDiagramDialog::onSpatialLayerComboBoxActivated(int index)
{
  QVariant varLayer = m_ui->m_spatialLayerComboBox->itemData(index, Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  m_ui->m_spatialPropertyComboBox->clear();

  //set properties from spatial layer into referency property combo
  std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();

  for (std::size_t t = 0; t < dsType->getProperties().size(); ++t)
  {
    te::dt::Property* prop = dsType->getProperties()[t];

    if (prop->getType() == te::dt::INT32_TYPE || prop->getType() == te::dt::INT64_TYPE || prop->getType() == te::dt::STRING_TYPE)
      m_ui->m_spatialPropertyComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
  }
}

void te::qt::plugins::fiocruz::FlowDiagramDialog::onTabularLayerComboBoxActivated(int index)
{
  QVariant varLayer = m_ui->m_tabularLayerComboBox->itemData(index, Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  m_ui->m_tabularOriginComboBox->clear();
  m_ui->m_tabularDestinyComboBox->clear();
  m_ui->m_tabularWeightComboBox->clear();

  //set properties from tabular layer into combos
  std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();

  for (std::size_t t = 0; t < dsType->getProperties().size(); ++t)
  {
    te::dt::Property* prop = dsType->getProperties()[t];

    if (prop->getType() == te::dt::INT32_TYPE || prop->getType() == te::dt::STRING_TYPE)
    {
      m_ui->m_tabularOriginComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
      m_ui->m_tabularDestinyComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
      m_ui->m_tabularWeightComboBox->addItem(dsType->getProperties()[t]->getName().c_str(), QVariant(t));
    }
  }
}

void te::qt::plugins::fiocruz::FlowDiagramDialog::onTargetDatasourceToolButtonPressed()
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

void te::qt::plugins::fiocruz::FlowDiagramDialog::onTargetFileToolButtonPressed()
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
