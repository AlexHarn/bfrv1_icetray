#ifndef I3RESPONSEMAPSPLITTER_H_INCLUDED
#define I3RESPONSEMAPSPLITTER_H_INCLUDED

/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3ResponseMapSplitter.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

// stdlib stuff
#include <vector>

// icetray stuff
#include "icetray/I3ConditionalModule.h"
#include "icetray/IcetrayFwd.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Map.h"
#include "icetray/OMKey.h"
#include "phys-services/I3Splitter.h"
#include "phys-services/I3RandomService.h"

/**
 * @class I3ResponseMapSplitter
 * @brief hit set splitteer
 *
 * @todo docs
 */
class I3ResponseMapSplitter : public I3ConditionalModule, private I3Splitter {

public:

    /// constructor (define configurables)
    I3ResponseMapSplitter(const I3Context& ctx);

    /// destructor
    ~I3ResponseMapSplitter(){}

    /// configure (get & check configurables)
    void Configure();

    /// do a reconstruction
    void Physics(I3FramePtr frame);
    /// monkey with entire readouts
    void DAQ(I3FramePtr frame);

    /// say bye
    void Finish();

    /// @enum sw_t for enumerating ways to weight the pulses (no weight, charge weight, only first pulse per DOM)
    enum splitweight_t {
        SW_OLD=0,
        SW_Charge,
        SW_DOM
    };

    /**
     * New implementation of pulsemap splitting using mean/median time
     * static method for convenience (unit test & external use)
     * instantiated for ResponseMap=I3RecoPulseMap 
     *
     * Note that you really need to call this method with unsigned ints for
     * nhits1 and nhits2, some or all compilers will not accept a call with
     * signed ints here.
     *
     * @returns split time (useful for testing)
     */
    template<typename Response>
    static double SplitByTime(
            typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
            unsigned int &nhits1,
            unsigned int &nhits2,
            bool median,
            I3ResponseMapSplitter::splitweight_t sw,
            const std::string &name );


    /**
     * Implementation of geometric pulsemap splitting (using track & COG)
     * static method for convenience (unit test & external use)
     * instantiated for ResponseMap=I3RecoPulseMap 
     */
    template<typename Response>
    static void SplitByTrackAndCOG(
            I3ParticleConstPtr track,
            I3GeometryConstPtr geoptr,
            typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
            unsigned int &nhits1,
            unsigned int &nhits2,
            const std::string &name );

    /**
     * Implementation of geometric pulsemap splitting (using track & time residuals)
     * static method for convenience (unit test & external use)
     * instantiated for ResponseMap=I3RecoPulseMap
     */
    template<typename Response>
    static void SplitByTimeResidual(
            double minTRes, double maxTRes,
            I3ParticleConstPtr track,
            I3GeometryConstPtr geoptr,
            typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
            unsigned int &nhits1,
            unsigned int &nhits2,
            const std::string &name );

    /**
     * Implementation of pulsemap splitting (using pulse charges)
     * static method for convenience (unit test & external use)
     * instantiated for ResponseMap=I3RecoPulseMap
     */
    template<typename Response>
    static void SplitByBrightness(
            double maxDBrightSt,
            I3GeometryConstPtr geoptr,
            typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
            unsigned int &nhits1,
            unsigned int &nhits2,
            const std::string &name );
    /**
     * Implementation of k-means clustering algorithm in x and y
     * can rotate into plane defined by a given track before splitting
     * largest number of hit DOMs is in "1" pulse
     * static method for convenience (unit test & external use)
     * instantiated for ResponseMap=I3RecoPulseMap
     */
    template<typename Response>
    static void SplitByKmeans(
            bool wantRotated_,
            I3ParticleConstPtr track,
            I3GeometryConstPtr geoptr,
            typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
            typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
            unsigned int &npulses1,
            unsigned int &npulses2,
            unsigned int &nhits1,
            unsigned int &nhits2,
            const std::string &name,
	    I3RandomServicePtr rnd);

 
private:

    // inhibit default constructors and assignment by making them private
    I3ResponseMapSplitter(); /// inhibited
    I3ResponseMapSplitter(const I3ResponseMapSplitter& source); /// inhibited
    I3ResponseMapSplitter& operator=(const I3ResponseMapSplitter& source); /// inhibited

    // configurables
    std::string inResponseMapName_; /// obsolete (setting this will result in a fatal)
    std::string inPulseMapName_;    /// name of pulseseriesmap
    std::string inTrackName_;       /// alternative splitting using a track (still to be implemented)
    std::string geoName_;           /// name of I3Geometry object in the frame
    int minNch_;                    /// minimum number of channels in the response map to be splitted
    bool doTRes_;                   /// sort hits according to time residual rel to inTrackName_
    double minTRes_;                /// minimum time residual rel to inTrackName_
    double maxTRes_;                /// maximum time residual rel to inTrackName_
    bool doBrightSt_;               /// sort hits according to proximity to brightest string
    double maxDBrightSt_;           /// maximum radial distance to brigthest string
    bool doKmeans_;                 /// sort hits using k-means clustering
    bool wantRotated_;              /// rotated hits into plane of track before clustering search
    bool splitEvents_;              /// run as an event splitter
    bool doTMedian_;                /// do time splitting on *median* instead of *mean* time
    std::string tSplitWeightName_;  /// option name for time based splitting strategy
    splitweight_t tSplitWeight_;    /// option enum for time based splitting strategy

    // names of output pulse sets
    std::string outResponseMap1Name_; /// name of the first half of the response map
    std::string outResponseMap2Name_; /// name of the other half of the response map

    unsigned int nEvents_; /// count events with enough hits to split
    unsigned int nBadEvents_; /// count events with too few hits
    unsigned int nHits1_; /// count total number of hits in set 1
    unsigned int nHits2_; /// count total number of hits in set 2
    unsigned int nMinHits1_; /// record minimum number of hits in set 1
    unsigned int nMinHits2_; /// record minimum number of hits in set 2
    unsigned int nMaxHits1_; /// record maximum number of hits in set 1
    unsigned int nMaxHits2_; /// record maximum number of hits in set 2

    /// type dependent implementation details
    template<class ResponseMap> void SplitMap(I3FramePtr frame);

    template <typename Response>
    void WriteOutput(I3FramePtr frame,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2);

    SET_LOGGER( "I3ResponseMapSplitter" );
  
};  // end of the class definition.


#endif /* I3RESPONSEMAPSPLITTER_H_INCLUDED */
