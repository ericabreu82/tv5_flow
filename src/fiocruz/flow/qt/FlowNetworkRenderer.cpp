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
\file fiocruz/src/fiocruz/flow/qt/FlowNetworkRenderer.cpp

\brief This file defines the Flow Network Renderer class
*/

#include "FlowNetworkRenderer.h"

#include <terralib/common/STLUtils.h>
#include <terralib/common/progress/TaskProgress.h>
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/maptools/Canvas.h>
#include <terralib/maptools/Chart.h>
#include <terralib/srs/Config.h>

te::qt::plugins::fiocruz::FlowNetworkRendererFactory* te::qt::plugins::fiocruz::FlowNetworkRendererFactory::sm_factory(0);

te::qt::plugins::fiocruz::FlowNetworkRenderer::FlowNetworkRenderer()
  : te::map::AbstractLayerRenderer()
{

}

te::qt::plugins::fiocruz::FlowNetworkRenderer::~FlowNetworkRenderer()
{

}

void te::qt::plugins::fiocruz::FlowNetworkRenderer::drawFlowLine(te::map::Canvas* canvas, te::gm::LineString* line)
{
  assert(line);

  const te::gm::Envelope* envelope = line->getMBR();

  if (envelope->getWidth() == 0. && envelope->getHeight() == 0.)
  {
    te::gm::Point point(envelope->getCenter().getX(), envelope->getCenter().getY());
    canvas->draw(&point);
  }

  canvas->draw(line);
}

void te::qt::plugins::fiocruz::FlowNetworkRenderer::drawDatSetGeometries(te::da::DataSet* dataset, const std::size_t& gpos,
  te::map::Canvas* canvas, int fromSRID, int toSRID, te::map::Chart* chart, bool* cancel, te::common::TaskProgress* task)
{
  assert(dataset);
  assert(canvas);

  // Verify if is necessary convert the data set geometries to the given srid
  bool needRemap = false;
  if ((fromSRID != TE_UNKNOWN_SRS) && (toSRID != TE_UNKNOWN_SRS) && (fromSRID != toSRID))
    needRemap = true;

  do
  {
    if (task)
    {
      if (!task->isActive())
      {
        *cancel = true;
        return;
      }

      // update the draw task
      task->pulse();
    }

    std::auto_ptr<te::gm::Geometry> geom(0);
    try
    {
      geom = dataset->getGeometry(gpos);
      if (geom.get() == 0)
        continue;
    }
    catch (std::exception& /*e*/)
    {
      continue;
    }

    // If necessary, geometry remap
    if (needRemap)
    {
      geom->setSRID(fromSRID);
      geom->transform(toSRID);
    }

    int typeCode = geom->getTypeCode();
    int geomType = geom->getGeomTypeId();
    switch (geomType)
    {
    case te::gm::LineStringType:
    case te::gm::LineStringZType:
    case te::gm::LineStringMType:
    case te::gm::LineStringZMType:
      drawFlowLine(canvas, static_cast<te::gm::LineString*>(geom.get()));
      break;

    default:
      canvas->draw(geom.get());
    }

    if (chart)
      buildChart(chart, dataset, geom.get());

    if (cancel != 0 && (*cancel))
      return;

  } while (dataset->moveNext()); // next geometry!

  // Let's draw the generated charts
  for (std::size_t i = 0; i < m_chartCoordinates.size(); ++i)
  {
    canvas->drawImage(static_cast<int>(m_chartCoordinates[i].x),
      static_cast<int>(m_chartCoordinates[i].y),
      m_chartImages[i],
      chart->getWidth(),
      chart->getHeight());

    te::common::Free(m_chartImages[i], chart->getHeight());
  }
}

void te::qt::plugins::fiocruz::FlowNetworkRendererFactory::initialize()
{
  finalize();
  sm_factory = new FlowNetworkRendererFactory;
}

void te::qt::plugins::fiocruz::FlowNetworkRendererFactory::finalize()
{
  delete sm_factory;
  sm_factory = 0;
}

te::qt::plugins::fiocruz::FlowNetworkRendererFactory::~FlowNetworkRendererFactory()
{
}

te::map::AbstractRenderer* te::qt::plugins::fiocruz::FlowNetworkRendererFactory::build()
{
  return new FlowNetworkRenderer;
}

te::qt::plugins::fiocruz::FlowNetworkRendererFactory::FlowNetworkRendererFactory()
  : RendererFactory("FLOWNETWORK_LAYER_RENDERER")
{
}


