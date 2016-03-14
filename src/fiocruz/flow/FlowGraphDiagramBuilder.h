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
\file fiocruz/src/fiocruz/flow/FlowGraphDiagramBuilder.h

\brief This file defines the Flow Graph Diagram Builder class
*/

#ifndef __FIOCRUZ_INTERNAL_FLOW_FLOWGRAPHDIAGRAMBUILDER_H
#define __FIOCRUZ_INTERNAL_FLOW_FLOWGRAPHDIAGRAMBUILDER_H

// TerraLib
#include <terralib/graph/core/AbstractGraph.h>

#include "../Config.h"

// STL
#include <map>
#include <memory>
#include <string>

// BOOST Includes
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>

namespace te
{
  // Forward declarations
  namespace da
  {
    class DataSource;
    class DataSet;
  }

  namespace dt { class Property; }

  namespace gm { class GeometryProperty; }

  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        /*!
        \class FlowGraphDiagramBuilder

        \brief This class defines the graph builder for flow graph
        */
        class FlowGraphDiagramBuilder
        {

        public:

          FlowGraphDiagramBuilder();

          ~FlowGraphDiagramBuilder();

        public:

          /*!
          \brief Function used to build the output graph based on input parameters.

          \param spatialDs            Data Source wiht vectorial data
          \param spatialDataSetName   Data set name wiht vectorial data
          \param linkColumn           Column name from vectorial data used as link column
          \param srid                 Vectorial projection id
          \param tabularDs            Data Source wiht tabular data
          \param tabularDataSetName   Data set name wiht tabular data
          \param fromIdx              Index for column table with origin information.
          \param toIdx                Index for column table with destiny information.
          \param weightIdx            Index for column table with weight information.
          \param dsInfo               Container with data source information
          \param graphType            Attribute used to define the output graph type
          \param gInfo                Container with graph generation parameters

          \return True if the graph was correctly generated and false in other case.

          */
          bool build(te::da::DataSourcePtr spatialDs, const std::string& spatialDataSetName, const std::string& linkColumn, const int& srid, 
            te::da::DataSourcePtr tabularDs, const std::string& tabularDataSetName, const int& fromIdx, const int& toIdx, const int& weightIdx,
            const std::map<std::string, std::string>& dsInfo, const std::string& graphType, const std::map<std::string, std::string>& gInfo);

          /*! \brief Get error message. */
          std::string getErrorMessage();

          /*! \brief Get generated graph. */
          boost::shared_ptr<te::graph::AbstractGraph> getGraph();

        protected:

          /*!
          \brief Function used to generated the edge id

          \return Integer value as ID

          */
          int getEdgeId();

          /*!
          \brief Function used to create all vertex object based on vectorial data

          \param spatialDs            Data Source wiht vectorial data
          \param spatialDataSetName   Data set name wiht vectorial data
          \param linkColumn           Column name from vectorial data used as link column
          \param srid                 Vectorial projection id

          \return True if the vertexs was created correctly and false in othe case

          */
          bool createVertexObjects(te::da::DataSourcePtr spatialDs, const std::string& spatialDataSetName, const std::string& linkColumn, const int& srid);

          /*!
          \brief Function used to create all edges object based on flow table data

          \param tabularDs            Data Source wiht tabular data
          \param tabularDataSetName   Data set name wiht tabular data
          \param fromIdx              Index for column table with origin information.
          \param toIdx                Index for column table with destiny information.
          \param weightIdx            Index for column table with weight information.

          \return True if the edges was created correctly and false in othe case

          */
          bool createEdgeObjects(te::da::DataSourcePtr tabularDs, const std::string& tabularDataSetName, const int& fromIdx, const int& toIdx, const int& weightIdx);

        protected:

          boost::shared_ptr<te::graph::AbstractGraph> m_graph;   //!< Graph object

          std::string m_errorMessage;                            //!< Error message

          int m_edgeId;  //!< Attribute used as a index counter for edge objects

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_FLOW_FLOWGRAPHDIAGRAMBUILDER_H

