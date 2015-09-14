/*  Copyright (C) 2008-2014 National Institute For Space Research (INPE) - Brazil.

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
  \file fiocruz/src/fiocruz/Config.h

  \brief Configuration flags for the fiocruz Qt plugin.
*/

#ifndef __FIOCRUZ_INTERNAL_CONFIG_H
#define __FIOCRUZ_INTERNAL_CONFIG_H

// TerraLib
//#include <terralib_config.h>


/*!
  \def FIOCRUZ_HAVE_FLOWCLASSIFY

  \brief It defines if the fiocruz Plugin has the flow classify operation.
*/
#define FIOCRUZ_HAVE_FLOWCLASSIFY

/*!
  \def FIOCRUZ_HAVE_FLOWDIAGRAM

  \brief It defines if the fiocruz Plugin has the flow diagram operation.
*/
#define FIOCRUZ_HAVE_FLOWDIAGRAM

/*!
  \def FIOCRUZ_HAVE_FLOWNETWORK

  \brief It defines if the fiocruz Plugin has the flow network operation.
*/
#define FIOCRUZ_HAVE_FLOWNETWORK

/*!
\def FIOCRUZ_HAVE_REGIONALIZATIONRASTER

\brief It defines if the fiocruz Plugin has the regionalization raster operation.
*/
#define FIOCRUZ_HAVE_REGIONALIZATIONRASTER

/*!
  \def FIOCRUZ_HAVE_REGIONALIZATIONVECTOR

  \brief It defines if the fiocruz Plugin has the regionalization vector operation.
*/
#define FIOCRUZ_HAVE_REGIONALIZATIONVECTOR

/*!
  \def FIOCRUZ_PLUGIN_NAME

  \brief It contains the plugin name.
*/
#define FIOCRUZ_PLUGIN_NAME "te.qt.fiocruz"

/*!
  \def FIOCRUZDLLEXPORT

  \brief You can use this macro in order to export/import classes and functions from all plug-ins files.

  \note To compile plug-ins in Windows, remember to insert FIOCRUZDLLEXPORT into the project's list of defines.
 */
#ifdef WIN32
  #ifdef FIOCRUZDLL
    #define FIOCRUZDLLEXPORT  __declspec(dllexport)   // export DLL information
  #else
    #define FIOCRUZDLLEXPORT  __declspec(dllimport)   // import DLL information
  #endif 
#else
  #define FIOCRUZDLLEXPORT
#endif

#endif  // __FIOCRUZ_INTERNAL_CONFIG_H

