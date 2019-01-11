/**
 *  copyright  (C) 2006
 *  the icecube collaboration
 *  $Id$
 *
 *  @version $Revision$
 *  @date $Date$
 *  @author boersma
 */

#include <I3Test.h>

#include "icetray/I3Logging.h"
#include <string>
#include <sstream>
#include <numeric>

#include "gulliver/I3GulliverBase.h"
#include "lilliput/minimizer/I3GulliverMinuit.h"
#ifdef USE_MINUIT2
#include "lilliput/minimizer/I3GulliverMinuit2.h"
#endif
#include "lilliput/minimizer/I3GSLSimplex.h"
#include "lilliput/minimizer/I3GSLMultiMin.h"
#include "lilliput/minimizer/I3GulliverAnnealing.h"

#ifdef USE_MULTINEST
#include "minimizer/I3GulliverMN.h" //ML
#endif

#include "phys-services/I3GSLRandomService.h"
#include "MinimizerTestFunctions.h"
#include "RandomInitSpecs.h"

/****************************************************\
 * A bunch of global constants                      *
\****************************************************/

// search specs
static const double fineTolerance = 0.00001;
static const int fineIterations = 10000;
static const double coarseTolerance = 0.05;
static const int coarseIterations = 1000;
static const double dummy = 0.1;

// orders of magnitudes for initial parameters
static const double smallScale = 0.5;
static const double mediumScale = 5.0;
static const double largeScale = 50.0;

// GSL gradient settings
static const unsigned int maxIterations = 1000;
static const double initialStepsize = 1.0;
static const double linesearchTolerance = 0.1;
static const double gradnormTolerance = 0.001;
static const double multiminStepfactor = 0.5;

// how often to try in case of a flat llh landscape before giving up
static const int patience = 50;

// const unsigned int maxit= I3GulliverAnnealing::DEFAULT_MAXITERATIONS; // = 10000;
static const unsigned int maxit= 100000;
static const double SEtol = I3GulliverAnnealing::DEFAULT_TOLERANCE; //=1.e-5;
static const bool shiftbounds = I3GulliverAnnealing::DEFAULT_SHIFTBOUNDARIES; // = false;
static const double quench = I3GulliverAnnealing::DEFAULT_QUENCHINGFACTOR; //=0.5;
static const double Tstart = I3GulliverAnnealing::DEFAULT_STARTTEMP; //=10000.0;
static const int Scycle = I3GulliverAnnealing::DEFAULT_SCYCLE; // = 20;
static const int Tcycle = I3GulliverAnnealing::DEFAULT_TCYCLE; // = 5;
// static const int Ecycle = I3GulliverAnnealing::DEFAULT_NCYCLESEPS; // = 5;
static const int Ecycle = 10;
static const double SEstepfactor = 0.5;
static const int SEmindim = 2;
static const int SEmaxdim = 20;
static unsigned long int kboltz = 13806503;
static unsigned long int oracle = 42;
static const bool hard2find = true;
static const bool easy2find = false;


TEST_GROUP(MinimizerTest);

/********************************************************************\
 * The actual test (templated)                                      *
 * If the function HAS A minimum, then check that it's found.       *
 * If the function HAS NO minimum, then the minimizer must diverge! *
 * If the function HAS A hard-to-find minimum, then check that it's *
 * EITHER converged & found _correctly_ OR diverged!
 * The function will be minimized for a range of dimensions         *
 * A hard to find minimum should be found for at least one dimension*
 * (number of parameters). Per dimension value we do 100 tries.     *
\********************************************************************/

template<class func>
void TestMyMinimizer( I3MinimizerBase &m, double scale, bool hard_to_find = false,
                                          double stepfactor = 0.1,
                                          unsigned int mindim=1,unsigned int maxdim=8,
                                          bool withbounds=false ){
    I3RandomServicePtr rndptr( new I3GSLRandomService(oracle) );
    double tol = 10 * m.GetTolerance();
    int nfound = 0;
    int ntries = hard_to_find ? 100 : 1;
    for ( unsigned int dim=mindim; dim<maxdim; ++dim ){
        bool oobtested = false;
        for (int itry = 0; itry<ntries; ++itry){
            specs_t parspecs( RandomInitSpecs(dim,scale,rndptr,stepfactor,withbounds) );
            func f(dim);
            log_trace( "(%d) checking %s with dim=%u parspecsize=%zu scale=%g tolerance=%7.4f",
                      itry, m.GetName().c_str(), dim, parspecs.size(), scale, tol );
            I3MinimizerResult dwarf(dim);
            log_trace("ok");
            if (withbounds){
                // set bounds
                f.EnableBoundsCheck(parspecs);
                // now first a test of the test: OOB values should cause an OOB exception
                if (!oobtested){
                    double fval;
                    std::vector<double> way_out_of_bounds(dim,20*scale);
                    try {
                        fval = f(way_out_of_bounds);
                        log_error("out of bounds query (parmin=%f, parmax=%f, parval=%f) returned value %f",
                                  parspecs[0].minval_, parspecs[0].maxval_, 20*scale, fval);
                        FAIL("TESTCODE NOT WORKING: failed to catch out of bounds function query");
                    } catch( const OutOfBounds& oob ){
                        log_trace("test works: %s",oob.str().c_str());
                    }
                    oobtested = true;
                }
            }
            try {
                dwarf = m.Minimize( f, parspecs );
            } catch( const OutOfBounds& oob ){
                FAIL(oob.str().c_str());
            }
            // I3MinimizerResult dwarf = m.Minimize( f, parspecs );
            std::ostringstream mininfo;
            mininfo << "[tolerance set to " << tol
                    << ", using " << dim << " fitting parameters," << std::endl
                    << "did " << f.GetNEval() << " function evaluations, "
                    << "max configured iterations: " << m.GetMaxIterations() << "]";
            for ( unsigned int ipar = 0; ipar < dim; ++ipar ){
                mininfo << std::endl
                        << "par[" << ipar << "]: initval=" << parspecs[ipar].initval_
                                          <<   " stepsize=" << parspecs[ipar].stepsize_
                                          <<   " endval=" << dwarf.par_[ipar] 
                                          <<   " diff= " << (dwarf.par_[ipar]-parspecs[ipar].initval_);
                if (withbounds){
                     mininfo <<  " minval=" << parspecs[ipar].minval_
                             <<  " maxval=" << parspecs[ipar].maxval_
                             << std::endl;
                }
            }
            mininfo << std::endl << "Vertical distance to minimum: " << (dwarf.minval_ - f.Minimum());
            if ( f.HasMinimum() ){
                std::ostringstream message;
                if ( ! hard_to_find ){
                    message << m.GetName() << " failed to converge" << std::endl
                            << mininfo.str();
                    ENSURE(dwarf.converged_, message.str().c_str() );
                }
                if ( dwarf.converged_ ){
                    if (hard_to_find){
                        log_trace("minimum found at try number %d", itry);
                    }
                    message.str("");
                    message << m.GetName()
                            << " CONVERGED to a WRONG value" << std::endl
                            << mininfo.str();
                    double true_min = f.Minimum();
                    ENSURE_DISTANCE(dwarf.minval_,true_min,tol, message.str().c_str() );
                    nfound += 1;
                    break; // no need to try more times
                }
                // TODO: also check location of minimum
            } else {
                std::ostringstream message;
                message << m.GetName() << " converged, but it SHOULD have FAILED to converge "
                        << std::endl << "(because the function HAS NO MINIMUM)" << std::endl
                        << mininfo.str();
                ENSURE(!dwarf.converged_, message.str().c_str() );
            }
        }
    }
    if (hard_to_find){
        std::ostringstream message;
        message << m.GetName()
                << " failed to find a hard to find minimum at least once."
                << std::endl;
        ENSURE(nfound>0, message.str().c_str() );
    }
}

/***********************\
 * Test instantiations *
\***********************/



#ifdef GSL_MAJOR_VERSION // older versions did not have GSL_VERSION_MAJOR/MINOR
 #if GSL_MAJOR_VERSION==1
  #if GSL_MINOR_VERSION>13
   #define HAVE_BFGS2 1
  #endif
 #else //GSL_VERSION_MAJOR==1
  #define HAVE_BFGS2 1
 #endif
#endif

#ifdef HAVE_BFGS2
static const int nGSLalg=5;
static const std::string GSLalgnames[nGSLalg] =
{ "conjugate_fr", "conjugate_pr", "vector_bfgs", "vector_bfgs2", "steepest_descent" };
#else
static const int nGSLalg=4;
static const std::string GSLalgnames[nGSLalg] =
{ "conjugate_fr", "conjugate_pr", "vector_bfgs", "steepest_descent" };
#endif

#ifdef USE_MINUIT2
static const int nMinuit2alg=3;
static const std::string Minuit2algnames[nMinuit2alg+1] = 
    {"MIGRAD","SIMPLEX", "COMBINED", "FUMILI"};
#endif

TEST(TMinuitConvergeEasy)
{
    I3GulliverMinuit minuitCoarse("coarse",coarseTolerance,coarseIterations,-2,2,"SIMPLEX",true);
    I3GulliverMinuit minuitFine("fine",fineTolerance,fineIterations,-2,2,"SIMPLEX",true);
    TestMyMinimizer<InfHole>(minuitCoarse,largeScale);
    TestMyMinimizer<InfHole>(minuitCoarse,smallScale);
    TestMyMinimizer<InfHole>(minuitFine,largeScale);
    TestMyMinimizer<InfHole>(minuitFine,smallScale);
}

#ifdef USE_MINUIT2
TEST(Minuit2ConvergeEasy)
{
    int i;

    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_nogradient",
            coarseTolerance, coarseIterations, -2, 2, alg, true, false, false, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_nogradient",
            fineTolerance, fineIterations, -2, 2, alg, true, false, false, false);
        TestMyMinimizer<InfHole>(minuitCoarse,largeScale);
        TestMyMinimizer<InfHole>(minuitCoarse,smallScale);
        TestMyMinimizer<InfHole>(minuitFine,largeScale);
        TestMyMinimizer<InfHole>(minuitFine,smallScale);
    }
    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_gradients",
            coarseTolerance, coarseIterations, -2, 2, alg, true, true, false, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_gradients",
            fineTolerance, fineIterations, -2, 2, alg, true, true, false, false);
        TestMyMinimizer<InfHole>(minuitCoarse,largeScale);
        TestMyMinimizer<InfHole>(minuitCoarse,smallScale);
        TestMyMinimizer<InfHole>(minuitFine,largeScale);
        TestMyMinimizer<InfHole>(minuitFine,smallScale);
    }
}
#endif

#ifdef USE_LBFGSB
#include "minimizer/I3GulliverLBFGSB.h"
TEST(LBFGSBConvergeEasy)
{
    I3Context ctx;
    I3GulliverLBFGSB mini(ctx);
    mini.Configure();

    TestMyMinimizer<InfHole>(mini,largeScale);
    TestMyMinimizer<InfHole>(mini,smallScale);
}

TEST(LBFGSBConvergeHard)
{
    I3Context ctx;
    I3GulliverLBFGSB mini(ctx);
    mini.Configure();

    TestMyMinimizer<PotHole>(mini,mediumScale,hard2find); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(mini,smallScale,hard2find); // either diverge or converge correctly
}

TEST(LBFGSBConvergeHardWithBounds)
{
    I3Context ctx;
    I3GulliverLBFGSB mini(ctx);
    mini.Configure();

    double stepfactor = 0.1;
    unsigned int mindim=1;
    unsigned int maxdim=8;
    bool withbounds=true;

    TestMyMinimizer<PotHole>(mini,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(mini,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
}

TEST(LBFGSBDivergeOnFlat)
{
    I3Context ctx;
    I3GulliverLBFGSB mini(ctx);
    mini.Configure();

    TestMyMinimizer<JustFlat>(mini,largeScale);
    TestMyMinimizer<JustFlat>(mini,smallScale);
}

TEST(LBFGSBDivergeOnSlope)
{
    I3Context ctx;
    I3GulliverLBFGSB mini(ctx);
    mini.Configure();

    TestMyMinimizer<Slope>(mini,largeScale);
    TestMyMinimizer<Slope>(mini,smallScale);
}
#endif

#ifdef USE_MULTINEST
TEST(MNConvergeEasy)
{
    I3Context ctx;
    I3GulliverMN mini(ctx);
    mini.Configure();

    TestMyMinimizer<InfHole>(mini,largeScale);
    TestMyMinimizer<InfHole>(mini,smallScale);
}

TEST(MNDivergeOnFlat)
{
    I3Context ctx;
    I3GulliverMN mini(ctx);
    mini.Configure();

    TestMyMinimizer<JustFlat>(mini,largeScale);
    TestMyMinimizer<JustFlat>(mini,smallScale);
}

/** Multinest is a minimizer working only on bounded parameter spaces.
 *  Since minimizers working on bounded parameter spaces are not expected
 *  to diverge towards a parameter par->infinity, but to converge towards
 *  a boundary of the corresponding parameter, this test was removed. */
/*
TEST(MNDivergeOnSlope)
{
    I3Context ctx;
    I3GulliverMN mini(ctx);
    mini.Configure();

    TestMyMinimizer<Slope>(mini,largeScale);
    TestMyMinimizer<Slope>(mini,smallScale);
}*/

TEST(MNConvergeHard)
{
    I3Context ctx;
    I3GulliverMN mini(ctx);
    mini.Configure();

    TestMyMinimizer<PotHole>(mini,smallScale);
    TestMyMinimizer<PotHole>(mini,largeScale); 
    TestMyMinimizer<PotHole>(mini,smallScale,hard2find); 
    TestMyMinimizer<PotHole>(mini,largeScale,hard2find); 
}

TEST(MNConvergeHardWithBounds)
{
    I3Context ctx;
    I3GulliverMN mini(ctx);
    mini.Configure();

    double stepfactor = 1.0;
    unsigned int mindim=2;
    unsigned int maxdim=8;
    bool withbounds=true;

    TestMyMinimizer<PotHole>(mini,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds); 
    TestMyMinimizer<PotHole>(mini,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds); 
    TestMyMinimizer<InfHole>(mini,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds); 
    TestMyMinimizer<InfHole>(mini,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds); 
}
#endif

TEST(GSLConvergeEasy)
{
    I3GSLSimplex gslCoarse("coarse",dummy,coarseTolerance,coarseIterations,patience);
    I3GSLSimplex gslFine("fine",dummy,fineTolerance,fineIterations,patience);
    TestMyMinimizer<InfHole>(gslCoarse,largeScale);
    TestMyMinimizer<InfHole>(gslCoarse,smallScale);
    TestMyMinimizer<InfHole>(gslFine,largeScale);
    TestMyMinimizer<InfHole>(gslFine,smallScale);
}

TEST(GSLGradientConvergeEasy)
{
    for ( int i= 0; i<nGSLalg; ++i){
        const std::string &name = GSLalgnames[i];
        I3GSLMultiMin gslGrad(name,maxIterations,initialStepsize,
                              linesearchTolerance,gradnormTolerance,name);
        TestMyMinimizer<InfHole>(gslGrad,smallScale,easy2find,multiminStepfactor);
        TestMyMinimizer<InfHole>(gslGrad,largeScale,easy2find,multiminStepfactor);
    }
}

TEST(AnnealingConvergeEasy)
{
    I3RandomServicePtr rndptr( new I3GSLRandomService(kboltz) );
    I3GulliverAnnealing cookNcool( "cookNcool", rndptr,
            maxit, SEtol, shiftbounds, quench, Tstart, Scycle, Tcycle, Ecycle );
    TestMyMinimizer<InfHole>(cookNcool,largeScale,easy2find,SEstepfactor,SEmindim,SEmaxdim);
    TestMyMinimizer<InfHole>(cookNcool,smallScale,easy2find,SEstepfactor,SEmindim,SEmaxdim);
}

TEST(TMinuitConvergeHard)
{
    I3GulliverMinuit minuitCoarse("coarse",coarseTolerance,coarseIterations,-2,2,"SIMPLEX",true);
    I3GulliverMinuit minuitFine("fine",fineTolerance,fineIterations,-2,2,"SIMPLEX",true);
    TestMyMinimizer<PotHole>(minuitCoarse,mediumScale,hard2find); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(minuitCoarse,smallScale,hard2find); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(minuitFine,mediumScale,hard2find); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(minuitFine,smallScale,hard2find); // either diverge or converge correctly
}

// disturbing: TMinuit fails this test! It converges to a wrong value...
/*
TSET(TMinuitConvergeHardWithBounds)
{
    double stepfactor = 0.1;
    unsigned int mindim=1;
    unsigned int maxdim=8;
    bool withbounds=true;
    I3GulliverMinuit minuitCoarse("coarse",coarseTolerance,coarseIterations,-2,2,"SIMPLEX",true);
    I3GulliverMinuit minuitFine("fine",fineTolerance,fineIterations,-2,2,"SIMPLEX",true);
    TestMyMinimizer<PotHole>(minuitCoarse,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(minuitCoarse,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(minuitFine,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(minuitFine,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
}
*/

#ifdef USE_MINUIT2
TEST(Minuit2ConvergeHard)
{
    int i;

    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_nogradient",
            coarseTolerance, coarseIterations, -2, 2, alg, true, false, false, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_nogradient",
            fineTolerance, fineIterations, -2, 2, alg, true, false, false, false);
        TestMyMinimizer<PotHole>(minuitCoarse,mediumScale,hard2find);
        TestMyMinimizer<PotHole>(minuitCoarse,smallScale,hard2find);
        TestMyMinimizer<PotHole>(minuitFine,mediumScale,hard2find);
        TestMyMinimizer<PotHole>(minuitFine,smallScale,hard2find);
    }
    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_gradients",
            coarseTolerance, coarseIterations, -2, 2, alg, true, true, false, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_gradients",
            fineTolerance, fineIterations, -2, 2, alg, true, true, false, false);
        TestMyMinimizer<PotHole>(minuitCoarse,mediumScale,hard2find);
        TestMyMinimizer<PotHole>(minuitCoarse,smallScale,hard2find);
        TestMyMinimizer<PotHole>(minuitFine,mediumScale,hard2find);
        TestMyMinimizer<PotHole>(minuitFine,smallScale,hard2find);
    }
}

TEST(Minuit2ConvergeHardWithBounds)
{
    int i;
    double stepfactor = 0.1;
    unsigned int mindim=1;
    unsigned int maxdim=8;
    bool withbounds=true;

    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_nogradient",
            coarseTolerance, coarseIterations, -2, 2, alg, true, false, false, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_nogradient",
            fineTolerance, fineIterations, -2, 2, alg, true, false, false, false);
        TestMyMinimizer<PotHole>(minuitCoarse,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds);
        TestMyMinimizer<PotHole>(minuitCoarse,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds);
        TestMyMinimizer<PotHole>(minuitFine,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds);
        TestMyMinimizer<PotHole>(minuitFine,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds);
    }
    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_gradients",
            coarseTolerance, coarseIterations, -2, 2, alg, true, true, false, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_gradients",
            fineTolerance, fineIterations, -2, 2, alg, true, true, false, false);
        TestMyMinimizer<PotHole>(minuitCoarse,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds);
        TestMyMinimizer<PotHole>(minuitCoarse,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds);
        TestMyMinimizer<PotHole>(minuitFine,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds);
        TestMyMinimizer<PotHole>(minuitFine,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds);
    }
}
#endif

TEST(GSLConvergeHard)
{
    I3GSLSimplex gslCoarse("coarse",dummy,coarseTolerance,coarseIterations,patience);
    I3GSLSimplex gslFine("fine",dummy,fineTolerance,fineIterations,patience);
    TestMyMinimizer<PotHole>(gslCoarse,mediumScale,hard2find); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(gslCoarse,smallScale,hard2find); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(gslFine,mediumScale,hard2find); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(gslFine,smallScale,hard2find); // either diverge or converge correctly
}

TEST(GSLConvergeHardWithBounds)
{
    double stepfactor = 0.1;
    unsigned int mindim=1;
    unsigned int maxdim=8;
    bool withbounds=true;
    I3GSLSimplex gslCoarse("coarse",dummy,coarseTolerance,coarseIterations,patience);
    I3GSLSimplex gslFine("fine",dummy,fineTolerance,fineIterations,patience);
    TestMyMinimizer<PotHole>(gslCoarse,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(gslCoarse,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(gslFine,mediumScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
    TestMyMinimizer<PotHole>(gslFine,smallScale,hard2find,stepfactor,mindim,maxdim,withbounds); // either diverge or converge correctly
}

TEST(GSLGradientConvergeHard)
{
    for ( int i= 0; i<nGSLalg; ++i){
        const std::string &name = GSLalgnames[i];
        I3GSLMultiMin gslGrad(name,maxIterations,initialStepsize,
                              linesearchTolerance,gradnormTolerance,name);
        TestMyMinimizer<PotHole>(gslGrad,smallScale,hard2find,multiminStepfactor);
        TestMyMinimizer<PotHole>(gslGrad,largeScale,hard2find,multiminStepfactor);
    }
}


// Removed test: it only passes if you tweak the random seed just right. That's not good enough.
/*
TSET(AnnealingConvergeHard)
{
    I3RandomServicePtr rndptr( new I3GSLRandomService(kboltz) );
    I3GulliverAnnealing cookNcool( "cookNcool", rndptr,
            maxit, SEtol, shiftbounds, quench, Tstart, Scycle, Tcycle, Ecycle );
    // TestMyMinimizer<PotHole>(cookNcool,smallScale,true,SEstepfactor,SEmindim,SEmaxdim); FAILs :-(
    TestMyMinimizer<PotHole>(cookNcool,mediumScale,hard2find,SEstepfactor,SEmindim,SEmaxdim);
    TestMyMinimizer<PotHole>(cookNcool,largeScale,hard2find,SEstepfactor,SEmindim,SEmaxdim);
}
*/

TEST(TMinuitDivergeOnFlat)
{
    I3GulliverMinuit minuitCoarse("coarse",coarseTolerance,coarseIterations,-2,2,"SIMPLEX",true);
    I3GulliverMinuit minuitFine("fine",fineTolerance,fineIterations,-2,2,"SIMPLEX",true);
    TestMyMinimizer<JustFlat>(minuitCoarse,largeScale);
    TestMyMinimizer<JustFlat>(minuitCoarse,smallScale);
    TestMyMinimizer<JustFlat>(minuitFine,largeScale);
    TestMyMinimizer<JustFlat>(minuitFine,smallScale);
}

#ifdef USE_MINUIT2
TEST(Minuit2DivergeOnFlat)
{
    int i;
    
    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_nogradient",
            coarseTolerance, coarseIterations, -2, 2, alg, true, false, false, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_nogradient",
            fineTolerance, fineIterations, -2, 2, alg, true, false, false, false);
        TestMyMinimizer<JustFlat>(minuitCoarse,largeScale);
        TestMyMinimizer<JustFlat>(minuitCoarse,smallScale);
        TestMyMinimizer<JustFlat>(minuitFine,largeScale);
        TestMyMinimizer<JustFlat>(minuitFine,smallScale);
    }
    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_gradients",
            coarseTolerance, coarseIterations, -2, 2, alg, true, true, false, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_gradients",
            fineTolerance, fineIterations, -2, 2, alg, true, true, false, false);
        TestMyMinimizer<JustFlat>(minuitCoarse,largeScale);
        TestMyMinimizer<JustFlat>(minuitCoarse,smallScale);
        TestMyMinimizer<JustFlat>(minuitFine,largeScale);
        TestMyMinimizer<JustFlat>(minuitFine,smallScale);
    }
}
#endif


TEST(GSLDivergeOnFlat)
{
    I3GSLSimplex gslCoarse("coarse",dummy,coarseTolerance,coarseIterations,patience);
    I3GSLSimplex gslFine("fine",dummy,fineTolerance,fineIterations,patience);
    TestMyMinimizer<JustFlat>(gslCoarse,largeScale);
    TestMyMinimizer<JustFlat>(gslCoarse,smallScale);
    TestMyMinimizer<JustFlat>(gslFine,largeScale);
    TestMyMinimizer<JustFlat>(gslFine,smallScale);
}

TEST(GSLGradientDivergeOnFlat)
{
    for ( int i= 0; i<nGSLalg; ++i){
        const std::string &name = GSLalgnames[i];
        I3GSLMultiMin gslGrad(name,maxIterations,initialStepsize,
                              linesearchTolerance,gradnormTolerance,name);
        TestMyMinimizer<JustFlat>(gslGrad,largeScale,easy2find,multiminStepfactor);
        TestMyMinimizer<JustFlat>(gslGrad,smallScale,easy2find,multiminStepfactor);
    }
}

TEST(AnnealingDivergeOnFlat)
{
    I3RandomServicePtr rndptr( new I3GSLRandomService(kboltz) );
    I3GulliverAnnealing cookNcool( "cookNcool", rndptr,
            maxit, SEtol, shiftbounds, quench, Tstart, Scycle, Tcycle, Ecycle );
    TestMyMinimizer<JustFlat>(cookNcool,largeScale,easy2find,SEstepfactor,SEmindim,SEmaxdim);
    TestMyMinimizer<JustFlat>(cookNcool,smallScale,easy2find,SEstepfactor,SEmindim,SEmaxdim);
}

TEST(TMinuitDivergeOnSlope)
{
    I3GulliverMinuit minuitCoarse("coarse",coarseTolerance,coarseIterations,-2,2,"SIMPLEX",true);
    I3GulliverMinuit minuitFine("fine",fineTolerance,fineIterations,-2,2,"SIMPLEX",true);
    TestMyMinimizer<Slope>(minuitCoarse,largeScale);
    TestMyMinimizer<Slope>(minuitCoarse,smallScale);
    TestMyMinimizer<Slope>(minuitFine,largeScale);
    TestMyMinimizer<Slope>(minuitFine,smallScale);
}

#if 0
#ifdef USE_MINUIT2
TEST(Minuit2DivergeOnSlope)
{
    int i;

    // NOTE: small scales with coarse grids are disabled with SIMPLEX
    // See note about "the estimated distance to the minimum is largely
    // fantasy, so it would even know if it did converge" in the Minuit2
    // manual to understand why.

    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        if (alg == "SIMPLEX")
            continue;
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_nogradient",
            coarseTolerance, coarseIterations, -2, 2, alg, true, false);
        I3GulliverMinuit2 minuitFine(alg + "_fine_nogradient",
            fineTolerance, fineIterations, -2, 2, alg, true, false);
        TestMyMinimizer<Slope>(minuitCoarse,largeScale);
        TestMyMinimizer<Slope>(minuitCoarse,smallScale);
        TestMyMinimizer<Slope>(minuitFine,largeScale);
        TestMyMinimizer<Slope>(minuitFine,smallScale);
    }
    for (i = 0; i < nMinuit2alg; i++) {
        std::string alg = Minuit2algnames[i];
        if (alg == "SIMPLEX")
            continue;
        I3GulliverMinuit2 minuitCoarse(alg + "_coarse_gradients",
            coarseTolerance, coarseIterations, -2, 2, alg, true, true);
        I3GulliverMinuit2 minuitFine(alg + "_fine_gradients",
            fineTolerance, fineIterations, -2, 2, alg, true, true);
        TestMyMinimizer<Slope>(minuitCoarse,largeScale);
        TestMyMinimizer<Slope>(minuitCoarse,smallScale);
        TestMyMinimizer<Slope>(minuitFine,largeScale);
        TestMyMinimizer<Slope>(minuitFine,smallScale);
    }
}
#endif
#endif


TEST(GSLDivergeOnSlope)
{
    I3GSLSimplex gslCoarse("coarse",dummy,coarseTolerance,coarseIterations,patience);
    I3GSLSimplex gslFine("fine",dummy,fineTolerance,fineIterations,patience);
    log_trace("testing gsl coarse on large scale");
    TestMyMinimizer<Slope>(gslCoarse,largeScale);
    log_trace("testing gsl coarse on small scale");
    TestMyMinimizer<Slope>(gslCoarse,smallScale);
    log_trace("testing gsl fine on large scale");
    TestMyMinimizer<Slope>(gslFine,largeScale);
    log_trace("testing gsl fine on small scale");
    TestMyMinimizer<Slope>(gslFine,smallScale);
}

TEST(GSLGradientDivergeOnSlope)
{
    for ( int i= 0; i<nGSLalg; ++i){
        const std::string &name = GSLalgnames[i];
        I3GSLMultiMin gslGrad(name,maxIterations,initialStepsize,
                              linesearchTolerance,gradnormTolerance,name);
        TestMyMinimizer<Slope>(gslGrad,largeScale,easy2find,multiminStepfactor);
        TestMyMinimizer<Slope>(gslGrad,smallScale,easy2find,multiminStepfactor);
    }
}

TEST(AnnealingDivergeOnSlope)
{
    I3RandomServicePtr rndptr( new I3GSLRandomService(kboltz) );
    I3GulliverAnnealing cookNcool( "cookNcool", rndptr,
            maxit, SEtol, shiftbounds, quench, Tstart, Scycle, Tcycle, Ecycle );
    TestMyMinimizer<Slope>(cookNcool,largeScale,easy2find,SEstepfactor,SEmindim,SEmaxdim);
    TestMyMinimizer<Slope>(cookNcool,smallScale,easy2find,SEstepfactor,SEmindim,SEmaxdim);
}

