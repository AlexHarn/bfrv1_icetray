/**
 * @brief declaration of the I3StartStopLProb class
 *
 * @file I3StartStopLProb.h   
 * @version $Revision: 54885 $
 * @date: $Date: 2009-05-11 15:24:36 +0200 (Mon, 11 May 2009) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This module calculates the likelihood parameter for a given muon track to be infinite, starting, stopping or contained. An I3StartStopParams object is placed in the frame.
 * The hits are arranged along the track. Some DOMs without hit might be between the first hit DOM and the detector border. For those DOMs the probability to have no hit is calculated with two different assumptions: a track passing but unseen, or no track in this area. The same procedure is done for the stop point. 
 * The ratio of the two likelihood values is a measure for the starting/stopping probability. More information can be found at <https://wiki.icecube.wisc.edu/index.php/FiniteReco.I3StartStopLProb>
*/

#ifndef I3STARTSTOPLPROB_H_INCLUDED
#define I3STARTSTOPLPROB_H_INCLUDED

#include "recclasses/I3StartStopParams.h"
#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3Particle.h"


class I3StartStopLProb : public I3ConditionalModule
{
 public:

  I3StartStopLProb(const I3Context& ctx);
  ~I3StartStopLProb();
  void Configure();
  void Physics(I3FramePtr frame);

 private:
  I3StartStopLProb();
  I3StartStopLProb(const I3StartStopLProb& source);
  I3StartStopLProb& operator=(const I3StartStopLProb& source);

  double GetProbability(const I3ParticlePtr& track)const;
  
  std::string fitName_;
  std::string serviceName_;
  I3EventLogLikelihoodBasePtr finite_;
  
  SET_LOGGER("I3StartStopLProb");
};

#endif
