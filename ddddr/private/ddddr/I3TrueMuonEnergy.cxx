#include "ddddr/I3TrueMuonEnergy.h"
#include "ddddr/MuonEnergyFunctions.h"

#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/geometry/I3TankGeo.h"
#include "dataclasses/geometry/I3OMGeo.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "phys-services/I3Calculator.h"
#include "icetray/OMKey.h"
#include "dataclasses/I3Map.h"
#include "MuonGunTrack.h"
#include <boost/foreach.hpp>

static const double SURFACE_HEIGHT = 1948.07; // https://wiki.icecube.wisc.edu/index.php/Coordinate_system

I3_MODULE(I3TrueMuonEnergy);

I3TrueMuonEnergy::I3TrueMuonEnergy(const I3Context & ctx)
	: I3ConditionalModule(ctx)
{
	mcTreeName_ = "I3MCTree";
	AddParameter("I3MCTree", "name of the I3MCTree", mcTreeName_);
	mmcName_ = "MMCTrackList";
	AddParameter("MMCTrackList", "name of the mmc track list", mmcName_);
	slantBinWidth_ = 50.;
	AddParameter("BinWidth", "the bin size of slant depth", slantBinWidth_);
	slantBinNo_ = 200;
	AddParameter("SlantBinNo", "the bin number of slant depth", slantBinNo_);
	energyLoss_ = true;
	AddParameter("EnergyLoss", "true for fit energy loss; false for energy", energyLoss_);
	AddOutBox("OutBox");
	maxRadius_ = 800;
	AddParameter("MaxRadius", "Max. radius of the cylindric simulation volume used by MMC/Proposal.", maxRadius_);
	framePrefix_ = "I3TrueMuonEnergy";
	AddParameter("Prefix", "prefix for energy (loss) information stored in the frame", framePrefix_);
	saveEnergyLosses_ = false;
	AddParameter("SaveEnergyLosses", "If true, save the binned energy losses along the muon bundle to the frame.",
			saveEnergyLosses_);
}

I3TrueMuonEnergy::~I3TrueMuonEnergy() {}

void I3TrueMuonEnergy::Configure() 
{
	GetParameter("I3MCTree", mcTreeName_);
	GetParameter("MMCTrackList", mmcName_);
	GetParameter("BinWidth", slantBinWidth_);
	GetParameter("SlantBinNo", slantBinNo_);
	GetParameter("EnergyLoss", energyLoss_);
	GetParameter("MaxRadius", maxRadius_);
	GetParameter("Prefix", framePrefix_);
	GetParameter("SaveEnergyLosses", saveEnergyLosses_);

	//minimization parameters, for now just fixed here
	tolerance_ = 0.0001;
	maxIterations_ = 10000;
}

void I3TrueMuonEnergy::Physics(I3FramePtr frame)
{
	if (!(frame->Has(mcTreeName_) && frame->Has(mmcName_)))
	{
		log_error("Not enough true info. in the frame!");
		PushFrame(frame, "OutBox");
		return;
	}

	using namespace I3MCTreeUtils;
	I3MCTreeConstPtr mctree = frame->Get<I3MCTreeConstPtr>(mcTreeName_);
	I3MMCTrackListConstPtr mmclist = frame->Get<I3MMCTrackListConstPtr>(mmcName_);
	I3ParticleConstPtr prim_par = MuonEnergyFunctions::getWeightedPrimary(frame, mcTreeName_);

	// get the tracks from the frame
	using namespace I3MuonGun;
	std::list<Track> muons = Track::Harvest(*mctree, *mmclist);
    double primary_zenith = prim_par->GetZenith();

	// divide the muon bundle in bins along its axis
	std::vector<double> bin_edges = MuonEnergyFunctions::getBinEdges(mmclist, 
			SURFACE_HEIGHT, primary_zenith, slantBinWidth_);
	std::vector<BundleBin> muon_bundle;
	std::vector<double> bin_edge_energies;

	// get the total muon bundle energy at each bin edge
	BOOST_FOREACH(double slant, bin_edges)
	{
		double energy = 0;
		BOOST_FOREACH(const Track &track, muons)
		{
			energy += track.GetEnergy(slant);
		}
		bin_edge_energies.push_back(energy);
	}

	// save the differential energy loss for each bin as well as the 
	// energy left at the end of the bin
	for (unsigned int i = 1; i < bin_edge_energies.size(); i++)
	{
		BundleBin * bin = new BundleBin;
		bin->slant = (bin_edges[i] + bin_edges[i-1]) / 2.;
		double bindepth = (bin_edges[i]*cos(primary_zenith) + 
			bin_edges[i-1]*cos(primary_zenith)) / 2.;
		bin->depth = SURFACE_HEIGHT - bindepth;
		bin->energy = bin_edge_energies[i];
		bin->energyloss = (bin_edge_energies[i-1] - bin_edge_energies[i])/slantBinWidth_;
		muon_bundle.push_back(*bin);
		delete bin;
	}

	if (saveEnergyLosses_)
	{
		// The energy of the muon bundle is saved as vectors in their order
		// along the track, together with the slant depth and depth of the bins.
		boost::shared_ptr<I3Vector<double> > slant_detail(new I3Vector<double>);
		boost::shared_ptr<I3Vector<double> > depth_detail(new I3Vector<double>);
		boost::shared_ptr<I3Vector<double> > dEdX_detail(new I3Vector<double>);
		boost::shared_ptr<I3Vector<double> > dEdX_loss(new I3Vector<double>);
		for (std::vector<BundleBin>::const_iterator it = muon_bundle.begin();
				it != muon_bundle.end(); ++it)
		{
			slant_detail->push_back(it->slant);
			depth_detail->push_back(it->depth);
			dEdX_detail->push_back(it->energy);
			dEdX_loss->push_back(it->energyloss);
		}
		frame->Put(framePrefix_ + "Slantbinned", slant_detail);
		frame->Put(framePrefix_ + "Depthbinned", depth_detail);
		frame->Put(framePrefix_ + "Energy", dEdX_detail);
		frame->Put(framePrefix_ + "dEdXbinned", dEdX_loss);
	}

	I3MuonEnergyParamsPtr results = getBundleEnergyDistribution(muon_bundle);
	I3MuonEnergyCascadeParamsPtr cascade_results = getCascadeEnergyParams(prim_par, muon_bundle);

	frame->Put(framePrefix_ + "Params", results);
	frame->Put(framePrefix_ + "CascadeParams", cascade_results);
	PushFrame(frame, "OutBox");
}

I3MuonEnergyCascadeParamsPtr I3TrueMuonEnergy::getCascadeEnergyParams(I3ParticleConstPtr &track, 
		std::vector<BundleBin> muon_bundle)
{
	double max_eloss = 0;
	double max_eloss_slant = 0;
	BOOST_FOREACH(BundleBin bin, muon_bundle)
	{
		if (bin.energyloss > max_eloss)
		{
			max_eloss = bin.energyloss;
			max_eloss_slant = bin.slant;
		}
	}

	I3MuonEnergyCascadeParamsPtr results(new I3MuonEnergyCascadeParams);
	results->cascade_energy = max_eloss * slantBinWidth_;
	results->cascade_position = track->GetPos() + max_eloss_slant * track->GetDir();
	results->cascade_slant_depth = max_eloss_slant;

	return results;
}

I3MuonEnergyParamsPtr I3TrueMuonEnergy::getBundleEnergyDistribution(std::vector<BundleBin> &bundle)
{
	I3MuonEnergyParamsPtr results(new I3MuonEnergyParams);

	std::vector<double> slant;
	std::vector<double> measure;

	for (std::vector<BundleBin>::const_iterator it = bundle.begin();
			it != bundle.end(); ++it)
	{
		slant.push_back(it->slant);
		if (energyLoss_)
			measure.push_back(it->energyloss);
		else
			measure.push_back(it->energy);
	}

	I3GSLSimplex minimizer("trueEnergyFit",
			       tolerance_, tolerance_, 
			       maxIterations_, false);

	boost::shared_ptr<ExpoFcn> fitfcn(new ExpoFcn(measure, slant));

	std::vector<I3FitParameterInitSpecs> parspecs;
	I3FitParameterInitSpecs norm("norm");
	norm.initval_ = 10;
	norm.stepsize_ = 0.1;
	norm.minval_ = 0;
	norm.maxval_ = 15;
	parspecs.push_back(norm);
	
	I3FitParameterInitSpecs exponent("exp");
	exponent.initval_ = 1e-3;
	exponent.stepsize_ = 1e-1;
	exponent.minval_ = -1e-1;
	exponent.maxval_ = 1e-1;
	parspecs.push_back(exponent);

	const std::vector<I3FitParameterInitSpecs> parspecsConst(parspecs.begin(),
			parspecs.begin()+2);

	I3MinimizerResult result = minimizer.Minimize(*fitfcn, parspecsConst);

	results->N = result.par_[0];
	results->b = result.par_[1];
	results->N_err = result.err_[0];
	results->b_err = result.err_[1];

	// calculate mean
	double mean = 0, sum = 0, dev=0, sdev = 0;

	for (std::vector<double>::size_type i = 0; i < measure.size(); i++)
	{
		sum += measure[i];
	}
	mean = sum / measure.size();

	// calculate standard deviation
	for (std::vector<double>::size_type i = 0; i < measure.size(); i++)
	{
		dev = (measure[i] - mean)*(measure[i] - mean);
		sdev += dev;
	}

	sdev /= (measure.size() -1);

	std::vector<double>::iterator maxeloss = 
		std::max_element(measure.begin(), measure.end());
	results->mean = mean;
	results->peak_energy = *maxeloss;
	results->median = getMedianOfVector(measure);


	return results;
}

double I3TrueMuonEnergy::getMedianOfVector(std::vector<double> vec)
{
	std::vector<double> non_zero_bins;
	for (std::vector<double>::size_type i = 0; i != vec.size(); i++)
	{
		if (vec[i] > 0.00000001) //sloppy compensation of floating point errors
		{
			non_zero_bins.push_back(vec[i]);
		}
	}

	if (non_zero_bins.size() <= 1)
	{
		return 0;
	}

	if (non_zero_bins.size() % 2 != 0)
	{
		size_t n = non_zero_bins.size() / 2;
		std::nth_element(non_zero_bins.begin(), non_zero_bins.begin()+n, non_zero_bins.end());
		return non_zero_bins[n];
	}
	else
	{
		size_t n = non_zero_bins.size() / 2;
		std::nth_element(non_zero_bins.begin(), non_zero_bins.begin()+n, non_zero_bins.end());
		std::nth_element(non_zero_bins.begin(), non_zero_bins.begin()+(n-1), non_zero_bins.end());
		return (non_zero_bins[n]+non_zero_bins[n-1])/2;
	}
}
