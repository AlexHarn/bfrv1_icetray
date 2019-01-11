/*
 *  $Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#include "credo/I3CredoDOMCache.h"
#include <icetray/serialization.h>

/******************************************************************************/
        
void I3CredoDOMCache::Reset() { 
    npe = nPoissonPE = baseContribution = outOfBoundContribution = firstPulseTime = 0;
    llhcontrib = expectedAmpliude = amplitudeCorrection = 0; 
    saturated = false;
    relativedomefficiency = 1.0;
    type = UNSPECIFIED;
}

/******************************************************************************/

template <class Archive>
void I3CredoDOMCache::serialize(Archive& ar, unsigned version) {
  ar & make_nvp("position",  position );
  ar & make_nvp("npe", npe );
  ar & make_nvp("nPoissonPE",  nPoissonPE );
  ar & make_nvp("baseContribution",  baseContribution );
  ar & make_nvp("outOfBoundContribution",  outOfBoundContribution );
  ar & make_nvp("firstPulseTime",  firstPulseTime );
  ar & make_nvp("llhcontrib",  llhcontrib );
  ar & make_nvp("expectedAmpliude",  amplitudeCorrection );
  ar & make_nvp("amplitudeCorrection",  amplitudeCorrection );
  ar & make_nvp("saturated",  saturated );
  ar & make_nvp("type",  type );
  ar & make_nvp("relativedomefficiency",  relativedomefficiency );
}

/******************************************************************************/

I3_CLASS_VERSION(I3CredoDOMCache, 0);
I3_SERIALIZABLE(I3CredoDOMCache);

