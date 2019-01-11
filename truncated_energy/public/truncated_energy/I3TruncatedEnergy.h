#ifndef TRUNCATED_ENERGY_I3TRUNCATEDENERGY_H_INCLUDED
#define TRUNCATED_ENERGY_I3TRUNCATEDENERGY_H_INCLUDED

#include <icetray/I3ConditionalModule.h>
#include <icetray/OMKey.h>
#include <photonics-service/I3PhotonicsService.h>
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/I3Position.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/calibration/I3Calibration.h>  // for useRDE
#include <dataclasses/geometry/I3Geometry.h>

namespace truncated_energy
{
    /// define inverse I3Units in advance because multiplications are much
    /// faster than divisions
    static const double i3unit_inv_m   = 1./I3Units::m;
    static const double i3unit_inv_deg = 1./I3Units::degree;
}

/**
 * @class I3TruncatedEnergy
 * @brief This module estimates an energy for a given track,
 *        using photorec tables.
 *
 * In case lightsaber tables are used, the fitted energy is actually an
 * energy loss per meter, for which I3Particle does not have a datamember,
 * so it will be stored in a separate I3Double. A simplistic linear
 * dE/dX(E) relation is used to associate an actual muon energy with this
 * energy loss value; this associated energy value is stored in the energy
 * field of the result I3Particle. The other datamembers are copied from
 * the input track.
 *
 * This module should in principle also be usable for cascades, but it has
 * not been tested yet. If the input particle is not a track,
 * I3TruncatedEnergy will just do nothing!
 */

class I3TruncatedEnergy : public I3ConditionalModule
{

  public:

    /**
     * @param ctx The context with which this module is built.
     */
    I3TruncatedEnergy(const I3Context &ctx);

    ~I3TruncatedEnergy();

    void Configure();
    void Geometry(I3FramePtr frame);
    void Calibration(I3FramePtr frame);
    void Physics(I3FramePtr frame);
    void Finish();

  private:

    /// DOM property structure
    struct dom_prop_t {
        double det_npes;                    //< number of detected photo electrons at the DOM
        double exp_npes;                    //< number of expected photo electrons at the DOM
        double dist_perpendicular_to_track; //< distance of the DOM perpendicular to the track
        double dist_along_track;            //< distance of the DOM along the track
    };
    typedef std::map<const OMKey, dom_prop_t> dom_prop_map_t;

    /// enum type for event classification for the BIN method
    enum BINMethodEventType_t {
        BIN_METHOD_EVENT_TYPE__BAD = 0,
        BIN_METHOD_EVENT_TYPE__BIN_COUNT_GREATER_EQUAL_3 = 1,
        BIN_METHOD_EVENT_TYPE__BIN_COUNT_GREATER_EQUAL_3_AND_DOM_COUNT_GREATER_EQUAL_20 = 2
    };

    /// enum type for event classification for the DOM method
    enum DOMMethodEventType_t {
        DOM_METHOD_EVENT_TYPE__BAD = 0,
        DOM_METHOD_EVENT_TYPE__DOM_COUNT_GREATER_EQUAL_8_AND_LESS_20 = 1,
        DOM_METHOD_EVENT_TYPE__DOM_COUNT_GREATER_EQUAL_20 = 2
    };

    /// current detector geometry
    I3GeometryConstPtr geo_;

    /// calibration info for Relative DOM Efficiency (RDE)
    I3CalibrationConstPtr calib_;
 
    /// name of the pulses to grab from the frame. 
    std::string RecoPulsesName_;

    /// name of the reconstructed particle to use.  
    std::string RecoParticleName_;

    /// name of the result particle to put in the frame.  
    std::string ResultParticleName_;

    /// name of the I3PhotonicsService object
    std::string I3PhotonicsServiceName_;

    /// pointer to I3PhotonicsService object
    I3PhotonicsServicePtr I3PhotonicsService_;

    /// check if pulses and fit are usable
    bool CheckEventPulsesAndFitStatus(
        I3RecoPulseSeriesMapConstPtr reco_pulses_map,
        I3ParticleConstPtr           reco_particle
    );

    /// get expected NPE from table, for a particular DOM+track
    double CalculateExpectedDomNPEsFromTrack(
        I3ParticleConstPtr reco_particle,
        const OMKey        &omkey,
        const I3OMGeo      &omgeo,
        const double       ref_energy,
        const I3VectorOMKeyConstPtr bad_dom_list
    );

    /// diagnostic counter: missing input particle (track, cascade)
    unsigned int nMissingParticle_;
    /// diagnostic counter: missing input pulses
    unsigned int nMissingPulses_;
    /// diagnostic counter: successful photonics calls
    unsigned int nCallsSuccess_;
    /// diagnostic counter: failed photonics calls
    unsigned int nCallsFail_;

    /// you can set the distance away from track for including DOMs or not
    double SetMinCylinderBIN_;
    double SetMaxCylinderBIN_;
    double SetMinCylinderDOM_;
    double SetMaxCylinderDOM_;

    /// you can set the minimum number of DOMS for the event to qualify or not
    double SetMinDOMS_;

    /// if Relative DOM Efficiency should be used
    bool UseRDE_;

    /// if all DOMs should be used in the likelihood instead of only hit ones
//    bool UseAllDOMs_;

    /// name of the BadDomList to grab from the frame
    std::string BadDomListName_;

    SET_LOGGER ("I3TruncatedEnergy");
};

#endif // TRUNCATED_ENERGY_I3TRUNCATEDENERGY_H_INCLUDED
