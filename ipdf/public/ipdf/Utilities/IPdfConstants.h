#ifndef IPdfConstants_H
#define IPdfConstants_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file IPdfConstants.h
    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/


namespace IPDF {
/**
    @brief Common repository for constants used by the IPDF 
    software.
    Constants taken from RDMC

    @todo differentiate between constants and model parameters.  
    For example, see the IceModel.h parameters.
    @todo Needs updating and units :-(
  */
namespace IPdfConstants {
/*******************************************************************/
/* Physical constants for common use                                        */
/*******************************************************************/
  const double C_VACUUM=2.997924e-1; /* m/nsec; light speed in vacuum */
  const double REZ_C_VACUUM = 1./C_VACUUM;        /* 1/c  [ ns/m ] */
  
  const double N_WATER=1.329; /* old value presently not used */
  const double N_ICE_P=1.3195; /* phase 400 nm according to amanda abs */
  const double N_ICE_G=1.35634;  /* group  400 nm according to amanda abs */
  
  const double C_WATER = C_VACUUM/N_WATER; /* m/nsec;  speed in water */
  const double REZ_C_WATER = N_WATER/C_VACUUM;/* nsec/m;  speed in water */
  const double C_ICE_P = C_VACUUM/N_ICE_P; /* m/nsec; speed in water */
  const double REZ_C_ICE_P = N_ICE_P/C_VACUUM;/* nsec/m;  speed in water */
  const double C_ICE_G = C_VACUUM/N_ICE_G; /* m/nsec;  speed in water */
  const double REZ_C_ICE_G = N_ICE_G/C_VACUUM;/* nsec/m;  speed in water */
  
  const double CS_CER_WAT = 1./N_WATER;       /* cos cerencov */
  const double CS_CER_ICE_P = 1./N_ICE_P;     /* cos cerencov */
  const double CS_CER_ICE_G = 1./N_ICE_G;     /* cos cerencov */
  
  const double CER_WAT = 0.71902928761;    /* acos(cs_cer_wat) */
  const double CER_ICE_P = 0.71076526742;  /* acos(cs_cer_ice_p) */
  const double CER_ICE_G = 0.74176356033;  /* acos(cs_cer_ice_g) */
  
  const double SN_CER_WAT = 0.65865457429;           /* sin(cer_wat) */
  const double SN_CER_ICE_P=0.65241392973;        /* sin(cer_ice_p) */
  const double SN_CER_ICE_G = 0.67558919624;        /* sin(cer_ice_g) */

  const double TG_CER_WAT = SN_CER_WAT*N_WATER; /*tangens  cer angle*/
  const double TG_CER_ICE_P = SN_CER_ICE_P*N_ICE_P; /*tangens cer angle*/
  const double TG_CER_ICE_G = SN_CER_ICE_G*N_ICE_G; /*tangens cer angle*/
  
  /* according to Kurt Woschnagg the arrival time is given by:
     t = t_0 + ( d*p + rho * koeff)/c_vac
     koeff = (n_p * n_g -1) / sqrt(n_p**2-1)
     = (n_p * n_g -1) / tan_cer_ice_p 
     in case of n_g=n_p this is the same as the old version:
     koeff = tan_cer
  */
  const double TG_CER_TKOEFF = (IPdfConstants::N_ICE_P*IPdfConstants::N_ICE_G -1.)/(IPdfConstants::SN_CER_ICE_P*IPdfConstants::N_ICE_P) ;
  
}
} // namespace IPDF

#endif // IPdfConstants_H
