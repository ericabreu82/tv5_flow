/*  Copyright (C) 2008-2013 National Institute For Space Research (INPE) - Brazil.

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
  \file fiocruz/src/fiocruz/Plugin.cpp

  \brief Plugin implementation for the Fiocruz Qt Plugin.
*/

// TerraLib
#include <terralib/common/Translator.h>
#include <terralib/common/Logger.h>
#include <terralib/qt/af/ApplicationController.h>

#include "Plugin.h"

#ifdef FIOCRUZ_HAVE_FLOWCLASSIFY
  #include "flow/FlowClassifyAction.h"
#endif

#ifdef FIOCRUZ_HAVE_FLOWDIAGRAM
  #include "flow/FlowDiagramAction.h"
#endif

#ifdef FIOCRUZ_HAVE_FLOWNETWORK
#include "flow/FlowNetworkAction.h"
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONRASTER
  #include "regionalization/RegionalizationRasterAction.h"
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONVECTOR
  #include "regionalization/RegionalizationVectorAction.h"
#endif

// QT
#include <QMenu>
#include <QMenuBar>

te::qt::plugins::fiocruz::Plugin::Plugin(const te::plugin::PluginInfo& pluginInfo)
  : te::plugin::Plugin(pluginInfo), m_menu(0)
{
}

te::qt::plugins::fiocruz::Plugin::~Plugin()
{
}

void te::qt::plugins::fiocruz::Plugin::startup()
{
  if(m_initialized)
    return;

  te::qt::af::AppCtrlSingleton::getInstance().addListener(this, te::qt::af::SENDER);

  TE_LOG_TRACE(TE_TR("Fiocruz Plugin startup!"));

// add plugin menu
  m_menu = te::qt::af::AppCtrlSingleton::getInstance().getMenu("Fiocruz");
  m_menu->setTitle(TE_TR("Fiocruz"));

  m_flowMenu = new QMenu(m_menu);
  m_flowMenu->setTitle(TE_TR("Flow"));
  m_menu->addMenu(m_flowMenu);

  m_regMenu = new QMenu(m_menu);
  m_regMenu->setTitle(TE_TR("Regionalization"));
  m_menu->addMenu(m_regMenu);

// register actions
  registerActions();

  m_initialized = true;
}

void te::qt::plugins::fiocruz::Plugin::shutdown()
{
  if(!m_initialized)
    return;

// remove menu
  delete m_menu;

// unregister actions
  unRegisterActions();

  TE_LOG_TRACE(TE_TR("Fiocruz Plugin shutdown!"));

  m_initialized = false;
}

void te::qt::plugins::fiocruz::Plugin::registerActions()
{
#ifdef FIOCRUZ_HAVE_FLOWCLASSIFY
  m_flowClassify = new te::qt::plugins::fiocruz::FlowClassifyAction(m_flowMenu);
  connect(m_flowClassify, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef FIOCRUZ_HAVE_FLOWDIAGRAM
  m_flowDiagram = new te::qt::plugins::fiocruz::FlowDiagramAction(m_flowMenu);
  connect(m_flowDiagram, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef FIOCRUZ_HAVE_FLOWNETWORK
  m_flowNetwork = new te::qt::plugins::fiocruz::FlowNetworkAction(m_flowMenu);
  connect(m_flowNetwork, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONRASTER
  m_regRaster = new te::qt::plugins::fiocruz::RegionalizationRasterAction(m_regMenu);
  connect(m_regRaster, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONVECTOR
  m_regVector = new te::qt::plugins::fiocruz::RegionalizationVectorAction(m_regMenu);
  connect(m_regVector, SIGNAL(triggered(te::qt::af::evt::Event*)), SIGNAL(triggered(te::qt::af::evt::Event*)));
#endif
}

void  te::qt::plugins::fiocruz::Plugin::unRegisterActions()
{
#ifdef FIOCRUZ_HAVE_FLOWCLASSIFY
    delete m_flowClassify;
#endif

#ifdef FIOCRUZ_HAVE_FLOWDIAGRAM
    delete m_flowDiagram;
#endif

#ifdef FIOCRUZ_HAVE_FLOWNETWORK
    delete m_flowNetwork;
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONRASTER
    delete m_regRaster;
#endif

#ifdef FIOCRUZ_HAVE_REGIONALIZATIONVECTOR
    delete m_regVector;
#endif
}

PLUGIN_CALL_BACK_IMPL(te::qt::plugins::fiocruz::Plugin)
