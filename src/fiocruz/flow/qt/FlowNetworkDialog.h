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
\file fiocruz/src/fiocruz/flow/qt/FlowNetworkDialog.h

\brief This file defines the Flow Network dialog class
*/

#ifndef __FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKDIALOG_H
#define __FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKDIALOG_H

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceInfo.h>
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class FlowNetworkDialogForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        /*!
        \class FlowNetworkDialog

        \brief This file defines the Flow Network dialog class
        */
        class FlowNetworkDialog : public QDialog
        {
          Q_OBJECT

        public:

          FlowNetworkDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

          ~FlowNetworkDialog();

        public:

          void setLayerList(std::list<te::map::AbstractLayerPtr> list);

          te::map::AbstractLayerPtr getOutputLayer();

        public slots:

          void onFlowLayerComboBoxActivated(int index);

          void onSpatialLayerComboBoxActivated(int index);

          void onTabularLayerComboBoxActivated(int index);

          void onTargetDatasourceToolButtonPressed();

          void onTargetFileToolButtonPressed();

          void onOkPushButtonClicked();

        private:

          std::auto_ptr<Ui::FlowNetworkDialogForm> m_ui;

          te::da::DataSourceInfoPtr m_outputDatasource;
          
          te::map::AbstractLayerPtr m_outputLayer;
        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_FLOW_FLOWNETWORKDIALOG_H

