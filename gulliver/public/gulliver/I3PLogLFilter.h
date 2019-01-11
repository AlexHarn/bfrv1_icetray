/**
 * copyright (C) 2007
 * the IceCube collaboration
 * $Id:$
 *
 * @file I3PLogLFilter.h
 * @version $Revision$
 * @author Anna Franckowiak
 * @date Oct 22 2007
 */

#ifndef I3PLOGLFILTER_H_INCLUDED
#define I3PLOGLFILTER_H_INCLUDED

class I3Context;
class I3Frame;


#include <string>

#include <icetray/I3IcePick.h>
#include <icetray/I3Logging.h>


/**
 * @brief An I3IcePick to select events which have
 * a particle with reduced log(likelihood) obtained by a 
 * likelihood fit smaller than the given value MaxPLogL.
 * Note that a small reduced log(likelhood) indicates a
 * good reconstruction quality.
 */
class I3PLogLFilter : public I3IcePick
{
 public:
  explicit I3PLogLFilter(const I3Context& context);
  virtual ~I3PLogLFilter();
  void Configure();
  bool SelectFrame(I3Frame& frame);

 private:
  std::string pulseSeriesName_;
  std::string particleKey_;
  std::string cutValuesKey_;
  double subFromNCh_;
  double maxPLogL_;


  // private copy constructor and assignment
  I3PLogLFilter(const I3PLogLFilter&);
  I3PLogLFilter& operator=(const I3PLogLFilter&);


  SET_LOGGER("I3PLogLFilter");
};

#endif // I3PLOGLFILTER_H_INCLUDED
