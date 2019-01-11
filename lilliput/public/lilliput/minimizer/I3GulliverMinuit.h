/**
 *
 * @brief declaration of the I3GulliverMinuit class
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3GulliverMinuit.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#ifndef I3GULLIVERMINUIT_H_INCLUDED
#define I3GULLIVERMINUIT_H_INCLUDED

// standard library stuff
#include <vector>
#include <string>
#include <algorithm>
#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"

// Gulliver stuff
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3MinimizerResult.h"
#include "gulliver/I3FitParameterInitSpecs.h"

/**
 * @class I3GulliverMinuit
 * @brief A minimizer class which wraps TMinuit such that it can be used in
 *        Gulliver-based reconstruction.
 */
class I3GulliverMinuit : public I3ServiceBase, public I3MinimizerBase {
    public:
        /// constructor with full initialization (for unit tests)
        I3GulliverMinuit(std::string name, const double tol, unsigned int maxi,
                         int mpl, int mstr, std::string alg,
                         bool flatnesscheck = true);

        /// constructor for service factory
        I3GulliverMinuit(const I3Context& context);

        /// destructor
        virtual ~I3GulliverMinuit();

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

        SET_LOGGER("I3GulliverMinuit");

    private:
        I3GulliverMinuit();
        I3GulliverMinuit(const I3GulliverMinuit&);
        I3GulliverMinuit operator= (const I3GulliverMinuit& rhs);

        const std::string name_;
        double tolerance_;
        unsigned int maxIterations_;
        int minuitPrintLevel_;
        int minuitStrategy_;
        std::string algorithm_;
        bool flatnessCheck_;
};

#endif
