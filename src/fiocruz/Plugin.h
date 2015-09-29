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
  \file fiocruz/src/fiocruz/Plugin.h

  \brief Plugin implementation for the Fiocruz Qt Plugin.
*/

#ifndef __FIOCRUZ_INTERNAL_PLUGIN_H
#define __FIOCRUZ_INTERNAL_PLUGIN_H

// TerraLib
#include <terralib/plugin/Plugin.h>
#include "Config.h"

// Qt
#include <QAction>
#include <QMenu>

namespace te
{
  namespace qt
  {
    namespace af
    {
      namespace evt
      {
        struct Event;
      }
    }
    namespace plugins
    {
      namespace fiocruz
      {
        class FlowClassifyAction;
        class FlowDiagramAction;
        class FlowNetworkAction;
        class RegionalizationRasterAction;
        class RegionalizationVectorAction;
        
        class Plugin : public QObject,  public te::plugin::Plugin
        {
          Q_OBJECT

          public:

            Plugin(const te::plugin::PluginInfo& pluginInfo);

            ~Plugin();

            void startup();

            void shutdown();

          protected:

            /*!
              \brief Function used to register all actions.

            */
            void registerActions();

            /*!
              \brief Function used to unregister all actions.

            */
            void unRegisterActions();

          Q_SIGNALS:

            void triggered(te::qt::af::evt::Event* e);

          protected:

            QMenu* m_menu;                                                      //!< fiocruz Main Menu registered.
            QMenu* m_flowMenu;                                                  //!< Flow Main Menu registered.
            QMenu* m_regMenu;                                                   //!< Regionalization Main Menu registered.

            te::qt::plugins::fiocruz::FlowClassifyAction* m_flowClassify;       //!< Flow Classify Operation Process Action
            te::qt::plugins::fiocruz::FlowDiagramAction* m_flowDiagram;         //!< Flow Diagram Operation Process Action
            te::qt::plugins::fiocruz::FlowNetworkAction* m_flowNetwork;         //!< Flow Network Operation Process Action
            te::qt::plugins::fiocruz::RegionalizationRasterAction* m_regRaster; //!< Regionalization Raster Operation Process Action
            te::qt::plugins::fiocruz::RegionalizationVectorAction* m_regVector; //!< Regionalization Vector Operation Process Action
        };

      } // end namespace fiocruz
    }   // end namespace plugins
  }     // end namespace qt
}       // end namespace te

PLUGIN_CALL_BACK_DECLARATION(FIOCRUZDLLEXPORT);

#endif //__FIOCRUZ_INTERNAL_PLUGIN_H
