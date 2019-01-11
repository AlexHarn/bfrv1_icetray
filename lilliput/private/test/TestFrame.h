/**
    copyright  (C) 2006
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author boersma
*/

#ifndef TESTFRAME_H_INCLUDED
#define TESTFRAME_H_INCLUDED

// generic stuff
#include "icetray/I3Frame.h"
#include "icetray/OMKey.h"
#include "icetray/I3Module.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Direction.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "gsl/gsl_rng.h"

class TestFrame : public I3Frame {
private:
    std::string pulseName_;
    I3RecoPulseSeriesMapPtr testPulses_;
    I3GeometryPtr geo_;
    I3Direction GetPerpDir(const I3Direction &p,double azimuth );
public:
    TestFrame(std::string pulsename="testpulses" );
    void AddOM(OMKey om, double x, double y, double z );

    /// adds a pulse to the pulseseriesmap
    void AddPulse(OMKey om, double t, double c, double w );

    /**
     * @returns a random OM key which is not yet in the Geometry
     */
    OMKey GetUnusedOMKey();

    /**
     * @brief Adding a pulse with a given time delay w.r.t. a given track

     * @param p the given track
     * @param tdelay the time delay
     * @param perpdist the perpendicular distance from the track
     * @param chpoint the cherenkov emission point (distance from vertex, can be negative)
     * @param chazi arbitrary azimuth around the track
     * @param charge the pulse charge
     * @param width the pulse width
     *
     * @returns the cosine of the direction under which the downlooking OM is hit
     */
    double AddPulse( I3ParticleConstPtr p, double tdelay, double perpdist,
                     double chpoint, double chazi, double charge,
                     double width );
};

I3_POINTER_TYPEDEFS(TestFrame);

class I3TestFrameSource : public I3Module {
    private:
        I3TestFrameSource();
        gsl_rng* rng_;
        std::string trackName_;
        std::string pulsesName_;
        double eMin_;
        double eMax_;
        SET_LOGGER( "I3TestFrameSource" );
    public:
        I3TestFrameSource(const I3Context& ctx);
        void Configure();
        ~I3TestFrameSource();
        void Process();
};

#endif /* TESTFRAME_H_INCLUDED */
