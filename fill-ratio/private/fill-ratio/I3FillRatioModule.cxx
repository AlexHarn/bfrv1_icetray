/**
 * copyright (C) 2006
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3FillRatioModule.cxx
 * @version
 * @author Doug Rutledge
 * @date 26Aug2006
 */

#include "fill-ratio/I3FillRatioModule.h"
#include "recclasses/I3FillRatioInfo.h"
#include "dataclasses/physics/I3Particle.h"
#include "icetray/OMKey.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/geometry/I3Geometry.h"

I3_MODULE(I3FillRatioModule);

I3FillRatioModule::
  I3FillRatioModule(const I3Context& context) :
  I3ConditionalModule(context), 
  vertexName_("CFirst"), 
  resultName_("FillRatioInfo"),
  recoPulseName_("RecoPulses"), 
  rmsSphereRadius_(3.0), 
  meanSphereRadius_(2.0),
  meanRMSSphereRadius_(1.3),
  nChSphereRadius_(1.3),
  energySphereRadius_(1.3),
  ampWeightPower_(0.0),
  eventsSeen_(0),
  minRadius_(40)
{
  AddParameter("VertexName",
    "Name of the previous vertex reconstruction",
    vertexName_);

  AddParameter("ResultName",
    "The name that the resulting I3FillRatioInfo object will "
    "take in the frame",
    resultName_);

  AddParameter("RecoPulseName",
    "The name of the recopulses to be used",
    recoPulseName_);
  
  AddParameter("SphericalRadiusRMS",
    "The radius (in units of the RMS) of the sphere to be used to "
    "calculate the fill ratio.",
    rmsSphereRadius_);

  AddParameter("SphericalRadiusMean",
    "The radius (in units of the mean) of the sphere used to "
    "calculate the fill ratio.",
    meanSphereRadius_);

  AddParameter("SphericalRadiusMeanPlusRMS",
    "The radius (in units of the mean plus rms) of the sphere used to "
    "calculate the fill ratio.",
    meanRMSSphereRadius_);

  AddParameter("SphericalRadiusNCh",
    "The radius (in units of the SPE Radius) of the sphere used to define the "
    "fill-ratio",
    nChSphereRadius_);

  AddParameter("AmplitudeWeightingPower",
    "The means and RMSs can be weighted by the charge of the hit. "
    "This parameter sets the power for the exponential wieghting.",
  ampWeightPower_);

  AddParameter("BadDOMList", "Name of the BadDOMList in the DetectorStatus frame",
    badDOMListName_);

  AddParameter("BadOMs","A user-defined vector of OMKeys that should not "
    "contribute to the calculation of the fill-ratio. This is especially "
    "useful for an IceCube-only analysis, as the presence of AMANDA geometry "
    "records can add a huge number of unhit OMs to the fill ratio.",
    std::vector<OMKey>());


  AddOutBox("OutBox");

  return;

}//Constructor

I3FillRatioModule::~I3FillRatioModule()
{
}//Destructor

void I3FillRatioModule::Configure()
{

  log_info("Configuring the I3FillRatio");

  GetParameter("VertexName",vertexName_);
  GetParameter("ResultName",resultName_);
  GetParameter("RecoPulseName",recoPulseName_);
  GetParameter("SphericalRadiusRMS",rmsSphereRadius_);
  GetParameter("SphericalRadiusMean",meanSphereRadius_);
  GetParameter("SphericalRadiusMeanPlusRMS",meanRMSSphereRadius_);
  GetParameter("AmplitudeWeightingPower",ampWeightPower_);
  GetParameter("SphericalRadiusNCh",nChSphereRadius_);
  GetParameter("BadDOMList",badDOMListName_);
  GetParameter("BadOms",staticBadDOMList_);
  std::copy(staticBadDOMList_.begin(), staticBadDOMList_.end(),
    std::inserter(badOMs_, badOMs_.begin()));

  log_info("VertexName              : %s",vertexName_.c_str());
  log_info("ResultName              : %s",resultName_.c_str());
  log_info("RecoPulseName           : %s",recoPulseName_.c_str());
  log_info("SphericalRadiusRMS      : %f",rmsSphereRadius_);
  log_info("SphericalRadiusMean     : %f",meanSphereRadius_);

  if ( ampWeightPower_ == 0.0 ) 
  {
     log_info("AmplitudeWeightingPower : %f => no weighting",ampWeightPower_);
  }
  else if ( ampWeightPower_ == 0.0 ) 
  {
    log_info("AmplitudeWeightingPower : %f => weight with charge",
      ampWeightPower_);
  }
  else 
  {
    log_info("AmplitudeWeightingPower : %f => weight with charge to this power",
      ampWeightPower_);
  }
  log_info("BadOms [size of list]   : %d",(int) badOMs_.size());

  eventsSeen_ = 0;

  return;

}//Configure

void I3FillRatioModule::Finish()
{
}//Finish

void I3FillRatioModule::DetectorStatus(I3FramePtr frame)
{
	I3VectorOMKeyConstPtr badDOMList =
	    frame->Get<I3VectorOMKeyConstPtr>(badDOMListName_);
	if (badDOMList) {
		badOMs_.clear();
	        std::copy(staticBadDOMList_.begin(), staticBadDOMList_.end(),
		    std::inserter(badOMs_, badOMs_.begin()));
	        std::copy(badDOMList->begin(), badDOMList->end(),
		    std::inserter(badOMs_, badOMs_.begin()));
	} else if (badDOMListName_.size() > 0)
		log_error("Got a DetectorStatus frame without BadDOMList '%s'!",
		    badDOMListName_.c_str());
	PushFrame(frame);
}

void I3FillRatioModule::Physics(I3FramePtr frame)
{
  bool fillRatioComputed = false;

  log_debug("Entering I3FillRatioModule::Physics() - Event %d",eventsSeen_);
  eventsSeen_++;
  fflush(stdout);

  // fill the result

  I3FillRatioInfoPtr fillRatioInfo = 
    boost::shared_ptr<I3FillRatioInfo>(new I3FillRatioInfo());
  fillRatioInfo->Clear();

  // Do some sanity checks
  if ( frame->find(vertexName_) == frame->end() )
  {
     log_debug("-> could not find vertex named %s in the frame ... "
       "skipping event",
       vertexName_.c_str());

     frame->Put(resultName_,fillRatioInfo); // put in a null result to keep 
                                            // frame structure consistent
     PushFrame(frame,"OutBox");
     return;
  }//end check for vertex

  if ( frame->find(recoPulseName_) == frame->end() )
  {
     log_debug("-> could not find reco pulses named %s in the frame ... "
       "skipping event #%d",
       recoPulseName_.c_str(),eventsSeen_);

     frame->Put(resultName_,fillRatioInfo); // put in a null result to keep 
                                            //frame structure consistent
     PushFrame(frame,"OutBox");
     return;
  }//end check for reco pulses

  // Get the geometry
  const I3Geometry& geometry = frame->Get<I3Geometry>();

  // Define some holders

  double totalCharge = 0.0;

  double runningMeanDistance = 0.0;
  double runningMeanDistanceSquared = 0.0;
  double sumOfAllWeights = 0.0;

  // Get the vertex position

  I3Particle vertex = frame->Get<I3Particle>(vertexName_);  

  if ( vertex.GetFitStatus() != I3Particle::OK ) 
  {
     log_debug("Vertex = %s - fit status != I3Particle::OK ... skipping "
       "event %d",
       vertexName_.c_str(),eventsSeen_);
     frame->Put(resultName_,fillRatioInfo); // put in a null result to 
                                            //keep frame structure consistent
     PushFrame(frame,"OutBox");
     return;
  } // end of IF-block for vertex fit failed, so can't compute fill-ratio

  double vertexX = vertex.GetPos().GetX();
  double vertexY = vertex.GetPos().GetY();
  double vertexZ = vertex.GetPos().GetZ();
  
  // Get the appropriate reco pulse series from the frame.
  // There was a sanity check on this above, so no need to do
  // another.
  I3RecoPulseSeriesMapConstPtr hitOMs = 
    frame->Get<I3RecoPulseSeriesMapConstPtr>(recoPulseName_); 
  
  if (!hitOMs) 
  { 
    log_fatal("Event #%5d: The reco pulse series named %s was null",
      eventsSeen_,recoPulseName_.c_str()); 
  }

  int hitCount = hitOMs->size();
  log_debug("-> Event %5d -- RecoPulseSeries = %s, size = %d",
    eventsSeen_,recoPulseName_.c_str(),hitCount);
  log_debug("-----------------> Vertex = %s, (x,y,z) = (%f,%f,%f)",
    vertexName_.c_str(),vertexX,vertexY,vertexZ);

  I3Map< OMKey, std::vector<I3RecoPulse> >::const_iterator om_iter;

  // This loop will measure the mean and RMS of the distribution of hits about
  // the vertex (which the user often chooses to be the COG or cscdllh vertex)

  for(om_iter  = hitOMs->begin();
      om_iter != hitOMs->end();
      om_iter++)
  {

    OMKey omKey = om_iter->first;
 
    // Get the OM Geo, which actually stores the position
    I3OMGeo omGeo = geometry.omgeo.find(omKey)->second;

    double omX = omGeo.position.GetX(); 
    double omY = omGeo.position.GetY();
    double omZ = omGeo.position.GetZ(); 

    double xDiff = vertexX - omX;
    double yDiff = vertexY - omY;
    double zDiff = vertexZ - omZ;

    double distanceSquared = (xDiff * xDiff) +
      (yDiff * yDiff) + (zDiff * zDiff);

    // declare a holder for the total charge in the waveform

    std::vector<I3RecoPulse>::const_iterator pulse_iter;
    
    double totalChargeThisOM = 0.0;
    for ( pulse_iter  = om_iter->second.begin() ;
          pulse_iter != om_iter->second.end()   ;
          pulse_iter++)
    {
      I3RecoPulse pulse = *pulse_iter;
      totalChargeThisOM += pulse.GetCharge();   // sum up the charge to the 
                                                //total for the waveform
    } //end calculation of the total charge
    totalCharge += totalChargeThisOM;

    if ( totalChargeThisOM == 0.0 )
    {
       continue; // skip this OM since its total charge is zero
    }

    double weight = CalculateWeight(totalChargeThisOM);

    runningMeanDistance += weight * sqrt(distanceSquared);
    runningMeanDistanceSquared += weight * distanceSquared;

    log_trace("----> OM %d-%d : vtx (x,y,z) = (%f,%f,%f) - hit (x,y,z) = "
      "(%f,%f,%f) - diff (x,y,z)=(%f,%f,%f) - charge = %f - weight = %f",
      (om_iter->first).GetString(),(om_iter->first).GetOM(),
      vertexX,vertexY,vertexZ,
      omX,omY,omZ,
      xDiff,yDiff,zDiff,
      totalChargeThisOM,weight);

    sumOfAllWeights += weight;

  } //end loop over reco pulses
 
  // finalize the calculations

  double numOMsInsideRMSSphere     =  0.0; // number of OMs within the 
                                           //RMS sphere
  double numHitOMsInsideRMSSphere  =  0.0; // number of these OMs which 
                                           //were actually hit

  double numOMsInsideMeanSphere    =  0.0; // number of OMs within the 
                                           //Mean sphere
  double numHitOMsInsideMeanSphere =  0.0; // number of these OMs which 
                                           //were actually hit
  double numOMsInsideMeanPlusRMSSphere    =  0.0; // number of OMs within the 
                                           //Mean Plus RMS sphere
  double numHitOMsInsideMeanPlusRMSSphere =  0.0; // number of these OMs which 
                                           //were actually hit
  double numOMsInsideNChSphere =      0.0;
  double numHitOMsInsideNChSphere =   0.0;

  double numOMsInsideEnergySphere =      0.0;
  double numHitOMsInsideEnergySphere =   0.0;
  double rmsDistance =               -1.0; // rms distance used for RMS 
                                           //sphere sizing
  double nChSPEDistance =               -1.0; 
  double energySPEDistance =         -1.0; 
  double fillRatio =                 -1.0; // fill fraction for RMS sphere
  double fillRatioFromMean =         -1.0; // fill fraction for Mean sphere
  double fillRatioFromNCh =          -1.0; // fill fraction for NCh sphere
  double fillRatioFromEnergy =       -1.0; // fill fraction for NCh sphere
  double fillRatioFromMeanPlusRMS =  -1.0; // fill fraction for Mean+RMS sphere

  if ( ( totalCharge > 0.0 ) && ( fabs(sumOfAllWeights) > 0.0 ) ) 
  {

    runningMeanDistance /=        sumOfAllWeights;
    runningMeanDistanceSquared /= sumOfAllWeights;

    rmsDistance = sqrt(runningMeanDistanceSquared - 
      runningMeanDistance * runningMeanDistance); 

    double nChLog10E = 0;
    nChLog10E = EstimateEnergyFromNCh(hitCount);
    //elsei log10E = log10(vertex.GetEnergy());
   
    nChSPEDistance = EstimateSPERadiusFromEnergy(nChLog10E);

    if(std::isnan(vertex.GetEnergy())) energySPEDistance = 1.0;
    else energySPEDistance = EstimateSPERadiusFromEnergy(log10(vertex.GetEnergy()));

    // Now, loop through the geometry, looking for all OMs within the
    //      sphere Radii

     I3OMGeoMap geoMap = geometry.omgeo;
  
     I3Map<OMKey, I3OMGeo>::const_iterator geo_iter;

     for( geo_iter  = geoMap.begin() ;
          geo_iter != geoMap.end()   ;
          geo_iter++ )
     {

        const OMKey& omKey = geo_iter->first;

        if( badOMs_.find(omKey) != badOMs_.end() ) 
        {
           log_trace("Digesting Geometry - Found bad OM: (%d,%d)",
                     omKey.GetString(), omKey.GetOM());
           continue;
        } // remove BadOMs (specified via a python argument) from consideration

        I3OMGeo omGeo = geo_iter->second;

        double omX = omGeo.position.GetX();
        double omY = omGeo.position.GetY();
        double omZ = omGeo.position.GetZ();

        double xDiff = vertexX - omX;
        double yDiff = vertexY - omY;
        double zDiff = vertexZ - omZ;

        double distanceToVertexSquared = (xDiff * xDiff) +
          (yDiff * yDiff) + (zDiff * zDiff);

        if ( distanceToVertexSquared <= 
          (rmsDistance * rmsDistance * rmsSphereRadius_ * rmsSphereRadius_) )
        { 
           numOMsInsideRMSSphere++;                  // count up the number 
                                                     //of OMs within sphere
           if (hitOMs->find(omKey) != hitOMs->end()) // check to see if this 
                                                     //OM was hit
           {
              numHitOMsInsideRMSSphere++;            // count up the number 
                                                     //of such OMs which 
                                                     //were hit
           } //end case of hit om inside rms sphere
        } //end case of an om inside rms sphere

        if ( distanceToVertexSquared <= 
          (runningMeanDistance * runningMeanDistance *
           meanSphereRadius_ * meanSphereRadius_) )
        { 
           numOMsInsideMeanSphere++;         //check to see if this OM was hit

           if ( hitOMs->find(omKey) != hitOMs->end())
           {
              numHitOMsInsideMeanSphere++;      //count up the hits
           } //end count up of hit om inside mean sphere
        } //end count up of om inside mean sphere

        if ( distanceToVertexSquared <= 
          ((runningMeanDistance+ rmsDistance) * 
           (runningMeanDistance + rmsDistance)*
           meanRMSSphereRadius_ * meanRMSSphereRadius_) )
        { 
           numOMsInsideMeanPlusRMSSphere++;         //check to see if this OM was hit

           if ( hitOMs->find(omKey) != hitOMs->end())
           {
              numHitOMsInsideMeanPlusRMSSphere++;      //count up the hits
           } //end count up of hit om inside mean sphere
        } //end count up of om inside mean sphere

        if ( distanceToVertexSquared <= 
           (nChSPEDistance * nChSPEDistance *
           nChSphereRadius_ * nChSphereRadius_) )
        { 
           numOMsInsideNChSphere++;         //check to see if this OM was hit

           if ( hitOMs->find(omKey) != hitOMs->end())
           {
              numHitOMsInsideNChSphere++;      //count up the hits
           } //end count up of hit om inside mean sphere
        } //end count up of om inside mean sphere

        if ( distanceToVertexSquared <= 
           (energySPEDistance * energySPEDistance *
           energySphereRadius_ * energySphereRadius_) )
        { 
           numOMsInsideEnergySphere++;         //check to see if this OM was hit

           if ( hitOMs->find(omKey) != hitOMs->end())
           {
              numHitOMsInsideEnergySphere++;      //count up the hits
           } //end count up of hit om inside mean sphere
        } //end count up of om inside mean sphere
     } //end loop over all OMs in the geometry

     // either calculate the actual fill Ratio, or set it to zero, 
     // depending on whether or not there actually was a hit inside
     // the sphere.

     fillRatio = (numOMsInsideRMSSphere  < 1) ? -1.0 :
       (numHitOMsInsideRMSSphere / numOMsInsideRMSSphere);
     fillRatioFromMean = (numOMsInsideMeanSphere < 1) ? -1.0 : 
       (numHitOMsInsideMeanSphere / numOMsInsideMeanSphere);
     fillRatioFromNCh = (numOMsInsideNChSphere < 1) ? -1.0 : 
       (numHitOMsInsideNChSphere / numOMsInsideNChSphere);
     fillRatioFromEnergy = (numOMsInsideEnergySphere < 1) ? -1.0 : 
       (numHitOMsInsideEnergySphere / numOMsInsideEnergySphere);
     fillRatioFromMeanPlusRMS = (numOMsInsideMeanPlusRMSSphere < 1) ? -1.0 :
       (numHitOMsInsideMeanPlusRMSSphere/numOMsInsideMeanPlusRMSSphere);

     log_debug("-----> Mean Inside/Total Hit Count         : %f/%f",
       numHitOMsInsideMeanSphere,numOMsInsideMeanSphere);
     log_debug("-----> RMS Inside/Total Hit Count          : %f/%f",
       numHitOMsInsideRMSSphere,numOMsInsideRMSSphere);

     fillRatioComputed = true;

  } // end of IF-block for able to compute the fill-ratio

  else
  {
     fillRatio                        = -1.0;
     fillRatioFromMean                = -1.0;
     fillRatioFromMeanPlusRMS         = -1.0;
     fillRatioFromNCh                 = -1.0;
     fillRatioFromEnergy              = -1.0;
     rmsDistance                      = -1.0; // rms distance used for RMS sphere 
                                       //sizing
     runningMeanDistance              = -1.0; // mean distance used for Mean sphere 
                                       //sizing
     nChSPEDistance                   = -1.0;
     energySPEDistance                = -1.0;
     fillRatioComputed               = false;

  } // end of IF-block for no hits or some such that which prevents us from 
    //computing the fill-ratio

  // fill the result output frame object depending on how the calculation went

  if ( fillRatioComputed ) 
  {
     fillRatioInfo->SetMeanDistance(runningMeanDistance);
     fillRatioInfo->SetRMSDistance(rmsDistance);
     fillRatioInfo->SetNChDistance(nChSPEDistance);
     fillRatioInfo->SetEnergyDistance(energySPEDistance);

     fillRatioInfo->SetFillRatioFromRMS(fillRatio);
     fillRatioInfo->SetFillRatioFromMean(fillRatioFromMean);
     fillRatioInfo->SetFillRatioFromMeanPlusRMS(fillRatioFromMeanPlusRMS);
     fillRatioInfo->SetFillRatioFromNCh(fillRatioFromNCh);
     fillRatioInfo->SetFillRatioFromEnergy(fillRatioFromEnergy);

     fillRatioInfo->SetFillRadius(rmsDistance * rmsSphereRadius_);
     fillRatioInfo->SetFillRadiusFromMean(runningMeanDistance *
        meanSphereRadius_);
     fillRatioInfo->SetFillRadiusFromMeanPlusRMS(
       (runningMeanDistance + rmsDistance) * meanRMSSphereRadius_);
     fillRatioInfo->SetFillRadiusFromNCh(nChSphereRadius_ * nChSPEDistance);
     fillRatioInfo->SetFillRadiusFromEnergy(nChSphereRadius_ * energySPEDistance);

     fillRatioInfo->SetHitCount(hitCount);
  } 
  else 
  {
     fillRatioInfo->Clear();
  }

  log_debug("-----> totalCharge                         : %f",
            totalCharge);
  log_debug("-----> sumOfAllWeights                     : %f",
            sumOfAllWeights);
  log_debug("-----> MeanDistance                        : %f (%f)",
            runningMeanDistance,
	    fillRatioInfo->GetMeanDistance());
  log_debug("-----> RMSDistance                         : %f (%f)",
            rmsDistance,
	    fillRatioInfo->GetRMSDistance());
  log_debug("-----> FillRatioFromRMS                    : %e (%e)",
            fillRatio,
            fillRatioInfo->GetFillRatioFromRMS());
  log_debug("-----> FillRatioFromMean                   : %e (%e)",
            fillRatioFromMean,
	    fillRatioInfo->GetFillRatioFromMean());
  log_debug("-----> FillRadiusFromRMS*rmsSphereRadius_  : %f (%f)",
            (runningMeanDistance * meanSphereRadius_),
            fillRatioInfo->GetFillRadiusFromMean());
  log_debug("-----> FillRadiusFromMean*rmsSphereRadius_ : %f (%f)",
            (rmsDistance * rmsSphereRadius_),
            fillRatioInfo->GetFillRadiusFromRMS());
  log_debug("-----> HitCount                            : %d (%f)",
            hitCount,fillRatioInfo->GetHitCount());

  //push the result into the frame  

  frame->Put(resultName_,fillRatioInfo);
  PushFrame(frame,"OutBox");

  log_debug("Exitting I3FillRatioModule::Physics()");
  fflush(stdout);

  return;

}//Physics


double I3FillRatioModule::CalculateWeight(double charge)
{
  //declare and initialize the return value;
  double weight = 0.0;

  //if the weight power os zero, weight all OMs equally.
  if (ampWeightPower_ == 0.0)
    weight = 1.0;

  //if the weight is 1.0, weight all OMs by the recorded charge
  else if (ampWeightPower_ == 1.0)
    weight = charge;
  //else, weight all OMs by an exponential distribution.
  else
    weight = std::pow(charge, ampWeightPower_);

  return weight;
}//CalculateWeight


double I3FillRatioModule::EstimateEnergyFromNCh(int nCh)
{
  double energyEstimate = 0.0;

  // Always assume this is for IceCube. This BREAKS AMANDA code.
  energyEstimate = 0.084 + 1.83*log10(nCh);
  
  return energyEstimate;
}


double I3FillRatioModule::EstimateSPERadiusFromEnergy(double log10E)
{
  // Assuming IceCube again.
  double speRadius = -0.6441 + 36.9* log10E;

  if( speRadius < minRadius_) speRadius = minRadius_;

  return speRadius;
}//EstimateSPERadiusFromNCh
