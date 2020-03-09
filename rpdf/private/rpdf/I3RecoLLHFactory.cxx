/**
 * @brief Provides a Gulliver likelihood service for muon based reconstructions
 *
 * @copyright (C) 2018 The Icecube Collaboration
 *
 * @file I3RecoLLH.cxx
 * @author Kevin Meagher
 * @date January 2018
 *
 */

#include "rpdf/I3RecoLLH.h"
#include "rpdf/I3RecoLLHFactory.h"

I3RecoLLHFactory::I3RecoLLHFactory( const I3Context& context ):
  I3ServiceFactory( context ),ice_model_(rpdf::H2)
{
  AddParameter("AbsorptionLength",
               "Not used: this is only included for backwards compatibility with "
               "I3GulliverIPDFPandelFactory",
               NAN);
  AddParameter("EventType",
               "Not used: this is only included for backwards compatibility with "
               "I3GulliverIPDFPandelFactory",
               "InfiniteMuon");
  AddParameter("IceFile",
               "Not used: this is only included for backwards compatibility with "
               "I3GulliverIPDFPandelFactory",
               "");
  AddParameter("MPETimingError",
               "Not used: this is only included for backwards compatibility with "
               "I3GulliverIPDFPandelFactory",
               NAN);

  likelihood_ = "SPE1st";
  AddParameter("Likelihood",
               "Specify which DOM Likelihood to combine multiple hits on the same DOM: "
               "SPE1st [default], MPE.",
               likelihood_ );

  AddParameter("IceModel",
               "Specify the the values of ice properties to use, scattering length, "
               "absorption length, etc...  default is rpdf.H2",
               rpdf::H2);

  jitter_ = 15.0*I3Units::ns;
  AddParameter("JitterTime",
               "Pandel jitter value: accounts for both PMT jitter and\n"
               "light model uncertainty. Default: 15ns",
               jitter_ );

  inputreadout_ = "";
  AddParameter("InputReadout",
               "which I3RecoPulseSeriesMap to use",
               inputreadout_ );

  noiseProb_ = 1.e-9*I3Units::hertz;
  AddParameter("NoiseProbability",
               "For each hit DOM a noise probability is added to the\n"
               "likelihood. Note that the default value (1.0e-9Hz)\n"
               "does not make physical sense.",
               noiseProb_ );

  peprob_="FastConvolutedPandel";
  AddParameter("PEProb",
               "Specify the method of computing the photoelectron probability "
               "options are GaussConvoluted [Default] or UnconvolutedPandel",
               peprob_);
}

I3RecoLLHFactory::~I3RecoLLHFactory()
{}

void I3RecoLLHFactory::Configure() {
  name_ = GetName();


  double d;
  std::string s;
  GetParameter("AbsorptionLength",d);
  if (!std::isnan(d)){
    log_warn("Parameter \"AbsorptionLength\" was set for I3RecoLLH module \"%s\" "
             "It was only provided for backwards compatibility with I3GulliverIPDFPandelFactory. "
             "You can safely stop using it",name_.c_str());
  }

  GetParameter("EventType",s);
  if (s!="InfiniteMuon"){
    log_fatal("Parameter \"EventType\" was set to something besides \"InfiniteMuon\" "
              "for I3RecoLLH module \"%s\".",name_.c_str());
  }

  GetParameter("IceFile",s);
  if (s!=""){
    log_warn("Parameter \"IceFile\" was set for I3RecoLLH module \"%s\" "
             "It was only provided for backwards compatibility with I3GulliverIPDFPandelFactory. "
             "You can safely stop using it",name_.c_str());
  }

  GetParameter("MPETimingError",d);
  if (!std::isnan(d)){
    log_warn("Parameter \"MPETimingError\" was set for I3RecoLLH module \"%s\" "
             "It was only provided for backwards compatibility with I3GulliverIPDFPandelFactory. "
             "You can safely stop using it",name_.c_str());
  }

  // The ice model can be passed either directly as an IceModel object or as an
  // integer for backwards compatibility with I3GulliverIPDFPandelFactory
  boost::python::object icemodel;
  GetParameter("IceModel",icemodel);
  std::string type = boost::python::extract<std::string>(icemodel.attr("__class__").attr("__name__"));
  if (type=="IceModel") {
    ice_model_=boost::python::extract<rpdf::IceModel>(icemodel);
  }
  else if (type=="int") {
    int icemodel_number = boost::python::extract<int>(icemodel);
    switch (icemodel_number) {
    case 0:
      ice_model_=rpdf::H0; break;
    case 1:
      ice_model_=rpdf::H1; break;
    case 2:
      ice_model_=rpdf::H2; break;
    case 3:
      ice_model_=rpdf::H3; break;
    case 4:
      ice_model_=rpdf::H4; break;
    default:
      log_fatal("Your selection of %d for parameter IceModel is invalid, "
                "It must be between 0 and 4 inclusive",icemodel_number);
    }
  }
  else{
    std::string repr= boost::python::extract<std::string>(icemodel.attr("__repr__")());
    log_fatal("You supplied a value of %s for parameter IceModel, "
              "It must be either an either a IceModel object or an integer",
              repr.c_str());
  }

  GetParameter("Likelihood", likelihood_ );
  GetParameter("JitterTime", jitter_ );
  GetParameter("InputReadout", inputreadout_ );
  GetParameter("NoiseProbability", noiseProb_ );
  GetParameter("PEProb", peprob_);
}

bool I3RecoLLHFactory::InstallService(I3Context& ctx)
{
  if(!llh_) {
    boost::shared_ptr<I3RecoLLH> rllh(new I3RecoLLH(inputreadout_,likelihood_,
                                                    peprob_,jitter_,noiseProb_,
                                                    ice_model_));
    rllh->SetName(name_);
    llh_ = I3EventLogLikelihoodBasePtr(rllh);
  }
  return ctx.Put< I3EventLogLikelihoodBase >( llh_, name_ );
}

I3_SERVICE_FACTORY(I3RecoLLHFactory)
