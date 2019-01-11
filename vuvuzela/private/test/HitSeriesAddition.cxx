#include <I3Test.h>

#include "icetray/I3Units.h"
#include "icetray/OMKey.h"
#include "vuvuzela/VuvuzelaFunctions.h"

TEST_GROUP(HitAddition);

TEST(HitSeriesMapAddition){
  I3MCPESeriesMap temp;
  {
    I3MCPESeries hitSeries;
    I3MCPE hit;
    hitSeries.push_back(hit);
    OMKey omkey(21,30);
    temp[omkey] = hitSeries;
  }
  I3MCPESeriesMapConstPtr firstterm(new I3MCPESeriesMap(temp));

  I3MCPESeriesMap temp2;
  {
    I3MCPESeries hitSeries;
    I3MCPE hit;
    hitSeries.push_back(hit);
    OMKey omkey(21,30);
    temp2[omkey] = hitSeries;

    OMKey omkey2(29,30);
    temp2[omkey2] = hitSeries;
  }
  I3MCPESeriesMapConstPtr secondterm(new I3MCPESeriesMap(temp2));

  I3MCPESeriesMapConstPtr lhs = AddHitMaps(firstterm, secondterm, I3ParticleIDMapPtr());

  int n_hits(0);
  I3MCPESeriesMap::const_iterator iter;
  for(iter = lhs->begin(); iter != lhs->end(); ++iter)
    n_hits += static_cast<int>(iter->second.size());

  ENSURE(n_hits == 3,"Wrong number of hits in the map.");
}

TEST(HitSeriesMapAddition_withSideTable){
  I3MCPESeriesMap temp;
  {
    I3MCPESeries hitSeries;
    I3MCPE hit(1,100.); //give this some time
    hitSeries.push_back(hit);
    OMKey omkey(21,30);
    temp[omkey] = hitSeries;
  }
  I3MCPESeriesMapPtr firstterm(new I3MCPESeriesMap(temp));
  
  //make up a supposed parent particle for our one input hit
  I3ParticleIDMapPtr firstinfo(new I3ParticleIDMap);
  (*firstinfo)[OMKey(21,30)][I3ParticleID(22,17)].push_back(0);
  
  I3MCPESeriesMap temp2;
  {
    I3MCPESeries hitSeries;
    I3MCPE hit(1,50.); //give this a time earlier than the time we gave in the other map
    hitSeries.push_back(hit);
    OMKey omkey(21,30);
    temp2[omkey] = hitSeries;
    
    OMKey omkey2(29,30);
    temp2[omkey2] = hitSeries;
  }
  I3MCPESeriesMapConstPtr secondterm(new I3MCPESeriesMap(temp2));
  
  I3MCPESeriesMapConstPtr lhs = AddHitMaps(firstterm, secondterm, firstinfo);
  
  int n_hits(0);
  I3MCPESeriesMap::const_iterator iter;
  for(iter = lhs->begin(); iter != lhs->end(); ++iter)
    n_hits += static_cast<int>(iter->second.size());
  
  ENSURE(n_hits == 3,"Wrong number of hits in the map.");
  ENSURE((*firstinfo)[OMKey(21,30)][I3ParticleID(22,17)][0]==1,"Adding hits should update indices in the side table");
}
