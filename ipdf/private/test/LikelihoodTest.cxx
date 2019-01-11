/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.6 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Test hooks for various likelihood classes.
    @todo Extend to more points.
*/

#include <I3Test.h>
#include "ipdf/AllOMsLikelihood.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Likelihood/SPEAll.h"
#include "ipdf/Likelihood/MPE.h"
#include "ipdf/Likelihood/PSA.h"
#include "ipdf/Simple/SimplePEHit.h"
#include "ipdf/Simple/SimpleHitOm.h"
#include "ipdf/Simple/SimpleOmReceiver.h"
#include "ipdf/Simple/SimpleDetectorResponse.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/UnConvolutedPEP.h"
#include "ipdf/Pandel/UPatchPEP.h"
#include "ipdf/Pandel/GConvolutePEP.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <sstream>

using std::cout;
using std::endl;

using IPDF::AllOMsLikelihood;
using IPDF::InfiniteMuon;
using IPDF::Likelihood::SPE1st;
using IPDF::Likelihood::SPEAll;
using IPDF::Likelihood::MPE;
using IPDF::Likelihood::PSA;
using IPDF::SimpleOmReceiver;
using IPDF::SimplePEHit;
using IPDF::SimpleHitOm;
using IPDF::SimpleDetectorResponse;

typedef IPDF::Pandel::UnConvolutedPEP<IPDF::Pandel::H2>	Pandel;
typedef IPDF::Pandel::UPatchPEP<IPDF::Pandel::H2>	PandelUPatch;
typedef IPDF::Pandel::GConvolutePEP<IPDF::Pandel::H2>	PandelGConvolute;

typedef SPE1st<Pandel> LikelihoodSPE1st;
typedef SPEAll<Pandel> LikelihoodSPEAll;
typedef MPE<Pandel>    LikelihoodMPE;
typedef PSA<Pandel>    LikelihoodPSA;

TEST_GROUP(LikelihoodTest)

namespace
{
  const double prec=1.e-13;

  /// Class used to test any likelihood
  template<class TLikelihood, class TPEP = typename TLikelihood::PEPType >
  class TestLikelihood {
  private:
    AllOMsLikelihood<TLikelihood>*      icepdf_;
    const double	      omx_, omy_, omz_;
    const SimpleOmReceiver    omr_;
    std::vector<SimplePEHit*> pehits_;
    SimpleDetectorResponse*   response_;
    const double	      px_, py_, pz_;
    const double	      dx_, dy_, dz_;
    const InfiniteMuon& infMuon_;
  public:
    typedef TLikelihood Likelihood;
    typedef TPEP PEPType;

    TestLikelihood()
    : icepdf_(new AllOMsLikelihood<TLikelihood>),
      omx_(0.), omy_(0.), omz_(0.),
      omr_(omx_,omy_,omz_,-1.,15.,1.3), 
      response_(0),
      px_(10.), py_(20.), pz_(30.),
      dx_(0.1), dy_(-0.1), dz_(1.),
      infMuon_(*new InfiniteMuon(px_,py_,pz_, dx_,dy_,dz_,1.e3/*GeV*/))
    {
      const InfiniteMuon::EmissionGeometry<double> egeom(infMuon_,omr_);
      for(int time=1; time<=51; time+=10) {
	const double le(time+egeom.geometricalTime());
	pehits_.push_back(new SimplePEHit( omr_, le ));
      }
      response_ = new SimpleDetectorResponse( new SimpleHitOm(omr_, pehits_) );
    }

    ~TestLikelihood() {
      delete response_;
      delete &infMuon_;
      delete icepdf_;
    }

    void doTest() {
      this->testCopy();
      this->testGetLikelihood();
      this->testGetLogLikelihood();
      this->testGetIntLikelihood();
    }

    void testCopy() {
      for(int i=0; i<=2; ++i) {
	TLikelihood copy = icepdf_->getLikelihood();  // copy ctor
	TLikelihood test = icepdf_->getLikelihood();
	test = copy;  // assignment operator
	copy = copy;  // assignment to self
      }
    }

      void testGetLikelihood() {
	double result = icepdf_->getLikelihood(*response_,infMuon_);
	std::ostringstream error;
	error<<"TestGetLikelihood failed: "<<(*icepdf_)<<" expected: "
	     <<std::setprecision(16)<<expected_getlike;
	ENSURE_DISTANCE(result,expected_getlike,prec,error.str());
      }
    static const double	      expected_getlike;

      void testGetLogLikelihood() const {
	double result = icepdf_->getLogLikelihood(*response_,infMuon_);
	std::ostringstream error;
	error<<"TestGetLogLikelihood failed: "<<(*icepdf_)<<" expected: "
	     <<std::setprecision(16)<<expected_getloglike;
	ENSURE_DISTANCE(result,expected_getloglike,prec*1.e1,error.str());
      }
    static const double	      expected_getloglike;

      void testGetIntLikelihood() const {
	double result = icepdf_->getIntLikelihood(*response_,infMuon_);
	std::ostringstream error;
	error<<"TestGetIntLikelihood failed: "<<(*icepdf_)<<" expected: "
	     <<std::setprecision(16)<<expected_getintlike;
	ENSURE_DISTANCE(result,expected_getintlike,prec,error.str());
      }
    static const double	      expected_getintlike;
  };

  template<> const double TestLikelihood<LikelihoodSPE1st>::expected_getlike	= 0.01739285396460634;
  template<> const double TestLikelihood<LikelihoodSPE1st>::expected_getloglike = -4.051695848807874;
  template<> const double TestLikelihood<LikelihoodSPE1st>::expected_getintlike = 0.02549825228936436;

  template<> const double TestLikelihood<LikelihoodSPEAll>::expected_getlike	= 8.112936442141587e-14;
  template<> const double TestLikelihood<LikelihoodSPEAll>::expected_getloglike = -30.142731422610382;
  template<> const double TestLikelihood<LikelihoodSPEAll>::expected_getintlike = 1.737597922731037e-05;

  template<> const double TestLikelihood<LikelihoodMPE>::expected_getlike    = 0.09171391076909799;
  template<> const double TestLikelihood<LikelihoodMPE>::expected_getloglike = -2.38908121254325;
  template<> const double TestLikelihood<LikelihoodMPE>::expected_getintlike = 0.1435623837646375;

  template<> const double TestLikelihood<LikelihoodPSA>::expected_getlike    = 0.03249305452124993;
  template<> const double TestLikelihood<LikelihoodPSA>::expected_getloglike = -3.426728919523512;
  template<> const double TestLikelihood<LikelihoodPSA>::expected_getintlike = 0.04856740871191749;

  template<> const double TestLikelihood<PSA<PandelUPatch> >::expected_getlike    = 0.0084509466606694748;
  template<> const double TestLikelihood<PSA<PandelUPatch> >::expected_getloglike = -4.773476813039583;
  template<> const double TestLikelihood<PSA<PandelUPatch> >::expected_getintlike = 0.1806750763486858;

  template<> const double TestLikelihood<PSA<PandelGConvolute> >::expected_getlike    = 0.00857951885443743;
  template<> const double TestLikelihood<PSA<PandelGConvolute> >::expected_getloglike = -4.758377444626067;
  template<> const double TestLikelihood<PSA<PandelGConvolute> >::expected_getintlike = 0.1256845809432498;
}

  TEST(SPE1st)
  {
    TestLikelihood<LikelihoodSPE1st> test;
    test.doTest();
  }

  TEST(SPEAll)
  {
    TestLikelihood<LikelihoodSPEAll> test;
    test.doTest();
  }

  TEST(MPE)
  {
    TestLikelihood<LikelihoodMPE> test;
    test.doTest();
  }

  TEST(PSA)
  {
    TestLikelihood<LikelihoodPSA> test;
    test.doTest();
  }

  /// Combination of PSA and UPatch
  TEST(PSA_UPatch)
  {
    TestLikelihood<PSA<PandelUPatch> > test;
    test.doTest();
  }

  /// Combination of PSA and GConvolute
  TEST(PSA_GConvolute)
  {
    TestLikelihood<PSA<PandelGConvolute> > test;
    test.doTest();
  }
