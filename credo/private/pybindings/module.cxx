/*
 *  $Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#include <icetray/load_project.h>
#include <boost/preprocessor.hpp>

#include "credo/I3CredoEventStatistics.h"
#include "credo/I3CredoDOMCache.h"
#include "credo/I3PoissonGaussLogLikelihood.h"
#include "credo/I3CredoFunctions.h"

#include <icetray/I3Frame.h>
#include <icetray/I3Logging.h>
#include "photonics-service/I3PhotonicsService.h"
#include <gulliver/I3EventHypothesis.h>

using namespace boost::python;


/* I3CredoDOMCache ************************************************************/

void register_I3CredoDOMCache() {
    #define CACHEFIELDS (position)(npe)(nPoissonPE)(baseContribution)       \
                        (outOfBoundContribution)(firstPulseTime)            \
                        (llhcontrib)(expectedAmpliude)(amplitudeCorrection) \
                        (type)(saturated)(relativedomefficiency)
    class_<I3CredoDOMCache>( "I3CredoDOMCache")
        BOOST_PP_SEQ_FOR_EACH(WRAP_RO, I3CredoDOMCache, CACHEFIELDS)
    ;   
    enum_<I3CredoDOMCache::OMType>("OMType")
        .value("UNSPECIFIED",       I3CredoDOMCache::UNSPECIFIED)
        .value("ICECUBE_DOM",       I3CredoDOMCache::ICECUBEDOM)
        .value("AMANDA_ELECTRICAL", I3CredoDOMCache::AMANDA_ELECTRICAL)
        .value("AMANDA_OPTICAL",    I3CredoDOMCache::AMANDA_OPTICAL)
        .value("DEEPCOREDOM",       I3CredoDOMCache::DEEPCOREDOM)
        .export_values()
        ;
    
    class_<std::map<OMKey, I3CredoDOMCache> >("I3CredoDOMCacheMap")
        .def(map_indexing_suite<std::map<OMKey, I3CredoDOMCache> >())
    ;

}

/* I3CredoEventStatistics *****************************************************/

void register_I3CredoEventStatistics() {

    #define EVENTSTATISTICSFIELDS (nCh_hit)(nCh_selected)(nCh_minball)(npe_all)\
                                  (npe_selected_all)(npe_selected_poisson) \
                                  (npe_max)(nPulses_all_good)(nPulses_all_bad) \
                                  (nPulses_selected_poisson) \
                                  (nPulses_selected_gaussian)(skipped_saturated)\
                                  (skipped_mincharge)(skipped_noStatus)(skipped_noHV)\
                                  (failed_photorec_calls)(start_time)(end_time)

    class_<I3CredoEventStatistics>( "I3CredoEventStatistics")
        BOOST_PP_SEQ_FOR_EACH(WRAP_RO, I3CredoEventStatistics, EVENTSTATISTICSFIELDS)
    ;

}

/* I3PoissonGaussLogLikelihood ************************************************/

// helper function to pass list of OMKeys to the likelihood
void SetBadDOMsWrapper(I3PoissonGaussLogLikelihood& self, const std::string& badDOMListName, 
                       const boost::python::list& badDOMList) {
    I3VectorOMKey baddomvect = I3VectorOMKey();
    // loop through the list and check if the contained object is an OMKey
    for (int i = 0; i < len(badDOMList); ++i) {
        extract<OMKey&> keyextractor(badDOMList[i]);
        if (keyextractor.check()) {
                OMKey& key = keyextractor();
                baddomvect.push_back(key);
        }
        else {
            log_warn("The list of bad DOMs contain at index %d an object that is not an OMKey!", i);
        }
    }
    self.SetBadDOMs(badDOMListName, baddomvect);
}

void register_I3PoissonGaussLogLikelihood() {
    #define LLHMETHODS (SetEvent)(GetLogLikelihood)(GetMultiplicity) \
                       (GetPDF)(SetPDF)(SetSaturation) 

    class_<I3PoissonGaussLogLikelihood, 
           I3PoissonGaussLogLikelihoodPtr,
           boost::noncopyable> ("I3PoissonGaussLogLikelihood")
        .def(init<const std::string&, I3PhotonicsServicePtr, const std::string&,
                  double, double, double, double, 
                  bool, bool, bool, double>())
        .def("SetBadDOMs", &SetBadDOMsWrapper)
        .def_readonly("__cacheMap__", &I3PoissonGaussLogLikelihood::cacheMap_)
        .def_readonly("stats", &I3PoissonGaussLogLikelihood::stats_)
        BOOST_PP_SEQ_FOR_EACH(WRAP_DEF, I3PoissonGaussLogLikelihood, LLHMETHODS)
    ;
}

void register_I3CredoFunctions() {
    def("GetPhotonicsCorrectionFactor", 
      &I3Credo::GetPhotonicsCorrectionFactor, 
      args("type","amplitude", "distance", "atwdonly") );
    def("GetPulseMapWithGaps", &I3Credo::GetPulseMapWithGaps);
}

BOOST_PYTHON_MODULE(credo) {
    load_project("credo", false);
    register_I3CredoDOMCache();
    register_I3CredoEventStatistics();
    register_I3PoissonGaussLogLikelihood();
    register_I3CredoFunctions();
}
