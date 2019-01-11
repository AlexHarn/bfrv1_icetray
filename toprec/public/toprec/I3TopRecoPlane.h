/**
 * Copyright (C) 2004
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file I3TopRecoPlane.h
 * @version $Rev: 29961 $
 * @date $Date$
 * @author $Author: csong $
 */

#ifndef __I3TOPRECOPLANE_H_
#define __I3TOPRECOPLANE_H_

#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3Particle.h"
#include "recclasses/I3TopRecoPlaneFitParams.h"

class TTopRecoShower;

/**
   @class I3TopRecoPlane 
   @brief Takes station responses and guesses the shower front by
   chisquare fitting a plane
 
  algorithm:
  minimise (analytically)
  chi^2 = sum (w_i [c(t_i - t_0) - n * X_i]^2)
  w.r.t. t_0 (some time) and n, the direction. t_i, X_i are hit's
  times and position. Direction n is obtained by solving

  sum (w_i (n.X_i) X_i) - (n.sum (w_i X_i))(sum (X_i)) = c sum (w_i
  t_i X_i) - c (sum (w_i t_i))(sum (w_i X_i))

  see source code for details.
*/
class I3TopRecoPlane : public I3ConditionalModule 
{
 private:
  
  bool fVerbose; // verbosity
  std::string fEventHeader; // event header name
  std::string fArrayHitName; // the name of the array hit in the map

  // some default values for the above
  static const bool DEFAULT_VERBOSE;
  static const std::string DEFAULT_DATA_READOUT_LABEL;
  static const std::string DEFAULT_ARRAYHITNAME;
  static const std::string DEFAULT_SHOWERPLANENAME;
  static const bool DEFAULT_USE_HITS;
  static const int DEFAULT_TRIGGER;
  static const double DEFAULT_RESIDUALCUT;
  static const double DEFAULT_THRESHOLD;

  static const std::string EVENT_HEADER_TAG;
  static const std::string VERBOSE_TAG;
  static const std::string DATA_READOUT_TAG;
  static const std::string SHOWERPLANENAME_TAG;
  static const std::string ARRAYHITNAME_TAG;
  static const std::string USE_HITS_TAG;
  static const std::string TRIGGER_TAG;
  static const std::string RESIDUALCUT_TAG;
  static const std::string THRESHOLD_TAG;

  static const std::string EVENT_HEADER_DESCRIPTION;
  static const std::string VERBOSE_DESCRIPTION;
  static const std::string DATA_READOUT_DESCRIPTION;
  static const std::string SHOWERPLANENAME_DESCRIPTION;
  static const std::string ARRAYHITNAME_DESCRIPTION;
  static const std::string USE_HITS_DESCRIPTION;
  static const std::string TRIGGER_DESCRIPTION;
  static const std::string RESIDUALCUT_DESCRIPTION;
  static const std::string THRESHOLD_DESCRIPTION;  

  // some define like statements which allow the compiler to check if
  // we made a mistake inside the I3Stream calls
  static const std::string PHYSICS_STREAM_NAME;
  static const std::string GEOMETRY_STREAM_NAME;
  static const std::string PHYSICS_STREAM_TITLE;
  static const std::string GEOMETRY_STREAM_TITLE;
  static const std::string OUTBOX_NAME;
  static const std::string INBOX_NAME;

 public:

  /**
   * The constructor getting a context
   */
  I3TopRecoPlane (const I3Context& ctx);

  /**
   * The destructor
   */
  ~I3TopRecoPlane ();

  /**
   * Configure the guess
   */
  void Configure();

  /**
   * Get a physics event
   */
  void Physics (I3FramePtr frame);

 private:

  std::string fShowerPlaneName;
  std::string fDataReadoutLabel;

  unsigned int fTrigger;

  bool EvaluateDirection(TTopRecoShower * inputShower,
                         I3ParticlePtr& shower_plane,
                         I3TopRecoPlaneFitParamsPtr& plane_params); 

  void OutputEmptyParams(I3FramePtr frame, std::string outShowerName);

  SET_LOGGER("I3TopRecoPlane"); 
};

#endif
