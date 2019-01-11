/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhBrent.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/minimizer/I3CscdLlhBrent.h"

#include <iostream>
#include <climits>
#include <limits>

using namespace std;


/* ****************************************************** */
/* constructors                                           */
/* ****************************************************** */
I3CscdLlhBrent::I3CscdLlhBrent() : I3CscdLlhMinimizer() 
{
  log_debug("Enter I3CscdLlhBrent::I3CscdLlhBrent()");
  Init(MAX_MINIMIZATION_PARAMS);
  log_debug("Exit I3CscdLlhBrent::I3CscdLlhBrent()");
}

I3CscdLlhBrent::I3CscdLlhBrent(int maxParams) :
  I3CscdLlhMinimizer(maxParams) 
{
  log_debug("Enter I3CscdLlhMinuit::I3CscdLlhBrent(int)");
  Init(maxParams);
  log_debug("Exit I3CscdLlhMinuit::I3CscdLlhBrent(int)");
}

/* ****************************************************** */
/* Init                                                   */
/* ****************************************************** */
void I3CscdLlhBrent::Init(int maxParams) 
{
  log_debug("Entering I3CscdLlhBrent::Init()");

  idxFreeParam_ = -1;

  fitParams_ = new double[maxParams];
  for (int i=0; i<maxParams; i++)
    fitParams_[i] = NAN;

  log_debug("Exiting I3CscdLlhBrent::Init()");
  return;
} // end Init

/* ****************************************************** */
/* destructor                                             */
/* ****************************************************** */ 
I3CscdLlhBrent::~I3CscdLlhBrent() 
{
  delete fitParams_;
}

/* ****************************************************** */
/* Minimize: Perform the actual minimization.             */
/* ****************************************************** */ 
bool I3CscdLlhBrent::Minimize(list<I3CscdLlhHitPtr>* hits,
  I3CscdLlhAbsPdfPtr pdf) 
{
  log_debug("Entering I3CscdLlhBrent::Minimize()");

  if (!I3CscdLlhMinimizer::Minimize(hits, pdf)) 
    return false;

  if (numFreeParams_ != 1)
    log_error("The Brent method only works with one free parameter!");

  DP ax = NAN;
  DP bx = NAN;
  DP cx = NAN;
  for (int i=0; i<numParams_; i++) 
  {
    fitParams_[i] = seed_[i];

    if (!fixParam_[i]) 
    {

      ax = seed_[i] - stepSize_[i];
      bx = seed_[i] + stepSize_[i];
      idxFreeParam_ = i;
    }
  } // for i

  log_debug("Seed bracket: (%.4e, %.4e)", ax, bx);

  DP fa = NAN;
  DP fb = NAN;
  DP fc = NAN;
  DP xmin = NAN;
  Mnbrak(ax, bx, cx, fa, fb, fc);
  log_debug("Starting bracket: (%.4e, %.4e, %.4e)", ax, bx, cx);

  DP funcMin = Brent(ax, bx, cx, xmin);
  log_debug("Final bracket: (%.2e, %.2e, %.2e)", ax, bx, cx);
  log_debug("Function minimum: (%.2e, %.2e)", xmin, funcMin);
  log_debug("Brent function calls: %d.", functionCalls_);

  result_->status = status_;
  if (status_) 
  {
    // Clear out the static hits_ and pdf_, just to make sure
    // they won't be used by the wrong Minimizer.
    hits_ = 0;
//    pdf_.Reset();

    log_debug("Minimization failed! [status: %d]", status_);
    return false;
  }

  result_->negLlh = funcMin;
  log_debug("Negative log-likelihood: %.2f", result_->negLlh);

  CopyFitParams(xmin);

  // Clear out the static hits_ and pdf_, just to make sure
  // they won't be used by the wrong Minimizer.
  hits_ = 0;
//  pdf_.Reset();

  log_debug("Exiting I3CscdLlhBrent::Minimize()");
  return true;
} // end Minimize

/* ****************************************************** */
/* copyFitParams                                          */
/* ****************************************************** */
void I3CscdLlhBrent::CopyFitParams(const DP xmin) 
{
  log_debug("Entering I3CscdLlhBrent::copyFitParams()");

  int idx;

  idx = pdf_->GetParamIndex("t");
  if (idx != INT_MIN) 
  {
    result_->t = (idx == idxFreeParam_ ? xmin : fitParams_[idx]);
  }

  idx = pdf_->GetParamIndex("x");
  if (idx != INT_MIN) 
  {
    result_->x = (idx == idxFreeParam_ ? xmin : fitParams_[idx]);
  }

  idx = pdf_->GetParamIndex("y");
  if (idx != INT_MIN) 
  {
    result_->y = (idx == idxFreeParam_ ? xmin : fitParams_[idx]);
  }

  idx = pdf_->GetParamIndex("z");
  if (idx != INT_MIN) 
  {
    result_->z = (idx == idxFreeParam_ ? xmin : fitParams_[idx]);
  }

  idx = pdf_->GetParamIndex("zenith");
  if (idx != INT_MIN) 
  {
    result_->theta = (idx == idxFreeParam_ ? xmin : fitParams_[idx]);
  }

  idx = pdf_->GetParamIndex("azimuth");
  if (idx != INT_MIN) 
  {
    result_->phi = (idx == idxFreeParam_ ? xmin : fitParams_[idx]);
  }

  idx = pdf_->GetParamIndex("energy");
  if (idx != INT_MIN) 
  {
    result_->energy = (idx == idxFreeParam_ ? xmin : fitParams_[idx]);
  }

  log_debug("Exiting I3CscdLlhBrent::CopyFitParams()");
  return;
} // end CopyFitParams

/* ****************************************************** */
/* Func: The function to be Minimized:                    */
/*      the negative log likelihood.                      */
/* ****************************************************** */
DP I3CscdLlhBrent::Func(const DP x) {

  functionCalls_++;

  fitParams_[idxFreeParam_] = x;
  
  double negLlh = 0.0;
  for(list<I3CscdLlhHitPtr>::iterator iter = I3CscdLlhBrent::hits_->begin();
               iter != I3CscdLlhBrent::hits_->end(); iter++) {
      
    I3CscdLlhHitPtr& hit = *iter;

    double prob = NAN;
    I3CscdLlhBrent::pdf_->Evaluate(hit, fitParams_, prob);
    log_trace("Probability: %f", prob);

    if (std::isnan(prob) || prob <= 0.0) {
      log_debug("Probability is negative, zero or NAN! [%f]", prob);
      return NAN;
    }

    negLlh -= hit->weight*log(prob);
  } // for each hit

  return negLlh; 
} // end func

/* ****************************************************** */
/* Mnbrak:                                                */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
void I3CscdLlhBrent::Mnbrak(DP &ax, DP &bx, DP &cx,
          DP &fa, DP &fb, DP &fc) {

  const DP GOLD=1.618034,GLIMIT=100.0,TINY=1.0e-20;
  DP ulim,u,r,q,fu;

  fa=Func(ax);
  fb=Func(bx);
  if (fb > fa) {
    std::swap(ax,bx);
    std::swap(fb,fa);
  }
  cx=bx+GOLD*(bx-ax);
  fc=Func(cx);
  while (fb > fc) {
    r=(bx-ax)*(fb-fc);
    q=(bx-cx)*(fb-fa);
    u=bx-((bx-cx)*q-(bx-ax)*r)/
      (2.0*SIGN(std::max(fabs(q-r),TINY),q-r));
    ulim=bx+GLIMIT*(cx-bx);
    if ((bx-u)*(u-cx) > 0.0) {
      fu=Func(u);
      if (fu < fc) {
        ax=bx;
        bx=u;
        fa=fb;
        fb=fu;
        return;
      } else if (fu > fb) {
        cx=u;
        fc=fu;
        return;
      }
      u=cx+GOLD*(cx-bx);
      fu=Func(u);
    } else if ((cx-u)*(u-ulim) > 0.0) {
      fu=Func(u);
      if (fu < fc) {
        Shft3(bx,cx,u,u+GOLD*(u-cx));
        Shft3(fb,fc,fu,Func(u));
      }
    } else if ((u-ulim)*(ulim-cx) >= 0.0) {
      u=ulim;
      fu=Func(u);
    } else {
      u=cx+GOLD*(cx-bx);
      fu=Func(u);
    }
    Shft3(ax,bx,cx,u);
    Shft3(fa,fb,fc,fu);
  }
} // end Mnbrak

/* ****************************************************** */
/* Brent:                                                 */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
DP I3CscdLlhBrent::Brent(const DP ax, const DP bx, const DP cx,
          DP &xmin) {

  const DP CGOLD=0.3819660;
  const DP ZEPS=numeric_limits<DP>::epsilon()*1.0e-3;
  DP a,b,d=0.0,etemp,fu,fv,fw,fx;
  DP p,q,r,tol1,tol2,u,v,w,x,xm;
  DP e=0.0;

  a=(ax < cx ? ax : cx);
  b=(ax > cx ? ax : cx);
  x=w=v=bx;
  fw=fv=fx=Func(x);
  while (functionCalls_ < maxCalls_) {
    xm=0.5*(a+b);
    tol2=2.0*(tol1=tolerance_*fabs(x)+ZEPS);
    if (fabs(x-xm) <= (tol2-0.5*(b-a))) {
      xmin=x;
      status_ = STATUS_SUCCESS;
      return fx;
    }
    if (fabs(e) > tol1) {
      r=(x-w)*(fx-fv);
      q=(x-v)*(fx-fw);
      p=(x-v)*q-(x-w)*r;
      q=2.0*(q-r);
      if (q > 0.0) p = -p;
      q=fabs(q);
      etemp=e;
      e=d;
      if (fabs(p) >= fabs(0.5*q*etemp) || p <= q*(a-x) || p >= q*(b-x))
        d=CGOLD*(e=(x >= xm ? a-x : b-x));
      else {
        d=p/q;
        u=x+d;
        if (u-a < tol2 || b-u < tol2)
          d=SIGN(tol1,xm-x);
      }
    } else {
      d=CGOLD*(e=(x >= xm ? a-x : b-x));
    }
    u=(fabs(d) >= tol1 ? x+d : x+SIGN(tol1,d));
    fu=Func(u);
    if (fu <= fx) {
      if (u >= x) a=x; else b=x;
      Shft3(v,w,x,u);
      Shft3(fv,fw,fx,fu);
    } else {
      if (u < x) a=u; else b=u;
      if (fu <= fw || w == x) {
        v=w;
        w=u;
        fv=fw;
        fw=fu;
      } else if (fu <= fv || v == x || v == w) {
        v=u;
        fv=fu;
      }
    }
  } // while

  xmin=x;
  status_ = STATUS_MAX_FUNCTION_CALLS;
  log_debug("Exceeded max function calls in Brent!");
  return fx;
} // end Brent
