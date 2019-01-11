/**
 * Copyright (C) 2007
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file LateralFitFunctions.h
 * @version $Rev$
 * @date $Date$
 * @author $Author$
 */

#ifndef __LATERALFITFUNCTIONS_H_
#define __LATERALFITFUNCTIONS_H_

#include <cstdlib>
#include <vector>

#include "icetray/I3Units.h"
#include "icetray/I3TrayHeaders.h"
#include "recclasses/I3LaputopParams.h"   // <--- contains the curvature functions


namespace LateralFitFunctions {

// constants for the fit function; partly have to be accesible to the
// customer, therefore in the header file
const double R0_PARAM = 125.0 * I3Units::m;  // parameter of the fit
const double X0 = log10(R0_PARAM/I3Units::m);  // R_0 as it is needed in the DLP fit
__attribute__((unused))
static double KAPPA = 0.30264;                // constant of the DLP function (set to zero for powerlaw)

// available LDFs
double top_ldf_nkg(double r, double *par);
double top_ldf_dlp(double r, double *par);
double top_ldf_sigma(double x, double logq);

// available time curvature functions
double top_curv_kislat(double r, double *par);

// available time curvature likelihoods
double top_curv_kislat_llh(double r, double deltaT, double *par);

// for the energy estimation
double estimate_firstguess_energy(double lsref, double szenith, double conversion_radius); // SAF formula
double estimate_energy(double lsref, double szenith, double conversion_radius); // correction to that
std::vector<double> E_parameters(double ref_radius);

} // end namespace

#endif
