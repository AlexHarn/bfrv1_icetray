/**
    copyright  (C) 2004
    The IceCube Collaboration
    $Id$

    @version $Rev$
    @date $Date$
    @author kath

    @todo
*/

#include <I3Test.h>
#include <stdio.h>

#include "dataclasses/physics/I3Particle.h"
#include "toprec/I3SnowCorrectionService.h"
 
// Tolerances
double tol_tight = 0.0000001;  // For things that should be very tight
double tol_loose = 0.00001;  // For things that are computed

TEST_GROUP(Snowservices);


TEST(SimpleLambda)
{
    log_debug("SimpleLambda test: Init!");
        
    // Create a SnowService for testing
    I3SimpleSnowCorrectionServicePtr serv_simple(new I3SimpleSnowCorrectionService("My simple service", 2.1));
  
    // Create the inputs -- Although these shouldn't matter!
    const I3Position *pos = new I3Position(200, 100, 1970);  // a made-up location, which shouldn't matter
    I3ParticlePtr pc = boost::make_shared<I3Particle>(); // a made-up track, which shouldn't matter
    pc->SetPos(10, 20, 1730);   // x, y, z
    pc->SetDir(M_PI/6, M_PI/4);  // zenith=30deg, azimuth=45deg
    pc->SetTime(5000.0);
    pc->SetFitStatus(I3Particle::OK);
    double sd = 1.5;  // snow depth in meters
    I3LaputopParamsPtr par = boost::make_shared<I3LaputopParams>(); // made-up params, which shouldn't matter
    par->SetValue(Laputop::Parameter::Log10_S125, 1.234);
    par->SetValue(Laputop::Parameter::Beta, 3.0);
    
    // Test the function itself
    double attf = serv_simple->AttenuationFactor(*pos, sd, *pc, *par);
    double expected = exp(-(1.5/cos(M_PI/6))/2.1); // <--- the simple expectation
    ENSURE_DISTANCE(attf, expected, tol_loose);
    // Test the SnowDiagnostics
    SnowCorrectionDiagnosticsPtr diag = boost::make_shared<SnowCorrectionDiagnostics>();
    serv_simple->FillSnowDiagnostics(diag, pc, par);
    ENSURE_DISTANCE(diag->tstage, -999, tol_tight);
    ENSURE_DISTANCE(diag->fEM_50m, 1.0, tol_tight);
    ENSURE_DISTANCE(diag->fEM_125m, 1.0, tol_tight);
    ENSURE_DISTANCE(diag->fEM_500m, 1.0, tol_tight);
    ENSURE_DISTANCE(diag->lambda_EM_50m, 2.1, tol_tight);
    ENSURE_DISTANCE(diag->lambda_EM_125m, 2.1, tol_tight);
    ENSURE_DISTANCE(diag->lambda_EM_500m, 2.1, tol_tight);

    // Test the reset function, and try it again
    serv_simple->ResetLambda(2.2);
    attf = serv_simple->AttenuationFactor(*pos, sd, *pc, *par);
    double expected2 = exp(-(1.5/cos(M_PI/6))/2.2); // <--- the simple expectation (again)
    ENSURE_DISTANCE(attf, expected2, tol_loose);
    // Test the SnowDiagnostics (again)
    serv_simple->FillSnowDiagnostics(diag, pc, par);
    ENSURE_DISTANCE(diag->lambda_EM_50m, 2.2, tol_tight);
    ENSURE_DISTANCE(diag->lambda_EM_125m, 2.2, tol_tight);
    ENSURE_DISTANCE(diag->lambda_EM_500m, 2.2, tol_tight);

    
}


TEST(BORSLambda)
{
    log_debug("BORSLambda test: Init!");
        
    // Create a SnowService for testing
    I3BORSSnowCorrectionServicePtr serv_bors(new I3BORSSnowCorrectionService("My BORS service", 0));  // Set it up in "with muons" mode
    
    // Create the inputs
    const I3Position *pos1 = new I3Position(200, 100, 1955);  // a made-up location
    I3ParticlePtr pc = boost::make_shared<I3Particle>(); // a made-up track
    pc->SetPos(10, 20, 1940);   // x, y, z
    pc->SetDir(M_PI/6, M_PI/4);  // zenith=30deg, azimuth=45deg
    pc->SetTime(5000.0);
    pc->SetFitStatus(I3Particle::OK);
    double sd = 1.5;  // snow depth in meters
    I3LaputopParamsPtr par = boost::make_shared<I3LaputopParams>(); // made-up params
    par->SetValue(Laputop::Parameter::Log10_S125, log10(1.234));
    par->SetValue(Laputop::Parameter::Beta, 3.0);
    
    // ----- The first one: without any nose: -----
    // Test the function itself: radius from the core
    double r1 = serv_bors->DistToAxis(*pc, *pos1);
    ENSURE_DISTANCE(r1, 175.96489, tol_loose);
    // Test the helper functions
    double t = serv_bors->T_from_beta_zenith(3.0, M_PI/6);
    ENSURE_DISTANCE(t, 5.6936828, tol_loose);
    double s = serv_bors->DominantExponentialSlope(r1, t);
    ENSURE_DISTANCE(s, -0.44342656, tol_loose);
    double fEM = serv_bors->FractionEM(r1, log10(1.234));
    ENSURE_DISTANCE(fEM, 0.71524146, tol_loose);
    // Test the conversion into an attenuation factor
    double attf = serv_bors->AttenuationFactor(*pos1, sd, *pc, *par);
    /*
    double s = -0.44342656;
    double fEM = 0.71524146;
    double expectedEM = exp((1.5/cos(M_PI/6))*s); // <--- the expectation
    double expectedall = fEM * expectedEM + (1.0 - fEM);
    printf("Expecting an attenuation factor of: %f\n", expectedall);
    //ENSURE_DISTANCE(attf, expectedall, tol_loose);
    */
    ENSURE_DISTANCE(attf, 0.616575, tol_loose);
    
    // Test the SnowDiagnostics
    SnowCorrectionDiagnosticsPtr diag = boost::make_shared<SnowCorrectionDiagnostics>();
    serv_bors->FillSnowDiagnostics(diag, pc, par);
    ENSURE_DISTANCE(diag->tstage, 5.6936828, tol_loose);
    ENSURE_DISTANCE(diag->tstage_restricted, 5.6936828, tol_loose);
    ENSURE_DISTANCE(diag->fEM_50m, 0.96456282, tol_loose);
    ENSURE_DISTANCE(diag->fEM_125m, 0.83720608, tol_loose);
    ENSURE_DISTANCE(diag->fEM_500m, 0.16993461, tol_loose);
    ENSURE_DISTANCE(diag->lambda_EM_50m, 2.63037, tol_loose);
    ENSURE_DISTANCE(diag->lambda_EM_125m, 2.346119, tol_loose);
    ENSURE_DISTANCE(diag->lambda_EM_500m, 2.016437, tol_loose);
    ENSURE(std::isnan(diag->snowdepth_39B));
    
    //  ----- A second one, with the nose -------
    // For a different distance, just for testing...
    const I3Position *pos2 = new I3Position(20, 40, 1955);  // move it closer
    // Test the function itself: radius from the core
    double r2 = serv_bors->DistToAxis(*pc, *pos2);
    ENSURE_DISTANCE(r2, 12.968516, tol_loose);
    // Test the function itself: three functions
    double c0 = serv_bors->Turnover_c0(r2, t);
    ENSURE_DISTANCE(c0, 1.7383789, tol_loose);
    double snose = serv_bors->TurnoverExponentialSlope(r2, t);
    ENSURE_DISTANCE(snose, -0.57577607, tol_loose);
    double s2 = serv_bors->DominantExponentialSlope(r2, t);
    ENSURE_DISTANCE(s2, -0.31233555, tol_loose);
    double fEM2 = serv_bors->FractionEM(r2, log10(1.234));
    ENSURE_DISTANCE(fEM2, 0.99384928, tol_loose);
    // Test the conversion into an attenuation factor (again)
    double attf2 = serv_bors->AttenuationFactor(*pos2, sd, *pc, *par);
    /*
    double c0 = 1.7383789;
    double snose = -0.57577607;
    double s2 = -0.31233555;
    double fEM2 = 0.99384928;
    double expectedEM2 = c0 * exp(1.5/cos(M_PI/6)*s2) - (c0-1)*exp(1.5/cos(M_PI/6)*snose);
    double expectedall2 = fEM2 * expectedEM2 + (1.0 - fEM2);
    printf("Expecting an attenuation factor of (2): %f\n", expectedall2);
    //ENSURE_DISTANCE(attf2, expectedall2, tol_loose);
    */
    ENSURE_DISTANCE(attf2, 0.741269, tol_loose);
    // Test the SnowDiagnostics (again)
    serv_bors->FillSnowDiagnostics(diag, pc, par);
    // These are the same as before (since depends on shower, not tank)...
    ENSURE_DISTANCE(diag->tstage, 5.69368, tol_loose);
    ENSURE_DISTANCE(diag->tstage_restricted, 5.69368, tol_loose);
    ENSURE_DISTANCE(diag->fEM_50m, 0.964562, tol_loose);
    ENSURE_DISTANCE(diag->fEM_125m, 0.837206, tol_loose);
    ENSURE_DISTANCE(diag->fEM_500m, 0.1699346, tol_loose);
    ENSURE_DISTANCE(diag->lambda_EM_50m, 2.63037, tol_loose);
    ENSURE_DISTANCE(diag->lambda_EM_125m, 2.346119, tol_loose);
    ENSURE_DISTANCE(diag->lambda_EM_500m, 2.016437, tol_loose);

}

TEST(RadeBasicLambda)
{
    log_debug("RadeLambda test: Init!");
        
    // Create a SnowService for testing
    I3RadeBasicSnowCorrectionServicePtr serv_rade(new I3RadeBasicSnowCorrectionService("My RADE service", 15.0));
    
    // Create the inputs
    const I3Position *pos = new I3Position(200, 100, 1970);  // a made-up location
    I3ParticlePtr pc = boost::make_shared<I3Particle>(); // a made-up track
    pc->SetPos(10, 20, 1730);   // x, y, z
    pc->SetDir(M_PI/6, M_PI/4);  // zenith=30deg, azimuth=45deg
    pc->SetTime(5000.0);
    pc->SetFitStatus(I3Particle::OK);
    double sd = 1.5;  // snow depth in meters
    I3LaputopParamsPtr par = boost::make_shared<I3LaputopParams>(); // made-up params (shouldn't matter)
    par->SetValue(Laputop::Parameter::Log10_S125, log10(1.234));
    par->SetValue(Laputop::Parameter::Beta, 3.0);
    
    // Test the function itself: radius from the core
    double r = serv_rade->DistToAxis(*pc, *pos);
    ENSURE_DISTANCE(r, 90.03203, tol_loose);
    // Test the function itself: lambda
    double l = serv_rade->Lambda(r, 1.234);
    ENSURE_DISTANCE(l, 2.29008, tol_loose);
    // Test the conversion into an attenuation factor
    double attf = serv_rade->AttenuationFactor(*pos, sd, *pc, *par);
    double expected = exp(-(1.5/cos(M_PI/6))/l); // <--- the expectation
    ENSURE_DISTANCE(attf, expected, tol_loose);
    // Test the SnowDiagnostics
    SnowCorrectionDiagnosticsPtr diag = boost::make_shared<SnowCorrectionDiagnostics>();
    serv_rade->FillSnowDiagnostics(diag, pc, par);
    ENSURE_DISTANCE(diag->tstage, -999, tol_tight);
    ENSURE_DISTANCE(diag->fEM_50m, 1.0, tol_tight);
    ENSURE_DISTANCE(diag->fEM_125m, 1.0, tol_tight);
    ENSURE_DISTANCE(diag->fEM_500m, 1.0, tol_tight);
    ENSURE_DISTANCE(diag->lambda_EM_50m, serv_rade->Lambda(50, 1.234), tol_tight);
    ENSURE_DISTANCE(diag->lambda_EM_125m, serv_rade->Lambda(125, 1.234), tol_tight);
    ENSURE_DISTANCE(diag->lambda_EM_500m, serv_rade->Lambda(500, 1.234), tol_tight);

    // Test another one, with a small radius: make sure it resets to "15.0" properly
    const I3Position *pos2 = new I3Position(20, 15, 1740);  // a made-up location
    double r2 = serv_rade->DistToAxis(*pc, *pos2);
    ENSURE_DISTANCE(r2, 10.782225, tol_loose);
    // Test the function itself: lambda
    double l2 = serv_rade->Lambda(r2, 1.234);  //<-- NOT the one it should use
    ENSURE_DISTANCE(l2, 3.4026621, tol_loose);
    double latmin = serv_rade->Lambda(15.0, 1.234);  //<-- the one it should use
    ENSURE_DISTANCE(latmin, 3.2295833, tol_loose);
    // The one it should really use
    double attf2 = serv_rade->AttenuationFactor(*pos2, sd, *pc, *par);
    double expected2 = exp(-(1.5/cos(M_PI/6))/latmin); // <--- the expectation
    ENSURE_DISTANCE(attf2, expected2, tol_loose);

}


