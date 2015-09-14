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
\file fiocruz/src/fiocruz/regionalization/RegionalizationVectorWizardPage.h

\brief This file defines the Regionalization Vector Wizard Page class
*/

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONVECTORWIZARDPAGE_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONVECTORWIZARDPAGE_H

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QWizardPage>


// Forward declaration
namespace Ui { class RegionalizationVectorWizardPageForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {

        /*!
        \class RegionalizationVectorWizardPage

        \brief This file defines the Regionalization Vector Wizard Page class
        */
        class RegionalizationVectorWizardPage : public QWizardPage
        {
          Q_OBJECT

        public:

          RegionalizationVectorWizardPage(QWidget* parent = 0);

          ~RegionalizationVectorWizardPage();

          bool isComplete() const;

        public:

          void setList(std::list<te::map::AbstractLayerPtr>& layerList);

        private:

          std::auto_ptr<Ui::RegionalizationVectorWizardPageForm> m_ui;

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONVECTORWIZARDPAGE_H
