/**
 * @brief Interface to Multinest minimization algorithm for Gulliver. (Use carefully! )
 *
 * References:
 * 'MultiNest: an efficient and robust Bayesian inference tool for cosmology and particle physics'
 * F. Feroz, M.P. Hobson, M. Bridges. Sep 2008. 14 pp.
 * Published in Mon.Not.Roy.Astron.Soc. 398 (2009) 1601-1614
 * DOI: 10.1111/j.1365-2966.2009.14548.x 
 *
 * Author: Martin Leuermann <leuermann@physik.rwth-aachen.de>
 */

#include <boost/foreach.hpp>
#include "gulliver/I3GulliverBase.h"

#include "gulliver/I3MinimizerResult.h"
#include "gulliver/I3FitParameterInitSpecs.h"
#include "icetray/I3SingleServiceFactory.h"
#include "minimizer/I3GulliverMN.h"
#include "minimizer/I3MinimizerUtils.h"


/** Constructor and destructor **/
I3GulliverMN::I3GulliverMN(const I3Context& context) : I3ServiceBase(context)
{
    AddParameter("Tolerance", "Tolerance of minimization", 1.0);
    AddParameter("NIS", "Do Nested Importance Sampling?", false);
    AddParameter("ModeSeparation", "Do Nested Importance Sampling?", true);
    AddParameter("ConstEff", "Run in constant efficiency mode?", false);
    AddParameter("NLive", "Number of live points", 250); 
    AddParameter("Efficiency", "Required efficiency of minimization", 1.0);
    AddParameter("FeedbackInterval", "After how many iteration feedback is required.", 100);
    AddParameter("MaxModes", "expected max no. of modes (used only for memory allocation)", 100);
    AddParameter("Feedback", "need feedback on standard output?", false);
    AddParameter("MaxIterations", "If zero, no boundaries", 0);
    //AddParameter("MinIterations", "If zero, no boundaries", 0); // seems to not exist in my version of multinest (https://github.com/JohannesBuchner/MultiNest)
    AddParameter("PeriodicParams", "Names of parameters that are periodic, e.g. Azimuth.", periodics_);
    AddParameter("ModeSeparatedParams", "Names of parameters that shall be mode-separated", modeSeparated_);
}
I3GulliverMN::~I3GulliverMN() {}


/** Set all parameters to configure minimizer **/
void
I3GulliverMN::Configure()
{
    GetParameter("Tolerance", tolerance_);
    GetParameter("NIS", ins_);
    GetParameter("ModeSeparation", mmodal_);
    GetParameter("ConstEff", ceff_);
    GetParameter("Efficiency", efr_);
    GetParameter("NLive", nlive_);
    GetParameter("FeedbackInterval", updint_);
    GetParameter("ModeSeparation", mmodal_);
    GetParameter("MaxModes", maxmodes_);
    GetParameter("Feedback", feedback_);
    GetParameter("MaxIterations", maxiter_);
    //GetParameter("MinIterations", miniter_);
    GetParameter("PeriodicParams", periodics_);
    GetParameter("ModeSeparatedParams", modeSeparated_);

    Ztol_       = -1E90;
    rseed_      = 1; //seed of random number generator in fortran, previously: -1 = system clock;
    resume_     = 0;
    init_mpi_   = 1;
    logZero_    = -1E90;
    writefiles_ = 0;
}


/** Function that shall be minimized. This is the actual function that is seen
 *  by the MultiNest Fortran code which itself refers to an I3GulliverBase
 *  Object to evaluate the llh.
 *
 *  \param Cube             Point of parameter space that shall be evaluated.
 *  \param ndim             Dimensionallity of parameter space in terms of
 *                          free parameter for minimization
 *  \param npars            Total number of parameters including derived params.
 *                          from the free ones (usually npars==ndim for most cases).
 *  \param lnew             Function value to be returned.
 *  \param misc             Any other stuff user wants to hand over to fct (structure
 *                          of function parameters determined by MN requirements). For
 *                          Gulliver, this is always an I3GulliverMN object that contains
 *                          all necessary informations about how to convert the standard
 *                          MN parameter ranges ([0,1]) into physical ranges for the
 *                          llh function (e.g. for millipede).
 * */
void I3GulliverMN::fct(double* Cube, int &ndim, int &npars, double &lnew, void* misc) {
    I3GulliverMN* tBase = reinterpret_cast<I3GulliverMN*>(misc);
    std::vector<double> params;
    for (int i=0; i<ndim; i++) {
        int index   = tBase->paramOrder_[i];
        double nval = tBase->lowerBnds_[index] + (tBase->upperBnds_[index] - tBase->lowerBnds_[index]) * Cube[index];
        params.push_back(nval);
    }
    lnew = (-1.0) * (*(tBase->g_))( params );
    return;
}


/** Dumps information about current status of minimization AND in the end dumps
 *  information about found minimum and parameter combination. Is called every
 *  10*updint_ iterations.
 *
 *  MultiNest documentation online. Most parameters not
 *  very interesting as long as you are not interested in
 *  details about the minimization.
 * */
void I3GulliverMN::InfoDumper(  int &nSamples, int &nlive, int &nPar,
                                double **physLive, double **posterior, 
                                double **paramConstr, double &maxLogLike, 
                                double &logZ, double &INSlogZ, double &logZerr, void * misc)
{
    I3GulliverMN* tBase = reinterpret_cast<I3GulliverMN*>(misc);
    log_trace("(%s)::InfoDumper() - MultiNest Dumper function called.", tBase->GetName().c_str());
    tBase->params_bestFit.clear();
    for(int i=0; i<nPar; i++) {
        tBase->params_bestFit.push_back(paramConstr[0][2*nPar + i]);
    }

    double curWorstLLH = maxLogLike;
    for(int k=0; k<nlive; k++) {
        if (curWorstLLH > physLive[0][nPar*nlive + k]) { curWorstLLH = physLive[0][nPar*nlive + k]; }
    }

    tBase->llh_bestFit_  = maxLogLike;
    tBase->llh_worstFit_ = curWorstLLH;

}

/** Required Minimize() function for every Gulliver-based minimizer.
 *
 *  \param g                I3GulliverBase module which can be called as a function.
 *  \param parspecs         Parametrization of hypothesis. Seeding is not needed for
 *                          MN. Therefore, the seed is used as CENTER of the scanned
 *                          parameter space, such that the parameter space can be varied
 *                          by moving the corresponding seed hypothesis or the relative
 *                          boundaries.
 *                          A stepsize is also not needed. Thus, the stepsize of I3FitParameterInitSpecs
 *                          determines only whether a parameter is periodic (<0) or
 *                          not (>0).
 *  \return                 Standard I3MinimizerResult object.
 * */
I3MinimizerResult
I3GulliverMN::Minimize(I3GulliverBase &g, const std::vector<I3FitParameterInitSpecs> &parspecs )
{
    log_debug("(%s)::Minimize() - based on nested::run fortran implementation called.",
                this->GetName().c_str());

    int ndims      = parspecs.size();
    int npars      = parspecs.size();
    int nclspar    = (modeSeparated_.size() > 0) ? modeSeparated_.size() : 1;

    /// clear stuff
    lowerBnds_.clear();
    upperBnds_.clear();

    /// convert parameter to certain order needed for the Fortran routines
    std::vector<I3FitParameterInitSpecs> reorderedParams; 
    for(uint i=0; i<parspecs.size(); i++) {
        const I3FitParameterInitSpecs &par = parspecs[i];
        for(uint k=0; k < modeSeparated_.size(); k++) {
            if (par.name_ == modeSeparated_[k]) {reorderedParams.push_back(par);  break; } 
        }
    }

    for(uint i=0; i<parspecs.size(); i++) {
        const I3FitParameterInitSpecs &par = parspecs[i];
        bool notModeSep = true;
        for(uint k=0; k < modeSeparated_.size(); k++) { if (par.name_ == modeSeparated_[k]) notModeSep = false; }
        if(notModeSep) { reorderedParams.push_back(par); }
    }

    for(uint i=0; i<parspecs.size(); i++) {
        for (uint k=0; k<reorderedParams.size(); k++) {
            if (parspecs[i].name_ == reorderedParams[k].name_) paramOrder_.push_back(k);
        }
    }

    /// set some needed variables
    assert(ndims>0);
    int pWrap[ndims];
    for(int i = 0; i < ndims; i++) pWrap[i] = 0;

    g_  = &g;
    std::string the_path("");

    /// Set ranges of all parameters and set peridodicity condition
    for (unsigned i=0; i < reorderedParams.size(); i++) {
        const I3FitParameterInitSpecs &par = reorderedParams[i];

        if (std::isfinite(par.minval_) && par.minval_ != par.maxval_) {
            lowerBnds_.push_back(par.minval_);
        } else {
            lowerBnds_.push_back(0.0);
        }
        if (std::isfinite(par.maxval_) && par.minval_ != par.maxval_) {
            upperBnds_.push_back(par.maxval_);
        } else {
            upperBnds_.push_back(1.0);
        }
        for(uint k=0; k < periodics_.size(); k++) {
            if (par.name_ == periodics_[k]) { pWrap[i] = 1;} 
        }
        log_debug("(%s)::Minimize() - Parameter (%1u) range set to: min = %e, max = %e",
                    this->GetName().c_str(), i, lowerBnds_[i], upperBnds_[i]);
    }
    /// should never happen... but just in case
    assert(reorderedParams.size() == parspecs.size());

    /// run Fortran routines for minimization
    nested::run(ins_, mmodal_, ceff_, nlive_, tolerance_, efr_,
                ndims, npars, nclspar, maxmodes_, updint_,
                Ztol_, the_path, rseed_, pWrap, feedback_, resume_,
                writefiles_, init_mpi_, logZero_, maxiter_,
                &I3GulliverMN::fct,
                &I3GulliverMN::InfoDumper,
                this);

    log_debug("(%s)::Minimize() - Calling nested::run with the following values:"
                "mmodal_ = %1u, ceff_ = %1u, nlive_ = %6u, tol_ = %e, "
                "efr_ = %e, ndims_ = %2u, npars_ = %2u, nclspar_ = %2u, "
                "maxmodes_ = %4u, updint_ = %3u, Ztol_ = %e, "
                "basename_ = %s, rseed_ %2i, feedback_ = %1u, "
                "resume_ = %1u, writefiles_ = %1u, init_mpi_ = %1u, "
                "logZero_ = %e, maxiter_ = %8u",
                this->GetName().c_str(),
                mmodal_, ceff_, nlive_, tolerance_,
                efr_, ndims, npars, nclspar,
                maxmodes_, updint_, Ztol_,
                the_path.c_str(), rseed_, feedback_,
                resume_, writefiles_, init_mpi_,
                logZero_, maxiter_);

    /// convert result into I3MinimizerResult object - take care of the differently ordered variables
    I3MinimizerResult result(parspecs.size());

    for (uint i = 0; i < parspecs.size(); i++) {
        for (uint k=0; k< reorderedParams.size(); k++) {
            if (reorderedParams[k].name_ == parspecs[i].name_) {
                result.par_[i] = lowerBnds_[k] + (upperBnds_[k] - lowerBnds_[k]) * params_bestFit[k];
                break;
            }
        }
    }

    if (fabs(llh_bestFit_ - llh_worstFit_) < 10.0 * std::numeric_limits<double>::epsilon() ) { 
        result.converged_ = false;
        log_warn("(%s)::Minimize() - found worst llh at %e and best one at %e. Thus,"
                 " the fit is considered as NOT converged (diverged).",
                 this->GetName().c_str(), llh_worstFit_, llh_bestFit_);
    } else { result.converged_ = true; }

    result.minval_      = llh_bestFit_;

    /// nothing we want to do so far...
    bool checkflat = false;
    if (checkflat)
        I3MinimizerUtils::CheckMinimum(*this, result, parspecs, g);

    reorderedParams.clear();
    return result;
}

typedef
I3SingleServiceFactory<I3GulliverMN,I3MinimizerBase>
I3GulliverMNFactory;
I3_SERVICE_FACTORY( I3GulliverMNFactory );
