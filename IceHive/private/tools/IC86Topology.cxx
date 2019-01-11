/**
 * \file IC86Topology.cxx
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@icecube.wisc.edu>
 *
 * Some topological functions useful for the IC86 detctor configuration
 * NOTE file in work progress
 */

#include "tools/IC86Topology.h"

bool IC86Topology::IsICDOM(const OMKey& omkey) {
  const unsigned int str = omkey.GetString();
  const unsigned int om = omkey.GetOM();
  return ((1<=str && str<=78) && (1<=om && om<=60));
}


bool IC86Topology::IsDCDOM(const OMKey& omkey) {
  const unsigned int str = omkey.GetString();
  const unsigned int om = omkey.GetOM();
  return ((79<=str && str<=86) && (11<=om && om<=60));
}

bool IC86Topology::IsVetoCapDOM(const OMKey& omkey) {
  const unsigned int str = omkey.GetString();
  const unsigned int om = omkey.GetOM();
  return ((79<=str && str<=86) && (1<=om && om<=10));
}

bool IC86Topology::IsDCFidDOM(const OMKey& omkey) {
  const unsigned int str = omkey.GetString();
  const unsigned int om = omkey.GetOM();
  return (((79<=str && str<=86) && (11<=om && om<=60))
    || ((str==36 || str==45 || str==46 || str==35 || str==37 || str==26 || str==27) && (40<=om && om<=60)));
}

bool IC86Topology::IsVetoCapFidDOM(const OMKey& omkey) {
  const unsigned int str = omkey.GetString();
  const unsigned int om = omkey.GetOM();
  return (((79<=str && str<=86) && (1<=om && om<=10)) 
    || ((str==36 || str==45 || str==46 || str==35 || str==37 || str==26 || str==27) && (20<=om && om<=25)));
}

bool IC86Topology::IsDCVetoCapFidDOM(const OMKey& omkey) {
  return (IsVetoCapFidDOM(omkey) || IsDCFidDOM(omkey));
}

bool IC86Topology::IsIceTopDOM(const OMKey& omkey) {
  const unsigned int om = omkey.GetOM();
  return (61 <= om && om <= 64);
}

// =========just strings =====================
bool IC86Topology::IsDCString(const OMKey& omkey) {
  const unsigned int str = omkey.GetString();
  return (81<=str && str<=86);
}

bool IC86Topology::IsPinguString(const OMKey& omkey) {
  const unsigned int str = omkey.GetString();
  return (str==79 || str==80);
}

bool IC86Topology::IsICString(const OMKey& omkey) {
  return !(IsPinguString(omkey) || IsDCString(omkey));
}
