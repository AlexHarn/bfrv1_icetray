/**
 * class: I3MonopoleRelativisticUtils.cxx
 * (c) 2004 IceCube Collaboration
 * Version $Id: I3MonopoleRelativisticUtils.cxx 124988 2014-10-23 15:17:00Z jacobi $
 *
 * Date 20 Oct 2006
 * @version $Revision: 124988 $
 * @date $Date: 2014-10-23 10:17:00 -0500 (Do, 23. Okt 2014) $
 * @author Brian Christy <bchristy@icecube.umd.edu>
 * @author Alex Olivas <olivas@icecube.umd.edu>
 * @brief Utility Namespace for the I3MonopolePropagator Module
 */
#include "I3MonopoleRelativisticUtils.h"

#include <icetray/I3Units.h>
#include <dataclasses/I3Constants.h>
#include <dataclasses/physics/I3MCTreeUtils.h>
#include <phys-services/I3Calculator.h>
#include <cmath>

using namespace std;

//Constants for Ionization Loss
const double MASS_E = 5.11e-4;     //In GeV/c^2
const double ION_LOSS = 7.5e-8;    //In GeV
const double DENSITY = 9.17e5;       //in g/m^3
const double Z_OVER_A = 0.55509;     //in mol/g

//Fairly certain formula is in cgs units, so write
//e^2 = (4.80320441e-10)^2esu^2->(...)(1e-18/1.6e-19)GeV*m
const double CHARGE_E_SQRD = 1.439964459e-18;        //GeV*m
const double COEFF1 = (4 * I3Constants::pi *
                       I3Constants::NA * DENSITY *
                       Z_OVER_A);                    //in m^-3
const double COEFF2 = pow(0.5 * 137.0 * CHARGE_E_SQRD, 2);
//in GeV^2 m^2
const double COEFF3 = MASS_E;                        //in GeV
const double COEFF = COEFF1 * COEFF2 / COEFF3;           //in GeV/m

//Values given for g=137e/2
const double BLOCH_CORR = 0.248;
const double QED_CORR = 0.406;

//Constants for Density Correction
const double A_DC = 0.09116;
const double X0 = 0.240;
const double X1 = 2.8004;
const double M_DC = 3.477;
const double C_BAR = 3.5017;

//For Calculation of new length
const double AVG_ENERGY_LOSS = 1200.0;//in GeV/m;

/**
 *Supervises the monopole propagation
 *@param frame instance of current frame
 *@param infoName_ name of info frame
 *@param mctree original mctree created by generator
 *@param prop_tree copy of mctree which you may adjust
 *@param stepSize if set, will over-ride track length setting algorithm
 *@param minlength if no step size set, this represents smallest segment value allowed
 *@param maxlength if no step size set, this represents largest segment value allowed
 *@param calcEn whether to actually calculate energy loss or just segment track
 *@param calcDen whether to include density correction in energy loss
 *@param speedmin_ if below that speed propagation stops
 *@param maxdistfromcenter_ propagation limit
 *@param checkParticle whether to include exhaustive checking
 *@param profiling_ if profiling should be written into frame
 */

void
I3MonopoleRelativisticUtils::PropagateFastMonopole(
        I3FramePtr frame,
        std::string infoName_,
        I3MCTreeConstPtr input_tree,
        I3MCTreePtr output_tree,
        // params
        double stepSize_,
        double minlength_,
        double maxlength_,
        bool calcEn_,
        bool calcDen_,
        double speedmin_,
        double maxdistfromcenter_,
        bool checkParticle_,
        bool profiling_) {

    log_debug("Entering PropagateFastMonopole");

    // get mass from info frame
    const I3MapStringDouble &mpinfo = frame->Get<I3MapStringDouble>(infoName_);
    const double &mp_mass = mpinfo.at("Mass");
    if (mp_mass < 1e5 * I3Units::GeV || mp_mass > 1e17 * I3Units::GeV) {
        log_fatal("Mass (%f GeV) is out of range", mp_mass / I3Units::GeV);
    }

    //I3MCTree::const_iterator prim_iter;
    //I3MCTree::iterator prop_tree_iter = prop_tree->begin();
    for (I3MCTree::const_iterator input_tree_iter = input_tree->begin(); input_tree_iter != input_tree->end(); input_tree_iter++) {
    //for (auto input_tree_iter : *input_tree) {
    //I3MCTree::const_iterator input_tree_iter;
    //for (input_tree_iter = input_tree->begin(); input_tree_iter != input_tree->end(); input_tree_iter++) {
        if (input_tree_iter->GetType() != I3Particle::Monopole) { continue; } // do not alter any no Monopoles
        //set length of initial particle!
        auto initial_monopole_iter = output_tree->find(*input_tree_iter);
        //Initialize the length if it is either not set or too large
        initial_monopole_iter->SetLength(CalculateNextLength(stepSize_, initial_monopole_iter->GetEnergy(), mp_mass, maxlength_, minlength_));
        I3Particle initial_monopole = *initial_monopole_iter;
        //no changes to the initial monopole from here on out, only checking but no setting
        
        //Check if particle is within valid range for Ionization formula used
        if ((initial_monopole.GetSpeed() < 0.1 * I3Constants::c) && (calcEn_ == true)) {
            log_warn(
                    "Attempting to calculate energy for a slow monopole (with beta %f); Shell corrections, ignored in this ionization formula, might be becoming important; If you just want to use this as a track segmenter, set CalculateEnergy to false; Othewise, make sure you know what you're doing",
                    initial_monopole.GetSpeed() / I3Constants::c);
        }
        if ((initial_monopole.GetSpeed() >= 0.99995 * I3Constants::c) && (calcEn_ == true)) {
            log_warn(
                    "So far, energy calculation only does Ionization Loss; For a monopole of beta %f, stochastic's become important; Make sure you know what you're doing",
                    initial_monopole.GetSpeed() / I3Constants::c);
        }
        //Check if particle is within the max distance its supposed to propagate
        //If not, particle start position will be used as max distance
        if ((initial_monopole.GetPos().GetR()) > maxdistfromcenter_) {
            log_warn(
                    "Start of particle further away from Detector Center than max distance its supposed to propagate; Propagation will continue until it reaches same distance as original start (%f); Currently, MaxDistanceFromCenter variable set to %f, and WILL NOT BE USED; Please make sure this is what you intended (perhaps just didn't set MaxDistanceFromCenter?)",
                    initial_monopole.GetPos().GetR(), maxdistfromcenter_);
        }

        log_debug("-----Starting Propagate on Monopole----");
        log_debug("Position: (%f,%f,%f)", initial_monopole.GetX(),
                  initial_monopole.GetY(), initial_monopole.GetZ());
        log_debug("Energy: %f GeV", initial_monopole.GetEnergy() / I3Units::GeV);
        log_debug("Length: %f m", initial_monopole.GetLength() / I3Units::m);
        if (checkParticle_) {
            I3MonopoleRelativisticUtils::CheckParticle(initial_monopole, mp_mass);
        }

        //Each loop generates a new track segment and adds to the tree
        int count = 0;
        I3Particle temp0 = initial_monopole;
        I3ParticlePtr temp1;
        while (true) {
            count++;
            
            temp1 = I3MonopoleRelativisticUtils::AddMonopole(temp0,
                                                            mp_mass,
                                                            calcEn_, calcDen_,
                                                            checkParticle_,
                                                            stepSize_,
                                                            maxlength_, minlength_);
            if (!temp1) {
                log_fatal("New particle did not get filled!");
                return; //silence temp could be null messages from IDEs //fhl
            }
            log_debug("New particle set with Energy %f(GeV)", temp1->GetEnergy() / I3Units::GeV);

            //add new monopole to the ouput tree as a child of the previous monopole
            output_tree->append_child(temp0.GetID(), *temp1);
            
            //Check to end loop
            //If speed falls below user defined minimum
            if (temp1->GetSpeed() <= speedmin_) {
                log_info("Beta of monopole (%f) fell below (%f)",
                         temp1->GetSpeed() / I3Constants::c,
                         speedmin_ / I3Constants::c);
                break;
            }
            /**
             *Note: If particle starts further away from detector,
             *      it will use that distance as the cutoff.
             *      Otherwise, cutoff whenever it goes beyond user defined max distance
             */
            if ((temp1->GetPos().GetR() > initial_monopole.GetPos().GetR()) &&
                (temp1->GetPos().GetR() >= maxdistfromcenter_)) {
                log_info("Monopole has moved through detector beyond %f(m) of detector center",
                         temp1->GetPos().GetR() / I3Units::m);
                break;
            }

            //catch just to ensure no infinite loops
            if (count > 100000) {
                log_warn("Propagator Method Exited due to too many iterations.");
                break;
            }
            
            //generated monopole becomes the previous monopole for the next loop iteration!
            temp0 = *temp1;
        }// End while loop
        
    }//End for loop over MCTree

    if (profiling_) {
        I3VectorDoublePtr profile(new I3VectorDouble());
        I3MonopoleRelativisticUtils::MPSpeedProfile(output_tree, profile);
        frame->Put("MPSpeedProfile", profile);
    }


}


/**
 *Generates monopole at end of given segment
 *@param mp the starting monopole particle
 *@param monopoleMass the mass of the starting monopole particle
 *@param calcEn whether to actually calculate energy loss or just segment track
 *@param calcDen whether to include density correction in energy loss
 *@param checkParticle whether to include exhaustive checking
 *@param stepSize if set, will over-ride track length setting algorithm
 *@param maxlength if no step size set, this represents largest segment value allowed
 *@param minlength if no step size set, this represents smallest segment value allowed
 *@return The next monopole at the end of the starting monopole segment
 */
I3ParticlePtr
I3MonopoleRelativisticUtils::AddMonopole(I3Particle mp,
                                         double monopoleMass,
                                         bool calcEn = true,
                                         bool calcDen = true,
                                         bool checkParticle = true,
                                         double stepSize = NAN,
                                         double maxlength = 10 * I3Units::m,
                                         double minlength = 0.001 * I3Units::m) {
    log_debug("Entering AddMonopole");
    double mpMass = monopoleMass / I3Units::GeV;
    double startEnergy = mp.GetEnergy() / I3Units::GeV;
    double startBeta = EnergyToBeta(startEnergy, mpMass);
    double startTime = mp.GetTime() / I3Units::ns;
    double trackLength = mp.GetLength() / I3Units::m;

    //Create new particle
    I3ParticlePtr mpNext(new I3Particle);
    mpNext->SetType(I3Particle::Monopole);
    mpNext->SetLocationType(I3Particle::InIce);
    mpNext->SetDir(mp.GetDir());
    //mpNext->SetPos(mp.ShiftAlongTrack(trackLength * I3Units::m));
    mpNext->SetPos(mp.GetPos() + mp.GetDir() * trackLength * I3Units::m);
    log_debug("New Particle Position (%f,%f,%f)",
              mpNext->GetX(), mpNext->GetY(), mpNext->GetZ());
    log_debug("New Particle Direction: %f*PI zenith, %f*PI azimuth",
              mpNext->GetZenith() / I3Constants::pi,
              mpNext->GetAzimuth() / I3Constants::pi);

    //Set new time in units of nanoseconds
    double timeNext = startTime +
                      trackLength / (mp.GetSpeed() * I3Units::ns / I3Units::m);
    mpNext->SetTime(timeNext * I3Units::ns);
    log_debug("New time %f (ns)", timeNext * I3Units::ns);

    double energyNext = 0.0;
    if (calcEn) {
        double energyLoss = CalculateEnergyLoss(startBeta, trackLength, calcDen);
        energyNext = startEnergy - energyLoss;
        log_debug("Energy Loss calculated as %f", energyLoss);
    }
    else {
        energyNext = startEnergy;
    }

    mpNext->SetEnergy(energyNext * I3Units::GeV);
    mpNext->SetSpeed(EnergyToBeta(energyNext, mpMass) * I3Constants::c);
    log_debug("New Energy set to %f GeV", energyNext);
    log_debug("New Beta set to %f ", mpNext->GetSpeed() / I3Constants::c);

    double lengthNext = CalculateNextLength(stepSize, energyNext, mpMass, maxlength, minlength);

    mpNext->SetLength(lengthNext * I3Units::m);
    log_debug("New Length set to %f", lengthNext);

    if (checkParticle)
        CheckParticle(*mpNext, mpMass);

    return mpNext;
}


/**
 * Responsible for taking a given segment of the Monopole and calculating
 * the energy loss due to Ionization (with Density Correction on/off option)
 * @param sBeta starting beta value of monopole
 * @param tLength length of segment to calculate energy loss over
 * @param cDensityCorrection whether to include density correction calculation
 * @return the Ionization energy loss for the given segment and starting speed
 */
double
I3MonopoleRelativisticUtils::CalculateEnergyLoss(double sBeta,
                                                 double tLength,
                                                 bool cDensityCorrection) {
    log_debug("Entering CalculateEnergyLoss");

    double densityCorrection = 0.0;
    if (cDensityCorrection) {
        densityCorrection = CalculateDensityCorrection(sBeta);
    }

    double dEdX = COEFF * (log((2 * MASS_E * pow(sBeta, 2)) / (ION_LOSS * (1 - pow(sBeta, 2)))) + (QED_CORR / 2) - 0.5 -
                           (densityCorrection / 2) - BLOCH_CORR);
    log_debug("dE/dX is %f", dEdX);

    return dEdX * tLength;
}


/**
 * Calculates the density correction part of the Ionization energy loss
 * for a given speed
 * @param betaPrime starting beta used to calculate density correction
 * @return density correction
 */
double
I3MonopoleRelativisticUtils::CalculateDensityCorrection(double betaPrime) {
    log_debug("Entering CalculateDensityCorrection");
    double denCorr = 0.0;
    double gammaPrime = 1 / (sqrt(1 - pow(betaPrime, 2)));
    double X = log10(betaPrime * gammaPrime);

    if (X > X0 && X < X1) {
        denCorr = log(pow(betaPrime * gammaPrime, 2)) - C_BAR + A_DC * pow((X1 - X), M_DC);
    }
    else if (X > X1) {
        denCorr = log(pow(betaPrime * gammaPrime, 2)) - C_BAR;
    }
    log_debug("Density Correction found to be %f", denCorr);
    return denCorr;
}


/**
 *Determines length to use for newly generated monopole
 *by estimating how far it could travel before losing 0.1%
 *of its kinetic energy
 *@param nEnergy starting energy of particle
 *@param particleMass mass of particle
 *@param maxL represents largest segment value allowed
 *@param minL represents smallest segment value allowed
 *@return A length over which a monopole would lose roughly 0.1% of its KE
 * within user defined boundaries
 */

double
I3MonopoleRelativisticUtils::CalculateNextLength(double stepSize,
                                                 double nextEnergy,
                                                 double particleMass,
                                                 double maxLength,
                                                 double minLength) {
    if (!__builtin_isnan(stepSize)) {
        return stepSize;
    }
    double newLength = 0.0;

    //Pick length so Energy loss is roughly 0.1% of total KE
    newLength = (0.001) * (nextEnergy - particleMass) / AVG_ENERGY_LOSS;
    
    if (__builtin_isnan(newLength)) {
        log_fatal("new length was NaN and was not set");
    }

    if (newLength > maxLength) {
        newLength = maxLength;
    }
    else if (newLength < minLength) {
        newLength = minLength;
    }

    if (newLength == 0.0) {
        log_fatal("new length was 0 and was not set");
    }
    return newLength;

}

/**
 * Helper function - got tired of writing function over and over
 * @param energy energy of particle
 * @param mass mass of particle
 * @return beta of particle
 */
double
I3MonopoleRelativisticUtils::EnergyToBeta(double energy, double mass) {
    double gamma = energy / mass;
    double beta = sqrt(1 - pow(gamma, -2));
    return beta;
}

/**
 * Responsible for performing extensive sanity checks on particle
 * result to ensure nothing went horribly wrong
 * @param checkptr particle to check
 * @param checkmass mass of particle to check
 */

void
I3MonopoleRelativisticUtils::CheckParticle(I3Particle &particle, double checkmass) {

    //if (!checkptr) {
    //    log_fatal("Somethings wrong...there is no particle");
    //    return; //while log_fatal already returns, this silences error messages from intelligent IDEs
    //    //which wrongly assume checkptr could be Null after this point
    //}
    if (particle.GetType() != I3Particle::Monopole)
        log_fatal("Particle is not a monopole! D'oh!");
    if (__builtin_isnan(particle.GetX()) || __builtin_isnan(particle.GetY()) || __builtin_isnan(particle.GetZ()))
        log_fatal("Particle position is not set");
    if ((particle.GetPos().GetR() / I3Units::m) > 5000)
        log_fatal("Particle more than 5 km away...why am I looking at this?");
    if (__builtin_isnan(particle.GetZenith()) || __builtin_isnan(particle.GetAzimuth()))
        log_fatal("Particle direction is not set");
    if (__builtin_isnan(particle.GetEnergy()) || particle.GetEnergy() == 0.0)
        log_fatal("Particle has no energy");
    if (__builtin_isnan(particle.GetTime()))
        log_fatal("Particle has no time");
    if (__builtin_isnan(particle.GetLength()) || (particle.GetLength() <= 0.0))
        log_fatal("Something's wrong with the length: %f", particle.GetLength());
    double checkbeta = EnergyToBeta(particle.GetEnergy(), checkmass);
    if (checkbeta > 0.99995)
        log_fatal("Pole out of Control!!! - speed of %f too much for assumptions of this module", checkbeta);

}

/**
 * Update the dictionary created by the monopole generator
 * @param mctree the monopoles mctree
 * @param mpinfo pointer to the dictionary(I3MapStringDouble) to update
 */

void
I3MonopoleRelativisticUtils::UpdateMPInfoDict(I3MCTreeConstPtr mctree, I3MapStringDoublePtr mpinfo) {
    //this function does not make any sense as long as we do not assume that we only have a tree made out of
    //monopoles!! This shouldn't really be here! //fh
    I3Particle maxEnergeticPrimary = I3Particle();
    maxEnergeticPrimary.SetEnergy(0);
    vector<I3Particle> heads = mctree->get_heads();
    for(std::vector<I3Particle>::iterator it = heads.begin(); it != heads.end(); it++){
    //for(I3Paricle particle: mctree->get_heads()){
        if (maxEnergeticPrimary.GetEnergy() < it->GetEnergy()){
            maxEnergeticPrimary = *it;
        }
    }
    //looop through is children to see track lenght and energy loss
    int steps = 0;
    
    I3Particle tmp = maxEnergeticPrimary;
    while (mctree->number_of_children(tmp.GetID())) {
        if (mctree->number_of_children(tmp.GetID()) > 1) log_fatal("wasn't expecting more than one child.");
        if (tmp.GetType() == I3Particle::Monopole) {
            steps++;
            tmp = mctree->first_child(tmp.GetID());
        }else{
            break;
        }
    }
    
    //I3MCTree::iterator iter = I3MCTreeUtils::GetMostEnergeticPrimary(*mctree);
    //I3ParticlePtr startParticlePtr(new I3Particle(*iter));
    //int steps = 0;
    //while (iter.number_of_children()) {
    //    if (iter.number_of_children() > 1) log_fatal("wasn't expecting more than one child.");
    //    if (iter->GetType() == I3Particle::Monopole) {
    //        steps++;
    //        iter = mctree->child(iter, 0);
    //    }
    //}
    (*mpinfo)["NSteps"] = double(steps);
    (*mpinfo)["ELoss"] = maxEnergeticPrimary.GetEnergy() - tmp.GetEnergy();
    (*mpinfo)["TrackLength"] = I3Calculator::Distance(tmp, maxEnergeticPrimary);
}

/**
 * Extract the speed profile of the monopole from the MCTree.
 * @param mctree the monopoles mctree
 * @param speed_prof pointer to the vector storing the profile
 */

void
I3MonopoleRelativisticUtils::MPSpeedProfile(I3MCTreeConstPtr mctree, I3VectorDoublePtr speed_prof) {
    speed_prof->clear();
    
    
    //this function does not make any sense as long as we do not assume that we only have a tree made out of
    //monopoles!! This shouldn't really be here! //fh
    I3Particle maxEnergeticPrimary = I3Particle();
    maxEnergeticPrimary.SetEnergy(0);
    //for(auto particle: mctree->get_heads()){
    vector<I3Particle> heads = mctree->get_heads();
    for(std::vector<I3Particle>::iterator it = heads.begin(); it != heads.end(); ++it){
        if (maxEnergeticPrimary.GetEnergy() < it->GetEnergy()){
            maxEnergeticPrimary = *it;
        }
    }
    
    I3Particle tmp = maxEnergeticPrimary;
    while (mctree->number_of_children(tmp.GetID())) {
        if (mctree->number_of_children(tmp.GetID()) > 1) log_fatal("wasn't expecting more than one child.");
        if (tmp.GetType() == I3Particle::Monopole) {
            speed_prof->push_back(tmp.GetSpeed() / I3Constants::c);
            tmp = mctree->first_child(tmp.GetID());
        }else{
            break;
        }
    }
    
    
    
//    I3MCTree::iterator iter = I3MCTreeUtils::GetMostEnergeticPrimary(*mctree);
//    speed_prof->clear();
//    while (iter.number_of_children()) {
//        if (iter.number_of_children() > 1) log_fatal("wasn't expecting more than one child.");
//        if (iter->GetType() == I3Particle::Monopole) {
//            speed_prof->push_back(iter->GetSpeed() / I3Constants::c);
//            iter = mctree->child(iter, 0);
//        }
//    }
}
