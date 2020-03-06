/**
 * @brief This file contains functions pertaining to geometrical calculations
 * needed for reconstructing muons
 *
 * @copyright (C) 2018 The Icecube Collaboration
 *
 * @file geometry.cxx
 * @author Kevin Meagher
 * @date January 2018
 *
 */


#include "dataclasses/physics/I3Particle.h"
#include "rpdf/geometry.h"

namespace rpdf{
  /*
   * These different ice models were used in the AMANDA era
   * H2 is what is reported in the AMANDA reconstruction paper
   * and is the only model to be used with IceCube data
   */

  ///AMANDA ice model 0
  IceModel H0(98.,596.0,36.93,0.9045,4.249,-6.629,5.430);
  ///AMANDA ice model 1
  IceModel H1(98.,578.0,35.04,0.8682,3.615,-5.081,5.015);
  ///AMANDA ice model 2, this is what is reported in the AMANDA muon reconstruction
  ///paper. It is the default ice model in I3RecoLLH
  IceModel H2(98.,556.7,33.29,0.8395,3.094,-3.946,4.636);
  ///AMANDA ice model 3
  IceModel H3(98.,542.2,31.72,0.8106,2.683,-3.090,4.436);
  ///AMANDA ice model 4
  IceModel H4(98.,526.3,29.94,0.7790,2.139,-0.9614,4.020);

  double effective_distance(const double distance,
                              const double cos_eta,
                              const IceModel& ice_model){
    return (ice_model.P1*distance
            + ice_model.P0_CS0
            + ice_model.P0_CS1*cos_eta
            + ice_model.P0_CS2*pow(cos_eta,2));
  }

  double cherenkov_time(const double d_track,
                         const double d_approach){
    return
      (d_track + d_approach*constants::EFF_TAN_CHERENKOV)/constants::C_VACUUM;
  }

  std::pair<double,double> muon_geometry(const I3Position& om_pos,
                                          const I3Particle& track,
                                          const IceModel& ice_model)
  {
    const I3Direction dir=track.GetDir();

    //delta is the difference between the OM and the track vertex
    const I3Position delta = om_pos - track.GetPos();

    //the distance along the track is the dot product of delta and the track direction
    const double d_track      = (delta.GetX()*dir.GetX()+
                                 delta.GetY()*dir.GetY()+
                                 delta.GetZ()*dir.GetZ());

    //the distance of closest approach is the magnitude of the cross product 
    //between delta and the track direction 
    const double d_approach = hypot(hypot(delta.GetY()*dir.GetZ()-delta.GetZ()*dir.GetY(), 
                                          delta.GetZ()*dir.GetX()-delta.GetX()*dir.GetZ()), 
                                    delta.GetX()*dir.GetY()-delta.GetY()*dir.GetX()); 

    //calculate the time residual
    const double t_geo        = track.GetTime()+cherenkov_time(d_track,d_approach);

    double coseta;
    if (d_approach<=0.0){
      //if the approach distance is zero than the below calculation won't work
      //so just skip it since we know that the effective distance will also be zero
      //eff_distance=0;
      coseta=0.0;
    }else{
      //this is the distance a direct photon travels from the track to the OM
      const double d_travel    = d_approach/constants::SIN_CHERENKOV;
      //this is the distance from the track's vertex to the origin of the direct photon
      const double d_to_vertex = d_track-d_approach/constants::TAN_CHERENKOV;
      //this is the distance the direct photon travels in the z direction
      const double d_travel_z  = delta.GetZ() - d_to_vertex * track.GetDir().GetZ();
      //eta is the angle between the direct photon and the orientation of the DOM
      //so the cos(eta) is the the length in the z direction divided by the total distance
      coseta                   = d_travel_z / d_travel;
    }
    //calculate the effective distance
    const double eff_distance = effective_distance(d_approach,coseta,ice_model);

    //return the time residual and the effective distance
    return std::make_pair(t_geo,eff_distance);
  }
}//namespace rpdf

