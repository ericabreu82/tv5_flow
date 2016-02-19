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
\file fiocruz/src/fiocruz/KernelInterpolationAlgorithms.cpp

\brief This class represents a set of Kernel Interpolation Algorithms
*/

#include "KernelInterpolationAlgorithms.h"

te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::KernelInterpolationAlgorithms(const KD_ADAPTATIVE_TREE& adaptativeTree)
  : m_tree(adaptativeTree)
  , m_pi(3.14159265358979323846)
{

}

te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::~KernelInterpolationAlgorithms()
{

}

//! Fills the nearest neighbour vector with default values
void te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::fillNNVector(std::vector<te::gm::Point>& report, size_t numberOfNeighbors) const
{
  for (unsigned int i = 0; i < numberOfNeighbors; ++i)
  {
    te::gm::Point point(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    report.push_back(point);
  }
}

double te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::TeKernelQuartic(double tau, double distance, double intensity)
{
  if (distance > tau)
    return 0.0;

  return intensity * (3.0 / (tau * tau * m_pi)) *
    pow(1 - ((distance * distance) / (tau * tau)), 2.0);
}

double te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::TeKernelNormal(double tau, double distance, double intensity)
{
  if (distance > tau)
    return 0.0;

  return intensity * (1.0 / (tau * tau * 2 * m_pi)) *
    exp(-1.0 * (distance * distance) / (2 * tau * tau));
}

double te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::TeKernelUniform(double tau, double distance, double intensity)
{
  if (distance > tau)
    return 0.0;

  return intensity;
}

double te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::TeKernelTriangular(double tau, double distance, double intensity)
{
  if (distance > tau)
    return 0.0;

  return intensity * (1.0 - 1.0 / tau) * distance;
}

double te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::TeKernelNegExponential(double tau, double distance, double intensity)
{
  if (distance > tau)
    return 0.0;

  return intensity * exp(-3.0 * distance);
}

double te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::distWeightAvgNearestNeighbor(const te::gm::Coord2D& coord, size_t numberOfNeighbors, const te::sa::KernelFunctionType& method)
{
  te::gm::Point refPoint(coord.getX(), coord.getY());

  std::vector<te::gm::Point> reportItem;
  std::vector<double> sqrDists;

  fillNNVector(reportItem, numberOfNeighbors);

  m_tree.nearestNeighborSearch(coord, reportItem, sqrDists, numberOfNeighbors);

  double adaptativeRatio = 0.;
  for (unsigned int i = 0; i < sqrDists.size(); ++i)
  {
    if (sqrDists[i] > adaptativeRatio)
    {
      adaptativeRatio = sqrDists[i];
    }
  }

  te::gm::Envelope box(coord.getX() - adaptativeRatio, coord.getY() - adaptativeRatio, coord.getX() + adaptativeRatio, coord.getY() + adaptativeRatio);

  std::vector<KD_ADAPTATIVE_NODE*> report;
  m_tree.search(box, report);

  size_t numberOfNodes = report.size();

  double value = 0.;

  for (unsigned int i = 0; i < numberOfNodes; ++i)
  {
    size_t nodeSize = report[i]->getData().size();

    for (unsigned int j = 0; j < nodeSize; ++j)
    {
      if (box.intersects(*report[i]->getData()[j].getMBR()) == true)
      {
        double distance = refPoint.distance(&report[i]->getData()[j]);

        if (method == te::sa::Quartic)
        {
          value += TeKernelQuartic(adaptativeRatio, distance, report[i]->getData()[j].getM());
        }
        else if (method == te::sa::Normal)
        {
          value += TeKernelNormal(adaptativeRatio, distance, report[i]->getData()[j].getM());
        }
        else if (method == te::sa::Uniform)
        {
          value += TeKernelUniform(adaptativeRatio, distance, report[i]->getData()[j].getM());
        }
        else if (method == te::sa::Triangular)
        {
          value += TeKernelTriangular(adaptativeRatio, distance, report[i]->getData()[j].getM());
        }
        else if (method == te::sa::Negative_Exp)
        {
          value += TeKernelNegExponential(adaptativeRatio, distance, report[i]->getData()[j].getM());
        }
      }
    }
  }

  return value;
}

//! Distance Weight Average of Elements in Box. If an error occur returns -TeMAXFLOAT
double te::qt::plugins::fiocruz::KernelInterpolationAlgorithms::boxDistWeightAvg(const te::gm::Coord2D& coord, const te::gm::Envelope& box, const const te::sa::KernelFunctionType& method)
{
  te::gm::Point refPoint(coord.getX(), coord.getY());

  std::vector<KD_ADAPTATIVE_NODE*> report;

  m_tree.search(box, report);

  size_t numberOfNodes = report.size();

  double value = 0.;

  for (unsigned int i = 0; i < numberOfNodes; ++i)
  {
    size_t nodeSize = report[i]->getData().size();

    for (unsigned int j = 0; j < nodeSize; ++j)
    {
      double ratio = box.getHeight() / 2;

      if (box.intersects(*report[i]->getData()[j].getMBR()) == true)
      {
        double distance = refPoint.distance(&report[i]->getData()[j]);

        if (method == te::sa::Quartic)
        {
          value += TeKernelQuartic(ratio, distance, report[i]->getData()[j].getM());
        }
        else if (method == te::sa::Normal)
        {
          value += TeKernelNormal(ratio, distance, report[i]->getData()[j].getM());
        }
        else if (method == te::sa::Uniform)
        {
          value += TeKernelUniform(ratio, distance, report[i]->getData()[j].getM());
        }
        else if (method == te::sa::Triangular)
        {
          value += TeKernelTriangular(ratio, distance, report[i]->getData()[j].getM());
        }
        else if (method == te::sa::Negative_Exp)
        {
          value += TeKernelNegExponential(ratio, distance, report[i]->getData()[j].getM());
        }
      }
    }
  }

  return value;
}