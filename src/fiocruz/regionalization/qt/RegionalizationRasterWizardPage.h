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
\file fiocruz/src/fiocruz/regionalization/RegionalizationRasterWizardPage.h

\brief This file defines the Regionalization Raster Wizard Page class
*/

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONRASTERWIZARDPAGE_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONRASTERWIZARDPAGE_H

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"
#include "../RasterInterpolate.h"
#include <terralib/sa/Enums.h>

// STL
#include <memory>

// Qt
#include <QWizardPage>


// Forward declaration
namespace Ui { class RegionalizationRasterWizardPageForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {

        /*!
        \class RegionalizationRasterWizardPage

        \brief This file defines the Regionalization Raster Wizard Page class
        */
        class RegionalizationRasterWizardPage : public QWizardPage
        {
          Q_OBJECT

        public:

          RegionalizationRasterWizardPage(QWidget* parent = 0);

          ~RegionalizationRasterWizardPage();

          bool isComplete() const;

        public:

          void setAttrbutes(std::vector<std::string>& attrs);

          bool hasSpatialInformation();

          void getSpatialAttributesNames(std::string& xAttr, std::string& yAttr);

          void getResolution(double& resX, double& resY);

          te::sa::KernelFunctionType getKernelFunctionType();

          KernelInterpolationAlgorithm getKernelInterpolationAlgorithm();

          size_t getNumberOfNeighbours();

          double getRadius();

          std::string getBaseName();

          std::string getPath();

          void setExtent(te::gm::Envelope env, int srid);


        protected slots:

          void onResXLineEditEditingFinished();

          void onResYLineEditEditingFinished();

          void onTargetFileToolButtonClicked();

        protected:

          void fillKernelParameters();

        private:

          std::auto_ptr<Ui::RegionalizationRasterWizardPageForm> m_ui;

          te::gm::Envelope m_envelope;

          int m_srid;

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_REGIONALIZATION_REGIONALIZATIONRASTERWIZARDPAGE_H
