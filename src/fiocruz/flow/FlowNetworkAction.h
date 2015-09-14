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
  \file fiocruz/src/fiocruz/flow/FlowNetworkAction.h

  \brief This file defines the Flow Network Action class
*/

#ifndef __FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKACTION_H
#define __FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKACTION_H

// Fiocruz
#include "../AbstractAction.h"
#include "../Config.h"

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        /*!
          \class FlowNetworkAction
          
          \brief This file defines the Flow Network Action class

        */
        class FlowNetworkAction : public te::qt::plugins::fiocruz::AbstractAction
        {
          Q_OBJECT

          public:

            FlowNetworkAction(QMenu* menu);

            virtual ~FlowNetworkAction();

          protected slots:

            virtual void onActionActivated(bool checked);
        };

      } // end namespace fiocruz
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

#endif //__FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKACTION_H
