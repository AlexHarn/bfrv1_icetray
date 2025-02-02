/**
 *   Copyright  (C) 2005
 *   The IceCube Collaboration
 *   $Id$
 *
 *   @file Particle.h
 *   @version $Revision$
 *   @date    $Date$ 
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *           modified by Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *
 *   @brief Particle header 
 *      particle class contains following information
 *      'in-addition to the I3Particle object',
 *
 *       - InteractionInfo
 *       - InteractionGeo
 *       - Step
 *       - detectionVolume (MuonGun::Surface class) 
 *       - Secondary particles
 *
 *      Note: all position information is stored in 
 *      that of center of detector system
 *      Direction is also stored respect to the center 
 *      of detector system.
 *   
 *      --- Comments on functions to access info and take into account the    
 *      coordinate system.
 *
 *      Interacted() function returns true if non-zero daughter
 *
 *      GetDaughterEnergySum() function returns the sum of all daughter energies
 *      if no daughters, returns zero, if non-zero, this essentially needs to 
 *      equal to the parent energy. 
 *
 *      GetEndPosition() returns constant of I3Particle
 *      which is (Start-)Position + (Direction*Length) if interacted/decayed
 *      if not interacted returns the (Start-)Position
 *      
 */
#ifndef NuGParticle_H
#define NuGParticle_H

#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Direction.h"
#include "dataclasses/I3Position.h"
#include "earthmodel-service/EarthModelCalculator.h"
#include "earthmodel-service/EarthModelService.h"
#include "neutrino-generator/InteractionInfo.h"
#include "neutrino-generator/InteractionGeo.h"
#include "neutrino-generator/Step.h"
#include "neutrino-generator/utils/Utils.h"
#include "phys-services/surfaces/SamplingSurface.h"


namespace nugen {

I3_FORWARD_DECLARATION(Steering); 

class Particle : public I3Particle {

 private :
  boost::shared_ptr<Steering>   steer_;
  InteractionInfo        intInfo_;
  InteractionGeo         intGeo_;
  std::vector<boost::shared_ptr<Particle> >   composite_list_;          
  DoubleStepMap          stepmap_;
  I3Surfaces::SamplingSurfacePtr detectionVolume_;

  /**
   * @brief boolean flag for if geom is calculated 
   */
  bool   isGeoCalculated_;
  bool   isStepCalculated_;

  void ClearFlags() { 
     isGeoCalculated_ = false;
     isStepCalculated_ = false;
  }

  /**
   * id for parent particle
   */
  I3ParticleID parent_id_; 

  /**
   * these ids are used for debug, do not use for analysis. 
   */
  int my_particle_id_;
  static int global_particle_id_;
  
 public:
  //------------------------------
  // constructor and destructor
  //------------------------------
  
  Particle(I3Particle::ParticleShape, I3Particle::ParticleType, 
           boost::shared_ptr<Steering> steer);

  Particle(const I3Particle &p, 
           boost::shared_ptr<Steering> steer);


 private:
  /**
   *private constructors
   */  
  Particle();

  Particle(const Particle& p);

  Particle& operator=(const Particle& p);

 public:

  //Default deconstructor
  virtual ~Particle() {}

  //get I3Particle
  I3Particle GetI3Particle() const;

  //------------------------------
  // overwrite functions
  //------------------------------

  inline void SetPos(const I3Position& p) {
                 this->I3Particle::SetPos(p); 
                 ClearFlags(); } 
  inline void SetPos(double p1, double p2, double p3,
                     I3Position::RefFrame frame) {
                 this->I3Particle::SetPos(p1, p2, p3, frame);
                 ClearFlags(); } 
  inline void SetPos(double x, double y, double z) { 
                 this->I3Particle::SetPos(x,y,z); 
                 ClearFlags(); } 

  inline void SetDir(const I3Direction& d) {
                 this->I3Particle::SetDir(d);
                 ClearFlags(); } 
  inline void SetDir(double zen, double azi) { 
                 this->I3Particle::SetDir(zen,azi);
                 ClearFlags(); } 
  inline void SetDir(double x, double y, double z) {
                 this->I3Particle::SetDir(x,y,z); 
                 ClearFlags(); } 
  inline void SetThetaPhi(double theta, double phi) { 
                 this->I3Particle::SetThetaPhi(theta,phi);
                 ClearFlags(); } 
  inline void SetEnergy(double energy) { 
                 this->I3Particle::SetEnergy(energy);
                 ClearFlags(); } 

  void SetLocationType(I3Particle::LocationType l);

  /**
   * check particle types
   */
   inline const bool IsElectron() const 
      { return Utils::IsElectron(GetType()); }
   inline const bool IsMuon() const 
      { return Utils::IsMuon(GetType()); }
   inline const bool IsTau() const 
      { return Utils::IsTau(GetType()); }
   inline const bool IsNuE() const 
      { return Utils::IsNuE(GetType()); }
   inline const bool IsNuMu() const 
      { return Utils::IsNuMu(GetType()); }
   inline const bool IsNuTau() const 
      { return Utils::IsNuTau(GetType()); }

   inline const bool IsEFlavor() const 
      { return Utils::IsEFlavor(GetType()); }
   inline const bool IsMuFlavor() const 
      { return Utils::IsMuFlavor(GetType()); }
   inline const bool IsTauFlavor() const 
      { return Utils::IsTauFlavor(GetType()); }

  //------------------------------
  // Setters
  //------------------------------

  /**
   * @brief this function copies extra info to this particle.
   */
  void CopyExtraInfoFrom(const Particle& p) ;
  void SetPropagationWeight(double w) { intInfo_.propweight_ = w; }

  void SetImpactParam(double x) { intGeo_.impactParam_ = x; }

  /**
   * @brief set default detection surface.
   * This surface is typically a cylinder placed surround IceCube,
   * with a fixed coordinate which does not rotate along
   * with incoming tracks.
   * The surface is originally given by Steering class,
   * but you may change it by particle.
   */

  void SetDetectionVolume(I3Surfaces::SamplingSurfacePtr &s)
                           { detectionVolume_ = s;
                             ClearFlags(); }

  //------------------------------
  // getters
  //------------------------------

  boost::shared_ptr<Steering>  GetSteer() {return steer_; }
  
  double GetPropagationWeight() { return intInfo_.propweight_; }

  /**
   * @return a non-const reference to InteractionInfo
   */
  inline InteractionInfo& GetInteractionInfoRef() { return intInfo_; }
  inline const InteractionInfo& GetInteractionInfoRef() const { return intInfo_; }

  /**
   * @return a non-const reference to InteractionGeo
   */
  InteractionGeo& GetInteractionGeoRef() ;
  inline const InteractionGeo GetInteractionGeo() const { return intGeo_; }

  /**
   * @return reference to a const stepmap
   */
  const DoubleStepMap& GetStepMapRef() ;

  /**
   * @return end of propagation position
   * @brief Returns end position which is defined  
   * if(non-zero Length) Position + Direction*Length
   * else if Length==0 EndPosition = Position
   */    
  const I3Position GetEndPosition();

  /**
   * @return vector of daughter particles
   */
  const std::vector<boost::shared_ptr<Particle> >& GetDaughters(){return composite_list_;};

  /**
   * add daughter particle
   * it also set parent id copy location type.
   */
   void AddDaughter(boost::shared_ptr<Particle> p);

  /**
   *@return the energy sum of next level daughters 
   *        should be equal to this particles energy
   */
  const double GetTotalDaughterEnergies();//defined 

  /**
   *@return total number of daughter particles
   */
  const int    GetTotalNDaughters(){return GetDaughters().size();};//

  /**
   * calculate muon range [water mater equiv.].
   */
  const double GetMuonRange() const;

  /**
   * calculate muon range [water mater equiv.] and convert it in 
   * [meter] with given track geometry. endposDC will be the end
   * point of the range.
   */
  const double GetMuonRangeInMeterTo(const I3Position &endposDC) const;

  /**
   * calculate muon range [water mater equiv.] and convert it in 
   * [meter] with given track geometry. posDC will be the start
   * position of the range.
   */
  const double GetMuonRangeInMeterFrom(const I3Position &posDC) const;

  /**
   * is interaction geom calculated?
   */
  const bool IsGeoCalculated() { return isGeoCalculated_; }

  /**
   * This returns NuGen INTERNAL ID.
   * It is NOT the particle's MajorID or MinorID.
   */
  const int GetMyID() const {return my_particle_id_; }

  /**
   * This returns parent's ID.
   */
  const I3ParticleID GetParentID() const {return parent_id_; }

  /**
   *  debug prints
   */
  std::string PrintID();
  void PrintDaughters(const std::string & indent);

  //------------------------------
  // Utilities
  //------------------------------

  /**
   * @brief check daughter particles
   */
  void CheckDaughters();

  /**
   * Print out all about this particle
   */
  void  CheckParticle();//
 
  /**
   * @return bool if this has interacted or decay 
   */
  bool InteractedOrDecayed();

 private:
  
  /**
   * calculate interaction geometry for this particle
   * need to be re-calculate when any of position,direction and
   * energy has been changed.
   */
  void CalculateInteractionGeometryLegacy();
  void CalculateInteractionGeometry();

  /**
   * @brief perform stepping calculation for final interaction 
   * need to be re-calculate when any of position,direction and
   * energy has been changed.
   * @param[in] map of interaction type and its weight factor
   */
  void Stepping();
  void SteppingSimple();
  void SteppingBruteforce();

  /**
   * @brief returns medium type and density for given position
   */
  void GetMaterial(const I3Position &posDC,
                 earthmodel::EarthModelServiceConstPtr earth,
                 earthmodel::EarthModelService::MediumType &med,
                 double &density);

  /**
   * convert NuG MuRangeOpt to inputs of EarthModelCalculator
   */
  void MuRangeOptConverter(double murangeopt,
                        earthmodel::EarthModelCalculator::LeptonRangeOption &opt,
                        double &scale) const ;


  void SetParentID(I3ParticleID p) {parent_id_ = p;}

  SET_LOGGER("I3NuG");

};

I3_POINTER_TYPEDEFS(Particle);
typedef I3Vector<ParticlePtr>  ParticlePtrList;

}
#endif //I3NuGPARTICLE_H
