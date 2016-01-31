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
\file fiocruz/src/fiocruz/regionalization/ExternalTableWizardPage.h

\brief This file defines the Regionalization External Table Wizard Page class
*/

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_EXTERNALTABLEWIZARDPAGE_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_EXTERNALTABLEWIZARDPAGE_H

//Regionalization
#include "../Regionalization.h"

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QWizardPage>


// Forward declaration
namespace Ui { class ExternalTableWizardPageForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        class RegionalizationInputParams;

        /*!
          \class ExternalTableWizardPage

          \brief This file defines the Regionalization External Table Wizard Page class
          */
        class ExternalTableWizardPage : public QWizardPage
        {
          Q_OBJECT

        public:

          ExternalTableWizardPage(QWidget* parent = 0);

          ~ExternalTableWizardPage();

          bool isComplete() const;

        public:

          void setList(std::list<te::map::AbstractLayerPtr>& layerList);

          te::qt::plugins::fiocruz::VecStringPair getUniqueObjects();

          te::qt::plugins::fiocruz::RegionalizationInputParams* getRegionalizationInputParameters();

          std::vector<std::string> getTabularAttributes();

          void getExtentInfo(te::gm::Envelope& env, int& srid);;

        public slots:

        void onSpatialLayerComboBoxActivated(int index);

        void onTabularLayerComboBoxActivated(int index);

        private:

          std::auto_ptr<Ui::ExternalTableWizardPageForm> m_ui;

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_REGIONALIZATION_EXTERNALTABLEWIZARDPAGE_H
