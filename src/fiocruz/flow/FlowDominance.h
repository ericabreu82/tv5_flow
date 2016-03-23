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
\file fiocruz/src/fiocruz/flow/FlowDominance.h

\brief This file defines the Flow Dominance class
*/

#ifndef __FIOCRUZ_INTERNAL_FLOW_FLOWDOMINANCE_H
#define __FIOCRUZ_INTERNAL_FLOW_FLOWDOMINANCE_H

// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/graph/core/AbstractGraph.h>
#include <terralib/graph/core/Edge.h>

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
        enum DominanceType
        {
          DOMINANCE_INPUTFLOW,
          DOMINANCE_OUTPUTFLOW
        };

        /*!
        \class FlowDominance

        \brief This class is used to associate a dominance value to a graph vertex
        */
        class FlowDominance
        {

          public:

            FlowDominance();

            ~FlowDominance();

          public:

            void associate(te::graph::AbstractGraph* graph, te::da::DataSourcePtr ds, std::string dataSetName, int idIdx, int domIdx);

            void calculate(te::graph::AbstractGraph* graph, DominanceType domType);

          protected:

            int addDominanceAttr(te::graph::AbstractGraph* graph);

            int getEdgeWeightAttrValue(te::graph::AbstractGraph* graph, te::graph::Edge* e);

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_FLOW_FLOWDOMINANCE_H

