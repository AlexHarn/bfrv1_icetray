/**
 * class: I3MonopolePropagator.cxx
 * (c) 2008 IceCube Collaboration
 * Version $Id: I3MonopolePropagator.cxx 141809 2016-02-13 17:31:29Z anna.obertacke $
 *
 * Date 06 Feb 2008
 * @version $Revision: 141809 $
 * @date $Date: 2016-02-13 11:31:29 -0600 (Sa, 13. Feb 2016) $
 * @author Brian Christy <bchristy@icecube.umd.edu>
 * @author Alex Olivas <olivas@icecube.umd.edu>
 *
 * @brief A module to convert Monopoles into a chain of propagated
 * @brief particles through the ice
 *
 */

#include "monopole-generator/I3MonopolePropagator.h"
#include "I3MonopoleRelativisticUtils.h"
#include "I3MonopoleSlowUtils.h"


#include "icetray/I3Frame.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"


I3_MODULE(I3MonopolePropagator);

I3MonopolePropagator::I3MonopolePropagator(const I3Context &ctx) :
        I3Module(ctx),
        inputTreeName_("I3MCTree"),
        outputTreeName_("I3MCTree"),
        infoName_("MPInfoDict"),
        betaThreshold_(0.09),

        // fast
        calcEn_(true),
        calcDen_(true),
        stepSize_(NAN),
        checkParticle_(true),
        speedmin_(0.09 * I3Constants::c),
        maxlength_(10.0 * I3Units::m),
        minlength_(0.001 * I3Units::m),
        maxdistfromcenter_(1300 * I3Units::m),
        profiling_(false),

        // slow
        meanFreePath_(NAN),
        useCorrectDecay_(false),
        scaleEnergy_(false),
        energyScaleFactor_(1.0) {
    log_debug("Constructor I3MonopolePropagator");

    AddParameter("InputTreeName", "Name of tree to read from frame", inputTreeName_);
    AddParameter("OutputTreeName", "Name of tree to write to the frame", outputTreeName_);
    AddParameter("InfoName", "Name of Info Map", infoName_);
    AddParameter("BetaThreshold", "Threshold wether to handle as fast or slow monopole. ", betaThreshold_);
    // fast
    AddParameter("CalculateEnergy", "Whether to use energy loss formula", calcEn_);
    //AddParameter("CalculateDensityCorrection", "Whether to calculate the density correction",calcDen_);
    AddParameter("StepSize", "Length of segments", stepSize_);
    //AddParameter("IncludeErrorChecking","Whether to check that each particle is filled correctly",checkParticle_);
    AddParameter("SpeedMin", "Minimum speed of monopole before propagation ends", speedmin_);
    AddParameter("MaxLength", "Maximum length of step size (if not set directly)", maxlength_);
    AddParameter("MinLength", "Minimum length of step size (if not set directly)", minlength_);
    AddParameter("MaxDistanceFromCenter",
                 "Maximum distance from center to keep propagating monopole", maxdistfromcenter_);
    AddParameter("Profiling", "", profiling_);
    // slow
    AddParameter("MeanFreePath", "Mean Free Path", meanFreePath_);
    AddParameter("ScaleEnergy", "Whether to scale up energy and mean free path", scaleEnergy_);
    AddParameter("UseCorrectDecay", "Wheter to simulate 2 opposite cascades ( 2 * ~ 500 MeV) instead of 1 GeV",
                 useCorrectDecay_);
    AddParameter("EnergyScaleFactor",
                 "Scale down the cascade energy in order to test the influence of other decay channels",
                 energyScaleFactor_);


    AddOutBox("OutBox");
}

I3MonopolePropagator::~I3MonopolePropagator() { }

void I3MonopolePropagator::Configure() {
    log_debug("Configure I3MonopolePropagator");

    GetParameter("InputTreeName", inputTreeName_);
    GetParameter("OutputTreeName", outputTreeName_);
    GetParameter("InfoName", infoName_);
    GetParameter("BetaThreshold", betaThreshold_);
    // fast
    GetParameter("CalculateEnergy", calcEn_);
    //GetParameter("CalculateDensityCorrection",calcDen_);
    GetParameter("StepSize", stepSize_);
    //GetParameter("IncludeErrorChecking",checkParticle_);
    GetParameter("SpeedMin", speedmin_);
    GetParameter("MaxLength", maxlength_);
    GetParameter("MinLength", minlength_);
    GetParameter("MaxDistanceFromCenter", maxdistfromcenter_);
    // slow
    GetParameter("MeanFreePath", meanFreePath_);
    GetParameter("ScaleEnergy", scaleEnergy_);
    GetParameter("UseCorrectDecay", useCorrectDecay_);
    GetParameter("EnergyScaleFactor", energyScaleFactor_);



    /**
     * Check that input parameters are sane
     */

    // fast
    if ((calcDen_ == true) && (calcEn_) == false) {
        log_warn(
                "Cannot calculate density correction (default true) if calculate energy loss is false; setting energy calculation to true");
        calcEn_ = true;
    }
    if (minlength_ > maxlength_) {
        log_fatal("Oops, MaxLength<MinLength.  Did you switch them?");
    }
    if ((speedmin_ < 0.1 * I3Constants::c) && (calcEn_ == true)) {
        log_warn(
                "Propagate method will continue until monopoles reach a speed of %f. However, shell corrections (amongst other things), ignored in this ionization formula, might be becoming important. If you just want to use this as a track segmenter, set CalculateEnergy to false. Othewise, make sure you know what you're doing",
                speedmin_);
    }

    // slow
    if (meanFreePath_ <= 0.0) {
        log_fatal("Mean Free Path must be a positive number");
    }

    if (energyScaleFactor_ > 1.0 || energyScaleFactor_ <= 0) {
        log_fatal(
                "The vaild range for the energy scale factor is between > 0 and <= 1.0, since then the cascade energy cannot be zero or negative and not be greater than the proton mass (rest energy). ");
    }
    if (useCorrectDecay_ && scaleEnergy_) {
        log_fatal(
                "Correct decay and scale energy cannot be used together, since correct decay depends on a hard coded energy.");
    }
    if (useCorrectDecay_ && energyScaleFactor_ != 1.0) {
        log_fatal(
                "Correct decay and energy scale factor cannot be used together, since correct decay depends on a hard coded energy.");
    }

    deltaEnergy_ = 0.94 * I3Units::GeV * energyScaleFactor_;

    if (scaleEnergy_) {
        //meanFreePath could be NAN here. Why is this not a problem?
        //wouldn't this set deltaEnergy to NAN as well? //FHL
        if (meanFreePath_ >= 1.0 * I3Units::m) {
            //message does not make sense, we are checking meanFreePath, not an Energy! //fhl
            log_error("Cannot scale energy down! 1 GeV is lower limit\n");
            log_error("Scale Energy turned off");
        }
        else { // scaling energy to 1 m mean free path
            log_info("Scaling Energy/MFP up");
            deltaEnergy_ = ((1.0 * I3Units::m) / meanFreePath_) * deltaEnergy_;
            meanFreePath_ = 1.0 * I3Units::m;
        }
    }

    log_debug("Energy for Delta Electrons is %f (GeV)", deltaEnergy_);
    log_debug("Mean Free Path for Delta Electrons is %f (m)", meanFreePath_);

    /**
    TODO rest
    */
}

void I3MonopolePropagator::DAQ(I3FramePtr frame) {
    log_debug("Entering DAQ I3MonopolePropagator");

    // get generated tree
    I3MCTreeConstPtr mctree = frame->Get<I3MCTreeConstPtr>(inputTreeName_);

    // create new tree for propagated monopole
    I3MCTreePtr prop_tree(new I3MCTree(*mctree));

    const I3MapStringDouble &mpinfo = frame->Get<I3MapStringDouble>(infoName_);
    const double &beta = mpinfo.at("Beta");

    if (beta <= 0.0 or beta > 0.99995) { //0.99995 is upper limit for relativistic propagator
        log_fatal("Got beta from %s frame, but value is not sane: %g", infoName_.c_str(), beta);
    }

    if (beta > betaThreshold_) {  // relativistic monopole
        I3MonopoleRelativisticUtils::PropagateFastMonopole(frame,
                                                           infoName_,
                                                           mctree,
                                                           prop_tree,
                // params
                                                           stepSize_,
                                                           minlength_,
                                                           maxlength_,
                                                           calcEn_,
                                                           calcDen_,
                                                           speedmin_,
                                                           maxdistfromcenter_,
                                                           checkParticle_,
                                                           profiling_);
    } // end fast

    else {       // non-relativistic monopole

        I3RandomServicePtr random = context_.Get<I3RandomServicePtr>();
        if (!random) {
            log_fatal("Failed to Get Random Service");
        }

        if (std::isnan(
                meanFreePath_)) {   // If the monopole is slow or not is read from the frame, so we have to check this in the DAQ routine.
            log_fatal("The frame seems to contain a slow monopole, but no mean free path is configured.");
        }

        I3MonopoleSlowUtils::PropagateSlowMonopole(frame,
                                                   infoName_,
                                                   mctree,
                                                   prop_tree,
                                                   // params
                                                   random,
                                                   meanFreePath_,
                                                   useCorrectDecay_,
                                                   deltaEnergy_);
    } // end slow


    //Check that there were monopoles
    /*log_info("Propagated %d Monopoles",numMonopoles);
    if(numMonopoles==0)
      log_error("Did not have any monopoles in the tree - should you be calling this module?");
    */ // OBI TODO


    //Remove old tree and replace with new, propagated one
    if (inputTreeName_ == outputTreeName_) {
        frame->Delete(inputTreeName_);
    }
    frame->Put(outputTreeName_, prop_tree);
    PushFrame(frame, "OutBox");

}//End DAQ

void I3MonopolePropagator::Finish() {
}


