#ifndef SMALLSHOWER_FILTER_I3SMALLSHOWERFILTER_H_INCLUDED
#define SMALLSHOWER_FILTER_I3SMALLSHOWERFILTER_H_INCLUDED

/**
 *  Copyright  (C) 2009
 *  the IceCube Collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author Chen Xu <chen@udel.edu>, Fabian Kislat <fabian.kislat@desy.de>, Bakhtiyar Ruzybayev <bahtiyar@udel.edu>
 *
 */

#include <icetray/I3IcePick.h>

#include <set>
#include <vector>


class I3SmallShowerFilter : public I3IcePick {
public:
  I3SmallShowerFilter(const I3Context&);
  void Configure();
  bool SelectFrame(I3Frame& frame);
  void Finish();

private:
  std::string filterGeometry_;
  std::string pulseKey_;
  std::string resultName_;
  const std::vector< std::set<int> > *station3_, *station4_;

  SET_LOGGER("I3SmallShowerFilter");
};

#endif // SMALLSHOWER_FILTER_I3SMALLSHOWERFILTER_H_INCLUDED
