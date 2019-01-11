/**
 * \file PartialCOG.cxx
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@fysik.su.se>
 */

#include <CoincSuite/lib/PartialCOG.h>

#include "dataclasses/I3Double.h"
#include "icetray/I3Bool.h"

using namespace HitSorting;
using namespace OMKeyHash;


std::vector<I3Position> PartialCOG::BuildDomPos_cache(const I3Geometry &geo) {
  log_debug("Entering BuildDomPos_cache()");

  std::vector<I3Position> domPos_cache;
  for (uint matrice_x=0;matrice_x<(MAX_STRINGS*MAX_OMS);matrice_x++) {
    const OMKey omkey = SimpleIndex2OMKey(matrice_x);
    I3OMGeoMap::const_iterator omgeo = geo.omgeo.find(omkey);
    if (omgeo != geo.omgeo.end())
      domPos_cache.push_back(omgeo->second.position);
    else
      domPos_cache.push_back(I3Position());
  }
  log_debug("Leaving BuildDomPos_cache()");
  return domPos_cache;
}


std::pair<I3Position, double> PartialCOG::PartialCOGhit (const HitSorting::TimeOrderedHitSet& hits,
  const std::vector<I3Position>& domPos_cache,
  const uint nFrac,
  const uint useFrac,
  const bool heads_tails,
  const uint min_inc,
  const uint max_inc,
  const bool useCharge)
{
  log_debug("Entering PartialCOG()");
  if (useFrac>nFrac || nFrac==0 || useFrac==0)
    log_fatal("'useFrac' cannot be greater than 'nFrac' and both must be greater than zero"); //FIXME use here alternativeley assert
  
  double cog_time(0.);
  I3Position cog_pos(0.,0.,0.);
  
  uint n_added(0);
  double coll_charge(0.); 

  const double n_per_frac = hits.size()/nFrac;

  if (heads_tails) { //counting forward
    TimeOrderedHitSet::const_iterator hits_iter = hits.begin();
    for (uint i=0; i<(uint)((useFrac-1)*n_per_frac); i++)
      hits_iter++;
    while (hits_iter != hits.end()) {
      n_added++;
      const double& charge = useCharge ? hits_iter->charge : 1.;
      coll_charge+=charge;
      cog_time += hits_iter->time * charge;
      cog_pos.SetX(cog_pos.GetX() + ( domPos_cache[hits_iter->domIndex].GetX() * charge ));
      cog_pos.SetY(cog_pos.GetY() + ( domPos_cache[hits_iter->domIndex].GetY() * charge ));
      cog_pos.SetZ(cog_pos.GetZ() + ( domPos_cache[hits_iter->domIndex].GetZ() * charge ));
      if ((n_added>=min_inc && n_added>=(uint)n_per_frac) || n_added >=max_inc)
        break;
      hits_iter++;
    }
  }
  else { //counting backward
    TimeOrderedHitSet::const_reverse_iterator hits_riter = hits.rbegin();
    for (uint i=0; i<(uint)((nFrac-useFrac)*n_per_frac); i++)
      hits_riter++;
    while (hits_riter != hits.rend()) {
      n_added++;
      const double& charge = useCharge ? hits_riter->charge : 1.;
      coll_charge+=charge;
      cog_time += hits_riter->time * charge;
      cog_pos.SetX(cog_pos.GetX() + ( domPos_cache[hits_riter->domIndex].GetX() * charge ));
      cog_pos.SetY(cog_pos.GetY() + ( domPos_cache[hits_riter->domIndex].GetY() * charge ));
      cog_pos.SetZ(cog_pos.GetZ() + ( domPos_cache[hits_riter->domIndex].GetZ() * charge ));
      if ((n_added>=min_inc && n_added>=(uint)n_per_frac) || n_added >=max_inc)
        break;
      hits_riter++;
    }
  }

  cog_time/= coll_charge;
  cog_pos.SetX(cog_pos.GetX() / coll_charge);
  cog_pos.SetY(cog_pos.GetY() / coll_charge);
  cog_pos.SetZ(cog_pos.GetZ() / coll_charge);

  log_debug("Leaving PartialCOG");
  return std::make_pair(cog_pos, cog_time);
}


//______________________________________________________________________________
std::pair < I3Position, double > PartialCOG::PartialCOGcharged (const HitSorting::TimeOrderedHitSet& hits,
  const std::vector<I3Position>& domPos_cache,
  const uint nFrac,
  const uint useFrac,
  const bool heads_tails,
  const uint min_inc,
  const uint max_inc)
{
  log_debug("Entering PartialCOGcharged()");
  if (useFrac>nFrac || nFrac==0 || useFrac==0)
    log_error("'useFrac' cannot be greater than 'nFrac' and both must be greater than zero");

  double cog_time(0.);
  I3Position cog_pos(0.,0.,0.);

  double tot_charge(0.);
  for (TimeOrderedHitSet::const_iterator hits_iter=hits.begin(); hits_iter!=hits.end(); ++hits_iter) {
    tot_charge+=hits_iter->charge;
  }
  
  uint n_added(0);
  double coll_charge(0.);

  const double charge_per_frac = tot_charge/nFrac;

  if (heads_tails) { //counting forward
    TimeOrderedHitSet::const_iterator hits_iter = hits.begin();
    //search the right start values
    double start_charge = 0.;
    if (useFrac!=1) {
      while (start_charge<charge_per_frac*(useFrac-1)) {
        start_charge+=hits_iter->charge;
        hits_iter++;
      }
    }
    //sum up until the conditions are satisfied
    while (hits_iter!=hits.end()) {
      n_added++;
      const double& charge = hits_iter->charge;
      coll_charge+=charge;
      cog_time += hits_iter->time * charge;
      cog_pos.SetX(cog_pos.GetX() + ( domPos_cache[hits_iter->domIndex].GetX() * charge ));
      cog_pos.SetY(cog_pos.GetY() + ( domPos_cache[hits_iter->domIndex].GetY() * charge ));
      cog_pos.SetZ(cog_pos.GetZ() + ( domPos_cache[hits_iter->domIndex].GetZ() * charge ));
      if ((n_added>=min_inc && coll_charge>=charge_per_frac) || n_added >=max_inc)
        break;
      hits_iter++;
    }
  }
  else { //counting backward
    TimeOrderedHitSet::const_reverse_iterator hits_riter = hits.rbegin();
    //search the right start values
    double start_charge = 0.;
    if (useFrac!=nFrac) {
      while (start_charge<charge_per_frac*(nFrac-useFrac)) {
        start_charge+=hits_riter->charge;
        hits_riter++;
      }
    }
    //sum up until the conditions are satisfied
    while (hits_riter!=hits.rend()) {
      n_added++;
      const double& charge = hits_riter->charge;
      coll_charge+=charge;
      cog_time += hits_riter->time * charge;
      cog_pos.SetX(cog_pos.GetX() + ( domPos_cache[hits_riter->domIndex].GetX() * charge ));
      cog_pos.SetY(cog_pos.GetY() + ( domPos_cache[hits_riter->domIndex].GetY() * charge ));
      cog_pos.SetZ(cog_pos.GetZ() + ( domPos_cache[hits_riter->domIndex].GetZ() * charge ));
      if ((n_added>=min_inc && coll_charge>=charge_per_frac) || n_added >=max_inc)
        break;
      hits_riter++;
    }
  }
  //renormalize to weight
  cog_time/= coll_charge;
  cog_pos.SetX(cog_pos.GetX() / coll_charge);
  cog_pos.SetY(cog_pos.GetY() / coll_charge);
  cog_pos.SetZ(cog_pos.GetZ() / coll_charge);

  log_debug("Leaving PartialCOGcharged");
  return std::make_pair(cog_pos, cog_time);
}


//______________________________________________________________________________
std::pair <I3Position, double> PartialCOG::PartialCOGexclude(const HitSorting::TimeOrderedHitSet& hits,
  const std::vector<I3Position>& domPos_cache,
  const bool heads_tails,
  const uint n_many,
  const uint n_exclude,
  const bool useCharge)
{
  log_debug("Entering PartialCOGexclude()");
  //test: if request is bigger than mapsize
  if (n_many==0)
    log_fatal("Cannot call this function with a n_many <=0");

  if (n_exclude+n_many>hits.size())
    log_error_stream("Cannot exclude "<<n_exclude<<" and simultaniouly include "<<n_many<<" while n_hits is "<<hits.size()<<"; returning the maximum I can");

  double cog_time(0.);
  I3Position cog_pos(0.,0.,0.);

  uint n_added(0);
  double coll_charge(0.);

  if (heads_tails) { //counting forward
    TimeOrderedHitSet::const_iterator hits_iter = hits.begin();
    for (uint i=0; i<n_exclude; i++)
      hits_iter++;
    while (hits_iter != hits.end()) {
      n_added++;
      const double& charge = useCharge ? hits_iter->charge : 1.;
      coll_charge+=charge;
      cog_time += hits_iter->time * charge;
      cog_pos.SetX(cog_pos.GetX() + ( domPos_cache[hits_iter->domIndex].GetX() * charge ));
      cog_pos.SetY(cog_pos.GetY() + ( domPos_cache[hits_iter->domIndex].GetY() * charge ));
      cog_pos.SetZ(cog_pos.GetZ() + ( domPos_cache[hits_iter->domIndex].GetZ() * charge ));
      if (n_added == n_many)
        break;
      hits_iter++;
    }
  }
  else { //counting backward
    TimeOrderedHitSet::const_reverse_iterator hits_riter = hits.rbegin();
    for (uint i=0; i<n_exclude; i++)
      hits_riter++;
    while (hits_riter != hits.rend()) {
      n_added++;
      const double& charge = useCharge ? hits_riter->charge : 1.;
      coll_charge+=charge;
      cog_time += hits_riter->time * charge;
      cog_pos.SetX(cog_pos.GetX() + ( domPos_cache[hits_riter->domIndex].GetX() * charge ));
      cog_pos.SetY(cog_pos.GetY() + ( domPos_cache[hits_riter->domIndex].GetY() * charge ));
      cog_pos.SetZ(cog_pos.GetZ() + ( domPos_cache[hits_riter->domIndex].GetZ() * charge ));
      if (n_added == n_many)
        break;
      hits_riter++;
    }
  }

  cog_time/= coll_charge;
  cog_pos.SetX(cog_pos.GetX() / coll_charge);
  cog_pos.SetY(cog_pos.GetY() / coll_charge);
  cog_pos.SetZ(cog_pos.GetZ() / coll_charge);

  log_debug("Leaving PartialCOGexclude");
  return std::make_pair(cog_pos, cog_time);
}
