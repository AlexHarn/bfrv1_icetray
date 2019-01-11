/**
 *   copyright  (C) 2005
 *   the IceCube Collaboration
 *   $Id:  $
 *   @version $Revision: $
 *   @date    $Date:     $
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 *   @brief Interaction implimentation file
 *
 */
//////////////////////////////////////////////////////////////////////

#include "neutrino-generator/Steering.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/interactions/InteractionBase.h"
#include "neutrino-generator/table-interface/CrosssectionTableReader.h"
#include "neutrino-generator/table-interface/FinalStateTableReader.h"
#include "neutrino-generator/utils/Constants.h"
#include "neutrino-generator/utils/Calculator.h"
#include "icetray/I3Units.h"
#include "dataclasses/I3Constants.h"

using namespace std;
using namespace earthmodel;

namespace nugen {

static const double ETHRES = 0.2 * I3Units::GeV;
static const double COSTHRES = 1e-5;

//____________________________________________________________________
InteractionBase::InteractionBase(I3RandomServicePtr random,
                                 SteeringPtr steer)
       :random_(random), steer_(steer)
{
  cross_file_ptr_ = CrosssectionTableReaderPtr(new CrosssectionTableReader());
  final_file_ptr_ = FinalStateTableReaderPtr(new FinalStateTableReader());
  sigmaname_ = "";
  finalname_ = "";
}

//____________________________________________________________________
InteractionBase::~InteractionBase()
{ 
  log_trace("Interaction deconstructed");
}

//____________________________________________________________________
void InteractionBase::InitializeCrosssectionTable(const string& sigmafile)
{
  sigmaname_ = sigmafile;
  cross_file_ptr_->ReadCrosssectionTableFillArray(sigmafile);
}

//____________________________________________________________________
void InteractionBase::InitializeFinalStateTable(const string& finalfile)
{  
  finalname_ = finalfile;
  final_file_ptr_->ReadFinalStateTableFillArray(finalfile);
}

//____________________________________________________________________
double InteractionBase::GetXsecCGS(const double energy) 
{
    double log_e = log10(energy);  
    double sigma = cross_file_ptr_->EvaluateCrosssection(log_e);
    //log_trace("Interaction type %s, Sigma for log(E[GeV])=%f is %f * 10 ^-12 [mb]", 
    //           GetInteractionTypeString().c_str(), log_e, sigma*1.0e12);

    // cross section files (csms, cteq) are in [mb]. 
    // conversion : mb to cm2
    return sigma * 1e-27;
}

//____________________________________________________________________
double InteractionBase::GetMinEnergy()
{
   return cross_file_ptr_->GetMinEnergy();
}

//____________________________________________________________________
double InteractionBase::GetMaxEnergy()
{
   return cross_file_ptr_->GetMaxEnergy();
}

//___________________________________________________________________
I3Direction InteractionBase::GetLeptonDirection(const I3Direction& direction, 
                                            const double lepton_energy, 
                                            const double cos_theta) {
  // This function is used when Steer::useSimpleScatterForm_ is true.
  //
  // Build an arbitrary dir axil, perpendicular
  // to the neutrino 3-momentum and assume it is {1,0,z};
  // as axil*nu_p = 0, px*1+py*0+pz*z=0, => z = -px/pz
  //

  // for following calculation I3Position is more convenient than
  // I3Direction, so let's use I3Position.
  I3Position axil(0.0, 0.0, 0.0);
  I3Position vector_out_direction(0.0, 0.0, 0.0);

  if (direction.GetZ() != 0.0){
    axil.SetX(1.0);
    axil.SetY(0.0);
    axil.SetZ((-direction).GetX()/direction.GetZ());
  } else {
    axil.SetX(1.0);
  }

  // throw an arbitrary azimuth angle
  double phi = random_->Uniform( 0.0, 2.0*I3Constants::pi );  
  I3Direction vector_in_direction(direction);
  // the rotated transversal unitary vector
  I3Direction axil_dir(Calculator::Rotate(axil, phi,
                                    vector_in_direction));

  double sin_theta = sqrt((1 + cos_theta) * (1 - cos_theta));

  // transversal  component of muon 3-momentum
  I3Position mu_trans = lepton_energy * sin_theta * axil_dir;
  // longitudinal component of muon 3-momentum
  I3Position mu_long  = lepton_energy * cos_theta * vector_in_direction;   
  vector_out_direction = mu_long + mu_trans;

  I3Direction in_dir(vector_in_direction);
  I3Direction out_dir(vector_out_direction);

  log_trace("incoming neu  direction (theta, phi)=(%f,%f)", 
                   in_dir.CalcTheta(), in_dir.CalcPhi());
  log_trace("outgoing muon direction (theta, phi)=(%f,%f)", 
                   out_dir.CalcTheta(), out_dir.CalcPhi());

  return out_dir;

}

//___________________________________________________________________
std::vector<double> InteractionBase::SelectXY(double log_e, I3Particle::ParticleType) 
{
  // make final states of the produced particles
  // if muon, lepton moves in a different direction  
  double finalxy[2]={0.0, 0.0};

  //fill final stage x and y to finalxy
  final_file_ptr_->SampleFinalState(log_e, random_->Uniform(0.0, 1.0), 
                                   random_->Uniform(0.0, 1.0), finalxy);
  
  // checks are necessary in case tables contain buggy numbers
  while (finalxy[1]<0||finalxy[1]>1||finalxy[0]<0||finalxy[0]>1) {
     log_warn("NC interaction: x=%f or y=%f is out of range, retry", 
               finalxy[0], finalxy[1]);
     final_file_ptr_->SampleFinalState(log_e, 
                                   random_->Uniform(0.0, 1.0), 
                                   random_->Uniform(0.0, 1.0), 
                                   finalxy);
  }

  std::vector<double> result;
  result.push_back(finalxy[0]);
  result.push_back(finalxy[1]);
  log_trace("final state x and y = (%f, %f)", result[0], result[1]);
  return result;
}

//___________________________________________________________________
void InteractionBase::SetSecondaryParticles(ParticlePtr nuin_ptr,
                        I3Particle::ParticleType out_ptype,
                        bool skipCalcCosTheta)
{

  if (steer_->UseSimpleScatterForm()) {
     // this is old function.
     // hadron direction is always same as 
     // parent neutrino.
     // I keep it for backward check.
     SetSecondaryLepton(nuin_ptr, out_ptype, skipCalcCosTheta);

  } else {
     // new function that calculates hadron's 
     // outgoing angle.
     // This function also takes into account of
     // lepton particle mass.
     SetSecondaries(nuin_ptr, out_ptype, skipCalcCosTheta);
  }
} 

//___________________________________________________________________
void InteractionBase::SetSecondaryLepton(ParticlePtr nuin_ptr,
                        I3Particle::ParticleType out_ptype,
                        bool skipCalcCosTheta)
{
  // this function is used when 
  // steer_->UseSimpleScatterForm() is true.
  // it ignores the mass of secondary lepton 
  // (high-energy apploximation)

  double lepton_mass = Constants::GetMass(out_ptype);
  double nu_energy = nuin_ptr->GetEnergy();
  if (std::isnan(nu_energy) || nu_energy <= 0) {
     log_fatal("Invalid neutrino energy %f", nu_energy);
  }

  // select a set of x, y
  const double log_e = log10(nu_energy/I3Units::GeV);
  std::vector<double> xy = SelectXY(log_e, out_ptype);
  double x = xy[0];
  double y = xy[1];
  InteractionInfo& intinfo = nuin_ptr->GetInteractionInfoRef();
  intinfo.SetBjorkenX(x);
  intinfo.SetBjorkenY(y);

  // set outgoing lepton energy
  double out_lepton_E = (1 - y) * nu_energy;

  // get original direction
  I3Direction direction = nuin_ptr->GetDir();

  ParticlePtr lepton_daughter_particle(new Particle(I3Particle::Null,
                                       I3Particle::unknown, steer_));
  lepton_daughter_particle->CopyExtraInfoFrom(*nuin_ptr);

  if (out_lepton_E <= lepton_mass) {

     // secondary lepton has no energy.
     // create a rest lepton

     out_lepton_E = lepton_mass;


  } else {

     if (!skipCalcCosTheta &&
         (Utils::IsMuFlavor(out_ptype) || 
          Utils::IsTauFlavor(out_ptype) || 
          Utils::IsNuE(out_ptype))) {

        double outgoing_costheta = 1.0;

        outgoing_costheta = CalcOutgoingCosThetaSimple(
                               nu_energy, x, y);

        // sanity check!
        // sometimes outgoing energy is too small and 
        // the outgoing_costheta is out of range.
        // If outgoing energy is less than 0.2GeV (which muon range is 1.2m),
        // we may be able to ignore the direction, then assign random costheta.
        // If the outgoing energy exceeds 0.2GeV, let's try to get
        // another sample.
        // this is dirty fix, check your setup if you see too much
        // errors.


        if (fabs(outgoing_costheta) > 1.0) {
           log_notice("Outgoing Muon direction (x = %f, y = %f, nu_energy = %f, out_lepton_E = %f, cos_theta = %f) "
                   " cos_theta is out of range.",
                   x, y, nuin_ptr->GetEnergy(), out_lepton_E,
                   outgoing_costheta);
           if (out_lepton_E < ETHRES) {
              outgoing_costheta = random_->Uniform(-1.0, 1.0);
              log_notice("because outgoing lepton energy is less than %f GeV, random costheta = %f is assigned) "
                        , ETHRES, outgoing_costheta);

           } else {
              log_warn("outgoing energy is more than %f GeV. Dirty Fix : "
                        "retry to get new x and y", ETHRES);
              return SetSecondaryLepton(nuin_ptr, out_ptype, skipCalcCosTheta);
           }
        }

        // update direction
        direction = GetLeptonDirection(direction, out_lepton_E, 
                                       outgoing_costheta);
        log_debug("Outgoing angle is %f deg", acos(outgoing_costheta)/I3Units::degree);

     } //else direction for the secondary is the same as parent neutrino
  }
      
  lepton_daughter_particle->SetFitStatus(I3Particle::NotSet);
  lepton_daughter_particle->SetPos(nuin_ptr->GetEndPosition());
  double daughter_time = nuin_ptr->GetTime()+nuin_ptr->GetLength()/nuin_ptr->GetSpeed();
  lepton_daughter_particle->SetTime(daughter_time);//7
  lepton_daughter_particle->SetDir(direction);//8
  lepton_daughter_particle->SetType(out_ptype);
  lepton_daughter_particle->SetEnergy(out_lepton_E);
  lepton_daughter_particle->SetLength(NAN);//9
  lepton_daughter_particle->SetSpeed(I3Constants::c);//10
  if (Utils::IsTau(out_ptype)) {
      // no additional setting?
  }

  if (Utils::IsElectron(out_ptype)) {
     lepton_daughter_particle->SetShape(I3Particle::Cascade);

  } else {
     // since we set length as NAN (length of mu/tau will be 
     // treated by mmc!),
     // we set shape as Null.
     lepton_daughter_particle->SetShape(I3Particle::Null);
  }

  nuin_ptr->AddDaughter(lepton_daughter_particle);

  //create hadron  
  double had_energy = nuin_ptr->GetEnergy() - out_lepton_E;

  ParticlePtr hadron_daughter_particle(new Particle(I3Particle::Null,
                                           I3Particle::unknown, steer_));
  hadron_daughter_particle->SetShape(I3Particle::Cascade);//4
  hadron_daughter_particle->SetFitStatus(I3Particle::NotSet);//5
  hadron_daughter_particle->SetPos(nuin_ptr->GetEndPosition());//6
  hadron_daughter_particle->SetTime(daughter_time);//7
  hadron_daughter_particle->SetDir(nuin_ptr->GetDir());//8
  hadron_daughter_particle->SetLength(NAN);//9
  hadron_daughter_particle->SetSpeed(I3Constants::c);//10
  hadron_daughter_particle->SetType(I3Particle::Hadrons);//11
  hadron_daughter_particle->SetEnergy(had_energy);//12
  hadron_daughter_particle->CopyExtraInfoFrom(*nuin_ptr);

  nuin_ptr->AddDaughter(hadron_daughter_particle);

}

//___________________________________________________________________
double InteractionBase::CalcOutgoingCosThetaSimple(
                        double ene,
                        double x, double y) 
{
  //--------------------
  // original equation :
  //--------------------
   //double outgoing_costheta = 1 - (x*y*M_N/nuin_ptr->GetEnergy()/(1. - y));
  // M_N is nucleon mass

  double out_lepton_E = (1 - y) * ene;
  double costheta = 1.0 - (x*y*Constants::M_N/out_lepton_E);
  return costheta;
}

//___________________________________________________________________
void InteractionBase::SetSecondaries(ParticlePtr nuin_ptr,
                        I3Particle::ParticleType out_ptype,
                        bool skipCalcCosTheta)
{
  // this function is used when 
  // steer_->UseSimpleScatterForm() is false.
  // it takes into accout of mass of secondary lepton.
  //
  // First, calculate outgoing theta angles
  double x = -1;  // Bjorken x
  double y = -1;  // Bjorken y
  double lep_theta = -2;  // outgoing lepton scatter angle
  double had_theta = -2;  // outgoing hadron scatter angle
  
  double nu_ene = nuin_ptr->GetEnergy();

  if (skipCalcCosTheta) {
     // This neutrino have not reached to detector,
     // and because skipCalcCosTheta is true, 
     // we ignore scattering angles.
     // Assume secondary lepton and hadron have 
     // the same direction as parent neutrino.
     lep_theta = 0;
     had_theta = 0; 
     double m_n = Constants::M_N; 
     double m_l = I3Particle::GetMassForType(out_ptype);
     GetGoodXY(nu_ene, m_n, m_l, out_ptype, x, y);

  } else {
     // calclate scattering angles of lepton and hadron
     bool success = CalcOutgoingTheta(nu_ene, 
                                 out_ptype, x, y, 
                                 lep_theta, had_theta); 
     int samplecount = 0;
     while (!success) {
        success = CalcOutgoingTheta(nu_ene, out_ptype, 
                                 x, y, 
                                 lep_theta, had_theta); 
        samplecount++;
        if (samplecount > 1000) {
           log_error("failed 1000 trials of CalcOutgoingTheta, check your data and program.");
        }
     }
  }

  // Set selected Bjorken X and Y to InteractionInfo
  InteractionInfo& intinfo = nuin_ptr->GetInteractionInfoRef();
  intinfo.SetBjorkenX(x);
  intinfo.SetBjorkenY(y);

  // Calculate directions. 
  // Because now I3Direction has utility functions, 
  // use them instead of Calculator::Rotate functions.

  double lep_phi = random_->Uniform(0.0, 2.0*I3Constants::pi);  
  double had_phi = lep_phi + (lep_phi < I3Constants::pi ? 1:-1)*I3Constants::pi;

  I3Direction nu_dir = nuin_ptr->GetDir();

  I3Direction lep_dir(1,1); // dummy
  lep_dir.SetThetaPhi(lep_theta, lep_phi);
  lep_dir.RotateY(nu_dir.CalcTheta());
  lep_dir.RotateZ(nu_dir.CalcPhi());
  log_trace("lep_dir theta %f, phi %f, nu_dir theta %f, phi %f, rotated lep_dir %f, phi %f", lep_theta, lep_phi, nu_dir.CalcTheta(), nu_dir.CalcPhi(), lep_dir.CalcTheta(), lep_dir.CalcPhi());

  I3Direction had_dir(1,1); // dummy
  had_dir.SetThetaPhi(had_theta, had_phi);
  had_dir.RotateY(nu_dir.CalcTheta());
  had_dir.RotateZ(nu_dir.CalcPhi());

  // Calculate lepton energy and speed. 

  double ene_l = nu_ene * (1.0 - y);
  double m_l = I3Particle::GetMassForType(out_ptype);
  double kene_l = 0;
  double speed_l = 0;
  CalcKineticEnergyAndSpeed(ene_l, m_l, kene_l, speed_l);
  if (kene_l > nu_ene) {
     log_fatal("secondary energy is greater than primary! nu_ene %g, lepton_ene %g", nu_ene, kene_l);
  }

  // Add secondary lepton.
  double daughter_time = nuin_ptr->GetTime()+nuin_ptr->GetLength()/nuin_ptr->GetSpeed();

  ParticlePtr lepton_daughter_particle(new Particle(I3Particle::Null,
                                       I3Particle::unknown, steer_));
  lepton_daughter_particle->CopyExtraInfoFrom(*nuin_ptr);
  lepton_daughter_particle->SetFitStatus(I3Particle::NotSet);
  lepton_daughter_particle->SetPos(nuin_ptr->GetEndPosition());
  lepton_daughter_particle->SetTime(daughter_time);
  lepton_daughter_particle->SetDir(lep_dir);
  lepton_daughter_particle->SetType(out_ptype);
  lepton_daughter_particle->SetEnergy(kene_l);//kinetic E
  lepton_daughter_particle->SetLength(NAN);
  lepton_daughter_particle->SetSpeed(speed_l);

  if (Utils::IsElectron(out_ptype)) {
     lepton_daughter_particle->SetShape(I3Particle::Cascade);

  } else {
     // since we set length as NAN (length of mu/tau will be 
     // treated by mmc!),
     // we set shape as Null.
     lepton_daughter_particle->SetShape(I3Particle::Null);
  }

  nuin_ptr->AddDaughter(lepton_daughter_particle);

  // Add hadron.
  // For hadron we ignore particle mass and speed, because
  // anyway cmc will recalculate it.

  double had_energy = nu_ene*y;

  ParticlePtr hadron_daughter_particle(new Particle(I3Particle::Null,
                                           I3Particle::unknown, steer_));
  hadron_daughter_particle->CopyExtraInfoFrom(*nuin_ptr);
  hadron_daughter_particle->SetEnergy(had_energy);
  hadron_daughter_particle->SetShape(I3Particle::Cascade);
  hadron_daughter_particle->SetFitStatus(I3Particle::NotSet);
  hadron_daughter_particle->SetPos(nuin_ptr->GetEndPosition());
  hadron_daughter_particle->SetTime(daughter_time);
  hadron_daughter_particle->SetDir(had_dir);
  hadron_daughter_particle->SetLength(NAN);
  hadron_daughter_particle->SetSpeed(NAN);
  hadron_daughter_particle->SetType(I3Particle::Hadrons);

  nuin_ptr->AddDaughter(hadron_daughter_particle);

}

//___________________________________________________________________
void InteractionBase::CalcKineticEnergyAndSpeed(
             double total_ene,
             double rest_mass,
             double &kinetic_ene,
             double &speed)
{
   // kinetic energy in GeV
   kinetic_ene = sqrt((total_ene + rest_mass)*(total_ene - rest_mass));

   // calc speed in icecube unit
   // gamma = E/m (in nature unit)
   // beta = sqrt(1 - (1/gamma)^2) (unitless) 
   double one_over_gamma = rest_mass/total_ene;
   double beta = sqrt((1+one_over_gamma)*(1-one_over_gamma));
   speed = beta*I3Constants::c; // in icecube unit

   if (std::isnan(speed)) {
      log_error("speed is nan, total energy = %e[GeV], rest_mass %e[GeV], kinetic energy %e[GeV], diff_e %e[GeV], 1/gamma %e, beta %e, speed %e[m/s]", total_ene, rest_mass, kinetic_ene, total_ene - kinetic_ene, one_over_gamma, beta,speed/(I3Units::m/I3Units::s));
   }

}



//___________________________________________________________________
bool InteractionBase::GetGoodXY(
                 double ene, // neutrino energy
                 double target_mass, // in GeV
                 double lepton_mass, // out lepton mass
                 I3Particle::ParticleType out_ptype,
                 double &x, double &y,
                 int ntrials) // for debug
{
  // 
  // coded by Askhat Gazizov (see resources/scripts/angles_dis.py)
  //

  double m2 = lepton_mass * lepton_mass;
  double ene2 = ene * ene; // square of neutrino E
  double m_loverE = lepton_mass / ene;
  double log10_e = log10(ene/I3Units::GeV);
  double m_n = target_mass;
  
  std::vector<double> xy = SelectXY(log10_e, out_ptype);
  x = xy[0];
  y = xy[1];
  
  double y1  = 1 - y;
  double xmin = m2 / (2 * m_n * (ene - lepton_mass));

  while (x < xmin) {
     ++ntrials;
     log_notice("retry %d, x(%g) < x_min(%g)",ntrials, x, xmin);
     GetGoodXY(ene, target_mass, lepton_mass, 
               out_ptype, x, y, ntrials);
  }

  // upper branch of Y(x) due to mass of lepton
  double a =  (1.0 - m2*(1.0/(2.0*m_n*ene*x) + 0.5/ene2)) / 
              (2 + m_n * x / ene);

  // calculate limits of y(yl, yu)
  double s1 = 1.0 - m2/(2.0*m_n*ene*x) - m_loverE;
  if (s1 < 0.) s1 = 0.0;
  double s2 = 1.0 - m2/(2.0*m_n*ene*x) + m_loverE;
  double b =  sqrt(s1)*sqrt(s2)/(2.0 + m_n*x/ene);
  double yl = a - b;
  double yu = a + b;

  // check y range
  if ((y-yl)*(y-yu) > 0) {
     ++ntrials;
     log_notice("retry %d, y=%g must be in range [%g,%g]. log10E = %g",
                ntrials, y, yl, yu, log10_e);
     GetGoodXY(ene, target_mass, lepton_mass, 
               out_ptype, x, y, ntrials);
  }

  // x and y are updated to good values.
  return true;

}

//___________________________________________________________________
bool InteractionBase::CalcOutgoingTheta(
                   double ene, 
                   I3Particle::ParticleType out_ptype,
                   double &x, double &y,
                   double &lep_theta,
                   double &had_theta)
{
  //---------------------------------------------
  // equation for outgoing lepton angle(costheta) 
  //---------------------------------------------
  // see script by Askhat Gazizov (resources/scripts/angles_dis.py)
  //

  // get reasonable Bjorken X and Y
  // m_n : target mass of nucleon in GeV
  double m_n = Constants::M_N; 
  double m_l = I3Particle::GetMassForType(out_ptype);
  GetGoodXY(ene, m_n, m_l, out_ptype, x, y); 

  //-----------------------
  // calculate costheta
  //-----------------------
  // here is Gazizov's original code
  //
  //double y1  = 1 - y;
  //double m_loverE = lepton_mass / ene;
  //double kappa = m_loverE * m_loverE;
  //double beta_l = sqrt(1 - kappa / (y1*y1));
  //lep_costheta = (1 - (m_n*x*y/ene + kappa/2)/y1)/beta_l;
  //
  // This equation can be modified to
  // costheta = 
  //  (E_l - m_n*x*y - m_l**2/(2*ene)) / sqrt(E_l**2 - m_l**2)
  // where E_l = ene*(1-y)
  //
  // same equation is used in LeptonInjector.
  // 

  // lepton energy and mass
  double ene_l = ene * (1.0 - y);

  // lepton kinetic energy
  double kene_lep2 = (ene_l + m_l)*(ene_l - m_l);
  double kene_lep = sqrt(kene_lep2);

  double lep_costheta = 
      (ene_l - m_n*x*y - m_l*m_l / (2*ene)) / kene_lep;

  if (fabs(lep_costheta) > 1.0) {
     log_notice("costheta overflow. nu_energy=%e, lepton_energy=%e, cos_theta=%e", ene, ene_l, lep_costheta);

     if (fabs(lep_costheta) - 1 < COSTHRES) {
        log_notice("maybe roundoff. set costheta 1.");
        lep_costheta = 1.0;
     } else if (ene_l < ETHRES) {
        log_notice("low energy. anyway we won't have a good directional resolution. set costheta 1.");
        lep_costheta = 1.0;

     } else {
        log_warn("something strange happened. return false");
        return false;
     }         
  }

  // calc hadron costheta.
  // longituldinal momentum
  double p_long = ene - kene_lep * lep_costheta;
  lep_theta = acos(lep_costheta);
  // transverse momentum 
  double p_trans = kene_lep * sin(lep_theta);

  if (p_long == 0) {
     had_theta = 0.5 * I3Constants::pi;
  } else {
     had_theta = atan(p_trans/p_long);
  }
  log_debug("lepton_theta %e deg, hadron_theta %e deg", lep_theta/I3Units::degree, had_theta/I3Units::degree);

  return true;

}

}
