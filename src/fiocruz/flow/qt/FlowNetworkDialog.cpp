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

#include "../CalculateMainFlow.h"
#include "../FlowGraphImport.h"
#include "../FlowGraphExport.h"
#include "../FlowDominance.h"
#include "FlowNetworkDialog.h"
#include "ui_FlowNetworkDialogForm.h"

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceInfoManager.h>
#include <terralib/dataaccess/datasource/DataSourceManager.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/dataaccess/query_h.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/maptools/QueryLayer.h>
#include <terralib/qt/widgets/datasource/selector/DataSourceSelectorDialog.h>
#include <terralib/qt/widgets/layer/utils/DataSet2Layer.h>
#include <terralib/qt/widgets/utils/ScopedCursor.h>
#include <terralib/se/Utils.h>

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

  //connects
  connect(m_ui->m_domLayerComboBox, SIGNAL(activated(int)), this, SLOT(onDomLayerComboBoxActivated(int)));
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

    //check if the layer is a flow layer
    te::dt::Property* fromProp = dsType->getProperty("from_id");
    te::dt::Property* fromNameProp = dsType->getProperty("from_name");
    te::dt::Property* toProp = dsType->getProperty("to_id");
    te::dt::Property* toNameProp = dsType->getProperty("to_name");
    te::dt::Property* weightProp = dsType->getProperty("weight");
    te::dt::Property* distProp = dsType->getProperty("distance");

    if (fromProp && fromNameProp && toProp && toNameProp && weightProp && distProp)
      m_ui->m_flowLayerComboBox->addItem(l->getTitle().c_str(), QVariant::fromValue(l));


    if (!dsType->hasRaster())
    {
      m_ui->m_domLayerComboBox->addItem(l->getTitle().c_str(), QVariant::fromValue(l));
    }

    ++it;
  }

  if (m_ui->m_domLayerComboBox->count() > 0)
    onDomLayerComboBoxActivated(0);
}

std::vector<te::map::AbstractLayerPtr> te::qt::plugins::fiocruz::FlowNetworkDialog::getOutputLayers()
{
  return m_outputLayerList;
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::onOkPushButtonClicked()
{
  te::qt::widgets::ScopedCursor cursor(Qt::WaitCursor);

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

  //check dominance relation parameters
  if (m_ui->m_domRelOthersLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Dominance relation not defined."));
    return;
  }

  double dominanceRelation = m_ui->m_domRelOthersLineEdit->text().toDouble();

  bool checkLocalDominance = m_ui->m_domRelLocalCheckBox->isChecked();

  if (checkLocalDominance && m_ui->m_domRelLocalLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Local Dominance relation not defined."));
    return;
  }

  double localDominanceRelation = 0.;

  if (checkLocalDominance)
    localDominanceRelation = m_ui->m_domRelLocalLineEdit->text().toDouble();
  
  //check output parameters
  if (m_ui->m_repositoryLineEdit->text().isEmpty() || m_ui->m_newLayerNameLineEdit->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Output information not defined."));
    return;
  }

  createDataSources();

  if (!m_outputDatasourceEdge.get() || !m_outputDatasourceVertex.get())
  {
    QMessageBox::warning(this, tr("Warning"), tr("Output data source not defined."));
    return;
  }

  //output info
  std::string dataSetName = m_ui->m_newLayerNameLineEdit->text().toStdString();
  std::size_t idx = dataSetName.find(".");
  if (idx != std::string::npos)
    dataSetName = dataSetName.substr(0, idx);

  //std::string graphName = dataSetName;

  //load graph
  QVariant flowVarLayer = m_ui->m_flowLayerComboBox->currentData(Qt::UserRole);
  te::map::AbstractLayerPtr flowLayer = flowVarLayer.value<te::map::AbstractLayerPtr>();
  std::auto_ptr<te::da::DataSet> flowDataSet = flowLayer->getData();

  int flowGeomColumnIdx = te::da::GetFirstSpatialPropertyPos(flowDataSet.get());

  te::qt::plugins::fiocruz::FlowGraphImport fgi;

  te::graph::AbstractGraph* graph = 0;

  try
  {
    graph = fgi.importGraph(flowDataSet, flowGeomColumnIdx, m_ui->m_outputStatisticsCheckBox->isChecked());
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

  //get main flow
  te::qt::plugins::fiocruz::CalculateMainFlow cmf;

  try
  {
    cmf.calculate(dynamic_cast<te::graph::BidirectionalGraph*>(graph), dominanceRelation, checkLocalDominance, localDominanceRelation, m_ui->m_outputStatisticsCheckBox->isChecked());
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());

    graph->flush();

    return;
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error"));

    graph->flush();

    return;
  }


  //export
  try
  {
    exportEdges(graph);

    exportNodes(graph);
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());

    graph->flush();

    return;
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error"));

    graph->flush();

    return;
  }

  graph->flush();

  QMessageBox::information(this, tr("Information"), tr("Flow Network Created."));

  accept();
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

void te::qt::plugins::fiocruz::FlowNetworkDialog::onTargetFileToolButtonPressed()
{
  m_ui->m_newLayerNameLineEdit->clear();
  m_ui->m_repositoryLineEdit->clear();

  QString fileName = QFileDialog::getExistingDirectory(this, tr("Select directory to save flow network..."));

  if (fileName.isEmpty())
    return;

  m_ui->m_repositoryLineEdit->setText(fileName);
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::createDataSources()
{
  std::string path = m_ui->m_repositoryLineEdit->text().toStdString();

  std::string baseName = m_ui->m_newLayerNameLineEdit->text().toStdString();

  {
    std::string edgeDataSourceName = path + "/" + baseName + "_edges.shp";

    //create new data source
    boost::filesystem::path uri(edgeDataSourceName);

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

    m_outputDatasourceEdge = dsInfoPtr;
  }

  {
    std::string vertexDataSourceName = path + "/" + baseName + "_nodes.shp";

    //create new data source
    boost::filesystem::path uri(vertexDataSourceName);

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

    m_outputDatasourceVertex = dsInfoPtr;
  }
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::exportEdges(te::graph::AbstractGraph* graph)
{
  std::string baseName = m_ui->m_newLayerNameLineEdit->text().toStdString();
  std::string dataSetName = baseName + "_edges";

  te::da::DataSourcePtr outputDataSource = te::da::DataSourceManager::getInstance().get(m_outputDatasourceEdge->getId(), m_outputDatasourceEdge->getType(), m_outputDatasourceEdge->getConnInfo());

  te::qt::plugins::fiocruz::FlowGraphExport fge;

  try
  {
    fge.exportGraph(outputDataSource, dataSetName, graph, te::qt::plugins::fiocruz::FLOWGRAPH_EDGE_TYPE);
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());

    graph->flush();

    return;
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error"));

    graph->flush();

    return;
  }

  outputDataSource->close();

  {
    //create layer
    te::da::DataSourcePtr ds = te::da::GetDataSource(m_outputDatasourceEdge->getId());

    te::qt::widgets::DataSet2Layer converter(m_outputDatasourceEdge->getId());

    te::da::DataSetTypePtr dt(ds->getDataSetType(ds->getDataSetNames()[0]).release());

    te::map::AbstractLayerPtr layerEdge = converter(dt);

    te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(layerEdge.get());
    if (dsLayer != 0)
    {
      dsLayer->setRendererType("FLOWNETWORK_LAYER_RENDERER");
    }

    m_outputLayerList.push_back(layerEdge);
  }


  {
    te::da::DataSourcePtr ds = te::da::GetDataSource(m_outputDatasourceEdge->getId());

    static boost::uuids::basic_random_generator<boost::mt19937> gen;
    boost::uuids::uuid u = gen();
    std::string id = boost::uuids::to_string(u);

    std::auto_ptr<te::da::DataSetType> dsTypeEdge = ds->getDataSetType(ds->getDataSetNames()[0]);

    std::string title = dsTypeEdge->getTitle() + "_mainFLow";
    std::vector<te::dt::Property*> props = dsTypeEdge->getProperties();

    te::da::Fields* fields = new te::da::Fields;

    for (std::size_t t = 0; t < props.size(); ++t)
    {
      if (props[t]->getName() == "FID" || props[t]->getName() == "fid")
        continue;

      te::da::Field* fItem = new te::da::Field(dataSetName + "." + props[t]->getName());

      fields->push_back(fItem);
    }

    te::da::From* from = new te::da::From;
    te::da::FromItem* fromItem = new te::da::DataSetName(dataSetName, dataSetName);
    from->push_back(fromItem);

    te::da::PropertyName* propMainFlow = new te::da::PropertyName("main_flow");
    te::da::Literal* valMainFlow = new te::da::LiteralInt32(1);
    te::da::EqualTo* equalToMainFlow = new te::da::EqualTo(propMainFlow, valMainFlow);

    te::da::Where* where = new te::da::Where(equalToMainFlow);

    te::da::Select* s = new te::da::Select();
    s->setFields(fields);
    s->setFrom(from);
    s->setWhere(where);

    te::map::QueryLayerPtr layer(new te::map::QueryLayer(id, title));
    layer->setDataSourceId(ds->getId());
    layer->setRendererType("FLOWNETWORK_LAYER_RENDERER");
    layer->setQuery(s);
    layer->computeExtent();

    // SRID
    std::auto_ptr<const te::map::LayerSchema> schema(layer->getSchema());
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());
    if (gp)
    {
      layer->setSRID(gp->getSRID());

      // style
      layer->setStyle(te::se::CreateFeatureTypeStyle(gp->getGeometryType()));
    }

    m_outputLayerList.push_back(layer);
  }
}

void te::qt::plugins::fiocruz::FlowNetworkDialog::exportNodes(te::graph::AbstractGraph* graph)
{
  std::string baseName = m_ui->m_newLayerNameLineEdit->text().toStdString();
  std::string dataSetName = baseName + "_nodes";

  te::da::DataSourcePtr outputDataSource = te::da::DataSourceManager::getInstance().get(m_outputDatasourceVertex->getId(), m_outputDatasourceVertex->getType(), m_outputDatasourceVertex->getConnInfo());

  te::qt::plugins::fiocruz::FlowGraphExport fge;

  try
  {
    fge.exportGraph(outputDataSource, dataSetName, graph, te::qt::plugins::fiocruz::FLOWGRAPH_VERTEX_TYPE);
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Warning"), e.what());

    graph->flush();

    return;
  }
  catch (...)
  {
    QMessageBox::warning(this, tr("Warning"), tr("Internal Error"));

    graph->flush();

    return;
  }

  outputDataSource->close();

  {
    //create layer
    te::da::DataSourcePtr ds = te::da::GetDataSource(m_outputDatasourceVertex->getId());

    te::qt::widgets::DataSet2Layer converter(m_outputDatasourceVertex->getId());

    te::da::DataSetTypePtr dt(ds->getDataSetType(ds->getDataSetNames()[0]).release());

    te::map::AbstractLayerPtr layerVertex = converter(dt);

    te::map::DataSetLayer* dsLayer = dynamic_cast<te::map::DataSetLayer*>(layerVertex.get());
    if (dsLayer != 0)
    {
      dsLayer->setRendererType("FLOWNETWORK_LAYER_RENDERER");
    }

    m_outputLayerList.push_back(layerVertex);
  }


  {
    te::da::DataSourcePtr ds = te::da::GetDataSource(m_outputDatasourceVertex->getId());

    static boost::uuids::basic_random_generator<boost::mt19937> gen;
    boost::uuids::uuid u = gen();
    std::string id = boost::uuids::to_string(u);

    std::auto_ptr<te::da::DataSetType> dsTypeVertex = ds->getDataSetType(ds->getDataSetNames()[0]);

    std::string title = dsTypeVertex->getTitle() + "_roots";
    std::vector<te::dt::Property*> props = dsTypeVertex->getProperties();

    te::da::Fields* fields = new te::da::Fields;

    for (std::size_t t = 0; t < props.size(); ++t)
    {
      if (props[t]->getName() == "FID" || props[t]->getName() == "fid")
        continue;

      te::da::Field* fItem = new te::da::Field(dataSetName + "." + props[t]->getName());

      fields->push_back(fItem);
    }

    te::da::From* from = new te::da::From;
    te::da::FromItem* fromItem = new te::da::DataSetName(dataSetName, dataSetName);
    from->push_back(fromItem);

    te::da::PropertyName* propLevel = new te::da::PropertyName("level");
    te::da::Literal* valLevel = new te::da::LiteralInt32(0);
    te::da::EqualTo* equalToLevel = new te::da::EqualTo(propLevel, valLevel);

    te::da::Where* where = new te::da::Where(equalToLevel);

    te::da::Select* s = new te::da::Select();
    s->setFields(fields);
    s->setFrom(from);
    s->setWhere(where);

    te::map::QueryLayerPtr layer(new te::map::QueryLayer(id, title));
    layer->setDataSourceId(ds->getId());
    layer->setRendererType("FLOWNETWORK_LAYER_RENDERER");
    layer->setQuery(s);
    layer->computeExtent();

    // SRID
    std::auto_ptr<const te::map::LayerSchema> schema(layer->getSchema());
    te::gm::GeometryProperty* gp = te::da::GetFirstGeomProperty(schema.get());
    if (gp)
    {
      layer->setSRID(gp->getSRID());

      // style
      layer->setStyle(te::se::CreateFeatureTypeStyle(gp->getGeometryType()));
    }

    m_outputLayerList.push_back(layer);
  }
}

