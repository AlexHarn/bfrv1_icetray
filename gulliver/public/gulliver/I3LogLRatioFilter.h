/**
 * copyright (C) 2007
 * the IceCube collaboration
 * $Id:$
 *
 * @file I3LogLRatioFilter.h
 * @version $Revision$
 * @author Anna Franckowiak
 * @date Oct 22 2007
 */

#ifndef I3LOGLRATIOFILTER_H_INCLUDED
#define I3LOGLRATIOFILTER_H_INCLUDED

class I3Context;
class I3Frame;


#include <string>

#include <icetray/I3IcePick.h>
#include <icetray/I3Logging.h>

/**
 * @brief An I3IcePick to select events with a maximum
 * log(likelihood ratio). 
 * The ratio logL1-logL2 is calculated from two different 
 * likelihood reconstructions. Both reconstructions have to
 * be applied before this modul and the resulting particle keys
 * have to be contained in the frame.
 */
class I3LogLRatioFilter : public I3IcePick
{
 public:
  explicit I3LogLRatioFilter(const I3Context& context);
  virtual ~I3LogLRatioFilter();
  void Configure();
  bool SelectFrame(I3Frame& frame);

 private:
  std::string particleKey1_;
  std::string particleKey2_;
  double maxLogLRatio_;


  // private copy constructor and assignment
  I3LogLRatioFilter(const I3LogLRatioFilter&);
  I3LogLRatioFilter& operator=(const I3LogLRatioFilter&);


  SET_LOGGER("I3LogLRatioFilter");
};

#endif // I3LOGLRATIOFILTER_H_INCLUDED
