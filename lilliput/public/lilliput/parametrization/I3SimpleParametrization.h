/**
 *
 * @file I3SimpleParametrization.h
 * @brief declaration of the I3SimpleParametrization class
 *
 * (c) 2005 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */
#ifndef I3SIMPLEPARAMETRIZATION_H_INCLUDED
#define I3SIMPLEPARAMETRIZATION_H_INCLUDED

// standard lib stuff
#include <bitset>
#include <string>
#include <vector>

// Gulliver stuff
#include "gulliver/I3ParametrizationBase.h"

// icetray stuff
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "icetray/I3ServiceBase.h"
#include "icetray/I3Logging.h"

// #include "dataclasses/I3Double.h"

/**
 * @class I3SimpleParametrization
 * @brief The simplest default stupid muon parametrization
 * 
 * Trivial parametrization: parameters are the same as the variables,
 * no transformation, except for the energy: it is converted to logE.
 *
 * A variable is made free for fitting by a setting a positive stepsize.
 * If all stepsizes are zero then the constructor will cal "log_fatal".
 *
 * @todo This class has a virtual destructor and protected members, suggesting
 *       that one could derive other classes from this. Is that really a good idea?
 */
class I3SimpleParametrization : public I3ServiceBase,
                                public I3ParametrizationBase {
private:

    // stop defaults
    I3SimpleParametrization();
    I3SimpleParametrization operator= (const I3SimpleParametrization& rhs);
    const std::string name_;

public:
    enum simplepar_t {
        PAR_T=0,
        PAR_X,
        PAR_Y,
        PAR_Z,
        PAR_Zen,
        PAR_Azi,
        PAR_LinE,
        PAR_LogE,
        PAR_LinL,
        PAR_LogL,
        PAR_Speed,
        PAR_N,
    };

#define I3SIMPLEPARAMETRIZATION_SIMPLEPAR_T_SEQ \
    (PAR_T) (PAR_X) (PAR_Y) (PAR_Z) (PAR_Zen) (PAR_Azi) (PAR_LinE) (PAR_LogE) \
    (PAR_LinL) (PAR_LogL) (PAR_Speed) (PAR_N)

    enum vertexmode_t {
        VERTEX_Default=0, /// xyz parameters correspond to I3Particle::pos_ (what that means depends on the shape)
        VERTEX_Start, /// xyz parameters correspond to starting point position
        VERTEX_Stop, /// xyz parameters correspond to stopping point position
        VERTEX_N, /// number of vertex parametrization modes
    };

#define I3SIMPLEPARAMETRIZATION_VERTEXMODE_T_SEQ \
        (VERTEX_Default) (VERTEX_Start) (VERTEX_Stop) (VERTEX_N)

    static const std::string parNames_[PAR_N];
    static const std::vector<double> no_steps_;
    static const std::vector< std::vector<double> > all_nobounds_;

protected:
    void ApplyChainRule();

    std::bitset<PAR_N> free_;
    int vertexMode_;
    std::string vertexModeString_;
    std::vector<double> steps_;
    std::vector< std::vector<double> > absBounds_;
    std::vector< std::vector<double> > relBounds_;
    std::map<int,int> relativeBounds_;
    bool wantTrace_;
    I3VectorI3ParticlePtr trace_;

    /// convenience: check and print configuration
    void ParInfo( const char *varname,
                  const char* unitname, double unitval,
                  int ilinpar, int ilogpar=-1 );

    /// convenience: define config params for stepsize and for absolute & relative bounds
    void FitParameterConfig( simplepar_t ipar, std::string description,
                              double eglim, double egval, std::string unitname );

    /**
     * convenience: shared initialization code relevant for both constructors
     * (and later also for changing individual parameter settings, useful in python environment)
     */
    void InitializeFitParSpecs(bool verbose=false);

public:
    /**
     * @brief constructor
     *
     * The steps and bounds vectors should have length PAR_N=10.
     */
    I3SimpleParametrization( const std::string name,
                             const std::vector<double> &steps,
                             const std::vector< std::vector<double> > &absbounds = all_nobounds_,
                             const std::vector< std::vector<double> > &relbounds = all_nobounds_,
                             int vertexmode=VERTEX_Default, bool trace=false );

    /// @brief factory constructor (for I3ServiceBase)
    I3SimpleParametrization( const I3Context& context );

    /// destructor
    virtual ~I3SimpleParametrization();

    /// configuration (for I3ServiceBase)
    void Configure();

    /// this should calculate datamembers of the I3Particle from the values in par_
    void UpdatePhysicsVariables();

    /**
     * This should calculate the values in par_ from datamembers of the I3Particle
     * If relevant it should also update stepsizes.
     */
    void UpdateParameters();

    /// Get name (useful for logging/debugging messages)
    const std::string GetName() const {
        return I3ServiceBase::GetName();
    }

    /// chain rule initialization (see base class for more info)
    bool InitChainRule(bool wantgradient);

    /// convert par number to par name
    static const std::string& GetParName(int ipar) {
        if (ipar<0||ipar>=PAR_N) log_fatal("this should never happen");
        return parNames_[ipar];
    }

    /// python convenience: set step at any time after construction
    void SetStep(simplepar_t i, double step=0., bool verbose=false);

    /// python convenience: set absolute bounds at time after construction
    void SetAbsBounds(simplepar_t i, double absmin=0., double absmax=0., bool verbose=false);

    /// python convenience: set relative bounds at time after construction
    void SetRelBounds(simplepar_t i, double absmin=0., double absmax=0., bool verbose=false);

    /**
     * @brief debugging diagnostics
     *
     * Currently this returns a vector all particle objects that were tried during the last fit
     * (the first particle in the list is the seed track). If we get ideas about other
     * interesting diagnostic information then we might introduce a new diagnostics class
     * (where this list of particles would probably still be a datamember).
     *
     * @sa The I3Gulliver class has tracing functionality which is more generic (but slightly
     * less user friendly.
     */
    virtual I3FrameObjectPtr GetDiagnostics( const I3EventHypothesis &fitresult ){
        return trace_;
    }

    SET_LOGGER( "I3SimpleParametrization" );

};

I3_POINTER_TYPEDEFS( I3SimpleParametrization );

#endif /* I3SIMPLEPARAMETRIZATION_H_INCLUDED */
