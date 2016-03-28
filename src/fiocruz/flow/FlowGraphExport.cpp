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
\file fiocruz/src/fiocruz/flow/FlowGraphExport.cpp

\brief This file defines the Flow Graph Export class
*/

//terralib
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/LineString.h>
#include <terralib/graph/core/AbstractGraph.h>
#include <terralib/graph/core/GraphMetadata.h>
#include <terralib/graph/core/Edge.h>
#include <terralib/graph/core/Vertex.h>
#include <terralib/graph/iterator/MemoryIterator.h>
#include <terralib/memory/DataSet.h>
#include <terralib/memory/DataSetItem.h>


#include "FlowGraphExport.h"

te::qt::plugins::fiocruz::FlowGraphExport::FlowGraphExport()
{

}

te::qt::plugins::fiocruz::FlowGraphExport::~FlowGraphExport()
{

}

void te::qt::plugins::fiocruz::FlowGraphExport::exportGraph(te::da::DataSourcePtr ds, std::string dataSetName, te::graph::AbstractGraph* graph, FlowGraphExportType exportType)
{
  std::auto_ptr<te::da::DataSetType> dsType;

  std::auto_ptr<te::mem::DataSet> dataSet;

  if (exportType == FlowGraphExportType::FLOWGRAPH_EDGE_TYPE)
  {
    dsType = createEdgeDataSetType(dataSetName, graph);
    dataSet = createEdgeDataSet(dsType.get(), graph);
  }
  else if (exportType == FlowGraphExportType::FLOWGRAPH_VERTEX_TYPE)
  {
    dsType = createVertexDataSetType(dataSetName, graph);
    dataSet = createVertexDataSet(dsType.get(), graph);
  }
  else
  {
    throw;
  }

  //save data
  if (dsType.get() && dataSet.get())
  {
    dataSet->moveBeforeFirst();

    std::map<std::string, std::string> options;

    ds->createDataSet(dsType.get(), options);

    ds->add(dataSetName, dataSet.get(), options);
  }
}

std::auto_ptr<te::da::DataSetType> te::qt::plugins::fiocruz::FlowGraphExport::createEdgeDataSetType(std::string dataSetName, te::graph::AbstractGraph* graph)
{
  std::auto_ptr<te::da::DataSetType> dataSetType(new te::da::DataSetType(dataSetName));

  //create index property
  te::dt::SimpleProperty* idxProperty = new te::dt::SimpleProperty("index", te::dt::INT32_TYPE);
  dataSetType->add(idxProperty);

  //create all graph properties
  for (int i = 0; i < graph->getMetadata()->getEdgePropertySize(); ++i)
  {
    te::dt::Property* prop = graph->getMetadata()->getEdgeProperty(i);

    te::dt::Property* newProp = prop->clone();
    newProp->setId(0);
    newProp->setParent(0);

    dataSetType->add(newProp);
  }

  //create geometry prop
  te::gm::GeometryProperty* geomProp = new te::gm::GeometryProperty("line", graph->getMetadata()->getSRID(), te::gm::LineStringType, true);
  dataSetType->add(geomProp);

  return dataSetType;
}

std::auto_ptr<te::da::DataSetType> te::qt::plugins::fiocruz::FlowGraphExport::createVertexDataSetType(std::string dataSetName, te::graph::AbstractGraph* graph)
{
  std::auto_ptr<te::da::DataSetType> dataSetType(new te::da::DataSetType(dataSetName));

  //create index property
  te::dt::SimpleProperty* idxProperty = new te::dt::SimpleProperty("index", te::dt::INT32_TYPE);
  dataSetType->add(idxProperty);

  //create all graph properties
  for (int i = 0; i < graph->getMetadata()->getVertexPropertySize(); ++i)
  {
    te::dt::Property* prop = graph->getMetadata()->getVertexProperty(i);

    te::dt::Property* newProp = prop->clone();
    newProp->setId(0);
    newProp->setParent(0);

    dataSetType->add(newProp);
  }

  return dataSetType;
}

std::auto_ptr<te::mem::DataSet> te::qt::plugins::fiocruz::FlowGraphExport::createEdgeDataSet(te::da::DataSetType* dsType, te::graph::AbstractGraph* graph)
{
  std::auto_ptr<te::mem::DataSet> outDataset(new te::mem::DataSet(dsType));

  //get property map
  std::map<int, std::string> propMap = getEdgePropertyMap(graph);

  //vertex geom prop
  int spatialPropertyId = -1;
  getGraphVerterxAttrIndex(graph, "coords", spatialPropertyId);

  //create graph iterator
  std::auto_ptr<te::graph::MemoryIterator> it(new te::graph::MemoryIterator(graph));

  te::graph::Edge* e = it->getFirstEdge();

  while (!it->isEdgeIteratorAfterEnd())
  {
    //create dataset item
    te::mem::DataSetItem* outDSetItem = new te::mem::DataSetItem(outDataset.get());

    //get vertex info
    int idx = e->getId();

    //set index information
    outDSetItem->setInt32("index", idx);

    //set the other attributes
    std::vector<te::dt::AbstractData*> adVec = e->getAttributes();

    std::map<int, std::string>::iterator itMap = propMap.begin();

    while (itMap != propMap.end())
    {
      te::dt::AbstractData* adClone = adVec[itMap->first]->clone();

      outDSetItem->setValue(itMap->second, adClone);

      ++itMap;
    }

    //create line
    te::graph::Vertex* vFrom = graph->getVertex(e->getIdFrom());
    te::graph::Vertex* vTo = graph->getVertex(e->getIdTo());

    if (vFrom && vTo)
    {
      te::gm::Point* pFrom = dynamic_cast<te::gm::Point*>(vFrom->getAttributes()[spatialPropertyId]);
      te::gm::Point* pTo = dynamic_cast<te::gm::Point*>(vTo->getAttributes()[spatialPropertyId]);

      te::gm::LineString* line = new te::gm::LineString(2, te::gm::LineStringType, graph->getMetadata()->getSRID());
      line->setPoint(0, pFrom->getX(), pFrom->getY());
      line->setPoint(1, pTo->getX(), pTo->getY());

      outDSetItem->setGeometry("line", line);

      //add item into dataset
      outDataset->add(outDSetItem);
    }
    else
    {
      delete outDSetItem;
    }

    e = it->getNextEdge();
  }

  return outDataset;
}

std::auto_ptr<te::mem::DataSet> te::qt::plugins::fiocruz::FlowGraphExport::createVertexDataSet(te::da::DataSetType* dsType, te::graph::AbstractGraph* graph)
{
  std::auto_ptr<te::mem::DataSet> outDataset(new te::mem::DataSet(dsType));

  //get property map
  std::map<int, std::string> propMap = getVertexPropertyMap(graph);

  //vertex geom prop
  int spatialPropertyId = -1;
  getGraphVerterxAttrIndex(graph, "coords", spatialPropertyId);

  //create graph iterator
  std::auto_ptr<te::graph::MemoryIterator> it(new te::graph::MemoryIterator(graph));

  te::graph::Vertex* v = it->getFirstVertex();

  while (!it->isVertexIteratorAfterEnd())
  {
    //create dataset item
    te::mem::DataSetItem* outDSetItem = new te::mem::DataSetItem(outDataset.get());

    //get vertex info
    int idx = v->getId();

    //set index information
    outDSetItem->setInt32("index", idx);

    //set the other attributes
    std::vector<te::dt::AbstractData*> adVec = v->getAttributes();

    std::map<int, std::string>::iterator itMap = propMap.begin();

    while (itMap != propMap.end())
    {
      te::dt::AbstractData* adClone = adVec[itMap->first]->clone();

      outDSetItem->setValue(itMap->second, adClone);

      ++itMap;
    }

    //add item into dataset
    outDataset->add(outDSetItem);
   

    v = it->getNextVertex();
  }

  return outDataset;
}

std::map<int, std::string> te::qt::plugins::fiocruz::FlowGraphExport::getEdgePropertyMap(te::graph::AbstractGraph* graph)
{
  std::map<int, std::string> propMap;

  for (int i = 0; i < graph->getMetadata()->getEdgePropertySize(); ++i)
  {
    te::dt::Property* prop = graph->getMetadata()->getEdgeProperty(i);

    propMap.insert(std::map<int, std::string>::value_type(i, prop->getName()));
  }

  return propMap;
}

std::map<int, std::string> te::qt::plugins::fiocruz::FlowGraphExport::getVertexPropertyMap(te::graph::AbstractGraph* graph)
{
  std::map<int, std::string> propMap;

  for (int i = 0; i < graph->getMetadata()->getVertexPropertySize(); ++i)
  {
    te::dt::Property* prop = graph->getMetadata()->getVertexProperty(i);

    propMap.insert(std::map<int, std::string>::value_type(i, prop->getName()));
  }

  return propMap;
}

bool te::qt::plugins::fiocruz::FlowGraphExport::getGraphVerterxAttrIndex(te::graph::AbstractGraph* graph, std::string attrName, int& index)
{
  for (int i = 0; i < graph->getVertexPropertySize(); ++i)
  {
    if (graph->getVertexProperty(i)->getName() == attrName)
    {
      index = i;
      return true;
    }
  }

  return false;
}