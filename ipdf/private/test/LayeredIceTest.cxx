/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Test depth-dependent PDF.
*/

#include <I3Test.h>
#include <sstream>
#include <iomanip>
#include <boost/make_shared.hpp>
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/LayeredIce.h"
#include "ipdf/Pandel/GConvolutePEP.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Simple/SimpleOmReceiver.h"
#include "ipdf/Simple/SimplePEHit.h"
#include "ipdf/Simple/SimpleHitOm.h"
#include "ipdf/Simple/SimpleDetectorResponse.h"

#include "ipdf/I3/I3MediumPropertiesFile.h"
#define ICE_PROPERTIES "/phys-services/resources/ice_tables/iceProperties.dat"

using std::cout;
using std::endl;

using namespace IPDF::Pandel;
using IPDF::Likelihood::SPE1st;
using IPDF::InfiniteMuon;
using IPDF::SimpleOmReceiver;
using IPDF::SimplePEHit;
using IPDF::SimpleHitOm;
using IPDF::SimpleDetectorResponse;

typedef GConvolutePEP<IPDF::Pandel::H2> GCL;
typedef GConvolutePEP<LayeredIce<> > GCI3M;

TEST_GROUP(LayeredIceTest)

TEST(ctor_bulk)
  {
    // Test construct/destruct
    boost::shared_ptr<IPDF::Pandel::H2> ice(new IPDF::Pandel::H2);
    GCL* bulk( new GCL(ice) );
    delete bulk;
  }

TEST(copy_bulk)
  {
    IPDF::Pandel::H2 ice;
    IPDF::Pandel::H2 copy = ice;
    IPDF::Pandel::H2* nice = new IPDF::Pandel::H2;
    IPDF::Pandel::H2 ncopy = *nice;
    delete nice;

    // use the copy
    copy=copy;
    ncopy=ncopy;
  }

TEST(copy_layered)
  {
    std::string works(getenv("I3_SRC"));
    std::string icefile=works+ICE_PROPERTIES;
    LayeredIce<> medium(icefile);
    for(int i=0; i<10; ++i) {
      LayeredIce<> copy = medium;  // copy ctor
      medium = copy;               // asignment operator (op=)
      medium = medium;             // self asignment
    }
  }

TEST(copy_bulk_gc)
  {
    GCL bulk;
    GCL copy = bulk;  // copy ctor
    bulk = copy;      // asignment operator (op=)
    bulk = bulk;      // self asignment
    GCL nbulk;
    GCL ncopy = nbulk;
  }

TEST(copy_layered_gc)
  {
    std::string works(getenv("I3_SRC"));
    std::string icefile=works+ICE_PROPERTIES;
    boost::shared_ptr<LayeredIce<> > medium(new LayeredIce<>(icefile));

    GCI3M layered( medium );

    for(int i=0; i<10; ++i) {
      GCI3M copy = layered;  // copy ctor
      layered = copy;        // asignment operator (op=)
      layered = layered;     // self asignment
    }
  }

TEST(getPdf)
  /// Test basic getPdf()
  {
    boost::shared_ptr<GCL> bulk( new GCL() );

    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=5., omy=0., omz=0.;
    const SimpleOmReceiver omr(omx,omy,omz,1.,1.);

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );

    double pdf = bulk->getPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.01215309996092021,1.e-13,
	"Unexpected result of GConvolute PDF");

    delete sInfMuon;
  }

TEST(spe1st)
  {
    boost::shared_ptr<GCL> bulk( new GCL() );

    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=5., omy=0., omz=0.;
    const SimpleOmReceiver omr(omx,omy,omz,1.,1.);

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit* pehit = new SimplePEHit( omr, le );

    SimpleHitOm* hitom = new SimpleHitOm( omr, pehit );

    SPE1st<GCL> spe( bulk );

    double like = spe.getLikelihood(*hitom,*sInfMuon);
    ENSURE_DISTANCE(like,0.01215309996092021,1.e-13,
	"Unexpected result of SPE with GConvoluted PDF");

    delete sInfMuon;
    // bulk destroyed by SPE1st<GCL> dtor
  }

TEST(getIntPdf)
{
    boost::shared_ptr<GCL> bulk( new GCL() );
    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=5., omy=0., omz=0.;
    const SimpleOmReceiver omr(omx,omy,omz,1.,1.);
    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit* pehit = new SimplePEHit( omr, le );
    
    /// @todo Improve implementation of PEPBase::getIntPdf()
    double intpdf = bulk->getIntPdf(*pehit,*sInfMuon);
    ENSURE_DISTANCE(intpdf,0.3197081433668546,1.e-13,
	"Unexpected result of GConvolute getIntPdf");
}

/*
  // [-800:800] m
OLD (with I3MediumService (TH2D) interpolation)
const double layered_results[17] = {
  0.01196264289706189,
  0.01196264289687183,
  0.01196264289690311,
  0.01202308440799091,
  0.0117984232608102,
  0.003044878484923868,
  0.01201256036253426,
  0.01204461760449751,
  0.0103956193991003,
  0.01070092521901064,
  0.01197938584744172,
  0.00906319000731971,
  0.002566476103023136,
  7.941684748102526e-05,
  7.442791779430102e-08,
  4.571005064338297e-09,
  4.571005064338297e-09
};
*/

// NEW (with IceTableInterpolator (GSL c-splines) interpolation)
const double layered_results[17] = {
    0.01196427709789878,
    0.0119642770977149,
    0.01196427709774465,
    0.01191653594785575,
    0.01162703970850807,
    0.003271986372457509,
    0.01199789673343277,
    0.01204330272277633,
    0.01040468802508322,
    0.01072595363242235,
    0.01198962061318583,
    0.008806701630283608,
    0.002821207876238065,
    7.795656388968199e-05,
    7.568289653818383e-08,
    4.51409868047221e-09,
    4.514098687129389e-09
};


TEST(depth_dependent)
  {
    std::string works(getenv("I3_SRC"));
    std::string icefile=works+ICE_PROPERTIES;
    LayeredIce<IPDF::Pandel::H2>::pointer_type medium( boost::make_shared< LayeredIce<IPDF::Pandel::H2> >(icefile) );

    GCI3M* layered( new GCI3M(medium) );

    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    for(int i=0; i<17; ++i) {
      double depth = 100.*(i-8);
      const SimpleOmReceiver omr(0.1,0.,depth,1.,1.);
      const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
      double le( 10. + egeom.geometricalTime() );
      SimplePEHit pehit( omr, le );

      double pdf = layered->getPdf(pehit,*sInfMuon);

      std::string error("Unexpected result of GConvoluted PDF at depth ");
      std::ostringstream numbers;
      numbers<<depth<<" is "<<std::setprecision(20)<<pdf;
      numbers<< " (should be "<<std::setprecision(20)<<layered_results[i]
             << ")";
      error+=numbers.str();
      long double ratio = pdf/layered_results[i];
      // log_notice("%s" , error.c_str());
      ENSURE_DISTANCE(ratio,1.0L,1.e-8,error.c_str());
      // ENSURE_DISTANCE(ratio,1.0L,1.e-9,error.c_str()); /// this fails on RHEL4-i686
    }

    delete layered;
    // medium destroyed in GCI3M dtor
    delete sInfMuon;
  }
