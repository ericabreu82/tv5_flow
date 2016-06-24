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
  std::vector<te::graph::Vertex*> roots = getRoots(biGraph, checkLocalDominance);

  //set level info into graph
  int levelVertexAttrIdx = getVertexAttrIdx(biGraph, "level");
  int domVertexAttrIdx = getVertexAttrIdx(biGraph, "dominance");

  buildLevel(biGraph, roots, levelVertexAttrIdx, domVertexAttrIdx);
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

int te::qt::plugins::fiocruz::CalculateMainFlow::getVertexAttrIdx(te::graph::AbstractGraph* graph, std::string attrName)
{
  int idx = -1;

  for (int i = 0; i < graph->getVertexPropertySize(); ++i)
  {
    if (graph->getVertexProperty(i)->getName() == attrName)
    {
      idx = i;
      break;
    }
  }

  return idx;
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

int te::qt::plugins::fiocruz::CalculateMainFlow::getEdgeMainFlowAttrValue(te::graph::Edge* e, int attrIdx)
{
  std::string mainFlowStr = e->getAttributes()[attrIdx]->toString();

  return atoi(mainFlowStr.c_str());
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

std::vector<te::graph::Vertex*> te::qt::plugins::fiocruz::CalculateMainFlow::getRoots(te::graph::AbstractGraph* graph, bool checkLocalDominance)
{
  std::vector<te::graph::Vertex*> roots;

  int mainFlowIdx = getEdgeAttrIdx(graph, "main_flow");
  int domVertexAttrIdx = getVertexAttrIdx(graph, "dominance");

  //iterate over the graph
  std::auto_ptr<te::graph::MemoryIterator> memIt(new te::graph::MemoryIterator(graph));

  te::graph::Edge* edge = memIt->getFirstEdge();

  while (edge)
  {
    int mainFlowValue = getEdgeMainFlowAttrValue(edge, mainFlowIdx);

    //verify only main edges
    if (mainFlowValue == 1)
    {
      te::graph::Vertex* vFrom = graph->getVertex(edge->getIdFrom());
      te::graph::Vertex* vTo = graph->getVertex(edge->getIdTo());

      if (vFrom && vTo)
      {
        bool check = false;

        if (vFrom->getId() == vTo->getId() && checkLocalDominance)
          check = true;
        else if (vFrom->getId() != vTo->getId())
          check = true;

        if (check)
        {
          te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>* vDomDataFrom = dynamic_cast<te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>*>(vFrom->getAttributes()[domVertexAttrIdx]);
          double vDomFromValue = vDomDataFrom->getValue();

          te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>* vDomDataTo = dynamic_cast<te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>*>(vTo->getAttributes()[domVertexAttrIdx]);
          double vDomToValue = vDomDataTo->getValue();

          //if main flow is from a vertex with dominance value higher than to destiny, than this vertex is root
          if (vDomFromValue >= vDomToValue)
          {
            roots.push_back(vFrom);
          }
        }
      }
    }

    edge = memIt->getNextEdge();
  }

  return roots;
}

void te::qt::plugins::fiocruz::CalculateMainFlow::buildLevel(te::graph::BidirectionalGraph* graph, std::vector<te::graph::Vertex*> roots, int levelVertexAttrIdx, int domVertexAttrIdx)
{
  for (std::size_t t = 0; t < roots.size(); ++t)
  {
    int level = 0;

    buildLevel(graph, roots[t], level, levelVertexAttrIdx, domVertexAttrIdx);
  }
}

void te::qt::plugins::fiocruz::CalculateMainFlow::buildLevel(te::graph::BidirectionalGraph* graph, te::graph::Vertex* vertex, int level, int levelVertexAttrIdx, int domVertexAttrIdx)
{
  vertex->addAttribute(levelVertexAttrIdx, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(level));

  std::vector<te::graph::Vertex*> dominatedNodes = getDominatedNodes(graph, vertex, domVertexAttrIdx);

  for (std::size_t t = 0; t < dominatedNodes.size(); ++t)
    buildLevel(graph, dominatedNodes[t], level + 1, levelVertexAttrIdx, domVertexAttrIdx);
}

std::vector<te::graph::Vertex*> te::qt::plugins::fiocruz::CalculateMainFlow::getDominatedNodes(te::graph::BidirectionalGraph* graph, te::graph::Vertex* vertex, int domVertexAttrIdx)
{
  std::vector<te::graph::Vertex*> domVec;

  te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>* vDomData = dynamic_cast< te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>* >(vertex->getAttributes()[domVertexAttrIdx]);

  double vDomValue = vDomData->getValue();

  std::vector<te::graph::Vertex*> vNeigh = graph->getVertexNeighborhood(vertex->getId());

  for (std::size_t t = 0; t < vNeigh.size(); ++t)
  {
    te::graph::Vertex* vCur = vNeigh[t];

    te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>* vCurDomData = dynamic_cast< te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>* >(vCur->getAttributes()[domVertexAttrIdx]);

    double vCurDomValue = vCurDomData->getValue();

    if (vDomValue > vCurDomValue)
      domVec.push_back(vCur);
  }

  return domVec;
}
