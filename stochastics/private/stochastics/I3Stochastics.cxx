#include "icetray/I3Frame.h"
#include "icetray/I3Tray.h"
#include "icetray/I3FrameObject.h"
#include "icetray/I3Context.h"
#include "icetray/I3Configuration.h"

#include "lilliput/minimizer/TMinuit.h"
#include "stochastics/I3Stochastics.h"
#include "stochastics/I3EnergyLoss.h"
#include "dataclasses/I3Constants.h"

I3Stochastics::I3Stochastics(const I3Context & context) :
  I3ConditionalModule(context)
{

  AddOutBox("OutBox");


  inputName_ = "EnergyLosses";
  AddParameter("InputParticleVector",
	       "Name of the input I3Vector<I3Particle> with the energylosses",
	       inputName_);

  outputName_ = "Stochastics";
  AddParameter("OutputName",
	       "Name of output I3EnergyLoss object",
	       outputName_);

  
  //fitresults of reduced one, if "", no output
  outputName_red_ = "Stochastics_red";
  AddParameter("OutputName_red",
	       "Name of output I3EnergyLoss object, which only contains results from fit without "
	       "found HE stochastics, if empty no output ",
	       outputName_red_);

  a_sel_ = 0.;
  AddParameter("A_Param",
	       "Value of parameter A for the Selection of HE stochastics",
	       a_sel_);

  b_sel_ = 0.;
  AddParameter("B_Param",
	       "Value of parameter B for the Selection of HE stochastics",
	       b_sel_);

  c_sel_ = 0.;
  AddParameter("C_Param",
	       "Value of parameter C for the Selection of HE stochastics",
	       c_sel_);

  type_sel_ = "Type1";  // type of selection
  AddParameter("SelectionType",
	       "There are two types for the selection process of HE stochastics, Type1 = "
	       "a <dE/dX> + b * Fit^c, Type2 = a + b* Fit^c, with Fit : average bundleElossFit ",
	       type_sel_);

  //// Some stuff related to minimizer !! 
  verbose_ = false;
  AddParameter("Verbose",
	       "Spits out information about the fitting",
	       verbose_);
  
  minimizer_ = "MIGRAD";
  AddParameter("Minimizer",
	       "Which TMinuit minimizer? MIGRAD or SIMPLEX or MINIMIZE (do simplex when migrad fails)",
	       minimizer_);
 
  freePar_ = 1; //[000011]
  AddParameter("FreeParams",
	       "Specify which parameters should be free and which should be fixed in a bitset : "
	       " [b,a,gamma_mu, kappa, A,E0], set with an int. "
	       " RECOMMENDED : only one free param, preferable first",
	       freePar_);

  tolerance_= 0.001;
  AddParameter("Tolerance",
	       "Criterium for convergence",
	       tolerance_);

  nIterations_= 1500;
  AddParameter("MaxIterations",
	       "Maximum number of iterations of the minimizer",
	       nIterations_);




  //type_fit_ //simple and tough Elbert fit, NO tough Elbert fit


}

void I3Stochastics::Configure()
{

  GetParameter("InputParticleVector",inputName_);
  GetParameter("OutputName",outputName_);
  GetParameter("A_Param",a_sel_);
  GetParameter("B_Param",b_sel_);
  GetParameter("C_Param",c_sel_);
  GetParameter("Verbose",verbose_);
  GetParameter("Minimizer",minimizer_);
  GetParameter("FreeParams",freePar_);
  GetParameter("OutputName_red", outputName_red_);
  GetParameter("Tolerance",tolerance_);
  GetParameter("MaxIterations", nIterations_);

  freeParams_ = freePar_;
  fixParams_ = 0;
  for(size_t j = 0; j < freeParams_.size(); j++){
    switch (j){
    case 0:
      if(freeParams_[j])
	log_info("Fitparameter E0 is free");
      else {
	log_info("Fitparameter E0 is fixed");
	fixParams_++;
      }
      break;
    case 1:
      if(freeParams_[j])
	log_info("Fitparameter A is free");
      else {
	log_info("Fitparameter A is fixed");
	fixParams_++;
      }
      break;
    case 2:
      if(freeParams_[j])
	log_info("Fitparameter kappa (normalization) is free");
      else {
	log_info("Fitparameter kappa (normalization) is fixed");
	fixParams_++;
      }
      break;
    case 3:
      if(freeParams_[j])
	log_info("Fitparameter gamma_mu is free. Are you sure?");
      else {
	log_info("Fitparameter gamma_mu is fixed");
	fixParams_++;
      }
      break;
    case 4:
      if(freeParams_[j])
	log_info("Fitparameter a is free. Are you sure?");
      else {
	log_info("Fitparameter a is fixed");
	fixParams_++;
      }
      break;
    case 5:
      if(freeParams_[j])
	log_info("Fitparameter b is free. Are you sure?");
      else {
	log_info("Fitparameter b is fixed");
	fixParams_++;
      }
      break;
    default:
      break;      
    }
  }

  log_info("There are %i parameters that will be fit",6 - fixParams_);
  
  if (type_sel_ != "Type1" && type_sel_ != "Type2")
    log_fatal("Please choose Type1 or Type2 as valid type of selection procedure");

}

void I3Stochastics::Physics(I3FramePtr frame)
{
  
/*
 * Skeleton :
 * 1) Loop over I3Vector<I3Particle> used as input
 * 2) Store X and E per segment
 * 3) Fit Elbert with A and E0 free using ROOT (gsl seems a bit more complicated). Fit through MaxLlh (like lateralfit?) or Chi2? Use toprec and muon-bundle-reco as examples!!!!
 * 4) Store E1500-E2500, A and E0
 * 5) Select Stoch :
 *    a)) a-b-c as input
 *    b)) Loop over vector again
 *    c)) if Ei > Selection * Fit => Count HE Brems
 *                                => Keep Energy and E/Fit
 *                                => Remove from fit for Chi2_reduced
 * 6) Output it into an I3EnergyLoss object :)
 */

  // Define output container
  I3EnergyLossPtr fitResult(new I3EnergyLoss);
  I3EnergyLossPtr fitResult_red(new I3EnergyLoss);

  I3VectorI3ParticleConstPtr eloss = frame->Get<I3VectorI3ParticleConstPtr>(inputName_);
  if(!eloss){
    log_warn("Input I3Vector<I3Particle> not found in frame");
    fitResult->status = I3Particle::MissingSeed;
    frame->Put(outputName_,fitResult);
    if(!outputName_red_.empty()){
      fitResult_red->status = I3Particle::MissingSeed;
      frame->Put(outputName_red_,fitResult_red);
    }
    PushFrame(frame);
    return;
  }

  if(eloss->size() == 0) {
    log_warn("Reconstructed Energylosses vector is empty, skipping fit and continuing. No output!");
    fitResult->status = I3Particle::InsufficientHits;
    frame->Put(outputName_,fitResult);
    if(!outputName_red_.empty()){
      fitResult_red->status = I3Particle::InsufficientHits;
      frame->Put(outputName_red_,fitResult_red);
    }
    PushFrame(frame);
    return;
  }


  // use zenith of first eloss segment as the track zenith
  // normally set by energy loss reconstruction (ok, cross checked it)
  double zenith = (eloss->begin())->GetZenith();
  double avEloss = 0.; 
  energyLossData_.clear(); //could even be filled from previous P frame, so very important

  if(!std::isfinite(zenith) || (zenith/I3Units::degree > 90.)){
    log_warn("Track reconstructed upwards (zenith %lf ), slant depth has no meaning, so also the fit is meaningless, skipping this event",zenith/I3Units::degree);
    fitResult->status = I3Particle::InsufficientQuality;
    frame->Put(outputName_,fitResult);
    if(!outputName_red_.empty()){
      fitResult_red->status = I3Particle::InsufficientQuality;
      frame->Put(outputName_red_,fitResult_red);
    }
    PushFrame(frame);
    return;
  } 

  // Loop over it and fill some object to use for the fitting : use MaxLlh
  for(I3Vector<I3Particle>::const_iterator iterX = eloss->begin(); iterX != eloss->end(); ++iterX){
    energyloss thisDepth;
    thisDepth.energy = iterX->GetEnergy();
    thisDepth.zenith = iterX->GetZenith();
    thisDepth.slantDepth = (I3Constants::zIceTop - iterX->GetZ())/cos(iterX->GetZenith()); // because I know that in muex-millipede-photonDensity the zenith is identical as the track zenith 
    energyLossData_.push_back(thisDepth);    
    avEloss += iterX->GetEnergy();
  }
  avEloss /= eloss->size();
 

  // Seed for energy : 
  // Plotted dE/dX vs E0 for X=2000. and A=16.
  // fitted a simple exp : dE/dX = exp(1.74303*log10(E0)+1.40241)
  // Thus seeding : take central value of reco'd dE/dX and invert for E0 seed
  //eg. dE/dX = 200 GeV/m => (ln(200)-1.40241)/1.74303 = 2.235 (as seed for E0 in log10 space)
  if(avEloss < 0.1){
    log_warn("Average reconstructed Eloss %lf < 0.1, Check reconstruction and qtot/qtot_reco",avEloss);
    // DO SOMETHING
    //PushFrame(frame);
  }
  double seedEnergy = ( log(avEloss) - 1.40241 )/1.74303;


  // Try computing the Chi2
  // The number of possible free parameters for TMinuit
  const int nParams = 6;  

  lilliput::TMinuit fit_eloss_minuit(nParams);
  if (!verbose_) {
    fit_eloss_minuit.SetPrintLevel(-1);
  }else{
    fit_eloss_minuit.SetPrintLevel(1);
  }
  fit_eloss_minuit.SetFCN(&elbert_simple_fcn);  // Set fitfunction/ptr to llh
  fit_eloss_minuit.SetErrorDef(1.);
  
  /* Set No Warnings */
  double standin = 0.0;
  int err;
  fit_eloss_minuit.mnexcm("SET NOW",&standin,0,err);
  fit_eloss_minuit.mnexcm("SET NOG",&standin,0,err);
  
  // The Likelihood related stuff :
  // Define : parNo, sname, initVal, initErr, lowerLimit, upperLimit, err
    
  // Define the initial values
  double  initialParams[6] = {seedEnergy,                // log10(E0) : "a" starting value for Primary Energy
			      16.,               // oxygen (intermediary value)
			      14.5,              // from Elbert paper (in GeV)
			      1.757,             // from Elbert paper
			      0.2388136,         //"a" from dE/dX = - a - bE (which is 0.25958*0.92 GeV*m^-1 according to PhD Pred. Miocinovic)
			      0.0003285228};     //"b" from above (0.00035709*0.92 m^-1 from same thesis)
		   
  // Define the initial stepsizes (about 10% of the starting value)
  double step[6] = {0.1*fabs(seedEnergy),1.,.5,0.2,0.02,0.00003};
  
  // Chi2 instead of llh, minimizers didn't like minimizing llh during test phase
  double chi2 = eloss_llh(energyLossData_, initialParams); 
  
  // What about starting values : average eLoss, transformed into "a" param, like avEloss/
  log_debug("Test Chi2 = %lf from initialParams with dE/dX(X=1500m) %lf and dE/dX (X=2500m) %lf",
	   chi2, muonBundleEnergyLoss(1500., zenith, initialParams), muonBundleEnergyLoss(2500., zenith, initialParams));
  
  // DefineParameter actually calls mnparm
  fit_eloss_minuit.DefineParameter(0, "Log10(Energy/PeV)", initialParams[0], step[0], 0, 0);
  fit_eloss_minuit.DefineParameter(1, "Mass", initialParams[1], step[1], 0.000001, 100.);  // Yes, limits
  fit_eloss_minuit.DefineParameter(2, "Normalization", initialParams[2], step[2], 0, 0);
  fit_eloss_minuit.DefineParameter(3, "MuonSpecIndex", initialParams[3], step[3], 0, 0); // or gamma_mu
  fit_eloss_minuit.DefineParameter(4, "IonLossConstant", initialParams[4], step[4], 0, 0);
  fit_eloss_minuit.DefineParameter(5, "StochLossConstant", initialParams[5], step[5], 0, 0);
  
  // Fix some parameters
  for(size_t j = 0; j < freeParams_.size(); j++){
    if(!freeParams_[j]){
      log_debug("Parameter %i fixed",(int)j);
      fit_eloss_minuit.FixParameter(j);
    }
  }

  // EXECUTE MINIMISATION
  double args[2];
  int ierflg = 0;
  args[0] = 2;
  //  fit_eloss_minuit.mnexcm("SET STRATEGY",args,1,ierflg);

  
  args[0] = nIterations_;  // number of max. iterations, good ones should not take more than 100 iter
  args[1] = tolerance_;   // tolerance

  fit_eloss_minuit.mnexcm(minimizer_.c_str(), args, 2, ierflg);
  
  if(ierflg != 0) {
    fitResult->status = I3Particle::FailedToConverge;
    frame->Put(outputName_,fitResult);
    if(!outputName_red_.empty()){
      fitResult_red->status = I3Particle::MissingSeed;
      frame->Put(outputName_red_,fitResult_red);
    }
    PushFrame(frame,"OutBox");
    return;
  }  else fitResult->status = I3Particle::OK;
  
  /// Get Parameters and their errors

  //outputObject, should maybe be defined higher up
  double minimized, error_mini;
  fit_eloss_minuit.GetParameter(0,minimized,error_mini);
  fitResult->primEnergyEstimate = pow(10,minimized)*1.e6;
  fitResult->primEnergyEstimate_err = error_mini;  // error on log10(E0/PeV) -->convert to err on E0
  initialParams[0] = minimized;

  fit_eloss_minuit.GetParameter(1,minimized,error_mini);
  fitResult->primMassEstimate = minimized;
  fitResult->primMassEstimate_err = error_mini;
  initialParams[1] = minimized;
  
  fitResult->Eloss_1000 = muonBundleEnergyLoss(1000., zenith, initialParams);
  fitResult->Eloss_1500 = muonBundleEnergyLoss(1500., zenith, initialParams);
  fitResult->Eloss_1600 = muonBundleEnergyLoss(1600., zenith, initialParams);
  fitResult->Eloss_1700 = muonBundleEnergyLoss(1700., zenith, initialParams);
  fitResult->Eloss_1800 = muonBundleEnergyLoss(1800., zenith, initialParams);
  fitResult->Eloss_1900 = muonBundleEnergyLoss(1900., zenith, initialParams);
  fitResult->Eloss_2000 = muonBundleEnergyLoss(2000., zenith, initialParams);
  fitResult->Eloss_2100 = muonBundleEnergyLoss(2100., zenith, initialParams);
  fitResult->Eloss_2200 = muonBundleEnergyLoss(2200., zenith, initialParams);
  fitResult->Eloss_2300 = muonBundleEnergyLoss(2300., zenith, initialParams);
  fitResult->Eloss_2400 = muonBundleEnergyLoss(2400., zenith, initialParams);
  fitResult->Eloss_3000 = muonBundleEnergyLoss(3000., zenith, initialParams);

  // Calc chi2, well actually is the same
  chi2 = eloss_llh(energyLossData_, initialParams);
  fitResult->chi2 = chi2/(energyLossData_.size() - (nParams - fixParams_));
  
  // Do selection : LOOP over energyLossData and catch interesting observables
  double avStochEnergy = 0.;
  double avStochDepth = 0.;
  double avRelStochEnergy = 0.;
  double highestStochEnergy = 0.;
  double highestRelStochEnergy = 0.;
  int nHEstoch = 0;

  energyLossData_.clear(); // reset and fill again
  for(I3Vector<I3Particle>::const_iterator iterX = eloss->begin(); iterX != eloss->end(); ++iterX){
    energyloss thisDepth;
    thisDepth.energy = iterX->GetEnergy();
    thisDepth.zenith = iterX->GetZenith();
    thisDepth.slantDepth = (I3Constants::zIceTop - iterX->GetZ())/cos(iterX->GetZenith()); 
    
    //The expected energy loss for this depth :
    double eLoss = muonBundleEnergyLoss(thisDepth.slantDepth,zenith,initialParams);
    
    bool select = false;
    // Selection Procedure (MasterThesis Elke Vandenbroucke, UGent)
    if(type_sel_ == "Type1"){
      if( thisDepth.energy > (a_sel_ * avEloss + b_sel_* pow(eLoss,c_sel_) ) )
	select = true;
    } else if(type_sel_ == "Type2"){
      if( thisDepth.energy > (a_sel_ + b_sel_* pow(eLoss,c_sel_) ) )
	select = true;
    }
    
    if(select) {
      log_debug("Selected stochastic at depth %lf m with energy %lf while fit said %lf",
		thisDepth.slantDepth,thisDepth.energy,eLoss);
      nHEstoch ++;
      avStochEnergy += thisDepth.energy;
      avRelStochEnergy += thisDepth.energy/eLoss;
      avStochDepth += thisDepth.slantDepth;
      if(thisDepth.energy > highestStochEnergy)
	highestStochEnergy = thisDepth.energy;
      if(thisDepth.energy/eLoss > highestRelStochEnergy)
	highestRelStochEnergy = thisDepth.energy/eLoss;
    } else
      // Fill vector for fitting without stochastics.
      energyLossData_.push_back(thisDepth);    
    
  } // end loop over eLosses

  double totalStochEnergy = avStochEnergy;
  double totalRelEnergy = avRelStochEnergy;
  if(nHEstoch > 0){
    avStochEnergy/= nHEstoch;
    avRelStochEnergy/= nHEstoch;
    avStochDepth/= nHEstoch;
  }
  
  //store :
  fitResult->nHEstoch = nHEstoch;
  fitResult->avStochEnergy = avStochEnergy;
  fitResult->avRelStochEnergy = avRelStochEnergy;
  fitResult->avStochDepth = avStochDepth;
  fitResult->highestStochEnergy = highestStochEnergy;
  fitResult->highestRelStochEnergy = highestRelStochEnergy;
  fitResult->totalStochEnergy = totalStochEnergy;
  fitResult->totalRelStochEnergy = totalRelEnergy;

  
  // Fit again : only interested in how the chi2 changes 
  fit_eloss_minuit.mnexcm(minimizer_.c_str(), args, 2, ierflg);
  if(ierflg == 0) {
    chi2 = eloss_llh(energyLossData_, initialParams);
    fitResult->chi2_red = chi2/(energyLossData_.size() - (nParams - fixParams_));
  } else {
    fitResult->chi2_red = NAN;
  }
    

  // output the results if wanted :
  if(!outputName_red_.empty()){
    if(ierflg != 0) {
      fitResult_red->status = I3Particle::FailedToConverge;
      fitResult_red->chi2 = NAN;
    }
    else {
      fitResult_red->status = I3Particle::OK;
      chi2 = eloss_llh(energyLossData_, initialParams);
      fitResult_red->chi2 = chi2/(energyLossData_.size() - (nParams - fixParams_));
    }

    fit_eloss_minuit.GetParameter(0,minimized,error_mini);
    fitResult_red->primEnergyEstimate = pow(10,minimized)*1.e6;
    fitResult_red->primEnergyEstimate_err = error_mini; // Change to err on E0
    initialParams[0] = minimized;
    
    fit_eloss_minuit.GetParameter(1,minimized,error_mini);
    fitResult_red->primMassEstimate = minimized;
    fitResult_red->primMassEstimate_err = error_mini;
    initialParams[1] = minimized;
    
    fitResult_red->Eloss_1000 = muonBundleEnergyLoss(1000., zenith, initialParams);
    fitResult_red->Eloss_1500 = muonBundleEnergyLoss(1500., zenith, initialParams);
    fitResult_red->Eloss_1600 = muonBundleEnergyLoss(1600., zenith, initialParams);
    fitResult_red->Eloss_1700 = muonBundleEnergyLoss(1700., zenith, initialParams);
    fitResult_red->Eloss_1800 = muonBundleEnergyLoss(1800., zenith, initialParams);
    fitResult_red->Eloss_1900 = muonBundleEnergyLoss(1900., zenith, initialParams);
    fitResult_red->Eloss_2000 = muonBundleEnergyLoss(2000., zenith, initialParams);
    fitResult_red->Eloss_2100 = muonBundleEnergyLoss(2100., zenith, initialParams);
    fitResult_red->Eloss_2200 = muonBundleEnergyLoss(2200., zenith, initialParams);
    fitResult_red->Eloss_2300 = muonBundleEnergyLoss(2300., zenith, initialParams);
    fitResult_red->Eloss_2400 = muonBundleEnergyLoss(2400., zenith, initialParams);
    fitResult_red->Eloss_3000 = muonBundleEnergyLoss(3000., zenith, initialParams);
    frame->Put(outputName_red_,fitResult_red);
  }

  // Output EnergyLoss object
  frame->Put(outputName_,fitResult);
  PushFrame(frame,"OutBox");
}

I3_MODULE(I3Stochastics);


