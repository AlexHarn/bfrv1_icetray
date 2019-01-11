/**
 * @file I3GulliverMinuit2.h
 * @brief definition of the I3GulliverMinuit2 class
 *
 * (c) 2010 the IceCube Collaboration
 *
 * $Id$
 * @version $Revision$
 * @date $Date$
 * @author Jakob van Santen <vansanten@wisc.edu>
 *
 * See the Minuit2 Manual: http://seal.web.cern.ch/seal/documents/minuit/mnusersguide.pdf
 */

#ifndef I3GULLIVERMINUIT2_H_INCLUDED
#define I3GULLIVERMINUIT2_H_INCLUDED

/* Gulliver stuff */
#include "gulliver/I3GulliverBase.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3MinimizerResult.h"
#include "gulliver/I3FitParameterInitSpecs.h"

#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"

/* Algorithms implemented in Minuit2 distribution */
enum I3GulliverMinuit2Algorithm {SIMPLEX, MIGRAD, COMBINED, FUMILI};

/**
 * @class I3GulliverMinuit2
 * @brief A minimizer class which wraps Minuit2 such that it can be used in
 *        Gulliver-based reconstruction.
 */
class I3GulliverMinuit2 : public I3ServiceBase, public I3MinimizerBase {
    public:
        /// constructor with full initialization (for unit tests)
        I3GulliverMinuit2(std::string name, const double tol,
                          unsigned int imax, int verbose, int strat,
                          std::string alg, bool flatcheck, bool gradient,
                          bool gradcheck, bool noedm);

        /// constructor for service factory
        I3GulliverMinuit2(const I3Context& context);

        /// destructor
        virtual ~I3GulliverMinuit2();

        /// configuration
        void Configure();

        /// core method: minimizer a given function with given initial
        /// conditions
        I3MinimizerResult Minimize(
                I3GulliverBase &g,
                const std::vector<I3FitParameterInitSpecs> &parspecs);

        /// set tolerance (minuit manual is not very clear on what "tolerance"
        /// exactly means)
        double GetTolerance() const {return tolerance_;}
        void SetTolerance(double newtol) {tolerance_ = newtol;}

        unsigned int GetMaxIterations() const {return maxIterations_;}
        void SetMaxIterations(unsigned int newmaxi) {maxIterations_ = newmaxi;}

        const std::string GetName() const {return I3ServiceBase::GetName();}
        bool UsesGradient() {return withGradient_;};

        bool GetCheckGradient() const {return checkGradient_;};
        void SetCheckGradient(bool newcheck) {checkGradient_ = newcheck;};

        SET_LOGGER("I3GulliverMinuit2");

    private:
        I3GulliverMinuit2();
        I3GulliverMinuit2(const I3GulliverMinuit2&);
        I3GulliverMinuit2 operator= (const I3GulliverMinuit2& rhs);

        void CheckParameters();

        const std::string name_;
        double tolerance_;
        unsigned int maxIterations_;
        int minuitPrintLevel_;
        int minuitStrategy_;
        std::string algorithmName_;
        I3GulliverMinuit2Algorithm algorithm_;
        bool flatnessCheck_;
        bool withGradient_;
        bool checkGradient_;
        bool ignoreEDM_;
};

#endif
