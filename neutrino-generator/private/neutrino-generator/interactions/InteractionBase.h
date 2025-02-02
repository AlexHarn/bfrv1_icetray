#ifndef NuGINTERACTIONBase_H
#define NuGINTERACTIONBase_H
/**
 *   copyright  (C) 2005
 *   the IceCube Collaboration
 *   $Id:  $
 *
 *   @version $Revision: $
 *   @date $Date: $
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 *   @brief Interaction header file for I3NeutrinoGenerator
 */
//////////////////////////////////////////////////////////////////////

#include "dataclasses/I3Direction.h"
#include "dataclasses/physics/I3Particle.h"
#include "phys-services/I3RandomService.h"
#include "neutrino-generator/utils/EnumTypeDefs.h"

#include <map>

namespace nugen {
 
I3_FORWARD_DECLARATION(Steering);
I3_FORWARD_DECLARATION(Particle);
I3_FORWARD_DECLARATION(CrosssectionTableReader);
I3_FORWARD_DECLARATION(FinalStateTableReader);

class InteractionBase {

 protected:
  I3RandomServicePtr  random_;
  boost::shared_ptr<Steering> steer_;
  boost::shared_ptr<CrosssectionTableReader> cross_file_ptr_;
  boost::shared_ptr<FinalStateTableReader>   final_file_ptr_;
  std::string sigmaname_;
  std::string finalname_;
  unsigned int flavormask_;
  unsigned int materialmask_;

  // default constructor
  InteractionBase() {}


 public:
   InteractionBase(I3RandomServicePtr random, 
                   boost::shared_ptr<Steering> steer);
   virtual ~InteractionBase();

   /**	
    * pure virtual function	
    */
   virtual void FillDaughterParticles(
                  boost::shared_ptr<Particle> particle_ptr, 
                  double energy = 0) = 0;
   virtual InteractionType GetInteractionType() = 0;

   virtual void InitializeCrosssectionTable(const std::string& sigmafile);
   virtual void InitializeFinalStateTable(const std::string& finalfile);

   virtual double GetMinEnergy(); 
   virtual double GetMaxEnergy();

   /**
    @brief returns cross section of this interaction in [cm^2]
    */
   virtual double GetXsecCGS (const double energy);

   std::string&  GetCrosssectionFileName() {return sigmaname_;};
   std::string&  GetFinalStateFileName()   {return finalname_;};
   unsigned int  GetActiveFlavorMask()     {return flavormask_;};
   unsigned int  GetActiveMaterialMask()   {return materialmask_;};

   void SetActiveFlavorMask(unsigned int mask)   {flavormask_ = mask;};
   void SetActiveMaterialMask(unsigned int mask) {materialmask_ = mask;};
        
  /**
   * @return outgoinc lepton direction
   */
   I3Direction GetLeptonDirection(const I3Direction& direction, 
                                  const double lepton_energy, 
                                  const double cos_theta);

   /**
    * @return vector<double> = (Bjorken X,  Bjorken Y)
    */
   virtual std::vector<double> SelectXY(double log_e, I3Particle::ParticleType ptype);

   /**
    *@brief wrapper function to set secondaries.
    *@param[in] particle pointer
    *@param[in] outgoing lepton type 
    *@param[in] flag to skip outgoing angle calculation
    */
   void SetSecondaryParticles(boost::shared_ptr<Particle> nuin_ptr,
                        I3Particle::ParticleType out_ptype,
                        bool skipCalcCosTheta = false);

   /**
    *@brief old function to set secondary particles
    * this function ignores lepton mass, and direction
    * of secondary hadron is same as parent neutrino.
    *@param[in] particle pointer
    *@param[in] outgoing lepton type 
    *@param[in] flag to skip outgoing angle calculation
    */
   void SetSecondaryLepton(boost::shared_ptr<Particle> nuin_ptr,
                        I3Particle::ParticleType out_ptype,
                        bool skipCalcCosTheta = false);

   /**
    *@brief old function to calculate lepton cos(angle)
    * this function ignores lepton mass 
    *@param[in] neutrino total energy
    *@param[in] Bjorken x
    *@param[in] Bjorken y
    */
   double CalcOutgoingCosThetaSimple(double ene,
                                     double x, double y) ;

   /**
    *@brief new function to set secondary particles
    * this function takes into account of lepton mass
    * and calculate outgoing hadron angle.
    *@param[in] particle pointer
    *@param[in] outgoing lepton type 
    *@param[in] flag to skip outgoing angle calculation
    */
   void SetSecondaries(boost::shared_ptr<Particle> nuin_ptr,
                        I3Particle::ParticleType out_ptype,
                        bool skipCalcCosTheta = false);

   /**
    *@brief utility function to calculate kinetic energy
    * and particle speed
    *@param[in] total energy of the particle 
    *@param[in] rest mass of the particle
    *@param[out] kinetic energy of the particle
    *@param[out] speed of the particle
    */
   void CalcKineticEnergyAndSpeed(
             double total_ene,
             double rest_mass,
             double &kinetic_ene,
             double &speed);

   /**
    *@brief utility function to get Bjorken X,Y value
    *@param[in] parent neutrino total energy
    *@param[in] mass of hit target
    *@param[in] mass of daughter lepton
    *@param[in] type of daughter lepton
    *@param[out] Bjorken x
    *@param[out] Bjorken y
    *@param[in] counter to save how many retry was happened (for debug)
    */
   bool GetGoodXY(
                 double ene, // neutrino energy
                 double target_mass, // in GeV
                 double lepton_mass, // out lepton mass
                 I3Particle::ParticleType out_ptype,
                 double &x, double &y,
                 int ntrials = 0);

   /**
    *@brief new function to calculate lepton angle
    * this function takes into account of lepton mass
    *@param[in] parent neutrino total energy
    *@param[in] type of daughter lepton
    *@param[out] Bjorken x
    *@param[out] Bjorken y
    *@param[out] daughter lepton scatter angle
    *@param[out] daughter hadron scatter angle
    */
   bool CalcOutgoingTheta(double ene, 
                   I3Particle::ParticleType out_ptype,
                   double &x, double &y,
                   double &lep_theta,
                   double &had_theta);


   SET_LOGGER("I3NuG");
};

I3_POINTER_TYPEDEFS(InteractionBase);
typedef std::pair<double, InteractionBasePtr> DoubleInteractionPair;
typedef std::map<double, InteractionBasePtr> DoubleInteractionMap;

}
#endif //I3NuGINTERACTION_H



