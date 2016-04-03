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
\file fiocruz/src/fiocruz/flow/CalculateMainFlow.h

\brief This file defines the CalulateMainFlow class
*/

#ifndef __FIOCRUZ_INTERNAL_FLOW_CALCULATEMAINFLOW_H
#define __FIOCRUZ_INTERNAL_FLOW_CALCULATEMAINFLOW_H

// TerraLib
#include <terralib/graph/core/AbstractGraph.h>
#include <terralib/graph/core/Vertex.h>
#include <terralib/graph/graphs/BidirectionalGraph.h>

#include "../Config.h"

// STL
#include <memory>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        /*!
        \class CalculateMainFlow

        \brief This class is used calculate the main flow of graph
        */
        class CalculateMainFlow
        {

          public:

            CalculateMainFlow();

            ~CalculateMainFlow();

          public:

            void calculate(te::graph::BidirectionalGraph* biGraph, const double& dominanceRelation, const bool& checkLocalDominance, const double& localDominanceValue, bool addStatisticsColumns);

          protected:

            void buildGraph(te::graph::BidirectionalGraph* biGraph, bool addStatisticsColumns);

            int getVertexAttrIdx(te::graph::AbstractGraph* graph, std::string attrName);

            int getEdgeAttrIdx(te::graph::AbstractGraph* graph, std::string attrName);

            int getEdgeWeightAttrValue(te::graph::Edge* e, int attrIdx);

            te::graph::Edge* getHighWeightEdge(te::graph::BidirectionalGraph* biGraph, std::vector<te::graph::Edge*> edgeVec);

            std::vector<te::graph::Vertex*> getRoots(te::graph::AbstractGraph* graph);

            void buildLevel(te::graph::BidirectionalGraph* graph, std::vector<te::graph::Vertex*> roots, int levelVertexAttrIdx, int domVertexAttrIdx);

            void buildLevel(te::graph::BidirectionalGraph* graph, te::graph::Vertex* vertex, int level, int levelVertexAttrIdx, int domVertexAttrIdx);

            std::vector<te::graph::Vertex*> getDominatedNodes(te::graph::BidirectionalGraph* graph, te::graph::Vertex* vertex, int domVertexAttrIdx);

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_FLOW_CALCULATEMAINFLOW_H

