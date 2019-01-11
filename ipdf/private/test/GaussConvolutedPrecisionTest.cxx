/**
    copyright  (C) 2008
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author David Boersma
 *
 *  @brief Precision tests for Gauss convoluted Pandel
*/

#include <iomanip>
#include <cassert>
#include <math.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_hyperg.h>

#include <I3Test.h>
#include "ipdf/AllOMsLikelihood.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/GaussConvolutedPEP.h"
#include "ipdf/Pandel/ConvolutedHyperg.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Hypotheses/PointCascade.h"
#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/I3/I3PEHit.h"
#include "ipdf/I3/I3HitOm.h"

using std::cout;
using std::endl;

using IPDF::AllOMsLikelihood;
using IPDF::InfiniteMuon;
using IPDF::PointCascade;
using IPDF::I3OmReceiver;
using IPDF::I3PEHit;
using IPDF::I3HitOm;

using namespace IPDF::Pandel;

typedef GaussConvolutedPEP<H2> GPandel;

TEST_GROUP(GaussConvolutedPrecisionTest)

// clunky trick to compute "perpendicular distance" from ksi at omz=0.
template< typename EmissionHypothesis, typename IceModel>
double ksi2perpd( double ksi, typename boost::shared_ptr<IceModel> iceptr ){
    return 0.;
}

template <>
double ksi2perpd<InfiniteMuon,IPDF::Pandel::H2>( double ksi, boost::shared_ptr<IPDF::Pandel::H2> iceptr ){
    double propd = ksi / iceptr->InvEffScattLength(0.);
    double perpd = (propd - IPDF::Pandel::H2::TD_DIST_P0_CS0) / IPDF::Pandel::H2::TD_DIST_P1;
    return perpd;
}

template <>
double ksi2perpd<PointCascade,IPDF::Pandel::H2>( double ksi, boost::shared_ptr<IPDF::Pandel::H2> iceptr ){
    double perpd = ksi / iceptr->InvEffScattLength(0.);
    return perpd;
}

template<typename IceModel,typename EmissionHypothesis>
double GPandelValue( double tres, double ksi, 
                   typename boost::shared_ptr<IceModel> iceptr,
                   double jitter ){
    GPandel gpandel(iceptr,jitter);
    double t=0.;
    double e0=1000.; // 1TeV
    double omx=0., omy=0., omz=0.; // OM at origin
    double dx=1., dy=0., dz=0.; // horizontal track
    double nx=0., ny=1., nz=0.; // normal (for placement of track relative to OM)
    const I3OmReceiver omr(omx,omy,omz,-1.,jitter,1.0);

    // this should work for both muons and cascades
    double perpd = ksi2perpd<EmissionHypothesis,IceModel>(ksi,iceptr);
    if ( perpd < 0 ){
        log_warn("negative perpd");
        return 0.;
    }
    double px = omx - perpd*nx;
    double py = omy - perpd*ny;
    double pz = omz - perpd*nz;
    const I3Position pos(px,py,pz);
    const I3Direction dir(dx,dy,dz);
    EmissionHypothesis emitter( pos, dir, e0, t );

    typename EmissionHypothesis::template EmissionGeometry<double> geo(emitter,omr);
    double le( tres + geo.geometricalTime() );
    I3PEHit hit( omr, le );

    return gpandel.getPdf(hit,emitter);
}

template<typename IceModel,typename EmissionHypothesis>
void BoundaryCheck( double tres, double dtres, double ksi, double dksi,
                    typename boost::shared_ptr<IceModel> iceptr, double jitter,
                    double precision, std::string chkname ){

    static int checkno = 0;
    static int checknull = 0;
    ++checkno;

    double ksiA = ksi + dksi;
    double ksiB = ksi - dksi;
    double tresA = tres + dtres;
    double tresB = tres - dtres;
    assert((ksiA>ksiB)||(tresA>tresB));
    double pdfA = GPandelValue<IceModel,EmissionHypothesis>(
            tresA, ksiA, iceptr, jitter );
    double pdfB = GPandelValue<IceModel,EmissionHypothesis>(
            tresB, ksiB, iceptr, jitter );
    double rij = 0.;
    if ( pdfA+pdfB== 0. ){
        ++checknull;
    } else {
        rij = 2.0*(pdfA-pdfB)/(pdfA+pdfB+1e-9);
    }
    if (std::abs(rij)>precision){
        log_error( "tA=%.1f, tB=%.1f, "
                   "ksiA=%.1f, ksiB=%.1f, "
                   "pdfA=%g, pdfB=%g, "
                   "diff=%g rij=%g allowed=%g",
                   tresA,tresB,ksiA,ksiB,
                   pdfA,pdfB,pdfA-pdfB,rij,precision);
        log_error( "(%d,%d) %s failed continuity",
                   checkno,checknull, chkname.c_str() );
    }
    ENSURE_DISTANCE(rij,0.,precision,"continuity error (rerun with logging output for specs)");

    log_debug( "OK (%d,%d) %s:",
               checkno, checknull, chkname.c_str());
    log_debug( "tA=%.1f, tB=%.1f, "
               "ksiA=%.1f, ksiB=%.1f, "
               "pdfA=%g, pdfB=%g, "
               "diff=%g rij=%g allowed=%g",
               tresA,tresB,ksiA,ksiB,
               pdfA,pdfB,pdfA-pdfB,rij,precision);

}

template<typename IceModel>
void BoundaryCheckLoop(
        double t0,double ksi0,double t1,double ksi1,
        double dtres, double dksi, int npoints,
        typename boost::shared_ptr<IceModel> iceptr, double jitter,
        const std::string &chkname, double precision ){

    // silly checks
    assert(npoints>1); // at least two points
    assert((ksi1>ksi0)||(t1>t0)); // not both constant
    assert((dtres>0)||(dksi>0)); // not both zero
    assert((dtres>0)^(ksi1==ksi0)); // infinitesimal step perp. to boundary
    assert((dksi>0)^(t1==t0)); // infinitesimal step perp. to boundary


    for ( int i=0; i<npoints; i++ ){

        double tres = (i*t1 + (npoints-1-i)*t0)/(npoints-1);
        double ksi = (i*ksi1 + (npoints-1-i)*ksi0)/(npoints-1);

        log_debug( "checking tres=%g dtres=%g ksi=%g dksi=%g",
                   tres, dtres, ksi, dksi );

        BoundaryCheck<IceModel,InfiniteMuon>(
                tres, dtres, ksi, dksi,
                iceptr, jitter, precision, chkname + " for muons" );

        BoundaryCheck<IceModel,PointCascade>(
                tres, dtres, ksi, dksi,
                iceptr, jitter, precision, chkname + " for pointcascades" );

    }
}

TEST(Continuity){
    boost::shared_ptr<IPDF::Pandel::H2> iceptr(new IPDF::Pandel::H2);
    const double rho(1./iceptr->TauScale(0.)+IPDF::IPdfConstants::C_ICE_G*iceptr->Absorptivity(0.));

    for ( double sigma = 3.; sigma < 22.; sigma += 3. ){
        const double biggestt = +3500.;
        double bigt = +30.*sigma;
        double smallt = -5*sigma;
        const double smallestt = -150.;
        double t34 = rho * sigma * sigma; // tres for which rho*sigma-tres/sigma=0
        const int npoints = 5;
        const double ksi0 = 0.0;
        const double ksi1 = 1.;
        const double ksi5 = 5.;
        const double ksi30 = 30.;
        double dt = 0.;
        double dksi = 0.;
        double delta = 0.000000001;

        // In Astroparticle Physics 28 (2007) 456-462, December 2007,
        // estimates are given for the differences between regions.
        // See Figure 3 in that paper.
        // On the boundaries of region 5 the relative difference should
        // be better than 1.e-3, between all other regions it should be
        // better than 1.e-6.
        const double eps = 1.e-4;
        const double eps5 = 3.e-2;

        log_debug("boundary 1 with 2 and 3: sigma=%g tres=%g ksi=0-1",
                  sigma, bigt );
        std::ostringstream name123;
        dt = delta;
        dksi = 0.;
        name123 << "boundary region 1 with 2 and 3, sigma=" << sigma;
        BoundaryCheckLoop(bigt,ksi0+delta,bigt,ksi5-delta,dt,dksi,npoints,
                      iceptr,sigma,name123.str(),eps);

        log_debug("boundary 1 with 4, sigma=%g tres=%g ksi=1-5",
                  sigma, smallt );
        std::ostringstream name14;
        dt = delta;
        dksi = 0.;
        name14 << "boundary region 1 with 4, sigma=" << sigma;
        BoundaryCheckLoop(smallt,ksi1+delta,smallt,ksi5-delta,dt,dksi,npoints,
                      iceptr,sigma,name14.str(),eps);

        log_debug("boundary 1 with 3 and 4");
        std::ostringstream name134;
        dt = 0.;
        dksi = delta;
        name134 << "boundary region 1 with 3 and 4, sigma=" << sigma;
        BoundaryCheckLoop(smallt,ksi5,bigt,ksi5,dt,dksi,npoints,
                      iceptr,sigma,name134.str(),eps);

        log_debug("boundary 3 with 4");
        std::ostringstream name34;
        dt = delta;
        dksi = 0.;
        name34 << "boundary region 3 and 4, sigma=" << sigma;
        BoundaryCheckLoop(t34,ksi5+delta,t34,ksi30-delta,dt,dksi,npoints,
                      iceptr,sigma,name34.str(),eps);

        log_debug("boundary 2 with 3");
        std::ostringstream name23;
        dt = 0.;
        dksi = delta;
        name23 << "boundary region 2 and 3, sigma=" << sigma;
        BoundaryCheckLoop(bigt+delta,ksi1,biggestt-delta,ksi1,dt,dksi,npoints,
                      iceptr,sigma,name23.str(),eps);

        log_debug("boundary 1 with 5");
        std::ostringstream name15;
        dt = delta;
        dksi = 0.;
        name15 << "boundary region 1 with 5, sigma=" << sigma;
        BoundaryCheckLoop(smallt,ksi0,smallt,ksi1,dt,dksi,npoints,
                      iceptr,sigma,name15.str(),eps5);

        log_debug("boundary 4 with 5");
        std::ostringstream name45;
        dt = 0.;
        dksi = delta;
        name45 << "boundary region 4 with 5, sigma=" << sigma;
        BoundaryCheckLoop(smallestt,ksi1,smallt,ksi1,dt,dksi,npoints,
                      iceptr,sigma,name45.str(),eps5);

    }
}

/*
 * Compute the error on the GSL computation of the hypergeometric term
 */
double gslConvoluted1F1Diff_error(const double xi, const double eta) {

  const double xi1 = 0.5*(xi+1);
  const double xi2 = 0.5*xi;
  const double eta2 = 0.5*eta*eta;

  gsl_sf_result r1, r2;
  gsl_sf_hyperg_1F1_e(xi2, 0.5, eta2, &r1);
  gsl_sf_hyperg_1F1_e(xi1, 1.5, eta2, &r2);

  
  return fabs(r1.err/PdfMath::Gamma(xi1)) +
               fabs(eta*M_SQRT2*r2.err/PdfMath::Gamma(xi2));
}

TEST(ConvolutedHyperg) {
  // Test that the "fast" computations of the hypergeometric
  // term of the convoluted Pandel are reasonably close to the GSL version
  // Agreement is typically 1e-15, but is worse than 1e-13 in some small
  // regions due to conditioning.
  gsl_sf_result r;
  for (double eta = 1.35; eta > -14.2; eta -= 0.01) {
    for (double xi = 0.05; xi < 5.; xi += 0.01) {
      double gsl = gslConvoluted1F1Diff(xi, eta);
      double fast = fastConvolutedHyperg(xi, eta);
      double gsl_err = gslConvoluted1F1Diff_error(xi, eta);
      // Ensure there is 1e-12 agreement with GSL
      ENSURE(fabs((fast-gsl) / gsl) < 1.e-12);
      // Ensure we're near GSL's own error bound for the computation
      ENSURE(fabs(fast-gsl) < 4.*gsl_err);
    }
  }
  // Ensure gslConvolutedU matches the fast computation at the boundary
  for (double xi = 0.05; xi < 5.; xi += 0.01) {
    double gslU = gslConvolutedU(xi, 1.35);
    double fast = fastConvolutedHyperg(xi, 1.35);
    ENSURE(fabs((fast-gslU) / gslU) < 1.e-12);
  }
}
