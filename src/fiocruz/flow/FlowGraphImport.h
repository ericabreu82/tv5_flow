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
\file fiocruz/src/fiocruz/flow/FlowGraphImport.h

\brief This file defines the Flow Graph Import class
*/

#ifndef __FIOCRUZ_INTERNAL_FLOW_FLOWGRAPHIMPORT_H
#define __FIOCRUZ_INTERNAL_FLOW_FLOWGRAPHIMPORT_H

// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/geometry/LineString.h>
#include <terralib/graph/core/AbstractGraph.h>
#include <terralib/memory/DataSet.h>

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
        \class FlowGraphImport

        \brief This class is used to Import a graph to a datasource
        */
        class FlowGraphImport
        {

          public:

            FlowGraphImport();

            ~FlowGraphImport();

          public:

            te::graph::AbstractGraph* importGraph(std::auto_ptr<te::da::DataSet> dataSet, int originIdx, int destinyIdx, int weightIdx, int geomidx);

          protected:

            te::graph::AbstractGraph* buildGraph();

            te::gm::LineString* getLine(te::gm::Geometry* geom);

            /*!
            \brief Function used to generated the edge id

            \return Integer value as ID

            */
            int getEdgeId();

          protected:

            int m_edgeId;  //!< Attribute used as a index counter for edge objects

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_FLOW_FLOWGRAPHIMPORT_H

