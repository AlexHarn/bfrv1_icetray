#ifndef MINIMIZERTESTFUNCTIONS_INCLUDED
#define MINIMIZERTESTFUNCTIONS_INCLUDED

#include <numeric>
#include <string>
#include <vector>

/// silly exception class
class OutOfBounds {
    public:
        OutOfBounds( const std::vector<double> &minval,
                     const std::vector<double> &parval,
                     const std::vector<double> &maxval ){
            assert(parval.size()==minval.size());
            assert(maxval.size()==minval.size());
            std::ostringstream oss;
            oss << "OUT OF BOUNDS ERROR: " << std::endl;
            for (unsigned int i=0; i<minval.size(); i++ ){
                oss << "par[" << i << "]=" << parval[i] << ": ";
                if ( minval[i]<maxval[i] ){
                    if (minval[i]>parval[i]){
                        oss << "ERROR (violates lower bound of " << minval[i] << ")";
                    } else if (maxval[i]<parval[i]){
                        oss << "ERROR (violates upper bound of " << maxval[i] << ")";
                    } else {
                        oss << "OK (within bounds)";
                    }
                    oss << std::endl;
                } else {
                    oss << "OK (no bounds)" << std::endl;
                }
            }
            message_ = oss.str();
        }
        ~OutOfBounds(){}
        std::string str() const {
            return message_;
        }
    private:
        std::string message_;
};

std::ostream& operator<<(std::ostream& os, const OutOfBounds &oob){
    return os << oob.str();
}


/****************************************************\
 * A bunch of test functions                        *
 * Some of them have a minimum, some have none      *
 * TODO: add a function with many fake local minima *
\****************************************************/

// a base function with some generic diagnostic utilities
class MinimizerTestFunction : public I3GulliverBase {
public:
    MinimizerTestFunction(bool hasmin,double mymin=NAN):
        nEval_(0),hasMinimum_(hasmin),myMinimum_(mymin),checkBounds_(false){}
    double operator()( const std::vector<double> &par,
                             std::vector<double> &grad ){
        if ( checkBounds_ ){
            BoundsCheck(par);
        }
        ++nEval_;
        return EvaluateWithGradient( par, grad );
    }
    virtual double EvaluateWithGradient( const std::vector<double> &par,
                                               std::vector<double> &grad ) = 0;
    double operator()( const std::vector<double> &par ){
        if ( checkBounds_ ){
            BoundsCheck(par);
        }
        ++nEval_;
        return Evaluate( par );
    }
    virtual double Evaluate( const std::vector<double> &par ) = 0;
    bool HasMinimum(){ return hasMinimum_; }
    double Minimum(){ return myMinimum_; }
    unsigned int GetNEval(){ return nEval_;}
    /**
     * This enables checking of bounds as specified in the specs.
     * Violation of bounds will be met with exceptional punishment.
     */
    void EnableBoundsCheck( const std::vector<I3FitParameterInitSpecs> &specs){
        checkBounds_ = true;
        parMin_.resize(specs.size());
        parMax_.resize(specs.size());
        std::vector<double>::iterator imin=parMin_.begin(), imax=parMax_.begin();
        for (std::vector<I3FitParameterInitSpecs>::const_iterator ispec = specs.begin(); ispec != specs.end(); ispec++ ){
            *(imin++) = ispec->minval_;
            *(imax++) = ispec->maxval_;
        }
    }
private:
    void BoundsCheck( const std::vector<double> &par ){
        std::vector<double>::const_iterator ipar = par.begin();
        std::vector<double>::iterator imin=parMin_.begin(), imax = parMax_.begin();
        while ( ipar != par.end() ){
            if ( *imin < *imax && (*imin>*ipar || *imax<*ipar) ){
                throw OutOfBounds(parMin_,par,parMax_);
            }
            ++imin;
            ++imax;
            ++ipar;
        }
    }
    unsigned int nEval_;
    std::vector<double> parMin_;
    std::vector<double> parMax_;
    bool hasMinimum_;
    double myMinimum_;
    bool checkBounds_;
};

// parabola: an infinite hole. just roll to the minimum. *should* be really easy(TM)
class InfHole : public MinimizerTestFunction {
public:
    InfHole( unsigned int dim):MinimizerTestFunction(true,0.0){}
    double EvaluateWithGradient( const std::vector<double> &par,
                                       std::vector<double> &grad ){
        double sum = 0.0;
        grad.resize(par.size());
        std::vector<double>::const_iterator ipar=par.begin();
        std::vector<double>::iterator igrad=grad.begin();
        for ( ; ipar != par.end(); ++ipar,++igrad ){
            double iparvalue=*ipar;
            sum += iparvalue*iparvalue;
            *igrad = 2.*iparvalue;
        }
        return sum;
    }
    double Evaluate( const std::vector<double> &par ){
        double sum = 0.0;
        std::vector<double>::const_iterator ipar;
        for ( ipar = par.begin(); ipar != par.end(); ++ipar ){
            sum += (*ipar)*(*ipar);
        }
        return sum;
    }
};

// flat (hyper)surface with a parabolic hole.
// find the hole in the tundra, then roll to its minimum...
class PotHole : public MinimizerTestFunction {
    // we need to smooth the edge of the pothole
    // r=sqrt(par[0]^2+par[1]^2 ...)
    // inner part of pothole: y=r^2 
    // edge part of pothole: y=ymax-a(r-b)^2
    // outer part of pothole (r>b): y=ymax
    // require that the function AND its derivative are continuous
    // choose some arbitrary value for a
    // (the larger a, the narrower the edge)
    // b=sqrt(ymax*(1+1/a))
    // r(inner->edge)=b/(1+1/a)=sqrt(ymax/(1+1/a))
    const double ymax_;
    const double a_;
    const double b2_;
    const double b_;
    const double rie2_;

public:
    PotHole( unsigned int dim ):
        MinimizerTestFunction(true,0.0),
        ymax_(10.), a_(5.), b2_(ymax_*(1.+1./a_)), b_(sqrt(b2_)), rie2_(ymax_/(1.+1./a_)){}
    double Evaluate( const std::vector<double> &par ){
        double sum = 0.0;
        std::vector<double>::const_iterator ipar;
        for ( ipar = par.begin(); ipar != par.end(); ++ipar ){
            sum += (*ipar)*(*ipar);
        }
        if ( sum > b2_ ){
            sum = ymax_;
        } else if ( sum > rie2_ ){
            double r=sqrt(sum);
            sum = ymax_ - a_*(b_-r)*(b_-r);
        }
        return sum;
    }
    double EvaluateWithGradient( const std::vector<double> &par,
                                       std::vector<double> &grad ){

        double sum = 0.0;
        grad.resize(0);
        grad.resize(par.size(),0.);
        std::vector<double>::const_iterator ipar = par.begin();
        for ( ; ipar != par.end(); ++ipar ){
            sum += (*ipar)*(*ipar);
        }
        if ( sum >= b2_ ){
            sum = ymax_;
        } else {
            std::vector<double>::iterator igrad = grad.begin();
            ipar = par.begin();
            if ( sum <= rie2_ ){
                for ( ; ipar != par.end(); ++ipar, ++igrad ){
                    *igrad = 2*(*ipar);
                }
            } else {
                double r=sqrt(sum);
                double gradmag=2.*a_*(b_-r);
                sum = ymax_ - a_*(b_-r)*(b_-r);
                for ( ; ipar != par.end(); ++ipar, ++igrad ){
                    *igrad = gradmag*(*ipar)/r;
                }
            }
        }
        return sum;
    }
};

// flat (hyper)surface: there is no minimum
// (well, mathematically there is, but for our purposes there isn't.)
class JustFlat : public MinimizerTestFunction {
public:
    JustFlat( unsigned int dim ):MinimizerTestFunction(false,NAN){}
    double Evaluate( const std::vector<double> &par ){
        return 10.0;
    }
    double EvaluateWithGradient( const std::vector<double> &par,
                                       std::vector<double> &grad ){
        grad.resize(0);
        grad.resize(par.size(),0.);
        return 10.0;
    }
};

// flat (hyper)surface: there is no minimum
class Slope : public MinimizerTestFunction {
private:
    std::vector<double> support_;
public:
    Slope( unsigned int dim ):MinimizerTestFunction(false,NAN){
        for ( unsigned int i=0; i<dim; ++i ){
            support_.push_back(i - 0.45*dim + 0.57 );
            log_trace("slope support vector [%d] is %g",i,support_.back());
        }
    }
    double Evaluate( const std::vector<double> &par ){
        assert( par.size() == support_.size() );
        return std::inner_product(par.begin(),par.end(),support_.begin(),0.);
    }
    double EvaluateWithGradient( const std::vector<double> &par,
                                       std::vector<double> &grad ){
        assert( par.size() == support_.size() );
        grad.assign( support_.begin(),support_.end() );
        return std::inner_product(par.begin(), par.end(), support_.begin(), 0.);
    }
    SET_LOGGER("Slope");
};


#endif /* MINIMIZERTESTFUNCTIONS_INCLUDED */
