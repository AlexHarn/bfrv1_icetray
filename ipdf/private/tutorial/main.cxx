/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file tutorial/main.cxx
    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
 
    @brief Simple demonstration of the IPDF framework

    @section tuorial Tutorial - basic IPDF usage

    This example demonstrates the basic interface to the 
    IPDF software and its use.
    
    1. Declare a PEP for use in the likelihood functions
       (this can then be configured and initialised by the user).
    
    2. Declare a likelihood function and initialize it 
       with a PEP, and place inside the AllOMsLikelihood.
    
    3. Initialise the event hypothesis.

    4. Initialise the detector configuration.

    5. Initialise the event detector response.

    6. Create the detector response.
    
    7. Calculate the likelihood.
*/

#include <iostream>
#include <iomanip>
#include <vector>

//
//--- Standard IPDF includes:
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Likelihood/SPEAll.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/UnConvolutedPEP.h"
#include "ipdf/AllOMsLikelihood.h"

//
//--- Example implementations of the detector objects:
#include "ipdf/Simple/SimplePEHit.h"
#include "ipdf/Simple/SimpleHitOm.h"
#include "ipdf/Simple/SimpleOmReceiver.h"
#include "ipdf/Simple/SimpleDetectorResponse.h"

using std::cout;
using std::endl;

using namespace IPDF;

///
///--- Provide a typedef of the class providing the Pandel 
///    photo-electron PDF:
typedef Pandel::UnConvolutedPEP<Pandel::H2> PandelPDF;
//		^PDF implementation     ^IceModel

int main() {

  try {
    //
    //--- configure the SPE likelihood for all 
    //	  photo-electron hits, using the Pandel PDF:
    AllOMsLikelihood< Likelihood::SPEAll<PandelPDF> > likelihoodSPE;
    cout<<likelihoodSPE<<endl;

    //
    //--- Configure the track and OM geometry...
    InfiniteMuon track(1.,2.,3.,4.,5.,-1.,2.e7);
	
    cout << track << endl;

    double om_sigma = 15.;
    double om_sensi = 1.3;

    //
    SimpleOmReceiver omr(0.,0.,0.,-1.,om_sigma,om_sensi);

    //
    //--- Create a photo-electron hit:
    double letime(10.);
    SimplePEHit* pehit = new SimplePEHit(omr,letime);
    std::vector<SimplePEHit*> hitsv;
    hitsv.push_back( pehit );

    //
    //--- Create an hit OM:
    SimpleHitOm* hitom = new SimpleHitOm( omr, hitsv );
    std::vector<SimpleHitOm*> omHits;
    omHits.push_back( hitom );

    SimpleDetectorResponse response( omHits );
    
    cout<<"  t="<<std::setw(4)<<std::setprecision(4)
	<<hitsv[0]->getLeTime()
	<<endl;

    cout
	<<" Pdf="<<std::setw(8)<<std::setprecision(16)
	<<likelihoodSPE.getLikelihood(response,track)
	<<" LogPdf="<<std::setw(8)<<std::setprecision(16)
	<<likelihoodSPE.getLogLikelihood(response,track)
	<<" IntPdf="<<std::setw(8)<<std::setprecision(16)
	<<likelihoodSPE.getIntLikelihood(response,track)
	;
    cout<<endl;

    
  } catch (const std::exception& exp) {
    std::cerr<<exp.what()<<std::endl;
    return 1;
  } catch (...) {
    cout<<"FATAL - unidentified exception."<<endl;
    return 911;
  }
}
