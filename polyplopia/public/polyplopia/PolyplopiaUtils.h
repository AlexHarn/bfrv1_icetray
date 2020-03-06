#ifndef MP_DIPLOPIA_UTILS_H_INLCUDED
#define MP_DIPLOPIA_UTILS_H_INLCUDED

#include "icetray/I3TrayHeaders.h"
#include "icetray/I3Logging.h"
#include "icetray/I3Frame.h"
#include "icetray/open.h"
#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3String.h"
#include "simclasses/I3MMCTrack.h"
#include "simclasses/I3MCPE.h"

#include <fstream>
#include <cassert>
#include <cmath>

/**
 * copyright  (C) 2006
 * The IceCube Collaboration
 *
 * @date $Date:$
 * @author Juan Carlos Diaz Velez
 *                                                                       
 * @brief Utilities for merging MCTrees MCInfo and MCHitSeriesMaps in separate 
 *	events to produce coincident in-ice events.
 *
 */

namespace PolyplopiaUtils
{

double
GetEarliestInIceTime(const I3MCTree& t);

I3MCTree::optional_value
GetEarliestInIce(const I3MCTree& t);

I3MCTree::optional_value
GetLatestInIce(const I3MCTree& t);

double
GetLatestInIceTime(const I3MCTree& t);

double
GetFirstHitTime(const I3MCPESeriesMap& hitmap);

/**
  * MergeMMCInfo - Adds the contents of the second MMCInfoList to the first
  *
  * @param dest - list we will add MMCInfo to
  * @param src  - list we will copy MMCInfo from
  */
  void MergeMMCInfo(I3MMCTrackList& dest, const I3MMCTrackList& src, double timeOffset);

/**
  * MergePrimaries - Add I3MCTree src at root level of tree dest
  *
  * @param dest - tree to merge into
  * @param src - tree to merge
  * @param timeOffset -- value to offset particle times in src 
  */
  void MergeMCTrees(I3MCTree& dest, const I3MCTree& src, double timeOffset);

  void CopyWeights(I3MapStringDouble& dest, const I3MapStringDouble src);

/**
  * Add time offset to MCTree
  * @param ctree  - tree to modify
  * @param offsetTime - time offset
  */
  void OffsetTime(I3MCTree& ctree,double offsetTime);

  I3Frame MergeFrames(I3Frame frame1, I3Frame frame2, I3Map<std::string,std::string> names, float delta_t);

  void MergeFrames(I3FramePtr frame1, const I3FramePtr frame2, const I3Map<std::string,std::string> names, float delta_t);

  /**
   * Merge all photons from one container into another, with a time offset
   * @tparam PhotonType a photon type which has `GetTime` and `SetTime` functions
   * @tparam ContainerType a container of lists of PhotonType indexed by a key type
   * @param dest the container into which the additional photons are to be added
   * @param src the container from which the additional photons are copied
   * @param timeOffset the time offset to apply to each added photon
   */
  template<typename PhotonType,
           typename ContainerType=I3Map<ModuleKey, I3Vector<PhotonType>>>
  void MergePhotons(ContainerType& dest, const ContainerType& src, double timeOffset){
    auto timeOrder=[](const PhotonType& p1, const PhotonType& p2)->bool{
      return(p1.GetTime()<p2.GetTime());
    };
    for(const auto module_entry : src){
      typename ContainerType::mapped_type& dest_photons=dest[module_entry.first];
      //copy all photons, offsetting times
      for(PhotonType p : module_entry.second){
        p.SetTime(p.GetTime() + timeOffset);
        dest_photons.push_back(p);
      }
      //re-sort the full resulting list by times
      std::sort(dest_photons.begin(),dest_photons.end(),timeOrder);
    }
  }
  
  template<typename PhotonType,
           typename ContainerType=I3Map<ModuleKey, I3Vector<PhotonType>>>
  double GetFirstPhotonTime(const ContainerType& photonMap){
    double start_time = NAN;
    for(const auto module_entry : photonMap){
      for(const PhotonType& p : module_entry.second){
        if(p.GetTime() >= start_time) //this will be false if start_time is NaN
          continue;
        start_time = p.GetTime();
      }
    }
    return start_time;
  }

  bool IsChargedLepton(I3Particle::ParticleType particle);
  bool IsNeutrino(I3Particle::ParticleType particle);

} // namespace PolyplopiaUtils


#endif
