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
\file fiocruz/src/fiocruz/flow/qt/FlowDiagramDialog.h

\brief This file defines the Flow Diagram dialog class
*/

#ifndef __FIOCRUZ_INTERNAL_FLOW_FLOWDIAGRAMDIALOG_H
#define __FIOCRUZ_INTERNAL_FLOW_FLOWDIAGRAMDIALOG_H

// TerraLib
#include <terralib/maptools/AbstractLayer.h>
#include "../../Config.h"

// STL
#include <memory>

// Qt
#include <QDialog>

namespace Ui { class FlowDiagramDialogForm; }

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        /*!
        \class FlowDiagramDialog

        \brief This file defines the Flow Diagram dialog class
        */
        class FlowDiagramDialog : public QDialog
        {
          Q_OBJECT

        public:

          FlowDiagramDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);

          ~FlowDiagramDialog();

        public:

          void setLayerList(std::list<te::map::AbstractLayerPtr> list);

        public slots:

          void onOkPushButtonClicked();

        private:

          std::auto_ptr<Ui::FlowDiagramDialogForm> m_ui;

        };
      }   // end namespace fiocruz
    }     // end namespace plugins
  }       // end namespace qt
}         // end namespace te

#endif  // __FIOCRUZ_INTERNAL_FLOW_FLOWDIAGRAMDIALOG_H

