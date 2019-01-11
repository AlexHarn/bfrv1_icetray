/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

#ifndef GULLIVER_I3SEEDSERVICEBASE_H_INCLUDED
#define GULLIVER_I3SEEDSERVICEBASE_H_INCLUDED

// smart pointer definition
#include "boost/shared_ptr.hpp"


// icetray stuff
#include "icetray/IcetrayFwd.h"
#include "icetray/I3DefaultName.h"
#include "gulliver/I3EventHypothesis.h"

/**
 * @class I3SeedServiceBase
 * @brief Base class seed track collection & preparation services
 *
 * Seed services are supposed to collect the first guess tracks, and do any
 * desired transformations on them.
 *
 * @sa I3Gulliver
 * @sa I3GulliverBase
 * @sa I3EventHypothesis
 *
 */
class I3SeedServiceBase {

public:

    /// Base class constructor (idle)
    I3SeedServiceBase(){}

    /// Base class destructor (idle)
    virtual ~I3SeedServiceBase(){}

    /**
     * Provide event data:
     * - purge old seed tracks
     * - get the first guess tracks
     * - fill in the blanks
     * - do tweaks (time/space corrections)
     *
     * The generated seeds should all be good, have OK fit status and position,
     * direction and time datamembers should not be NAN.
     *
     * @todo Think about no-NAN requirement for energy and length.
     *
     * @param[in] f Frame with event data
     * @returns number of available seeds
     */
    virtual unsigned int SetEvent(const I3Frame &f) = 0;

    /**
     * Return a seed. May call log_fatal() if @c iseed is larger
     * than the number of available seeds.
     *
     * When you implement this method: make sure that the returned seed
     * is a deep copy (not a shallow copy) of some internally stored seed,
     * because the receiver will most likely change its values.
     * (This method is declared @c const to enforce this requirement...)
     */
    virtual I3EventHypothesis GetSeed(unsigned int iseed) const = 0;

    virtual I3EventHypothesisPtr GetSeedPtr(unsigned int iseed) const {
        return I3EventHypothesisPtr(new I3EventHypothesis(this->GetSeed(iseed)));
    }

    /**
     * Get dummy seed; useful in case all first guesses fail.
     * In particular if the seeds normally have a non-NULL "nonstd"
     * data member, then this should be initialized in this dummy seed
     * as well.
     * (I would find it more logical and safer to have reconstruction
     * results always in some kind of sortable result container, which
     * could be set to zero results in case of total failure. But that
     * does not seem to be compatible with with the IceTray design: there
     * is no possibility to store a vector of fit parameters.)
     *
     * When you implement this method: make sure that the returned dummy seed
     * is a deep copy (not a shallow copy) of some internally stored seed,
     * because the receiver will likely change its values.
     * (This method is declared @c const to enforce this requirement...)
     */
    virtual I3EventHypothesis GetDummy() const {
        return I3EventHypothesis();
    }

    virtual I3EventHypothesisPtr GetDummyPtr() const {
        return I3EventHypothesisPtr(new I3EventHypothesis(this->GetDummy()));
    }

    /**
     * Get a (deep) copy of an existing event hypothesis.
     * The caller (a log-likelihood fitter module) does not need to know what
     * kind of track exactly it is. E.g. for a tau seed service, this method
     * correctly copies over the tau-specific variables in the "nonstd" data
     * member. (This is not so relevant for vanilla muons and cascades; you
     * could also just copy the I3Particle information.) Call fatal if the
     * hypothesis is of the wrong type.
     *
     * The default implementation just makes a (deep) copy of the I3Particle
     * data member of the I3EventHypothesis. The "nonstd" data member is not
     * copied.
     */
    virtual I3EventHypothesis GetCopy(const I3EventHypothesis &eh) const {
        return I3EventHypothesis(*(eh.particle));
    }

    virtual I3EventHypothesisPtr GetCopyPtr(const I3EventHypothesis &eh) const {
        return I3EventHypothesisPtr(new I3EventHypothesis(this->GetCopy(eh)));
    }

    /**
     * When a seed service generates a seed, it may perform some tweaks or
     * transformations on it intended to facilitate the minimization. The
     * generic example is the vertex correction for an infinite track:
     * purely mathematically it is irrelevant where the vertex (base point)
     * is chosen, one can shift the vertex over an arbitrary length
     * L along the track, as long as the vertex time is adjusted by
     * L/c. However, a minimizer will more easily navigate the phase
     * space if the vertex is chosen well within the clouds of hit
     * DOMs. It can also be good to do a correction to the vertex time,
     * for instance for first guess algorithms which use the average of
     * the hit times, one can improve by choosing a vertex time based
     * on the distribution of time residuals.
     * 
     * Now, make sure you do those tweaks in this method. Because then
     * gulliver modules which generate new seed solutions (e.g. iterative
     * and paraboloid) can use it as well.
     *
     * The default implementation does no tweaking.
     */
    virtual void Tweak(I3EventHypothesis &eh) const {}

    /**
     * Many first guess algorithms do not provide the complete description
     * for the kind of event we are trying to reconstruct. A simple example
     * is the energy of a cascade or muon. The seed service might take care
     * of filling in a sensible initial value for such a quantity.
     *
     * For track types other than cascades or muons, there might be other
     * less trivial preparations to do. E.g. for a "sugardaddy" tau track, the
     * point where the tau decays into a muon should be set to some sensible
     * value, in the "nonstd" data member of the event hypothesis.
     *
     * The default implementation does no filling.
     */
    virtual void FillInTheBlanks(I3EventHypothesis &eh) const {}

};

I3_POINTER_TYPEDEFS(I3SeedServiceBase);
I3_DEFAULT_NAME(I3SeedServiceBase);

#endif /* GULLIVER_I3SEEDSERVICE_H_INCLUDED */
