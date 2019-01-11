
#ifndef I3GULLIVERMN_H_INCLUDED
#define I3GULLIVERMN_H_INCLUDED

#include "gulliver/I3MinimizerBase.h"

#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"

#include <multinest.h>

class I3GulliverMN : public I3ServiceBase, public I3MinimizerBase {
public:
	/// constructor for service factory
	I3GulliverMN(const I3Context& context);

	/// destructor
	virtual ~I3GulliverMN();

	/// configuration
	void Configure();
	
        /// set tolerance and other minimizer parameters
        double GetTolerance() const { return tolerance_; }
        void SetTolerance(double newtol ){ tolerance_ = newtol; }
        unsigned int GetMaxIterations() const { return maxiter_; }
        void SetMaxIterations(unsigned int newmaxi ){ maxiter_ = newmaxi; }
        const std::string GetName() const {
            return I3ServiceBase::GetName();
        }
        bool UsesGradient() { return false; };
	
	/// core method: minimizer a given function with given initial conditions
	I3MinimizerResult Minimize(I3GulliverBase &g, const std::vector<I3FitParameterInitSpecs> &parspecs );

    /// required by MN structure of minimizer
    static void fct(double* Cube, int &ndim, int &npars, double &lnew, void* misc);
    static void InfoDumper(  int &nSamples, int &nlive, int &nPar,
                                    double **physLive, double **posterior, 
                                    double **paramConstr, double &maxLogLike, 
                                    double &logZ, double &INSlogZ, double &logZerr, void * misc);    
private:
	SET_LOGGER("I3GulliverMN");

    std::vector<double> upperBnds_, lowerBnds_;
    std::vector<double> params_bestFit;
    double llh_bestFit_, llh_worstFit_;
    std::vector<std::string> periodics_, modeSeparated_; 
    std::vector<int> paramOrder_; 
    
    int ins_, mmodal_, ceff_, nlive_, maxmodes_, updint_, rseed_, feedback_, resume_, writefiles_, init_mpi_, maxiter_;
    double efr_, Ztol_, logZero_, tolerance_;  
    std::string the_path;
    
    I3GulliverBase* g_;
};

#endif

