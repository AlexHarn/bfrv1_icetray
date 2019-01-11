/**
 * @brief declaration of the I3LengthLLH class
 *
 * @file I3LengthLLH.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This module reconstructs the length of a given track. An I3PDFService depending on the start and stop point is required (I3GulliverFinitePhPnh). The module calculates the likelihood value for each length at each position on the track. The best-fitting length is returned, independent of the true event shape. This module does not determine whether an an event is starting or not. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.I3LengthLLH>)
 */

#ifndef I3LENGTHLLH_H_INCLUDED
#define I3LENGTHLLH_H_INCLUDED

#include "icetray/I3Module.h"


class I3LengthLLH : public I3Module
{
 public:

  I3LengthLLH(const I3Context& ctx);
  ~I3LengthLLH();
  void Configure();
  void Physics(I3FramePtr frame);
  
 
private:
  I3LengthLLH();
  I3LengthLLH(const I3LengthLLH& source);
  I3LengthLLH& operator=(const I3LengthLLH& source);
  double GetProbability(const I3ParticlePtr& track)const;
  std::map<double,double> DoLengthIter(const I3ParticlePtr& partPtr)const ;
  
  I3EventLogLikelihoodBasePtr finite_;
  std::string fitName_;
  std::string serviceName_;
  double stepSize_;
  double maxLength_;
  SET_LOGGER("I3LengthLLH");

};

#endif
