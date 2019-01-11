// ======================================================================
//  DSimulatedAnnealing.h   - Last modif: Mo 10 Sep 2007 18:15:29 CEST -
//  $Id$
//  Ralf  Auer, auer(at)physik.uni-erlangen.de
//  Felix Fehr, fehr(at)physik.uni-erlangen.de
// ======================================================================

/******************************************************
 *                                                    *
 * C++ implementation of the "Simulated Annealing"    *
 * algorithm.                                         *
 * Note that throughout the class double precision    *
 * is used, although it's slower than with single.    *
 *                                                    *
 ******************************************************/

//  How to minimise functions with DSimulatedAnnealing
// ======================================================================
//
//  The class requires the number of parameters, the boundaries of the
//  parameter space and a pointer to the objective function.
//  This function can be a global function or a member function of a
//  class object. It must take a const std::vector<double> as an
//  argument and return a doubleing point number.
//
//  Here's an example of a proper initialisation:
//
//  double ourObjectiveFunction(const std::vector<double>* x) { ... };
//  int nPar = 5;
//  std::vector vLowerBounds;
//  std::vector vUpperBounds;
//  // feed the vector with boundaries here ...
//  DSimulatedAnnealing::PtrToObjFunc f = &ourObjectiveFunction;
//  DSimulatedAnnealing sa(nPar,f,vLowerBounds,vUpperBounds);
//
//  (In this case
//    DSimulatedAnnealing sa(nPar,&ourObjectiveFunction,
//      vLowerBounds,vUpperBounds
//    );
//  is also OK.)
//
//  Working with bound member functions is only slighty more complicated.
//  For an object myClass of class MYCLASS with public(!) member function
//  (say double memFun(std::vector<double>* x) {...} ) this would read:
//
//  MYCLASS myClass;
//  DSimulatedAnnealing::PtrToObjFunc f
//    = std::bind1st(std::mem_fun(&MYCLASS::memFun),&myClass);
//
//  Inside the class declaration, the "this" pointer can be used:
//
//  class MYCLASS {
//    ...
//    double memfun(const std::vector<double>*) {...};
//    DSimulatedAnnealing::PtrToObjFunc f
//      = std::bind1st(std::mem_fun(&MYCLASS::memfun),this);
//    ...
//  };
//
//  It's also possible to use the standard constructor and to set
//  the details afterwards.
//
//  DSimulatedAnnealing sa;
//  sa.setNPar(nPar);
//  sa.setBounds(vLowerBounds,vUpperBounds);
//  sa.setFunction(f);
//
//  A starting point for the random walk can be set using
//
//  sa.setStartPar(xStart);
//
//  where xStart is an std::vector<double> of dimension nPar.
//  If no starting point is specified, it is automatically set to the
//  middle of the parameter space.
//  The minimisation routine is called with:
//
//  sa.minimise();
//
//  The minimised function value and the optimal params, can then be
//  obtained with:
//
//  double          fopt   = sa.getOptValue();
//  vector<double>& params = sa.getOptPar();
//
//  The CPU time for the minimisation process and the status of the
//  minimisation are available via the members
//
//  double miniTime = sa.getTime();
//  int   miniStat = sa.getStatus();
//
//  The status method returns the status of the minimisation.
//  The following codes are used:
//
//  99 - ERROR:   unknown exception
//  06 - ERROR:   parameters ill defined
//  03 - ERROR:   temperature ill defined
//  02 - ERROR:   boundaries ill defined 
//  01 - WARNING: minimisation terminated by maxevals
//  00 - SUCCESS: regular convergence
//
//  The two most important parameters of the algorithm are the initial
//  temperature T and the cooling factor rt. You can adapt them to your
//  specific problem with:
//
//  double T = 100000.0, rt = 0.85;
//  sa.setStartTemp(T);
//  sa.setQuenchingFactor(rt);
//
//  There are more parameters which can be adjusted - have a look at the
//  class declaration below.
//  If not set, default values are chosen.
//
//  You can contact us for further information.
// ======================================================================

#ifndef _SIMULATED_ANNEALING_
#define _SIMULATED_ANNEALING_

// std
#include <vector>
// boost
#include <boost/function.hpp> 
// boost::function1 is used together with std::bind1st()
// and std::mem_fun() as a wrapper for a bound member
// function call, the implementation is transparent, i.e. it
// works as well with a global function

// this does not really belong here and should be abstraced away.
// #include "phys-services/I3RandomService.h"
class I3RandomService;

// Declaration of class DSimulatedAnnealing 

class DSimulatedAnnealing {
    // Simulated Annealing optimisation algorithm
    // ----------------------------------------------------------------------
public:
    // define a short-hand for objective function pointer type
    typedef boost::function1<double, const std::vector<double>* >
    PtrToObjFunc;

    // ctor:
    DSimulatedAnnealing(
                        int nPar_,                                     // num of.parameters
                        PtrToObjFunc funk_,                            // obj. function
                        const std::vector<double>& vLowerBounds_,       // lower bounds
                        const std::vector<double>& vUpperBounds_,       // upper bounds
                        I3RandomService &random_
                        );
    //DSimulatedAnnealing(void);

    // dtor:
    ~DSimulatedAnnealing();

    // minimise function:

    // all the hard work is done here :-)
    int minimise(void);

    void useDefaults(void);
    // (re)set default options

    // setters:

    void setNPar(int nPar_);
    // sets number of parameters
    void setStartTemp(double temp_); 
    // sets the initial temperature for the random walk
    void setStartPar(const std::vector<double>& vXstart_);
    // sets the initial parameters for the random walk
    void setStepSizes(const std::vector<double>& vVM_);
    // sets the initial stepsizes for the random walk
    void setQuenchingFactor(double rt_);
    // sets the cooling factor of the temperature
    void setNeps(int nEps_);
    // sets the number of for convergence
    void setEpsTol(double epsTol_);
    // sets the convergence tolerance
    void setTCycle(int nT_);
    // set steps at fixed temperature
    void setSCycle(int nS_);
    // set steps with fixed step length
    void setSeed(int rSeed_);
    // set seed for random generator
    void setMaxEvals(int nMaxEvals_);
    // limits the random walk to a maximum of funct. evaluations
    void setBounds(const std::vector<double>& vLowerBounds,
                   const std::vector<double>& vUpperBounds
                   );
    // sets the boundaries for the walk
    void setPrintLevel(int iPrint_);
    // sets the print level at which info is thrown at the user
    void setFunction(PtrToObjFunc funk_);
    // expects a boost::function1 wrapper of objective function

    // getters:

    int getStatus(void) const;
    // returns the status of minimisation
    long getNEval(void) const;
    // returns the number of function evaluations
    double getTemp(void) const;
    // returns temperature (at the end of minimisation)
    double getTime(void) const;
    // returns minimisation time in seconds
    double getOptValue(void) const;
    // returns the best value at the end of walk
    const std::vector<double>& getOptPar(void) const;
    // returns the end position (optimal parameters) at
    // the end of the walk
    const std::vector<double>& getStepVector(void) const;
    // returns the step size vector

    // ----------------------------------------------------------------------
private:
    I3RandomService &random;

    int nPar;
    // number of parameters to optimise
    PtrToObjFunc funk;
    // the objective function is wrapped using
    // boost::function1. This allow to use
    // object member functions
    int nEps;
    // parameter for early convergence
    // specifies how many times tolerance
    // level must be reached for convergence
    int nS;
    // number of cycles before v-adaption
    int nT;
    // number of v-adaption before cooling
    long nFcnEv;
    // number of function evaluations
    double miniTime;
    // time spend to minimise function
    int iError;
    // error code
    int nMaxEval;
    // maximal number of function evals
    int iPrint;
    // defines print level
    int rSeed;
    // seed for random number generator
    int nAcc;
    // number of accepted steps
    int nOBDS;
    // number of steps outside space
    std::vector<double> vLowerBounds;
    // vector containing lower bounds of par. space
    std::vector<double> vUpperBounds;
    // vector containing upper bounds of par. space
    std::vector<double> vXopt;
    // vector of optimised parameters
    std::vector<double> vC;
    // defines the scaling length of stepsize vector
    std::vector<double> vVM;
    // stepsize vector
    std::vector<double> vFstar;
    // neps latest function values;
    std::vector<double> vXp;
    // current parameter values
    std::vector<double> vX;
    // vector with parameters
    std::vector<double> vXstart;
    // initial parameters
    std::vector<int> vNacp;
    // number of accepted steps in i-th direction
    double startTemp;
    // initial temperature
    double temp;
    // temperature
    double epsTol;
    // convergence level
    double rt;
    // quenching factor
    double fopt;
    // optimised function value
}; // end of class declaration: DSimulatedAnnealing

#endif // _SIMULATED_ANNEALING_
