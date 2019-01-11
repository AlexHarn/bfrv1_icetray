/**
 * copyright  (C) 2008
 * the icecube collaboration
 * $Id: I3OpheliaBadDomFinder.cxx 19641 2006-05-10 14:03:22Z dule $
 *
 * @file I3OpheliaBadDomFinder.cxx
 * @version $Revision: 1.7 $
 * @date $Date: 2006-05-10 23:03:22 +0900 (水, 10  5月 2006) $
 * @author mio
 */
#include "dataclasses/geometry/I3Geometry.h"
#include "ophelia/util/I3OpheliaBadDomFinder.h"

using namespace std;

I3_MODULE(I3OpheliaBadDomFinder);

/* ******************************************************************** */
/* Constructor                                                          */
/* ******************************************************************** */
I3OpheliaBadDomFinder::I3OpheliaBadDomFinder(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  AddOutBox("OutBox");

  inputBadDomListName_ = "BadDomsList";
  AddParameter("InputBadDomListName", 
	       "Input name of vector of bad DOM OMKeys", inputBadDomListName_);

  outputBadDomListName_ = "BadDomsList";
  AddParameter("OutputBadDomListName", 
	       "Output name of vector of bad DOM OMKeys", outputBadDomListName_);

  checkTransitTime_ = true;
  AddParameter("CheckTransitTime", 
	       "Flag to check PMT transit time", checkTransitTime_);

}

/* ******************************************************************** */
/* Destructor                                                            */
/* ******************************************************************** */
I3OpheliaBadDomFinder::~I3OpheliaBadDomFinder(){}

/* ******************************************************************** */
/* Configure                                                            */
/* ******************************************************************** */
void I3OpheliaBadDomFinder::Configure()
{
  GetParameter("InputBadDomListName", inputBadDomListName_);
  GetParameter("OutputBadDomListName", outputBadDomListName_);
  GetParameter("CheckTransitTime",checkTransitTime_);

  log_info("Input:  InputBadDomListName  = %s", inputBadDomListName_.c_str());
  log_info("Output: OutputBadDomListName = %s", outputBadDomListName_.c_str());
  if(checkTransitTime_) log_info("Flag to check transit time is ON");

}

/* ******************************************************************** */
/* Physics                                                              */
/* ******************************************************************** */
void I3OpheliaBadDomFinder::Physics(I3FramePtr frame)
{
  log_debug("Entering Physics...");

  /* Create new bad DOM list (BDL) */
  I3VectorOMKeyPtr vect_newBadDom(new I3VectorOMKey );

  /* Get I3Calibration */
  const I3OMGeoMap& omgeo = frame->Get<I3Geometry>().omgeo;
  const I3Calibration&    cal = frame->Get<I3Calibration>();
  const std::map<OMKey, I3DOMCalibration>& domCal = cal.domCal;

  /* Get BDL */
  if (inputBadDomListName_ != ""){
    if (!frame->Has(inputBadDomListName_))
      log_error("Input bad DOM list: %s does not exist.", inputBadDomListName_.c_str());
    else {
      I3VectorOMKeyConstPtr vect_oldBadDom = frame->Get<I3VectorOMKeyConstPtr>(inputBadDomListName_);

      /* Add the BDL to new BDL */
      vector<OMKey>::const_iterator iter_obd = vect_oldBadDom->begin();
      for(;iter_obd != vect_oldBadDom->end(); iter_obd++)
	vect_newBadDom->push_back(*iter_obd);
    }
  }

  /* Check fitting of PMT transit time was succeeded or failed */
  if(checkTransitTime_)
    CheckTransitTime(omgeo, domCal, vect_newBadDom);

  if(!vect_newBadDom->empty()) frame->Put(outputBadDomListName_, vect_newBadDom);

  PushFrame(frame,"OutBox");

  log_debug("Exiting Physics.");

}// end of physics

/* ******************************************************************** */
/* CheckTransitTime                                                     */
/* ******************************************************************** */
void I3OpheliaBadDomFinder::CheckTransitTime(const I3OMGeoMap& omgeo,
					     const std::map<OMKey, I3DOMCalibration>& domCal,
					     I3VectorOMKeyPtr vect_bd){

  log_debug("Entering CheckTransitTime");
  std::map<OMKey, I3DOMCalibration>::const_iterator iter_cal = domCal.begin();

  /* Loop over all DOMs */
  for(; iter_cal != domCal.end(); iter_cal++){
    OMKey omkey = iter_cal->first;

    /* Check if the OM is IceCube DOM */
    I3OMGeo geo = omgeo.find(omkey)->second;
    if(geo.omtype!=I3OMGeo::IceCube && geo.omtype!=I3OMGeo::IceTop)
      continue;
    /* Check if this DOM is already listed as bad DOM */
    vector<OMKey>::const_iterator iter_bd = vect_bd->begin();
    for(;iter_bd != vect_bd->end(); iter_bd++)
      if(omkey==(*iter_bd)) continue;

    /* Take calibration info */
    const I3DOMCalibration& om_cal = iter_cal->second;
    const LinearFit transitTimeFit = om_cal.GetTransitTime();

    /* Check fit parameters of transit time */
    if(transitTimeFit.slope==0.0 && transitTimeFit.intercept==0.0){
      log_info("DOM(%d,%u) is failed to fit transit time.", omkey.GetString(), omkey.GetOM());
      vect_bd->push_back(omkey);
    }

  }// end of loop over DOMs

}// end of CheckTransitTime
