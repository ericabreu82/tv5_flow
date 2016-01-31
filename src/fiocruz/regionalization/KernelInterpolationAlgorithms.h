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
\file fiocruz/src/fiocruz/KernelInterpolationAlgorithms.h

\brief This filerepresents a set of Kernel Interpolation Algorithms
*/

#ifndef __FIOCRUZ_INTERNAL_REGIONALIZATION_KERNELINTERPOLATIONALGORITMS_H
#define __FIOCRUZ_INTERNAL_REGIONALIZATION_KERNELINTERPOLATIONALGORITMS_H

#include "terralib/geometry/Coord2D.h"
#include "terralib/geometry/Point.h"
#include "terralib/sam/kdtree.h"

#include <map>
#include <string>

namespace te
{
  namespace qt
  {
    namespace plugins
    {
      namespace fiocruz
      {
        /*! \enum TeKernelInterpolationAlgorithm

        Algorithms of interpolation, may be:
        - TeDistWeightAvgInterpolation       Interpolation with weight average (inverse of square distance or other) of k-nearest neighbors values
        - TeDistWeightAvgInBoxInterpolation  Interpolation with weight average of elements in box
        */
        enum KernelInterpolationAlgorithm { TeDistWeightAvgInterpolation, TeDistWeightAvgInBoxInterpolation };

        /*! \enum TeKernelInterpolationMethod

        Methods of interpolation, may be:
        - TeQuarticKernelMethod
        - TeNormalKernelMethod
        - TeUniformKernelMethod
        - TeTriangularKernelMethod
        - TeNegExpKernelMethod
        */
        enum KernelInterpolationMethod { TeQuarticKernelMethod, TeNormalKernelMethod, TeUniformKernelMethod, TeTriangularKernelMethod, TeNegExpKernelMethod };


        /*!
        \class KernelInterpolationAlgorithms

        \brief This class represents a set of Kernel Interpolation Algorithms

        */
        class KernelInterpolationAlgorithms
        {

        public:

          typedef te::sam::kdtree::AdaptativeNode<te::gm::Coord2D, std::vector<te::gm::Point>, te::gm::Point> KD_ADAPTATIVE_NODE;
          typedef te::sam::kdtree::AdaptativeIndex<KD_ADAPTATIVE_NODE> KD_ADAPTATIVE_TREE;

          KernelInterpolationAlgorithms(const KD_ADAPTATIVE_TREE& adaptativeTree);

          virtual ~KernelInterpolationAlgorithms();

          void fillNNVector(std::vector<te::gm::Point>& report, size_t numberOfNeighbors) const;

          double TeKernelQuartic(double tau, double distance, double intensity);

          double TeKernelNormal(double tau, double distance, double intensity);

          double TeKernelUniform(double tau, double distance, double intensity);

          double TeKernelTriangular(double tau, double distance, double intensity);

          double TeKernelNegExponential(double tau, double distance, double intensity);

          //! Weight Average of Nearest Neighbors. If an error occur returns -TeMAXFLOAT
          double distWeightAvgNearestNeighbor(const te::gm::Coord2D& coord, size_t numberOfNeighbors, const KernelInterpolationMethod& method);

          double boxDistWeightAvg(const te::gm::Coord2D& coord, const te::gm::Envelope& box, const KernelInterpolationMethod& method);

        protected:

          const KD_ADAPTATIVE_TREE& m_tree;
          const double m_pi;

        };
      }
    }
  }
}

#endif //__FIOCRUZ_INTERNAL_REGIONALIZATION_KERNELINTERPOLATIONALGORITMS_H
