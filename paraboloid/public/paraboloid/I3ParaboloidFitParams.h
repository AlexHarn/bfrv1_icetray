#ifndef I3PARABOLOIDFITPARAMS_H_INCLUDED
#define I3PARABOLOIDFITPARAMS_H_INCLUDED

/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id$
 *
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

#include "gulliver/I3LogLikelihoodFitParams.h"

/**
 * @class I3ParaboloidFitParams
 * @brief The I3LogLikelihoodFitParams extended with paraboloid-specific information
 */

class I3ParaboloidFitParams : public I3LogLikelihoodFitParams {
    public:

    /// default constructor
    I3ParaboloidFitParams( double l=NAN, double r=NAN,
                              int dof=-1, int mini=-1 ){
        log_debug( "hello" );
        Reset();
        this->logl_ = l;
        this->rlogl_ = r;
        this->ndof_ = dof;
        this->nmini_ = mini;
    }

    /// destructor
    virtual ~I3ParaboloidFitParams(){}
  
    std::ostream& Print(std::ostream&) const override;

    /// put all datamembers to default values
    void Reset() override;

    /**
     * @enum ParaboloidFitStatus
     * ParaboloidFitStatus:
     * Initialized to PBF_UNDEFINED = -1000
     * The general scheme is:
     *
     * - Negative values occur when paraboloid cannot successfully 
     *   complete the fit.
     *
     * - Positive values occur after additional checks, 
     *   when the fit is deemed "not good".  (Somewhat subjective?)
     *
     * - Identically Zero means everything was okay = PBF_SUCCESS
     */
    enum ParaboloidFitStatus {
        PBF_UNDEFINED = -1000,               ///< undefined (initvalue before fit)
        PBF_NO_SEED = -100,                  ///< missing input track
        PBF_INCOMPLETE_GRID = -80,           ///< on too many gridpoints vertex refit failed
        PBF_FAILED_PARABOLOID_FIT = -50,     ///< no parabola could be fitted to grid
	PBF_SINGULAR_CURVATURE_MATRIX = -20, ///< curvature matrix singular
        PBF_SUCCESS = 0,                     ///< successful fit
        PBF_NON_POSITIVE_ERRS = 20,          ///< both errors are *negative* (llh landscape looks like *maximum instead of minimum)
        PBF_NON_POSITIVE_ERR_1 = 21,         ///< only error2>0 (saddle point)
        PBF_NON_POSITIVE_ERR_2 = 22,         ///< only error1>0 (saddle point)
	PBF_TOO_SMALL_ERRS = 30              ///< error is infinitesimally small, probably a numerical problem happened
    };

    /**
     * Paraboloid fit status: initialized to PBF_UNDEFINED = -1000
     * @sa ParaboloidFitStatus
     */
    ParaboloidFitStatus pbfStatus_;

    // -----------------------------------------------------------------
    // the following lines were almost literally copied from sieglinde's
    // result/SLResultParabola.hh
    // (Float_t was replaced by double)
    // -----------------------------------------------------------------

    // Main results of Paraboloid fit
    double pbfZen_;        ///< theta-angle of paraboloid fit (global coordinates)
    double pbfAzi_;        ///< phi-angle of paraboloid fit (global coordinates)
    double pbfErr1_;       ///< parameter 1 of error ellipse (using diagonalized hesse matrix)
    double pbfErr2_;       ///< parameter 2 of error ellipse (using diagonalized hesse matrix)
    double pbfRotAng_;     ///< rotation angle of error ellipse (using diagonalized hesse matri
    // llh value of grid center is different from seed llh due to the changed
    // number of free parameters (only x,y,z free)
    double pbfCenterLlh_;  ///< llh-value of grid center

    // Internal parameters that could be used to determine the usability of the
    // Paraboloid results
    double pbfLlh_;        ///< llh-value of paraboloid fit
    double pbfZenOff_;     ///< theta offset of paraboloid fit (local coordinates)
    double pbfAziOff_;     ///< phi-offset of paraboloid fit (local coordinates)
    double pbfCurv11_;     ///< (theta) curvature-value of paraboloid fit
    double pbfCurv12_;     ///< (1/cov) curvature-value of paraboloid fit
    double pbfCurv22_;     ///< (phi)   curvature-value of paraboloid fit
    double pbfChi2_;       ///< chi^2 of parabola fit
    double pbfDetCurvM_;   ///< determinant of curvature matrix of parabola fit
    double pbfSigmaZen_;   ///< theta error of fit using inverted curvature matrix
    double pbfSigmaAzi_;   ///< phi error of fit using inverted curvature matrix
    double pbfCovar_;      ///< covariance from inverted curvature matrix

    // ----------------------------------------------------------------
    // the previous lines were almost literally copied from sieglinde's
    // result/SLResultParabola.hh
    // ---------------------------------------------------------

    // The recoos version of the paraboloid fit writes four more numbers
    // comparing the reconstructed track to a MC track.
    // They were used by Till Neunhoefer to check whether or not the fit
    // results are usable as an estimator for the angular resolution (pull).
    // With Sieglinde they were not calculated.

    double pbfTrOffZen_;   ///< true zenith (of highest energy muon) in local paraboloid coordinate system
    double pbfTrOffAzi_;   ///< true azimuth (of highest energy muon) in local paraboloid coordinate system
    double pbfTrZen_;      ///< true zenith (of highest energy muon)
    double pbfTrAzi_;      ///< true azimuth (of highest energy muon)

protected:

    friend class icecube::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, unsigned version);

};

std::ostream& operator<<(std::ostream&, const I3ParaboloidFitParams&);

I3_CLASS_VERSION(I3ParaboloidFitParams, 2);

I3_POINTER_TYPEDEFS( I3ParaboloidFitParams );

#endif /* I3PARABOLOIDFITPARAMS_H_INCLUDED */
