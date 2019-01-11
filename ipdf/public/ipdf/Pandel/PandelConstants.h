#ifndef PandelConstants_H
#define PandelConstants_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file PandelConstants.h
    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Utilities/IPdfConstants.h"

namespace IPDF {
  /**
    @brief Common repository for constants used by the IPDF 
    software.

    @todo differentiate between constants and model parameters.  
    For example, see the IceModel.h parameters.

    Constants taken from RDMC
    needs updating and units :-(
  */
namespace PandelConstants {
/********************************************************************/
/* Constants specific to the Pandel PDF				    */
/********************************************************************/
/*  Constants are in GeV but rdmc uses MeV */
  const double  TD_PH_EPS_PE0 = 1.978;
  const double  TD_PH_EPS_PE1 = 0.2314e-3;
  const double  TD_PH_EPS_ORI_N0 =  1.1932;
  const double  TD_PH_EPS_ORI_POW = -0.20084;
  const double  TD_PH_DIST_A = 0.7886;
  const double  TD_PH_DIST_B = 0.1851;
  const double  TD_PH_DIST_L = 0.5967;
  const double  TD_PH_DIST_E0 = 1409.;

  const double  DIST_MAGIC=100.;
  const double  MIN_ION_ENERGY=100e3; /* 100 GeV in MeV */
  const double  MIN_ION_ENERGY_GEV=100.0; /* 100 GeV in GeV */
  const double  MAX_PE_FROM_PNH=40.;
  const double  MAX_PNH_FOR_PE=exp(-MAX_PE_FROM_PNH);

  const double  TD_DIST_PERPTOLON =1.51676;

} // namespace IPDF
} // namespace PandelConstants

#endif
