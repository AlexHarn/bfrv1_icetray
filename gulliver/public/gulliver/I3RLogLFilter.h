/**
 * copyright (C) 2007
 * the IceCube collaboration
 * $Id:$
 *
 * @file I3RLogLFilter.h
 * @version $Revision$
 * @author Anna Franckowiak
 * @date Oct 22 2007
 */

#ifndef I3RLOGLFILTER_H_INCLUDED
#define I3RLOGLFILTER_H_INCLUDED

class I3Context;
class I3Frame;


#include <string>

#include <icetray/I3IcePick.h>
#include <icetray/I3Logging.h>


/**
 * @brief An I3IcePick to select events which have
 * a particle with reduced log(likelihood) obtained by a
 * likelihood fit smaller than the given value MaxRLogL.
 * Note that a small reduced log(likelhood) indicates a
 * good reconstruction quality.
 */
class I3RLogLFilter : public I3IcePick
{
 public:
  explicit I3RLogLFilter(const I3Context& context);
  virtual ~I3RLogLFilter();
  void Configure();
  bool SelectFrame(I3Frame& frame);

 private:
  std::string particleKey_;
  double maxRLogL_;


  // private copy constructor and assignment
  I3RLogLFilter(const I3RLogLFilter&);
  I3RLogLFilter& operator=(const I3RLogLFilter&);


  SET_LOGGER("I3RLogLFilter");
};

#endif // I3RLOGLFILTER_H_INCLUDED
