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
\file fiocruz/src/fiocruz/flow/qt/FlowNetworkRenderer.h

\brief This file defines the Flow Network Renderer class
*/

#ifndef __FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKRENDERER_H
#define __FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKRENDERER_H

// TerraLib
#include "../../Config.h"

#include <terralib/maptools/AbstractLayerRenderer.h>

namespace te
{
  namespace gm
  {
    Envelope;
  }

  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        /*!
        \class FlowNetworkRenderer

        \brief This file defines the Flow Network Renderer class
        */
        class FlowNetworkRenderer : public te::map::AbstractLayerRenderer
        {

        public:

          FlowNetworkRenderer();

          virtual ~FlowNetworkRenderer();

          //void drawDatSetGeometries(te::da::DataSet* dataset, const std::size_t& gpos,
          //  te::map::Canvas* canvas, int fromSRID, int toSRID, te::map::Chart* chart, te::common::TaskProgress* task = 0);

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKRENDERER_H
