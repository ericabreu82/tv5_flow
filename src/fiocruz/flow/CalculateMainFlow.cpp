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
\file fiocruz/src/fiocruz/flow/CalculateMainFlow.cpp

\brief This file defines the CalulateMainFlow class
*/


#include "CalculateMainFlow.h"

//terralib
#include <terralib/datatype/SimpleData.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/graph/core/AbstractGraph.h>
#include <terralib/graph/Globals.h>
#include <terralib/graph/core/AbstractGraphFactory.h>
#include <terralib/graph/core/GraphMetadata.h>
#include <terralib/graph/core/Edge.h>
#include <terralib/graph/core/Vertex.h>
#include <terralib/graph/iterator/MemoryIterator.h>

// STL
#include <queue>


te::qt::plugins::fiocruz::CalculateMainFlow::CalculateMainFlow()
{
}

te::qt::plugins::fiocruz::CalculateMainFlow::~CalculateMainFlow()
{

}

void te::qt::plugins::fiocruz::CalculateMainFlow::calculate(te::graph::BidirectionalGraph* biGraph, const double& dominanceRelation, const bool& checkLocalDominance, const double& localDominanceValue, bool addStatisticsColumns)
{
  //add new properties
  buildGraph(biGraph, addStatisticsColumns);

  int mainFlowIdx = getEdgeAttrIdx(biGraph, "main_flow");

  //iterate over the graph
  std::auto_ptr<te::graph::MemoryIterator> memIt(new te::graph::MemoryIterator(biGraph));

  te::graph::Vertex* vertex = memIt->getFirstVertex();

  while (vertex)
  {
    //get output edges from this vertex
    std::vector<te::graph::Edge*> outEdges = biGraph->getOutEdges(vertex->getId());

    if (!outEdges.empty())
    {
      //get the edge with the higher value of weight
      te::graph::Edge* edge = getHighWeightEdge(biGraph, outEdges);

      //change value of main flow attr to 1
      edge->addAttribute(mainFlowIdx, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(1));
    }

    vertex = memIt->getNextVertex();
  }

  //get roots
  std::vector<int> roots = getRoots(biGraph);

  //set level info into graph
  buildLevel(roots);
}
void te::qt::plugins::fiocruz::CalculateMainFlow::buildGraph(te::graph::BidirectionalGraph* biGraph, bool addStatisticsColumns)
{
  //add level attr to vertex
  {
    te::dt::SimpleProperty* p = new te::dt::SimpleProperty("level", te::dt::INT32_TYPE);
    p->setParent(0);
    p->setId(0);
    biGraph->addVertexProperty(p);
  }

  if (addStatisticsColumns)
  {
    // Adiciona o campo com a informacao de quem e o no superior imediato do no corrente
    {
      te::dt::SimpleProperty* p = new te::dt::SimpleProperty("destiny", te::dt::INT32_TYPE);
      p->setParent(0);
      p->setId(0);
      biGraph->addVertexProperty(p);
    }

    // Adiciona o campo com a informacao de quantos nohs possuem na rede deste root
    {
      te::dt::SimpleProperty* p = new te::dt::SimpleProperty("tree", te::dt::INT32_TYPE);
      p->setParent(0);
      p->setId(0);
      biGraph->addVertexProperty(p);
    }

    // Adiciona o campo com a informacao de quantos filhos este noh possue
    {
      te::dt::SimpleProperty* p = new te::dt::SimpleProperty("input", te::dt::INT32_TYPE);
      p->setParent(0);
      p->setId(0);
      biGraph->addVertexProperty(p);
    }
  }

  //add main flow attr to edge
  {
    te::dt::SimpleProperty* p = new te::dt::SimpleProperty("main_flow", te::dt::INT32_TYPE);
    p->setParent(0);
    p->setId(0);
    biGraph->addEdgeProperty(p);
  }

  int grapEdgeAttrSize = biGraph->getMetadata()->getEdgePropertySize();


  //initialize main flow attr with 0 value
  std::auto_ptr<te::graph::MemoryIterator> memIt(new te::graph::MemoryIterator(biGraph));

  te::graph::Edge* edge = memIt->getFirstEdge();

  while (edge)
  {
    edge->setAttributeVecSize(grapEdgeAttrSize);

    edge->addAttribute(grapEdgeAttrSize - 1, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(0));

    edge = memIt->getNextEdge();
  }


  //initialize vector attrs with -1 value
  int vecAttrSize = biGraph->getMetadata()->getVertexPropertySize();

  te::graph::Vertex* vertex = memIt->getFirstVertex();

  while (vertex)
  {
    vertex->setAttributeVecSize(vecAttrSize);

    vertex->addAttribute(vecAttrSize - 4, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(-1));

    if (addStatisticsColumns)
    {
      vertex->addAttribute(vecAttrSize - 3, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(-1));
      vertex->addAttribute(vecAttrSize - 2, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(-1));
      vertex->addAttribute(vecAttrSize - 1, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(-1));
    }

    vertex = memIt->getNextVertex();
  }

}

int te::qt::plugins::fiocruz::CalculateMainFlow::getEdgeAttrIdx(te::graph::AbstractGraph* graph, std::string attrName)
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

int te::qt::plugins::fiocruz::CalculateMainFlow::getEdgeWeightAttrValue(te::graph::Edge* e, int attrIdx)
{
  std::string weightStr = e->getAttributes()[attrIdx]->toString();

  return atoi(weightStr.c_str());
}

te::graph::Edge* te::qt::plugins::fiocruz::CalculateMainFlow::getHighWeightEdge(te::graph::BidirectionalGraph* biGraph, std::vector<te::graph::Edge*> edgeVec)
{
  te::graph::Edge* edge = 0;

  int weightAttrIdx = getEdgeAttrIdx(biGraph, "weight");

  int highWeight = -1;

  for (std::size_t t = 0; t < edgeVec.size(); ++t)
  {
    te::graph::Edge* curEdge = edgeVec[t];

    int curWeight = getEdgeWeightAttrValue(curEdge, weightAttrIdx);

    if (curWeight > highWeight)
    {
      highWeight = curWeight;
      edge = curEdge;
    }
  }

  return edge;
}

std::vector<int> te::qt::plugins::fiocruz::CalculateMainFlow::getRoots(te::graph::AbstractGraph* graph)
{
  std::vector<int> roots;

  //iterate over the graph
  std::auto_ptr<te::graph::MemoryIterator> memIt(new te::graph::MemoryIterator(graph));

  te::graph::Vertex* vertex = memIt->getFirstVertex();

  while (vertex)
  {
    std::set<int> successors = vertex->getSuccessors();

    if (successors.empty())
    {
      roots.push_back(vertex->getId());
    }

    vertex = memIt->getNextVertex();
  }

  return roots;
}

void te::qt::plugins::fiocruz::CalculateMainFlow::buildLevel(std::vector<int> roots)
{

}

void te::qt::plugins::fiocruz::CalculateMainFlow::buildLevel(te::graph::Vertex* vertex, int level)
{

}
