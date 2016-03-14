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
\file fiocruz/src/fiocruz/flow/FlowGraphDiagramBuilder.cpp

\brief This file defines the Flow Graph Diagram Builder class
*/

//terralib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/datatype/SimpleData.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/geometry/GeometryProperty.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Polygon.h>
#include <terralib/graph/core/AbstractGraphFactory.h>
#include <terralib/graph/core/Edge.h>
#include <terralib/graph/core/Vertex.h>


#include "FlowGraphDiagramBuilder.h"

te::qt::plugins::fiocruz::FlowGraphDiagramBuilder::FlowGraphDiagramBuilder()
{
  m_graph.reset();
  m_errorMessage = "";
  m_edgeId = 0;
}

te::qt::plugins::fiocruz::FlowGraphDiagramBuilder::~FlowGraphDiagramBuilder()
{

}

bool te::qt::plugins::fiocruz::FlowGraphDiagramBuilder::build(te::da::DataSourcePtr spatialDs, const std::string& spatialDataSetName, const std::string& linkColumn, const int& srid,
  te::da::DataSourcePtr tabularDs, const std::string& tabularDataSetName, const int& fromIdx, const int& toIdx, const int& weightIdx,
  const std::map<std::string, std::string>& dsInfo, const std::string& graphType, const std::map<std::string, std::string>& gInfo)
{
  //create output graph
  m_graph.reset(te::graph::AbstractGraphFactory::make(graphType, dsInfo, gInfo));

  assert(m_graph);

  if (createVertexObjects(spatialDs, spatialDataSetName, linkColumn, srid) == false)
  {
    return false;
  }

  if (createEdgeObjects(tabularDs, tabularDataSetName, fromIdx, toIdx, weightIdx) == false)
  {
    return false;
  }

  return true;
}

std::string te::qt::plugins::fiocruz::FlowGraphDiagramBuilder::getErrorMessage()
{
  return m_errorMessage;
}

boost::shared_ptr<te::graph::AbstractGraph> te::qt::plugins::fiocruz::FlowGraphDiagramBuilder::getGraph()
{
  return m_graph;
}

int  te::qt::plugins::fiocruz::FlowGraphDiagramBuilder::getEdgeId()
{
  int id = m_edgeId;

  m_edgeId++;

  return id;
}

bool te::qt::plugins::fiocruz::FlowGraphDiagramBuilder::createVertexObjects(te::da::DataSourcePtr spatialDs, const std::string& spatialDataSetName, const std::string& linkColumn, const int& srid)
{
  //get data set
  std::auto_ptr<te::da::DataSet> dataSet = spatialDs->getDataSet(spatialDataSetName);

  if (dataSet.get() == 0)
  {
    return false;
  }

  //get properties
  boost::ptr_vector<te::dt::Property> properties = spatialDs->getProperties(spatialDataSetName);

  if (properties.empty())
  {
    return false;
  }

  //create graph vertex attrs
  boost::ptr_vector<te::dt::Property>::iterator it = properties.begin();

  int count = 0;

  while (it != properties.end())
  {
    te::dt::Property* p;

    if (it->getType() == te::dt::GEOMETRY_TYPE)
    {
      //create graph attrs
      te::gm::GeometryProperty* gProp = new te::gm::GeometryProperty("coords");
      gProp->setId(0);
      gProp->setGeometryType(te::gm::PointType);
      gProp->setSRID(srid);

      p = gProp;
    }
    else
    {
      p = it->clone();
      p->setParent(0);
      p->setId(count);
    }

    m_graph->addVertexProperty(p);

    ++count;

    ++it;
  }

  //create vertex objects
  while (dataSet->moveNext())
  {
    int id = dataSet->getInt32(linkColumn);

    te::graph::Vertex* v = new te::graph::Vertex(id);

    v->setAttributeVecSize(properties.size());

    it = properties.begin();

    count = 0;

    while (it != properties.end())
    {
      te::dt::AbstractData* ad = 0;

      if (it->getType() == te::dt::GEOMETRY_TYPE)
      {
        te::gm::Geometry* g = dataSet->getGeometry(it->getName()).release();
        g->setSRID(srid);

        if (g->getGeomTypeId() == te::gm::PointType)
        {
          ad = g;
        }
        else if (g->getGeomTypeId() == te::gm::PolygonType)
        {
          te::gm::Point* p = ((te::gm::Polygon*)g)->getCentroid();
          p->setSRID(srid);

          ad = p;
        }
        else if (g->getGeomTypeId() == te::gm::MultiPolygonType)
        {
          te::gm::Polygon* poly = (te::gm::Polygon*)((te::gm::MultiPolygon*)g)->getGeometryN(0);

          te::gm::Point* p = poly->getCentroid();
          p->setSRID(srid);

          ad = p;
        }
      }
      else
      {
        ad = dataSet->getValue(it->getName()).release();
      }

      v->addAttribute(count, ad);

      ++count;

      ++it;
    }

    m_graph->add(v);
  }

  return true;
}

bool te::qt::plugins::fiocruz::FlowGraphDiagramBuilder::createEdgeObjects(te::da::DataSourcePtr tabularDs, const std::string& tabularDataSetName, const int& fromIdx, const int& toIdx, const int& weightIdx)
{
  //add weight property to graph
  te::dt::SimpleProperty* p = new te::dt::SimpleProperty("weight", te::dt::INT32_TYPE);
  p->setParent(0);
  p->setId(0);

  m_graph->addEdgeProperty(p);

  //access tabular data set
  std::auto_ptr<te::da::DataSet> dataSet = tabularDs->getDataSet(tabularDataSetName);

  dataSet->moveBeforeFirst();

  //create edges
  while (dataSet->moveNext())
  {
    std::string fromStr = dataSet->getAsString(fromIdx);
    std::string toStr = dataSet->getAsString(toIdx);
    std::string weightStr = dataSet->getAsString(weightIdx);

    //create edge
    int id = getEdgeId();
    int from = atoi(fromStr.c_str());
    int to = atoi(toStr.c_str());
    int weight = atoi(weightStr.c_str());

    te::graph::Edge* e = new te::graph::Edge(id, from, to);

    e->setAttributeVecSize(1); //weight attribute
    e->addAttribute(0, new te::dt::SimpleData<int, te::dt::INT32_TYPE>(weight));

    m_graph->add(e);
  }

  return true;
}
