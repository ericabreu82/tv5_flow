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
#include <terralib/qt/widgets/datasource/selector/DataSourceSelectorDialog.h>
#include <terralib/graph/Globals.h>

#include "../FlowGraphDiagramBuilder.h"
#include "../FlowGraphExport.h"
#include "FlowDiagramDialog.h"
#include "ui_FlowDiagramDialogForm.h"

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

void te::qt::plugins::fiocruz::FlowDiagramDialog::onOkPushButtonClicked()
{
  std::string shapeFileName = "D:/Workspace/FIOCRUZ/data/flow/Censo_2000_Municipios.shp";
  std::string linkColumn = "cod_munic";
  std::string csvFileName = "D:/Workspace/FIOCRUZ/data/flow/c50mun_rec.csv";
  int fromIdx = 0;
  int toIdx = 1;
  int weightIdx = 2;

  //std::string shapeFileName = "D:/WS/dados/flow/reg.txt_pol.shp";
  //std::string linkColumn = "object_id_";
  //std::string csvFileName = "D:/WS/dados/flow/T_INT_97.csv";
  //int fromIdx = 1;
  //int toIdx = 3;
  //int weightIDx = 5;

  int srid = 4618;

  std::string graphName = "flowMuG";

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

    //if (!builder.build(shapeFileName, linkColumn, srid, csvFileName, fromIdx, toIdx, weightIdx, connInfo, graphType, graphInfo))
    //{
    //  //std::cout << std::endl << "An exception has occuried in Graph Example: " << builder->getErrorMessage() << std::endl;
    //}

    graph = builder.getGraph();
  }
  catch (const std::exception& e)
  {
    //std::cout << std::endl << "An exception has occuried in Graph Example: " << e.what() << std::endl;
  }
  catch (...)
  {
    //std::cout << std::endl << "An unexpected exception has occuried in Graph Example!" << std::endl;
  }

  //export graph to shape
  std::map<std::string, std::string> connInfoOut;
  connInfoOut["URI"] = "d:/teste.shp";

  std::auto_ptr<te::da::DataSource> ds = te::da::DataSourceFactory::make("OGR");
  ds->setConnectionInfo(connInfoOut);
  ds->open();

  std::string dataSetName = "teste";

  //export
  te::qt::plugins::fiocruz::FlowGraphExport fge;

  try
  {
    te::da::DataSourcePtr dsPtr(ds.get());

    fge.exportGraph(dsPtr, dataSetName, graph.get());
  }
  catch (const std::exception& e)
  {
    
  }
  catch (...)
  {
    
  }

  ds->close();

  graph->flush();
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
      m_ui->m_spatialPropertyComboBox->addItem(dsType->getProperties()[t]->getName().c_str());
  }
}

void te::qt::plugins::fiocruz::FlowDiagramDialog::onTabularLayerComboBoxActivated(int index)
{
  QVariant varLayer = m_ui->m_tabularLayerComboBox->itemData(index, Qt::UserRole);

  te::map::AbstractLayerPtr layer = varLayer.value<te::map::AbstractLayerPtr>();

  m_ui->m_tabularOriginComboBox->clear();
  m_ui->m_tabularDestinyComboBox->clear();

  //set properties from tabular layer into combos
  std::auto_ptr<te::da::DataSetType> dsType = layer->getSchema();

  for (std::size_t t = 0; t < dsType->getProperties().size(); ++t)
  {
    te::dt::Property* prop = dsType->getProperties()[t];

    if (prop->getType() == te::dt::INT32_TYPE || prop->getType() == te::dt::STRING_TYPE)
    {
      m_ui->m_tabularOriginComboBox->addItem(dsType->getProperties()[t]->getName().c_str());
      m_ui->m_tabularDestinyComboBox->addItem(dsType->getProperties()[t]->getName().c_str());
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
