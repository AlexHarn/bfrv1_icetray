/**
 *  copyright  (C) 2007
 *  the icecube collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

#include <algorithm>
#include <icetray/I3SingleServiceFactory.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <gulliver/I3EventLogLikelihoodCombiner.h>
#include <gulliver/I3EventHypothesis.h>

#include <boost/foreach.hpp>
#include <boost/python/list.hpp>

static const char* inputllhs_optionname = "InputLogLikelihoods";
static const char* relweights_optionname = "RelativeWeights";
static const char* multiplicity_optionname = "Multiplicity";

I3EventLogLikelihoodCombiner::I3EventLogLikelihoodCombiner(const I3Context& c):
    I3ServiceBase(c),I3EventLogLikelihoodBase(){

    AddParameter( inputllhs_optionname,
         "List of names of event loglikelihood calculation services.",
         llhNamesVector_ );

    AddParameter( relweights_optionname,
         "List of relative weights. This list should be either empty "
         "or of the same length as the list of likelihood services. "
         "If you leave it empty, then the likelihoods will be weighted "
         "with 1.0.",
         weightsVector_ );

    multiMode_ = MMode_Max;
    multiModeString_ = "Max";
    AddParameter( multiplicity_optionname,
         "Specify with this option how the likelihood combiner should "
         "obtain/define the multiplicity of an event, using one or several "
         "of the multiplicity values calculated by the likelihood services. "
         "Give either the name of one of the combined likelihood services, "
         "or any of the keywords 'Max', 'Min' or 'Sum'. Default is 'Max'. "
         "For 'Min' the minimum of the *nonzero* multiplicities will be taken.",
         multiModeString_ );

}

I3EventLogLikelihoodCombiner::~I3EventLogLikelihoodCombiner(){}

void I3EventLogLikelihoodCombiner::Configure(){
    boost::python::object inputllhs;
    GetParameter( inputllhs_optionname, inputllhs );
    GetParameter( relweights_optionname, weightsVector_ );
    GetParameter( multiplicity_optionname, multiModeString_ );
    if(!boost::python::len(inputllhs))
        log_fatal_stream(inputllhs_optionname << " must be an iterable object with at least one entry");
    for(unsigned int i=0; i<boost::python::len(inputllhs); i++){
        boost::python::object llh=inputllhs[i];
        I3EventLogLikelihoodBasePtr ehptr;
        try {
            ehptr = boost::python::extract<I3EventLogLikelihoodBasePtr>(llh);
            log_debug("got llh pointer from python");
        } catch (boost::python::error_already_set&) {
            PyErr_Clear();
            log_debug("got something else from python");
            // try again as a string
            std::string name;
            try{
                name=boost::python::extract<std::string>(llh);
            } catch(boost::python::error_already_set&){
                log_fatal_stream("Entry " << i << " in " << inputllhs_optionname
                  << " does not seem to be an I3EventLogLikelihoodBase or a string");
            }
            log_debug("something string-like from python");
            try{
                ehptr = context_.Get<I3EventLogLikelihoodBasePtr>(name);
                if(!ehptr)
                    log_fatal_stream('\'' << name << "' was not found in the context");
            } catch(std::runtime_error&){
                log_fatal_stream('\'' << name << "' does not seem to refer to an I3EventLogLikelihoodBase in the context");
            }
            log_debug("the string-like thing seems to be recognized in the context");
        }
        // survived?
        if ( ehptr ){
            log_debug("we got a usable likelihood pointer");
            logLikelihoods_.push_back(ehptr);
        } else {
            log_fatal("boost extract gave null pointer");
        }
    }

    // check correctness & consistency of likelihood services & weights
    // unsigned int nllh = llhNamesVector_.size();
    unsigned int nllh = logLikelihoods_.size();
    unsigned int nweights = weightsVector_.size();
    if ( nweights == 0 ){
        weightsVector_.resize(nllh, 1.0 );
    } else if ( weightsVector_.size() != nllh ){
        log_fatal( "(%s) The list of names of likelihood services has %u "
                   "entries and the list of weights has %u entries. The "
                   "lists must be equally long!",
                   GetName().c_str(), nllh, nweights );
    }

    llhNamesVector_.clear();
    std::vector< I3EventLogLikelihoodBasePtr >::iterator illh;
    for (illh  = logLikelihoods_.begin(); illh != logLikelihoods_.end(); ++illh ){
        llhNamesVector_.push_back((*illh)->GetName());
    }

    // define multiplicity mode
    if ( count( llhNamesVector_.begin(),
                llhNamesVector_.end(),
                multiModeString_ ) == 1 ){
        multiMode_ = MMode_Favorite;
    } else if ( multiModeString_ == "Max" ){
        multiMode_ = MMode_Max;
    } else if ( multiModeString_ == "Min" ){
        multiMode_ = MMode_Min;
    } else if ( multiModeString_ == "Sum" ){
        multiMode_ = MMode_Sum;
    } else {
        log_fatal( "(%s) got '%s' as argument for the multiplicity option; "
                   "should be either the name of one of the services, "
                   "or any of 'Max', 'Sum' or 'Average'.",
                   GetName().c_str(), multiModeString_.c_str() );
    }

}

void I3EventLogLikelihoodCombiner::SetGeometry( const I3Geometry &geo ){
    std::vector<I3EventLogLikelihoodBasePtr>::iterator illh;
    for ( illh = logLikelihoods_.begin();
          illh != logLikelihoods_.end();
        ++illh ){
        (*illh)->SetGeometry(geo);
    }
}

/**
 * This will or should be called before doing a new fit.
 */
void I3EventLogLikelihoodCombiner::SetEvent( const I3Frame &f ){
    std::vector<I3EventLogLikelihoodBasePtr>::iterator illh;
    for ( illh = logLikelihoods_.begin();
          illh != logLikelihoods_.end();
        ++illh ){
        (*illh)->SetEvent(f);
    }
}

bool I3EventLogLikelihoodCombiner::HasGradient()
{
    std::vector<I3EventLogLikelihoodBasePtr> with, without;
    BOOST_FOREACH(const I3EventLogLikelihoodBasePtr &llh, logLikelihoods_)
        if (llh->HasGradient())
            with.push_back(llh);
        else
            without.push_back(llh);

    if (with.size() == logLikelihoods_.size()) {
        return true;
    } else {
        if (without.size() != logLikelihoods_.size()) {
            std::ostringstream withlist, withoutlist;
            BOOST_FOREACH(const I3EventLogLikelihoodBasePtr &llh, with)
                withlist << llh->GetName() << ",";
            BOOST_FOREACH(const I3EventLogLikelihoodBasePtr &llh, without)
                withoutlist << llh->GetName() << ",";
            log_error("(%s) Some likelihoods support gradients (%s), but others (%s) do not. "
                "The combined likelihood will not have gradients!", this->GetName().c_str(),
                withlist.str().c_str(), withoutlist.str().c_str());
        }
        return false;
    }
}

/**
 * Get +log(likelihood) for a particular emission hypothesis
 *
 * This returns the weighted sum of the log-likelihood values
 * returned by each of the configured likelihood services.
 */
double I3EventLogLikelihoodCombiner::GetLogLikelihood( const I3EventHypothesis &ehypo ){
    double wllh = 0.0;
    std::vector<I3EventLogLikelihoodBasePtr>::iterator illh;
    std::vector<double>::iterator iweight;
    for ( illh = logLikelihoods_.begin(), iweight = weightsVector_.begin();
            (illh != logLikelihoods_.end()) && (iweight != weightsVector_.end());
            ++illh, ++iweight ){
        double llh = (*illh)->GetLogLikelihood(ehypo);
        double weight = *iweight;
        wllh += weight * llh;
    }
    return wllh;
}

double
I3EventLogLikelihoodCombiner::GetLogLikelihoodWithGradient(const I3EventHypothesis &ehypo,
    I3EventHypothesis &gradient, double weight)
{
    double wllh = 0.0;

    std::vector<I3EventLogLikelihoodBasePtr>::iterator illh;
    std::vector<double>::iterator iweight;
    for (illh = logLikelihoods_.begin(), iweight = weightsVector_.begin();
        (illh != logLikelihoods_.end()) && (iweight != weightsVector_.end());
        ++illh, ++iweight) {
        double llh = (*illh)->GetLogLikelihoodWithGradient(ehypo, gradient, (*iweight)*weight);
        wllh += weight*(*iweight)*llh;
    }

    return wllh;
}

I3FrameObjectPtr
I3EventLogLikelihoodCombiner::GetDiagnostics(const I3EventHypothesis &fitresult)
{
    std::vector<I3FrameObjectPtr> diagnostics;
    int n_valid = 0;
    BOOST_FOREACH(I3EventLogLikelihoodBasePtr &llh, logLikelihoods_) {
        I3FrameObjectPtr diag = llh->GetDiagnostics(fitresult);
        diagnostics.push_back(diag);
        if (diag)
            n_valid++;
    }
    std::vector<I3FrameObjectPtr>::iterator it = diagnostics.begin();
    while (it != diagnostics.end() && !*it)
        it++;

    if (it == diagnostics.end()) {
        return I3FrameObjectPtr();
    } else {
        if (n_valid > 1)
            log_warn("More than one likelihood can provide a diagnostics object; "
                "picking the first ('%s')",
                logLikelihoods_[std::distance(diagnostics.begin(), it)]->GetName().c_str());
        return (*it);
    }
}

/**
 * Get the multiplicity of the current input event (e.g. number of good hits).
 * The number of degrees of freedom for the fit will be calculated as:
 * multiplicity minus number of fit parameters.
 *
 * Need to think how to define this.
 */
unsigned int I3EventLogLikelihoodCombiner::GetMultiplicity(){
    unsigned int multiplicity = 0;
    switch( multiMode_ ){
        case MMode_Favorite:
            {
                I3EventLogLikelihoodBasePtr favoriteLlh =
                    context_.Get<I3EventLogLikelihoodBasePtr>( multiModeString_ );
                multiplicity = favoriteLlh->GetMultiplicity();
            }
            break;
        case MMode_Max:
            {
                unsigned int maxmulti = 0;
                unsigned int multi = 0;
                std::vector<I3EventLogLikelihoodBasePtr>::iterator illh;
                for ( illh = logLikelihoods_.begin();
                        illh != logLikelihoods_.end();
                        ++illh ){
                    multi = (*illh)->GetMultiplicity();
                    if ( multi > maxmulti ){
                        maxmulti = multi;
                    }
                }
                multiplicity = maxmulti;
            }
            break;
        case MMode_Min:
            {
                unsigned int minmulti = INT_MAX;
                unsigned int multi = 0;
                std::vector<I3EventLogLikelihoodBasePtr>::iterator illh;
                for ( illh = logLikelihoods_.begin();
                        illh != logLikelihoods_.end();
                        ++illh ){
                    multi = (*illh)->GetMultiplicity();
                    if ( (multi > 0) && (multi < minmulti) ){
                        minmulti = multi;
                    }
                }
                multiplicity = (minmulti<INT_MAX) ? minmulti : 0;
            }
            break;
        case MMode_Sum:
            {
                std::vector<I3EventLogLikelihoodBasePtr>::iterator illh;
                for ( illh = logLikelihoods_.begin();
                        illh != logLikelihoods_.end();
                        ++illh ){
                    multiplicity += (*illh)->GetMultiplicity();
                }
            }
            break;
        default:
            log_fatal("programming error");
    }
    return multiplicity;
}


typedef
I3SingleServiceFactory<I3EventLogLikelihoodCombiner,I3EventLogLikelihoodBase>
I3EventLogLikelihoodCombinerFactory;
I3_SERVICE_FACTORY( I3EventLogLikelihoodCombinerFactory );
