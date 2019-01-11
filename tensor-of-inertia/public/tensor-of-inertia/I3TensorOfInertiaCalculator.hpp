#ifndef I3TENSOR_OF_INERTIA_CALCULATOR_H
#define I3TENSOR_OF_INERTIA_CALCULATOR_H
/**
 * copyright  (C) 2006
 * the icecube collaboration
 * $Id: I3TensorOfInertiaCalculator.h 18552 2006-04-19 15:45:25Z grullon $
 *
 * @file I3TensorOfInertiaCalculator.hpp
 * @version $Revision: 1.3 $
 * @date $Date: 2006-04-19 10:45:25 -0500 (Wed, 19 Apr 2006) $
 */

#include <boost/tuple/tuple.hpp>
#include <dataclasses/I3Matrix.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/geometry/I3OMGeo.h>
#include <dataclasses/I3Position.h>

/**
 * This class contains a series of functions to calculate the tensor of inertia and 
 * related quantities.  With this, you can calculate the inertia tensor of the hits, 
 * diagonalize the inertia tensor, and correct the direction of the longest eigenvector.
 */

typedef boost::tuples::tuple<std::vector<double>,std::vector<double>,std::vector<double> > eval_tuple_t;

class I3TensorOfInertiaCalculator {

 private:
  double ampWeight_;
  
 public:
  
  I3TensorOfInertiaCalculator(double ampWeight) :
    ampWeight_(ampWeight)
  {}
  
  /**
   *Calculates the Tensor of Inertia
   */
  I3Matrix CalculateTOI(I3RecoPulseSeriesMapConstPtr pulse_series, 
                        const I3OMGeoMap& om_geo, 
                        const I3Position& cogPosition);  
  /**
   * Corrects the direction of a reconstructed particle.
   */  
  int CorrectDirection(I3RecoPulseSeriesMapConstPtr pulse_series, 
                       const I3OMGeoMap& om_geo,                       
                       const I3Position& cog, 
                       const std::vector<double>& minevect);
  
  /**
   * Diagonalizes the Tensor of Inertia
   */
  eval_tuple_t DiagonalizeTOI(I3Matrix& inertiatensor, 
                              double& mineval, 
                              double& eval2, 
                              double& eval3, 
                              double& evalratio);

};

#endif
