/*  Copyright (C) 2008 National Institute For Space Research (INPE) - Brazil.

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
\file fiocruz/src/fiocruz/regionalization/RegionalizationVectorWizard.h

\brief This file defines the Regionalization Vector Wizard class
*/

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONVECTORWIZARD_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONVECTORWIZARD_H

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QWizard>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {

        class ExternalTableWizardPage;
        class LegendWizardPage;
        class MapWizardPage;
        class RegionalizationVectorWizardPage;

        /*!
        \class RegionalizationVectorWizard

        \brief This file defines the Regionalization Vector Wizard class
        */
        class RegionalizationVectorWizard : public QWizard
        {

        public:

          RegionalizationVectorWizard(QWidget* parent);

          ~RegionalizationVectorWizard();

        public:

          virtual bool validateCurrentPage();

          void setList(std::list<te::map::AbstractLayerPtr>& layerList);

        protected:

          void addPages();

        private:

          std::auto_ptr<te::qt::plugins::fiocruz::RegionalizationVectorWizardPage> m_regionalizationPage;
          std::auto_ptr<te::qt::plugins::fiocruz::MapWizardPage> m_mapPage;
          std::auto_ptr<te::qt::plugins::fiocruz::LegendWizardPage> m_legendPage;
          std::auto_ptr<te::qt::plugins::fiocruz::ExternalTableWizardPage> m_externalTablePage;


        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONVECTORWIZARD_H
