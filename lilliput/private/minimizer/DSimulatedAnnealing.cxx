// ======================================================================
//  DSimulatedAnnealing.cc  - Last modif: Wed 12 Sep 2007 15:38:17 CEST - 
//  $Id$
//  Ralf  Auer, auer(at)physik.uni-erlangen.de
//  Felix Fehr, fehr(at)physik.uni-erlangen.de
// ======================================================================

/******************************************************
 *                                                    *
 *  C++ implementation of the Simulated Annealing     *
 *  algorithm.                                        *
 *  Note that throughout the class single precision   *
 *  is used in order to speed up the code             *
 *                                                    *
 *  This version uses the really good random number   *
 *  generator ran1 from NR-C++, not the ROOT crap.    *
 ******************************************************/

// Note:
// You can find a brief manual in the header file.

// std
#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "phys-services/I3RandomService.h"
#include "DSimulatedAnnealing.h"

// Stolen from ROOT's TStopWatch.
// We want to avoid using ROOT directly.
class myStopwatch
{
public:
    myStopwatch() : sys(0), user(0) {};
    void Start();
    void Stop();
    double CpuTime() { return (user+sys); };
private:
    struct rusage stop, start;
    double sys, user;
    bool fail;
};

void
myStopwatch::Start()
{
    fail = (getrusage(RUSAGE_SELF, &start) == -1);
}

void
myStopwatch::Stop()
{
    if (getrusage(RUSAGE_SELF, &stop) != -1 && !fail) {
        user += (stop.ru_utime.tv_sec - start.ru_utime.tv_sec);
        user += double(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) / 1e6;

        sys += (stop.ru_stime.tv_sec - start.ru_stime.tv_sec);
        sys += double(stop.ru_stime.tv_usec - start.ru_stime.tv_usec) / 1e6;
    }
}

using namespace std;

// Implementation of Simulated Annealing

// ctor
DSimulatedAnnealing::DSimulatedAnnealing(
                                         int nPar_,
                                         PtrToObjFunc funk_,
                                         const vector<double>& vLowerBounds_,
                                         const vector<double>& vUpperBounds_,
                                         I3RandomService &random_
                                         )

:  random(random_)
,  nPar(nPar_)
,  funk(funk_)
,  vLowerBounds(vLowerBounds_)
,  vUpperBounds(vUpperBounds_)

{
    //default values:
    useDefaults();
}

////default ctor
//DSimulatedAnnealing::DSimulatedAnnealing(void) {
//    //default values:
//    useDefaults();
//}


// dtor
DSimulatedAnnealing::~DSimulatedAnnealing() {
    // nothing to do here :-)
}

void DSimulatedAnnealing::useDefaults(void) {
    //default values:
    nFcnEv    = 0;
    startTemp = 10000.0;
    epsTol    = 1e-5;
    rt        = 0.5;
    nMaxEval  = 50000;
    nS        = 20;
    nT        = 5;
    iPrint    = 0; // no printing
    rSeed     = 65539;
    iError    = 99;
    nEps      = 5;
}

void DSimulatedAnnealing::setNPar(int nPar_) {
    nPar = nPar_;
}

void DSimulatedAnnealing::setFunction(PtrToObjFunc funk_) {
    funk = funk_;
}

void DSimulatedAnnealing::setStartTemp(double temp_) {
    startTemp = temp_;
}

void DSimulatedAnnealing::setStartPar(const vector<double>& vXstart_) {
    vXstart=vXstart_;
}

void DSimulatedAnnealing::setStepSizes(const vector<double>& vVM_) {
    vVM=vVM_;
}

void DSimulatedAnnealing::setQuenchingFactor(double rt_) {
    rt = rt_;
}

void DSimulatedAnnealing::setNeps(int nEps_) {
    nEps = nEps_;
}

void DSimulatedAnnealing::setEpsTol(double epsTol_) {
    epsTol = epsTol_;
}

void DSimulatedAnnealing::setTCycle(int nT_) {
    nT = nT_;
}

void DSimulatedAnnealing::setSCycle(int nS_) {
    nS = nS_;
}

void DSimulatedAnnealing::setSeed(int rSeed_) {
    rSeed = rSeed_;
}

void DSimulatedAnnealing::setMaxEvals(int nMaxEval_) {
    nMaxEval = nMaxEval_;
}

void DSimulatedAnnealing::setBounds(const vector<double>& vLowerBounds_,
                                    const vector<double>& vUpperBounds_) {
    vLowerBounds = vLowerBounds_;
    vUpperBounds = vUpperBounds_;
}

void  DSimulatedAnnealing::setPrintLevel(int iPrint_) {
    iPrint = iPrint_;
}


// getters:

int DSimulatedAnnealing::getStatus(void) const {
    return iError;
}

double DSimulatedAnnealing::getTime(void) const {
    return miniTime;
}

long DSimulatedAnnealing::getNEval(void) const {
    return nFcnEv;
}

double DSimulatedAnnealing::getTemp(void) const {
    return temp;
}

double  DSimulatedAnnealing::getOptValue(void) const {
    return fopt;
}

const vector<double>& DSimulatedAnnealing::getOptPar(void) const {
    return vXopt;
}

const vector<double>& DSimulatedAnnealing::getStepVector(void) const {
    return vVM;
}


int DSimulatedAnnealing::minimise(void)
{
    double F, FP, P, PP;
    // double vars used in minimisation
    int nUp, nDown, nRej, nNew, LnOBDS;
    // int vars used in minimisation
    bool bQuit;
    // true if convergence occurred

    nAcc = 0;
    // number of accepted steps
    nOBDS = 0;
    // number of steps outside bounds
    nFcnEv = 0;
    // number of function evaluations

    // error code
    iError = 99;

    // check number of parameters
    if ((vUpperBounds.size()!=(unsigned int)nPar) or
        (vLowerBounds.size()!=(unsigned int)nPar)) {
        std::cerr << "ERROR: Check number of parameters!"
        << std::endl;
        iError = 6;
        return iError;
    }

    // check if starting position is set correctly
    if (vXstart.size()!=(unsigned int)nPar) {
        vXstart.clear();
        for(int i=0;i<nPar;i++) {
            vXstart.push_back(vLowerBounds[i]+
                              (vUpperBounds[i]-vLowerBounds[i])/2.0);
        }
    }

    if (vVM.size()!=(unsigned int)nPar) {
        vVM.clear();
        for(int i=0;i<nPar;i++) {
            vVM.push_back(vLowerBounds[i]+(vUpperBounds[i]-vLowerBounds[i])/2.0);
        }
    }

    vX = vXstart;

    // init parameters

    // need a clear here and there
    vXopt.clear();
    vNacp.clear();
    vC.clear();
    // vVM.clear();
    vXp.clear();

    for(int i=0; i<nPar; i++) {
        vXopt.push_back(vX[i]);
        vNacp.push_back(0);
        vC.push_back(2.0);
        // vVM.push_back(1.0);
        vXp.push_back(.0);

        // check bounds
        if((vX[i] > vUpperBounds[i]) or 
           (vX[i] < vLowerBounds[i])){
            iError = 2;
            return iError;
        }

    }

    vFstar.clear();
    for(int i=0; i<nEps; i++) {
        vFstar.push_back(1.0E20); // infinity ;-)
    }

    // check temp
    temp=startTemp;
    if(temp <= 0.0) {
        std::cerr << "T <= 0!" << std::endl;
        iError = 3;
        return iError;
    }

    F = funk(&vX);
    F = -F;
    nFcnEv++;
    fopt = F;
    vFstar[0] = F;


    myStopwatch watch;
    watch.Start();

    // do until convergence or
    // maxevals:
    for(;;){

        nUp = 0;
        nRej = 0;
        nNew = 0;
        nDown = 0;
        LnOBDS = 0;

        // temperature cycle
        for(int M=0; M<nT; M++) {

            // cycle with fixed step length
            for(int J=0; J<nS; J++) {

                // try steps in all directions
                for(int H=0; H<nPar; H++) {

                    // -----------------------------------

                    for(int i = 0; i<nPar; i++) {
                        // adjust h-th component ...
                        if(i == H) {
                            vXp[i] = vX[i] + 
                            //(myrand.next()*2. -1.) * vVM[i];
                            ((random.Uniform())*2. -1.) * vVM[i];
                        }
                        // ... and keep the others unchanged
                        else {
                            vXp[i] = vX[i];
                        }
                        // check if out of bounds
                        // if yes, put it back in allowed space
                        if((vXp[i] < vLowerBounds[i]) or
                           (vXp[i] > vUpperBounds[i])
                           ) {
                            vXp[i] = vLowerBounds[i] + 
                            (vUpperBounds[i] - vLowerBounds[i])
                            //* myrand.next();
                            * (random.Uniform());
                            LnOBDS++;
                            nOBDS++;
                        }
                    }

                    // --------------------------------

                    // evaluate function at new pos
                    FP = funk(&vXp);
                    FP = -FP;
                    nFcnEv++;

                    // --------------------------------

                    // check if maxevals is reached
                    if(nFcnEv >= nMaxEval) {
                        // terminated by maxevals
                        // set error code 1: diverge (not enough close hits)
                        // set error code 8: weak convergence (enough close hits)
                        iError = 8;
                        for(int i=0; i<nEps; i++){
                            if ( fabs(fopt-vFstar[i]) > epsTol ){
                                iError = 1;
                            }
                            if (iPrint >= 4){
                                std::cerr << "vF[" << i << "]=" << vFstar[i] << std::endl;
                            }
                        }
                        if (iPrint >= 4){
                            std::cerr << "epsTol=" << epsTol << std::endl;
                        }
                        fopt = -fopt;
                        watch.Stop();
                        miniTime=watch.CpuTime();
                        return iError;
                    }

                    // -------------------------------

                    if(FP >= F) {
                        // regular step
                        for (int i=0; i<nPar; i++){
                            vX[i] = vXp[i];
                        }
                        F = FP;
                        nAcc++;
                        vNacp[H]++;
                        nUp++;
                        if(FP > fopt){
                            if(iPrint >= 4){
                                std::cerr << "NEW OPTIMUM!" << std::endl;
                                std::cerr << "FP: " << FP << std::endl;
                            }
                            for(int i=0; i<nPar; i++){
                                vXopt[i] = vXp[i];
                            }
                            fopt = FP;
                            nNew++;
                        }
                    }
                    else {
                        // metropolis criterion
                        P = exp((FP-F)/temp);
                        //PP = myrand.next();
                        PP = random.Uniform();
                        if(PP < P){
                            // step is accepted by chance
                            for(int i=0; i<nPar;i++){
                                vX[i] = vXp[i];
                            }
                            F = FP;
                            nAcc++;
                            vNacp[H]++;
                            nDown++;
                        }
                        else{
                            nRej++;
                        }

                        //--------------------------------------

                    }

                }


            }  // end of fixed step length cycle

            // check if step length should be adjusted
            for(int i=0;i<nPar;i++){
                double ratio;
                ratio = static_cast<double>(vNacp[i])/static_cast<double>(nS);
                // compute the ratio of accepted steps to total steps
                // in i-th direction

                // if ratio is too large increase stepsize
                // and scan larger parts of landscape:
                if(0.6 < ratio){
                    vVM[i] = vVM[i]*(1.+vC[i]*(ratio -0.6)/0.4);
                }
                // if ratio is too small decrease stepsize
                // and take a closer look at a smaller region:
                else{
                    if(0.4 > ratio){
                        vVM[i] = vVM[i]/(1.0 + vC[i]*((0.4 - ratio)/0.4));
                    }
                }
                // but keep step size within bounds:
                if(vVM[i] > (vUpperBounds[i]-vLowerBounds[i])){
                    vVM[i] = vUpperBounds[i] - vLowerBounds[i];
                }
            }

            // clear for the next cycle:
            for(int i=0; i<nPar; i++){
                vNacp[i] = 0;
            }

        } // end of temperature cycle

        // convergence test:
        bQuit = false;
        vFstar[0] = F;
        // check if compatible with current best:
        if((fopt - vFstar[0]) <= epsTol)
            bQuit = true;
        // check if convergence level is reached
        // neps times:
        for(int i=0; i<nEps; i++){
            if(fabs(F-vFstar[i]) > epsTol)
                bQuit = false;
        }
        if(bQuit){ // if yes, then get out of here
            for(int i=0;i<nPar;i++){
                vX[i] = vXopt[i];
            }
            // normal convergence
            iError = 0;
            fopt = -fopt;
            watch.Stop();
            miniTime=watch.CpuTime();
            return iError;
        }

        temp = rt * temp;
        // cool baby

        // save neps latest results
        for(int i=nEps-1; i>0; i--){
            vFstar[i] = vFstar[i-1];
        }

        // reset starting point for new t-cycle
        // to current optimum:
        F = fopt;
        for(int i=0; i<nPar; i++){
            vX[i] = vXopt[i];
        }

    } // until convergence or maxevals

} // DSimulatedAnnealing::minimise()
