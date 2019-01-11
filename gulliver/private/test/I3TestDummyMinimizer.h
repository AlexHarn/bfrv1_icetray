#ifndef GULLIVER_I3TESTDUMMYMINIMIZER_H_INCLUDED
#define GULLIVER_I3TESTDUMMYMINIMIZER_H_INCLUDED
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iterator>
#include <numeric>
#include "gulliver/I3MinimizerBase.h"


// heck, let's also define our own minimizer
// normally you'd take a standard one
// this dummy one just follows the gradient, taking smaller and smaller steps
class I3TestDummyMinimizer : public I3MinimizerBase {
    protected:
        double tolerance_;
        const double magic_;
        unsigned int maxiterations_;
        bool use_gradient_;
        std::string name_;
        SET_LOGGER("I3TestDummyMinimizer");
    public:
        I3TestDummyMinimizer( double t=0.000001, double m=0.5,
                              int i=10000, bool g=false ):
            I3MinimizerBase(), tolerance_(t), magic_(m), maxiterations_(i),
            use_gradient_(g), name_("DummyMinimizer"){ }
        ~I3TestDummyMinimizer(){}
        bool UsesGradient(){
            return use_gradient_;
        }
        void SetMaxIterations( unsigned int newmaxi ){
            maxiterations_ = newmaxi;
        }
        unsigned int GetMaxIterations( ) const {
            return 0;
        }
        void SetTolerance( double newtol ){
            tolerance_ = newtol;
        }
        double GetTolerance( ) const {
            return tolerance_;
        }
        const std::string GetName() const {
            return name_;
        }
        I3MinimizerResult Minimize( I3GulliverBase &gullifunctor,
                                    const std::vector<I3FitParameterInitSpecs> &parinit ){
            if ( use_gradient_ ){
                return MinimizeWithGradient( gullifunctor, parinit );
            }
            unsigned int npar = parinit.size();
            vector<double> par(npar,0.);
            vector<double> newpar(npar,0.);
            vector<double> delta(npar,0.);
            vector<double> step(npar,0.);
            vector<int> ibest(npar,0);
            for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                newpar[ipar] = par[ipar] = parinit[ipar].initval_;
                step[ipar] = parinit[ipar].stepsize_;
            }
            double f=0, fnew=0;
            f = gullifunctor( par );
            bool converged = false;
            unsigned int iteration;
            const int smax = 10;
            for ( iteration = 0; iteration < maxiterations_; ++iteration ){
                double totaldelta = 0;
                bool dip = true;
                for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                    double fbest = f;
                    // walk forward, backward
                    newpar[ipar] = par[ipar] + step[ipar];
                    double fplus = gullifunctor( newpar );
                    newpar[ipar] = par[ipar] - step[ipar];
                    double fminus = gullifunctor( newpar );
                    ibest[ipar]=0;
                    // check which one is better
                    int sign = 1;
                    if ( fplus < fminus ){
                        fnew = fplus;
                    } else {
                        fnew = fminus;
                        sign = -1;
                    }
                    // check if any of them is actually an improvement
                    if ( fnew < f ){
                        fbest = fnew;
                        ibest[ipar] = sign*1;
                        for ( int istep=2; istep<=smax; istep++ ){
                           newpar[ipar] = par[ipar] + sign*istep*step[ipar];
                           fnew = gullifunctor( newpar );
                           if ( fnew > fbest ) break;
                           fbest = fnew;
                           ibest[ipar] = sign * istep;
                        }

                        // update
                        newpar[ipar] = par[ipar] + ibest[ipar] * step[ipar];
                        par[ipar] = newpar[ipar];
                        log_trace( "MOVE %d. %g(%g) -> %g(%g) step=%g ibest=%i",
                                   ipar,
                                   par[ipar], f,
                                   par[ipar] + ibest[ipar]*step[ipar], fbest,
                                   step[ipar], ibest[ipar] );
                        f = fbest;
                    } else {
                        log_trace( "KEEP %d. %g(%g)", ipar, par[ipar], f );
                    }

                    // update stepsize
                    step[ipar] *= (abs(ibest[ipar])<smax) ? magic_: 1.0/magic_;

                    // compare
                    newpar[ipar] = par[ipar] + step[ipar];
                    fplus = gullifunctor( newpar );
                    newpar[ipar] = par[ipar] - step[ipar];
                    fminus = gullifunctor( newpar );
                    newpar[ipar] = par[ipar];
                    totaldelta += (fplus-f)*(fplus-f) + (fminus-f)*(fminus-f);
                    if ( ( fplus < f ) || ( fminus < f ) ){
                        dip = false;
                    }
                }

                totaldelta=sqrt(totaldelta);
                log_trace( "DIP: %s totaldelta=%g", (dip?"YES":"NO"), totaldelta );
                if ( dip && (totaldelta < tolerance_) ){
                    converged = true;
                    break;
                }

            }
            I3MinimizerResult m(npar);
            m.converged_ = converged;
            // m.iterations_ = iteration;
            if ( converged ){
                log_trace( "CONVERGED after %u iterations", iteration );
                m.par_ = par;
            } else {
                m.par_ = vector<double>(npar,NAN);
                log_trace( "FAILED TO CONVERGE after %u iterations", iteration );
            }
            m.minval_ = f;
            return m;
        }

        // static double sumsquare(double sum, double x){ return sum+x*x;}


        const std::string& print_par(const std::vector<double> &v ){
            static std::string pstring;
            std::ostringstream oss;
            copy(v.begin(),v.end(),std::ostream_iterator<double>(oss,", "));
            pstring = oss.str();
            return pstring;
        }

        void check_optimum( I3GulliverBase &g, double f,
                            std::vector<double> &par,
                            const std::vector<double> &step, double &delta ){
            std::string spar = print_par(par).c_str();
            std::string sstep = print_par(step).c_str();
            log_trace( "checking optimum at f=%g par=%s step=%s",
                      f, spar.c_str(), sstep.c_str());
            unsigned int npar = par.size();
            std::vector<double> newpar(par);
            std::vector<double> newgrad(npar,0.);
            int istep = 0;
            delta = 0.;
            for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                for ( int i=-1; i<=1; i+= 2){
                   newpar[ipar] = par[ipar] + i*step[ipar];
                   double newf = g(newpar,newgrad);
                   double newdelta = newf - f;
                   log_trace("f=%g newf=%g delta=%g", f, newf, newdelta );
                   if ( newdelta > 0. ){
                       // neighbor is worse
                       if ( ( delta >= 0. ) && ( newdelta > delta ) ){
                           delta = newdelta;
                       }
                   } else if ( newdelta < 0. ){
                       // neighbor is better!
                       if (  newdelta < delta ){
                           delta = newdelta;
                           istep = i * (1+ipar);
                       }
                   }
                }
                newpar[ipar] = par[ipar];
            }
            if ( delta < 0. ){
                // not a minimum! get away from there!
                int ipar = abs(istep) - 1;
                int isign = (istep<0)?-1:1;
                log_trace( "not a minimum; steepest descent in direction %s%d",
                          ((istep>0)?"+":"-"), ipar );
                par[ipar] += isign * step[ipar];
            } else if ( delta == 0. ){
                log_trace( "not a minimum; looks like a big flat" );
            } else {
                log_trace( "YES: a minimum (looks like)." );
            }
        }

        /*
         * some random hackish variation on the steepest descent method
         */
        I3MinimizerResult MinimizeWithGradient(
                I3GulliverBase &gullifunctor,
                const std::vector<I3FitParameterInitSpecs> &parinit ){
            unsigned int npar = parinit.size();
            std::vector<double> par(npar,0.);
            std::vector<double> grad(npar,0.);
            std::vector<double> newpar(npar,0.);
            std::vector<double> newgrad(npar,0.);
            std::vector<double> step(npar,0.);
            std::vector<double> smallstep(npar,0.);
            for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                newpar[ipar] = par[ipar] = parinit[ipar].initval_;
                step[ipar] = parinit[ipar].stepsize_;
                smallstep[ipar] = tolerance_*parinit[ipar].stepsize_;
            }

            // get initial function value and gradient
            double f= gullifunctor( par, grad );
            std::string spar = print_par(par).c_str();
            std::string sstep = print_par(step).c_str();
            std::string sgrad = print_par(grad).c_str();
            log_trace( "START: f=%g\npar=%s\nstep=%s\ngrad=%s",
                      f, spar.c_str(), sstep.c_str(), sgrad.c_str() );

            // try to find an initial scale factor
            // such that scale*gradient is about one stepsize
            double invscale = 0.;
            for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                invscale += fabs(grad[ipar])/step[ipar];
            }
            double delta=0.;
            I3MinimizerResult m(npar);
            if ( invscale <= 0. ){
                log_trace( "START: gradient is NULL!!!!" );
                check_optimum(gullifunctor,f,par,smallstep,delta);
                if ( delta > 0 ){
                    // gamble: seed was already optimum!
                    m.converged_ = true;;
                    m.minval_ = f;
                    m.par_ = par;
                } else {
                    // bummer
                    m.converged_ = false;;
                }
                return m;
            }
            double scale = npar/invscale;
            log_trace( "START: invscale=%g scale=%g", invscale/npar, scale );

            bool converged = false;

            unsigned int iteration;
            for ( iteration = 0; iteration < maxiterations_; ++iteration ){
                double stepsize=0.;
                for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                    double change = - scale * grad[ipar];
                    newpar[ipar] = par[ipar] + change;
                    stepsize += fabs(change);
                }
                stepsize /= npar;
                double fnew= gullifunctor( newpar, newgrad );
                std::string spar  = print_par(par).c_str();
                std::string sstep = print_par(newpar).c_str();
                std::string sgrad = print_par(newgrad).c_str();
                log_trace( "ITERATION %3d: fnew=%g scale=%g\npar=%s\nnewpar=%s\newgrad=%s",
                          iteration, fnew, scale,
                          spar.c_str(), sstep.c_str(), sgrad.c_str());
                /*
                if ( fnew < f ){
                    log_trace( "IMPROVEMENT, continuing" );
                    scale /= magic_;
                    par=newpar;
                    f = fnew;
                } else {
                    log_trace( "NO IMPROVEMENT, shrinking" );
                    scale *= magic_;
                }
                if ( stepsize < tolerance_ ){
                    check_optimum(gullifunctor,f,par,smallstep,delta);
                    if ( delta > 0 ){
                        log_trace("converged after %d iterations", iteration);
                        converged = true;
                        break;
                    }
                }
                continue;
                */
                /********************************************/

                double gradprod = 0.;
                double gradsquare = 0.;
                for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                    gradprod += newgrad[ipar] * grad[ipar];
                    gradsquare += grad[ipar] * grad[ipar];
                }
                if ( gradsquare <= 0. ){
                    delta = 0.;
                    check_optimum(gullifunctor,f,par,smallstep,delta);
                    if ( delta>0 ){
                        log_trace("converged after %d iterations", iteration);
                        converged = true;
                        break;
                    } else if ( delta==0. ){
                        log_trace("got stuck after %d iterations", iteration);
                        converged = false; // got stuck on a plane
                        break;
                    } else {
                        // We were stuck on a plane or local max,
                        // but found a way out.
                        log_trace("managed to escape from a local flat after "
                                 "%d iterations", iteration);
                        continue;
                    }
                }
                double r = gradprod/gradsquare;
                log_trace("%3d. gradprod=%g  gradsquare=%g ratio=%g",
                          iteration,gradprod, gradsquare,r);
                if (  r <= 0 ){
                    // gradients are opposing or perpendicular to eachother:
                    // too large step
                    log_trace("scaling back to scale=%g",scale);
                    scale *= 0.5;
                    continue;
                } else if (  (r > magic_)  && (r < 1.0/magic_) ){
                    r = 1.0/magic_;
                } else if (  10. < r ){
                    r = 10.;
                    log_trace("scaling back to ratio=%g",r);
                }
                int np = 3;
                double bestnewscale=0;
                double fbest=f;
                double normsteps=0.;
                double scale_factor=pow(r,1.0/np);
                double newscale=scale/scale_factor;
                for ( int ip = -1; ip<=np; ++ip ){
                    newscale *= scale_factor;
                    if ( ip == 0 ) continue;
                    for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                        newpar[ipar] = par[ipar] - newscale * grad[ipar];
                    }
                    fnew= gullifunctor( newpar, newgrad );
                    if ( fnew < fbest ){
                        fbest = fnew;
                        par = newpar;
                        grad = newgrad;
                        bestnewscale = newscale;
                    }
                }
                if ( bestnewscale > 0 ){
                    log_trace( "%d. changing scale %g -> %g",
                              iteration, scale, bestnewscale );
                    scale = bestnewscale;
                } else {
                    log_trace( "%d. keeping scale %g",
                              iteration, scale );
                }
                f = fbest;
                for ( unsigned int ipar=0; ipar<npar; ipar++ ){
                    normsteps = scale * grad[ipar] / step[ipar];
                }
                normsteps /= npar;
                if ( normsteps < tolerance_ ){
                    delta = 0.;
                    check_optimum(gullifunctor,f,par,smallstep,delta);
                    if ( delta>0 ){
                        log_trace("converged after %d iterations", iteration);
                        converged = true;
                        break;
                    } // else: continue
                }
            }
            if ( ! converged ){
                check_optimum(gullifunctor,f,par,smallstep,delta);
                if ( delta > 0 ){
                    log_trace( "converged after %d iterations", maxiterations_ );
                    converged = true;
                }
            }

            m.converged_ = converged;
            m.minval_ = f;
            m.par_ = par;
            return m;
        }
};
typedef boost::shared_ptr<I3TestDummyMinimizer> I3TestDummyMinimizerPtr;
#endif /* GULLIVER_I3TESTDUMMYMINIMIZER_H_INCLUDED */
