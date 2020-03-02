/**
 * I3MonopoleSlowUtils.cxx
 * Propagating slow monopoles using the Rubakov Callan Effect
 * @revision $Revision: 125103 $
 * @date $Date: 2014-10-27 10:25:33 -0500 (Mo, 27. Okt 2014) $
 *
 * @brief Propagate slow monopoles
*/

#include <dataclasses/I3Constants.h>
#include "I3MonopoleSlowUtils.h"


/**
 *Supervises the monopole propagation
 *@param frame instance of current frame
 *@param infoName_ name of info frame
 *@param mctree original mctree created by generator
 *@param prop_tree copy of mctree which you may adjust
 *@param random pointer to random number service
 *@param meanFreePath_ mean free path
 *@param useCorrectDecay_ Wheter to simulate 2 opposite cascades ( 2 * ~ 500 MeV) instead of 1 GeV
 *@param deltaEnergy_ Energy lost after meanFreePath? I am not sure (fhl)
 */

void
I3MonopoleSlowUtils::PropagateSlowMonopole(
        I3FramePtr frame,
        std::string infoName_,
        I3MCTreeConstPtr mctree,
        I3MCTreePtr prop_tree,
        // params
        I3RandomServicePtr random,
        double meanFreePath_,
        bool useCorrectDecay_,
        double deltaEnergy_) {
  for (auto const &mp:*prop_tree) {
    if (mp.GetType() != I3Particle::Monopole) continue;
    log_debug("Adding cascades to the monopole");

    const double X_MP(mp.GetX());
    const double Y_MP(mp.GetY());
    const double Z_MP(mp.GetZ());
    const double T_MP(mp.GetTime());

    const double THETA_MP = I3Constants::pi - mp.GetZenith();
    const double PHI_MP = mp.GetAzimuth() - I3Constants::pi;

    if (std::isnan(mp.GetLength())) {
      log_error(
              "The monopole has a length of NaN. I don't know where to put the cascades. I'M NOT PROPAGATING THIS!");
      // if this is not propagated, why not stop the function here via return? //fhl
    }

    int nint = static_cast<int>(random->Poisson(mp.GetLength() / meanFreePath_));

    for (int i(0); i < nint; ++i) {
      double l(random->Uniform(0, mp.GetLength()));
      double phi(random->Uniform(0, 2. * I3Constants::pi));
      double theta(acos(random->Uniform(-1., 1)));

      double x = X_MP + l * sin(THETA_MP) * cos(PHI_MP);
      double y = Y_MP + l * sin(THETA_MP) * sin(PHI_MP);
      double z = Z_MP + l * cos(THETA_MP);

      if (!useCorrectDecay_) {  // default (just one eplus carrying all energy. this gives a smaller MCTree and at the end the light yield is the same)
        I3Particle cascade;
        cascade.SetType(I3Particle::EPlus);
        cascade.SetLocationType(I3Particle::InIce);
        cascade.SetEnergy(deltaEnergy_);
        cascade.SetThetaPhi(theta, phi);
        cascade.SetPos(x, y, z);
        cascade.SetTime(T_MP + l / mp.GetSpeed());
        prop_tree->append_child(mp, cascade);
        log_debug("Added cascade with position: (%f,%f,%f) and energy %f GeV",
                  cascade.GetX(), cascade.GetY(), cascade.GetZ(),
                  cascade.GetEnergy() / I3Units::GeV);
      } else {   // eplus + pi_0, the most dominant proton decay channel
        I3Particle cascade;
        cascade.SetType(I3Particle::EPlus);
        cascade.SetLocationType(I3Particle::InIce);
        cascade.SetEnergy(460 * I3Units::MeV);
        cascade.SetThetaPhi(theta, phi);
        cascade.SetPos(x, y, z);
        cascade.SetTime(T_MP + l / mp.GetSpeed());
        prop_tree->append_child(mp, cascade);
        log_debug("Added cascade with position: (%f,%f,%f) and energy %f GeV",
                  cascade.GetX(), cascade.GetY(), cascade.GetZ(),
                  cascade.GetEnergy() / I3Units::GeV);

        I3Particle hadron;
        hadron.SetType(I3Particle::Pi0);   // pi_0 and eplus are treated equally with photonics
        hadron.SetLocationType(I3Particle::InIce);
        hadron.SetEnergy(480 * I3Units::MeV);
        double phi_corr = phi + I3Constants::pi;
        if (phi_corr > 2 * I3Constants::pi) {
          phi_corr = phi_corr - 2 * I3Constants::pi;
        }
        hadron.SetThetaPhi(I3Constants::pi - theta, phi_corr);
        hadron.SetPos(x, y, z);
        hadron.SetTime(T_MP + l / mp.GetSpeed());
        prop_tree->append_child(mp, hadron);
        log_debug("Added hadron with position: (%f,%f,%f) and energy %f GeV",
                  hadron.GetX(), hadron.GetY(), hadron.GetZ(),
                  hadron.GetEnergy() / I3Units::GeV);
      }
    }
  }
}


