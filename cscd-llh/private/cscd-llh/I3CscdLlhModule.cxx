/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3CscdLlhModule.cxx
 * @version $Revision: 1.2 $
 * @date $Date$
 * @author mggreene
 * @author Doug Rutledge (maintenance and DC V2 Conversion)
 */
//#define  CSCD_LLH_USING_PHYSICS_TIMER 1

#include <icetray/I3Configuration.h>

#ifdef CSCD_LLH_USING_PHYSICS_TIMER
#include <icetray/I3PhysicsTimer.h>
#endif

#include "cscd-llh/I3CscdLlhModule.h"
#include "recclasses/I3CscdLlhFitParams.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/status/I3DOMStatus.h"
#include "dataclasses/status/I3DetectorStatus.h"

#include "icetray/I3TrayHeaders.h"

#include "cscd-llh/minimizer/I3CscdLlhHit.h"

#include <iostream>
using namespace std;

I3_MODULE(I3CscdLlhModule);

/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3CscdLlhModule::I3CscdLlhModule(const I3Context& context) : 
  I3ConditionalModule(context) 
{
  log_debug("Enter I3CscdLlhModule::I3CscdLlhModule().");

  countEvents_ = 0;
  countRecords_ = 0;
  countAllOk_ = 0;

  addNoHits_ = false;

  optInputType_ = "RecoHit";
  AddParameter("InputType",
    "Type of input hits."
    "  Options are \"RecoHit\" and \"RecoPulse\"",
    optInputType_);
  
  optRecoSeries_ = "Muon-DAQ Hits";
  AddParameter("RecoSeries", "RecoSeries name", optRecoSeries_);
  
  optFirstLE_ = true;
  AddParameter("FirstLE",
    "If true, use first leading edge with amplitude weight.\n"
    "If false, use all leading edges with no weights.",
    optFirstLE_);

  optSeedWithOrigin_ = false;
  AddParameter("SeedWithOrigin",
    "If true, use the detector origin and t=0 as a first guess.\n"
    "The \"SeedKey\" will be ignored.  To be used only for testing purposes.",
    optSeedWithOrigin_);

  optSeedKey_.clear();
  AddParameter("SeedKey", "The dictionary key that"
    " holds the first guess vertex.", optSeedKey_);

  optMinHits_ = 10;
  AddParameter("MinHits", 
    "Minimum number of hits in the event.",
    optMinHits_);
    
  optAmpWeightPower_ = 0.0;
  AddParameter("AmpWeightPower", 
    "Hits are weighted with an amplitude raised to this power, if"
    " \"FirstLE\" is true.\nThe \"amplitude\" is Set equal to the "
    "number of hits in the RecoHitsSeries for the OM.\n"
    "Typically 0.0 (weight=1 for all hits) or 1.0 (weight=amplitude).",
    optAmpWeightPower_);

  optResultName_ = "CscdLlh";
  AddParameter("ResultName", 
    "Key for the RecoResultDict entry that holds the result of "
    "the reconstruction.", optResultName_);

  optEnergySeed_ = NAN;
  AddParameter("EnergySeed", 
    "The starting value for the cascade energy (GeV).",
    optEnergySeed_);

  optMinimizer_.clear();
  AddParameter("Minimizer", 
    "The function minimizer.\n"
    "Options are \"Brent\", \"Powell\", and \"Simplex\"",
    optMinimizer_);
    
  optPdf_.clear();
  AddParameter("PDF", 
    "The probability density function.\n"
    "Options are \"UPandel\", \"UPandelMpe\","
    " \"HitNoHit\", \"HitNoHitMpe\", and \"PndlHnh\","
    "\"HnhDir \"",
    optPdf_);

  excludedOMs_.clear();
  AddParameter("ExcludedOMs",
    "This is a vector of OMs that are to be excluded from contributing to the "
    "likelihood. This is necessary for PDFs that need to know about un-hit "
    "OMs.",
    excludedOMs_);
  
  parserGeneral_ = 
    I3CscdLlhGeneralParserPtr(
      new I3CscdLlhGeneralParser(configuration_));
  parserGeneral_->AddParameters();

  parserUPandel_ = I3CscdLlhUPandelParserPtr(
    new I3CscdLlhUPandelParser(configuration_));
  parserUPandel_->AddParameters();

  parserHitNoHit_ = I3CscdLlhHitNoHitParserPtr(
    new I3CscdLlhHitNoHitParser(configuration_));
  parserHitNoHit_->AddParameters();
    
  parserPndlHnh_ = I3CscdLlhPndlHnhParserPtr(
    new I3CscdLlhPndlHnhParser(configuration_));
  parserPndlHnh_->AddParameters();

  parserHnhDir_ = I3CscdLlhHnhDirParserPtr(
    new I3CscdLlhHnhDirParser(configuration_));
  parserHnhDir_->AddParameters();
  
  AddOutBox("OutBox");
  
  log_debug("Exit I3CscdLlhModule::I3CscdLlhModule().");
} // end constructor

/* ******************************************************************** */
/* Destructor                                                           */
/* ******************************************************************** */
I3CscdLlhModule::~I3CscdLlhModule() 
{
}

/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3CscdLlhModule::Configure() 
{
  log_debug("Enter I3CscdLlhModule::Configure().");

  GetParameter("InputType", optInputType_);
  GetParameter("RecoSeries", optRecoSeries_);
  GetParameter("FirstLE", optFirstLE_);
  GetParameter("SeedWithOrigin", optSeedWithOrigin_);
  GetParameter("SeedKey", optSeedKey_);
  GetParameter("MinHits", optMinHits_);
  GetParameter("AmpWeightPower", optAmpWeightPower_);
  GetParameter("ResultName", optResultName_);
  GetParameter("EnergySeed", optEnergySeed_);
  GetParameter("Minimizer", optMinimizer_);
  GetParameter("PDF", optPdf_);
  GetParameter("ExcludedOMs", excludedOMs_);

  log_info("InputType = %s", optInputType_.c_str());
  log_info("RecoSeries = %s", optRecoSeries_.c_str());
  log_info("FirstLE = %s", optFirstLE_ ? "true" : "false");
  log_info("SeedWithOrigin = %s", optSeedWithOrigin_ ? "true" : "false");
  log_info("SeedKey = %s", optSeedKey_.c_str()); 
  log_info("MinHits = %d", optMinHits_);
  log_info("AmpWeightPower = %f", optAmpWeightPower_);
  log_info("ResultName = %s", optResultName_.c_str()); 
  log_info("EnergySeed = %f", optEnergySeed_);
  log_info("Minimizer = %s", optMinimizer_.c_str());
  log_info("PDF = %s", optPdf_.c_str());

  bool SetMinimizer = false;
  if (optMinimizer_ == "Brent") {
    SetMinimizer = SetBrent();  
  } else if (optMinimizer_ == "Powell") {
    SetMinimizer = SetPowell();
  } else if (optMinimizer_ == "Simplex") {
    SetMinimizer = SetSimplex();
  } else {
    log_fatal("Unknown Minimizer: %s", optMinimizer_.c_str());
  }

  if (!SetMinimizer) return;

  bool SetPdf = false;
  if (optPdf_ == "UPandel")
  {
    SetPdf = SetUPandel(false);
  }
  else if (optPdf_ == "UPandelMpe")
    SetPdf = SetUPandel(true);
  else if (optPdf_ == "HitNoHit")
    SetPdf = SetHitNoHit(false);
  else if (optPdf_ == "HitNoHitMpe")
    SetPdf = SetHitNoHit(true);
  else if (optPdf_ == "PndlHnh")
    SetPdf = SetPndlHnh();
  else if (optPdf_ == "HnhDir")
    SetPdf = SetHnhDir();
  else
    log_fatal("Unknown PDF: '%s'", optPdf_.c_str());

  if (!SetPdf) return;

  if (!fitter_.Configure(parserGeneral_)) 
  {
    log_fatal("Unable to Configure general parameters!");
    return;
  }

  log_debug("Exit I3CscdLlhModule::Configure().");
  return;
} // end Configure

/* ******************************************************************** */
/* SetBrent                                                             */
/* ******************************************************************** */
bool I3CscdLlhModule::SetBrent() 
{
  log_debug("Enter I3CscdLlhModule::SetBrent().");

  if (!fitter_.SetMinimizer(I3CscdLlhFitter::MINIMIZER_BRENT)) 
  {
    log_fatal("Unable to Set Minimizer to Brent");
    return false;
  }

  log_debug("Exit I3CscdLlhModule::SetBrent().");
  return true;
} // end SetBrent

/* ******************************************************************** */
/* SetSimplex                                                           */
/* ******************************************************************** */
bool I3CscdLlhModule::SetSimplex() 
{
  log_debug("Enter I3CscdLlhModule::SetSimplex().");

  if (!fitter_.SetMinimizer(I3CscdLlhFitter::MINIMIZER_SIMPLEX)) 
  {
    log_fatal("Unable to Set Minimizer to Simplex");
    return false;
  }

  log_debug("Exit I3CscdLlhModule::SetSimplex().");
  return true;
} // end SetSimplex

/* ******************************************************************** */
/* SetPowell                                                            */
/* ******************************************************************** */
bool I3CscdLlhModule::SetPowell() 
{
  log_debug("Enter I3CscdLlhModule::SetPowell().");

  if (!fitter_.SetMinimizer(I3CscdLlhFitter::MINIMIZER_POWELL)) 
  {
    log_fatal("Unable to Set Minimizer to Powell");
    return false;
  }

  log_debug("Exit I3CscdLlhModule::SetPowell().");
  return true;
} // end SetPowell

/* ******************************************************************** */
/* SetUPandel                                                           */
/* ******************************************************************** */
bool I3CscdLlhModule::SetUPandel(bool mpe) 
{
  log_debug("Enter I3CscdLlhModule::SetUPandel().");

  if (mpe) 
  {
    if (!fitter_.SetPdf(I3CscdLlhFitter::PDF_UPANDEL_MPE)) 
    {
      log_fatal("Unable to Set PDF to UPandelMpe");
      return false;
    }
  }
  else 
  {
    if (!fitter_.SetPdf(I3CscdLlhFitter::PDF_UPANDEL)) 
    {
      log_fatal("Unable to Set PDF to UPandel");
      return false;
    }
  }

//DLR
  parserUPandel_->I3CscdLlhAbsParser::SetFitter(&fitter_);

  //DLRif (!fitter_.Configure(parserUPandel_)) 
  if (!parserUPandel_->Configure()) 
  {
    log_fatal("Unable to Configure UPandel!");
    return false;
  }

  log_debug("Exit I3CscdLlhModule::SetUPandel().");
  return true;
} // end SetUPandel

/* ******************************************************************** */
/* SetHitNoHit                                                          */
/* ******************************************************************** */
bool I3CscdLlhModule::SetHitNoHit(bool mpe) 
{
  log_debug("Enter I3CscdLlhModule::SetHitNoHit().");

  if (mpe) 
  {
    if (!fitter_.SetPdf(I3CscdLlhFitter::PDF_HIT_NO_HIT_MPE)) 
    {
      log_fatal("Unable to Set PDF to HitNoHitMpe");
      return false;
    }
  }
  else 
  {
    if (!fitter_.SetPdf(I3CscdLlhFitter::PDF_HIT_NO_HIT)) 
    {
      log_fatal("Unable to Set PDF to HitNoHit");
      return false;
    }
  }

  if (!fitter_.Configure(parserHitNoHit_)) 
  {
    log_fatal("Unable to Configure HitNoHit!");
    return false;
  }

  // The Hit/No-hit algorithm needs to know about unhit OM's!
  addNoHits_ = true;

  log_debug("Exit I3CscdLlhModule::SetHitNoHit().");
  return true;
} // end SetHitNoHit

/* ******************************************************************** */
/* SetPndlHnh                                                           */
/* ******************************************************************** */
bool I3CscdLlhModule::SetPndlHnh() 
{
  log_debug("Enter I3CscdLlhModule::SetPndlHnh().");

  if (!fitter_.SetPdf(I3CscdLlhFitter::PDF_PNDL_HNH)) 
  {
    log_fatal("Unable to Set PDF to PndlHnh");
    return false;
  }

  if (!fitter_.Configure(parserPndlHnh_)) 
  {
    log_fatal("Unable to Configure PndlHnh!");
    return false;
  }

  // PndlHnh algorithm needs to know about unhit OM's!
  addNoHits_ = true;

  log_debug("Exit I3CscdLlhModule::SetPndlHnh().");
  return true;
} // end SetPndlHnh

/* ******************************************************************** */
/* SetHnhDir                                                          */
/* ******************************************************************** */
bool I3CscdLlhModule :: SetHnhDir()
{
  log_debug("Enter I3CscdLlhModule :: SetHnhDir");

  if (!fitter_.SetPdf(I3CscdLlhFitter::PDF_HNH_DIR)) 
  {
    log_fatal("Unable to Set PDF to HnhDir");
    return false;
  }

  if (!fitter_.Configure(parserHnhDir_)) 
  {
    log_fatal("Unable to Configure HnhDir!");

    return false;
  }

  log_debug("Exit I3CscdLlhModule :: SetHnhDir");
  return true;
}

/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
void I3CscdLlhModule::Physics(I3FramePtr frame) 
{ 
#ifdef CSCD_LLH_USING_PHYSICS_TIMER
I3PhysicsTimer timer(frame, GetName());
#endif
  log_debug("******** Enter I3CscdLlhModule::Physics() [%s]. ********",
         optResultName_.c_str());

  log_debug("Processing event #%d", countEvents_);
  countEvents_++;

  fitter_.Clear();

  bool allOk = true;

  bool seedSet = true;
  if (allOk && !SetSeed(frame)) 
  {
    allOk = false;
    seedSet = false;
    log_info("%s: Event# %d: Unable to Set seed!",
      optResultName_.c_str(), countEvents_);
  }

  bool sufficientHits = true;
  if (optInputType_ == "RecoHit") 
  {
    if (allOk && !AddRecoHits(frame)) 
    {
      allOk = false;
      sufficientHits = false;
      log_info("%s: Event# %d: Unable to add RecoHits!",
        optResultName_.c_str(), countEvents_);
    }
  }
  else if (optInputType_ == "RecoPulse")
  {
    if (allOk && !AddRecoPulses(frame))
    {
      allOk = false;
      sufficientHits = false;
      log_info("%s: Event# %d: Unable to add RecoPulses!",
        optResultName_.c_str(), countEvents_);
    }
  }
  else 
  {
    allOk = false;
    log_error("Unknown input type! [%s]", optInputType_.c_str());
  }

  if (allOk && addNoHits_ && !AddNoHits(frame)) 
  {
    allOk = false;
    log_warn("%s: Event# %d: Unable to add no-hits!",
        optResultName_.c_str(), countEvents_);
  }
  
  bool fitOk = true;
  if (allOk && !fitter_.Fit()) 
  {
    allOk = false;
    fitOk = false;
    log_info("%s: Event# %d: Fit failed!",
        optResultName_.c_str(), countEvents_);
  }

  if (!allOk) fitter_.Clear();
  I3ParticlePtr cascadePtr = fitter_.GetCascade();
  I3CscdLlhFitParamsPtr fitParams = fitter_.GetFitParams();

  /*This next part might be a little confusing, so here's what's
    going on. I need to set a field in the result which tells
    people about the status of the fit. There are many different
    ways that this can fail, and there are many different 
    combinations of these failures (i.e. there was no seed
    because of an insufficient number of hits). So, I'm 
    going to first set the status to be OK, and then 
    check for that there was no general failure when all others
    are true. Then I'm going to check each failure mode 
    individually, in order of precedent (a prioritization that
    I've decided upon), and set the status accordingly. I will
    ingore cases of combinations, since the FitStatus field 
    ignores them.DLR */
  cascadePtr->SetFitStatus(I3Particle::OK);

  if(!allOk && fitOk && seedSet && sufficientHits)
  {
    cascadePtr->SetFitStatus(I3Particle::GeneralFailure);
  }
  else if(!fitOk)
  {
    cascadePtr->SetFitStatus(I3Particle::FailedToConverge);
  }
  else if(!sufficientHits)
  {
    cascadePtr->SetFitStatus(I3Particle::InsufficientHits);
  }
  else if(!seedSet)
  {
    cascadePtr->SetFitStatus(I3Particle::MissingSeed);
  }

  log_debug("Reco vertex (t, x, y, z) ="
        " (%f, %f, %f, %f)", 
        cascadePtr->GetTime(),
        cascadePtr->GetPos().GetX(),
        cascadePtr->GetPos().GetY(),
        cascadePtr->GetPos().GetZ());

  log_debug("Reco vertex (theta, phi, energy) ="
        " (%f, %f, %5.2e)", 
        cascadePtr->GetDir().GetZenith(),
        cascadePtr->GetDir().GetAzimuth(),
        cascadePtr->GetEnergy());

  // Put the cascade into the dictionary.
  frame->Put(optResultName_, cascadePtr);
  frame->Put(optResultName_ + "Params",fitParams);
  countRecords_++;

  if (allOk) 
  {
    countAllOk_++;
    log_debug("Successfully added result named \"%s\".", 
      optResultName_.c_str());
  }

  PushFrame(frame, "OutBox");
  log_debug("******** Exit I3CscdLlhModule::Physics() [%s]. ********",
         optResultName_.c_str());
} // end Physics

/* ******************************************************************** */
/* SetSeed                                                              */
/* ******************************************************************** */
bool I3CscdLlhModule::SetSeed(I3FramePtr frame) 
{
  log_debug("Enter I3CscdLlhModule::SetSeed()."); 

  if (optSeedWithOrigin_) 
  {
    if (parserGeneral_->UseParamT()) 
      fitter_.SetSeed("t", 0.0);
    if (parserGeneral_->UseParamX()) 
      fitter_.SetSeed("x", 0.0);
    if (parserGeneral_->UseParamY()) 
      fitter_.SetSeed("y", 0.0);
    if (parserGeneral_->UseParamZ()) 
      fitter_.SetSeed("z", 0.0);
    return true;
  }

  log_debug("Getting seed from RecoResult named %s.", optSeedKey_.c_str());

  const I3Particle& recoResult = frame->Get<I3Particle>(optSeedKey_);
  
  fitter_.InitializeResult(recoResult);

  double seedT = recoResult.GetTime();
  double seedX = recoResult.GetPos().GetX();
  double seedY = recoResult.GetPos().GetY();
  double seedZ = recoResult.GetPos().GetZ();

  if (parserGeneral_->UseParamT()) 
  {
    if (std::isnan(seedT)) 
    {
      log_debug("Time seed is NAN!");
      return false;
    }

    fitter_.SetSeed("t", seedT);
    log_debug("Seed time with %f.", seedT);
  }

  if (parserGeneral_->UseParamX()) 
  {
    if (std::isnan(seedX)) 
    {
      log_debug("x position seed is NAN!");
      return false;
    }

    fitter_.SetSeed("x", seedX);
    log_debug("Seed x with %f.", seedX);
  }

  if (parserGeneral_->UseParamY()) 
  {
    if (std::isnan(seedY)) 
    {
      log_debug("y position seed is NAN!");
      return false;
    }

    fitter_.SetSeed("y", seedY);
    log_debug("Seed y with %f.", seedY);
  }

  if (parserGeneral_->UseParamZ()) 
  {
    if (std::isnan(seedZ)) 
    {
      log_debug("z position seed is NAN!");
      return false;
    }

    fitter_.SetSeed("z", seedZ);
    log_debug("Seed z with %f.", seedZ);
  }

  if (parserGeneral_->UseParamEnergy() && !std::isnan(optEnergySeed_)) 
  {
    fitter_.SetSeed("energy", optEnergySeed_);
    log_debug("Seed energy with %f.", optEnergySeed_);
  }
  else if (parserGeneral_->UseParamEnergy())// && std::isnan(optEnergySeed_))
  {
    double seedEnergy = recoResult.GetEnergy();
    if (std::isnan(seedEnergy))
    {
      log_debug("Energy Seed is nan!");
      return false;
    }
    
    fitter_.SetSeed("energy",seedEnergy);
    log_debug("Seed energy with %f GeV",seedEnergy);
  }

  if (parserGeneral_->UseParamZenith())
  {
    fitter_.SetSeed("zenith",90.0*I3Units::deg);
    log_debug("Seed zenith with 90 degrees");
  }

  if (parserGeneral_->UseParamAzimuth())
  {
    fitter_.SetSeed("azimuth", 90.0*I3Units::deg);
    log_debug("Seed Zenith with 90 degrees");
  }

  log_debug("Exit I3CscdLlhModule::SetSeed()."); 
  return true;
} // end SetSeed

/* ******************************************************************** */
/* AddRecoHits                                                          */
/* ******************************************************************** */
bool I3CscdLlhModule::AddRecoHits(I3FramePtr frame) 
{
  log_debug("Enter I3CscdLlhModule::AddRecoHits().");

  if (frame->find(optRecoSeries_) == frame->end())
  {
    log_error("RecoHitSeriesMap named %s is missing!",optRecoSeries_.c_str());
  }
 
  const I3Geometry& geometry = frame->Get<I3Geometry>();

  I3RecoHitSeriesMapConstPtr recoHitSeriesMap =
    frame->Get<I3RecoHitSeriesMapConstPtr>(optRecoSeries_);

  //needed incase the optFirstLE_ = true
  const I3DetectorStatus& detectorStatus = 
    frame->Get<I3DetectorStatus>();

  // Loop over OM's.
  int nHitOms = 0;
  int nHits = 0;

  I3Map<OMKey,vector<I3RecoHit> > ::
    const_iterator iter;

  for(iter = recoHitSeriesMap->begin();
    iter != recoHitSeriesMap->end();
    iter++)
  {
    const OMKey& hitOM = iter->first;

    if(SkipThisOM(hitOM)) continue;

    I3OMGeo omGeo = geometry.omgeo.find(hitOM)->second;

    double hitX = omGeo.position.GetX();
    double hitY = omGeo.position.GetY();
    double hitZ = omGeo.position.GetZ();

    I3RecoHitSeries recoHitSeries = iter->second;

    int omHitCount = recoHitSeries.size();

    if (optFirstLE_) 
    {
      vector<I3RecoHit>::const_iterator firstHit = recoHitSeries.begin();
      if (firstHit == recoHitSeries.end()) 
      {
        log_error("Missing Hit Series for Module [%d,%d]",
          hitOM.GetString(),hitOM.GetOM());
        continue;
      }  
      // only if hit series is not empty
      double hitTime = firstHit->GetTime();
      double hitWeight = CalculateWeight(omHitCount);
        
      map<OMKey, I3DOMStatus>::const_iterator statusIter = 
        detectorStatus.domStatus.find(hitOM);
     
      I3DOMStatus domStatus = statusIter->second;
      double thresh = domStatus.speThreshold;

      I3CscdLlhHitPtr hit = 
        I3CscdLlhHitPtr(new I3CscdLlhHit(hitTime, hitX, hitY, hitZ, 
          hitWeight, omHitCount, -1, hitOM, omHitCount, thresh));
        
      if (std::isnan(hit->t) || std::isnan(hit->x) || std::isnan(hit->y) || std::isnan(hit->z)) 
      {
        log_debug("Rejecting invalid hit.");
        continue;
      }
      
      fitter_.AddHit(hit);
      nHits++;
    } // firstLE only
    else 
    {
      int hitNumber = 0;

      vector<I3RecoHit> :: const_iterator hit;
      
      for(hit = recoHitSeries.begin();
        hit != recoHitSeries.end(); 
	hit++) 
      {
        double hitTime = hit->GetTime();
        double hitWeight = 1.0;

        I3CscdLlhHitPtr hit = I3CscdLlhHitPtr(
          new I3CscdLlhHit(hitTime,hitX,hitY,hitZ,
            hitWeight,omHitCount,hitNumber,hitOM));
	
        if (std::isnan(hit->t) || std::isnan(hit->x) || std::isnan(hit->y) || std::isnan(hit->z)) 
        {
          log_debug("Rejecting invalid hit.");
          continue;
        }

        hitNumber++;

        fitter_.AddHit(hit);
        nHits++;
      } // for each RecoHit
    } // all RecoHits

    nHitOms++;
  } // for each OM

  if (nHits < optMinHits_) 
  {
    log_debug("Not enough hit modules (%d)!", nHits);
    return false;
  }

  fitter_.SetHitOmCount(nHitOms);

  log_debug("Exit I3CscdLlhModule::AddRecoHits().");
  return true;
} // end AddRecoHits


/* ******************************************************************** */
/* AddRecoPulses                                                        */
/* ******************************************************************** */
bool I3CscdLlhModule :: AddRecoPulses(I3FramePtr frame)
{
  log_debug("Enter I3CscdLlhModule::AddRecoPulses().");

  if(frame->find(optRecoSeries_) == frame->end())
  {
    log_error("Cannot find input RecoPulseSeriesMap named %s",
    optRecoSeries_.c_str());

    return false;
  }

  const I3Geometry& geometry = frame->Get<I3Geometry>();

  I3RecoPulseSeriesMapConstPtr recoPulseSeriesMap =
    frame->Get<I3RecoPulseSeriesMapConstPtr>(optRecoSeries_);
  
  const I3DetectorStatus& detectorStatus = 
    frame->Get<I3DetectorStatus>();

  // Loop over OM's.
  int nHitOms = 0;
  int nHits = 0;

  if (!recoPulseSeriesMap)
  {
    log_warn("Cannot find input RecoPulseSeries named %s",
    optRecoSeries_.c_str());
    return false;
  }

  I3Map<OMKey,vector<I3RecoPulse> > ::
    const_iterator iter;

  for(iter = recoPulseSeriesMap->begin();
    iter != recoPulseSeriesMap->end();
    iter++)
  {
    const OMKey& hitOM = iter->first;

    if (SkipThisOM(hitOM)) continue;

    I3OMGeo omGeo = geometry.omgeo.find(hitOM)->second;
    
    map<OMKey, I3DOMStatus>::const_iterator statusIter = 
      detectorStatus.domStatus.find(hitOM);
     
    I3DOMStatus domStatus = statusIter->second;
    double thresh = domStatus.speThreshold;

    double pulseX = omGeo.position.GetX();
    double pulseY = omGeo.position.GetY();
    double pulseZ = omGeo.position.GetZ();

    I3RecoPulseSeries recoPulseSeries = iter->second;
    
    int omHitCount = recoPulseSeries.size();

    if (optFirstLE_)
    { 
      vector<I3RecoPulse>::const_iterator firstPulse = recoPulseSeries.begin();
      if (firstPulse == recoPulseSeries.end())
      {
        //This sort of thing happens if one is using simulated data. 
        //There are entries in the pulse series map that have no 
        //actual pulses.
        log_trace("Missing Pulse Series for Module [%d,%d]",
          hitOM.GetString(),hitOM.GetOM());
        continue;
      }

      double charge = 0.0; 
      //now, loop over all pulses, adding up the charge
      vector<I3RecoPulse>::const_iterator pulse = recoPulseSeries.begin();
      for(;pulse != recoPulseSeries.end();pulse++)
      {
        charge += pulse->GetCharge();
      }

      // only if pulse series is not empty
      double pulseTime = firstPulse->GetTime();
      double weight = CalculateWeight(charge);

      I3CscdLlhHitPtr hit = 
        I3CscdLlhHitPtr(new I3CscdLlhHit(pulseTime,pulseX,pulseY,pulseZ,
          weight,omHitCount,-1,hitOM, thresh, charge));

      if (!hit)
      {
        log_fatal("hit was null for DOM (%d,%d)! This was hit #%d",
        hitOM.GetString(), hitOM.GetOM(), omHitCount );
      }

      if (std::isnan(hit->t) || std::isnan(hit->x) || std::isnan(hit->y) || std::isnan(hit->z)) 
      {
        log_debug("Rejecting invalid hit.");
        continue;
      }
        
      if(!fitter_.AddHit(hit))
      {
        log_fatal("Cannot add hit for pulse from: (%d,%d)",
          hitOM.GetString(), hitOM.GetOM());
      }
      
      nHits++;
    }
    else
    {
      int hitNumber = 0;

      vector<I3RecoPulse> :: const_iterator pulse;
      
      for(pulse = recoPulseSeries.begin();
        pulse != recoPulseSeries.end(); 
	pulse++)
      {
        double charge = pulse->GetCharge();
        double pulseTime = pulse->GetTime();
        double pulseWeight = CalculateWeight(charge);
        
        I3CscdLlhHitPtr hit =
          I3CscdLlhHitPtr(new I3CscdLlhHit(pulseTime,pulseX,pulseY,pulseZ,
            pulseWeight,omHitCount,hitNumber,hitOM, charge, thresh));

        if (std::isnan(hit->t) || std::isnan(hit->x) || std::isnan(hit->y) || std::isnan(hit->z))
        {
          log_debug("Rejecting invalid hit.");
          continue;
        }
        log_debug("Adding Hit: hitNumber: %d, Amplitude: %f, Weight: %f",
          hitNumber,charge,pulseWeight);

        hitNumber++;

        fitter_.AddHit(hit);
        nHits++;
      }
    }
  }
  
  if (nHits < optMinHits_) 
  {
    log_debug("Not enough hit modules (%d)!", nHits);
    return false;
  }
  log_debug("Found NHits: %d",nHits);

  fitter_.SetHitOmCount(nHitOms);

  log_debug("Exit I3CscdLlhModule::AddRecoPulses().");
  return true;
}//AddRecoPulses

/* ******************************************************************** */
/* AddNoHits                                                              */
/* ******************************************************************** */
bool I3CscdLlhModule::AddNoHits(I3FramePtr frame) 
{
  log_debug("Enter I3CscdLlhModule::AddNoHits().");

  const I3Geometry& geometry = frame->Get<I3Geometry>();

  I3Map<OMKey,I3OMGeo> :: const_iterator iter;
  
  int nUnhitOms = 0;

  I3CscdLlhHitPtr hit;

  // Loop over all (hit and unhit) OM's.
  for (iter = geometry.omgeo.begin();
    iter != geometry.omgeo.end(); 
    iter++) 
  {

    const OMKey& omKey = iter->first;


    if (SkipThisOM(omKey)) continue;

    I3OMGeo omGeo = iter->second;
   
    if(optInputType_ == "RecoHit")
    {
      I3RecoHitSeriesMapConstPtr hitSeriesMap = 
        frame->Get<I3RecoHitSeriesMapConstPtr>(optRecoSeries_); 
      
      I3Map<OMKey,vector<I3RecoHit> > :: const_iterator reco_iter;
      
      bool is_Hit = false;
      // Loop over hit OM's
      for (reco_iter = hitSeriesMap->begin(); 
        reco_iter != hitSeriesMap->end(); reco_iter++) 
      {

        if (reco_iter->first == omKey) 
        {
          is_Hit = true;
          break;
        }
      } // for each hit OM

      if (is_Hit) 
      {
        log_trace("Hit OM: string %d, OM %u", omKey.GetString(), omKey.GetOM());
        continue;
      }
      else 
      {
        log_trace("Unhit OM: string %d, OM %u",
          omKey.GetString(), omKey.GetOM());
      }

      hit = I3CscdLlhHitPtr(new I3CscdLlhHit());
      hit->x = omGeo.position.GetX();
      hit->y = omGeo.position.GetY();
      hit->z = omGeo.position.GetZ();
      hit->t = NAN;
      hit->weight = 1.0;
      hit->omHitCount = 0;

      if (std::isnan(hit->x) || std::isnan(hit->y) || std::isnan(hit->z)) 
      {
        log_debug("Rejecting invalid hit.");
        continue;
      }
    }//case of recohits
    else if (optInputType_ == "RecoPulse") 
    {
      I3RecoPulseSeriesMapConstPtr pulseSeriesMap = 
        frame->Get<I3RecoPulseSeriesMapConstPtr>(optRecoSeries_); 
      
      I3Map<OMKey,vector<I3RecoPulse> > :: const_iterator reco_iter;
      
      bool is_Hit = false;
      // Loop over hit OM's
      for (reco_iter = pulseSeriesMap->begin(); 
        reco_iter != pulseSeriesMap->end(); reco_iter++) 
      {

        if (reco_iter->first == omKey) 
        {
          is_Hit = true;
          break;
        }
      } // for each hit OM

      if (is_Hit) 
      {
        log_trace("Hit OM: string %d, OM %u", omKey.GetString(), omKey.GetOM());
        continue;
      }
      else 
      {
        log_trace("Unhit OM: string %d, OM %u",
          omKey.GetString(), omKey.GetOM());
      }

      const I3DetectorStatus& detectorStatus = 
        frame->Get<I3DetectorStatus>();

      map<OMKey, I3DOMStatus>::const_iterator statusIter = 
        detectorStatus.domStatus.find(omKey);
     
      I3DOMStatus domStatus = statusIter->second;
      double thresh = domStatus.speThreshold;

      hit = I3CscdLlhHitPtr(new I3CscdLlhHit());
      hit->x = omGeo.position.GetX();
      hit->y = omGeo.position.GetY();
      hit->z = omGeo.position.GetZ();
      hit->t = NAN;
      hit->weight = 1.0;
      hit->omHitCount = 0;
      hit->amplitude = 0.0;
      hit->triggerThresh = thresh;

      if (std::isnan(hit->x) || std::isnan(hit->y) || std::isnan(hit->z)) 
      {
        log_debug("Rejecting invalid hit.");
        continue;
      }
    }

    fitter_.AddHit(hit);
    nUnhitOms++;
  } // for each (hit and unhit) OM

  fitter_.SetUnhitOmCount(nUnhitOms);

  log_debug("Exit I3CscdLlhModule::AddNoHits().");
  return true;
} // end AddNoHits

/* ******************************************************************** */
/* CalculateWeight                                                      */
/* ******************************************************************** */
double I3CscdLlhModule::CalculateWeight(double amplitude) {
  log_trace("Enter I3CscdLlhModule::CalculateWeight().");

  double weight = 0.0;
  if (optAmpWeightPower_ == 0.0) {
    weight = 1.0;
  }
  else if (optAmpWeightPower_ == 1.0) {
    weight = amplitude;
  }
  else {
    weight = std::pow(amplitude, optAmpWeightPower_);
  }

  log_trace("Exit I3CscdLlhModule::CalculateWeight().");
  return weight;
} // end CalculateWeight

/* ******************************************************************** */
/* Finish                                                               */
/* ******************************************************************** */
void I3CscdLlhModule::Finish() {
  log_debug("Enter I3CscdLlhModule::Finish().");

  log_info("CscdLlh events: %d", countEvents_);
  log_info("CscdLlh records named \"%s\" in output root file: %d.",
           optResultName_.c_str(), countRecords_);
  log_info("CscdLlh successful fits: %d.", countAllOk_);

  log_debug("Exit I3CscdLlhModule::Finish().");
  return;
} // end Finish

bool I3CscdLlhModule :: SkipThisOM(const OMKey& omKey)
{
  bool skipThisOM = false;

  vector<OMKey> :: const_iterator excludedOM_iter;
  for( excludedOM_iter = excludedOMs_.begin();
    excludedOM_iter != excludedOMs_.end();
    excludedOM_iter++)
  {
    const OMKey& excludedOM = *excludedOM_iter;
    if (omKey == excludedOM)
    {
      skipThisOM = true;
    }
  }

  return skipThisOM;
}
