/**
 *
 * Definition of I3CascadeFitCoreRemoval
 *
 * (c) 2009
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3CascadeFitCoreRemoval.cxx
 * @date $Date$
 * @author rutledge
 *
 */

#include "core-removal/I3CascadeFitCoreRemoval.h"
#include "dataclasses/I3Double.h" 
#include "icetray/I3Units.h"

I3_MODULE(I3CascadeFitCoreRemoval);

I3CascadeFitCoreRemoval :: I3CascadeFitCoreRemoval(const I3Context& context) : 
    I3Module(context), vertexName_("VertexReco"), 
    recoPulseInputName_("OfflinePulses"),
    recoPulseOutputName_("CoreRemovedPulses"),
    radiusName_("SPERadius"), nChCalib_(false),
    speFraction_(0.8), lambdaAttn_(29.0), cLambda_(-105),
    minRadius_(5.0), splineConst_(0.0), criticalEnergy_(750)
{
  AddParameter("VertexName",
    "INPUT: The name of a prior vertex reconstrution, about which the pulses will be split",
     vertexName_);

  AddParameter("InputRecoPulseSeries",
    "INPUT: The name of the Reco Pulse Series map to be split",
    recoPulseInputName_);

  AddParameter("OutputRecoPulseSeries",
    "OUTPUT: The name of the Corona pulses",
    recoPulseOutputName_);
  AddParameter("CorePulsesName",
    "OUTPUT: The name of the Core pulses",
    corePulsesOutputName_);

  AddParameter("NChCalib",
    "SWITCH: Obtain the energy estimate from the nchannels (true), or from an "
    "energy fit (false)",
    nChCalib_);

  AddParameter("SPEFraction",
    "INPUT: A scaling factor applied to the SPE Radius, in order to tune the radius of removal",
    speFraction_);

  AddParameter("LambdaAttn",
    "INPUT: The effective attenuation length in the ice. Determnes the slope for the energy to "
    "SPE Radius calibration",
    lambdaAttn_);

  AddParameter("SPERadiusName",
    "OUTPUT: The name of the SPERadius",
    radiusName_);

  AddParameter("CriticalEnergy",
    "INPUT: The energy below which the R_SPE -> Energy mapping is no longer linear (will be patched "
    "to a constant value by a 2nd order poly spline",
    criticalEnergy_);

  AddParameter("MinimumSPERadius",
    "The absolute minimum distance to be used in the core removal. Used in the spline calculation",
    minRadius_);

  AddOutBox("OutBox");
}//Constructor

I3CascadeFitCoreRemoval :: ~I3CascadeFitCoreRemoval()
{
}//Destructor

void I3CascadeFitCoreRemoval :: Configure()
{
  GetParameter("VertexName",vertexName_);
  GetParameter("InputRecoPulseSeries",recoPulseInputName_);
  GetParameter("OutputRecoPulseSeries",recoPulseOutputName_);
  GetParameter("CorePulsesName",corePulsesOutputName_);
  GetParameter("SPEFraction",speFraction_);
  GetParameter("LambdaAttn", lambdaAttn_);
  GetParameter("NChCalib",nChCalib_);
  GetParameter("SPERadiusName",radiusName_);
  GetParameter("MinimumSPERadius",minRadius_);
  GetParameter("CriticalEnergy",criticalEnergy_);

  CalculateSpline(); 
}//Configure

void I3CascadeFitCoreRemoval :: Finish()
{
}//Finish

void I3CascadeFitCoreRemoval :: Physics(I3FramePtr frame)
{
  log_debug("Entering : %s",__PRETTY_FUNCTION__);

  if(!frame->Has(vertexName_))
  {
    log_error("The Frame is missing a vertex named %s",
      vertexName_.c_str());

    PushEmptyResultIntoFrameToHandleBadCase(frame);
    return;
  }//end sanity check

  if(!frame->Has(recoPulseInputName_))
  {
    log_error("The Frame is missing a reco pulse series named %s",
      recoPulseInputName_.c_str());

    PushEmptyResultIntoFrameToHandleBadCase(frame);

    return;
  }//end sanity check

  I3RecoPulseSeriesMapConstPtr inputPulseSeriesMap = 
    frame->Get<I3RecoPulseSeriesMapConstPtr>(recoPulseInputName_);

  I3ParticleConstPtr vertex =
    frame->Get<I3ParticleConstPtr>(vertexName_);

  if(!vertex)
  {
    log_error("The seed vertex fit was null!");

    PushEmptyResultIntoFrameToHandleBadCase(frame);

    return;
  }//end sanity check

  if(vertex->GetFitStatus() != I3Particle :: OK)
  {
    log_error("The seed vertex fit had a bad status");

    PushEmptyResultIntoFrameToHandleBadCase(frame);

    return;
  }//end sanity check

  const I3Geometry& geometry = 
    frame->Get<I3Geometry>();

  double speRadius = 0.0;

  if(nChCalib_)
  {
    int nCh = inputPulseSeriesMap->size();
    const double energy = EstimateEnergyFromNCh(nCh);
    speRadius = CalculateSPERadius(energy);
  }
  else 
  {
    speRadius = CalculateSPERadius(vertex);
  }

  const double hitRemovalRadius = speRadius * speFraction_;

  SplitPulsesByEnergyRadius(frame, inputPulseSeriesMap, hitRemovalRadius, vertex,
       geometry);
}//Physics

double I3CascadeFitCoreRemoval :: CalculateSPERadius(I3ParticleConstPtr vertex)
{
  double energy = vertex->GetEnergy() / (I3Units :: GeV);

  return CalculateSPERadius(energy);
}//CalculateSPERadius

double I3CascadeFitCoreRemoval :: CalculateSPERadius(double energy)
{
  double result = minRadius_ * (I3Units :: meter);

  //N.B. that these are the natural logs, not base 10. The SPE Radius formula
  //is based on an approximation based on the expected number of PEs from
  //PHit. There is an exponential in that formula, so we get log_e here.
  if (std::isnan(energy))
  { 
    return result; 
  }

  if(energy > criticalEnergy_)
  {
    result = lambdaAttn_* log(energy) + cLambda_;
  }else if(energy <= criticalEnergy_)
  {
    double logE = log(energy);
    result = minRadius_ + splineConst_ * logE * logE;
  }

  return result;
}//CalculateSPERadius


double I3CascadeFitCoreRemoval::EstimateEnergyFromNCh( int nCh )
{
    double result = 0.0;

    if( nCh < 1 )
    {
        log_error( "nCh must be set to an integer > 0." );

        result = 3.;  // Fixme: Why 3?
    }    
    else
    {
        result = 0.084 + 1.83 * log10( nCh );
    }

  return pow( 10, result );
}//EstimateEnergyFromNCh


void I3CascadeFitCoreRemoval :: CalculateSpline()
{
  //A lot of simplifications went into this formula. First, the quadratic patch
  //is assumed to be centered at "0." Second, it is assumed that the linear fit 
  //is such that it doesn't break the spline.

  //N.B. This is Log base e, not base 10. This is because the 
  double logEc = log(criticalEnergy_);
  splineConst_ = lambdaAttn_/(2.0*logEc);

  cLambda_ = minRadius_ + splineConst_ * logEc * logEc - lambdaAttn_ * logEc;

}//CalculateSpline 

void I3CascadeFitCoreRemoval :: PushEmptyResultIntoFrameToHandleBadCase(I3FramePtr frame)
{
    I3RecoPulseSeriesMapMaskPtr core_mask(new I3RecoPulseSeriesMapMask(*frame, recoPulseInputName_));
    I3RecoPulseSeriesMapMaskPtr corona_mask(new I3RecoPulseSeriesMapMask(*frame, recoPulseInputName_));

    core_mask->SetNone();
    corona_mask->SetNone();

    frame->Put(recoPulseOutputName_,corona_mask);
    frame->Put(corePulsesOutputName_,core_mask);
    frame->Put(radiusName_,boost::shared_ptr<I3Double>(new I3Double(-1)));

    PushFrame(frame);
}//PushEmptyResultIntoFrameToHandleBadCase

void
I3CascadeFitCoreRemoval::SplitPulsesByEnergyRadius(
     I3FramePtr frame,
     I3RecoPulseSeriesMapConstPtr inputPulseSeriesMap, 
     const double hitRemovalRadius, I3ParticleConstPtr vertex,
     const I3Geometry& geometry) 
{
  I3RecoPulseSeriesMapMaskPtr core_mask(new I3RecoPulseSeriesMapMask(*frame, recoPulseInputName_));
  I3RecoPulseSeriesMapMaskPtr corona_mask(new I3RecoPulseSeriesMapMask(*frame, recoPulseInputName_));

  double vertexX = vertex->GetX()/(I3Units :: meter);
  double vertexY = vertex->GetY()/(I3Units :: meter);
  double vertexZ = vertex->GetZ()/(I3Units :: meter);

  I3Map<OMKey, std::vector<I3RecoPulse> > :: const_iterator om_iter;

  for(om_iter = inputPulseSeriesMap->begin();
    om_iter != inputPulseSeriesMap->end();
    om_iter++)
  {
    OMKey omKey = om_iter->first;

    const I3OMGeo& omGeo = geometry.omgeo.find(omKey)->second;
  
    double omX = omGeo.position.GetX()/(I3Units :: meter);
    double omY = omGeo.position.GetY()/(I3Units :: meter);
    double omZ = omGeo.position.GetZ()/(I3Units :: meter);

    double distance = sqrt(
      (omX - vertexX) * (omX - vertexX) + 
      (omY - vertexY) * (omY - vertexY) +
      (omZ - vertexZ) * (omZ - vertexZ));

    if(distance < (hitRemovalRadius/(I3Units :: meter)))
        corona_mask->Set(omKey, false);
    else
        core_mask->Set(omKey, false);
        
  }//end loop over OMs 
 
  frame->Put(recoPulseOutputName_,corona_mask);
  frame->Put(corePulsesOutputName_,core_mask);
  frame->Put(radiusName_,boost::shared_ptr<I3Double>(new I3Double(hitRemovalRadius)));

  PushFrame(frame);
}
