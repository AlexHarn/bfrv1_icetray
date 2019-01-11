/**
 *  copyright  (C) 2006
 *  the icecube collaboration
 *  $Id$
 *
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 */

#ifndef GULLIVER_I3EVENTHYPOTHESIS_H_INCLUDED
#define GULLIVER_I3EVENTHYPOTHESIS_H_INCLUDED

#include "icetray/IcetrayFwd.h"
#include "dataclasses/physics/I3Particle.h"
#include "icetray/I3FrameObject.h"

/**
 * @class I3EventHypothesis
 *
 * Reconstructions in IceTray should always be stored in an object
 * of type I3Particle, and any additional variables which cannot be
 * accommodated in such an object (or in its vector of children, for
 * composite solutions) should be stored in a custom frame object with
 * "fit parameters". IceTray does not provide a method to unambiguously
 * link these two, so that's why we have this hyperparticle in Gulliver.
 *
 * This object is for use in Gulliver services and modules, not for
 * storage in the I3Frame; particle and nonstd are stored separately.
 *
 * Note that the "nonstd" part is intended solely for fittable variables
 * related to the physics of the kind of event you are trying to
 * reconstruct; for instance, for a double bang maybe you would store the
 * times and energies of the first and second bang. You would *not* store
 * e.g. the number of the number of hits associated to the two bangs.
 *
 * @todo Actually, why should we not allow unfittable parameters in nonstd?
 */
class I3EventHypothesis {
    public:

        /// default constructor
        I3EventHypothesis():particle(new I3Particle){}

        /**
         * partial copy constructor
         * @param[in] p This particle is *copied* into this hypothesis
         */
        I3EventHypothesis( const I3Particle &p ):
            particle(new I3Particle(p)){}

        /**
         * partial copy constructor
         * @param[in] p This *pointer* is copied, *no* deep/shallow copy is made.
         * @param[in] nstd This *pointer* is copied, *no* deep/shallow copy is made.
         */
        I3EventHypothesis( I3ParticlePtr p, I3FrameObjectPtr nstd):
            particle(p),nonstd(nstd){}

        /// destructor
        ~I3EventHypothesis(){}

        /// the standard physics variables
        I3ParticlePtr particle;

        /// object holding any non-standard physics variables (may be NULL)
        I3FrameObjectPtr nonstd;

};

I3_POINTER_TYPEDEFS( I3EventHypothesis );

#endif /* GULLIVER_I3EVENTHYPOTHESIS_H_INCLUDED */
