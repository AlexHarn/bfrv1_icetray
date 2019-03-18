/**<
 *  Implementation of I3MuonEnergy (DDDDR)
 *
 *  @file I3MuonEnergy.cxx
 *  @date $Date: 14/01/23 
 *  @author Hans-Peter Bretz (hbretz@icecube.wisc.edu) 
 *  @author Patrick Berghaus
 */
#include "limits.h"

#include "ddddr/ExpoFcn.h"
#include "ddddr/TomFFcn.h"
#include "ddddr/I3MuonEnergy.h"
#include "ddddr/MuonEnergyFunctions.h"
#include "ddddr/ExpoFunction.h"
#include "ddddr/TomFFunction.h"

#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/geometry/I3TankGeo.h"
#include "dataclasses/geometry/I3OMGeo.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "phys-services/I3Calculator.h"
#include "icetray/OMKey.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3Vector.h"
#include "phys-services/I3Calculator.h"
#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"
#include "simclasses/I3MMCTrack.h"

// some default values
static const double SURFACE_HEIGHT = 1948.07; // https://wiki.icecube.wisc.edu/index.php/Coordinate_system
static const bool FIXB = true;
static const bool FIXGAMMA = true;

// default values for Minuit parameters
static const double EXPO_NORM_INIT = 10;
static const double EXPO_NORM_STEPS = 0.1;
static const double EXPO_NORM_MINV = 0;
static const double EXPO_NORM_MAXV = 15;

static const double EXPO_EXP_INIT = 1e-3;
static const double EXPO_EXP_STEPS = 1e-1;
static const double EXPO_EXP_MINV = -1e-1;
static const double EXPO_EXP_MAXV = 1e-1;

static const double TOMF_NORM_INIT = 1e3;
static const double TOMF_NORM_STEPS = 100;
static const double TOMF_NORM_MINV = 1e-10;
static const double TOMF_NORM_MAXV = 1e10;

static const double TOMF_B_INIT = 3.3e-4;
static const double TOMF_B_STEPS = 1e-3;
static const double TOMF_B_MINV = 1e-4;
static const double TOMF_B_MAXV = 1e-1;

static const double TOMF_GAMMA_INIT = 1.757;
static const double TOMF_GAMMA_STEPS = 1e-1;
static const double TOMF_GAMMA_MINV = 1;
static const double TOMF_GAMMA_MAXV = 4;

static const int CASCADE_CONTAINMENT_DISTANCE = 150; 

/**
 * @brief Helper function that compares two DOMDATA structs
 * by their position along the muon bundle track.
 *
 * @param r1
 * @param r2
 *
 * @return True if first DOMDATA is earlier than second.
 */
bool EarlierThan(const DOMDATA & r1, const DOMDATA & r2)
{
	if (r1.slant < r2.slant) 
		return true;
	else 
		return false;
}

/**
 * @brief Helper function that compares the energy loss
 * measured by two different DOMs, given in DOMDATA structs.
 *
 * @param r1 
 * @param r2
 *
 * @return True if first DOMDATA contains energy loss smaller than the second one.
 */
bool smallerThan(const DOMDATA & r1, const DOMDATA & r2)
{
	if (r1.dEdX < r2.dEdX) 
		return true;
	else 
		return false;
}

// Return the distance between a DOM and a given position
double geometricDistance(const DOMDATA r1, I3Position p1)
{
	//return sqrt( pow(r1.x - r2.x, 2) + pow(r1.y - r2.y, 2) + pow(r1.z - r2.z, 2));
	return (I3Position(r1.x, r1.y, r1.z) - p1).Magnitude();
}

double medianOfVector(boost::shared_ptr<I3Vector<double> > vec)
{
	if (vec->size() % 2 != 0)
	{
		size_t n = vec->size() / 2;
		std::nth_element(vec->begin(), vec->begin()+n, vec->end());
		return (*vec)[n];
	}
	else
	{
		size_t n = vec->size() / 2;
		std::nth_element(vec->begin(), vec->begin()+n, vec->end());
		std::nth_element(vec->begin(), vec->begin()+(n-1), vec->end());
		return ((*vec)[n]+(*vec)[n-1])/2;
	}
}	

I3_MODULE(I3MuonEnergy);

I3MuonEnergy::I3MuonEnergy(const I3Context & ctx)
	: I3ConditionalModule(ctx)
{
	//Define all parameters for the module.
	trackName_ = "";
	AddParameter("Seed", "Name of the seed track", trackName_);
	inIcePulseName_ = "TWOfflinePulseSeriesReco";
	AddParameter("InputPulses", "name of the input data", inIcePulseName_);

	std::string i3src = getenv("I3_SRC");
	iceFileName_ = i3src + "/ddddr/resources/tables/implied";
	AddParameter("IceModelFileName", "name of the ice model file name", iceFileName_);
	framePrefix_ = "";
	AddParameter("Prefix", "Prefix for objects that are stored in the frame", framePrefix_);
	levelDist_ = 19.;
	AddParameter("LevelDist", "For distances smaller than LevelDist, the light yield at the DOMs "
			                  "is assumed to scale linearly with the distance to the track.", levelDist_);
	fitFunctionInt_ = 0;
	AddParameter("Method", "-1: no fit, 0: expo; 1: tomF", fitFunctionInt_);
	fixB_ = FIXB;
	AddParameter("FixB", "fix free parameter b in tomF's function", fixB_);
	fixGamma_ = FIXGAMMA;
	AddParameter("FixGamma", "fix free parameter gamma in tomF's function", fixGamma_);
	saveDomResults_ = false;
	AddParameter("SaveDomResults", "If true, saves impact, slant depth and dEdX for each DOM to frame", saveDomResults_);
	purityAdjust_ = 0.00;
	AddParameter("PurityAdjust", "Scales the atten. length with a linear function of the depth, pos. values "
			                     "decrease the atten. length. Default is 0. (leaves atten. length unchanged)."
								 , purityAdjust_);
	maxImpact_ = 100;
	AddParameter("MaxImpact", "Max. closest distance of a DOM to the muon track to be considered for the reconstruction", maxImpact_);
	fitBinWidth_ = 50;
	AddParameter("BinWidth", "Bin width of the slant depth for the binned dEdX fit.", fitBinWidth_);
	mcTreeName_ = "";
	AddParameter("I3MCTree", "I3MCTree to use in case Monte Carlo track is used for the reconstruction.", 
			mcTreeName_);
	mmcName_ = "";
	AddParameter("MMCTrackList", "MMCTrackList to use in case Monte Carlo track is used for the reconstruction",
		   mmcName_);
	useMonteCarloTrack_ = false;
	AddParameter("UseMonteCarloTrack", "Use Monte Carlo track instead of reconstructed track for the energy loss reco.",
		   useMonteCarloTrack_);
	slantBinNo_ = 20;
	AddParameter("SlantBinNumber", "Number of bins for slant depth. Only used together with Monte Carlo.",
			slantBinNo_);
	minuitMaxIterations_ = 500;
	AddParameter("MaxIterations", "Maximum number of iterations for minuit.", minuitMaxIterations_);
	badDomListName_ = "BadDomsList";

	minuitTolerance_ = 0.00001;
	AddParameter("Tolerance", "Tolerance to pass to Minuit", minuitTolerance_);
	AddParameter("BadDomListName", "Name of the BadDomList frame object. Default is \"BadDomList\".",
			badDomListName_);
	AddOutBox("OutBox");

	//Minuit parameters, for now just fixed here
	//minuitMaxIterations_ = 10000;
	minuitPrintLevel_ = -2;
	minuitStrategy_ = 2;
	minuitAlgorithm_ = "MIGRAD";
}

I3MuonEnergy::~I3MuonEnergy() {}

void I3MuonEnergy::Configure()
{
	GetParameter("Seed", trackName_);
	GetParameter("InputPulses", inIcePulseName_);
	GetParameter("IceModelFileName", iceFileName_);
	GetParameter("Prefix", framePrefix_);
	GetParameter("LevelDist", levelDist_);
	GetParameter("Method", fitFunctionInt_);
	GetParameter("BinWidth", fitBinWidth_);
	GetParameter("FixB", fixB_);
	GetParameter("FixGamma", fixGamma_);
	GetParameter("SaveDomResults", saveDomResults_);
	GetParameter("PurityAdjust", purityAdjust_);
	GetParameter("MaxImpact", maxImpact_);
	GetParameter("I3MCTree", mcTreeName_);
	GetParameter("MMCTrackList", mmcName_);
	GetParameter("UseMonteCarloTrack", useMonteCarloTrack_);
	GetParameter("SlantBinNumber", slantBinNo_);
	GetParameter("MaxIterations", minuitMaxIterations_);
	GetParameter("Tolerance", minuitTolerance_);
	GetParameter("BadDomListName", badDomListName_);


	// Consistency checks__________________________________________________
	if (inIcePulseName_ == "")
		log_fatal("You have to specify a Pulse map.");

#ifndef USE_MINUIT2
	if (fitFunctionInt_ != -1)
                log_fatal("Minuit2 was not found, so fitting cannot be performed. Set "
                          "the Method to -1 to continue without minimization");
#endif

	// check if correct track parameters are set. Warn user if ambiguous
	if (useMonteCarloTrack_)
	{
		if (mcTreeName_ == "")
			log_fatal("You have to specify the I3MCTree if you want to use the true MC track.");
		if (mmcName_ == "")
			log_fatal("You have to specify the MMCTrackList if you want to use the true MC track.");

		if (framePrefix_ == "")
		{
			outputDOMs_ = "I3MuonEnergyMC";
			outputPars_ = "I3MuonEnergyMC";
		}
		else
		{
			outputDOMs_ = framePrefix_;
			outputPars_ = framePrefix_;
		}

		if (trackName_ != "")
			log_warn("You set useMonteCarloTrack to true but also specified a track name. "
					 "Continuing assuming you want to use the monte carlo track and not %s.", trackName_.c_str());
	}
	else
	{
		if (trackName_ == "")
			log_fatal("You have to specify a Seed track for the energy loss reconstruction.");

		if (mcTreeName_ != "" or mmcName_ != "")
			log_warn("You specified a I3MCTree and/or an MMCTrackList, but set useMonteCarloTrack to false. "
					 "Continuing assuming ou want to use %s and not the Monte Carlo track.", trackName_.c_str());

		if (framePrefix_ == "")
		{
			outputDOMs_ = "I3MuonEnergy" + trackName_;
			outputPars_ = "I3MuonEnergy" + trackName_;
		}
		else
		{
			outputDOMs_ = framePrefix_;
			outputPars_ = framePrefix_;
		}
	}
	outputCascadePars_ = outputPars_ + "CascadeParams";
	outputPars_ += "Params";

	
	// Set degrees of freedom according to chosen fit function
	if (fitFunctionInt_ == -1) {
		fitFunction_ = FitFunction::NOFCN;
		nDoF_ = 0;
	}
	else if (fitFunctionInt_ == 0)
	{
		fitFunction_ = FitFunction::EXPOFCN;
		nDoF_ = 2;
	}
	else if (fitFunctionInt_ == 1)
	{
		fitFunction_ = FitFunction::EXPOFCN;
		fitFunction_ = FitFunction::TOMFFCN;
		nDoF_ = 3;
		if (fixGamma_)
			nDoF_--;
		if (fixB_)
		        nDoF_--;
	}
	else
	{
		log_fatal("Please specify the fit function!");
	}

	if (fitFunction_ == FitFunction::EXPOFCN && (fixGamma_ != FIXGAMMA || fixB_ != FIXB))
	{
		// fitFunction is expo, but parameters for tomF have been set
		log_warn("You have changed the parameters for the tomF function but have chosen the exponential fit. "
				 "Continuing with the exponential fit. Set parameter Method to 1 to use the tomF function.");
	}

	lambdaFile_ = readLambdaFile(iceFileName_);

	// count number of tracks fitted
	tracksFitted_ = 0;

	surface_height_ = SURFACE_HEIGHT;
}


void I3MuonEnergy::Finish()
{
	log_info("Processed %d tracks", tracksFitted_);
}

void I3MuonEnergy::Geometry(I3FramePtr frame)
{
	geometry_ = frame->Get<I3GeometryConstPtr>();
	omGeo_ = I3OMGeoMapPtr(new I3OMGeoMap(geometry_->omgeo));
	PushFrame(frame, "OutBox");
}

void I3MuonEnergy::Calibration(I3FramePtr frame)
{
	I3CalibrationConstPtr calibration = frame->Get<I3CalibrationConstPtr>();
	domCal_ = I3DOMCalibrationMapPtr(new I3DOMCalibrationMap(calibration->domCal));
	PushFrame(frame, "OutBox");
}

void I3MuonEnergy::DetectorStatus(I3FramePtr frame)
{
	badDomList_ = frame->Get<I3VectorOMKeyConstPtr>(badDomListName_);

	if(badDomListName_.empty())
    {
        badDomList_ = I3VectorOMKeyConstPtr(new I3VectorOMKey());
    }
    if(!badDomList_)
    {
        log_warn("(%s) BadDomList %s can not be found in frame. Using all DOMs...",
            GetName().c_str(), badDomListName_.c_str());
        badDomList_ = I3VectorOMKeyConstPtr(new I3VectorOMKey());
    }
    log_debug("(%s) BadDomList loaded with %zu DOMs.",
        GetName().c_str(), badDomList_->size());
	PushFrame(frame, "OutBox");
}

void I3MuonEnergy::Physics(I3FramePtr frame)
{
	//We need a track fit to perform DDDDR.
	if (!(useMonteCarloTrack_) && (!(frame->Has(trackName_) && frame->Has(inIcePulseName_))))
	{
		PushFrame(frame, "OutBox");
		return;
	}
	else if (useMonteCarloTrack_ && ( !(frame->Has(mmcName_)) || !(frame->Has(mcTreeName_)) ))
	{
		PushFrame(frame, "OutBox");
		return;
	}

	std::vector<DOMDATA> icdata_;

	I3DetectorStatusConstPtr status = frame->Get<I3DetectorStatusConstPtr>();

	double prim_zen = 0.;

	I3ParticlePtr track = getSeedTrack(frame);

	if (useMonteCarloTrack_)
	{
		prim_zen = track->GetZenith();
		surface_height_ = SURFACE_HEIGHT;
	}

	//The vectors will contain slant length/impact/energy loss for each DOM along the track.
	boost::shared_ptr<I3Vector<double> > slant_(new I3Vector<double>);
	boost::shared_ptr<I3Vector<double> > impact_(new I3Vector<double>);
	boost::shared_ptr<I3Vector<double> > dEdX_(new I3Vector<double>);

	I3RecoPulseSeriesMapConstPtr inIcePulsesMap 
		= frame->Get<I3RecoPulseSeriesMapConstPtr>(inIcePulseName_);

	double tot_eloss = 0;
	for (I3OMGeoMap::const_iterator om_iter = omGeo_->begin();
			om_iter != omGeo_->end(); ++om_iter)
	{
		/*Loop over all DOMs, check if they are close enough to the track,
		 * get information about depth, slant depth and calculate the energy loss.
		 * All relevant data for each DOM is saved in a vector of structs of 
		 * type DOMDATA.
		 */

		OMKey omkey = om_iter->first;
    
		if (om_iter->second.omtype != I3OMGeo::IceCube)
			continue;

		// skip bad doms
		if (std::find(badDomList_->begin(), badDomList_->end(), omkey) != badDomList_->end())
			continue;

		I3Position om_pos = om_iter->second.position;

		double impact = I3Calculator::ClosestApproachDistance(*track, om_pos);

		if (!(impact<maxImpact_)) 
		{
			continue;
		}

		DOMDATA data_of_this_dom = {};

		double z = om_pos.GetZ();

		double lambda = MuonEnergyFunctions::getLambda(z, lambdaFile_);
		lambda *= 1-purityAdjust_*(z+500)/1000;

		if (lambda<=1e-2) 
		{
			log_warn("The eff. attenuation length lambda is too small, z = %f!",z);
		}

		double slant = MuonEnergyFunctions::getSlantDepth(*track, om_pos, surface_height_);

		data_of_this_dom.depth = SURFACE_HEIGHT - om_pos.GetZ();
		data_of_this_dom.impact = impact;
		data_of_this_dom.slant = slant;
		data_of_this_dom.lambda = lambda;
		data_of_this_dom.x = om_pos.GetX();
		data_of_this_dom.y = om_pos.GetY();
		data_of_this_dom.z = om_pos.GetZ();
		std::map<OMKey, I3DOMCalibration>::const_iterator
		    calib = domCal_->find(omkey);

		// This is a bit of a hack, but there might be DOMs that are not in the BadDomsList, but 
		// have a reldomeff of NaN since they are problem doms. I'll treat these DOMs like bad DOMs.
		if (std::isnan(calib->second.GetRelativeDomEff())) 
		{
			log_debug("Found DOM that is missing in BadDomsList but has Relative DOM Efficiency of NaN. Skipping this DOM.");
			continue;
		}
		data_of_this_dom.eff = calib->second.GetRelativeDomEff();

		// determine charge and energy loss 
		if (inIcePulsesMap->find(omkey) == inIcePulsesMap->end())
		{
			data_of_this_dom.charge = 0;
			data_of_this_dom.time = 0;
			data_of_this_dom.dEdX = 0;
		}
		else
		{
			const I3RecoPulseSeries pulses = inIcePulsesMap->find(omkey)->second;
			try
			{
				data_of_this_dom.time = pulses.at(0).GetTime();
			}
			catch(const std::out_of_range& oor)
			{
				data_of_this_dom.charge = 0;
				data_of_this_dom.time = 0;
				data_of_this_dom.dEdX = 0;
			}
			double charge = 0;
			for (I3RecoPulseSeries::const_iterator pulse_iter = pulses.begin();
					pulse_iter != pulses.end(); ++pulse_iter)
			{
				charge += pulse_iter->GetCharge();
			}
			data_of_this_dom.charge = charge;

			double dEdX = MuonEnergyFunctions::eLoss(charge, impact, lambda, data_of_this_dom.z,
				   	data_of_this_dom.eff, levelDist_);
			if (std::isnan(dEdX))
			{
				log_debug("dEdX is nan for charge %g, impact %g, lambda %g, z %g, dom_eff %g",
						charge, impact, lambda, data_of_this_dom.z, data_of_this_dom.eff);
			}
			//if (dEdX < 0.000001)
			//{
			//	log_warn("Got dEdX=%f for charge %f, impact %f, lambda %f", dEdX, charge, impact, lambda);
			//}
			data_of_this_dom.dEdX = dEdX;
		}

		tot_eloss += data_of_this_dom.dEdX;
		slant_->push_back(slant);
		impact_->push_back(impact);
		dEdX_->push_back(data_of_this_dom.dEdX);
		icdata_.push_back(data_of_this_dom);

	}//for

	// If some DOMs along the track saw light, create binned average, perform fit
	// and save results to the frame.
	if (icdata_.size() >= 2 && tot_eloss > 0.)
	{
		log_debug("Found %d DOMs and a total energy loss of %.2g", int(icdata_.size()), tot_eloss);
		boost::shared_ptr<I3MuonEnergyProfile> eprofile = getMuonEnergyProfile(frame, surface_height_, prim_zen, icdata_);

		std::vector<double> slantBins_;
		std::vector<double> dEdXPerBin_;


		// Only perform fit and give results with at least a few bins with energy loss
		if (eprofile->GetNBins()>(2))
		{
			log_debug("Created energy loss histogram with %d bins total and %d bins with non zero content, fitting distribution.", eprofile->GetNBins(), eprofile->GetNNonZeroBins());
			slantBins_ = eprofile->GetBinCenters();
			dEdXPerBin_ = eprofile->GetBinContents();

			I3MuonEnergyParamsPtr results = getMuonEnergyParams(eprofile, slantBins_, dEdXPerBin_);

			tracksFitted_++;

			// Try to fit cascade like energy loss around the track position corresponding 
			// to the DOM with the highest energy loss

			// get DOM with highest energy loss
			std::vector<DOMDATA>::iterator maxDOMElossIterator = 
				 std::max_element(icdata_.begin(), icdata_.end(), smallerThan);
			DOMDATA maxDOM = *maxDOMElossIterator;

			// get projection of DOM position on the track
			I3Position cascadePosition = I3Calculator::ClosestApproachPosition(*track,
					I3Position(maxDOM.x, maxDOM.y, maxDOM.z));


			boost::shared_ptr<I3Vector<double> > cascadeEnergyLosses(new I3Vector<double>);
			boost::shared_ptr<I3Vector<double> > cascadeDomDistances(new I3Vector<double>);
			boost::shared_ptr<I3Vector<double> > cascadeDomCharges(new I3Vector<double>);
			boost::shared_ptr<I3Vector<double> > domDepths(new I3Vector<double>);
			boost::shared_ptr<I3Vector<double> > cascadeDomDepths(new I3Vector<double>);
			for (std::vector<DOMDATA>::const_iterator it = icdata_.begin();
				 it != icdata_.end(); ++it)
			{
				domDepths->push_back(it->z);
				// check if DOM is within maxImpact from cascade
				if (geometricDistance(*it, cascadePosition) > maxImpact_)
					continue;
				// calculate energy loss
				double cascadeEnergyLoss = MuonEnergyFunctions::eLossCascade(it->charge,
						geometricDistance(*it, cascadePosition), it->lambda, it->z, it->eff, levelDist_);
				cascadeDomDepths->push_back(it->z);

				// add to energy loss vector
				cascadeEnergyLosses->push_back(cascadeEnergyLoss);
				cascadeDomDistances->push_back(geometricDistance(*it, cascadePosition));
				cascadeDomCharges->push_back(it->charge);
			}

			I3MuonEnergyCascadeParamsPtr cascadeResults = getCascadeEnergyParams(cascadeEnergyLosses, 
					cascadePosition, *track);

			frame->Put(outputPars_, results);
			frame->Put(outputCascadePars_, cascadeResults);

			if (saveDomResults_) 
			{
				boost::shared_ptr<I3Vector<double> > domCharges(new I3Vector<double>);
				for (std::vector<DOMDATA>::const_iterator it = icdata_.begin();
				 it != icdata_.end(); ++it)
				{
					domCharges->push_back(it->charge);
				}
				saveDomResultsEnergyProfile(eprofile, frame, slant_, dEdX_, impact_, *track);
				frame->Put(outputDOMs_ + "CascadeEnergyLosses", cascadeEnergyLosses);
				frame->Put(outputDOMs_ + "CascadeDomDistances", cascadeDomDistances);
				frame->Put(outputDOMs_ + "CascadeDomCharges", cascadeDomCharges);
				frame->Put(outputDOMs_ + "Depth", domDepths);
				frame->Put(outputDOMs_ + "CascadeDepth", cascadeDomDepths);
				frame->Put(outputDOMs_ + "DomCharges", domCharges);
			}
		}//if eprofile->GetBinCenters().size()>=(nDoF_)
		else
			log_debug("Energy loss distribution has only %d bins, no fit perforemd, no prams created.", eprofile->GetNBins());

	}//if icdata_.size() > 2
	else
	{
		log_debug("Not doing anything, found only %d DOMs and a total energy loss of %.2g", int(icdata_.size()), tot_eloss);
	}

	PushFrame(frame, "OutBox");

}

I3MuonEnergyCascadeParamsPtr I3MuonEnergy::getCascadeEnergyParams(boost::shared_ptr<I3Vector<double> > energyLosses, 
		I3Position cascadePosition, I3Particle &track)
{
	log_debug("Creating CascadeEnergyParams.");
	I3MuonEnergyCascadeParamsPtr results(new I3MuonEnergyCascadeParams);

	int ndoms = 0;
	for (I3OMGeoMap::const_iterator om_iter = omGeo_->begin();
			om_iter != omGeo_->end(); ++om_iter)
	{
		I3Position om_pos = om_iter->second.position;
		if ((om_pos - cascadePosition).Magnitude() <= CASCADE_CONTAINMENT_DISTANCE)
			ndoms++;
	}

	// calculate mean
	double mean = 0, sum = 0, dev = 0, sdev = 0;

	for (std::vector<double>::size_type i = 0; i < energyLosses->size(); i++)
	{
		sum += (*energyLosses)[i];
	}

	mean = sum / energyLosses->size();

	// calculate standard deviation
	for (std::vector<double>::size_type i = 0; i < energyLosses->size(); i++)
	{
		dev = ((*energyLosses)[i] - mean)*((*energyLosses)[i] - mean);
		sdev += dev;
	}

	sdev /= (energyLosses->size() -1);

	results->cascade_energy = medianOfVector(energyLosses);
	results->cascade_energy_sigma = sqrt(sdev);
	results->nDOMsCascade = ndoms;
	results->cascade_slant_depth = MuonEnergyFunctions::getSlantDepth(track,
			cascadePosition, SURFACE_HEIGHT);
	results->cascade_position = cascadePosition;

	return results;
}

void I3MuonEnergy::saveDomResultsEnergyProfile(boost::shared_ptr<I3MuonEnergyProfile> eprofile, 
		I3FramePtr &frame, boost::shared_ptr<I3Vector<double> > slant, 
		boost::shared_ptr<I3Vector<double> > dEdX, boost::shared_ptr<I3Vector<double> > impact, 
		I3Particle &track)
{
	// Names of the frame objects
	std::string outputDOMsSlant_ = outputDOMs_ + "Slant";
	std::string outputDOMsImpact_ = outputDOMs_ + "Impact";
	std::string outputDOMsDepth_ = outputDOMs_ + "Depth";
	std::string outputDOMsdEdX_ = outputDOMs_ + "dEdX";
	std::string outputDOMsedEdX_ = outputDOMs_ + "dEdX_err";

	frame->Put(outputDOMsSlant_, slant);
	frame->Put(outputDOMsdEdX_, dEdX);

	boost::shared_ptr<I3Vector<double> > slantbinned(new I3Vector<double>);
	boost::shared_ptr<I3Vector<double> > dEdXbinned(new I3Vector<double>);
	boost::shared_ptr<I3Vector<double> > edEdXbinned(new I3Vector<double>);
	boost::shared_ptr<I3Vector<double> > depthbinned(new I3Vector<double>);
	for(std::vector<double>::size_type i=0; i < eprofile->GetBinCenters().size(); i++)
	{
		slantbinned->push_back(eprofile->GetBinCenter(i));
		dEdXbinned->push_back(eprofile->GetBinContent(i));
		edEdXbinned->push_back(eprofile->GetBinError(i));
		double vertical_height = MuonEnergyFunctions::getVerticalDepthZ(track, eprofile->GetBinCenter(i), surface_height_);
		depthbinned->push_back(vertical_height);
	}
	frame->Put(outputDOMsSlant_+"binned", slantbinned);
	frame->Put(outputDOMsdEdX_+"binned", dEdXbinned);
	frame->Put(outputDOMsedEdX_+"binned", edEdXbinned);
	frame->Put(outputDOMsDepth_+"binned", depthbinned);
	frame->Put(outputDOMsImpact_, impact);
}

/**
 * The I3TrueMuonEnergy module also determines the energy loss in bins along the track
 * of the primary particle. This method makes sure to use the same binning when using 
 * the Monte Carlo track as input track, e.g. when doing an comparison of true 
 * and reconstructed energy loss.
 */
boost::shared_ptr<I3MuonEnergyProfile> I3MuonEnergy::getMuonEnergyProfile(I3FramePtr &frame, double surface_height, double prim_zen, std::vector<DOMDATA> &icdata)
{
	if (useMonteCarloTrack_)
	{
		/* This makes sure that the binning is the same as used in the
		 * I3TrueMuonEnergy module, so that the results can be compared. */
		I3MMCTrackListConstPtr mmclist = frame->Get<I3MMCTrackListConstPtr>(mmcName_);

		std::vector<double> binEdges = MuonEnergyFunctions::getBinEdges(mmclist, surface_height, prim_zen, fitBinWidth_);

		boost::shared_ptr<I3MuonEnergyProfile> eprofiletmp(new I3MuonEnergyProfile(binEdges, fitBinWidth_));

		for (std::vector<DOMDATA>::const_iterator it = icdata.begin();
		     it != icdata.end(); ++it)
		{
			eprofiletmp->Fill(it->slant, it->dEdX);
		}
		return eprofiletmp;

	}
	else
	{
		boost::shared_ptr<I3MuonEnergyProfile> eprofiletmp(new I3MuonEnergyProfile(icdata, fitBinWidth_));
		return eprofiletmp;
	}

}

/**
 * The fit function is either an exponential function or Tom Feusels' function for 
 * muon energy loss (s. TomFFunction.h). In the case of the latter, an additional
 * parameter is needed, and parameters are fixed of specified so in the module
 * parameters.
 *
 * @param fitFunction
 *
 * @return Vector of FitParameterSpecs
 */
std::vector<I3FitParameterInitSpecs> I3MuonEnergy::getFitParameterSpecs(FitFunction &fitFunction)
{
	std::vector<I3FitParameterInitSpecs> parspecs;
	if (fitFunction == FitFunction::NOFCN)
	{ 
		// no actual fit, no fit parameters
	}
	else if (fitFunction == FitFunction::EXPOFCN)
	{
	    I3FitParameterInitSpecs norm("norm");
	    norm.initval_ = EXPO_NORM_INIT;
	    norm.stepsize_ = EXPO_NORM_STEPS;
	    norm.minval_ = EXPO_NORM_MINV;
	    norm.maxval_ = EXPO_NORM_MAXV;
	    parspecs.push_back(norm);

	    I3FitParameterInitSpecs exponent("exp");
	    exponent.initval_ = EXPO_EXP_INIT;
	    exponent.stepsize_ = EXPO_EXP_STEPS;
	    exponent.minval_ = EXPO_EXP_MINV; 
	    exponent.maxval_ = EXPO_EXP_MAXV;
	    parspecs.push_back(exponent);
	}
	else if (fitFunction == FitFunction::TOMFFCN)
	{
	    I3FitParameterInitSpecs norm("norm");
	    norm.initval_ = TOMF_NORM_INIT;
	    norm.stepsize_ = TOMF_NORM_STEPS;
	    norm.minval_ = TOMF_NORM_MINV;
	    norm.maxval_ = TOMF_NORM_MAXV;
	    
	    I3FitParameterInitSpecs b("b");
	    b.initval_ = TOMF_B_INIT;
	    b.stepsize_ = TOMF_B_STEPS;
	    b.minval_ = TOMF_B_MINV;
	    b.maxval_ = TOMF_B_MAXV;

	    I3FitParameterInitSpecs gamma("gamma");
	    gamma.initval_ = TOMF_GAMMA_INIT;
	    gamma.stepsize_ = TOMF_GAMMA_STEPS;
	    gamma.minval_ = TOMF_GAMMA_MINV;
	    gamma.maxval_ = TOMF_GAMMA_MAXV;

	    int numFixedParameters = 0;
	    
	    // The minimizer interpretes minval==maxval as fixed parameter.
	    if (fixB_) 
		{
		    b.minval_ = b.initval_;
		    b.maxval_ = b.initval_;
		    numFixedParameters++;
		}
	    if (fixGamma_) 
	        {
		    gamma.minval_ = gamma.initval_;
		    gamma.maxval_ = gamma.initval_;
		    numFixedParameters++;
		}

	    parspecs.push_back(norm);
	    parspecs.push_back(b);
	    parspecs.push_back(gamma);
	    
	    log_info("The no. of fixed parameters for the TomFcn is %i.", numFixedParameters);
	}

	return parspecs;
}

/**
 * In addition to the fit parameters, the chi^2 and the reduced log likelihood 
 * are determined, as well as peak energy and sigma, median and mean of the 
 * energy loss distribution.
 *
 * @param minuitResult	The result from the fit
 * @param eprofile		The histogram with the energy losses
 * @param fitFunction	The chosen fit function
 *
 * @return I3MuonEnergyParamsPtr containing the results and the bundle supression 
 * parameters (s. https://wiki.icecube.wisc.edu/index.php/Muon_Bundle_Suppression#Stochasticity).
 */
I3MuonEnergyParamsPtr I3MuonEnergy::getMuonEnergyParams(boost::shared_ptr<I3MuonEnergyProfile> eprofile, 
		std::vector<double> slantBins_, std::vector<double> dEdXPerBin_)
{
	I3MuonEnergyParamsPtr results(new I3MuonEnergyParams);

	results->nDOMs = eprofile->GetNEnergyLosses();
	results->rllh = 0;
	results->chi2 = 0;

	/*
	If `fitFunction_` is set to NOFCN, no fit will be done.
	This allows running the module without Minuit2.

	Configure() ensures that fitFunction_ is set to NOFCN
	when Minuit2 is not available.
	*/
#ifdef USE_MINUIT2
	if (fitFunction_ != FitFunction::NOFCN)
	{
	    boost::shared_ptr<I3GulliverBase> fitfcn;
	    I3GulliverMinuit2 minuit2("d4rfit",
				      minuitTolerance_, minuitMaxIterations_,
				      minuitPrintLevel_, minuitStrategy_, 
				      minuitAlgorithm_,
				      false, false, false, false);
		    
	    if (fitFunction_ == FitFunction::EXPOFCN)
	        {
		    fitfcn.reset(new ExpoFcn(dEdXPerBin_, slantBins_));
		}
	    else if (fitFunction_ == FitFunction::TOMFFCN)
	        {
		    fitfcn.reset(new TomFFcn(dEdXPerBin_, slantBins_));
		}

	    const std::vector<I3FitParameterInitSpecs> parspecsConst = getFitParameterSpecs(
				fitFunction_);
	    
	    I3MinimizerResult minuitResult = minuit2.Minimize(*fitfcn, parspecsConst);
	    log_debug("Creating MuonEnergyParams.");
	    
	    results->N = minuitResult.par_[0];
	    results->b = minuitResult.par_[1];
	    results->N_err = minuitResult.err_[0];
	    results->b_err = minuitResult.err_[1];
	    boost::function<double (double)> f(ExpoFunction(results->N,
							    results->b));
	    	    
	    if (fitFunction_ == FitFunction::TOMFFCN)
	        {
		    results->gamma = minuitResult.par_[2];
		    results->gamma_err = minuitResult.err_[2];
		    f = TomFFunction(results->N, results->b, results->gamma);
		    //f.swap(TomFFunction(results->N,
		    //results->b,
		    //results->gamma));
		}

	    if (!minuitResult.converged_)
	        results->status = I3Particle::FailedToConverge;
	    else if (std::isnan(results->N))
	        results->status = I3Particle::GeneralFailure;
	    else
	        results->status = I3Particle::OK;
	    
	    log_debug("Calculating Chi^2 and rllh for the fit.");
	    if (results->nDOMs > 1 && !std::isnan(results->N)){
	        for (int i = 0; i < eprofile->GetNBins(); i++){
		    try {
		        double dEdX = eprofile->GetBinContent(i);
			double edEdX = eprofile->GetBinError(i);
			double slant = eprofile->GetBinCenter(i);
			results->rllh 
			    += pow(f(slant), 2)
			    / (dEdX+1);
			if (edEdX > 0.){
			  results->chi2 
			      += pow(f(slant) / edEdX, 2);
			}
		    }
		    catch (const std::out_of_range& e){
		      log_warn("Couldn't compute rllh or chi^2, exception occured: %s", 
			       e.what());
		    }
		}
	    }
	    results->chi2ndof = results->chi2 / ((double) eprofile->GetNBins()-nDoF_-1.);
	}
#else
	/*
	If Minuit2 is not available, no fit is performed.
	The corresponding fields in the `results` are initialized to
	NAN in the constructor of I3MuonEnergyParams and stay like that.
	*/
#endif
      
	//Compute peak energy and error
	results->peak_energy = eprofile->GetBinContent(eprofile->GetMaxBin());
	results->peak_sigma = eprofile->GetBinError(eprofile->GetMaxBin());

	//Compute median and mean energy loss
	results->median = eprofile->GetMedianEnergyLoss();
	results->mean   = eprofile->GetMeanEnergyLoss();

	results->bin_width = eprofile->GetBinWidth();

	return results;
}

I3ParticlePtr I3MuonEnergy::getSeedTrack(I3FramePtr &frame)
{
	if (useMonteCarloTrack_)
	{
		return MuonEnergyFunctions::getWeightedPrimary(frame, mcTreeName_);
	}
	else
	{
		I3ParticlePtr fittrack(new I3Particle(*(frame->Get<I3ParticleConstPtr>(trackName_))));
		return fittrack;
	}
}

std::vector<std::vector<double> > I3MuonEnergy::readLambdaFile(std::string iceFileName)
{
	std::vector<std::vector<double> > lambdaFile;
	// Read attenuation length table into vector of rows of zmax, zmin, lambda
	log_info("Reading attenuation length from %s", iceFileName_.c_str());
	std::ifstream fin_;

	fin_.open(iceFileName_.c_str(), std::ifstream::in);
	if (!fin_.is_open())
	{
		log_fatal("Attenuation length file %s cannot be opened!", iceFileName_.c_str());
	}

	std::string line;
	while (std::getline(fin_, line))
	{
		double zmax, zmin, l;
		std::istringstream line_stream(line);
		if (line_stream >> zmax >> zmin >> l)
		{
			std::vector<double> row;
			row.push_back(zmax);
			row.push_back(zmin);
			row.push_back(l);

			lambdaFile.push_back(row);
		}
	}

	fin_.close();
	fin_.clear();

	return lambdaFile;
}
