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
\file fiocruz/src/fiocruz/flow/FlowDominance.cpp

\brief This file defines the Flow Dominance class
*/


#include "FlowDominance.h"

//terralib
#include <terralib/datatype/SimpleData.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/graph/core/AbstractGraph.h>
#include <terralib/graph/core/AbstractGraphFactory.h>
#include <terralib/graph/core/GraphMetadata.h>
#include <terralib/graph/core/Edge.h>
#include <terralib/graph/core/Vertex.h>
#include <terralib/graph/graphs/BidirectionalGraph.h>
#include <terralib/graph/iterator/MemoryIterator.h>


te::qt::plugins::fiocruz::FlowDominance::FlowDominance()
{
}

te::qt::plugins::fiocruz::FlowDominance::~FlowDominance()
{

}

void te::qt::plugins::fiocruz::FlowDominance::associate(te::graph::AbstractGraph* graph, te::da::DataSourcePtr ds, std::string dataSetName, int idIdx, int domIdx)
{
  assert(graph);

  int domAttrIdx = addDominanceAttr(graph);

  int graphVecAttrSize = graph->getMetadata()->getVertexPropertySize();

  //get dataset
  std::auto_ptr<te::da::DataSet> dataSet = ds->getDataSet(dataSetName);
  dataSet->moveBeforeFirst();

  while (dataSet->moveNext())
  {
    std::string curIdStr = dataSet->getAsString(idIdx);
    std::string curDomStr = dataSet->getAsString(domIdx);

    //get vertex
    te::graph::Vertex* v = graph->getVertex(atoi(curIdStr.c_str()));

    if (v)
    {
      //set dominance value
      v->setAttributeVecSize(graphVecAttrSize);

      v->addAttribute(graphVecAttrSize - 1, new te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>(atol(curDomStr.c_str())));
    }
  }
}

void te::qt::plugins::fiocruz::FlowDominance::calculate(te::graph::AbstractGraph* graph, DominanceType domType)
{
  assert(graph);

  int domAttrIdx = addDominanceAttr(graph);

  int graphVecAttrSize = graph->getMetadata()->getVertexPropertySize();

  te::graph::BidirectionalGraph* biGraph = dynamic_cast<te::graph::BidirectionalGraph*>(graph);

  if (!biGraph)
    return;

  //get iterator
  te::graph::MemoryIterator* iterator = new te::graph::MemoryIterator(biGraph);

  te::graph::Vertex* v = iterator->getFirstVertex();

  while (v)
  {
    //get vertex neighbour
    std::vector<te::graph::Edge*> edges;

    if (domType == te::qt::plugins::fiocruz::DOMINANCE_INPUTFLOW)
      edges = biGraph->getInEdges(v->getId());
    else if (domType == te::qt::plugins::fiocruz::DOMINANCE_OUTPUTFLOW)
      edges = biGraph->getOutEdges(v->getId());

    //calculate dominance
    double dominanceValue = 0.;

    for (std::size_t t = 0; t < edges.size(); ++t)
    {
      dominanceValue += getEdgeWeightAttrValue(graph, edges[t]);
    }

    //set dominance value
    v->setAttributeVecSize(graphVecAttrSize);
    v->addAttribute(graphVecAttrSize - 1, new te::dt::SimpleData<double, te::dt::DOUBLE_TYPE>(dominanceValue));

    v = iterator->getNextVertex();
  }
}

int te::qt::plugins::fiocruz::FlowDominance::addDominanceAttr(te::graph::AbstractGraph* graph)
{
  //create property
  std::string attrName = "dominance";
  te::dt::SimpleProperty* p = new te::dt::SimpleProperty(attrName, te::dt::DOUBLE_TYPE);
  p->setParent(0);
  p->setId(0);

  graph->addVertexProperty(p);

  // verify what the index of the new property
  int idx = 0;

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

int te::qt::plugins::fiocruz::FlowDominance::getEdgeWeightAttrValue(te::graph::AbstractGraph* graph, te::graph::Edge* e)
{
  std::string attrName = "weight";

  // verify what the index of the weight property
  int idx = -1;

  for (int i = 0; i < graph->getVertexPropertySize(); ++i)
  {
    if (graph->getVertexProperty(i)->getName() == attrName)
    {
      idx = i;
      break;
    }
  }

  //get value
  if (idx != -1)
  {
    std::string weightStr = e->getAttributes()[idx]->toString();

    return atoi(weightStr.c_str());
  }

  return 0;
}
