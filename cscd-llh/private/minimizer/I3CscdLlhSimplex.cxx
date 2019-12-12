/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhSimplex.cxx
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */

#include "cscd-llh/minimizer/I3CscdLlhHit.h"
#include "cscd-llh/minimizer/I3CscdLlhSimplex.h"
#include <boost/format.hpp>
#include <iostream>
#include <climits>

using namespace std;


/* ****************************************************** */
/* constructors                                           */
/* ****************************************************** */
I3CscdLlhSimplex::I3CscdLlhSimplex() : I3CscdLlhMinimizer() 
{
  log_debug("Enter I3CscdLlhSimplex::I3CscdLlhSimplex()");
  Init(MAX_MINIMIZATION_PARAMS);
  log_debug("Exit I3CscdLlhSimplex::I3CscdLlhSimplex()");
}

I3CscdLlhSimplex::I3CscdLlhSimplex(int maxParams) :
  I3CscdLlhMinimizer(maxParams) 
{
  log_debug("Enter I3CscdLlhMinuit::I3CscdLlhSimplex(int)");
  Init(maxParams);
  log_debug("Exit I3CscdLlhMinuit::I3CscdLlhSimplex(int)");
}

/* ****************************************************** */
/* Init                                                   */
/* ****************************************************** */
void I3CscdLlhSimplex::Init(int maxParams) 
{
  log_debug("Entering I3CscdLlhSimplex::Init()");
 
  log_debug("Exiting I3CscdLlhSimplex::Init()");
  return;
} // end Init

/* ****************************************************** */
/* destructor                                             */
/* ****************************************************** */ 
I3CscdLlhSimplex::~I3CscdLlhSimplex() 
{
}

/* ****************************************************** */
/* Minimize: Perform the actual minimization.             */
/*   Note that, unlike the original recipe,               */
/*   the p matrix is not, in general N+1 x N.             */
/*   This is because some of the paramters are fixed.     */
/*   In my code, p is M+1 x N, where N is the number of   */
/*   (fixed+free) parameters, and M is the number of      */
/*   free parameters.                                     */
/* ****************************************************** */ 
bool I3CscdLlhSimplex::Minimize(list<I3CscdLlhHitPtr>* hits,
  I3CscdLlhAbsPdfPtr pdf) 
{
  log_debug("Entering I3CscdLlhSimplex::Minimize()");

  if (!I3CscdLlhMinimizer::Minimize(hits, pdf)) 
    return false;

  if (numFreeParams_ < 2)
    log_error("The Simplex method must have at least two free parameters!");

  int mpts = numFreeParams_+1;
  int ndim = numParams_;

  Mat_DP p(mpts, ndim);
  Vec_DP y(mpts);

  log_debug("Initialize the p matrix and y vector:");
  for (int i=0; i<mpts; i++) 
  {
    Vec_DP x(ndim);

    int nFix = 0;
    for (int j=0; j<ndim; j++) 
    {
      if (fixParam_[j]) 
      {
        x[j] = p[i][j] = seed_[j];
        nFix++;
      } // fixed
      else 
      {
        if (j == i + nFix - 1) 
        {
	  x[j] = p[i][j] = seed_[j] + stepSize_[j];
	}
        else 
        {
	  x[j] = p[i][j] = seed_[j] - stepSize_[j];
	}
      } // free
    } // for j
   
    y[i] = Func(x);
  } // for i

  log_debug("Starting simplex:");
  for (int iRow=0; iRow<mpts; iRow++) 
  {
    string line("\0");
    string buff;
//    char buff[80];
//    char line[512];
//    line[0] = '\0';*/
    for (int iColumn=0; iColumn<ndim; iColumn++) 
    {
  //    sprintf(buff, "  %.4e", p[iRow][iColumn]);
     // strcat(line, buff);
      buff = str(boost :: format(" %.4e") % p[iRow][iColumn]);
      line.append(buff); 
    }
    
    log_debug("%s", line.c_str());
  }

  Amoeba(p, y);

  log_debug("Final simplex:");
  for (int iRow=0; iRow<mpts; iRow++) 
  {
    string line("\0");
    string buff;
//    char buff[80];
//    char line[512];
//    line[0] = '\0';
    for (int iColumn=0; iColumn<ndim; iColumn++) 
    {
//      sprintf(buff, "  %.4e", p[i][j]);
//      strcat(line, buff);
      buff = str(boost :: format(" %.4e") % p[iRow][iColumn]);
      line.append(buff);
    }

    log_debug("%s", line.c_str());
  }

  log_debug("Simplex Function calls: %d.", functionCalls_);

  result_->status = status_;
  if (status_) 
  {
    // Clear out the static hits_ and pdf_, just to make sure
    // they won't be used by the wrong Minimizer.
    hits_ = 0;
//    pdf_.reSet();

    log_debug("Minimization failed! [status: %d]", status_);
    return false;
  }

  result_->negLlh = y[0];
  log_debug("Negative log-likelihood: %.2f", result_->negLlh);

  CopyFitParams(p);

  // Clear out the static hits_ and pdf_, just to make sure
  // they won't be used by the wrong Minimizer.
  hits_ = 0;
//  pdf_.reSet();

  log_debug("Exiting I3CscdLlhSimplex::Minimize()");
  return true;
} // end Minimize

/* ****************************************************** */
/* CopyFitParams                                          */
/* ****************************************************** */
void I3CscdLlhSimplex::CopyFitParams(const Mat_DP p) 
{
  log_debug("Entering I3CscdLlhSimplex::CopyFitParams()");

  int idx;

  idx = pdf_->GetParamIndex("t");
  if (idx != INT_MIN) 
  {
    result_->t = p[0][idx];
  }

  idx = pdf_->GetParamIndex("x");
  if (idx != INT_MIN) 
  {
    result_->x = p[0][idx];
  }

  idx = pdf_->GetParamIndex("y");
  if (idx != INT_MIN) 
  {
    result_->y = p[0][idx];
  }

  idx = pdf_->GetParamIndex("z");
  if (idx != INT_MIN) 
  {
    result_->z = p[0][idx];
  }

  idx = pdf_->GetParamIndex("zenith");
  if (idx != INT_MIN) 
  {
    result_->theta = p[0][idx];
  }

  idx = pdf_->GetParamIndex("azimuth");
  if (idx != INT_MIN) 
  {
    result_->phi = p[0][idx];
  }

  idx = pdf_->GetParamIndex("energy");
  if (idx != INT_MIN) 
  {
    result_->energy = p[0][idx];
  }

  log_debug("Exiting I3CscdLlhSimplex::CopyFitParams()");
  return;
} // end CopyFitParams

/* ****************************************************** */
/* Func: The Function to be Minimized:                    */
/*      the negative log likelihood.                      */
/* ****************************************************** */
DP I3CscdLlhSimplex::Func(Vec_I_DP &x) 
{

  functionCalls_++;

  // Need to copy the Vec_DP to an array.
  // Maybe in the future I'll require PDF's to overload Evaluate().
  int ndim = x.size();
  assert (ndim>0);
  double param[ndim];
  for (int i=0; i<ndim; i++)
    param[i] = x[i];

  double negLlh = 0.0;

  for(list<I3CscdLlhHitPtr>::iterator iter = I3CscdLlhSimplex::hits_->begin();
    iter != I3CscdLlhSimplex::hits_->end(); iter++) 
  {
      
    I3CscdLlhHitPtr hit = *iter;

    double prob = NAN;

    I3CscdLlhSimplex::pdf_->Evaluate(hit, param, prob);
    log_trace("Probability: %f", prob);

    if (std::isnan(prob) || prob <= 0.0) 
    {
      log_debug("Probability is negative, zero or NAN! [%f]", prob);
      return NAN;
    }

    negLlh -= hit->weight * log(prob);
  } // for each hit

  return negLlh; 
} // end Func

/* ****************************************************** */
/* Amoeba:  Simplex minimization.                         */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
void I3CscdLlhSimplex::Amoeba(Mat_IO_DP &p, Vec_IO_DP &y) 
{
  const DP TINY=1.0e-10;
  int i,ihi,ilo,inhi,j;
  DP rtol,ysave,ytry;

  int mpts=p.nrows();
  int ndim=p.ncols();
  Vec_DP psum(ndim);

  Get_psum(p,psum);
  for (;;) 
  {
    ilo=0;
    ihi = y[0]>y[1] ? (inhi=1,0) : (inhi=0,1);
    for (i=0;i<mpts;i++) 
    {
      if (y[i] <= y[ilo]) ilo=i;
      if (y[i] > y[ihi]) 
      {
        inhi=ihi;
        ihi=i;
      } else if (y[i] > y[inhi] && i != ihi) 
        inhi=i;
    }
    rtol=2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo])+TINY);
    if (rtol < tolerance_) 
    {
      // Success!  Put the best vertex in the zeroth position.
      std::swap(y[0],y[ilo]);
      for (i=0;i<ndim;i++) std::swap(p[0][i],p[ilo][i]);
      break;
    }
    if (functionCalls_ >= maxCalls_) 
    {
      log_debug("Exceeded max Function calls in Amoeba!");
      status_ = STATUS_MAX_FUNCTION_CALLS;
      return;
    }

    ytry=Amotry(p,y,psum,ihi,-1.0);
    if (ytry <= y[ilo])
      {
	//ytry=Amotry(p,y,psum,ihi,2.0);
      }
    else if (ytry >= y[inhi]) 
    {
      ysave=y[ihi];
      ytry=Amotry(p,y,psum,ihi,0.5);
      if (ytry >= ysave) 
      {
        for (i=0;i<mpts;i++) 
        {
          if (i != ilo) 
          {
            for (j=0;j<ndim;j++)
              p[i][j]=psum[j]=0.5*(p[i][j]+p[ilo][j]);
            y[i]=Func(psum);
          }
        }

        Get_psum(p,psum);
      }
    }
  }

  status_ = STATUS_SUCCESS;
  return;
} // end Amoeba

/* ****************************************************** */
/* Get_psum:                                              */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
void I3CscdLlhSimplex::Get_psum(Mat_I_DP &p, Vec_O_DP &psum) 
{
  int i,j;
  DP sum;

  int mpts=p.nrows();
  int ndim=p.ncols();
  for (j=0;j<ndim;j++) 
  {
    for (sum=0.0,i=0;i<mpts;i++)
      {
	sum += p[i][j];
      }
    psum[j]=sum;
  }
} // end Get_psum

/* ****************************************************** */
/* Amotry:                                                */
/*     This code is adapted from Numerical Recipes.       */
/*                                                        */
/* ****************************************************** */
DP I3CscdLlhSimplex::Amotry(Mat_IO_DP &p, Vec_O_DP &y, Vec_IO_DP &psum,
  const int ihi, const DP fac) 
{
  int j;
  DP fac1,fac2,ytry;

  int mpts=p.nrows();
  int ndim=p.ncols();
  Vec_DP ptry(ndim);
  // This is the only change necessary to make simplex work
  // with fixed parameters!!!!
  //fac1=(1.0-fac)/ndim;
  fac1=(1.0-fac)/(mpts-1);
  fac2=fac1-fac;
  for (j=0;j<ndim;j++)
    ptry[j]=psum[j]*fac1-p[ihi][j]*fac2;
  ytry=Func(ptry);
  if (ytry < y[ihi]) 
  {
    y[ihi]=ytry;
    for (j=0;j<ndim;j++) 
    {
      psum[j] += ptry[j]-p[ihi][j];
      p[ihi][j]=ptry[j];
    }
  }
  return ytry;
} // end Amotry
