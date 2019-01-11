/**
 * Implementation of I3Truncated_Energy
 * 
 * (c) 2011
 * the IceCube Collaboration
 * 
 * @file I3TruncatedEnergy.cxx
 * @date $Date: 2012-01-15 07:53:03 -0700 (15 Jan 2012) $
 * @author smiarecki
 * @author mwolf
 *
 */

/* 
 * CHANGELOG
 * =========
 * Version 4.3
 *     (updated equations for icerec 04.00.01), with calculated energy resolution for each event, 15 Nov 2011
 * Version 5.0
 *     Code rewrite for a none-fixed detector geometry by Martin Wolf
 * Version 5.1
 *     Added UseAllDOMs mode and BadDomList awareness, see RELEASE_NOTES
 *
 * SEE THE DOCUMENTATION UNDER /truncated_energy/resources/docs/index.dox
 *
 * Please send feeback !  miarecki@berkeley.edu
 */

#include "truncated_energy/I3TruncatedEnergy.h"

#include <icetray/I3TrayHeaders.h>
#include <icetray/I3Units.h>

#include <phys-services/I3Calculator.h>

#include <dataclasses/I3Double.h>
#include <dataclasses/I3Direction.h>
#include <dataclasses/I3Constants.h>

#include <algorithm>  // for SORT command
#include <cmath>


// length along the track of one bin in the BIN method, 120 m chosen for best precision:
#define BINMETHOD_BIN_LENGTH 120.

I3_MODULE(I3TruncatedEnergy);

namespace truncated_energy {

double GetRDE(OMKey omkey, I3CalibrationConstPtr cal)
{
  std::map<OMKey, I3DOMCalibration>::const_iterator calIter = cal->domCal.find(omkey);
  if(calIter == cal->domCal.end() )
    return 1.0;
  
  const double rde = calIter->second.GetRelativeDomEff();
  
  if(std::isnan(rde) || rde < 0.) {
    log_fatal("Relative DOM efficiency for OMKey %s is %f. "
              "Check your GCD file!",
              omkey.str().c_str(), rde);
  }    
  return rde;    
}

} // namespace truncated_energy

//______________________________________________________________________________
/** Constructor
 */
I3TruncatedEnergy::I3TruncatedEnergy(const I3Context &ctx)
  : I3ConditionalModule(ctx),
    nMissingParticle_(0),
    nMissingPulses_(0),
    nCallsSuccess_(0),
    nCallsFail_(0)
{
    AddOutBox("OutBox");

    RecoPulsesName_ = "RecoPulseSeries";
    AddParameter("RecoPulsesName",
        "Name of the MC Hits you want to extract. "
        "Default is \"RecoPulseSeries\"",
        RecoPulsesName_
    );

    RecoParticleName_ = "";
    AddParameter("RecoParticleName",
        "Name of the reconstructed particle you want to use. "
        "Default is \"\".",
        RecoParticleName_
    );

    ResultParticleName_ = "TruncatedEnergy_"+RecoParticleName_;
    AddParameter("ResultParticleName",
        "Name of the resulting particle you want to write to the frame. "
        "Default is \"TruncatedEnergy_\"+RecoParticleName.",
        ResultParticleName_
    );

    I3PhotonicsServiceName_ = "I3PhotonicsService";
    AddParameter("I3PhotonicsServiceName",
        "Name of the I3PhotonicsService instance you want to use. "
        "Default is \"I3PhotonicsService\".",
        I3PhotonicsServiceName_
    );

    SetMinCylinderBIN_ = 10.0;
    AddParameter("SetMinCylinderBIN",
        "Set minimum cylinder distance for BINs method. "
        "Default is 10.0m for best precision, use 0.0 to include entire detector.",
        SetMinCylinderBIN_
    );

    SetMaxCylinderBIN_ = 80.0;
    AddParameter("SetMaxCylinderBIN",
        "Set maximum cylinder distance for BINs method. "
        "Default is 80.0m for best precision, use 1000.0 to include entire detector.",
        SetMaxCylinderBIN_
    );

    SetMinCylinderDOM_ = 0.0;
    AddParameter("SetMinCylinderDOM",
        "Set minimum cylinder distance for DOMs method. "
        "Default is 0.0m for best precision, use 0.0 to include entire detector.",
        SetMinCylinderDOM_
    );

    SetMaxCylinderDOM_ = 60.0;
    AddParameter("SetMaxCylinderDOM",
        "Set maximum cylinder distance for DOMs method. "
        "Default is 60.0m for best precision, use 1000.0 to include entire detector.",
        SetMaxCylinderDOM_
    );

    SetMinDOMS_ = 8;
    AddParameter("SetMinDOMS",
        "Set minimum number of DOMs for the event to qualify. "
        "Default is 8.",
        SetMinDOMS_
    );

    UseRDE_ = true;
    AddParameter("UseRDE",
        "Use relative DOM efficiency from GCD file (takes into account QE of HQE DOMs) or not? "
        "Default is True.",
        UseRDE_
    );

    BadDomListName_ = "BadDomsList";
    AddParameter("BadDomListName",
        "Name of the BadDomList frame object, only needed for method \"UseAllDOMs\". "
        "Default is \"BadDomList\", set to \"\" to use all DOMs.",
        BadDomListName_
    );
}

//______________________________________________________________________________
I3TruncatedEnergy::~I3TruncatedEnergy(){
}

//______________________________________________________________________________
void
I3TruncatedEnergy::Configure ()
{
    GetParameter("RecoPulsesName",         RecoPulsesName_);
    GetParameter("RecoParticleName",       RecoParticleName_);
    GetParameter("ResultParticleName",     ResultParticleName_);
    GetParameter("I3PhotonicsServiceName", I3PhotonicsServiceName_);
    GetParameter("SetMinCylinderBIN",      SetMinCylinderBIN_);
    GetParameter("SetMaxCylinderBIN",      SetMaxCylinderBIN_);
    GetParameter("SetMinCylinderDOM",      SetMinCylinderDOM_);
    GetParameter("SetMaxCylinderDOM",      SetMaxCylinderDOM_);
    GetParameter("SetMinDOMS",             SetMinDOMS_);
    GetParameter("UseRDE",                 UseRDE_);
    GetParameter("BadDomListName",         BadDomListName_);

    I3PhotonicsService_ = context_.Get<I3PhotonicsServicePtr>(I3PhotonicsServiceName_);
    if(! I3PhotonicsService_) {
        log_fatal("(%s) missing photonics service \"%s\"!",
            GetName().c_str(), I3PhotonicsServiceName_.c_str());
    }

    if(ResultParticleName_.empty()) {
        log_fatal("(%s) 'ResultParticleName' cannot be an empty string!",
            GetName().c_str());
    }
}

//______________________________________________________________________________
/** This method is called by icetray whenever a new calibration frame is
 *  issued.
 */
void
I3TruncatedEnergy::Calibration(I3FramePtr frame)
{
  // load the new calibration
  calib_ = frame->Get<I3CalibrationConstPtr>();
  if(UseRDE_ && !calib_)
      log_fatal("(%s) No Calibration info but this is essential for Relative DOM Efficiencies!",
                GetName().c_str());
  PushFrame(frame, "OutBox");
}

//______________________________________________________________________________
/** This method is called by icetray whenever a new geometry frame is
 *  issued.
 */
void
I3TruncatedEnergy::Geometry(I3FramePtr frame)
{
    geo_ = frame->Get<I3GeometryConstPtr>();
    if(! geo_) {
        log_fatal("(%s) Could not load I3Geometry object from the Geometry frame!",
            GetName().c_str());
    }
    PushFrame(frame, "OutBox");
}

//______________________________________________________________________________
/** This method is called in Physics() below, to count the expected PE from
 *  photonics for each DOM.
 */

double
I3TruncatedEnergy::CalculateExpectedDomNPEsFromTrack(
    I3ParticleConstPtr          reco_particle,
    const OMKey                 &omkey,
    const I3OMGeo               &omgeo,
    const double                ref_energy,
    const I3VectorOMKeyConstPtr bad_dom_list
)
{
    if(std::find(bad_dom_list->begin(), bad_dom_list->end(), omkey) != bad_dom_list->end())
    {
        log_debug("(%s) Found DOM %s in BadDomList, returning 0. expected NPE.",
                    GetName().c_str(), omkey.str().c_str());
        return 0.;
    }

    const int    photonics_flag   = 1;    // preset for Lightsaber tables for muons
    const double photonics_length = -1.0; // preset for Lightsaber tables for muons

    PhotonicsSource source(
        reco_particle->GetX() * truncated_energy::i3unit_inv_m,
        reco_particle->GetY() * truncated_energy::i3unit_inv_m,
        reco_particle->GetZ() * truncated_energy::i3unit_inv_m,
        reco_particle->GetZenith()  * truncated_energy::i3unit_inv_deg,
        reco_particle->GetAzimuth() * truncated_energy::i3unit_inv_deg,
	1.,
        photonics_length,
        ref_energy,
        photonics_flag
    );

    // select OM coordinates in photonics
    const double omx = omgeo.position.GetX() * truncated_energy::i3unit_inv_m;
    const double omy = omgeo.position.GetY() * truncated_energy::i3unit_inv_m;
    const double omz = omgeo.position.GetZ() * truncated_energy::i3unit_inv_m;
    I3PhotonicsService_->SelectModuleCoordinates(omx, omy, omz);

    // get photorec information about the selected OM
    double probability = 1;
    double expected_npe = 0;
    double dummytime = 20.*I3Units::ns;
    bool goodinfo = I3PhotonicsService_->GetPhotorecInfo(
        expected_npe,
        probability,
        dummytime,
        source
    );

    if(goodinfo && std::isnormal(expected_npe) && (expected_npe > 0))
    {
        ++nCallsSuccess_;

        // Correct for electronics, DeepCore, ...
        if(UseRDE_ && omgeo.omtype == I3OMGeo::IceCube) {
            expected_npe *= truncated_energy::GetRDE(omkey, calib_);
        }

        return expected_npe;
    }

    ++nCallsFail_;
    return 0.;
}

//______________________________________________________________________________
/** This method is used to make sure the event is a good event containing all
 *  required frame objects
 */
bool
I3TruncatedEnergy::CheckEventPulsesAndFitStatus(
    I3RecoPulseSeriesMapConstPtr reco_pulses_map,
    I3ParticleConstPtr           reco_particle)
{
    if(reco_pulses_map && !(reco_pulses_map->empty()) &&
       reco_particle && (reco_particle->GetFitStatus() == I3Particle::OK)
      )
    {
        if(reco_particle->IsTrack())
        {
            if(! std::isnan(reco_particle->GetX()+
                     reco_particle->GetY()+
                     reco_particle->GetZ()+
                     reco_particle->GetZenith()+
                     reco_particle->GetAzimuth()) 
              )
            {
                // event is good to process
                return true;
            }
            else
            {
                log_debug("(%s) \"%s\" has NAN values! Skipping energy estimation for this event!",
                    GetName().c_str(), RecoParticleName_.c_str());
                ++nMissingParticle_;
                return false;
            }
        }
        else
        {
            log_debug("(%s) \"%s\" is not a track, skipping energy estimation for this event!",
                GetName().c_str(), RecoParticleName_.c_str());
            ++nMissingParticle_;
            return false;
        }
    }

    // event is not healthy, try to figure out and to report why not

    if(! reco_pulses_map)
    {
        log_debug("(%s) No reco pulses with name \"%s\", skipping energy estimation for this event!",
            GetName().c_str(), RecoPulsesName_.c_str());
        ++nMissingPulses_;
    }
    else if(reco_pulses_map->empty())
    {
        log_debug("(%s) Pulse map \"%s\" found but empty, skipping energy estimation for this event!",
            GetName().c_str(), RecoPulsesName_.c_str());
        ++nMissingPulses_;
    }

    if(! reco_particle)
    {
        log_debug("(%s) \"%s\" absent in frame, skipping energy estimation for this event!",
            GetName().c_str(), RecoParticleName_.c_str());
        ++nMissingParticle_; 
    }
    else if(reco_particle->GetFitStatus() != I3Particle::OK)
    {
        log_debug("(%s) \"%s\" status not OK, skipping energy estimation for this event!",
            GetName().c_str(), RecoParticleName_.c_str());
        ++nMissingParticle_;
    }

    return false;
}

//______________________________________________________________________________
void
I3TruncatedEnergy::Physics(I3FramePtr frame)
{
    // check if geometry has been loaded and get the OM geometry from the I3Geometry object
    if(! geo_) {
        log_fatal("(%s) No I3Geometry object has been loaded. Is GCD file missing?",
            GetName().c_str());
    }
    const I3OMGeoMap &omgeomap = geo_->omgeo;

    // check if the calibration has been loaded if it is needed in case the
    // relative DOM efficiency should be used to treat each DOM equally
    if(UseRDE_ && !calib_)
      log_fatal("(%s) No Calibration info but this is essential for Relative DOM Efficiencies!",
                GetName().c_str()); 

    // get the reconstructed particle and pulses map from the frame
    I3ParticleConstPtr reco_particle =
        frame->Get<I3ParticleConstPtr>(RecoParticleName_);
    I3RecoPulseSeriesMapConstPtr reco_pulses_map =
        frame->Get<I3RecoPulseSeriesMapConstPtr>(RecoPulsesName_);

    // check event's health, if bad, push the frame with no more calculations
    if(! CheckEventPulsesAndFitStatus(reco_pulses_map, reco_particle) )
    {
        PushFrame(frame, "OutBox");
        return;
    }

    log_debug("(%s) Geo map has %zu DOMs and reco pulses map has %zu DOMs with pulses.",
        GetName().c_str(), omgeomap.size(), reco_pulses_map->size());

    // check if the BadDomList can be found in case we need it (only in UseAllDOMs mode)
    I3VectorOMKeyConstPtr badDomList = frame->Get<I3VectorOMKeyConstPtr>(BadDomListName_);
    
    if(BadDomListName_.empty())
    {
        badDomList = I3VectorOMKeyConstPtr(new I3VectorOMKey());
    }
    if(!badDomList)
    {
        log_warn("(%s) BadDomList %s can not be found in frame. Using all DOMs...",
            GetName().c_str(), BadDomListName_.c_str());
        badDomList = I3VectorOMKeyConstPtr(new I3VectorOMKey());
    }
    log_debug("(%s) BadDomList loaded with %zu DOMs.",
        GetName().c_str(), badDomList->size());
    

    // ---------------------------------------------------------------
    // SECTION FOR BINNING THE ENERGIES OF PHOTONICS AND ACTUAL NPE
    // ---------------------------------------------------------------

    dom_prop_map_t DomPropMap;

    // define the minimal maximum number of bins for the BIN method
    // NOTE: This number is only used for optimization, the actual number of
    //       bins will be increased automatically for a possible bigger detector!
    const unsigned BINMethod_min_N_bins = 15;

    // feed the reference energy of 1 GeV/m to Photonics
    // We are trying to find the scale factor for dEdx based upon this nominal
    // value.
    const double ref_energy = 1.0 * I3Units::GeV;

    double tot_det_npes = 0.;
    double tot_exp_npes = 0.;
    double minDomDistAlongTrack =  HUGE_VAL;
    double maxDomDistAlongTrack = -HUGE_VAL;

    // iterate through all DOMs of the configured geometry (given by the
    // geometry frame) and get the expected and measured number of photo
    // electrons.
    // Save everything inside the DomPropMap structure.
    I3OMGeoMap::const_iterator omgeomap_citer;
    for(omgeomap_citer = omgeomap.begin();
        omgeomap_citer != omgeomap.end();
        ++omgeomap_citer
       )
    {
        const OMKey   &omkey = omgeomap_citer->first;
        const I3OMGeo &omgeo = omgeomap_citer->second;

        // exclude IceTop DOMs
        if(omgeo.omtype != I3OMGeo::IceCube)
            continue;

        // get expected NPEs of the track on this DOM by asking photonics
        const double exp_npes = CalculateExpectedDomNPEsFromTrack(reco_particle, omkey, omgeo, ref_energy, badDomList);
        if(exp_npes == 0.)
        {
            // do not consider bad DOMs 
            continue;
        }
        // NOTE: expected charge > 0. means that the DOM is not bad and
        //       photonics didn't fail
        
        // count the total expected PEs for the photorec method
        tot_exp_npes += exp_npes;

        // check if we got pulses on this DOM, if so, count all the measured PEs
        // and store it inside dom_prop.npes

        double det_npes = 0.;
        I3RecoPulseSeriesMap::const_iterator reco_pulses_map_citer = reco_pulses_map->find(omkey);
        if(reco_pulses_map_citer != reco_pulses_map->end())
        {
            // this DOM has pulses in this event; sum them up!
            const I3RecoPulseSeries &reco_pulses = reco_pulses_map_citer->second;
            for(I3RecoPulseSeries::const_iterator reco_pulses_citer = reco_pulses.begin();
                reco_pulses_citer != reco_pulses.end();
                ++reco_pulses_citer
               )
            {
                double reco_pulse_npes = reco_pulses_citer->GetCharge();
                if(reco_pulse_npes < 0) {
                    log_warn("(%s) Got a sick pulse in DOM %s! Ignore the pulse.",
                        GetName().c_str(), omkey.str().c_str());
                    continue;
                }
                det_npes += reco_pulse_npes;
            }
            tot_det_npes += det_npes;
        }

        // create DOM property structure
        dom_prop_t dom_prop;

        // set measured und expected number of photo electrons
        dom_prop.det_npes = det_npes;
        dom_prop.exp_npes = exp_npes;
        // calculate DOM distance perpendicular to the track
        dom_prop.dist_perpendicular_to_track = I3Calculator::ClosestApproachDistance(*reco_particle, omgeo.position);
        // calculate DOM distance along the track
        dom_prop.dist_along_track = I3Calculator::DistanceAlongTrack(*reco_particle, omgeo.position);
        // insert HIT DOM into DomPropMap
        std::pair<dom_prop_map_t::iterator, bool> ret;
        ret = DomPropMap.insert(std::pair<const OMKey, dom_prop_t>(omkey, dom_prop));
        if(ret.second == false)
        {
            log_fatal("(%s) Unable to insert DOM %s into DomPropMap!",
                GetName().c_str(), omkey.str().c_str());
        }

        // calculate minimum and maximum DOM distance along the track of hit
        // DOMs for determining the binning
        if((det_npes > 0.)                                                    &&
           (dom_prop.dist_perpendicular_to_track >= SetMinCylinderBIN_) &&
           (dom_prop.dist_perpendicular_to_track <= SetMaxCylinderBIN_) 
          )
        {
            if(dom_prop.dist_along_track < minDomDistAlongTrack)
            {
                minDomDistAlongTrack = dom_prop.dist_along_track;
            }
            if(dom_prop.dist_along_track > maxDomDistAlongTrack)
            {
                maxDomDistAlongTrack = dom_prop.dist_along_track;
            }
        }
    } // END geometry FOR loop
    if(minDomDistAlongTrack == HUGE_VAL)
    {
        log_debug("(%s) Not a single DOM with charge in cylinder, probably reco-track went horribly wrong!",
            GetName().c_str() );
        //std::cout << reco_particle->GetZenith() << " " << reco_particle->GetAzimuth() << "\n" << "\n";
    }

    // calculate original photorec ratio
    double OrigPhotorec = 0.;
    if((tot_det_npes > 0.) && (tot_exp_npes > 0.)) {
        OrigPhotorec = tot_det_npes / tot_exp_npes;
    }

    // fill the histograms for the BIN method and
    // fill the list of DOM NPEs / ExpNPEs ratios for the DOM method
    std::vector<double> BINMethod_MesNPEsBins(BINMethod_min_N_bins, 0.); // initialize vector with zeros
    std::vector<double> BINMethod_ExpNPEsBins(BINMethod_min_N_bins, 0.); // initialize vector with zeros
    std::vector<double> BINMethod_ExpNPEsBins_AllDOMs(BINMethod_min_N_bins, 0.); // ditto
    std::vector<double> DOMMethod_DomNPEsRatioList;
    // reserve enough space for the maximum number of DOMs that can be hit with the default
    // cylinder: four strings for a vertical track in DeepCoreCore -> 240 DOMs
    // purely for optimization reasons, no physics impact
    DOMMethod_DomNPEsRatioList.reserve(240);
    unsigned int DOMMethod_N_DOMs_all = 0;
    for(dom_prop_map_t::const_iterator dom_prop_map_citer = DomPropMap.begin();
        dom_prop_map_citer != DomPropMap.end();
        ++dom_prop_map_citer
       )
    {
        // get DOM property structure for this DOM
        // NOTE: There are only DOMs inside the DomProbMap that are relevant for
        //       the current method (i.e., only hit DOMs if not UseAllDoms_).
        const dom_prop_t &dom_prop = dom_prop_map_citer->second;

        // BIN method
        if((dom_prop.dist_perpendicular_to_track >= SetMinCylinderBIN_) &&
           (dom_prop.dist_perpendicular_to_track <= SetMaxCylinderBIN_) &&
           (dom_prop.dist_along_track >= minDomDistAlongTrack) &&
           (dom_prop.dist_along_track <= maxDomDistAlongTrack)
          )
        {
            const double reduced_dom_dist_along_track = dom_prop.dist_along_track - minDomDistAlongTrack;
            // using BINMETHOD_BIN_LENGTH meter bins to try to keep number of DOMs and strings the same in each bin
            const unsigned int bin_index = static_cast<unsigned int>(reduced_dom_dist_along_track/BINMETHOD_BIN_LENGTH);
            // check if histogram is big enough
            if(bin_index >= BINMethod_MesNPEsBins.size())
            {
                // the bin index exeeds the size of the histogram
                // -> allocate more bins
                BINMethod_MesNPEsBins.resize(bin_index+1, 0.);
                BINMethod_ExpNPEsBins.resize(bin_index+1, 0.);
                BINMethod_ExpNPEsBins_AllDOMs.resize(bin_index+1, 0.);
            }
            // accumulate the bin contents
            BINMethod_MesNPEsBins[bin_index] += dom_prop.det_npes;
            BINMethod_ExpNPEsBins_AllDOMs[bin_index] += dom_prop.exp_npes;
            if(dom_prop.det_npes > 0.)
            {
                BINMethod_ExpNPEsBins[bin_index] += dom_prop.exp_npes;
            }
        }

        // DOM method
        if((dom_prop.dist_perpendicular_to_track >= SetMinCylinderDOM_) &&
           (dom_prop.dist_perpendicular_to_track <= SetMaxCylinderDOM_) &&
           (dom_prop.dist_along_track >= minDomDistAlongTrack) &&
           (dom_prop.dist_along_track <= maxDomDistAlongTrack)
          )
        {
            // count how many ratios we would have if we allowed zero ratios;
            // we do not actually need those in the vector because they do not
            // contribute to the sum
            ++DOMMethod_N_DOMs_all;
            if(dom_prop.det_npes > 0.)
            {
                DOMMethod_DomNPEsRatioList.push_back(dom_prop.det_npes / dom_prop.exp_npes);
            }
        }
    }

    //--------------------------------------------------------------------------
    // BIN Method section
    //--------------------------------------------------------------------------

    // calculate the BIN histogram ratios and count how many histogram bins are
    // filled (greater than zero)
    const unsigned BINMethod_N_bins = BINMethod_MesNPEsBins.size();
    std::vector<double> BINMethod_NPEsBinRatios(BINMethod_N_bins, 0.);
    std::vector<double> BINMethod_NPEsBinRatios_AllDOMs(BINMethod_N_bins, 0.);
    unsigned BINMethod_N_NPEsBinRatios_entries_greater_zero = 0;
    for(unsigned bin_index=0; bin_index < BINMethod_N_bins; ++bin_index)
    {
        // zero expected charge is possible because of cylinder restriction
        if(BINMethod_ExpNPEsBins[bin_index] > 0.) {
            BINMethod_NPEsBinRatios[bin_index] = BINMethod_MesNPEsBins[bin_index] / BINMethod_ExpNPEsBins[bin_index];
        }
        if(BINMethod_ExpNPEsBins_AllDOMs[bin_index] > 0.) {
            BINMethod_NPEsBinRatios_AllDOMs[bin_index] = BINMethod_MesNPEsBins[bin_index] / BINMethod_ExpNPEsBins_AllDOMs[bin_index];
        }
        // check if histogram bin is not zero
        if(BINMethod_NPEsBinRatios[bin_index] > 0.)
        {
            ++BINMethod_N_NPEsBinRatios_entries_greater_zero;
        }
    }

    // calculate the truncated reco energy by the BIN method
    double BINMethod_40p_cor_reco_E = 0.;
    double BINMethod_40p_cor_reco_E_AllDOMs = 0.;
    BINMethodEventType_t BINMethod_event_type;
    if(BINMethod_N_NPEsBinRatios_entries_greater_zero < 3)
    {
        BINMethod_event_type = BIN_METHOD_EVENT_TYPE__BAD;
    }
    else if(BINMethod_N_NPEsBinRatios_entries_greater_zero >= 3)
    {
        BINMethod_event_type = BIN_METHOD_EVENT_TYPE__BIN_COUNT_GREATER_EQUAL_3;

        /*  calculate the bin ranking
         *  We want to make a rank 1-15, if the event had 15 non-zero bins,
         *  to find the highest (rank 1) ratio, then next highest (rank 2), etc.
         *  This ranking is needed to preserve the bin index relation between
         *  the BINMethod_MesNPEsBins and BINMethod_ExpNPEsBins vectors.
         */
        std::vector<unsigned> BINMethod_NPEsBinRatios_Ranks(BINMethod_N_bins, 0);
        std::vector<unsigned> BINMethod_NPEsBinRatios_Ranks_AllDOMs(BINMethod_N_bins, 0);
        for(unsigned i=0; i<BINMethod_N_bins; ++i)
        {
            for(unsigned j=0; j<BINMethod_N_bins; ++j)
            {
                if(BINMethod_NPEsBinRatios[i] <= BINMethod_NPEsBinRatios[j])
                {
                    ++BINMethod_NPEsBinRatios_Ranks[i];
                }
                if(BINMethod_NPEsBinRatios_AllDOMs[i] <= BINMethod_NPEsBinRatios_AllDOMs[j])
                {
                    ++BINMethod_NPEsBinRatios_Ranks_AllDOMs[i];
                }
            }
        }

        // sum the actual and expected NPEs per bin by applying the
        // 40% truncation
        double BINMethod_truncated_MesNPEsBins_sum = 0.;
        double BINMethod_truncated_ExpNPEsBins_sum = 0.;
        double BINMethod_truncated_ExpNPEsBins_sum_AllDOMs = 0.;
        for(unsigned bin_index=0; bin_index < BINMethod_N_bins; ++bin_index)
        {
            if(BINMethod_NPEsBinRatios_Ranks[bin_index] > 0.40*BINMethod_N_NPEsBinRatios_entries_greater_zero)
            {
                BINMethod_truncated_MesNPEsBins_sum += BINMethod_MesNPEsBins[bin_index];
                BINMethod_truncated_ExpNPEsBins_sum += BINMethod_ExpNPEsBins[bin_index];
            }
            if(BINMethod_NPEsBinRatios_Ranks_AllDOMs[bin_index] > 0.40*BINMethod_N_NPEsBinRatios_entries_greater_zero)
            {
                BINMethod_truncated_ExpNPEsBins_sum_AllDOMs += BINMethod_ExpNPEsBins_AllDOMs[bin_index];
            }
        }

        BINMethod_40p_cor_reco_E = ref_energy * BINMethod_truncated_MesNPEsBins_sum / BINMethod_truncated_ExpNPEsBins_sum;
        BINMethod_40p_cor_reco_E_AllDOMs = ref_energy * BINMethod_truncated_MesNPEsBins_sum / BINMethod_truncated_ExpNPEsBins_sum_AllDOMs;
    }

    //--------------------------------------------------------------------------
    // DOM Method section
    //--------------------------------------------------------------------------

    // classify the event for the DOM method
    DOMMethodEventType_t DOMMethod_event_type;
    unsigned int DOMMethod_N_DOMs_hit = DOMMethod_DomNPEsRatioList.size();
    if(DOMMethod_N_DOMs_hit < 8)
    {
        DOMMethod_event_type = DOM_METHOD_EVENT_TYPE__BAD;
    }
    else if(DOMMethod_N_DOMs_hit >= 8 && DOMMethod_N_DOMs_hit < 20)
    {
        DOMMethod_event_type = DOM_METHOD_EVENT_TYPE__DOM_COUNT_GREATER_EQUAL_8_AND_LESS_20;
    }
    else //if(DOMMethod_N_DOMs_hit >= 20) // this is is always true once we get here
    {
        DOMMethod_event_type = DOM_METHOD_EVENT_TYPE__DOM_COUNT_GREATER_EQUAL_20;
        if(BINMethod_event_type == BIN_METHOD_EVENT_TYPE__BIN_COUNT_GREATER_EQUAL_3) {
            BINMethod_event_type = BIN_METHOD_EVENT_TYPE__BIN_COUNT_GREATER_EQUAL_3_AND_DOM_COUNT_GREATER_EQUAL_20;
        }
    }

    // calculate the truncated reco energy by the DOM method
    double DOMMethod_50p_cor_reco_E = 0.;
    double DOMMethod_50p_cor_reco_E_AllDOMs = 0.;
    if(DOMMethod_event_type != DOM_METHOD_EVENT_TYPE__BAD)
    {
        sort(DOMMethod_DomNPEsRatioList.begin(), DOMMethod_DomNPEsRatioList.end());

        // determines how many DOMs to cut, rounded down
        // 50% cuts, sum the first 50% lower ratios
        unsigned int n_doms_keep_50 = static_cast<unsigned int>(std::ceil(DOMMethod_N_DOMs_hit * 0.50));
        unsigned int n_doms_cut_50 = DOMMethod_N_DOMs_hit - n_doms_keep_50;
        double sum_50 = 0.;
        for(unsigned i=0; i<n_doms_keep_50; ++i)
        { 
            sum_50 += DOMMethod_DomNPEsRatioList[i];
        }

        DOMMethod_50p_cor_reco_E         = ref_energy * sum_50 / (DOMMethod_N_DOMs_hit - n_doms_cut_50);
        DOMMethod_50p_cor_reco_E_AllDOMs = ref_energy * sum_50 / (DOMMethod_N_DOMs_all - n_doms_cut_50);
    }

    /*  At this point BINMethod_event_type and DOMMethod_event_type as been set
     *  and if they are not set to BAD, then
     *  BINMethod_40p_cor_reco_E, DOMMethod_50p_cor_reco_E, and the *_AllDOMs versions
     *  have been set accordingly.
     */

    //--------------------------------------------------------------------------
    // Final output into I3Particles for BINS method
    //--------------------------------------------------------------------------

    if(BINMethod_event_type != BIN_METHOD_EVENT_TYPE__BAD)
    {
        // classic BIN mode first, i.e., non-AllBins method

        double pwr_BINS       = 0.; // in log10
        double mu_energy_BINS = 0.; // normal units
        double nu_energy_BINS = 0.; // normal units
        double res_BINS       = 0.; // in log10

        if(BINMethod_40p_cor_reco_E > 0.0)
        {
            // determine power value pwr_BINS

            // set variable with log10 of the corresponding reco energy for
            // faster comparisons
            const double log10_BINMethod_40p_cor_reco_E = log10(BINMethod_40p_cor_reco_E);

            // less precision fits
            if(BINMethod_event_type == BIN_METHOD_EVENT_TYPE__BIN_COUNT_GREATER_EQUAL_3) 
            {
                // low dedx values, pol2 fit
                if(log10_BINMethod_40p_cor_reco_E < 0.2)
                {
                    pwr_BINS =   3.62775
                               + 2.03302  * log10_BINMethod_40p_cor_reco_E
                               - 0.482786 * pow(log10_BINMethod_40p_cor_reco_E, 2.);
                }
                // middle dedx values
                else if(log10_BINMethod_40p_cor_reco_E >= 0.2 &&
                        log10_BINMethod_40p_cor_reco_E <= 2.7)
                {
                    pwr_BINS =  log10((BINMethod_40p_cor_reco_E-0.793159)/7.64771e-05);
                }
                // high dedx values
                else if(log10_BINMethod_40p_cor_reco_E > 2.7)
                {
                    pwr_BINS =   4.48103
                               + 0.658602 * log10_BINMethod_40p_cor_reco_E
                               + 0.0763384 * pow(log10_BINMethod_40p_cor_reco_E, 2.);
                }
            }
            // higher quality events, better fits
            else if(BINMethod_event_type == BIN_METHOD_EVENT_TYPE__BIN_COUNT_GREATER_EQUAL_3_AND_DOM_COUNT_GREATER_EQUAL_20)
            {
                // low dedx values, pol2 fit
                if(log10_BINMethod_40p_cor_reco_E < 0.2)
                {
                    pwr_BINS =   3.75849
                               + 2.00334 * log10_BINMethod_40p_cor_reco_E
                               - 0.621624 * pow(log10_BINMethod_40p_cor_reco_E, 2.);
                }
                // middle dedx values
                else if(log10_BINMethod_40p_cor_reco_E >= 0.2 &&
                        log10_BINMethod_40p_cor_reco_E <= 2.7)
                {
                    pwr_BINS =  log10((BINMethod_40p_cor_reco_E-0.630442)/7.00521e-05);
                }
                // high dedx values
                else if(log10_BINMethod_40p_cor_reco_E > 2.7)
                {
                    pwr_BINS =   6.36485
                               - 0.671043 * log10_BINMethod_40p_cor_reco_E
                               + 0.315628 * pow(log10_BINMethod_40p_cor_reco_E, 2.);
                }
            }

            // calculate neutrino energy
            if(log10_BINMethod_40p_cor_reco_E < 0.2) {
                nu_energy_BINS = pow(10,   4.94684
                                         + 1.61624  * log10_BINMethod_40p_cor_reco_E
                                         - 0.654983 * pow(log10_BINMethod_40p_cor_reco_E, 2.)
                                    );
            }
            else if(log10_BINMethod_40p_cor_reco_E >= 0.2) {
                nu_energy_BINS = pow(10,   5.07538
                                         + 0.894061 * log10_BINMethod_40p_cor_reco_E
                                    );
            }

            // calculate E resolution
            if(log10_BINMethod_40p_cor_reco_E < 0.2)
            {
                if(BINMethod_N_NPEsBinRatios_entries_greater_zero >= 3 &&
                   BINMethod_N_NPEsBinRatios_entries_greater_zero < 11
                  )
                {
                    res_BINS = -0.576826 + 2.77366/sqrt(BINMethod_N_NPEsBinRatios_entries_greater_zero + 2.10693);
                }
                else if(BINMethod_N_NPEsBinRatios_entries_greater_zero >= 11)
                {
                    res_BINS = 0.210;
                }
            }
            else if(log10_BINMethod_40p_cor_reco_E >= 0.2)
            {
                if(BINMethod_N_NPEsBinRatios_entries_greater_zero >= 3 &&
                   BINMethod_N_NPEsBinRatios_entries_greater_zero < 12
                  )
                {
                    res_BINS = -0.0164825 + 0.653909/sqrt(BINMethod_N_NPEsBinRatios_entries_greater_zero + 0.296923);
                }
                else if(BINMethod_N_NPEsBinRatios_entries_greater_zero >= 12)
                {
                    res_BINS = 0.175;
                }
            }
        }

        //----------------------------------------------------------------------
        // output muon particle for BINS method
        I3ParticlePtr i3particle_result_muon_BINS(new I3Particle(*reco_particle));
        // convert energy from log10(E) to E
        if(BINMethod_40p_cor_reco_E > 0.0)
        {
            mu_energy_BINS = pow(10.0, pwr_BINS);
        }
        i3particle_result_muon_BINS->SetEnergy(mu_energy_BINS);
        frame->Put(ResultParticleName_+"_BINS_Muon", i3particle_result_muon_BINS);

        // output double for dEdx
        I3DoublePtr i3double_dedx_BINS(new I3Double(BINMethod_40p_cor_reco_E));
        frame->Put(ResultParticleName_+"_BINS_dEdX", i3double_dedx_BINS);

        // output double for Eres in log10(E)
        I3DoublePtr i3double_res_BINS(new I3Double(res_BINS));
        frame->Put(ResultParticleName_+"_BINS_MuEres", i3double_res_BINS);

        // output neutrino particle for Bins method
        I3ParticlePtr i3particle_result_nu_BINS(new I3Particle(*reco_particle));
        i3particle_result_nu_BINS->SetEnergy(nu_energy_BINS);
        frame->Put(ResultParticleName_+"_BINS_Neutrino", i3particle_result_nu_BINS);
    }

    // repeat the if to get a new scope; those equations might be error-prone
    if(BINMethod_event_type != BIN_METHOD_EVENT_TYPE__BAD)
    {
        // now the same for the AllDOMs BIN method
        // here, one fit is enough; no need for different low/high quality equations

        double pwr_BINS_AllDOMs       = 0.; // in log10
        double mu_energy_BINS_AllDOMs = 0.; // normal units
        double nu_energy_BINS_AllDOMs = 0.; // normal units
        double res_BINS_AllDOMs       = 0.; // in log10

        if(BINMethod_40p_cor_reco_E_AllDOMs > 0.0)
        {
            // determine power value pwr_BINS

            // set variable with log10 of the corresponding reco energy for
            // faster comparisons
            const double log10_BINMethod_40p_cor_reco_E_AllDOMs = log10(BINMethod_40p_cor_reco_E_AllDOMs);

            // low dedx values, pol2 fit
            if(log10_BINMethod_40p_cor_reco_E_AllDOMs < 0.2)
            {
                pwr_BINS_AllDOMs =   4.04062
                                   + 1.25493  * log10_BINMethod_40p_cor_reco_E_AllDOMs
                                   - 0.738844 * pow(log10_BINMethod_40p_cor_reco_E_AllDOMs, 2.);
            }
            // middle dedx values
            else if(log10_BINMethod_40p_cor_reco_E_AllDOMs >= 0.2 &&
                    log10_BINMethod_40p_cor_reco_E_AllDOMs <= 2.7)
            {
                pwr_BINS_AllDOMs = log10((BINMethod_40p_cor_reco_E_AllDOMs-0.288059)/7.14560e-05);
            }
            // high dedx values
            else if(log10_BINMethod_40p_cor_reco_E_AllDOMs > 2.7)
            {
                pwr_BINS_AllDOMs =   5.05698
                                   + 0.175089 * log10_BINMethod_40p_cor_reco_E_AllDOMs
                                   + 0.181212 * pow(log10_BINMethod_40p_cor_reco_E_AllDOMs, 2.);
            }

            // calculate neutrino energy
            nu_energy_BINS_AllDOMs = pow(10,   5.03249
                                             + 1.041044  * log10_BINMethod_40p_cor_reco_E_AllDOMs
                                        );

            // calculate E resolution
            if(BINMethod_N_NPEsBinRatios_entries_greater_zero >= 3 &&
               BINMethod_N_NPEsBinRatios_entries_greater_zero < 12)
            {
                res_BINS_AllDOMs = -0.0793072 + 0.835233/sqrt(BINMethod_N_NPEsBinRatios_entries_greater_zero - 0.66603);
            }
            else if(BINMethod_N_NPEsBinRatios_entries_greater_zero >= 12)
            {
                res_BINS_AllDOMs = 0.175;
            }
        }

        //----------------------------------------------------------------------
        // output muon particle for BINS method in AllDOMs mode
        I3ParticlePtr i3particle_result_muon_BINS_AllDOMs(new I3Particle(*reco_particle));
        // convert energy from log10(E) to E
        if(BINMethod_40p_cor_reco_E_AllDOMs > 0.0)
        {
            mu_energy_BINS_AllDOMs = pow(10.0, pwr_BINS_AllDOMs);
        }
        i3particle_result_muon_BINS_AllDOMs->SetEnergy(mu_energy_BINS_AllDOMs);
        frame->Put(ResultParticleName_+"_AllBINS_Muon", i3particle_result_muon_BINS_AllDOMs);

        // output double for dEdx
        I3DoublePtr i3double_dedx_BINS_AllDOMs(new I3Double(BINMethod_40p_cor_reco_E_AllDOMs));
        frame->Put(ResultParticleName_+"_AllBINS_dEdX", i3double_dedx_BINS_AllDOMs);

        // output double for Eres in log10(E) in AllDOMs mode
        I3DoublePtr i3double_res_BINS_AllDOMs(new I3Double(res_BINS_AllDOMs));
        frame->Put(ResultParticleName_+"_AllBINS_MuEres", i3double_res_BINS_AllDOMs);

        // output neutrino particle for Bins method in AllDOMs mode
        I3ParticlePtr i3particle_result_nu_BINS_AllDOMs(new I3Particle(*reco_particle));
        i3particle_result_nu_BINS_AllDOMs->SetEnergy(nu_energy_BINS_AllDOMs);
        frame->Put(ResultParticleName_+"_AllBINS_Neutrino", i3particle_result_nu_BINS_AllDOMs);
    }

    //--------------------------------------------------------------------------
    // Final output into I3Particles for DOMS method
    //--------------------------------------------------------------------------

    if(DOMMethod_event_type != DOM_METHOD_EVENT_TYPE__BAD)
    {
        // classic DOM mode first, i.e., non-AllBins method

        double pwr_DOMS       = 0.; // in log10
        double mu_energy_DOMS = 0.; // normal units
        double nu_energy_DOMS = 0.; // normal units
        double res_DOMS       = 0.; // in log10

        if(DOMMethod_50p_cor_reco_E > 0.)
        {
            // determine power value pwr_DOMS

            // set variable with log10 of the corresponding reco energy for
            // faster comparisons
            const double log10_DOMMethod_50p_cor_reco_E = log10(DOMMethod_50p_cor_reco_E);

            // events with 8-19 DOMs, less precision fits
            if(DOMMethod_event_type == DOM_METHOD_EVENT_TYPE__DOM_COUNT_GREATER_EQUAL_8_AND_LESS_20)
            {
                // low dedx values, pol2 fit
                if(log10_DOMMethod_50p_cor_reco_E < 0.2)
                {
                    pwr_DOMS =   3.8632
                               + 1.83621 * log10_DOMMethod_50p_cor_reco_E
                               - 0.44404 * pow(log10_DOMMethod_50p_cor_reco_E, 2.);
                }
                // middle dedx values
                else if(log10_DOMMethod_50p_cor_reco_E >= 0.2 &&
                        log10_DOMMethod_50p_cor_reco_E <= 2.7)
                {
                    pwr_DOMS =  log10((DOMMethod_50p_cor_reco_E-0.683068)/5.52596e-05);
                }
                // high dedx values
                else if(log10_DOMMethod_50p_cor_reco_E > 2.7)
                {
                    pwr_DOMS =   6.63673
                               - 0.693507  * log10_DOMMethod_50p_cor_reco_E
                               + 0.300791 * pow(log10_DOMMethod_50p_cor_reco_E, 2.);
                }
            }
            // higher quality events with DOMs >= 20, better fits
            else if(DOMMethod_event_type == DOM_METHOD_EVENT_TYPE__DOM_COUNT_GREATER_EQUAL_20)
            {
                // low dedx values, pol2 fit
                if(log10_DOMMethod_50p_cor_reco_E < 0.2)
                {
                    pwr_DOMS =   4.07958
                               + 1.59556  * log10_DOMMethod_50p_cor_reco_E
                               - 0.353131 * pow(log10_DOMMethod_50p_cor_reco_E, 2.);
                }
                // middle dedx values
                else if(log10_DOMMethod_50p_cor_reco_E >= 0.2 &&
                        log10_DOMMethod_50p_cor_reco_E <= 2.7)
                {
                    pwr_DOMS =  log10((DOMMethod_50p_cor_reco_E-0.432383)/4.75372e-05);
                }
                else if(log10_DOMMethod_50p_cor_reco_E > 2.7)
                {
                    pwr_DOMS =   6.77789
                               - 0.945079 * log10_DOMMethod_50p_cor_reco_E
                               + 0.383592 * pow(log10_DOMMethod_50p_cor_reco_E, 2.);
                }
            }

            // calculate neutrino energy
            if(log10_DOMMethod_50p_cor_reco_E < 0.2)
            {
                nu_energy_DOMS = pow(10,   5.16307
                                         + 1.28699  * log10_DOMMethod_50p_cor_reco_E
                                         - 0.402463 * pow(log10_DOMMethod_50p_cor_reco_E, 2.)
                                    );
            }
            else if(log10_DOMMethod_50p_cor_reco_E >= 0.2)
            {
                nu_energy_DOMS = pow(10,   5.20804
                                         + 0.896602 * log10_DOMMethod_50p_cor_reco_E
                                    );
            }

            // calculate energy resolution
            // (high and low dE/dx, high and low DOMcount, different fits)
            if(log10_DOMMethod_50p_cor_reco_E < 0.2)
            {
                if(DOMMethod_N_DOMs_hit >= 8 && DOMMethod_N_DOMs_hit < 40)
                {
                    res_DOMS = -0.26171 + 3.195 / sqrt(DOMMethod_N_DOMs_hit + 3.549);
                }
                else if(DOMMethod_N_DOMs_hit >= 40)
                {
                    res_DOMS = 0.220;
                }
            }
            else if(log10_DOMMethod_50p_cor_reco_E >= 0.2)
            {
                if(DOMMethod_N_DOMs_hit >= 8 && DOMMethod_N_DOMs_hit < 50)
                {
                    res_DOMS = 0.06507 + 0.946285 / sqrt(DOMMethod_N_DOMs_hit + 3.93375);
                }
                else if(DOMMethod_N_DOMs_hit >= 50)
                {
                    res_DOMS = 0.191;
                }
            }
        }

        //----------------------------------------------------------------------
        // output muon particle for DOMS method
        I3ParticlePtr i3particle_result_muon_DOMS(new I3Particle(*reco_particle));
        // convert muon energy from log10(E) to E
        if(DOMMethod_50p_cor_reco_E > 0.)
        {
            mu_energy_DOMS = pow(10., pwr_DOMS);
        }
        i3particle_result_muon_DOMS->SetEnergy(mu_energy_DOMS);
        frame->Put(ResultParticleName_+"_DOMS_Muon", i3particle_result_muon_DOMS);

        // output double for dEdx
        I3DoublePtr i3double_dedx_DOMS(new I3Double(DOMMethod_50p_cor_reco_E));
        frame->Put(ResultParticleName_+"_DOMS_dEdX", i3double_dedx_DOMS);

        // output double for Eres in log10(E)
        I3DoublePtr i3double_res_DOMS(new I3Double(res_DOMS));
        frame->Put(ResultParticleName_+"_DOMS_MuEres", i3double_res_DOMS);

        // output neutrino particle for DOMs method
        I3ParticlePtr i3particle_result_nu_DOMS(new I3Particle(*reco_particle));
        i3particle_result_nu_DOMS->SetEnergy(nu_energy_DOMS);
        frame->Put(ResultParticleName_+"_DOMS_Neutrino", i3particle_result_nu_DOMS);
    }

    // and again, new scope for the AllDOMs method
    if(DOMMethod_event_type != DOM_METHOD_EVENT_TYPE__BAD)
    {
        // now the same for the AllDOMs DOM method
        // again, one fit is enough; no need for different low/high quality equations

        double pwr_DOMS_AllDOMs       = 0.; // in log10
        double mu_energy_DOMS_AllDOMs = 0.; // normal units
        double nu_energy_DOMS_AllDOMs = 0.; // normal units
        double res_DOMS_AllDOMs       = 0.; // in log10

        if(DOMMethod_50p_cor_reco_E_AllDOMs > 0.)
        {
            // determine power value pwr_DOMS

            // set variable with log10 of the corresponding reco energy for
            // faster comparisons
            const double log10_DOMMethod_50p_cor_reco_E_AllDOMs = log10(DOMMethod_50p_cor_reco_E_AllDOMs);

            // low dedx values, pol2 fit
            if(log10_DOMMethod_50p_cor_reco_E_AllDOMs < 0.2)
            {
                pwr_DOMS_AllDOMs =   4.27281
                                   + 1.12814  * log10_DOMMethod_50p_cor_reco_E_AllDOMs
                                   - 0.154217 * pow(log10_DOMMethod_50p_cor_reco_E_AllDOMs, 2.);
            }
            // middle dedx values
            else if(log10_DOMMethod_50p_cor_reco_E_AllDOMs >= 0.2 &&
                    log10_DOMMethod_50p_cor_reco_E_AllDOMs <= 2.7)
            {
                pwr_DOMS_AllDOMs = log10((DOMMethod_50p_cor_reco_E_AllDOMs-0.0623464)/4.92307e-05);
            }
            else if(log10_DOMMethod_50p_cor_reco_E_AllDOMs > 2.7)
            {
                pwr_DOMS_AllDOMs =   6.54385
                                   - 0.791083 * log10_DOMMethod_50p_cor_reco_E_AllDOMs
                                   + 0.35674 * pow(log10_DOMMethod_50p_cor_reco_E_AllDOMs, 2.);
            }

            // calculate neutrino energy
            nu_energy_DOMS_AllDOMs = pow(10,   5.25034
                                             + 1.014671  * log10_DOMMethod_50p_cor_reco_E_AllDOMs
                                        );

            // calculate energy resolution
            if(DOMMethod_N_DOMs_hit >= 8 && DOMMethod_N_DOMs_hit < 50)
            {
                res_DOMS_AllDOMs = -0.0435194 + 1.69656 / sqrt(DOMMethod_N_DOMs_hit + 0.768114);
            }
            else if(DOMMethod_N_DOMs_hit >= 50)
            {
                res_DOMS_AllDOMs = 0.190;
            }
        }

        //----------------------------------------------------------------------
        // output muon particle for DOMS method in AllDOMs mode
        I3ParticlePtr i3particle_result_muon_DOMS_AllDOMs(new I3Particle(*reco_particle));
        // convert muon energy from log10(E) to E
        if(DOMMethod_50p_cor_reco_E_AllDOMs > 0.)
        {
            mu_energy_DOMS_AllDOMs = pow(10., pwr_DOMS_AllDOMs);
        }
        i3particle_result_muon_DOMS_AllDOMs->SetEnergy(mu_energy_DOMS_AllDOMs);
        frame->Put(ResultParticleName_+"_AllDOMS_Muon", i3particle_result_muon_DOMS_AllDOMs);

        // output double for dEdx
        I3DoublePtr i3double_dedx_AllDOMs(new I3Double(DOMMethod_50p_cor_reco_E_AllDOMs));
        frame->Put(ResultParticleName_+"_AllDOMS_dEdX", i3double_dedx_AllDOMs);

        // output double for Eres in log10(E) in AllDOMs mode
        I3DoublePtr i3double_res_DOMS_AllDOMs(new I3Double(res_DOMS_AllDOMs));
        frame->Put(ResultParticleName_+"_AllDOMS_MuEres", i3double_res_DOMS_AllDOMs);

        // output neutrino particle for DOMs method in AllDOMs mode
        I3ParticlePtr i3particle_result_nu_DOMS_AllDOMs(new I3Particle(*reco_particle));
        i3particle_result_nu_DOMS_AllDOMs->SetEnergy(nu_energy_DOMS_AllDOMs);
        frame->Put(ResultParticleName_+"_AllDOMS_Neutrino", i3particle_result_nu_DOMS_AllDOMs);
    }

    // all events should get the Orig Photorec energy value as a minimum

    const double ICE_DENSITY = 0.931; // density of ice 0.931, needs to be updated to 0.92 ?
    const double DEDX_A = ICE_DENSITY*0.25958;
    const double DEDX_B = ICE_DENSITY*3.5709e-4;

    //--------------------------------------------------------------------------
    // output particle for Orig Photorec
    I3ParticlePtr i3particle_photorec_result(new I3Particle(*reco_particle));
    double muon_energy_ORIG = ( OrigPhotorec - DEDX_A ) / DEDX_B;
    if(muon_energy_ORIG < 0.0 || std::isinf(muon_energy_ORIG))
    {
        // method doesn't work for small energy, dE/dx is too small
        muon_energy_ORIG = 0.0;
    }
    i3particle_photorec_result->SetEnergy(muon_energy_ORIG);
    frame->Put(ResultParticleName_+"_ORIG_Muon", i3particle_photorec_result);

    //--------------------------------------------------------------------------
    // give the Original dE/dx value for all events
    I3DoublePtr dEdX(new I3Double(OrigPhotorec));
    frame->Put(ResultParticleName_+"_ORIG_dEdX", dEdX);

    //--------------------------------------------------------------------------
    // neutrino for Orig Photorec
    double nu_energy_ORIG = 0.; // normal units
    I3ParticlePtr i3particle_result_nu_ORIG(new I3Particle(*reco_particle));
    if(OrigPhotorec > 0.0)
    {
        nu_energy_ORIG = pow(10, 4.80773 + log10(OrigPhotorec)*1.031928);
    }
    if(nu_energy_ORIG < 0.0 || std::isinf(nu_energy_ORIG))
    {
        // method doesn't work for small energy, dE/dx is too small
        nu_energy_ORIG = 0.0;
    }
    i3particle_result_nu_ORIG->SetEnergy(nu_energy_ORIG);
    frame->Put(ResultParticleName_+"_ORIG_Neutrino", i3particle_result_nu_ORIG);

    // Don't forget to push the frame back to the outbox!!!
    PushFrame(frame, "OutBox");
}

//______________________________________________________________________________
/** This method gives the user some screen output to let them know how the
 *  program has performed.
 */
void
I3TruncatedEnergy::Finish()  
{
    if(nMissingPulses_ > 0) {
        log_warn("(%s) Missing pulses in %u events!",
            GetName().c_str(), nMissingPulses_);
    }
    if(nMissingParticle_ > 0) {
        log_warn("(%s) Missing input particle in %u events!",
            GetName().c_str(), nMissingParticle_);
    }

    log_debug("(%s) Photonics calls: %u OK, %u FAIL.",
        GetName().c_str(), nCallsSuccess_, nCallsFail_);
}

