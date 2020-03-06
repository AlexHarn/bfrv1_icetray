/**
 * @brief Python bindings for rpdf
 *
 * @copyright (C) 2018 The Icecube Collaboration
 *
 * @file rpdf.cxx
 * @author Kevin Meagher
 * @date January 2018
 *
 */

#include "icetray/load_project.h"
#include "dataclasses/physics/I3Particle.h"
#include "gulliver/I3EventHypothesis.h"
#include "rpdf/geometry.h"
#include "rpdf/I3RecoLLH.h"

namespace bp=boost::python;


std::string IceModel_repr(const rpdf::IceModel& ice)
{
  std::stringstream out;
  out <<  "IceModel("
      << ice.absorption_length << "," << ice.tau_scale << "," << ice.scattering_length
      << ice.P1 << "," << ice.P0_CS0 << "," << ice.P0_CS1 << "," << ice.P0_CS2
      << ")";
  return out.str();
}

BOOST_PYTHON_MODULE(rpdf)
{
  load_project("rpdf", false);
  bp::import("icecube.icetray");
  bp::import("icecube.gulliver");

  //from geometry.h
  bp::class_<rpdf::IceModel>
    ("IceModel",
     "This structure holds all the relevant parameters to describe the optical\n"
     "properties of ice used for reconstruction\n"
     "\n"
     "Parameters\n"
     "----------\n"
     "absorption_length: float\n"
     "  The distance a photon can go before being absorbed on average \n"
     "tau_scale: float\n"
     "  The pandel tau parameter, represents the non-linear\n"
     "  behavior of multiple photon scatters\n"
     "scattering_length: float\n"
     "  The distance a photon can go on average before being absorbed\n"
     "P1: float\n"
     "  The coefficient in front of the distance when calculating effective distance\n"
     "P0_CS0: float\n"
     "  The constant coefficient when calculating effective distance\n"
     "P0_CS1: float\n"
     "  The coefficient in front of :math:`\\cos\\eta` when calculating effective distance\n"
     "P0_CS2:float\n"
     "  The coefficient in front of :math:`\\cos^2\\eta` when calculating effective distance\n",
     bp::init<double,double,double,double,double,double,double>()
     )
    .add_property("absorption_length", &rpdf::IceModel::absorption_length,
                  "The distance a photon can go before being absorbed on average")
    .add_property("tau_scale", &rpdf::IceModel::tau_scale,
                  "The pandel tau parameter, represents the non-linear"
                  "behavior of multiple photon scatters")
    .add_property("scattering_length", &rpdf::IceModel::scattering_length,
                  "The distance a photon can go on average before being absorbed")

    .add_property("P1", &rpdf::IceModel::P1,
                  "The coefficient in front of the distance when calculating effective distance")
    .add_property("P0_CS0", &rpdf::IceModel::P0_CS0,
                  "The constant coefficient when calculating effective distance")
    .add_property("P0_CS1", &rpdf::IceModel::P0_CS1,
                  "The coefficient in front of :math:`\\cos\\eta` when calculating effective distance")
    .add_property("P0_CS2", &rpdf::IceModel::P0_CS2,
                  "The coefficient in front of :math:`\\cos^2\\eta` when calculating effective distance")
    .add_property("rho", &rpdf::IceModel::rho,
                  "the scale parameter in the Pandel function")
    .def("__repr__",&IceModel_repr)
    ;

  bp::def("effective_distance", rpdf::effective_distance,
          (bp::arg("distance"), bp::arg("cos_eta"), bp::arg("ice_model")),
          "Calculate the effective distance between an emitter and a DOM due\n"
          "to scattering. This is a small correction for DOMs orientated away\n"
          "from the source.\n"
          "\n"
          "parameters\n"
          "----------\n"
          "distance: float\n"
          "  the actual distance traveled by the photon\n"
          "cos_eta: float\n"
          "  the cosine of the angle between the photon's trajectory and\n"
          "  the direction the DOM is pointing\n"
          "ice_model: float\n"
          "  the object holding the description of the ice\n"
          "\n"
          "returns\n"
          "-------\n"
          "float\n"
          "  the equivalent distance that would have experienced the same amount of\n"
          "  scattering if the DOM was orientated toward the photon path\n");
  bp::def("cherenkov_time",rpdf::effective_distance,
          (bp::arg("d_track"),bp::arg("d_approach")),
          "Calculate the time cherenkov photon arrives at a position d_track\n"
          "along the track, to a DOM located d_approach from the track\n"
          "\n"
          "parameters\n"
          "----------\n"
          "d_track: float\n"
          "  The distance along the track from the vertex to the point of closest approach\n"
          "d_approach: float\n"
          "  The distance of closest approach between the DOM and track\n"
          "\n"
          "returns\n"
          "-------\n"
          "float\n"
          "  the time at which the cherenkov photon arrives at the DOM\n");
  bp::def("muon_geometry",rpdf::muon_geometry,
          (bp::arg("om"),bp::arg("track"),bp::arg("ice_model")=rpdf::H2),
          "This calculates the two geometrical parameters needed by the pandel\n"
          "function to calculate the PDF.\n"
          "\n"
          "Because the calculations relating to these function are very tightly\n"
          "coupled they are both calculated in a single function.\n"
          "\n"
          "parameters\n"
          "----------\n"
          "om: I3Position\n"
          "  the position of the optical module being hit\n"
          "track: I3Particle\n"
          "  the muon track hypothesis\n"
          "ice_model: IceModel\n"
          "  the model used to describe the optical properties of the ice\n"
          "\n"
          "returns\n"
          "-------\n"
          "t_hit: float\n"
          "  the difference in time between the hit and the expected time\n"
          "  of a direct photon\n"
          "effective_distance: float\n"
          "  the distance the photon travels between the track hypothesis\n"
          "  including the correction from scattering needed to hit an OM\n"
          "  facing the other way\n"
          );

  //from pandel.h

  bp::def("pandel_pdf",&rpdf::pandel_pdf,
          (bp::arg("t_res"),bp::arg("eff_distance"),bp::arg("ice_model")),
          "Pure function implementation of the Pandel PDF. The probability of\n"
          "observing a hit at t_res a distance eff_distance away from the track\n"
          "\n"
          "parameters\n"
          "----------\n"
          "t_res: float\n"
          "  the time residual of a photon which hits a DOM\n"
          "eff_distance: float\n"
          "  the distance of the DOM from the hypothesis track accounting\n"
          "  for extra scatters caused by the PMT looking away from the track\n"
          "ice_model: float\n"
          "  a struct containing the description of optical properties of the ice\n"
          "\n"
          "returns\n"
          "-------\n"
          "float\n"
          "  the probability density of a photon arriving at :math:`t=t_{res}`\n"
          );

  bp::def("pandel_sample",&rpdf::pandel_sample,
          (bp::arg("eff_distance"),bp::arg("ice_model"),bp::arg("rng")),
          "Sample a photon arrival time from the Pandel PDF."
          "\n"
          "parameters\n"
          "----------\n"
          "eff_distance: float\n"
          "  the distance of the DOM from the hypothesis track accounting\n"
          "  for extra scatters caused by the PMT looking away from the track\n"
          "ice_model: float\n"
          "  a struct containing the description of optical properties of the ice\n"
          "rng: float\n"
          "  an I3RandomService to use for random numbers\n"
          "\n"
          "returns\n"
          "-------\n"
          "float\n"
          "  the time residual of a photon which hits a DOM\n"
          );

  bp::def("pandel_sf",&rpdf::pandel_sf,
          (bp::arg("t_res"),bp::arg("eff_distance"),bp::arg("ice_model")),
          "Pure function implementation of the Pandel complementary cumulative distribution\n"
          "function: the probability of observing a hit at t_res or later a distance\n"
          "eff_distance away from the track\n"
          "\n"
          "parameters\n"
          "----------\n"
          "t_res: float\n"
          "  the time residual of the of a photon which hits a DOM\n"
          "eff_distance: float\n"
          "  the distance of the DOM from the hypothesis track accounting\n"
          "  for extra scatters caused by the PMT looking away from the track\n"
          "ice_model: float\n"
          "  a struct containing the description of optical properties of the ice\n"
          "\n"
          "returns\n"
          "-------\n"
          "float\n"
          "  the probability of a photon arriving at :math:`t>=t_{res}`\n"
          );

  bp::class_<rpdf::PhotoElectronProbability, boost::noncopyable>
    ("PhotoElectronProbability",
     "Base Class for Photoelectron Probabilities. :py:class:`I3RecoLLH` uses a plugable\n"
     "system to pick which PE probability to use. Those object inherent from this class\n",
     bp::no_init);


  bp::class_<rpdf::FastConvolutedPandel,
             bp::bases<rpdf::PhotoElectronProbability>,
             boost::shared_ptr<rpdf::FastConvolutedPandel> >
    ("FastConvolutedPandel",
     "Contains functions for calculating the convoluted Pandel function\n"
     "and it's survival function using approximations of these functions\n"
     "which were optimized to be as fast as possible.\n"
     "\n"
     "Intended to be used with :py:class:`I3RecoLLH` parameter ``PEProb=\"GaussConvoluted\"``\n"
     "\n"
     "Parameters\n"
     "----------\n"
     "jitter: float\n"
     "  the width of the Gaussian which is convoluted with the pandel function\n"
     "ice_model: IceModel\n"
     "  a struct containing the description of optical properties of the ice\n",
     bp::init<double,rpdf::IceModel>()
     )
    .def("pdf", &rpdf::FastConvolutedPandel::pdf,
         (bp::arg("t_res"),bp::arg("eff_distance")),
         "Calculate the convoluted pandel function using the approximation by\n"
         "van Eindhoven et al, by breaking it into 5 regions and evaluating a different\n"
         "approximation in each region\n"
         "\n"
         "parameters\n"
         "----------\n"
         "t_res: float\n"
         "  the time residual of the of a photon which hits a DOM\n"
         "eff_distance: float\n"
         "  the distance of the DOM from the hypothesis track accounting\n"
         "  for extra scatters caused by the PMT looking away from the track\n"
         "\n"
         "returns\n"
         "-------\n"
         "float\n"
         "  the probability density of a photon arriving at math:`t=t_res`\n"
         )
    .def("sf", &rpdf::FastConvolutedPandel::sf,
         (bp::arg("t_res"),bp::arg("eff_distance")),
         "Calculate complementary cumulative distribution function of the\n"
         "convoluted pandel function using an approximate integral by D. Chirkin.\n"
         "\n"
         "The approximation replaces the Gaussian with a box of the same first and second\n"
         "moment. An additional term is added to account for the behavior at the\n"
         "singularity at :math:`t=0`. This approximation is very accurate. See\n"
         "https://icecube.wisc.edu/~dima/work/WISC/cpdf/a.pdf for more information\n"
         "\n"
         "parameters\n"
         "----------\n"
         "t_res: float\n"
         "  the time residual of the of a photon which hits a DOM\n"
         "eff_distance: float\n"
         "  the distance of the DOM from the hypothesis track accounting\n"
         "  for extra scatters caused by the PMT looking away from the track\n"
         "\n"
         "returns\n"
         "-------\n"
         "float\n"
         "  the probability density of a photon arriving at math:`t>=t_res`\n"
         )
    ;

  bp::class_<rpdf::UnconvolutedPandel,
             bp::bases<rpdf::PhotoElectronProbability>,
             boost::shared_ptr<rpdf::UnconvolutedPandel> >
    ("UnconvolutedPandel",

     "Contains functions for calculating the pandel function without any\n"
     "convolution. Intended for use with :py:class:`I3RecoLLH` with parameter\n"
     "``PEProb=\"UnconvolutedPandel\"``",
     bp::init<rpdf::IceModel>())
    .def("pdf", &rpdf::UnconvolutedPandel::pdf,
 (bp::arg("t_res"),bp::arg("eff_distance")),
         "Calculate the unconvouted pandel function using boost's implementation\n"
         "of the gamma distribution\n"
         "\n"
         "parameters\n"
         "----------\n"
         "t_res: float\n"
         "  the time residual of the of a photon which hits a DOM\n"
         "eff_distance: float\n"
         "  the distance of the DOM from the hypothesis track accounting\n"
         "  for extra scatters caused by the PMT looking away from the track\n"
         "\n"
         "returns\n"
         "-------\n"
         "float\n"
         "  the probability density of a photon arriving at math:`t=t_res`\n"
         )
    .def("sf", &rpdf::UnconvolutedPandel::sf,
 (bp::arg("t_res"),bp::arg("eff_distance")),
         "Calculate complementary cumulative distribution function of the unconvouted pandel\n"
         "function using gsl's implementation of the gamma distribution\n"
         "\n"
         "The approximation replaces the Gaussian with a box of the same first and second\n"
         "moment. An additional term is added to account for the behavior at the\n"
         "singularity at :math:`t=0`. This approximation is very accurate. See\n"
         "https://icecube.wisc.edu/~dima/work/WISC/cpdf/a.pdf for more information\n"
         "\n"
         "parameters\n"
         "----------\n"
         "t_res: float\n"
         "  the time residual of the of a photon which hits a DOM\n"
         "eff_distance: float\n"
         "  the distance of the DOM from the hypothesis track accounting\n"
         "  for extra scatters caused by the PMT looking away from the track\n"
         "\n"
         "returns\n"
         "-------\n"
         "float\n"
         "  the probability density of a photon arriving at math:`t>=t_res`\n"
         )
    ;


  bp::def("SPEFunc",&rpdf::SPEfunc::operator(),
          (bp::arg("peprob"),bp::arg("t_res"),bp::arg("eff_distance"),bp::arg("Npe")),
          "Function for calculating the Single-Photo-Electron likelihood for an individual DOM.\n"
          "\n"
          "It takes the time of the first from the hit series and simply returns the\n"
          "PDF for that hit.\n"
          "Intended for use with :py:class:`I3RecoLLH` parameter DOMLikelihood=SPE1st.\n"
          "\n"
          "parameters\n"
          "----------\n"
          "peprob: PhotoElectronProbability\n"
          "  the class representing the photoelectron probability calculation\n"
          "t_res: float\n"
          "  the time residual of the of a photon which hits a DOM\n"
          "eff_distance: float\n"
          "  the distance of the DOM from the hypothesis track accounting\n"
          "  for extra scatters caused by the PMT looking away from the track\n"
          "Npe: float\n"
          "  the number of photoelectrons observed by this DOM in this event\n"
          "\n"
          "returns\n"
          "-------\n"
          "float:\n"
          "  the SPE1st Likelihood for this DOM\n"
          );
  bp::def("MPEfunc",&rpdf::MPEfunc::operator(),
          (bp::arg("peprob"),bp::arg("t_res"),bp::arg("eff_distance"),bp::arg("Npe")),
          "Function for calculating the Multi-Photo-Electron likelihood for an individual DOM\n"
          "\n"
          "MPE is the likelihood of observing the first photon at :math:`t=t_{res}` given that\n"
          "the DOM saw ``Npe`` photoelectrons total. This is intended for use with\n"
          ":py:class:`I3RecoLLH` parameter ``DOMLikelihood=\"MPE\"``.\n"
          "\n"
          "parameters\n"
          "----------\n"
          "peprob: PhotoElectronProbability\n"
          "  the class representing the photoelectron probability calculation\n"
          "t_res: float\n"
          "  the time residual of the of a photon which hits a DOM\n"
          "eff_distance: float\n"
          "  the distance of the DOM from the hypothesis track accounting\n"
          "  for extra scatters caused by the PMT looking away from the track\n"
          "Npe: float\n"
          "  the number of photoelectrons observed by this DOM in this event\n"
          "\n"
          "returns\n"
          "-------\n"
          "float:\n"
          "  the MPE Likelihood for this DOM\n"
          );

  bp::class_<I3RecoLLH,
             bp::bases<I3EventLogLikelihoodBase>,
             boost::shared_ptr<I3RecoLLH>,
             boost::noncopyable>
    (
     "I3RecoLLH",
     "A gulliver likelihood service for reconstructions which use\n"
     "the Pandel function.\n"
     "\n"
     "It calculates the  likelihood of a muon track hypothesis using an analytic description of light\n"
     "propagation through the ice. This analytic description is referred to as the\n"
     "Pandel function. This implementation assumes all ice in the detector has\n"
     "uniform optical properties.\n"
     "\n"
     "Parameters\n"
     "----------\n"
     "input_readout: str\n"
     "  The name of the :py:class:`I3RecoPulseSeriesMap` to read from the frame\n"
     "likelihood: str\n"
     "  The name of the DOM likelihood algorithm to use.\n"
     "  Options are ``\"GaussConvoluted\"`` or ``\"UnconvolutedPandel\"``\n"
     "peprob: str\n"
     "  The name of the photoelectron probability calculation to use.\n"
     "  Options are ``\"SPE1st\"`` or ``\"MPE\"``.\n"
     "jitter: float\n"
     "  the width of the Gaussian which is convoluted with the pandel function\n"
     "noise: float\n"
     "  The frequency of noise hits to use in the reconstruction.\n"
     "ice_model: IceModel\n"
     "  A struct containing the description of optical properties of the ice.\n",
     bp::init<std::string,std::string,std::string,double,double,rpdf::IceModel>
     ((bp::arg("input_readout"),
       bp::arg("likelihood"),
       bp::arg("pe_prob"),
       bp::arg("jitter_time"),
       bp::arg("noise_probability"),
       bp::arg("ice_model")
       ))
     )
    .def("set_geometry", &I3RecoLLH::SetGeometry,
         (bp::arg("geometry")),
         "This is called when the reader gets a new geometry.\n"
         "It saves the geometry for :py:func:`SetEvent` to save the location\n"
         "of each DOM in the hit cache\n"
         "\n"
         "Parameters\n"
         "----------\n"
         "geometry: I3Geometry\n"
         "  The Geometry to use to calculate likelihoods.\n")
    .def("set_event",    &I3RecoLLH::SetEvent,
         (bp::arg("frame")),
         "Called when a new event occurs, this reads the new event\n"
         "and the pulse map in the frame.  and calls :py:func:`SetHitCache`\n"
         "\n"
         "Parameters\n"
         "----------\n"
         "frame: I3Frame\n"
         "  Frame the frame to extract the I3RecoPulseSeriesMap from\n")
    .def("set_pulse_map",&I3RecoLLH::SetPulseMap,
         (bp::arg("publse_map")),
         "Calculates the hit cache from the geometry and given pulse map\n"
         "\n"
         "Parameters\n"
         "----------\n"
         "pulse_map: I3RecoPulseMap\n"
         "  map containing the pulses for calculating the likelihood\n")
    .def("get_log_likelihood",&I3RecoLLH::GetLogLikelihood,
         (bp::arg("event_hypothesis")),
         "Calculate the log likelihood of an event hypothesis\n"
         "for the current event\n"
         "\n"
         "Parameters\n"
         "----------\n"
         "event_hypothesis: I3EventHypothesis\n"
         "  event_hypothesis the gulliver event hypothesis (which contains an\n"
         "  I3Parthcle) with which to calculate the likelihood\n"
         "\n"
         "Returns\n"
         "-------\n"
         "float:\n"
         "  the likelihood of the hypothesis\n")
    .def("get_multiplicity",&I3RecoLLH::GetMultiplicity,
         "returns\n"
         "-------\n"
         "int:"
         "  the multiplicity of the event in question: the number of hit DOMs\n"
         )
    ;

  bp::scope().attr("C_VACUUM")=rpdf::constants::C_VACUUM;
  bp::scope().attr("N_ICE_G")=rpdf::constants::N_ICE_G;
  bp::scope().attr("C_ICE_G")=rpdf::constants::C_ICE_G;
  bp::scope().attr("SIN_CHERENKOV")=rpdf::constants::SIN_CHERENKOV;
  bp::scope().attr("TAN_CHERENKOV")=rpdf::constants::TAN_CHERENKOV;
  bp::scope().attr("EFF_TAN_CHERENKOV")=rpdf::constants::EFF_TAN_CHERENKOV;

  bp::scope().attr("H0")=rpdf::H0;
  bp::scope().attr("H1")=rpdf::H1;
  bp::scope().attr("H2")=rpdf::H2;
  bp::scope().attr("H3")=rpdf::H3;
  bp::scope().attr("H4")=rpdf::H4;
}

