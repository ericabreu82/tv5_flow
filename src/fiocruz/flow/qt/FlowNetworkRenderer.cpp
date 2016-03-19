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
#include <terralib/geometry/Curve.h>
#include <terralib/geometry/LinearRing.h>
#include <terralib/geometry/MultiLineString.h>
#include <terralib/geometry/Polygon.h>
#include <terralib/maptools/Canvas.h>
#include <terralib/maptools/Chart.h>
#include <terralib/maptools/MarkRendererManager.h>
#include <terralib/se/Fill.h>
#include <terralib/se/Mark.h>
#include <terralib/se/Stroke.h>
#include <terralib/se/Utils.h>
#include <terralib/srs/Config.h>

#define PATTERN_SIZE 12

te::qt::plugins::fiocruz::FlowNetworkRendererFactory* te::qt::plugins::fiocruz::FlowNetworkRendererFactory::sm_factory(0);

te::qt::plugins::fiocruz::FlowNetworkRenderer::FlowNetworkRenderer()
  : te::map::AbstractLayerRenderer()
{

}

te::qt::plugins::fiocruz::FlowNetworkRenderer::~FlowNetworkRenderer()
{
  te::common::Free(m_pointPattern, PATTERN_SIZE);
  delete m_pointMark;
}

void te::qt::plugins::fiocruz::FlowNetworkRenderer::drawFlowMultiLine(te::map::Canvas* canvas, te::gm::MultiLineString* line)
{
  assert(line);

  std::size_t size = line->getNumGeometries();

  for (size_t i = 0; i < size; ++i)
    drawFlowLine(canvas, static_cast<te::gm::LineString*>(line->getGeometryN(i)));
}

void te::qt::plugins::fiocruz::FlowNetworkRenderer::drawFlowLine(te::map::Canvas* canvas, te::gm::LineString* line)
{
  assert(line);

  const te::gm::Envelope* envelope = line->getMBR();
  int srid = line->getSRID();

  canvas->draw(line);
  int width = canvas->getWidth();
  //int arrowSize = (int)(width * 0.025); //2.5% of the width
  double arrowSize = (double)(envelope->getWidth() * 0.2); //2.5% of the width
  //envelope

  te::gm::Coord2D coordBegin(line->getX(0), line->getY(0));
  te::gm::Coord2D coordEnd(line->getX(1), line->getY(1));

  double centerX = (coordBegin.getX() + coordEnd.getX()) / 2.0;

  
  double	slopy , cosy , siny;
  
  slopy = atan2((double)(coordBegin.getY() - coordEnd.getY()), (double)(coordBegin.getX() - coordEnd.getX()));
  cosy = cos( slopy );
  siny = sin( slopy );

  //te::gm::Coord2D coord1(coordEnd.getX(), coordEnd.getY());
  //te::gm::Coord2D coord2(coordEnd.getX() - int(-arrowSize*cosy - (arrowSize / 2.0*siny)), coordEnd.getY() - int(-arrowSize*siny + (arrowSize / 2.0*cosy)));
  //te::gm::Coord2D coord3(coordEnd.getX() - int(-arrowSize*cosy + (arrowSize / 2.0*siny)), coordEnd.getY() + int(arrowSize / 2.0*cosy + arrowSize*siny));
  //te::gm::Coord2D coord4(coordEnd.getX(), coordEnd.getY());

  te::gm::Coord2D coord1(coordEnd.getX(), coordEnd.getY());
  te::gm::Coord2D coord2(coordEnd.getX() - double(-arrowSize*cosy - (arrowSize / 2.0*siny)), coordEnd.getY() - double(-arrowSize*siny + (arrowSize / 2.0*cosy)));
  te::gm::Coord2D coord3(coordEnd.getX() - double(-arrowSize*cosy + (arrowSize / 2.0*siny)), coordEnd.getY() + double(arrowSize / 2.0*cosy + arrowSize*siny));
  te::gm::Coord2D coord4(coordEnd.getX(), coordEnd.getY());

  te::gm::LinearRing* ring = new te::gm::LinearRing(4, te::gm::GeomType::LineStringType, srid);
  ring->setPoint(0, coord1.getX(), coord1.getY());
  ring->setPoint(1, coord2.getX(), coord2.getY());
  ring->setPoint(2, coord3.getX(), coord3.getY());
  ring->setPoint(3, coord4.getX(), coord4.getY());

  te::gm::Polygon pArrow(1, te::gm::GeomType::PolygonType, srid);
  pArrow.setRingN(0, ring);

  canvas->draw(&pArrow);
  /*
  QPointArray points(4);
  points[0]=QPoint(ptEnd.x(),ptEnd.y());
  points[1] = QPoint(ptEnd.x() - int(-arrowSize*cosy - (arrowSize / 2.0*siny)), ptEnd.y() - int(-arrowSize*siny + (arrowSize / 2.0*cosy)));
  points[2] = QPoint(ptEnd.x() - int(-arrowSize*cosy + (arrowSize / 2.0*siny)), ptEnd.y() + int(arrowSize / 2.0*cosy + arrowSize*siny));
  points[3]=QPoint(ptEnd.x(),ptEnd.y());
  */

  
  if (envelope->getWidth() == 0. && envelope->getHeight() == 0.)
  {
    te::gm::Point point(envelope->getCenter().getX(), envelope->getCenter().getY());
    canvas->draw(&point);
  }
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

  //set renderer mask
  //define mark selected
  te::se::Stroke* stroke = te::se::CreateStroke("#000000", "1");
  te::se::Fill* fill = te::se::CreateFill("#0000FF", "1.0");
  m_pointMark = te::se::CreateMark("circle", stroke, fill);

  m_pointPattern = te::map::MarkRendererManager::getInstance().render(m_pointMark, PATTERN_SIZE);
  canvas->setPointPattern(m_pointPattern, PATTERN_SIZE, PATTERN_SIZE);

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

    case te::gm::MultiLineStringType:
    case te::gm::MultiLineStringZType:
    case te::gm::MultiLineStringMType:
    case te::gm::MultiLineStringZMType:
      drawFlowMultiLine(canvas, static_cast<te::gm::MultiLineString*>(geom.get()));
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


