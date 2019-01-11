/*
 * Follow similar structure as muon-bundle-reco
 */

#include "stochastics/I3Stochastics.h"
#include <vector>

// The class variables I need :
std::vector<energyloss> I3Stochastics::energyLossData_;  // this now contains E,X, zenith

// The FCN for Minuit :
// function for fitting with Minuit has fixed parameters
void elbert_simple_fcn(int &nPar, double *grad, double &value, double *params, int iflag){
  value = eloss_llh(I3Stochastics::energyLossData_,params); 
}

/********************************************************************/
// Here the likelihood is actually calculated :
// I expect the energyloss for a certain depth to follow a Landau
// Use a Gaussian approximation to start..., good enough for rough fit.
double eloss_llh(std::vector<energyloss> energyLossData, double *par) {

  double llh = 0.;
  //llh = sum over all depth bins/segments, gaussian llh of exp eLoss - measured eLoss
  for(size_t i = 0; i < energyLossData.size(); i++){
    
    //The expected energy loss for this depth :
    double eLoss = muonBundleEnergyLoss(energyLossData.at(i).slantDepth,energyLossData.at(i).zenith,par);
    
    // If all errors/weight the same and if they do not depend on the free params :
    llh += (energyLossData.at(i).energy - eLoss)*(energyLossData.at(i).energy - eLoss); 
  }
  
  return llh; //minimize the chi2, minimizers don't like the llh
}

/*************************************************************************/

/* 
 * Here come several slantDepth dependent energyloss fitfunctions. In the case of Cosmic Ray 
 * muonbundles all composition physics is inside the muonbundle energyloss, 
 * so fitting this is extracting physics!!
 */
double muonBundleEnergyLoss(double slantDepth,double zenith, double *par){
  double eLoss = 0.;
  double E0 = pow(10,par[0])*1.e6;   // seed in Log10(E0/PeV)
  double A = par[1];    // A
  double k = par[2];    // 14.5
  double g_mu = par[3]; // gamma_mu
  double a = par[4];    // a_ionisationLossConstant
  double b = par[5];    // b_stochasticLossConstant

  // The Formula... (using a first approximation), the one up to second order, see below
  // (dE/dX) (X)_muonbundle == int_{E_mu_min}^{E_mu_max} dN_mu(X)/dE_mu(X) (X) * dE_mu/dX (E_mu(X)) * dE_mu(X)
  //                        ==                           dN_mu(X)/dE_mu(surf) * dE_mu(surf)/dE_mu(X) * dE_mu/dX (E_mu(X)) * dE_mu(X)
  //                        ==                           dN_mu(X)/dE_mu(surf) * dE_mu/dX (E_mu(X)) * dE_mu(surf)
  // with dE_mu/dX (E_mu(X)) = -a-bE_mu(X) = -a-b[ (E_mu(surf) +a/b) exp(-bX) -a/b ] = -b (E_mu(surf) +a/b) exp(-bX)
  // with dN_mu(X)/dE_mu(surf) = dN_mu(E_mu>E_mu_min) (X)/ dE_mu(surf)  
  //                                            if we use Kath's approx : N_mu(E>E_mu_surf) = K*E_mu_surf^(-gamma_mu)
  //                    =>     = gamma_mu * K *E_mu_surf^(-gammma_mu-1) -> see notes TomF. for derivation (p.35)


  double E_min = a/b * (exp(b*slantDepth) -1.);

  log_trace("The params for the calculation: k :%lf, a %lf b %lf A %lf E0 %lf g_mu %lf slantdepth %lf",k,a,b,A,E0,g_mu,slantDepth);
  eLoss = exp(-b*slantDepth)*k*A/cos(zenith)*g_mu*pow(E0/A,g_mu-1.)* ( -pow(E0/A,-g_mu)*(a/g_mu - b/(1-g_mu)*E0/A) + pow(E_min,-g_mu)* (a/g_mu - b/(1-g_mu)*E_min)) ;         //minus sign corrected in first term!!
  if(eLoss < 0.)
    log_debug("Some combination of params still gives : Negative Eloss, why? : The params for the calculation: k :%lf, a %lf b %lf A %lf E0 %lf g_mu %lf slantdepth %lf",k,a,b,A,E0,g_mu,slantDepth);
  return eLoss;

}
