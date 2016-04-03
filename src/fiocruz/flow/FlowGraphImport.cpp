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
\file fiocruz/src/fiocruz/flow/FlowGraphImport.cpp

\brief This file defines the Flow Graph Import class
*/


#include "FlowGraphImport.h"

//terralib
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/datatype/SimpleData.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/MultiLineString.h>
#include <terralib/graph/Globals.h>
#include <terralib/graph/core/AbstractGraph.h>
#include <terralib/graph/core/AbstractGraphFactory.h>
#include <terralib/graph/core/GraphMetadata.h>
#include <terralib/graph/core/Edge.h>
#include <terralib/graph/core/Vertex.h>
#include <terralib/graph/iterator/MemoryIterator.h>


te::qt::plugins::fiocruz::FlowGraphImport::FlowGraphImport()
{
  m_edgeId = 0;
}

te::qt::plugins::fiocruz::FlowGraphImport::~FlowGraphImport()
{

}

te::graph::AbstractGraph* te::qt::plugins::fiocruz::FlowGraphImport::importGraph(std::auto_ptr<te::da::DataSet> dataSet, int geomidx, bool addStatisticsColumns)
{
  te::graph::AbstractGraph* graph = buildGraph(addStatisticsColumns);

  int originIdx = te::da::GetPropertyIndex(dataSet.get(), "from_id");
  int originNameIdx = te::da::GetPropertyIndex(dataSet.get(), "from_name");
  int destinyIdx = te::da::GetPropertyIndex(dataSet.get(), "to_id");
  int destinyNameIdx = te::da::GetPropertyIndex(dataSet.get(), "to_name");
  int weightIdx = te::da::GetPropertyIndex(dataSet.get(), "weight");
  int distIdx = te::da::GetPropertyIndex(dataSet.get(), "distance");

  //fill graph
  dataSet->moveBeforeFirst();

  while (dataSet->moveNext())
  {
    std::string originId = dataSet->getAsString(originIdx);
    std::string originName = dataSet->getAsString(originNameIdx);
    std::string destinyId = dataSet->getAsString(destinyIdx);
    std::string destinyName = dataSet->getAsString(destinyNameIdx);
    std::string weightStr = dataSet->getAsString(weightIdx);
    std::string distStr = dataSet->getAsString(distIdx);

    //get geometry from edge (lineString with 2 coords)
    std::auto_ptr<te::gm::Geometry> geom = dataSet->getGeometry(geomidx);
    te::gm::LineString* line = getLine(geom.get());

    if (line)
    {
      //check if graph has the origin vertex
      te::graph::Vertex* vFrom = graph->getVertex(atoi(originId.c_str()));

      if (!vFrom)
      {
        //create vertex
        vFrom = new te::graph::Vertex(atoi(originId.c_str()));
        vFrom->setAttributeVecSize(2); 

        vFrom->addAttribute(0, new te::dt::SimpleData<std::string, te::dt::STRING_TIME>(originName));
        vFrom->addAttribute(1, new te::gm::Point(*line->getPointN(0)));

        //add to graph
        graph->add(vFrom);
      }

      //check if graph has the destiny vertex
      te::graph::Vertex* vTo = graph->getVertex(atoi(destinyId.c_str()));

      if (!vTo)
      {
        //create vertex
        vTo = new te::graph::Vertex(atoi(destinyId.c_str()));
        vTo->setAttributeVecSize(2);

        vTo->addAttribute(0, new te::dt::SimpleData<std::string, te::dt::STRING_TIME>(destinyName));
        vTo->addAttribute(1, new te::gm::Point(*line->getPointN(1)));

        //add to graph
        graph->add(vTo);
      }

      //create edge
      if (vFrom && vTo)
      {
        int id = getEdgeId();

        te::graph::Edge* e = new te::graph::Edge(id, vFrom->getId(), vTo->getId());

        e->setAttributeVecSize(6);

        e->addAttribute(0, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(vFrom->getId()));
        e->addAttribute(1, new te::dt::SimpleData<std::string, te::dt::STRING_TIME>(originName));
        e->addAttribute(2, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(vTo->getId()));
        e->addAttribute(3, new te::dt::SimpleData<std::string, te::dt::STRING_TIME>(destinyName));
        e->addAttribute(4, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(atoi(weightStr.c_str())));
        e->addAttribute(5, new te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>(atof(distStr.c_str())));

        graph->add(e);
      }
    }
  }

  if (addStatisticsColumns)
    calculateStatistics(graph);

  return graph;
}

te::graph::AbstractGraph* te::qt::plugins::fiocruz::FlowGraphImport::buildGraph(bool addStatisticsColumns)
{
  std::string graphName = "flowGraph";

  // data source information
  std::map<std::string, std::string> connInfo;

  // graph type
  std::string graphType = te::graph::Globals::sm_factoryGraphTypeBidirectionalGraph;

  // graph information
  std::map<std::string, std::string> graphInfo;
  graphInfo["GRAPH_DATA_SOURCE_TYPE"] = "MEM";
  graphInfo["GRAPH_NAME"] = graphName;
  graphInfo["GRAPH_DESCRIPTION"] = "Generated by Flow Builder.";

  te::graph::AbstractGraph* graph = te::graph::AbstractGraphFactory::make(graphType, connInfo, graphInfo);

  //create vertex attributes
  {
    te::dt::SimpleProperty* p = new te::dt::StringProperty("name");
    p->setParent(0);
    p->setId(0);
    graph->addVertexProperty(p);
  }

  {
    te::gm::GeometryProperty* gProp = new te::gm::GeometryProperty("coords");
    gProp->setId(0);
    gProp->setGeometryType(te::gm::PointType);
    graph->addVertexProperty(gProp);
  }

  if (addStatisticsColumns)
  {
    // Adiciona o campo com o número de fluxos de entrada
    {
      te::dt::SimpleProperty* p = new te::dt::SimpleProperty("in_flows", te::dt::INT32_TYPE);
      p->setParent(0);
      p->setId(0);
      graph->addVertexProperty(p);
    }

    // Adiciona o campo com o número de fluxos de saída
    {
      te::dt::SimpleProperty* p = new te::dt::SimpleProperty("out_flows", te::dt::INT32_TYPE);
      p->setParent(0);
      p->setId(0);
      graph->addVertexProperty(p);
    }

    // Adiciona o campo com o somatório dos valores dos fluxos de entrada
    {
      te::dt::SimpleProperty* p = new te::dt::SimpleProperty("sum_in", te::dt::INT32_TYPE);
      p->setParent(0);
      p->setId(0);
      graph->addVertexProperty(p);
    }

    // Adiciona o campo com o somatório dos valores dos fluxos de saída
    {
      te::dt::SimpleProperty* p = new te::dt::SimpleProperty("sum_out", te::dt::INT32_TYPE);
      p->setParent(0);
      p->setId(0);
      graph->addVertexProperty(p);
    }
  }

  //create edge attributes
  {//add from property to graph
    te::dt::SimpleProperty* p = new te::dt::SimpleProperty("from_id", te::dt::INT32_TYPE);
    p->setParent(0);
    p->setId(0);
    graph->addEdgeProperty(p);
  }

  {//add from alias property to graph
    te::dt::SimpleProperty* p = new te::dt::StringProperty("from_name");
    p->setParent(0);
    p->setId(0);
    graph->addEdgeProperty(p);
  }

  {//add to property to graph
    te::dt::SimpleProperty* p = new te::dt::SimpleProperty("to_id", te::dt::INT32_TYPE);
    p->setParent(0);
    p->setId(0);
    graph->addEdgeProperty(p);
  }

  {//add to alias property to graph
    te::dt::SimpleProperty* p = new te::dt::StringProperty("to_name");
    p->setParent(0);
    p->setId(0);
    graph->addEdgeProperty(p);
  }

  {//add weight property to graph
    te::dt::SimpleProperty* p = new te::dt::SimpleProperty("weight", te::dt::INT32_TYPE);
    p->setParent(0);
    p->setId(0);
    graph->addEdgeProperty(p);
  }

  {//add distance property to graph
    te::dt::SimpleProperty* p = new te::dt::SimpleProperty("distance", te::dt::DOUBLE_TYPE);
    p->setParent(0);
    p->setId(0);
    graph->addEdgeProperty(p);
  }

  return graph;
}

void te::qt::plugins::fiocruz::FlowGraphImport::calculateStatistics(te::graph::AbstractGraph* graph)
{
  //calculate statistics
  std::auto_ptr<te::graph::MemoryIterator> memIt(new te::graph::MemoryIterator(graph));

  te::graph::Vertex* vertex = memIt->getFirstVertex();

  int graphVertexAttrSize = graph->getMetadata()->getVertexPropertySize();

  int weightAttrIdx = getEdgeAttrIdx(graph, "weight");

  while (vertex)
  {
    vertex->setAttributeVecSize(graphVertexAttrSize);

    // Adiciona o campo com o número de fluxos de entrada
    vertex->addAttribute(graphVertexAttrSize - 4, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(vertex->getPredecessors().size()));

    // Adiciona o campo com o número de fluxos de saída
    vertex->addAttribute(graphVertexAttrSize - 3, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(vertex->getSuccessors().size()));

    // Adiciona o campo com o somatório dos valores dos fluxos de entrada
    int sumFlowIn = calculateWeightSum(graph, weightAttrIdx, vertex->getPredecessors());
    vertex->addAttribute(graphVertexAttrSize - 2, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(sumFlowIn));

    // Adiciona o campo com o somatório dos valores dos fluxos de saída
    int sumFlowOut = calculateWeightSum(graph, weightAttrIdx, vertex->getSuccessors());
    vertex->addAttribute(graphVertexAttrSize - 1, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(sumFlowOut));

    vertex = memIt->getNextVertex();
  }

}

int te::qt::plugins::fiocruz::FlowGraphImport::calculateWeightSum(te::graph::AbstractGraph* graph, int weighAttrIdx, std::set<int> edges)
{
  int count = 0;

  std::set<int>::iterator it = edges.begin();

  for (it = edges.begin(); it != edges.end(); ++it)
  {
    te::graph::Edge* e = graph->getEdge(*it);

    if (e)
    {
      std::string weightStr = e->getAttributes()[weighAttrIdx]->toString();

      count += atoi(weightStr.c_str());
    }
  }

  return count;
}

int te::qt::plugins::fiocruz::FlowGraphImport::getEdgeAttrIdx(te::graph::AbstractGraph* graph, std::string attrName)
{
  int idx = -1;

  for (int i = 0; i < graph->getEdgePropertySize(); ++i)
  {
    if (graph->getEdgeProperty(i)->getName() == attrName)
    {
      idx = i;
      break;
    }
  }

  return idx;
}

te::gm::LineString* te::qt::plugins::fiocruz::FlowGraphImport::getLine(te::gm::Geometry* geom)
{
  assert(geom);

  te::gm::LineString* line = 0;

  if (geom->getGeomTypeId() == te::gm::LineStringType)
  {
    line = dynamic_cast<te::gm::LineString*>(geom);
  }
  else if (geom->getGeomTypeId() == te::gm::MultiLineStringType)
  {
    te::gm::MultiLineString* multiLine = dynamic_cast<te::gm::MultiLineString*>(geom);
    
    if (multiLine)
      line = dynamic_cast<te::gm::LineString*>(multiLine->getGeometryN(0));
  }

  return line;
}

int  te::qt::plugins::fiocruz::FlowGraphImport::getEdgeId()
{
  int id = m_edgeId;

  m_edgeId++;

  return id;
}

