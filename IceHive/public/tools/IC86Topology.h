/**
 * \file IC86Topology.h
 *
 * (c) 2013 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author mzoll <marcel.zoll@icecube.wisc.edu>
 *
 * Some topological functions useful for the IC86 detector configuration
 */

#ifndef IC86TOPOLOGY_H
#define IC86TOPOLOGY_H

#include "icetray/OMKey.h"

///Some topological functions useful for the IC86 detector configuration
namespace IC86Topology{
  /// Is this a DOM in IceCube?
  bool IsICDOM(const OMKey& omkey);
  
  /// Is this a DOM in DeepCore?
  bool IsDCDOM(const OMKey& omkey);
  
  /// Is this DOM in the DeepCore Veto layer?
  bool IsVetoCapDOM(const OMKey& omkey);
  
  /// Is this DOM in the denser populated DeepCore region?
  bool IsDCFidDOM(const OMKey& omkey);
  
  /// Is this DOM in the denser populated  DeepCore-Veto region?
  bool IsVetoCapFidDOM(const OMKey& omkey);
  
  /// Is this DOM in any dense populated region?
  bool IsDCVetoCapFidDOM(const OMKey& omkey);
  
  /// Is this DOM in IceTop?
  bool IsIceTopDOM(const OMKey& omkey);

  // just strings
  /// Is this string a in the regular IceCube array?
  bool IsICString(const OMKey& omkey);
  
  /// Is this string in the DeepCore array?
  bool IsDCString(const OMKey& omkey);
  
  /// Is this string in the Pingu (DeepCore infill) array
  bool IsPinguString(const OMKey& omkey) ;
};

#endif //IC86TOPOLOGY_H
