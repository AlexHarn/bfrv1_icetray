/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhPowell.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/minimizer/I3CscdLlhPowell.h"

#include <iostream>
#include <climits>
#include <limits>
#include <algorithm>

using namespace std;


/* ****************************************************** */
/* constructors                                           */
/* ****************************************************** */
I3CscdLlhPowell::I3CscdLlhPowell() : I3CscdLlhMinimizer()
{
  log_debug("Enter I3CscdLlhPowell::I3CscdLlhPowell()");
  Init(MAX_MINIMIZATION_PARAMS);
  log_debug("Exit I3CscdLlhPowell::I3CscdLlhPowell()");
}

I3CscdLlhPowell::I3CscdLlhPowell(int maxParams) :
  I3CscdLlhMinimizer(maxParams) 
{
  log_debug("Enter I3CscdLlhMinuit::I3CscdLlhPowell(int)");
  Init(maxParams);
  log_debug("Exit I3CscdLlhMinuit::I3CscdLlhPowell(int)");
}

/* ****************************************************** */
/* Init                                                   */
/* ****************************************************** */
void I3CscdLlhPowell::Init(int maxParams) 
{
  log_debug("Entering I3CscdLlhPowell::Init()");

  log_debug("Exiting I3CscdLlhPowell::Init()");
  return;
} // end Init

/* ****************************************************** */
/* destructor                                             */
/* ****************************************************** */ 
I3CscdLlhPowell::~I3CscdLlhPowell() 
{
}

/* ****************************************************** */
/* Minimize: Perform the actual minimization.             */
/* ****************************************************** */ 
bool I3CscdLlhPowell::Minimize(list<I3CscdLlhHitPtr>* hits,
  I3CscdLlhAbsPdfPtr pdf) 
{
  log_debug("Entering I3CscdLlhPowell::Minimize()");

  if (!I3CscdLlhMinimizer::Minimize(hits, pdf)) 
    return false;

  Vec_DP p(numParams_);
  Mat_DP xi(numParams_, numParams_);

  for (int i=0; i<numParams_; i++) 
  {
    p[i] = seed_[i];
    
    for (int j=0; j<numParams_; j++) 
    {
            xi[i][j] = (j == i ? 1.0 : 0.0);
    }
  }

  DP fret;
  Powell(p, xi, fret);

  log_debug("Function minimum: %.2e", fret);
  log_debug("Powell Function calls: %d.", functionCalls_);

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

  result_->negLlh = fret;
  log_debug("Negative log-likelihood: %.2f", result_->negLlh);

  log_debug("copying fit params...");
  CopyFitParams(p);

  // Clear out the static hits_ and pdf_, just to make sure
  // they won't be used by the wrong Minimizer.
  hits_ = 0;
//  pdf_.Reset();

  log_debug("Exiting I3CscdLlhPowell::Minimize()");
  return true;
} // end Minimize

/* ****************************************************** */
/* CopyFitParams                                          */
/* ****************************************************** */
void I3CscdLlhPowell::CopyFitParams(const Vec_DP p) 
{
  log_debug("Entering I3CscdLlhPowell::CopyFitParams()");

  if (!result_)
  {
    log_fatal("No result instantiated in Powell minimizer");
  }

  int idx;

  idx = pdf_->GetParamIndex("t");
  //if (idx != -1) 
  if (idx != INT_MIN) 
  {
    result_->t = p[idx];
  }

  idx = pdf_->GetParamIndex("x");
  //if (idx != -1) 
  if (idx != INT_MIN) 
  {
    result_->x = p[idx];
  }

  idx = pdf_->GetParamIndex("y");
  //if (idx != -1) 
  if (idx != INT_MIN) 
  {
    result_->y = p[idx];
  }

  idx = pdf_->GetParamIndex("z");
  //if (idx != -1) 
  if (idx != INT_MIN) 
  {
    result_->z = p[idx];
  }

  //idx = pdf_->GetParamIndex("theta");
  idx = pdf_->GetParamIndex("zenith");
  //if (idx != -1) 
  if (idx != INT_MIN) 
  {
    result_->theta = p[idx];
  }

//  idx = pdf_->GetParamIndex("phi");
  idx = pdf_->GetParamIndex("azimuth");
  //if (idx != -1) 
  if (idx != INT_MIN) 
  {
    result_->phi = p[idx];
  }

  idx = pdf_->GetParamIndex("energy");
  //if (idx != -1) 
  if (idx != INT_MIN) 
  {
    result_->energy = p[idx];
  }

  log_debug("Exiting I3CscdLlhPowell::CopyFitParams()");
  return;
} // end CopyFitParams

/* ****************************************************** */
/* Func: The Function to be minimized:                    */
/*      the negative log likelihood.                      */
/* ****************************************************** */
DP I3CscdLlhPowell::Func(Vec_I_DP &p) 
{
  functionCalls_++;

  // Need to copy the Vec_DP to an array.
  // Maybe in the future I'll require PDF's to overload evaluate().
  double param[numParams_];
  for (int i=0; i<numParams_; i++)
    param[i] = p[i];

  double negLlh = 0.0;

  for(list<I3CscdLlhHitPtr>::iterator iter = I3CscdLlhPowell::hits_->begin();
    iter != I3CscdLlhPowell::hits_->end(); iter++) 
  {
    I3CscdLlhHitPtr& hit = *iter;

    double prob = NAN;

    I3CscdLlhPowell::pdf_->Evaluate(hit, param, prob);
    log_trace("Probability: %f", prob);

    if (std::isnan(prob) || prob <= 0.0) 
    {
      log_debug("Probability is negative, zero or NAN! [%f]", prob);
      return NAN;
    }

    negLlh -= hit->weight*log(prob);
  } // for each hit

  return negLlh; 
} // end Func

/* ****************************************************** */
/* Powell:                                                */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
void I3CscdLlhPowell::Powell(Vec_IO_DP &p, Mat_IO_DP &xi, DP &fret) 
{
  const DP TINY=1.0e-25;
  int i,j,ibig;
  DP del,fp,fptt,t;

  int n=p.size();
  Vec_DP pt(n),ptt(n),xit(n);
  fret=Func(p);
  for (j=0;j<n;j++) pt[j]=p[j];
  while (true) 
  {
    fp=fret;
    ibig=0;
    del=0.0;
    for (i=0;i<n;i++) 
    {
      // This is the only change necessary to make Powell work
      // with fixed parameters!!!!
      if (fixParam_[i]) 
        continue;

      for (j=0;j<n;j++) 
        xit[j]=xi[j][i];
      fptt=fret;
      Linmin(p, xit, fret);
      if (fptt-fret > del) 
      {
        del=fptt-fret;
        ibig=i+1;
      }
    }
    if (2.0*(fp-fret) <= tolerance_*(fabs(fp)+fabs(fret))+TINY) 
    {
      status_ = STATUS_SUCCESS;
      return;
    }
    if (functionCalls_ >= maxCalls_) 
    {
      log_debug("Exceeded max Function calls in Powell!");
      status_ = STATUS_MAX_FUNCTION_CALLS;
      return;
    }
 
    for (j=0;j<n;j++) 
    {
      ptt[j]=2.0*p[j]-pt[j];
      xit[j]=p[j]-pt[j];
      pt[j]=p[j];
    }
    fptt=Func(ptt);
    if (fptt < fp) 
    {
      t=2.0*(fp-2.0*fret+fptt)*SQR(fp-fret-del)-del*SQR(fp-fptt);
      if (t < 0.0) 
      {
        Linmin(p, xit, fret);
        for (j=0;j<n;j++) 
        {
          xi[j][ibig-1]=xi[j][n-1];
          xi[j][n-1]=xit[j];
        }
      }
    }
  } // while

  return;
} // end Powell

/* ****************************************************** */
/* Linmin:                                                */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
void I3CscdLlhPowell::Linmin(Vec_IO_DP &p, Vec_IO_DP &xi, DP &fret) 
{
  int j;
  DP xx,xmin,fx,fb,fa,bx,ax;

  int n=p.size();
  nCom_=n;
  pComP_=new Vec_DP(n);
  xiComP_=new Vec_DP(n);

  Vec_DP &pcom=*pComP_,&xicom=*xiComP_;
  for (j=0;j<n;j++) 
  {
    pcom[j]=p[j];
    xicom[j]=xi[j];
  }
  ax=0.0;
  xx=1.0;
  Mnbrak(ax,xx,bx,fa,fx,fb);
  fret=Brent(ax,xx,bx,xmin);
  if (status_ != STATUS_SUCCESS) 
  {
    delete xiComP_;
    delete pComP_;
    return;
  }

  for (j=0;j<n;j++) 
  {
    xi[j] *= xmin;
    p[j] += xi[j];
  }
  delete xiComP_;
  delete pComP_;

  return;
} // end Linmin

/* ****************************************************** */
/* Mnbrak:                                                */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
void I3CscdLlhPowell::Mnbrak(DP &ax, DP &bx, DP &cx,
  DP &fa, DP &fb, DP &fc) 
{

  const DP GOLD=1.618034,GLIMIT=100.0,TINY=1.0e-20;
  DP ulim,u,r,q,fu;

  fa=F1dim(ax);
  fb=F1dim(bx);
  if (fb > fa) 
  {
    std::swap(ax,bx);
    std::swap(fb,fa);
  }
  cx=bx+GOLD*(bx-ax);
  fc=F1dim(cx);
  while (fb > fc) 
  {
    r=(bx-ax)*(fb-fc);
    q=(bx-cx)*(fb-fa);
    u=bx-((bx-cx)*q-(bx-ax)*r)/
      (2.0*SIGN(std::max(fabs(q-r),TINY),q-r));
    ulim=bx+GLIMIT*(cx-bx);
    if ((bx-u)*(u-cx) > 0.0) 
    {
      fu=F1dim(u);
      if (fu < fc) 
      {
        ax=bx;
        bx=u;
        fa=fb;
        fb=fu;
        return;
      } 
      else if (fu > fb) 
      {
        cx=u;
        fc=fu;
        return;
      }
      u=cx+GOLD*(cx-bx);
      fu=F1dim(u);
    } 
    else if ((cx-u)*(u-ulim) > 0.0) 
    {
      fu=F1dim(u);
      if (fu < fc) 
      {
        Shft3(bx,cx,u,u+GOLD*(u-cx));
        Shft3(fb,fc,fu,F1dim(u));
      }
    } 
    else if ((u-ulim)*(ulim-cx) >= 0.0) 
    {
      u=ulim;
      fu=F1dim(u);
    } 
    else 
    {
      u=cx+GOLD*(cx-bx);
      fu=F1dim(u);
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
DP I3CscdLlhPowell::Brent(const DP ax, const DP bx, const DP cx,
  DP &xmin)
{

  const DP CGOLD=0.3819660;
  const DP ZEPS=numeric_limits<DP>::epsilon()*1.0e-3;
  DP a,b,d=0.0,etemp,fu,fv,fw,fx;
  DP p,q,r,tol1,tol2,u,v,w,x,xm;
  DP e=0.0;

  a=(ax < cx ? ax : cx);
  b=(ax > cx ? ax : cx);
  x=w=v=bx;
  fw=fv=fx=F1dim(x);
  while (functionCalls_ < maxCalls_) 
  {
    xm=0.5*(a+b);
    tol2=2.0*(tol1=tolerance_*fabs(x)+ZEPS);
    if (fabs(x-xm) <= (tol2-0.5*(b-a))) 
    {
      xmin=x;
      status_ = STATUS_SUCCESS;
      return fx;
    }
    if (fabs(e) > tol1) 
    {
      r=(x-w)*(fx-fv);
      q=(x-v)*(fx-fw);
      p=(x-v)*q-(x-w)*r;
      q=2.0*(q-r);
      if (q > 0.0) 
        p = -p;
      q=fabs(q);
      etemp=e;
      e=d;
      if (fabs(p) >= fabs(0.5*q*etemp) || p <= q*(a-x) || p >= q*(b-x))
        d=CGOLD*(e=(x >= xm ? a-x : b-x));
      else 
      {
        d=p/q;
        u=x+d;
        if (u-a < tol2 || b-u < tol2)
          d=SIGN(tol1,xm-x);
      }
    } else 
    {
      d=CGOLD*(e=(x >= xm ? a-x : b-x));
    }
    u=(fabs(d) >= tol1 ? x+d : x+SIGN(tol1,d));
    fu=F1dim(u);
    if (fu <= fx) 
    {
      if (u >= x) 
        a=x; 
      else 
        b=x;
      Shft3(v,w,x,u);
      Shft3(fv,fw,fx,fu);
    } else 
    {
      if (u < x) 
        a=u; 
      else 
        b=u;
      if (fu <= fw || w == x) 
      {
        v=w;
        w=u;
        fv=fw;
        fw=fu;
      } 
      else if (fu <= fv || v == x || v == w) 
      {
        v=u;
        fv=fu;
      }
    }
  } // while

  xmin=x;
  status_ = STATUS_MAX_FUNCTION_CALLS;
  log_debug("Exceeded max Function calls in Powell!");
  return fx;
} // end Powell

/* ****************************************************** */
/* F1dim:                                                 */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
DP I3CscdLlhPowell::F1dim(const DP x) {
  int j;

  Vec_DP xt(nCom_);
  Vec_DP &pcom=*pComP_,&xicom=*xiComP_;
  for (j=0;j<nCom_;j++)
    xt[j]=pcom[j]+x*xicom[j];

  return Func(xt);
} // end F1dim
