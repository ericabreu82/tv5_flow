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
\file fiocruz/src/fiocruz/regionalization/MapWizardPage.h

\brief This file defines the Regionalization Map Wizard Page class
*/

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_MAPWIZARDPAGE_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_MAPWIZARDPAGE_H

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QWizardPage>


// Forward declaration
namespace Ui { class MapWizardPageForm; }

namespace te
{
  namespace qt
  {
    namespace widgets { class ColorCatalogWidget; }

    namespace plugins
    {
      namespace fiocruz
      {

        /*!
        \class MapWizardPage

        \brief This file defines the Regionalization Map Wizard Page class
        */
        class MapWizardPage : public QWizardPage
        {
          Q_OBJECT

        public:

          MapWizardPage(QWidget* parent = 0);

          ~MapWizardPage();

          bool isComplete() const;

        public:

          void setList(std::list<te::map::AbstractLayerPtr>& layerList);

        private:

          std::auto_ptr<Ui::MapWizardPageForm> m_ui;

          te::qt::widgets::ColorCatalogWidget* m_colorBar;          //!< Widget used to pick a color.

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_REGIONALIZATION_MAPWIZARDPAGE_H
