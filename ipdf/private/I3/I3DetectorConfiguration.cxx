/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.3 $
    @date $Date$
    @author C Wiebusch <wiebusch@physik.uni-wuppertal.de>
        and S Robbins  <robbins@physik.uni-wuppertal.de>

    @brief Implementation of example DetectorConfiguration representation
*/

#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3OmReceiver.h"
#include <iostream>

using namespace IPDF;

I3DetectorConfiguration::I3DetectorConfiguration(const I3Geometry& i3geom)
{
  I3OMGeoMap::const_iterator giter = i3geom.omgeo.begin();
  I3OMGeoMap::const_iterator gend  = i3geom.omgeo.end();

  for( ; giter != gend ; ++giter ) {
    // Make a new I3OmReceiver and add it to the hash
    const I3OMGeo& i3om( giter->second );
    IPDF::I3OmReceiver* omr( new I3OmReceiver(i3om) ); 
    omReceivers_.push_back( omr );
    i3omHash_[giter->first] = omr;
  }
}

IPDF::I3DetectorConfiguration::~I3DetectorConfiguration()
{
  for(I3DetectorConfiguration::iterator omriter = omReceivers_.begin();
      omriter != omReceivers_.end(); ++omriter) {
    delete *omriter;
  }
}

std::ostream& IPDF::operator<<(std::ostream& os,
    const I3DetectorConfiguration& detcon) {
  os<<"I3DetectorConfiguration: ( "
      <<detcon.size()<<" OM receivers: ";

  for(IPDF::I3DetectorConfiguration::const_iterator oiter = detcon.omReceivers_.begin();
      oiter != detcon.omReceivers_.end(); ++oiter) {
    os<<(**oiter)<<"; ";
  }
      
  os<<" )";
  return os;
}
