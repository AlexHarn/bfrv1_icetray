// @(#)root/minuit:$Id$
// Author: Rene Brun, Frederick James   12/08/95
// hacked first by Claudio Kopper
// hacked second (put TMinuit into lilliput namespace) by Kai Krings

//Ignore this file in test coverage
//LCOV_EXCL_START

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//______________________________________________________________________________
//*-*-*-*-*-*-*-*-*-*-*-*The Minimization package*-*--*-*-*-*-*-*-*-*-*-*-*
//*-*                    ========================                         *
//*-*                                                                     *
//*-*   This package was originally written in Fortran by Fred James      *
//*-*   and part of PACKLIB (patch D506)                                  *
//*-*                                                                     *
//*-*   It has been converted to a C++ class  by R.Brun                   *
//*-*   The current implementation in C++ is a straightforward conversion *
//*-*   of the original Fortran version: The main changes are:            *
//*-*                                                                     *
//*-*   - The variables in the various Minuit labelled common blocks      *
//*-*     have been changed to the TMinuit class data members.            *
//*-*   - The internal arrays with a maximum dimension depending on the   *
//*-*     maximum number of parameters are now data members arrays with   *
//*-*     a dynamic dimension such that one can fit very large problems   *
//*-*     by simply initialising the TMinuit constructor with the maximum *
//*-*     number of parameters.                                           *
//*-*   - The include file Minuit.h has been commented as much as possible*
//*-*     using existing comments in the code or the printed documentation*
//*-*   - The original Minuit subroutines are now member functions.       *
//*-*   - Constructors and destructor have been added.                    *
//*-*   - Instead of passing the FCN  function in the argument            *
//*-*     list, the addresses of this function is stored as pointer       *
//*-*     in the data members of the class. This is by far more elegant   *
//*-*     and flexible in an interactive environment.                     *
//*-*     The member function SetFCN can be used to define this pointer.  *
//*-*   - The ROOT static function log_info is provided to replace all      *
//*-*     format statements and to print on currently defined output file.*
//*-*   - The functions SetObjectFit(TObject *obj)/GetObjectFit() can be  *
//*-*     used inside the FCN function to set/get a referenced object     *
//*-*     instead of using global variables.                              *
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//BEGIN_HTML <!--
/* -->
<P>
<H2><A NAME=H2Basic-concepts-of-MINUIT.html>Basic concepts of MINUIT</A></H2>
<P>
The <A HREF="http://wwwinfo.cern.ch/asdoc/minuit/minmain.html"> MINUIT</A> package acts on a multiparameter Fortran function to which one
must give the generic name <TT>FCN</TT>. In the ROOT implementation,
the function <TT>FCN</TT> is defined via the MINUIT SetFCN member function
when an Histogram.Fit command is invoked.
The value of <TT>FCN</TT> will in general depend on one
or more variable parameters.
<P>
To take a simple example, in case of ROOT histograms (classes TH1C,TH1S,TH1F,TH1D)
the Fit function defines the Minuit fitting function as being H1FitChisquare
or H1FitLikelihood depending on the options selected.
H1FitChisquare
calculates the chisquare between the user fitting function (gaussian, polynomial,
user defined,etc) and the data for given values of the parameters.
It is the task of MINUIT to find those values of the parameters
which give the lowest value of chisquare.
<P>
<H3>Basic concepts - The transformation for parameters with limits.</H3>
<P>
For variable parameters with limits, MINUIT uses the following
transformation:
<P>
<PRE>
  P   = arcsin(2((P   -a)/(b- a))-1)                P    = a+((b- a)/(2))(sinP    +1)
   int             ext                               ext                      int
</PRE>
<P>
so that the internal value P    can take on any value, while the external
                            int
value P    can take on values only between the lower limit a and the
       ext
upper limit b. Since the transformation is necessarily non-linear, it
would transform a nice linear problem into a nasty non-linear one, which
is the reason why limits should be avoided if not necessary. In addition,
the transformation does require some computer time, so it slows down the
computation a little bit, and more importantly, it introduces additional
numerical inaccuracy into the problem in addition to what is introduced in
the numerical calculation of the <TT>FCN</TT> value. The effects of
non-linearity and numerical roundoff both become more important as the
external value gets closer to one of the limits (expressed as the distance
to nearest limit divided by distance between limits). The user must
therefore be aware of the fact that, for example, if he puts limits of
(0,10^10  ) on a parameter, then the values 0.0 and 1. 0 will be
indistinguishable to the accuracy of most machines.
<P>
The transformation also affects the parameter error matrix, of course, so
MINUIT does a transformation of the error matrix (and the ``parabolic''
parameter errors) when there are parameter limits. Users should however
realize that the transformation is only a linear approximation, and that
it cannot give a meaningful result if one or more parameters is very close
to a limit, where partial Pext /partial Pint  #0. Therefore, it is
recommended that:
<P>
<OL>
<LI>Limits on variable parameters should be used only when needed in order
to prevent the parameter from taking on unphysical values.
<LI>When a satisfactory minimum has been found using limits, the limits
should then be removed if possible, in order to perform or re-perform the
error analysis without limits.
</OL>
<P>
<H3>How to get the right answer from MINUIT.</H3>
<P>
MINUIT offers the user a choice of several minimization algorithms. The
MIGRAD algorithm is in general the best minimizer for
nearly all functions. It is a variable-metric method with inexact line
search, a stable metric updating scheme, and checks for
positive-definiteness. Its main weakness is that it depends heavily on
knowledge of the first derivatives, and fails miserably if they are very
inaccurate.
<P>
If parameter limits are needed, in spite of the side effects, then the
user should be aware of the following techniques to alleviate problems
caused by limits:
<P>
<H4>Getting the right minimum with limits.</H4>
<P>
If MIGRAD converges normally to a point where no parameter is near one of
its limits, then the existence of limits has probably not prevented MINUIT
from finding the right minimum. On the other hand, if one or more
parameters is near its limit at the minimum, this may be because the true
minimum is indeed at a limit, or it may be because the minimizer has
become ``blocked'' at a limit. This may normally happen only if the
parameter is so close to a limit (internal value at an odd multiple of #((pi)/(2))
that MINUIT prints a warning to this effect when it prints
the parameter values.

The minimizer can become blocked at a limit, because at a limit the
derivative seen by the minimizer partial F/partial Pint  is zero no matter
what the real derivative partial F/partial Pext  is.
<P>
<P>
<PRE>
((partial F)/(partial P   ))= ((partial F)/(partial P   ))((partial P    )/(partial P   )) =((partial F)/(partial P    ))= 0
                       int                           ext             ext             int                           ext
</PRE>
<P>
<P>
<H4>Getting the right parameter errors with limits.</H4>
<P>
In the best case, where the minimum is far from any limits, MINUIT will
correctly transform the error matrix, and the parameter errors it reports
should be accurate and very close to those you would have got without
limits. In other cases (which should be more common, since otherwise you
wouldn't need limits), the very meaning of parameter errors becomes
problematic. Mathematically, since the limit is an absolute constraint on
the parameter, a parameter at its limit has no error, at least in one
direction. The error matrix, which can assign only symmetric errors, then
becomes essentially meaningless.
<P>
<H3>Interpretation of Parameter Errors:</H3>
<P>
There are two kinds of problems that can arise: the reliability of
MINUIT's error estimates, and their statistical interpretation, assuming
they are accurate.
<P>
<H3>Statistical interpretation:</H3>
<P>
For discussuion of basic concepts, such as the meaning of the elements of
the error matrix, or setting of exact confidence levels see:
<ol>
  <li>F.James.
     Determining the statistical Significance of experimental Results.
     Technical Report DD/81/02 and CERN Report 81-03, CERN, 1981.</li>

  <li>W.T.Eadie, D.Drijard, F.James, M.Roos, and B.Sadoulet.
     Statistical Methods in Experimental Physics.
     North-Holland, 1971.</li>
</ol>
<P>
<H3>Reliability of MINUIT error estimates.</H3>
<P>
MINUIT always carries around its own current estimates of the parameter
errors, which it will print out on request, no matter how accurate they
are at any given point in the execution. For example, at initialization,
these estimates are just the starting step sizes as specified by the user.
After a HESSE step, the errors are usually quite accurate,
unless there has been a problem. MINUIT, when it prints out error values,
also gives some indication of how reliable it thinks they are. For
example, those marked <TT>CURRENT GUESS ERROR</TT> are only working values
not to be believed, and <TT>APPROXIMATE ERROR</TT> means that they have
been calculated but there is reason to believe that they may not be
accurate.
<P>
If no mitigating adjective is given, then at least MINUIT believes the
errors are accurate, although there is always a small chance that MINUIT
has been fooled. Some visible signs that MINUIT may have been fooled are:
<P>
<OL>
<LI>Warning messages produced during the minimization or error analysis.
<LI>Failure to find new minimum.
<LI>Value of <TT>EDM</TT> too big (estimated Distance to Minimum).
<LI>Correlation coefficients exactly equal to zero, unless some parameters
are known to be uncorrelated with the others.
<LI>Correlation coefficients very close to one (greater than 0.99). This
indicates both an exceptionally difficult problem, and one which has been
badly parameterized so that individual errors are not very meaningful
because they are so highly correlated.
<LI>Parameter at limit. This condition, signalled by a MINUIT warning
message, may make both the function minimum and parameter errors
unreliable. See the discussion above ``Getting the right parameter errors
with limits''.
</OL>
<P>
The best way to be absolutely sure of the errors, is to use
``independent'' calculations and compare them, or compare the calculated
errors with a picture of the function. Theoretically, the covariance
matrix for a ``physical'' function must be positive-definite at the
minimum, although it may not be so for all points far away from the
minimum, even for a well-determined physical problem. Therefore, if MIGRAD
reports that it has found a non-positive-definite covariance matrix, this
may be a sign of one or more of the following:
<P>
<H5>A non-physical region:</H5>
<P>
On its way to the minimum, MIGRAD may have traversed a region which has
unphysical behaviour, which is of course not a serious problem as long as
it recovers and leaves such a region.
<P>
<H5>An underdetermined problem:</H5>
<P>
If the matrix is not positive-definite even at the minimum, this may mean
that the solution is not well-defined, for example that there are more
unknowns than there are data points, or that the parameterization of the
fit contains a linear dependence. If this is the case, then MINUIT (or any
other program) cannot solve your problem uniquely, and the error matrix
will necessarily be largely meaningless, so the user must remove the
underdeterminedness by reformulating the parameterization. MINUIT cannot
do this itself.
<P>
<H5>Numerical inaccuracies:</H5>
<P>
It is possible that the apparent lack of positive-definiteness is in fact
only due to excessive roundoff errors in numerical calculations in the
user function or not enough precision. This is unlikely in general, but
becomes more likely if the number of free parameters is very large, or if

the parameters are badly scaled (not all of the same order of magnitude),
and correlations are also large. In any case, whether the
non-positive-definiteness is real or only numerical is largely irrelevant,
since in both cases the error matrix will be unreliable and the minimum
suspicious.
<P>
<H5>An ill-posed problem:</H5>
<P>
For questions of parameter dependence, see the discussion above on
positive-definiteness.
<P>
Possible other mathematical problems are the following:
<P>
<H5>Excessive numerical roundoff:</H5>
<P>
Be especially careful of exponential and factorial functions which get big
very quickly and lose accuracy.
<P>
<H5>Starting too far from the solution:</H5>
<P>
The function may have unphysical local minima, especially at infinity in
some variables.

<H5>Minuit parameter errors in the presence of limits</H5>
This concerns the way Minuit reports the symmetric (or parabolic)  errors
on parameters.  It does not apply to the errors reported from Minos, which
are in general asymmetric.
<P>
The symmetric errors reported by Minuit are always calculated from
the covariance matrix, assuming that this matrix has been calculated,
usually as the result of a Migrad minimization or a direct
calculation by Hesse which inverts the second derivative matrix.
<P>
When there are no limits on the parameter in question, the error reported
by Minuit should therefore be exactly equal to the square root of the
corresponding diagonal element of the error matrix reported by Minuit.
<P>
However, when there are limits on the parameter, there is a transformation
between the internal parameter values seen by Minuit (which are unbounded)
and the external parameter values seen by the user in FCN (which remain
inside the desired limits).  Therefore the internal error matrix kept by
Minuit must be transformed to an external error matrix for the user.
This is done by multiplying the (I,J)th element by DEXDIN(I)*DEXDIN(J),
where DEXDIN is the derivative of the external value with respect to the
internal value at the minimum.  This is a linearization of the
transformation, and is the only way to produce an error matrix in external
coordinates meaningful to the user.  But when reporting the individual
parabolic errors for limited parameters, Minuit can do a little better, so
it does.  In this case, Minuit actually transforms the ends of the
internal "error bar" to external coordinates and reports the length of
this transformed interval.  Strictly speaking, it is now asymmetric, but
since the origin of the asymmetry is only an artificial transformation it
does not make much sense, so the transformed errors are symmetrized.
<P>
The result of all the above is that for parameters with limits, the error
reported by Minuit is not exactly equal to the square root of the diagonal
element of the error matrix.  The difference is a measure of how much the
limits deform the problem.  If possible, it is suggested not to use limits
on parameters, and the problem goes away.  If for some reason limits are
necessary, and you are sensitive to the difference between the two ways of
calculating the errors, it is suggested to use Minos errors which take
into account the non-linearities much more precisely.

<!--*/
// -->END_HTML

#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <cstdio>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "icetray/I3Logging.h"

#include "lilliput/minimizer/TMinuit.h"

const char charal[29] = " .ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//______________________________________________________________________________
lilliput::TMinuit::TMinuit()
{
//*-*-*-*-*-*-*-*-*-*-*Minuit normal constructor*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                  ========================

   BuildArrays(25);

   fUp        = 0;
   fEpsi      = 0;
   fApsi      = 0;
   fXmidcr    = 0;
   fYmidcr    = 0;
   fXdircr    = 0;
   fYdircr    = 0;

   fStatus       = 0;
   fEmpty        = 0;
   SetMaxIterations();
   mninit(5,6,7);

   fFCN = 0;
}

//______________________________________________________________________________
lilliput::TMinuit::TMinuit(int32_t maxpar)
{
//*-*-*-*-*-*-*-*-*-*-*Minuit normal constructor*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                  ========================
//
//  maxpar is the maximum number of parameters used with this TMinuit object.

   fFCN = 0;

   BuildArrays(maxpar);

   fStatus       = 0;
   fEmpty        = 0;
   SetMaxIterations();

   mninit(5,6,7);
}

//______________________________________________________________________________
lilliput::TMinuit::TMinuit(const TMinuit &minuit)
{
   // Private TMinuit copy ctor. TMinuit can not be copied.
}

//______________________________________________________________________________
lilliput::TMinuit::~TMinuit()
{
//*-*-*-*-*-*-*-*-*-*-*Minuit default destructor*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                  =========================

   DeleteArrays();
}

//______________________________________________________________________________
void lilliput::TMinuit::BuildArrays(int32_t maxpar)
{
//*-*-*-*-*-*-*Create internal Minuit arrays for the maxpar parameters*-*-*-*
//*-*          =======================================================

   fMaxpar = 25;
   if (maxpar >= fMaxpar) fMaxpar = maxpar+1;
   fMaxpar1= fMaxpar*(fMaxpar+1);
   fMaxpar2= 2*fMaxpar;
   fMaxpar5= fMaxpar1/2;
   fMaxcpt = 101;
   fCpnam  = new std::string[fMaxpar2];
   fU      = new double[fMaxpar2];
   fAlim   = new double[fMaxpar2];
   fBlim   = new double[fMaxpar2];
   fPstar  = new double[fMaxpar2];
   fGin    = new double[fMaxpar2];
   fNvarl  = new int32_t[fMaxpar2];
   fNiofex = new int32_t[fMaxpar2];

   fNexofi = new int32_t[fMaxpar];
   fIpfix  = new int32_t[fMaxpar];
   fErp    = new double[fMaxpar];
   fErn    = new double[fMaxpar];
   fWerr   = new double[fMaxpar];
   fGlobcc = new double[fMaxpar];
   fX      = new double[fMaxpar];
   fXt     = new double[fMaxpar];
   fDirin  = new double[fMaxpar];
   fXs     = new double[fMaxpar];
   fXts    = new double[fMaxpar];
   fDirins = new double[fMaxpar];
   fGrd    = new double[fMaxpar];
   fG2     = new double[fMaxpar];
   fGstep  = new double[fMaxpar];
   fDgrd   = new double[fMaxpar];
   fGrds   = new double[fMaxpar];
   fG2s    = new double[fMaxpar];
   fGsteps = new double[fMaxpar];
   fPstst  = new double[fMaxpar];
   fPbar   = new double[fMaxpar];
   fPrho   = new double[fMaxpar];
   fWord7  = new double[fMaxpar];
   fVhmat  = new double[fMaxpar5];
   fVthmat = new double[fMaxpar5];
   fP      = new double[fMaxpar1];
   fXpt    = new double[fMaxcpt];
   fYpt    = new double[fMaxcpt];
   fChpt   = new char[fMaxcpt+1];
   // initialisation of dynamic arrays used internally in some functions
   // these arrays had a fix dimension in Minuit
   fCONTgcc   = new double[fMaxpar];
   fCONTw     = new double[fMaxpar];
   fFIXPyy    = new double[fMaxpar];
   fGRADgf    = new double[fMaxpar];
   fHESSyy    = new double[fMaxpar];
   fIMPRdsav  = new double[fMaxpar];
   fIMPRy     = new double[fMaxpar];
   fMATUvline = new double[fMaxpar];
   fMIGRflnu  = new double[fMaxpar];
   fMIGRstep  = new double[fMaxpar];
   fMIGRgs    = new double[fMaxpar];
   fMIGRvg    = new double[fMaxpar];
   fMIGRxxs   = new double[fMaxpar];
   fMNOTxdev  = new double[fMaxpar];
   fMNOTw     = new double[fMaxpar];
   fMNOTgcc   = new double[fMaxpar];
   fPSDFs     = new double[fMaxpar];
   fSEEKxmid  = new double[fMaxpar];
   fSEEKxbest = new double[fMaxpar];
   fSIMPy     = new double[fMaxpar];
   fVERTq     = new double[fMaxpar];
   fVERTs     = new double[fMaxpar];
   fVERTpp    = new double[fMaxpar];
   fCOMDplist = new double[fMaxpar];
   fPARSplist = new double[fMaxpar];

   for (int i = 0; i < fMaxpar; i++) {
      fErp[i] = 0;
      fErn[i] = 0;
   }
}


//______________________________________________________________________________
int32_t lilliput::TMinuit::Command(const char *command)
{
// execute a Minuit command
//     Equivalent to MNEXCM except that the command is given as a
//     character string.
// See TMinuit::mnhelp for the full list of available commands
// See also http://wwwasdoc.web.cern.ch/wwwasdoc/minuit/node18.html for
//  a complete documentation of all the available commands
//
// Returns the status of the execution:
//   = 0: command executed normally
//     1: command is blank, ignored
//     2: command line unreadable, ignored
//     3: unknown command, ignored
//     4: abnormal termination (e.g., MIGRAD not converged)
//     5: command is a request to read PARAMETER definitions
//     6: 'SET INPUT' command
//     7: 'SET TITLE' command
//     8: 'SET COVAR' command
//     9: reserved
//    10: END command
//    11: EXIT or STOP command
//    12: RETURN command
//
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   int32_t status = 0;
   mncomd(command,status);
   return status;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::DefineParameter( int32_t parNo, const char *name, double initVal, double initErr, double lowerLimit, double upperLimit )
{
// Define a parameter

   int32_t err;

   std::string sname = name;
   mnparm( parNo, sname, initVal, initErr, lowerLimit, upperLimit, err);

   return err;
}

//______________________________________________________________________________
void lilliput::TMinuit::DeleteArrays()
{
//*-*-*-*-*-*-*-*-*-*-*-*Delete internal Minuit arrays*-*-*-*-*-*-*-*-*
//*-*                    =============================
   if (fEmpty) return;
   delete [] fCpnam;
   delete [] fU;
   delete [] fAlim;
   delete [] fBlim;
   delete [] fErp;
   delete [] fErn;
   delete [] fWerr;
   delete [] fGlobcc;
   delete [] fNvarl;
   delete [] fNiofex;
   delete [] fNexofi;
   delete [] fX;
   delete [] fXt;
   delete [] fDirin;
   delete [] fXs;
   delete [] fXts;
   delete [] fDirins;
   delete [] fGrd;
   delete [] fG2;
   delete [] fGstep;
   delete [] fGin;
   delete [] fDgrd;
   delete [] fGrds;
   delete [] fG2s;
   delete [] fGsteps;
   delete [] fIpfix;
   delete [] fVhmat;
   delete [] fVthmat;
   delete [] fP;
   delete [] fPstar;
   delete [] fPstst;
   delete [] fPbar;
   delete [] fPrho;
   delete [] fWord7;
   delete [] fXpt;
   delete [] fYpt;
   delete [] fChpt;

   delete [] fCONTgcc;
   delete [] fCONTw;
   delete [] fFIXPyy;
   delete [] fGRADgf;
   delete [] fHESSyy;
   delete [] fIMPRdsav;
   delete [] fIMPRy;
   delete [] fMATUvline;
   delete [] fMIGRflnu;
   delete [] fMIGRstep;
   delete [] fMIGRgs;
   delete [] fMIGRvg;
   delete [] fMIGRxxs;
   delete [] fMNOTxdev;
   delete [] fMNOTw;
   delete [] fMNOTgcc;
   delete [] fPSDFs;
   delete [] fSEEKxmid;
   delete [] fSEEKxbest;
   delete [] fSIMPy;
   delete [] fVERTq;
   delete [] fVERTs;
   delete [] fVERTpp;
   delete [] fCOMDplist;
   delete [] fPARSplist;

   fEmpty = 1;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::Eval(int32_t npar, double *grad, double &fval, double *par, int32_t flag)
{
// Evaluate the minimisation function
//  Input parameters:
//    npar:    number of currently variable parameters
//    par:     array of (constant and variable) parameters
//    flag:    Indicates what is to be calculated (see example below)
//    grad:    array of gradients
//  Output parameters:
//    fval:    The calculated function value.
//    grad:    The (optional) vector of first derivatives).
//
// The meaning of the parameters par is of course defined by the user,
// who uses the values of those parameters to calculate his function value.
// The starting values must be specified by the user.
// Later values are determined by Minuit as it searches for the minimum
// or performs whatever analysis is requested by the user.
//
// Note that this virtual function may be redefined in a class derived from TMinuit.
// The default function calls the function specified in SetFCN
//
// Example of Minimisation function:
/*
   if (flag == 1) {
      read input data,
      calculate any necessary constants, etc.
   }
   if (flag == 2) {
      calculate GRAD, the first derivatives of FVAL
     (this is optional)
   }
   Always calculate the value of the function, FVAL,
   which is usually a chisquare or log likelihood.
   if (iflag == 3) {
      will come here only after the fit is finished.
      Perform any final calculations, output fitted data, etc.
   }
*/
//  See concrete examples in TH1::H1FitChisquare, H1FitLikelihood

   if (fFCN) (*fFCN)(npar,grad,fval,par,flag);
   return 0;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::FixParameter( int32_t parNo)
{
// fix a parameter

   int32_t err;
   double tmp[1];
   tmp[0] = parNo+1; //set internal Minuit numbering

   mnexcm( "FIX", tmp,  1,  err );

   return err;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::GetParameter( int32_t parNo, double &currentValue, double &currentError ) const
{
// return parameter value and error
   int32_t    err;
   std::string  name; // ignored
   double bnd1, bnd2; // ignored

   mnpout( parNo, name, currentValue, currentError, bnd1, bnd2, err );

   return err;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::GetNumFixedPars() const
{
// returns the number of currently fixed parameters

   return fNpfix;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::GetNumFreePars() const
{
// returns the number of currently free parameters

   return fNpar;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::GetNumPars() const
{
// returns the total number of parameters that have been defined.
// (fixed and free)

   return fNpar + fNpfix;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::Migrad()
{
// invokes the MIGRAD minimizer
   int32_t err;
   double tmp[1];
   tmp[0] = 0;

   mnexcm( "MIGRAD", tmp, 0, err );

   return err;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::Release( int32_t parNo)
{
// release a parameter

   int32_t err;
   double tmp[1];
   tmp[0] = parNo+1; //set internal Minuit numbering

   mnexcm( "RELEASE", tmp, 1, err );

   return err;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::SetErrorDef( double up )
{
// To get the n-sigma contour the error def parameter "up" has to set to n^2.

   int32_t err;

   mnexcm( "SET ERRDEF", &up, 1, err );

   return err;
}

//______________________________________________________________________________
void lilliput::TMinuit::SetFCN(void (*fcn)(int32_t &, double *, double &f, double *, int32_t))
{
//*-*-*-*-*-*-*To set the address of the minimization function*-*-*-*-*-*-*-*
//*-*          ===============================================
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
   fFCN = fcn;
}

//______________________________________________________________________________
int32_t lilliput::TMinuit::SetPrintLevel( int32_t printLevel )
{
   //set Minuit print level
   // printlevel = -1  quiet (also suppresse all warnings)
   //            =  0  normal
   //            =  1  verbose
   int32_t    err;
   double tmp[1];
   tmp[0] = printLevel;

   mnexcm( "SET PRINT", tmp, 1, err );

   if (printLevel <=-1) mnexcm("SET NOWarnings",tmp,0,err);

   return err;
}

//______________________________________________________________________________
void lilliput::TMinuit::mnamin()
{
//*-*-*-*-*-*-*-*-*-*-*-*-*Initialize AMIN*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                      ===============
//*-*C        Called  from many places.  Initializes the value of AMIN by
//*-*C        calling the user function. Prints out the function value and
//*-*C        parameter values if Print Flag value is high enough.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double fnew;
   int32_t nparx;

   nparx = fNpar;
   if (fISW[4] >= 1) {
      log_info(" FIRST CALL TO USER FUNCTION AT NEW START POINT, WITH IFLAG=4.");
   }
   mnexin(fX);
   Eval(nparx, fGin, fnew, fU, 4);    ++fNfcn;
   fAmin = fnew;
   fEDM  = fBigedm;
} /* mnamin_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnbins(double a1, double a2, int32_t naa, double &bl, double &bh, int32_t &nb, double &bwid)
{
//*-*-*-*-*-*-*-*-*-*-*Compute reasonable histogram intervals*-*-*-*-*-*-*-*-*
//*-*                  ======================================
//*-*        Function TO DETERMINE REASONABLE HISTOGRAM INTERVALS
//*-*        GIVEN ABSOLUTE UPPER AND LOWER BOUNDS  A1 AND A2
//*-*        AND DESIRED MAXIMUM NUMBER OF BINS NAA
//*-*        PROGRAM MAKES REASONABLE BINNING FROM BL TO BH OF WIDTH BWID
//*-*        F. JAMES,   AUGUST, 1974 , stolen for Minuit, 1988
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double awid,ah, al, sigfig, sigrnd, alb;
   int32_t kwid, lwid, na=0, log_;

   al = std::min(a1,a2);
   ah = std::max(a1,a2);
   if (al == ah) ah = al + 1;

//*-*-       IF NAA .EQ. -1 , PROGRAM USES BWID INPUT FROM CALLING ROUTINE
   if (naa == -1) goto L150;
L10:
   na = naa - 1;
   if (na < 1) na = 1;

//*-*-        GET NOMINAL BIN WIDTH IN EXPON FORM
L20:
   awid = (ah-al) / double(na);
   log_ = int32_t(std::log10(awid));
   if (awid <= 1) --log_;
   sigfig = awid*std::pow(double(10), -log_);
//*-*-       ROUND MANTISSA UP TO 2, 2.5, 5, OR 10
   if (sigfig > 2) goto L40;
   sigrnd = 2;
   goto L100;
L40:
   if (sigfig > 2.5) goto L50;
   sigrnd = 2.5;
   goto L100;
L50:
   if (sigfig > 5) goto L60;
   sigrnd = 5;
   goto L100;
L60:
   sigrnd = 1;
   ++log_;
L100:
   bwid = sigrnd*std::pow(double(10), log_);
   goto L200;
//*-*-       GET NEW BOUNDS FROM NEW WIDTH BWID
L150:
   if (bwid <= 0) goto L10;
L200:
   alb  = al / bwid;
   lwid = int32_t(alb);
   if (alb < 0) --lwid;
   bl   = bwid*double(lwid);
   alb  = ah / bwid + 1;
   kwid = int32_t(alb);
   if (alb < 0) --kwid;
   bh = bwid*double(kwid);
   nb = kwid - lwid;
   if (naa > 5) goto L240;
   if (naa == -1) return;
//*-*-        REQUEST FOR ONE BIN IS DIFFICULT CASE
   if (naa > 1 || nb == 1) return;
   bwid *= 2;
   nb = 1;
   return;
L240:
   if (nb << 1 != naa) return;
   ++na;
   goto L20;
} /* mnbins_ */

//______________________________________________________________________________
void lilliput::TMinuit::mncalf(double *pvec, double &ycalf)
{
//*-*-*-*-*-*-*-*-*-*Transform FCN to find further minima*-*-*-*-*-*-*-*-*-*
//*-*                ====================================
//*-*        Called only from MNIMPR.  Transforms the function FCN
//*-*        by dividing out the quadratic part in order to find further
//*-*        minima.    Calculates  ycalf = (f-fmin)/(x-xmin)*v*(x-xmin)
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   int32_t ndex, i, j, m, n, nparx;
   double denom, f;

   nparx = fNpar;
   mninex(&pvec[0]);
   Eval(nparx, fGin, f, fU, 4);    ++fNfcn;
   for (i = 1; i <= fNpar; ++i) {
      fGrd[i-1] = 0;
      for (j = 1; j <= fNpar; ++j) {
         m = std::max(i,j);
         n = std::min(i,j);
         ndex = m*(m-1) / 2 + n;
         fGrd[i-1] += fVthmat[ndex-1]*(fXt[j-1] - pvec[j-1]);
      }
   }
   denom = 0;
   for (i = 1; i <= fNpar; ++i) {denom += fGrd[i-1]*(fXt[i-1] - pvec[i-1]); }
   if (denom <= 0) {
      fDcovar = 1;
      fISW[1] = 0;
      denom   = 1;
   }
   ycalf = (f - fApsi) / denom;
} /* mncalf_ */

//______________________________________________________________________________
void lilliput::TMinuit::mncler()
{
//*-*-*-*-*-*-*-*-*-*-*Resets the parameter list to UNDEFINED*-*-*-*-*-*-*-*
//*-*                  ======================================
//*-*        Called from MINUIT and by option from MNEXCM
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   int32_t i;

   fNpfix = 0;
   fNu = 0;
   fNpar = 0;
   fNfcn = 0;
   fNwrmes[0] = 0;
   fNwrmes[1] = 0;
   for (i = 1; i <= fMaxext; ++i) {
      fU[i-1]      = 0;
      fCpnam[i-1]  = fCundef;
      fNvarl[i-1]  = -1;
      fNiofex[i-1] = 0;
   }
   mnrset(1);
   fCfrom  = "CLEAR   ";
   fNfcnfr = fNfcn;
   fCstatu = "UNDEFINED ";
   fLnolim = true;
   fLphead = true;
} /* mncler_ */

//______________________________________________________________________________
void lilliput::TMinuit::mncomd(const char *crdbin, int32_t &icondn)
{
//*-*-*-*-*-*-*-*-*-*-*Reads a command string and executes*-*-*-*-*-*-*-*-*-*
//*-*                  ===================================
//*-*        Called by user.  'Reads' a command string and executes.
//*-*     Equivalent to MNEXCM except that the command is given as a
//*-*          character string.
//*-*
//*-*     ICONDN = 0: command executed normally
//*-*              1: command is blank, ignored
//*-*              2: command line unreadable, ignored
//*-*              3: unknown command, ignored
//*-*              4: abnormal termination (e.g., MIGRAD not converged)
//*-*              5: command is a request to read PARAMETER definitions
//*-*              6: 'SET INPUT' command
//*-*              7: 'SET TITLE' command
//*-*              8: 'SET COVAR' command
//*-*              9: reserved
//*-*             10: END command
//*-*             11: EXIT or STOP command
//*-*             12: RETURN command
//*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   int32_t ierr, ipos, i, llist, lenbuf, lnc;
   bool leader;
   std::string comand, crdbuf, ctemp;

   crdbuf = boost::to_upper_copy(std::string(crdbin));
   lenbuf = crdbuf.length();
   icondn = 0;
//*-*-    record not case-sensitive, get upper case, strip leading blanks
   leader = true;
   ipos = 1;
   for (i = 1; i <= std::min(20,lenbuf); ++i) {
      if (crdbuf[i-1] == '\'') break;
      if (crdbuf[i-1] == ' ') {
         if (leader) ++ipos;
         continue;
      }
      leader = false;
   }

//*-*-                    blank or null command
   if (ipos > lenbuf) {
      log_info(" BLANK COMMAND IGNORED.");
      icondn = 1;
      return;
   }
//*-*-                                          . .   preemptive commands
//*-*-              if command is 'PARAMETER'
   if (crdbuf.substr(ipos-1,3) == "PAR") {
      icondn = 5;
      fLphead = true;
      return;
   }
//*-*-              if command is 'SET INPUT'
   if (crdbuf.substr(ipos-1,3) == "SET INP") {
      icondn = 6;
      fLphead = true;
      return;
   }
//*-*-              if command is 'SET TITLE'
   if (crdbuf.substr(ipos-1,7) == "SET TIT") {
      icondn = 7;
      fLphead = true;
      return;
   }
//*-*-              if command is 'SET COVARIANCE'
   if (crdbuf.substr(ipos-1,7) == "SET COV") {
      icondn = 8;
      fLphead = true;
      return;
   }
//*-*-              crack the command . . . . . . . . . . . . . . . .
   ctemp = crdbuf.substr(ipos-1,lenbuf-ipos+1);
   mncrck(ctemp, 20, comand, lnc, fMaxpar, fCOMDplist, llist, ierr, fIsyswr);
   if (ierr > 0) {
      log_info(" COMMAND CANNOT BE INTERPRETED");
      icondn = 2;
      return;
   }

   mnexcm(comand.c_str(), fCOMDplist, llist, ierr);
   icondn = ierr;
} /* mncomd_ */

//______________________________________________________________________________
void lilliput::TMinuit::mncont(int32_t ike1, int32_t ike2, int32_t nptu, double *xptu, double *yptu, int32_t &ierrf)
{
//*-*-*-*-*-*-*Find points along a contour where FCN is minimum*-*-*-*-*-*-*
//*-*          ================================================
//*-*       Find NPTU points along a contour where the function
//*-*             FMIN (X(KE1),X(KE2)) =  AMIN+UP
//*-*       where FMIN is the minimum of FCN with respect to all
//*-*       the other NPAR-2 variable parameters (if any).
//*-*   IERRF on return will be equal to the number of points found:
//*-*     NPTU if normal termination with NPTU points found
//*-*     -1   if errors in the calling sequence (KE1, KE2 not variable)
//*-*      0   if less than four points can be found (using MNMNOT)
//*-*     n>3  if only n points can be found (n < NPTU)
//*-*
//*-*                 input arguments: parx, pary, devs, ngrid
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
    /* System generated locals */
   int32_t i__1;

   /* Local variables */
   double d__1, d__2;
   double dist, xdir, ydir, aopt,  u1min, u2min;
   double abest, scalx, scaly;
   double a1, a2, val2mi, val2pl, dc, sclfac, bigdis, sigsav;
   int32_t nall, iold, line, mpar, ierr, inew, move, next, i, j, nfcol, iercr;
   int32_t idist=0, npcol, kints, i2, i1, lr, nfcnco=0, ki1, ki2, ki3, ke3;
   int32_t nowpts, istrav, nfmxin, isw2, isw4;
   bool ldebug;

   /* Function Body */
   int32_t ke1 = ike1+1;
   int32_t ke2 = ike2+1;
   ldebug = fIdbg[6] >= 1;
   if (ke1 <= 0 || ke2 <= 0) goto L1350;
   if (ke1 > fNu || ke2 > fNu) goto L1350;
   ki1 = fNiofex[ke1-1];
   ki2 = fNiofex[ke2-1];
   if (ki1 <= 0 || ki2 <= 0) goto L1350;
   if (ki1 == ki2) goto L1350;
   if (nptu < 4)  goto L1400;

   nfcnco  = fNfcn;
   fNfcnmx = (nptu + 5)*100*(fNpar + 1);
//*-*-          The minimum
   mncuve();
   u1min  = fU[ke1-1];
   u2min  = fU[ke2-1];
   ierrf  = 0;
   fCfrom = "MNContour ";
   fNfcnfr = nfcnco;
   if (fISW[4] >= 0) {
      log_info(" START MNCONTOUR CALCULATION OF %4d POINTS ON CONTOUR.",nptu);
      if (fNpar > 2) {
         if (fNpar == 3) {
            ki3 = 6 - ki1 - ki2;
            ke3 = fNexofi[ki3-1];
            log_info(" EACH POINT IS A MINIMUM WITH RESPECT TO PARAMETER %3d  %s",ke3,fCpnam[ke3-1].c_str());
         } else {
            log_info(" EACH POINT IS A MINIMUM WITH RESPECT TO THE OTHER %3d VARIABLE PARAMETERS.",fNpar - 2);
         }
      }
   }

//*-*-          Find the first four points using MNMNOT
//*-*-             ........................ first two points
   mnmnot(ke1, ke2, val2pl, val2mi);
   if (fErn[ki1-1] == fUndefi) {
      xptu[0] = fAlim[ke1-1];
      mnwarn("W", "MNContour ", "Contour squeezed by parameter limits.");
   } else {
      if (fErn[ki1-1] >= 0) goto L1500;
      xptu[0] = u1min + fErn[ki1-1];
   }
   yptu[0] = val2mi;

   if (fErp[ki1-1] == fUndefi) {
      xptu[2] = fBlim[ke1-1];
      mnwarn("W", "MNContour ", "Contour squeezed by parameter limits.");
   } else {
      if (fErp[ki1-1] <= 0) goto L1500;
      xptu[2] = u1min + fErp[ki1-1];
   }
   yptu[2] = val2pl;
   scalx = 1 / (xptu[2] - xptu[0]);
//*-*-             ........................... next two points
   mnmnot(ke2, ke1, val2pl, val2mi);
   if (fErn[ki2-1] == fUndefi) {
      yptu[1] = fAlim[ke2-1];
      mnwarn("W", "MNContour ", "Contour squeezed by parameter limits.");
   } else {
      if (fErn[ki2-1] >= 0) goto L1500;
      yptu[1] = u2min + fErn[ki2-1];
   }
   xptu[1] = val2mi;
   if (fErp[ki2-1] == fUndefi) {
      yptu[3] = fBlim[ke2-1];
      mnwarn("W", "MNContour ", "Contour squeezed by parameter limits.");
   } else {
      if (fErp[ki2-1] <= 0) goto L1500;
      yptu[3] = u2min + fErp[ki2-1];
   }
   xptu[3] = val2pl;
   scaly   = 1 / (yptu[3] - yptu[1]);
   nowpts  = 4;
   next    = 5;
   if (ldebug) {
      log_info(" Plot of four points found by MINOS");
      fXpt[0]  = u1min;
      fYpt[0]  = u2min;
      fChpt[0] = ' ';
//*-*  Computing MIN
      nall = std::min(nowpts + 1,101);
      for (i = 2; i <= nall; ++i) {
         fXpt[i-1] = xptu[i-2];
         fYpt[i-1] = yptu[i-2];
      }
      sprintf(fChpt,"%s"," ABCD");
      // mnplot(fXpt, fYpt, fChpt, nall, fNpagwd, fNpagln);
      log_warn("mnplot plotting has been disabled");
   }

//*-*-              ..................... save some values before fixing
   isw2   = fISW[1];
   isw4   = fISW[3];
   sigsav = fEDM;
   istrav = fIstrat;
   dc     = fDcovar;
   fApsi  = fEpsi*.5;
   abest  = fAmin;
   mpar   = fNpar;
   nfmxin = fNfcnmx;
   for (i = 1; i <= mpar; ++i) { fXt[i-1] = fX[i-1]; }
   i__1 = mpar*(mpar + 1) / 2;
   for (j = 1; j <= i__1; ++j) { fVthmat[j-1] = fVhmat[j-1]; }
   for (i = 1; i <= mpar; ++i) {
      fCONTgcc[i-1] = fGlobcc[i-1];
      fCONTw[i-1]   = fWerr[i-1];
   }
//*-*-                          fix the two parameters in question
   kints = fNiofex[ke1-1];
   mnfixp(kints-1, ierr);
   kints = fNiofex[ke2-1];
   mnfixp(kints-1, ierr);
//*-*-              ......................Fill in the rest of the points
   for (inew = next; inew <= nptu; ++inew) {
//*-*            find the two neighbouring points with largest separation
      bigdis = 0;
      for (iold = 1; iold <= inew - 1; ++iold) {
         i2 = iold + 1;
         if (i2 == inew) i2 = 1;
         d__1 = scalx*(xptu[iold-1] - xptu[i2-1]);
         d__2 = scaly*(yptu[iold-1] - yptu[i2-1]);
         dist = d__1*d__1 + d__2*d__2;
         if (dist > bigdis) {
            bigdis = dist;
            idist  = iold;
         }
      }
      i1 = idist;
      i2 = i1 + 1;
      if (i2 == inew) i2 = 1;
//*-*-                  next point goes between I1 and I2
      a1 = .5;
      a2 = .5;
L300:
      fXmidcr = a1*xptu[i1-1] + a2*xptu[i2-1];
      fYmidcr = a1*yptu[i1-1] + a2*yptu[i2-1];
      xdir    = yptu[i2-1] - yptu[i1-1];
      ydir    = xptu[i1-1] - xptu[i2-1];
      sclfac  = std::max(std::abs(xdir*scalx),std::abs(ydir*scaly));
      fXdircr = xdir / sclfac;
      fYdircr = ydir / sclfac;
      fKe1cr  = ke1;
      fKe2cr  = ke2;
//*-*-               Find the contour crossing point along DIR
      fAmin = abest;
      mncros(aopt, iercr);
      if (iercr > 1) {
//*-*-             If cannot find mid-point, try closer to point 1
         if (a1 > .5) {
            if (fISW[4] >= 0) {
               log_info(" MNCONT CANNOT FIND NEXT POINT ON CONTOUR.  ONLY %3d POINTS FOUND.",nowpts);
            }
            goto L950;
         }
         mnwarn("W", "MNContour ", "Cannot find midpoint, try closer.");
         a1 = .75;
         a2 = .25;
         goto L300;
      }
//*-*-               Contour has been located, insert new point in list
      for (move = nowpts; move >= i1 + 1; --move) {
         xptu[move] = xptu[move-1];
         yptu[move] = yptu[move-1];
      }
      ++nowpts;
      xptu[i1] = fXmidcr + fXdircr*aopt;
      yptu[i1] = fYmidcr + fYdircr*aopt;
   }
L950:

   ierrf = nowpts;
   fCstatu = "SUCCESSFUL";
   if (nowpts < nptu)         fCstatu = "INCOMPLETE";

//*-*-               make a lineprinter plot of the contour
   if (fISW[4] >= 0) {
      fXpt[0]  = u1min;
      fYpt[0]  = u2min;
      fChpt[0] = ' ';
      nall = std::min(nowpts + 1,101);
      for (i = 2; i <= nall; ++i) {
         fXpt[i-1]  = xptu[i-2];
         fYpt[i-1]  = yptu[i-2];
         fChpt[i-1] = 'X';
      }
      fChpt[nall] = 0;
      log_info(" Y-AXIS: PARAMETER %3d  %s",ke2,fCpnam[ke2-1].c_str());

      // mnplot(fXpt, fYpt, fChpt, nall, fNpagwd, fNpagln);
      log_warn("mnplot plotting has been disabled");

      log_info("                         X-AXIS: PARAMETER %3d  %s",ke1,fCpnam[ke1-1].c_str());
   }
//*-*-                print out the coordinates around the contour
   if (fISW[4] >= 1) {
      npcol = (nowpts + 1) / 2;
      nfcol = nowpts / 2;
      log_info("%5d POINTS ON CONTOUR.   FMIN=%13.5e   ERRDEF=%11.3g",nowpts,abest,fUp);
      log_info("         %s%s%s%s",fCpnam[ke1-1].c_str(),
                                 fCpnam[ke2-1].c_str(),
                                 fCpnam[ke1-1].c_str(),
                                 fCpnam[ke2-1].c_str());
      for (line = 1; line <= nfcol; ++line) {
         lr = line + npcol;
         log_info(" %5d%13.5e%13.5e          %5d%13.5e%13.5e",line,xptu[line-1],yptu[line-1],lr,xptu[lr-1],yptu[lr-1]);
      }
      if (nfcol < npcol) {
         log_info(" %5d%13.5e%13.5e",npcol,xptu[npcol-1],yptu[npcol-1]);
      }
   }
//*-*-                                   . . contour finished. reset v
   fItaur = 1;
   mnfree(1);
   mnfree(1);
   i__1 = mpar*(mpar + 1) / 2;
   for (j = 1; j <= i__1; ++j) { fVhmat[j-1] = fVthmat[j-1]; }
   for (i = 1; i <= mpar; ++i) {
      fGlobcc[i-1] = fCONTgcc[i-1];
      fWerr[i-1]   = fCONTw[i-1];
      fX[i-1]      = fXt[i-1];
   }
   mninex(fX);
   fEDM    = sigsav;
   fAmin   = abest;
   fISW[1] = isw2;
   fISW[3] = isw4;
   fDcovar = dc;
   fItaur  = 0;
   fNfcnmx = nfmxin;
   fIstrat = istrav;
   fU[ke1-1] = u1min;
   fU[ke2-1] = u2min;
   goto L2000;
//*-*-                                    Error returns
L1350:
   log_info(" INVALID PARAMETER NUMBERS.");
   goto L1450;
L1400:
   log_info(" LESS THAN FOUR POINTS REQUESTED.");
L1450:
   ierrf   = -1;
   fCstatu = "USER ERROR";
   goto L2000;
L1500:
   log_info(" MNCONT UNABLE TO FIND FOUR POINTS.");
   fU[ke1-1] = u1min;
   fU[ke2-1] = u2min;
   ierrf     = 0;
   fCstatu   = "FAILED";
L2000:
   fCfrom  = "MNContour ";
   fNfcnfr = nfcnco;
} /* mncont_ */

//______________________________________________________________________________
void lilliput::TMinuit::mncrck(std::string cardbuf, int32_t maxcwd, std::string &comand, int32_t &lnc,
        int32_t mxp, double *plist, int32_t &llist, int32_t &ierr, int32_t)
{
//*-*-*-*-*-*-*-*-*-*-*-*Cracks the free-format input*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                    ============================
//*-*       Cracks the free-format input, expecting zero or more
//*-*         alphanumeric fields (which it joins into COMAND(1:LNC))
//*-*         followed by one or more numeric fields separated by
//*-*         blanks and/or one comma.  The numeric fields are put into
//*-*         the LLIST (but at most MXP) elements of PLIST.
//*-*      IERR = 0 if no errors,
//*-*           = 1 if error(s).
//*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
   /* Initialized data */

   char *cnull  = 0;
   const char *cnumer = "123456789-.0+";

   /* Local variables */
   int32_t ifld, iend, lend, left, nreq, ipos, kcmnd, nextb, ic, ibegin, ltoadd;
   int32_t ielmnt, lelmnt[25], nelmnt;
   std::string ctemp;
   const char *celmnt[25];
   char command[25];

   /* Function Body */
   const char *crdbuf = cardbuf.c_str();
   lend   = cardbuf.length();
   ielmnt = 0;
   nextb  = 1;
   ierr   = 0;
//*-*-                                  . . . .  loop over words CELMNT
L10:
   for (ipos = nextb; ipos <= lend; ++ipos) {
      ibegin = ipos;
      if (crdbuf[ipos-1] == ' ') continue;
      if (crdbuf[ipos-1] == ',') goto L250;
      goto L150;
   }
   goto L300;
L150:
//*-*-              found beginning of word, look for end
   for (ipos = ibegin + 1; ipos <= lend; ++ipos) {
      if (crdbuf[ipos-1] == ' ') goto L250;
      if (crdbuf[ipos-1] == ',') goto L250;
   }
   ipos = lend + 1;
L250:
   iend = ipos - 1;
   ++ielmnt;
   if (iend >= ibegin) celmnt[ielmnt-1] = &crdbuf[ibegin-1];
   else                celmnt[ielmnt-1] = cnull;
   lelmnt[ielmnt-1] = iend - ibegin + 1;
   if (lelmnt[ielmnt-1] > 19) {
      log_info(" MINUIT WARNING: INPUT DATA WORD TOO LONG.");
      ctemp = cardbuf.substr(ibegin-1,iend-ibegin+1);
      log_info("     ORIGINAL:%s",ctemp.c_str());
      log_info(" TRUNCATED TO:%s",celmnt[ielmnt-1]);
      lelmnt[ielmnt-1] = 19;
   }
   if (ipos >= lend) goto L300;
   if (ielmnt >= 25) goto L300;
//*-*-                    look for comma or beginning of next word
   for (ipos = iend + 1; ipos <= lend; ++ipos) {
      if (crdbuf[ipos-1] == ' ') continue;
      nextb = ipos;
      if (crdbuf[ipos-1] == ',') nextb = ipos + 1;
      goto L10;
   }
//*-*-                All elements found, join the alphabetic ones to
//*-*-                               form a command
L300:
   nelmnt      = ielmnt;
   command[0]  = ' '; command[1] = 0;
   lnc         = 1;
   plist[0]    = 0;
   llist       = 0;
   if (ielmnt == 0) goto L900;
   kcmnd = 0;
   for (ielmnt = 1; ielmnt <= nelmnt; ++ielmnt) {
      if ( celmnt[ielmnt-1] == cnull) goto L450;
      for (ic = 1; ic <= 13; ++ic) {
         if (*celmnt[ielmnt-1] == cnumer[ic-1]) goto L450;
      }
      if (kcmnd >= maxcwd) continue;
      left = maxcwd - kcmnd;
      ltoadd = lelmnt[ielmnt-1];
      if (ltoadd > left) ltoadd = left;
      strncpy(&command[kcmnd],celmnt[ielmnt-1],ltoadd);
      kcmnd += ltoadd;
      if (kcmnd == maxcwd) continue;
      command[kcmnd] = ' ';
      ++kcmnd;
      command[kcmnd] = 0;
   }
   lnc = kcmnd;
   goto L900;
L450:
   lnc = kcmnd;
//*-*-                     . . . .  we have come to a numeric field
   llist = 0;
   for (ifld = ielmnt; ifld <= nelmnt; ++ifld) {
      ++llist;
      if (llist > mxp) {
         nreq = nelmnt - ielmnt + 1;
         log_info(" MINUIT WARNING IN MNCRCK: ");
         log_info(" COMMAND HAS INPUT %5d NUMERIC FIELDS, BUT MINUIT CAN ACCEPT ONLY%3d",nreq,mxp);
         goto L900;
      }
      if (celmnt[ifld-1] == cnull) plist[llist-1] = 0;
      else {
         sscanf(celmnt[ifld-1],"%lf",&plist[llist-1]);
      }
   }
//*-*-                                 end loop over numeric fields
L900:
   if (lnc <= 0) lnc = 1;
   comand = command;
} /* mncrck_ */

//______________________________________________________________________________
void lilliput::TMinuit::mncros(double &aopt, int32_t &iercr)
{
//*-*-*-*-*-*-*-*-*-*-*Find point where MNEVAL=AMIN+UP*-*-*-*-*-*-*-*-*-*-*-*
//*-*                  ===============================
//*-*       Find point where MNEVAL=AMIN+UP, along the line through
//*-*       XMIDCR,YMIDCR with direction XDIRCR,YDIRCR,   where X and Y
//*-*       are parameters KE1CR and KE2CR.  If KE2CR=0 (from MINOS),
//*-*       only KE1CR is varied.  From MNCONT, both are varied.
//*-*       Crossing point is at
//*-*        (U(KE1),U(KE2)) = (XMID,YMID) + AOPT*(XDIR,YDIR)
//*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double alsb[3], flsb[3], bmin, bmax, zmid, sdev, zdir, zlim;
   double coeff[3], aleft, aulim, fdist, adist, aminsv;
   double anext, fnext, slope, s1, s2, x1, x2, ecarmn, ecarmx;
   double determ, rt, smalla, aright, aim, tla, tlf, dfda,ecart;
   int32_t iout=0, i, ileft, ierev, maxlk, ibest, ik, it;
   int32_t noless, iworst=0, iright, itoohi, kex, ipt;
   bool ldebug;
   const char *chsign;
   x2 = 0;

   ldebug = fIdbg[6] >= 1;
   aminsv = fAmin;
//*-*-       convergence when F is within TLF of AIM and next prediction
//*-*-       of AOPT is within TLA of previous value of AOPT
   aim      = fAmin + fUp;
   tlf      = fUp*.01;
   tla      = .01;
   fXpt[0]  = 0;
   fYpt[0]  = aim;
   fChpt[0] = ' ';
   ipt = 1;
   if (fKe2cr == 0) {
      fXpt[1]  = -1;
      fYpt[1]  = fAmin;
      fChpt[1] = '.';
      ipt      = 2;
   }
//*-*-                   find the largest allowed A
   aulim = 100;
   for (ik = 1; ik <= 2; ++ik) {
      if (ik == 1) {
         kex  = fKe1cr;
         zmid = fXmidcr;
         zdir = fXdircr;
      } else {
         if (fKe2cr == 0) continue;
         kex  = fKe2cr;
         zmid = fYmidcr;
         zdir = fYdircr;
      }
      if (fNvarl[kex-1] <= 1) continue;
      if (zdir == 0) continue;
      zlim = fAlim[kex-1];
      if (zdir > 0) zlim = fBlim[kex-1];
      aulim = std::min(aulim,(zlim - zmid) / zdir);
   }
//*-*-                 LSB = Line Search Buffer
//*-*-         first point
   anext   = 0;
   aopt    = anext;
   fLimset = false;
   if (aulim < aopt + tla) fLimset = true;
   mneval(anext, fnext, ierev);
//*-* debug printout:
   if (ldebug) {
      log_info(" MNCROS: calls=%8d   AIM=%10.5f  F,A=%10.5f%10.5f",fNfcn,aim,fnext,aopt);
   }
   if (ierev > 0) goto L900;
   if (fLimset && fnext <= aim) goto L930;
   ++ipt;
   fXpt[ipt-1]  = anext;
   fYpt[ipt-1]  = fnext;
   fChpt[ipt-1] = charal[ipt-1];
   alsb[0] = anext;
   flsb[0] = fnext;
   fnext   = std::max(fnext,aminsv + fUp*.1);
   aopt    = std::sqrt(fUp / (fnext - aminsv)) - 1;
   if (std::abs(fnext - aim) < tlf) goto L800;

   if (aopt < -.5)aopt = -.5;
   if (aopt > 1)  aopt = 1;
   fLimset = false;
   if (aopt > aulim) {
      aopt    = aulim;
      fLimset = true;
   }
   mneval(aopt, fnext, ierev);
//*-* debug printout:
   if (ldebug) {
      log_info(" MNCROS: calls=%8d   AIM=%10.5f  F,A=%10.5f%10.5f",fNfcn,aim,fnext,aopt);
   }
   if (ierev > 0) goto L900;
   if (fLimset && fnext <= aim) goto L930;
   alsb[1] = aopt;
   ++ipt;
   fXpt[ipt-1]  = alsb[1];
   fYpt[ipt-1]  = fnext;
   fChpt[ipt-1] = charal[ipt-1];
   flsb[1]      = fnext;
   dfda         = (flsb[1] - flsb[0]) / (alsb[1] - alsb[0]);
//*-*-                  DFDA must be positive on the contour
   if (dfda > 0) goto L460;
L300:
   mnwarn("D", "MNCROS    ", "Looking for slope of the right sign");
   maxlk = 15 - ipt;
   for (it = 1; it <= maxlk; ++it) {
      alsb[0] = alsb[1];
      flsb[0] = flsb[1];
      aopt    = alsb[0] + double(it)*.2;
      fLimset = false;
      if (aopt > aulim) {
         aopt    = aulim;
         fLimset = true;
      }
      mneval(aopt, fnext, ierev);
//*-* debug printout:
      if (ldebug) {
         log_info(" MNCROS: calls=%8d   AIM=%10.5f  F,A=%10.5f%10.5f",fNfcn,aim,fnext,aopt);
      }
      if (ierev > 0) goto L900;
      if (fLimset && fnext <= aim) goto L930;
      alsb[1] = aopt;
      ++ipt;
      fXpt[ipt-1]  = alsb[1];
      fYpt[ipt-1]  = fnext;
      fChpt[ipt-1] = charal[ipt-1];
      flsb[1]      = fnext;
      dfda         = (flsb[1] - flsb[0]) / (alsb[1] - alsb[0]);
      if (dfda > 0) goto L450;
   }
   mnwarn("W", "MNCROS    ", "Cannot find slope of the right sign");
   goto L950;
L450:
//*-*-                   we have two points with the right slope
L460:
   aopt  = alsb[1] + (aim - flsb[1]) / dfda;
   fdist = std::min(std::abs(aim - flsb[0]),std::abs(aim - flsb[1]));
   adist = std::min(std::abs(aopt - alsb[0]),std::abs(aopt - alsb[1]));
   tla = .01;
   if (std::abs(aopt) > 1) tla = std::abs(aopt)*.01;
   if (adist < tla && fdist < tlf) goto L800;
   if (ipt >= 15) goto L950;
   bmin = std::min(alsb[0],alsb[1]) - 1;
   if (aopt < bmin) aopt = bmin;
   bmax = std::max(alsb[0],alsb[1]) + 1;
   if (aopt > bmax) aopt = bmax;
//*-*-                   Try a third point
   fLimset = false;
   if (aopt > aulim) {
      aopt    = aulim;
      fLimset = true;
   }
   mneval(aopt, fnext, ierev);
//*-* debug printout:
   if (ldebug) {
      log_info(" MNCROS: calls=%8d   AIM=%10.5f  F,A=%10.5f%10.5f",fNfcn,aim,fnext,aopt);
   }
   if (ierev > 0) goto L900;
   if (fLimset && fnext <= aim) goto L930;
   alsb[2] = aopt;
   ++ipt;
   fXpt[ipt-1]  = alsb[2];
   fYpt[ipt-1]  = fnext;
   fChpt[ipt-1] = charal[ipt-1];
   flsb[2]      = fnext;
//*-*-               now we have three points, ask how many <AIM
   ecarmn = std::abs(fnext-aim);
   ibest  = 3;
   ecarmx = 0;
   noless = 0;
   for (i = 1; i <= 3; ++i) {
      ecart = std::abs(flsb[i-1] - aim);
      if (ecart > ecarmx) { ecarmx = ecart; iworst = i; }
      if (ecart < ecarmn) { ecarmn = ecart; ibest = i; }
      if (flsb[i-1] < aim) ++noless;
   }
//*-*-          if at least one on each side of AIM, fit a parabola
   if (noless == 1 || noless == 2) goto L500;
//*-*-          if all three are above AIM, third must be closest to AIM
   if (noless == 0 && ibest != 3) goto L950;
//*-*-          if all three below, and third is not best, then slope
//*-*-            has again gone negative, look for positive slope.
   if (noless == 3 && ibest != 3) {
      alsb[1] = alsb[2];
      flsb[1] = flsb[2];
      goto L300;
   }
//*-*-          in other cases, new straight line thru last two points
   alsb[iworst-1] = alsb[2];
   flsb[iworst-1] = flsb[2];
   dfda = (flsb[1] - flsb[0]) / (alsb[1] - alsb[0]);
   goto L460;
//*-*-               parabola fit
L500:
   mnpfit(alsb, flsb, 3, coeff, sdev);
   if (coeff[2] <= 0) {
      mnwarn("D", "MNCROS    ", "Curvature is negative near contour line.");
   }
   determ = coeff[1]*coeff[1] - coeff[2]*4*(coeff[0] - aim);
   if (determ <= 0) {
      mnwarn("D", "MNCROS    ", "Problem 2, impossible determinant");
      goto L950;
   }
//*-*-               Find which root is the right one
   rt = std::sqrt(determ);
   x1 = (-coeff[1] + rt) / (coeff[2]*2);
   x2 = (-coeff[1] - rt) / (coeff[2]*2);
   s1 = coeff[1] + x1*2*coeff[2];
   s2 = coeff[1] + x2*2*coeff[2];
   if (s1*s2 > 0) {
      log_info(" MNCONTour problem 1");
   }
   aopt  = x1;
   slope = s1;
   if (s2 > 0) {
      aopt  = x2;
      slope = s2;
   }
//*-*-        ask if converged
   tla = .01;
   if (std::abs(aopt) > 1) tla = std::abs(aopt)*.01;
   if (std::abs(aopt - alsb[ibest-1]) < tla && std::abs(flsb[ibest-1] - aim) < tlf) {
      goto L800;
   }
   if (ipt >= 15) goto L950;

//*-*-        see if proposed point is in acceptable zone between L and R
//*-*-        first find ILEFT, IRIGHT, IOUT and IBEST
   ileft  = 0;
   iright = 0;
   ibest  = 1;
   ecarmx = 0;
   ecarmn = std::abs(aim - flsb[0]);
   for (i = 1; i <= 3; ++i) {
      ecart = std::abs(flsb[i-1] - aim);
      if (ecart < ecarmn) { ecarmn = ecart; ibest = i; }
      if (ecart > ecarmx) { ecarmx = ecart; }
      if (flsb[i-1] > aim) {
         if (iright == 0) iright = i;
         else if (flsb[i-1] > flsb[iright-1]) iout = i;
         else { iout = iright; iright = i; }
      }
      else if (ileft == 0) ileft = i;
      else if (flsb[i-1] < flsb[ileft-1]) iout = i;
      else { iout = ileft; ileft = i;        }
   }
//*-*-      avoid keeping a very bad point next time around
   if (ecarmx > std::abs(flsb[iout-1] - aim)*10) {
      aopt = aopt*.5 + (alsb[iright-1] + alsb[ileft-1])*.25;
   }
//*-*-        knowing ILEFT and IRIGHT, get acceptable window
   smalla = tla*.1;
   if (slope*smalla > tlf) smalla = tlf / slope;
   aleft  = alsb[ileft-1] + smalla;
   aright = alsb[iright-1] - smalla;
//*-*-        move proposed point AOPT into window if necessary
   if (aopt < aleft)   aopt = aleft;
   if (aopt > aright)  aopt = aright;
   if (aleft > aright) aopt = (aleft + aright)*.5;

//*-*-        see if proposed point outside limits (should be impossible!)
   fLimset = false;
   if (aopt > aulim) {
      aopt    = aulim;
      fLimset = true;
   }
//*-*-                 Evaluate function at new point AOPT
   mneval(aopt, fnext, ierev);
//*-* debug printout:
   if (ldebug) {
      log_info(" MNCROS: calls=%8d   AIM=%10.5f  F,A=%10.5f%10.5f",fNfcn,aim,fnext,aopt);
   }
   if (ierev > 0) goto L900;
   if (fLimset && fnext <= aim) goto L930;
   ++ipt;
   fXpt[ipt-1]  = aopt;
   fYpt[ipt-1]  = fnext;
   fChpt[ipt-1] = charal[ipt-1];
//*-*-               Replace odd point by new one
   alsb[iout-1] = aopt;
   flsb[iout-1] = fnext;
//*-*-         the new point may not be the best, but it is the only one
//*-*-         which could be good enough to pass convergence criteria
   ibest = iout;
   goto L500;

//*-*-      Contour has been located, return point to MNCONT OR MINOS
L800:
   iercr = 0;
   goto L1000;
//*-*-               error in the minimization
L900:
   if (ierev == 1) goto L940;
   goto L950;
//*-*-               parameter up against limit
L930:
   iercr = 1;
   goto L1000;
//*-*-               too many calls to FCN
L940:
   iercr = 2;
   goto L1000;
//*-*-               cannot find next point
L950:
   iercr = 3;
//*-*-               in any case
L1000:
   if (ldebug) {
      itoohi = 0;
      for (i = 1; i <= ipt; ++i) {
         if (fYpt[i-1] > aim + fUp) {
            fYpt[i-1]  = aim + fUp;
            fChpt[i-1] = '+';
            itoohi     = 1;
         }
      }
      fChpt[ipt] = 0;
      chsign = "POSI";
      if (fXdircr < 0) chsign = "NEGA";
      if (fKe2cr == 0) {
         log_info("  %sTIVE MINOS ERROR, PARAMETER %3d",chsign,fKe1cr);
      }
      if (itoohi == 1) {
         log_info("POINTS LABELLED '+' WERE TOO HIGH TO PLOT.");
      }
      if (iercr == 1) {
         log_info("RIGHTMOST POINT IS UP AGAINST LIMIT.");
      }
      // mnplot(fXpt, fYpt, fChpt, ipt, fNpagwd, fNpagln);
      log_warn("mnplot plotting has been disabled");
   }
} /* mncros_ */

//______________________________________________________________________________
void lilliput::TMinuit::mncuve()
{
//*-*-*-*-*-*-*-*Makes sure that the current point is a local minimum*-*-*-*-*
//*-*            ====================================================
//*-*        Makes sure that the current point is a local
//*-*        minimum and that the error matrix exists,
//*-*        or at least something good enough for MINOS and MNCONT
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double dxdi, wint;
   int32_t ndex, iext, i, j;

   if (fISW[3] < 1) {
      log_info(" FUNCTION MUST BE MINIMIZED BEFORE CALLING %s",fCfrom.c_str());
      fApsi = fEpsi;
      mnmigr();
   }
   if (fISW[1] < 3) {
      mnhess();
      if (fISW[1] < 1) {
         mnwarn("W", fCfrom.c_str(), "NO ERROR MATRIX.  WILL IMPROVISE.");
         for (i = 1; i <= fNpar; ++i) {
            ndex = i*(i-1) / 2;
            for (j = 1; j <= i-1; ++j) {
               ++ndex;
               fVhmat[ndex-1] = 0;
            }
            ++ndex;
            if (fG2[i-1] <= 0) {
               wint = fWerr[i-1];
               iext = fNexofi[i-1];
               if (fNvarl[iext-1] > 1) {
                  mndxdi(fX[i-1], i-1, dxdi);
                  if (std::abs(dxdi) < .001) wint = .01;
                  else                   wint /= std::abs(dxdi);
               }
               fG2[i-1] = fUp / (wint*wint);
            }
            fVhmat[ndex-1] = 2 / fG2[i-1];
         }
         fISW[1] = 1;
         fDcovar = 1;
      } else  mnwerr();
   }
} /* mncuve_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnderi()
{
//*-*-*-*-*-*-*-*Calculates the first derivatives of FCN (GRD)*-*-*-*-*-*-*-*
//*-*            =============================================
//*-*        Calculates the first derivatives of FCN (GRD),
//*-*        either by finite differences or by transforming the user-
//*-*        supplied derivatives to internal coordinates,
//*-*        according to whether fISW[2] is zero or one.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double step, dfmin, stepb4, dd, df, fs1;
   double tlrstp, tlrgrd, epspri, optstp, stpmax, stpmin, fs2, grbfor=0, d1d2, xtf;
   int32_t icyc, ncyc, iint, iext, i, nparx;
   bool ldebug;

   nparx = fNpar;
   ldebug = fIdbg[2] >= 1;
   if (fAmin == fUndefi) mnamin();
   if (fISW[2] == 1) goto L100;

   if (ldebug) {
//*-*-                      make sure starting at the right place
      mninex(fX);
      nparx = fNpar;
      Eval(nparx, fGin, fs1, fU, 4);        ++fNfcn;
      if (fs1 != fAmin) {
         df    = fAmin - fs1;
         mnwarn("D", "MNDERI", (boost::format("function value differs from AMIN by %12.3g") % df).str().c_str());
         fAmin = fs1;
      }
      log_info("  FIRST DERIVATIVE DEBUG PRINTOUT.  MNDERI");
      log_info(" PAR    DERIV     STEP      MINSTEP   OPTSTEP  D1-D2    2ND DRV");
   }
   dfmin = fEpsma2*8*(std::abs(fAmin) + fUp);
   if (fIstrat <= 0) {
      ncyc   = 2;
      tlrstp = .5;
      tlrgrd = .1;
   } else if (fIstrat == 1) {
      ncyc   = 3;
      tlrstp = .3;
      tlrgrd = .05;
   } else {
      ncyc   = 5;
      tlrstp = .1;
      tlrgrd = .02;
   }
//*-*-                               loop over variable parameters
   for (i = 1; i <= fNpar; ++i) {
      epspri = fEpsma2 + std::abs(fGrd[i-1]*fEpsma2);
//*-*-        two-point derivatives always assumed necessary
//*-*-        maximum number of cycles over step size depends on strategy
      xtf = fX[i-1];
      stepb4 = 0;
//*-*-                              loop as little as possible here!/
      for (icyc = 1; icyc <= ncyc; ++icyc) {
//*-*-                ........ theoretically best step
         optstp = std::sqrt(dfmin / (std::abs(fG2[i-1]) + epspri));
//*-*-                    step cannot decrease by more than a factor of ten
         step = std::max(optstp,std::abs(fGstep[i-1]*.1));
//*-*-                but if parameter has limits, max step size = 0.5
         if (fGstep[i-1] < 0 && step > .5) step = .5;
//*-*-                and not more than ten times the previous step
         stpmax = std::abs(fGstep[i-1])*10;
         if (step > stpmax) step = stpmax;
//*-*-                minimum step size allowed by machine precision
         stpmin = std::abs(fEpsma2*fX[i-1])*8;
         if (step < stpmin) step = stpmin;
//*-*-                end of iterations if step change less than factor 2
         if (std::abs((step - stepb4) / step) < tlrstp) goto L50;
//*-*-        take step positive
         stepb4 = step;
         if (fGstep[i-1] > 0) fGstep[i-1] =  std::abs(step);
         else                 fGstep[i-1] = -std::abs(step);
         stepb4  = step;
         fX[i-1] = xtf + step;
         mninex(fX);
         Eval(nparx, fGin, fs1, fU, 4);            ++fNfcn;
//*-*-        take step negative
         fX[i-1] = xtf - step;
         mninex(fX);
         Eval(nparx, fGin, fs2, fU, 4);            ++fNfcn;
         grbfor = fGrd[i-1];
         fGrd[i-1] = (fs1 - fs2) / (step*2);
         fG2[i-1]  = (fs1 + fs2 - fAmin*2) / (step*step);
         fX[i-1]   = xtf;
         if (ldebug) {
            d1d2 = (fs1 + fs2 - fAmin*2) / step;
            log_info("%4d%11.3g%11.3g%10.2g%10.2g%10.2g%10.2g",i,fGrd[i-1],step,stpmin,optstp,d1d2,fG2[i-1]);
         }
//*-*-        see if another iteration is necessary
         if (std::abs(grbfor - fGrd[i-1]) / (std::abs(fGrd[i-1]) + dfmin/step) < tlrgrd)
            goto L50;
      }
//*-*-                          end of ICYC loop. too many iterations
      if (ncyc == 1) goto L50;
      mnwarn("D", "MNDERI", (boost::format("First derivative not converged. %g%g") % fGrd[i-1] % grbfor).str().c_str());
L50:
      ;
   }
   mninex(fX);
   return;
//*-*-                                       .  derivatives calc by fcn
L100:
   for (iint = 1; iint <= fNpar; ++iint) {
      iext = fNexofi[iint-1];
      if (fNvarl[iext-1] <= 1) {
         fGrd[iint-1] = fGin[iext-1];
      } else {
         dd = (fBlim[iext-1] - fAlim[iext-1])*.5*std::cos(fX[iint-1]);
         fGrd[iint-1] = fGin[iext-1]*dd;
      }
   }
} /* mnderi_ */

//______________________________________________________________________________
void lilliput::TMinuit::mndxdi(double pint, int32_t ipar, double &dxdi)
{
//*-*-*-*Calculates the transformation factor between ext/internal values*-*
//*-*    =====================================================================
//*-*        calculates the transformation factor between external and
//*-*        internal parameter values.     this factor is one for
//*-*        parameters which are not limited.     called from MNEMAT.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   int32_t i = fNexofi[ipar];
   dxdi = 1;
   if (fNvarl[i-1] > 1) {
      dxdi = std::abs((fBlim[i-1] - fAlim[i-1])*std::cos(pint))*.5;
   }
} /* mndxdi_ */

//______________________________________________________________________________
void lilliput::TMinuit::mneig(double *a, int32_t ndima, int32_t n, int32_t mits, double *work, double precis, int32_t &ifault)
{
//*-*-*-*-*-*-*-*-*-*-*-*Compute matrix eigen values*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                    ===========================
   /* System generated locals */
   int32_t a_offset;
   double d__1;

   /* Local variables */
   double b, c, f, h, r, s, hh, gl, pr, pt;
   int32_t i, j, k, l, m=0, i0, i1, j1, m1, n1;

//*-*-         PRECIS is the machine precision EPSMAC
   /* Parameter adjustments */
   a_offset = ndima + 1;
   a -= a_offset;
   --work;

   /* Function Body */
   ifault = 1;

   i = n;
   for (i1 = 2; i1 <= n; ++i1) {
      l  = i-2;
      f  = a[i + (i-1)*ndima];
      gl = 0;

      if (l < 1) goto L25;

      for (k = 1; k <= l; ++k) {
         d__1 = a[i + k*ndima];
         gl  += d__1*d__1;
      }
L25:
      h = gl + f*f;

      if (gl > 1e-35) goto L30;

      work[i]     = 0;
      work[n + i] = f;
      goto L65;
L30:
      ++l;
      gl = std::sqrt(h);
      if (f >= 0) gl = -gl;
      work[n + i] = gl;
      h -= f*gl;
      a[i + (i-1)*ndima] = f - gl;
      f = 0;
      for (j = 1; j <= l; ++j) {
         a[j + i*ndima] = a[i + j*ndima] / h;
         gl = 0;
         for (k = 1; k <= j; ++k) { gl += a[j + k*ndima]*a[i + k*ndima]; }
         if (j >= l) goto L47;
         j1 = j + 1;
         for (k = j1; k <= l; ++k) {        gl += a[k + j*ndima]*a[i + k*ndima]; }
L47:
         work[n + j] = gl / h;
         f += gl*a[j + i*ndima];
      }
      hh = f / (h + h);
      for (j = 1; j <= l; ++j) {
         f  = a[i + j*ndima];
         gl = work[n + j] - hh*f;
         work[n + j] = gl;
         for (k = 1; k <= j; ++k) {
            a[j + k*ndima] = a[j + k*ndima] - f*work[n + k] - gl*a[i + k*ndima];
         }
      }
      work[i] = h;
L65:
      --i;
   }
   work[1] = 0;
   work[n + 1] = 0;
   for (i = 1; i <= n; ++i) {
      l = i-1;
      if (work[i] == 0 || l == 0) goto L100;

      for (j = 1; j <= l; ++j) {
         gl = 0;
         for (k = 1; k <= l; ++k) { gl += a[i + k*ndima]*a[k + j*ndima]; }
         for (k = 1; k <= l; ++k) { a[k + j*ndima] -= gl*a[k + i*ndima]; }
      }
L100:
      work[i] = a[i + i*ndima];
      a[i + i*ndima] = 1;
      if (l == 0) continue;

      for (j = 1; j <= l; ++j) {
         a[i + j*ndima] = 0;
         a[j + i*ndima] = 0;
      }
   }

   n1 = n - 1;
   for (i = 2; i <= n; ++i) {
      i0 = n + i-1;
      work[i0] = work[i0 + 1];
   }
   work[n + n] = 0;
   b = 0;
   f = 0;
   for (l = 1; l <= n; ++l) {
      j = 0;
      h = precis*(std::abs(work[l]) + std::abs(work[n + l]));
      if (b < h) b = h;
      for (m1 = l; m1 <= n; ++m1) {
         m = m1;
         if (std::abs(work[n + m]) <= b)        goto L150;
      }

L150:
      if (m == l) goto L205;

L160:
      if (j == mits) return;
      ++j;
      pt = (work[l + 1] - work[l]) / (work[n + l]*2);
      r  = std::sqrt(pt*pt + 1);
      pr = pt + r;
      if (pt < 0) pr = pt - r;

      h = work[l] - work[n + l] / pr;
      for (i = l; i <= n; ++i) { work[i] -= h; }
      f += h;
      pt = work[m];
      c  = 1;
      s  = 0;
      m1 = m - 1;
      i  = m;
      for (i1 = l; i1 <= m1; ++i1) {
         j = i;
         --i;
         gl = c*work[n + i];
         h  = c*pt;
         if (std::abs(pt) >= std::abs(work[n + i])) goto L180;

         c = pt / work[n + i];
         r = std::sqrt(c*c + 1);
         work[n + j] = s*work[n + i]*r;
         s  = 1 / r;
         c /= r;
         goto L190;
L180:
         c = work[n + i] / pt;
         r = std::sqrt(c*c + 1);
         work[n + j] = s*pt*r;
         s = c / r;
         c = 1 / r;
L190:
         pt = c*work[i] - s*gl;
         work[j] = h + s*(c*gl + s*work[i]);
         for (k = 1; k <= n; ++k) {
            h = a[k + j*ndima];
            a[k + j*ndima] = s*a[k + i*ndima] + c*h;
            a[k + i*ndima] = c*a[k + i*ndima] - s*h;
         }
      }
      work[n + l] = s*pt;
      work[l]     = c*pt;

      if (std::abs(work[n + l]) > b) goto L160;

L205:
      work[l] += f;
   }
   for (i = 1; i <= n1; ++i) {
      k  = i;
      pt = work[i];
      i1 = i + 1;
      for (j = i1; j <= n; ++j) {
         if (work[j] >= pt) continue;
         k  = j;
         pt = work[j];
      }

      if (k == i) continue;

      work[k] = work[i];
      work[i] = pt;
      for (j = 1; j <= n; ++j) {
         pt = a[j + i*ndima];
         a[j + i*ndima] = a[j + k*ndima];
         a[j + k*ndima] = pt;
      }
   }
   ifault = 0;
} /* mneig_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnemat(double *emat, int32_t ndim)
{
// Calculates the external error matrix from the internal matrix
//
// Note that if the matrix is declared like double matrix[5][5]
// in the calling program, one has to call mnemat with, eg
//     gMinuit->mnemat(&matrix[0][0],5);

   /* System generated locals */
   int32_t emat_dim1, emat_offset;

   /* Local variables */
   double dxdi, dxdj;
   int32_t i, j, k, npard, k2, kk, iz, nperln, kga, kgb;
   std::string ctemp;

   /* Parameter adjustments */
   emat_dim1 = ndim;
   emat_offset = emat_dim1 + 1;
   emat -= emat_offset;

   /* Function Body */
   if (fISW[1] < 1) return;
   if (fISW[4] >= 2) {
      log_info(" EXTERNAL ERROR MATRIX.    NDIM=%4d    NPAR=%3d    ERR DEF=%g",ndim,fNpar,fUp);
   }
//*-*-                   size of matrix to be printed
   npard = fNpar;
   if (ndim < fNpar) {
      npard = ndim;
      if (fISW[4] >= 0) {
         log_info(" USER-DIMENSIONED  ARRAY EMAT NOT BIG ENOUGH. REDUCED MATRIX CALCULATED.");
      }
   }
//*-*-                NPERLN is the number of elements that fit on one line

   nperln = (fNpagwd - 5) / 10;
   nperln = std::min(nperln,13);
   if (fISW[4] >= 1 && npard > nperln) {
      log_info(" ELEMENTS ABOVE DIAGONAL ARE NOT PRINTED.");
   }
//*-*-                I counts the rows of the matrix
   for (i = 1; i <= npard; ++i) {
      mndxdi(fX[i-1], i-1, dxdi);
      kga = i*(i-1) / 2;
      for (j = 1; j <= i; ++j) {
         mndxdi(fX[j-1], j-1, dxdj);
         kgb = kga + j;
         emat[i + j*emat_dim1] = dxdi*fVhmat[kgb-1]*dxdj*fUp;
         emat[j + i*emat_dim1] = emat[i + j*emat_dim1];
      }
   }
//*-*-                   IZ is number of columns to be printed in row I
   if (fISW[4] >= 2) {
      for (i = 1; i <= npard; ++i) {
         iz = npard;
         if (npard >= nperln) iz = i;
         ctemp = " ";
         for (k = 1; nperln < 0 ? k >= iz : k <= iz; k += nperln) {
            k2 = k + nperln - 1;
            if (k2 > iz) k2 = iz;
            for (kk = k; kk <= k2; ++kk) {
               ctemp += boost::str(boost::format("%10.3e ") % emat[i + kk*emat_dim1]);
            }
            log_info("%s",ctemp.c_str());
         }
      }
   }
} /* mnemat_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnerrs(int32_t number, double &eplus, double &eminus, double &eparab, double &gcc)
{
//*-*-*-*-*-*-*-*-*-*Utility routine to get MINOS errors*-*-*-*-*-*-*-*-*-*-*
//*-*                ===================================
//*-*    Called by user.
//*-*    NUMBER is the parameter number
//*-*    values returned by MNERRS:
//*-*       EPLUS, EMINUS are MINOS errors of parameter NUMBER,
//*-*       EPARAB is 'parabolic' error (from error matrix).
//*-*                 (Errors not calculated are set = 0)
//*-*       GCC is global correlation coefficient from error matrix
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   double dxdi;
   int32_t ndiag, iin, iex;

   iex = number+1;

   if (iex > fNu || iex <= 0) goto L900;
   iin = fNiofex[iex-1];
   if (iin <= 0) goto L900;

//*-*-            IEX is external number, IIN is internal number
   eplus  = fErp[iin-1];
   if (eplus == fUndefi)  eplus = 0;
   eminus = fErn[iin-1];
   if (eminus == fUndefi) eminus = 0;
   mndxdi(fX[iin-1], iin-1, dxdi);
   ndiag  = iin*(iin + 1) / 2;
   eparab = std::abs(dxdi*std::sqrt(std::abs(fUp*fVhmat[ndiag- 1])));
//*-*-             global correlation coefficient
   gcc = 0;
   if (fISW[1] < 2) return;
   gcc = fGlobcc[iin-1];
   return;
//*-*-                 ERROR.  parameter number not valid
L900:
   eplus  = 0;
   eminus = 0;
   eparab = 0;
   gcc    = 0;
} /* mnerrs_ */

//______________________________________________________________________________
void lilliput::TMinuit::mneval(double anext, double &fnext, int32_t &ierev)
{
//*-*-*-*-*-*-*Evaluates the function being analyzed by MNCROS*-*-*-*-*-*-*-*
//*-*          ===============================================
//*-*      Evaluates the function being analyzed by MNCROS, which is
//*-*      generally the minimum of FCN with respect to all remaining
//*-*      variable parameters.  The class data members contains the
//*-*      data necessary to know the values of U(KE1CR) and U(KE2CR)
//*-*      to be used, namely     U(KE1CR) = XMIDCR + ANEXT*XDIRCR
//*-*      and (if KE2CR .NE. 0)  U(KE2CR) = YMIDCR + ANEXT*YDIRCR
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   int32_t nparx;

   fU[fKe1cr-1] = fXmidcr + anext*fXdircr;
   if (fKe2cr != 0) fU[fKe2cr-1] = fYmidcr + anext*fYdircr;
   mninex(fX);
   nparx = fNpar;
   Eval(nparx, fGin, fnext, fU, 4);    ++fNfcn;
   ierev = 0;
   if (fNpar > 0) {
      fItaur = 1;
      fAmin = fnext;
      fISW[0] = 0;
      mnmigr();
      fItaur = 0;
      fnext = fAmin;
      if (fISW[0] >= 1) ierev = 1;
      if (fISW[3] < 1)  ierev = 2;
   }
} /* mneval_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnexcm(const char *command, double *plist, int32_t llist, int32_t &ierflg)
{
//*-*-*-*-*-*Interprets a command and takes appropriate action*-*-*-*-*-*-*-*
//*-*        =================================================
//*-*        either directly by skipping to the corresponding code in
//*-*        MNEXCM, or by setting up a call to a function
//*-*
//*-*  recognized MINUIT commands:
//*-*  obsolete commands:
//*-*      IERFLG is now (94.5) defined the same as ICONDN in MNCOMD
//*-*            = 0: command executed normally
//*-*              1: command is blank, ignored
//*-*              2: command line unreadable, ignored
//*-*              3: unknown command, ignored
//*-*              4: abnormal termination (e.g., MIGRAD not converged)
//*-*              9: reserved
//*-*             10: END command
//*-*             11: EXIT or STOP command
//*-*             12: RETURN command
//*-*
//*-*     see also http://wwwasdoc.web.cern.ch/wwwasdoc/minuit/node18.html for the possible list
//*-*     of all Minuit commands
//*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Initialized data */

   std::string comand = command;
   static const char *cname[40] = {
      "MINImize  ",
      "SEEk      ",
      "SIMplex   ",
      "MIGrad    ",
      "MINOs     ",
      "SET xxx   ",
      "SHOw xxx  ",
      "TOP of pag",
      "FIX       ",
      "REStore   ",
      "RELease   ",
      "SCAn      ",
      "CONtour   ",
      "HESse     ",
      "SAVe      ",
      "IMProve   ",
      "CALl fcn  ",
      "STAndard  ",
      "END       ",
      "EXIt      ",
      "RETurn    ",
      "CLEar     ",
      "HELP      ",
      "MNContour ",
      "STOp      ",
      "JUMp      ",
      "          ",
      "          ",
      "          ",
      "          ",
      "          ",
      "          ",
      "          ",
      "COVARIANCE",
      "PRINTOUT  ",
      "GRADIENT  ",
      "MATOUT    ",
      "ERROR DEF ",
      "LIMITS    ",
      "PUNCH     "};

   int32_t nntot = 40;

   /* Local variables */
   double step, xptu[101], yptu[101], f, rno;
   int32_t icol, kcol, ierr, iint, iext, lnow, nptu, i, iflag, ierrf;
   int32_t ilist, nparx, izero, nf, lk, it, iw, inonde, nsuper;
   int32_t it2, ke1, ke2, nowprt, kll, krl;
   std::string chwhy, c26, cvblnk, cneway, comd;
   std::string ctemp;
   bool lfreed, ltofix, lfixed;

//*-*  alphabetical order of command names!

   /* Function Body */

   lk = comand.length();
   if (lk > 20) lk = 20;
   fCword =  boost::to_upper_copy(comand);
//*-*-          Copy the first MAXP arguments into WORD7, making
//*-*-          sure that WORD7(1)=0 if LLIST=0
   for (iw = 1; iw <= fMaxpar; ++iw) {
      fWord7[iw-1] = 0;
      if (iw <= llist) fWord7[iw-1] = plist[iw-1];
   }
   ++fIcomnd;
   fNfcnlc = fNfcn;
   if (fCword.substr(0,7) != "SET PRI" || fWord7[0] >= 0) {
      if (fISW[4] >= 0) {
         lnow = llist;
         if (lnow > 4) lnow = 4;
         log_info(" **********");
         ctemp = boost::str(boost::format(" **%5d **%s") % fIcomnd % fCword.c_str());
         for (i = 1; i <= lnow; ++i) {
            ctemp += boost::str(boost::format("%12.4g") % plist[i-1]);
         }
         log_info("%s",ctemp.c_str());
         inonde = 0;
         if (llist > lnow) {
            kll = llist;
            if (llist > fMaxpar) {
               inonde = 1;
               kll = fMaxpar;
            }
            log_info(" ***********");
            for (i = lnow + 1; i <= kll; ++i) {
               log_info("%12.4g",plist[i-1]);
            }
         }
         log_info(" **********");
         if (inonde > 0) {
            log_info("  ERROR: ABOVE CALL TO MNEXCM TRIED TO PASS MORE THAN %d PARAMETERS.", fMaxpar);
         }
      }
   }
   fNfcnmx = int32_t(fWord7[0]);
   if (fNfcnmx <= 0) {
      fNfcnmx = fNpar*100 + 200 + fNpar*fNpar*5;
   }
   fEpsi = fWord7[1];
   if (fEpsi <= 0) {
      fEpsi = fUp*.1;
   }
   fLnewmn = false;
   fLphead = true;
   fISW[0] = 0;
   ierflg = 0;
//*-*-               look for command in list CNAME . . . . . . . . . .
   ctemp = fCword.substr(0,3);
   for (i = 1; i <= nntot; ++i) {
      if (strncmp(ctemp.c_str(),cname[i-1],3) == 0) goto L90;
   }
   log_info("UNKNOWN COMMAND IGNORED:%s", comand.c_str());
   ierflg = 3;
   return;
//*-*-               normal case: recognized MINUIT command . . . . . . .
L90:
   if (fCword.substr(0,4) == "MINO") i = 5;
   if (i != 6 && i != 7 && i != 8 && i != 23) {
      fCfrom  = cname[i-1];
      fNfcnfr = fNfcn;
   }
//*-*-             1    2    3    4    5    6    7    8    9   10
   switch (i) {
      case 1:  goto L400;
      case 2:  goto L200;
      case 3:  goto L300;
      case 4:  goto L400;
      case 5:  goto L500;
      case 6:  goto L700;
      case 7:  goto L700;
      case 8:  goto L800;
      case 9:  goto L900;
      case 10:  goto L1000;
      case 11:  goto L1100;
      case 12:  goto L1200;
      case 13:  goto L1300;
      case 14:  goto L1400;
      case 15:  goto L1500;
      case 16:  goto L1600;
      case 17:  goto L1700;
      case 18:  goto L1800;
      case 19:  goto L1900;
      case 20:  goto L1900;
      case 21:  goto L1900;
      case 22:  goto L2200;
      case 23:  goto L2300;
      case 24:  goto L2400;
      case 25:  goto L1900;
      case 26:  goto L2600;
      case 27:  goto L3300;
      case 28:  goto L3300;
      case 29:  goto L3300;
      case 30:  goto L3300;
      case 31:  goto L3300;
      case 32:  goto L3300;
      case 33:  goto L3300;
      case 34:  goto L3400;
      case 35:  goto L3500;
      case 36:  goto L3600;
      case 37:  goto L3700;
      case 38:  goto L3800;
      case 39:  goto L3900;
      case 40:  goto L4000;
   }
//*-*-                                       . . . . . . . . . . seek
L200:
   mnseek();
   return;
//*-*-                                       . . . . . . . . . . simplex
L300:
   mnsimp();
   if (fISW[3] < 1) ierflg = 4;
   return;
//*-*-                                       . . . . . . migrad, minimize
L400:
   nf = fNfcn;
   fApsi = fEpsi;
   mnmigr();
   mnwerr();
   if (fISW[3] >= 1) return;
   ierflg = 4;
   if (fISW[0] == 1) return;
   if (fCword.substr(0,3) == "MIG") return;

   fNfcnmx = fNfcnmx + nf - fNfcn;
   nf = fNfcn;
   mnsimp();
   if (fISW[0] == 1) return;
   fNfcnmx = fNfcnmx + nf - fNfcn;
   mnmigr();
   if (fISW[3] >= 1) ierflg = 0;
   mnwerr();
   return;
//*-*-                                       . . . . . . . . . . minos
L500:
   nsuper = fNfcn + ((fNpar + 1) << 1)*fNfcnmx;
//*-*-         possible loop over new minima
   fEpsi = fUp*.1;
L510:
   fCfrom  = cname[i-1]; // ensure that mncuve complains about MINOS not MIGRAD
   mncuve();
   mnmnos();
   if (! fLnewmn) return;
   mnrset(0);
   mnmigr();
   mnwerr();
   if (fNfcn < nsuper) goto L510;
   log_info(" TOO MANY FUNCTION CALLS. MINOS GIVES UP");
   ierflg = 4;
   return;
//*-*-                                       . . . . . . . . . .set, show
L700:
   mnset();
   return;
//*-*-                                       . . . . . . . . . . top of page

L800:
   log_info("1");
   return;
//*-*-                                       . . . . . . . . . . fix
L900:
   ltofix = true;
//*-*-                                       . . (also release) ....
L901:
   lfreed = false;
   lfixed = false;
   if (llist == 0) {
      log_info("%s:  NO PARAMETERS REQUESTED ",fCword.c_str());
      return;
   }
   for (ilist = 1; ilist <= llist; ++ilist) {
      iext = int32_t(plist[ilist-1]);
      chwhy = " IS UNDEFINED.";
      if (iext <= 0) goto L930;
      if (iext > fNu) goto L930;
      if (fNvarl[iext-1] < 0) goto L930;
      chwhy = " IS CONSTANT.  ";
      if (fNvarl[iext-1] == 0) goto L930;
      iint = fNiofex[iext-1];
      if (ltofix) {
         chwhy = " ALREADY FIXED.";
         if (iint == 0) goto L930;
         mnfixp(iint-1, ierr);
         if (ierr == 0) lfixed = true;
         else           ierflg = 4;
      } else {
         chwhy = " ALREADY VARIABLE.";
         if (iint > 0) goto L930;
         krl = -abs(iext);
         mnfree(krl);
         lfreed = true;
      }
      continue;
L930:
      if (fISW[4] >= 0) log_info(" PARAMETER %4d %s IGNORED.",iext,chwhy.c_str());
   }
   if (lfreed || lfixed) mnrset(0);
   if (lfreed) {
      fISW[1] = 0;
      fDcovar = 1;
      fEDM = fBigedm;
      fISW[3] = 0;
   }
   mnwerr();
   if (fISW[4] > 1) mnprin(5, fAmin);
   return;
//*-*-                                       . . . . . . . . . . restore
L1000:
   it = int32_t(fWord7[0]);
   if (it > 1 || it < 0) goto L1005;
   lfreed = fNpfix > 0;
   mnfree(it);
   if (lfreed) {
      mnrset(0);
      fISW[1] = 0;
      fDcovar = 1;
      fEDM    = fBigedm;
   }
   return;
L1005:
   log_info(" IGNORED.  UNKNOWN ARGUMENT:%4d",it);
   ierflg = 3;
   return;
//*-*-                                       . . . . . . . . . . release
L1100:
   ltofix = false;
   goto L901;
//*-*-                                      . . . . . . . . . . scan . . .
L1200:
   iext = int32_t(fWord7[0]);
   if (iext <= 0) goto L1210;
   it2 = 0;
   if (iext <= fNu) it2 = fNiofex[iext-1];
   if (it2 <= 0) goto L1250;

L1210:
   mnscan();
   return;
L1250:
   log_info(" PARAMETER %4d NOT VARIABLE.",iext);
   ierflg = 3;
   return;
//*-*-                                       . . . . . . . . . . contour
L1300:
   ke1 = int32_t(fWord7[0]);
   ke2 = int32_t(fWord7[1]);
   if (ke1 == 0) {
      if (fNpar == 2) {
         ke1 = fNexofi[0];
         ke2 = fNexofi[1];
      } else {
         log_info("%s:  NO PARAMETERS REQUESTED ",fCword.c_str());
         ierflg = 3;
         return;
      }
   }
   fNfcnmx = 1000;
   ierrf = 0;
   // mncntr(ke1-1, ke2-1, ierrf);
   log_info("CONTOURS HAVE BEEN DISABLED IN THIS VERSION.");
   if (ierrf > 0) ierflg = 3;
   return;
//*-*-                                       . . . . . . . . . . hesse
L1400:
   mnhess();
   mnwerr();
   if (fISW[4] >= 0) mnprin(2, fAmin);
   if (fISW[4] >= 1) mnmatu(1);
   return;
//*-*-                                       . . . . . . . . . . save
L1500:
   mnsave();
   return;
//*-*-                                       . . . . . . . . . . improve
L1600:
   mncuve();
   mnimpr();
   if (fLnewmn) goto L400;
   ierflg = 4;
   return;
//*-*-                                       . . . . . . . . . . call fcn
L1700:
   iflag = int32_t(fWord7[0]);
   nparx = fNpar;
   f = fUndefi;
   Eval(nparx, fGin, f, fU, iflag);    ++fNfcn;
   nowprt = 0;
   if (f != fUndefi) {
      if (fAmin == fUndefi) {
         fAmin  = f;
         nowprt = 1;
      } else if (f < fAmin) {
         fAmin  = f;
         nowprt = 1;
      }
      if (fISW[4] >= 0 && iflag <= 5 && nowprt == 1) {
         mnprin(5, fAmin);
      }
      if (iflag == 3)  fFval3 = f;
   }
   if (iflag > 5) mnrset(1);
   return;
//*-*-                                       . . . . . . . . . . standard
L1800:
//    stand();
   return;
//*-*-                                      . . . return, stop, end, exit
L1900:
   it = int32_t(fWord7[0]);
   if (fFval3 != fAmin && it == 0) {
      iflag = 3;
      if (fISW[4] >= 0) log_info(" CALL TO USER FUNCTION WITH IFLAG = 3");
      nparx = fNpar;
      Eval(nparx, fGin, f, fU, iflag);        ++fNfcn;
   }
   ierflg = 11;
   if (fCword.substr(0,3) == "END") ierflg = 10;
   if (fCword.substr(0,3) == "RET") ierflg = 12;
   return;
//*-*-                                       . . . . . . . . . . clear
L2200:
   mncler();
   if (fISW[4] >= 1) {
      log_info(" MINUIT MEMORY CLEARED. NO PARAMETERS NOW DEFINED.");
   }
   return;
//*-*-                                       . . . . . . . . . . help
L2300:
   kcol = 0;
   for (icol = 5; icol <= lk; ++icol) {
      if (fCword[icol-1] == ' ') continue;
      kcol = icol;
      goto L2320;
   }
L2320:
   if (kcol == 0) comd = "*   ";
   else           comd = fCword.substr(kcol-1,lk-kcol+1);
   mnhelp(comd);
   return;
//*-*-                                      . . . . . . . . . . MNContour
L2400:
   fEpsi = fUp*.05;
   ke1 = int32_t(fWord7[0]);
   ke2 = int32_t(fWord7[1]);
   if (ke1 == 0 && fNpar == 2) {
      ke1 = fNexofi[0];
      ke2 = fNexofi[1];
   }
   nptu = int32_t(fWord7[2]);
   if (nptu <= 0)  nptu = 20;
   if (nptu > 101) nptu = 101;
   fNfcnmx = (nptu + 5)*100*(fNpar + 1);
   mncont(ke1-1, ke2-1, nptu, xptu, yptu, ierrf);
   if (ierrf < nptu) ierflg = 4;
   if (ierrf == -1)  ierflg = 3;
   return;
//*-*-                                     . . . . . . . . . . jump
L2600:
   step = fWord7[0];
   if (step <= 0) step = 2;
   rno = 0;
   izero = 0;
   for (i = 1; i <= fNpar; ++i) {
      mnrn15(rno, izero);
      rno      = rno*2 - 1;
      fX[i-1] += rno*step*fWerr[i-1];
   }
   mninex(fX);
   mnamin();
   mnrset(0);
   return;
//*-*-                                     . . . . . . . . . . blank line
L3300:
   log_info(" BLANK COMMAND IGNORED.");
   ierflg = 1;
   return;
//*-*  . . . . . . . . obsolete commands     . . . . . . . . . . . . . .
//*-*-                                     . . . . . . . . . . covariance
L3400:
   log_info(" THE *COVARIANCE* COMMAND IS OSBSOLETE. THE COVARIANCE MATRIX IS NOW SAVED IN A DIFFERENT FORMAT WITH THE *SAVE* COMMAND AND READ IN WITH:*SET COVARIANCE*");
   ierflg = 3;
   return;
//*-*-                                       . . . . . . . . . . printout
L3500:
   cneway = "SET PRInt ";
   goto L3100;
//*-*-                                       . . . . . . . . . . gradient
L3600:
   cneway = "SET GRAd  ";
   goto L3100;
//*-*-                                       . . . . . . . . . . matout
L3700:
   cneway = "SHOW COVar";
   goto L3100;
//*-*-                                       . . . . . . . . . error def
L3800:
   cneway = "SET ERRdef";
   goto L3100;
//*-*-                                       . . . . . . . . . . limits
L3900:
   cneway = "SET LIMits";
   goto L3100;
//*-*-                                       . . . . . . . . . . punch
L4000:
   cneway = "SAVE      ";
//*-*-                               ....... come from obsolete commands
L3100:
   log_info(" OBSOLETE COMMAND:%s   PLEASE USE:%s",fCword.c_str()
                                                 ,cneway.c_str());
   fCword = cneway;
   if (fCword == "SAVE      ") goto L1500;
   goto L700;
//*-*                                 . . . . . . . . . . . . . . . . . .
} /* mnexcm_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnexin(double *pint)
{
//*-*-*-*-*Transforms the external parameter values U to internal values*-*-*
//*-*      =============================================================
//*-*        Transforms the external parameter values U to internal
//*-*        values in the dense array PINT.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   double pinti;
   int32_t iint, iext;

   fLimset = false;
   for (iint = 1; iint <= fNpar; ++iint) {
      iext = fNexofi[iint-1];
      mnpint(fU[iext-1], iext-1, pinti);
      pint[iint-1] = pinti;
   }
} /* mnexin_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnfixp(int32_t iint1, int32_t &ierr)
{
//*-*-*-*-*-*-*Removes parameter IINT from the internal parameter list*-*-*
//*-*          =======================================================
//*-*        and arranges the rest of the list to fill the hole.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double yyover;
   int32_t kold, nold, ndex, knew, iext, i, j, m, n, lc, ik;

//*-*-                          first see if it can be done
   ierr = 0;
   int32_t iint = iint1+1;
   if (iint > fNpar || iint <= 0) {
      ierr = 1;
      log_info(" MINUIT ERROR.  ARGUMENT TO MNFIXP=%4d",iint);
      return;
   }
   iext = fNexofi[iint-1];
   if (fNpfix >= fMaxpar) {
      ierr = 1;
      log_info(" MINUIT CANNOT FIX PARAMETER %4d MAXIMUM NUMBER THAT CAN BE FIXED IS %d",iext,fMaxpar);
      return;
   }
//*-*-                          reduce number of variable parameters by one

   fNiofex[iext-1] = 0;
   nold = fNpar;
   --fNpar;
//*-*-                      save values in case parameter is later restored

   ++fNpfix;
   fIpfix[fNpfix-1]  = iext;
   lc                = iint;
   fXs[fNpfix-1]     = fX[lc-1];
   fXts[fNpfix-1]    = fXt[lc-1];
   fDirins[fNpfix-1] = fWerr[lc-1];
   fGrds[fNpfix-1]   = fGrd[lc-1];
   fG2s[fNpfix-1]    = fG2[lc-1];
   fGsteps[fNpfix-1] = fGstep[lc-1];
//*-*-                       shift values for other parameters to fill hole
   for (ik = iext + 1; ik <= fNu; ++ik) {
      if (fNiofex[ik-1] > 0) {
         lc = fNiofex[ik-1] - 1;
         fNiofex[ik-1] = lc;
         fNexofi[lc-1] = ik;
         fX[lc-1]      = fX[lc];
         fXt[lc-1]     = fXt[lc];
         fDirin[lc-1]  = fDirin[lc];
         fWerr[lc-1]   = fWerr[lc];
         fGrd[lc-1]    = fGrd[lc];
         fG2[lc-1]     = fG2[lc];
         fGstep[lc-1]  = fGstep[lc];
      }
   }
   if (fISW[1] <= 0) return;
//*-*-                   remove one row and one column from variance matrix
   if (fNpar <= 0)   return;
   for (i = 1; i <= nold; ++i) {
      m       = std::max(i,iint);
      n       = std::min(i,iint);
      ndex    = m*(m-1) / 2 + n;
      fFIXPyy[i-1] = fVhmat[ndex-1];
   }
   yyover = 1 / fFIXPyy[iint-1];
   knew   = 0;
   kold   = 0;
   for (i = 1; i <= nold; ++i) {
      for (j = 1; j <= i; ++j) {
         ++kold;
         if (j == iint || i == iint) continue;
         ++knew;
         fVhmat[knew-1] = fVhmat[kold-1] - fFIXPyy[j-1]*fFIXPyy[i-1]*yyover;
      }
   }
} /* mnfixp_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnfree(int32_t k)
{
//*-*-*-*Restores one or more fixed parameter(s) to variable status*-*-*-*-*-*
//*-*    ==========================================================
//*-*        Restores one or more fixed parameter(s) to variable status
//*-*        by inserting it into the internal parameter list at the
//*-*        appropriate place.
//*-*
//*-*        K = 0 means restore all parameters
//*-*        K = 1 means restore the last parameter fixed
//*-*        K = -I means restore external parameter I (if possible)
//*-*        IQ = fix-location where internal parameters were stored
//*-*        IR = external number of parameter being restored
//*-*        IS = internal number of parameter being restored
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double grdv, xv, dirinv, g2v, gstepv, xtv;
   int32_t i, ipsav, ka, lc, ik, iq, ir, is;

   if (k > 1) {
      log_info(" CALL TO MNFREE IGNORED.  ARGUMENT GREATER THAN ONE");
   }
   if (fNpfix < 1) {
      log_info(" CALL TO MNFREE IGNORED.  THERE ARE NO FIXED PARAMETERS");
   }
   if (k == 1 || k == 0) goto L40;

//*-*-                  release parameter with specified external number
   ka = abs(k);
   if (fNiofex[ka-1] == 0) goto L15;
   log_info(" IGNORED.  PARAMETER SPECIFIED IS ALREADY VARIABLE.");
   return;
L15:
   if (fNpfix < 1) goto L21;
   for (ik = 1; ik <= fNpfix; ++ik) { if (fIpfix[ik-1] == ka) goto L24; }
L21:
   log_info(" PARAMETER %4d NOT FIXED.  CANNOT BE RELEASED.",ka);
   return;
L24:
   if (ik == fNpfix) goto L40;

//*-*-                  move specified parameter to end of list
   ipsav  = ka;
   xv     = fXs[ik-1];
   xtv    = fXts[ik-1];
   dirinv = fDirins[ik-1];
   grdv   = fGrds[ik-1];
   g2v    = fG2s[ik-1];
   gstepv = fGsteps[ik-1];
   for (i = ik + 1; i <= fNpfix; ++i) {
      fIpfix[i-2]  = fIpfix[i-1];
      fXs[i-2]     = fXs[i-1];
      fXts[i-2]    = fXts[i-1];
      fDirins[i-2] = fDirins[i-1];
      fGrds[i-2]   = fGrds[i-1];
      fG2s[i-2]    = fG2s[i-1];
      fGsteps[i-2] = fGsteps[i-1];
   }
   fIpfix[fNpfix-1]  = ipsav;
   fXs[fNpfix-1]     = xv;
   fXts[fNpfix-1]    = xtv;
   fDirins[fNpfix-1] = dirinv;
   fGrds[fNpfix-1]   = grdv;
   fG2s[fNpfix-1]    = g2v;
   fGsteps[fNpfix-1] = gstepv;
//*-*-               restore last parameter in fixed list  -- IPFIX(NPFIX)
L40:
   if (fNpfix < 1) goto L300;
   ir = fIpfix[fNpfix-1];
   is = 0;
   for (ik = fNu; ik >= ir; --ik) {
      if (fNiofex[ik-1] > 0) {
         lc = fNiofex[ik-1] + 1;
         is = lc - 1;
         fNiofex[ik-1] = lc;
         fNexofi[lc-1] = ik;
         fX[lc-1]      = fX[lc-2];
         fXt[lc-1]     = fXt[lc-2];
         fDirin[lc-1]  = fDirin[lc-2];
         fWerr[lc-1]   = fWerr[lc-2];
         fGrd[lc-1]    = fGrd[lc-2];
         fG2[lc-1]     = fG2[lc-2];
         fGstep[lc-1]  = fGstep[lc-2];
      }
   }
   ++fNpar;
   if (is == 0) is = fNpar;
   fNiofex[ir-1] = is;
   fNexofi[is-1] = ir;
   iq           = fNpfix;
   fX[is-1]     = fXs[iq-1];
   fXt[is-1]    = fXts[iq-1];
   fDirin[is-1] = fDirins[iq-1];
   fWerr[is-1]  = fDirins[iq-1];
   fGrd[is-1]   = fGrds[iq-1];
   fG2[is-1]    = fG2s[iq-1];
   fGstep[is-1] = fGsteps[iq-1];
   --fNpfix;
   fISW[1] = 0;
   fDcovar = 1;
   if (fISW[4] - fItaur >= 1) {
      log_info("                   PARAMETER %4d  %s RESTORED TO VARIABLE.",ir,
                      fCpnam[ir-1].c_str());
   }
   if (k == 0) goto L40;
L300:
//*-*-        if different from internal, external values are taken
   mnexin(fX);
} /* mnfree_ */

//______________________________________________________________________________
void lilliput::TMinuit::mngrad()
{
//*-*-*-*-*-*-*-*-*-*Interprets the SET GRAD command*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                ===============================
//*-*       Called from MNSET
//*-*       Interprets the SET GRAD command, which informs MINUIT whether
//*-*       the first derivatives of FCN will be calculated by the user
//*-*       inside FCN.  It can check the user derivative calculation
//*-*       by comparing it with a finite difference approximation.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double fzero, err;
   int32_t i, nparx, lc, istsav;
   bool lnone;
   static std::string cwd = "    ";

   fISW[2] = 1;
   nparx   = fNpar;
   if (fWord7[0] > 0) goto L2000;

//*-*-                 get user-calculated first derivatives from FCN
   for (i = 1; i <= fNu; ++i) { fGin[i-1] = fUndefi; }
   mninex(fX);
   Eval(nparx, fGin, fzero, fU, 2);    ++fNfcn;
   mnderi();
   for (i = 1; i <= fNpar; ++i) { fGRADgf[i-1] = fGrd[i-1]; }
//*-*-                   get MINUIT-calculated first derivatives
   fISW[2] = 0;
   istsav  = fIstrat;
   fIstrat = 2;
   mnhes1();
   fIstrat = istsav;
   log_info(" CHECK OF GRADIENT CALCULATION IN FCN");
   log_info("            PARAMETER      G(IN FCN)   G(MINUIT)  DG(MINUIT)   AGREEMENT");
   fISW[2] = 1;
   lnone = false;
   for (lc = 1; lc <= fNpar; ++lc) {
      i   = fNexofi[lc-1];
      cwd = "GOOD";
      err = fDgrd[lc-1];
      if (std::abs(fGRADgf[lc-1] - fGrd[lc-1]) > err)  cwd = " BAD";
      if (fGin[i-1] == fUndefi) {
         cwd      = "NONE";
         lnone    = true;
         fGRADgf[lc-1] = 0;
      }
      if (cwd != "GOOD") fISW[2] = 0;
      log_info("       %5d  %10s%12.4e%12.4e%12.4e    %s",i
                    ,fCpnam[i-1].c_str()
                    ,fGRADgf[lc-1],fGrd[lc-1],err,cwd.c_str());
   }
   if (lnone) {
      log_info("  AGREEMENT=NONE  MEANS FCN DID NOT CALCULATE THE DERIVATIVE");
   }
   if (fISW[2] == 0) {
      log_info(" MINUIT DOES NOT ACCEPT DERIVATIVE CALCULATIONS BY FCN");
      log_info(" TO FORCE ACCEPTANCE, ENTER *SET GRAD    1*");
   }

L2000:
   return;
} /* mngrad_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnhelp(const char *command)
{
   //interface to Minuit help
   std::string comd = command;
   mnhelp(comd);
}

//______________________________________________________________________________
void lilliput::TMinuit::mnhelp(std::string comd)
{
//*-*-*-*-*-*-*-*HELP routine for MINUIT interactive commands*-*-*-*-*-*-*-*-*
//*-*            ============================================
//*-*
//*-*      COMD ='*' or "" prints a global help for all commands
//*-*      COMD =Command_name: print detailed help for one command.
//*-*         Note that at least 3 characters must be given for the command
//*-*         name.
//*-*
//*-*     Author: Rene Brun
//*-*             comments extracted from the MINUIT documentation file.
//*-*
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//*-*.......................................................................
//*-*
//*-*  Global HELP: Summary of all commands
//*-*  ====================================
//*-*
  boost::to_upper(comd);
   if( comd.length() == 0 || comd[0] == '*' || comd[0] == '?' || comd[0] == 0 || comd=="HELP" ) {
      log_info("   ==>List of MINUIT Interactive commands:");
      log_info(" CLEar     Reset all parameter names and values undefined");
      log_info(" CONtour   Make contour map of the user function");
      log_info(" EXIT      Exit from Interactive Minuit");
      log_info(" FIX       Cause parameter(s) to remain constant");
      log_info(" HESse     Calculate the Hessian or error matrix.");
      log_info(" IMPROVE   Search for a new minimum around current minimum");
      log_info(" MIGrad    Minimize by the method of Migrad");
      log_info(" MINImize  MIGRAD + SIMPLEX method if Migrad fails");
      log_info(" MINOs     Exact (non-linear) parameter error analysis");
      log_info(" MNContour Calculate one MINOS function contour");
      log_info(" PARameter Define or redefine new parameters and values");
      log_info(" RELease   Make previously FIXed parameters variable again");
      log_info(" REStore   Release last parameter fixed");
      log_info(" SAVe      Save current parameter values on a file");
      log_info(" SCAn      Scan the user function by varying parameters");
      log_info(" SEEk      Minimize by the method of Monte Carlo");
      log_info(" SET       Set various MINUIT constants or conditions");
      log_info(" SHOw      Show values of current constants or conditions");
      log_info(" SIMplex   Minimize by the method of Simplex");
      goto L99;
   }

//*-* __________________________________________________________________
//*-*
//*-* --  Command CLEAR
//*-* --  =============
//*-*
   if( !strncmp(comd.c_str(),"CLE",3) ) {
      log_info(" ***>CLEAR");
      log_info(" Resets all parameter names and values to undefined.");
      log_info(" Must normally be followed by a PARameters command or ");
      log_info(" equivalent, in order to define parameter values.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command CONTOUR
//*-* --  ===============
//*-* .
   if( !strncmp(comd.c_str(),"CON",3) ) {
      log_info(" ***>CONTOUR <par1>  <par2>  [devs]  [ngrid]");
      log_info(" Instructs Minuit to trace contour lines of the user function");
      log_info(" with respect to the two parameters whose external numbers");
      log_info(" are <par1> and <par2>.");
      log_info(" Other variable parameters of the function, if any, will have");
      log_info(" their values fixed at the current values during the contour");
      log_info(" tracing. The optional parameter [devs] (default value 2.)");
      log_info(" gives the number of standard deviations in each parameter");
      log_info(" which should lie entirely within the plotting area.");
      log_info(" Optional parameter [ngrid] (default value 25 unless page");
      log_info(" size is too small) determines the resolution of the plot,");
      log_info(" i.e. the number of rows and columns of the grid at which the");
      log_info(" function will be evaluated. [See also MNContour.]");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command END
//*-* --  ===========
//*-* .
   if( !strncmp(comd.c_str(),"END",3) ) {
      log_info(" ***>END");
      log_info(" Signals the end of a data block (i.e., the end of a fit),");
      log_info(" and implies that execution should continue, because another");
      log_info(" Data Block follows. A Data Block is a set of Minuit data");
      log_info(" consisting of");
      log_info("     (1) A Title,");
      log_info("     (2) One or more Parameter Definitions,");
      log_info("     (3) A blank line, and");
      log_info("     (4) A set of Minuit Commands.");
      log_info(" The END command is used when more than one Data Block is to");
      log_info(" be used with the same FCN function. It first causes Minuit");
      log_info(" to issue a CALL FCN with IFLAG=3, in order to allow FCN to");
      log_info(" perform any calculations associated with the final fitted");
      log_info(" parameter values, unless a CALL FCN 3 command has already");
      log_info(" been executed at the current FCN value.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* .
//*-* --
//*-* --  Command EXIT
//*-* --  ============
   if( !strncmp(comd.c_str(),"EXI",3) ) {
      log_info(" ***>EXIT");
      log_info(" Signals the end of execution.");
      log_info(" The EXIT command first causes Minuit to issue a CALL FCN");
      log_info(" with IFLAG=3, to allow FCN to perform any calculations");
      log_info(" associated with the final fitted parameter values, unless a");
      log_info(" CALL FCN 3 command has already been executed.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command FIX
//*-* --  ===========
//*-* .
   if( !strncmp(comd.c_str(),"FIX",3) ) {
      log_info(" ***>FIX} <parno> [parno] ... [parno]");
      log_info(" Causes parameter(s) <parno> to be removed from the list of");
      log_info(" variable parameters, and their value(s) will remain constant");
      log_info(" during subsequent minimizations, etc., until another command");
      log_info(" changes their value(s) or status.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command HESSE
//*-* --  =============
//*-* .
   if( !strncmp(comd.c_str(),"HES",3) ) {
      log_info(" ***>HESse  [maxcalls]");
      log_info(" Calculate, by finite differences, the Hessian or error matrix.");
      log_info("  That is, it calculates the full matrix of second derivatives");
      log_info(" of the function with respect to the currently variable");
      log_info(" parameters, and inverts it, printing out the resulting error");
      log_info(" matrix. The optional argument [maxcalls] specifies the");
      log_info(" (approximate) maximum number of function calls after which");
      log_info(" the calculation will be stopped.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command IMPROVE
//*-* --  ===============
//*-* .
   if( !strncmp(comd.c_str(),"IMP",3) ) {
      log_info(" ***>IMPROVE  [maxcalls]");
      log_info(" If a previous minimization has converged, and the current");
      log_info(" values of the parameters therefore correspond to a local");
      log_info(" minimum of the function, this command requests a search for");
      log_info(" additional distinct local minima.");
      log_info(" The optional argument [maxcalls] specifies the (approximate");
      log_info(" maximum number of function calls after which the calculation");
      log_info(" will be stopped.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command MIGRAD
//*-* --  ==============
//*-* .
   if( !strncmp(comd.c_str(),"MIG",3) ) {
      log_info(" ***>MIGrad  [maxcalls]  [tolerance]");
      log_info(" Causes minimization of the function by the method of Migrad,");
      log_info(" the most efficient and complete single method, recommended");
      log_info(" for general functions (see also MINImize).");
      log_info(" The minimization produces as a by-product the error matrix");
      log_info(" of the parameters, which is usually reliable unless warning");
      log_info(" messages are produced.");
      log_info(" The optional argument [maxcalls] specifies the (approximate)");
      log_info(" maximum number of function calls after which the calculation");
      log_info(" will be stopped even if it has not yet converged.");
      log_info(" The optional argument [tolerance] specifies required tolerance");
      log_info(" on the function value at the minimum.");
      log_info(" The default tolerance is 0.1, and the minimization will stop");
      log_info(" when the estimated vertical distance to the minimum (EDM) is");
      log_info(" less than 0.001*[tolerance]*UP (see [SET ERRordef]).");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command MINIMIZE
//*-* --  ================
//*-* .
   if( !strncmp(comd.c_str(),"MINI",4) ) {
      log_info(" ***>MINImize  [maxcalls] [tolerance]");
      log_info(" Causes minimization of the function by the method of Migrad,");
      log_info(" as does the MIGrad command, but switches to the SIMplex method");
      log_info(" if Migrad fails to converge. Arguments are as for MIGrad.");
      log_info(" Note that command requires four characters to be unambiguous.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command MINOS
//*-* --  =============
//*-* .
   if( !strncmp(comd.c_str(),"MIN0",4) ) {
      log_info(" ***>MINOs  [maxcalls]  [parno] [parno] ...");
      log_info(" Causes a Minos error analysis to be performed on the parameters");
      log_info(" whose numbers [parno] are specified. If none are specified,");
      log_info(" Minos errors are calculated for all variable parameters.");
      log_info(" Minos errors may be expensive to calculate, but are very");
      log_info(" reliable since they take account of non-linearities in the");
      log_info(" problem as well as parameter correlations, and are in general");
      log_info(" asymmetric.");
      log_info(" The optional argument [maxcalls] specifies the (approximate)");
      log_info(" maximum number of function calls per parameter requested,");
      log_info(" after which the calculation will stop for that parameter.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command MNCONTOUR
//*-* --  =================
//*-* .
   if( !strncmp(comd.c_str(),"MNC",3) ) {
      log_info(" ***>MNContour  <par1> <par2> [npts]");
      log_info(" Calculates one function contour of FCN with respect to");
      log_info(" parameters par1 and par2, with FCN minimized always with");
      log_info(" respect to all other NPAR-2 variable parameters (if any).");
      log_info(" Minuit will try to find npts points on the contour (default 20)");
      log_info(" If only two parameters are variable at the time, it is not");
      log_info(" necessary to specify their numbers. To calculate more than");
      log_info(" one contour, it is necessary to SET ERRordef to the appropriate");
      log_info(" value and issue the MNContour command for each contour.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command PARAMETER
//*-* --  =================
//*-* .
   if( !strncmp(comd.c_str(),"PAR",3) ) {
      log_info(" ***>PARameters");
      log_info(" followed by one or more parameter definitions.");
      log_info(" Parameter definitions are of the form:");
      log_info("   <number>  ''name''  <value>  <step>  [lolim] [uplim] ");
      log_info(" for example:");
      log_info("  3  ''K width''  1.2   0.1");
      log_info(" the last definition is followed by a blank line or a zero.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command RELEASE
//*-* --  ===============
//*-* .
   if( !strncmp(comd.c_str(),"REL",3) ) {
      log_info(" ***>RELease  <parno> [parno] ... [parno]");
      log_info(" If <parno> is the number of a previously variable parameter");
      log_info(" which has been fixed by a command: FIX <parno>, then that");
      log_info(" parameter will return to variable status.  Otherwise a warning");
      log_info(" message is printed and the command is ignored.");
      log_info(" Note that this command operates only on parameters which were");
      log_info(" at one time variable and have been FIXed. It cannot make");
      log_info(" constant parameters variable; that must be done by redefining");
      log_info(" the parameter with a PARameters command.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command RESTORE
//*-* --  ===============
//*-* .
   if( !strncmp(comd.c_str(),"RES",3) ) {
      log_info(" ***>REStore  [code]");
      log_info(" If no [code] is specified, this command restores all previously");
      log_info(" FIXed parameters to variable status. If [code]=1, then only");
      log_info(" the last parameter FIXed is restored to variable status.");
      log_info(" If code is neither zero nor one, the command is ignored.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command RETURN
//*-* --  ==============
//*-* .
   if( !strncmp(comd.c_str(),"RET",3) ) {
      log_info(" ***>RETURN");
      log_info(" Signals the end of a data block, and instructs Minuit to return");
      log_info(" to the program which called it. The RETurn command first");
      log_info(" causes Minuit to CALL FCN with IFLAG=3, in order to allow FCN");
      log_info(" to perform any calculations associated with the final fitted");
      log_info(" parameter values, unless a CALL FCN 3 command has already been");
      log_info(" executed at the current FCN value.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command SAVE
//*-* --  ============
//*-* .
   if( !strncmp(comd.c_str(),"SAV",3) ) {
      log_info(" ***>SAVe");
      log_info(" Causes the current parameter values to be saved on a file in");
      log_info(" such a format that they can be read in again as Minuit");
      log_info(" parameter definitions. If the covariance matrix exists, it is");
      log_info(" also output in such a format. The unit number is by default 7,");
      log_info(" or that specified by the user in his call to MINTIO or");
      log_info(" MNINIT. The user is responsible for opening the file previous");
      log_info(" to issuing the [SAVe] command (except where this can be done");
      log_info(" interactively).");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command SCAN
//*-* --  ============
//*-* .
   if( !strncmp(comd.c_str(),"SCA",3) ) {
      log_info(" ***>SCAn  [parno]  [numpts] [from]  [to]");
      log_info(" Scans the value of the user function by varying parameter");
      log_info(" number [parno], leaving all other parameters fixed at the");
      log_info(" current value. If [parno] is not specified, all variable");
      log_info(" parameters are scanned in sequence.");
      log_info(" The number of points [numpts] in the scan is 40 by default,");
      log_info(" and cannot exceed 100. The range of the scan is by default");
      log_info(" 2 standard deviations on each side of the current best value,");
      log_info(" but can be specified as from [from] to [to].");
      log_info(" After each scan, if a new minimum is found, the best parameter");
      log_info(" values are retained as start values for future scans or");
      log_info(" minimizations. The curve resulting from each scan is plotted");
      log_info(" on the output unit in order to show the approximate behaviour");
      log_info(" of the function.");
      log_info(" This command is not intended for minimization, but is sometimes");
      log_info(" useful for debugging the user function or finding a");
      log_info(" reasonable starting point.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command SEEK
//*-* --  ============
//*-* .
   if( !strncmp(comd.c_str(),"SEE",3) ) {
      log_info(" ***>SEEk  [maxcalls]  [devs]");
      log_info(" Causes a Monte Carlo minimization of the function, by choosing");
      log_info(" random values of the variable parameters, chosen uniformly");
      log_info(" over a hypercube centered at the current best value.");
      log_info(" The region size is by default 3 standard deviations on each");
      log_info(" side, but can be changed by specifying the value of [devs].");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command SET
//*-* --  ===========
//*-* .
   if( !strncmp(comd.c_str(),"SET",3) ) {
      log_info(" ***>SET <option_name>");
      log_info("  SET BATch");
      log_info("    Informs Minuit that it is running in batch mode.");

      log_info(" ");
      log_info("  SET EPSmachine  <accuracy>");
      log_info("    Informs Minuit that the relative floating point arithmetic");
      log_info("    precision is <accuracy>. Minuit determines the nominal");
      log_info("    precision itself, but the SET EPSmachine command can be");
      log_info("    used to override Minuit own determination, when the user");
      log_info("    knows that the FCN function value is not calculated to");
      log_info("    the nominal machine accuracy. Typical values of <accuracy>");
      log_info("    are between 10**-5 and 10**-14.");

      log_info(" ");
      log_info("  SET ERRordef  <up>");
      log_info("    Sets the value of UP (default value= 1.), defining parameter");
      log_info("    errors. Minuit defines parameter errors as the change");
      log_info("    in parameter value required to change the function value");
      log_info("    by UP. Normally, for chisquared fits UP=1, and for negative");
      log_info("    log likelihood, UP=0.5.");

      log_info(" ");
      log_info("   SET GRAdient  [force]");
      log_info("    Informs Minuit that the user function is prepared to");
      log_info("    calculate its own first derivatives and return their values");
      log_info("    in the array GRAD when IFLAG=2 (see specs of FCN).");
      log_info("    If [force] is not specified, Minuit will calculate");
      log_info("    the FCN derivatives by finite differences at the current");
      log_info("    point and compare with the user calculation at that point,");
      log_info("    accepting the user values only if they agree.");
      log_info("    If [force]=1, Minuit does not do its own derivative");
      log_info("    calculation, and uses the derivatives calculated in FCN.");

      log_info(" ");
      log_info("   SET INPut  [unitno]  [filename]");
      log_info("    Causes Minuit, in data-driven mode only, to read subsequent");
      log_info("    commands (or parameter definitions) from a different input");
      log_info("    file. If no [unitno] is specified, reading reverts to the");
      log_info("    previous input file, assuming that there was one.");
      log_info("    If [unitno] is specified, and that unit has not been opened,");
      log_info("    then Minuit attempts to open the file [filename]} if a");
      log_info("    name is specified. If running in interactive mode and");
      log_info("    [filename] is not specified and [unitno] is not opened,");
      log_info("    Minuit prompts the user to enter a file name.");
      log_info("    If the word REWIND is added to the command (note:no blanks");
      log_info("    between INPUT and REWIND), the file is rewound before");
      log_info("    reading. Note that this command is implemented in standard");
      log_info("    Fortran 77 and the results may depend on the  system;");
      log_info("    for example, if a filename is given under VM/CMS, it must");
      log_info("    be preceded by a slash.");

      log_info(" ");
      log_info("   SET INTeractive");
      log_info("    Informs Minuit that it is running interactively.");

      log_info(" ");
      log_info("   SET LIMits  [parno]  [lolim]  [uplim]");
      log_info("    Allows the user to change the limits on one or all");
      log_info("    parameters. If no arguments are specified, all limits are");
      log_info("    removed from all parameters. If [parno] alone is specified,");
      log_info("    limits are removed from parameter [parno].");
      log_info("    If all arguments are specified, then parameter [parno] will");
      log_info("    be bounded between [lolim] and [uplim].");
      log_info("    Limits can be specified in either order, Minuit will take");
      log_info("    the smaller as [lolim] and the larger as [uplim].");
      log_info("    However, if [lolim] is equal to [uplim], an error condition");
      log_info("    results.");

      log_info(" ");
      log_info("   SET LINesperpage");
      log_info("     Sets the number of lines for one page of output.");
      log_info("     Default value is 24 for interactive mode");

      log_info(" ");
      log_info("   SET NOGradient");
      log_info("    The inverse of SET GRAdient, instructs Minuit not to");
      log_info("    use the first derivatives calculated by the user in FCN.");

      log_info(" ");
      log_info("   SET NOWarnings");
      log_info("    Supresses Minuit warning messages.");

      log_info(" ");
      log_info("   SET OUTputfile  <unitno>");
      log_info("    Instructs Minuit to write further output to unit <unitno>.");

      log_info(" ");
      log_info("   SET PAGethrow  <integer>");
      log_info("    Sets the carriage control character for ``new page'' to");
      log_info("    <integer>. Thus the value 1 produces a new page, and 0");
      log_info("    produces a blank line, on some devices (see TOPofpage)");


      log_info(" ");
      log_info("   SET PARameter  <parno>  <value>");
      log_info("    Sets the value of parameter <parno> to <value>.");
      log_info("    The parameter in question may be variable, fixed, or");
      log_info("    constant, but must be defined.");

      log_info(" ");
      log_info("   SET PRIntout  <level>");
      log_info("    Sets the print level, determining how much output will be");
      log_info("    produced. Allowed values and their meanings are displayed");
      log_info("    after a SHOw PRInt command, and are currently <level>=:");
      log_info("      [-1]  no output except from SHOW commands");
      log_info("       [0]  minimum output");
      log_info("       [1]  default value, normal output");
      log_info("       [2]  additional output giving intermediate results.");
      log_info("       [3]  maximum output, showing progress of minimizations.");
      log_info("    Note: See also the SET WARnings command.");

      log_info(" ");
      log_info("   SET RANdomgenerator  <seed>");
      log_info("    Sets the seed of the random number generator used in SEEk.");
      log_info("    This can be any integer between 10000 and 900000000, for");
      log_info("    example one which was output from a SHOw RANdom command of");
      log_info("    a previous run.");

      log_info(" ");
      log_info("   SET STRategy  <level>");
      log_info("    Sets the strategy to be used in calculating first and second");
      log_info("    derivatives and in certain minimization methods.");
      log_info("    In general, low values of <level> mean fewer function calls");
      log_info("    and high values mean more reliable minimization.");
      log_info("    Currently allowed values are 0, 1 (default), and 2.");

      log_info(" ");
      log_info("   SET TITle");
      log_info("    Informs Minuit that the next input line is to be considered");
      log_info("    the (new) title for this task or sub-task.  This is for");
      log_info("    the convenience of the user in reading his output.");

      log_info(" ");
      log_info("   SET WARnings");
      log_info("    Instructs Minuit to output warning messages when suspicious");
      log_info("    conditions arise which may indicate unreliable results.");
      log_info("    This is the default.");

      log_info(" ");
      log_info("    SET WIDthpage");
      log_info("    Informs Minuit of the output page width.");
      log_info("    Default values are 80 for interactive jobs");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command SHOW
//*-* --  ============
//*-* .
   if( !strncmp(comd.c_str(),"SHO",3) ) {
      log_info(" ***>SHOw  <option_name>");
      log_info("  All SET XXXX commands have a corresponding SHOw XXXX command.");
      log_info("  In addition, the SHOw commands listed starting here have no");
      log_info("  corresponding SET command for obvious reasons.");

      log_info(" ");
      log_info("   SHOw CORrelations");
      log_info("    Calculates and prints the parameter correlations from the");
      log_info("    error matrix.");

      log_info(" ");
      log_info("   SHOw COVariance");
      log_info("    Prints the (external) covariance (error) matrix.");

      log_info(" ");
      log_info("   SHOw EIGenvalues");
      log_info("    Calculates and prints the eigenvalues of the covariance");
      log_info("    matrix.");

      log_info(" ");
      log_info("   SHOw FCNvalue");
      log_info("    Prints the current value of FCN.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command SIMPLEX
//*-* --  ===============
//*-* .
   if( !strncmp(comd.c_str(),"SIM",3) ) {
      log_info(" ***>SIMplex  [maxcalls]  [tolerance]");
      log_info(" Performs a function minimization using the simplex method of");
      log_info(" Nelder and Mead. Minimization terminates either when the");
      log_info(" function has been called (approximately) [maxcalls] times,");
      log_info(" or when the estimated vertical distance to minimum (EDM) is");
      log_info(" less than [tolerance].");
      log_info(" The default value of [tolerance] is 0.1*UP(see SET ERRordef).");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command STANDARD
//*-* --  ================
//*-* .
   if( !strncmp(comd.c_str(),"STA",3) ) {
      log_info(" ***>STAndard");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command STOP
//*-* --  ============
//*-* .
   if( !strncmp(comd.c_str(),"STO",3) ) {
      log_info(" ***>STOP");
      log_info(" Same as EXIT.");
      goto L99;
   }
//*-* __________________________________________________________________
//*-* --
//*-* --  Command TOPOFPAGE
//*-* --  =================
//*-* .
   if( !strncmp(comd.c_str(),"TOP",3) ) {
      log_info(" ***>TOPofpage");
      log_info(" Causes Minuit to write the character specified in a");
      log_info(" SET PAGethrow command (default = 1) to column 1 of the output");
      log_info(" file, which may or may not position your output medium to");
      log_info(" the top of a page depending on the device and system.");
      goto L99;
   }
//*-* __________________________________________________________________
   log_info(" Unknown MINUIT command. Type HELP for list of commands.");

L99:
   return;
} /* mnhelp_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnhess()
{
//*-*-*-*-*-*Calculates the full second-derivative matrix of FCN*-*-*-*-*-*-*-*
//*-*        ===================================================
//*-*        by taking finite differences. When calculating diagonal
//*-*        elements, it may iterate so that step size is nearly that
//*-*        which gives function change= UP/10. The first derivatives
//*-*        of course come as a free side effect, but with a smaller
//*-*        step size in order to obtain a known accuracy.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double dmin_, dxdi, elem, wint, tlrg2, d, dlast, ztemp, g2bfor;
   double df, aimsag, fs1, tlrstp, fs2, stpinm, g2i, sag=0, xtf, xti, xtj;
   int32_t icyc, ncyc, ndex, idrv, iext, npar2, i, j, ifail, npard, nparx, id, multpy;
   bool ldebug;

   ldebug = fIdbg[3] >= 1;
   if (fAmin == fUndefi) {
      mnamin();
   }
   if (fIstrat <= 0) {
      ncyc   = 3;
      tlrstp = .5;
      tlrg2  = .1;
   } else if (fIstrat == 1) {
      ncyc   = 5;
      tlrstp = .3;
      tlrg2  = .05;
   } else {
      ncyc   = 7;
      tlrstp = .1;
      tlrg2  = .02;
   }
   if (fISW[4] >= 2 || ldebug) {
      log_info("   START COVARIANCE MATRIX CALCULATION.");
   }
   fCfrom  = "HESSE   ";
   fNfcnfr = fNfcn;
   fCstatu = "OK        ";
   npard   = fNpar;
//*-*-                make sure starting at the right place
   mninex(fX);
   nparx = fNpar;
   Eval(nparx, fGin, fs1, fU, 4);    ++fNfcn;
   if (fs1 != fAmin) {
      df    = fAmin - fs1;
      mnwarn("D", "MNHESS", (boost::format("function value differs from AMIN by %g") % df).str().c_str());
   }
   fAmin = fs1;
   if (ldebug) {
      log_info(" PAR D   GSTEP           D          G2         GRD         SAG    ");
   }
//*-*-                                       . . . . . . diagonal elements .

//*-*-        fISW[1] = 1 if approx, 2 if not posdef, 3 if ok
//*-*-        AIMSAG is the sagitta we are aiming for in second deriv calc.

   aimsag = std::sqrt(fEpsma2)*(std::abs(fAmin) + fUp);
//*-*-        Zero the second derivative matrix
   npar2 = fNpar*(fNpar + 1) / 2;
   for (i = 1; i <= npar2; ++i) { fVhmat[i-1] = 0; }

//*-*-        Loop over variable parameters for second derivatives
   idrv = 2;
   for (id = 1; id <= npard; ++id) {
      i = id + fNpar - npard;
      iext = fNexofi[i-1];
      if (fG2[i-1] == 0) {
         mnwarn("W", "HESSE", (boost::format("Second derivative enters zero, param %d") % iext).str().c_str());
         wint = fWerr[i-1];
         if (fNvarl[iext-1] > 1) {
            mndxdi(fX[i-1], i-1, dxdi);
            if (std::abs(dxdi) < .001) wint = .01;
            else                          wint /= std::abs(dxdi);
         }
         fG2[i-1] = fUp / (wint*wint);
      }
      xtf   = fX[i-1];
      dmin_ = fEpsma2*8*std::abs(xtf);

//*-*-                              find step which gives sagitta = AIMSAG
      d = std::abs(fGstep[i-1]);
      int skip50 = 0;
      for (icyc = 1; icyc <= ncyc; ++icyc) {
//*-*-                              loop here only if SAG=0
         for (multpy = 1; multpy <= 5; ++multpy) {
//*-*-          take two steps
            fX[i-1] = xtf + d;
            mninex(fX);
            nparx = fNpar;
            Eval(nparx, fGin, fs1, fU, 4);    ++fNfcn;
            fX[i-1] = xtf - d;
            mninex(fX);
            Eval(nparx, fGin, fs2, fU, 4);    ++fNfcn;
            fX[i-1] = xtf;
            sag = (fs1 + fs2 - fAmin*2)*.5;
            if (sag != 0) goto L30;
            if (fGstep[i-1] < 0) {
               if (d >= .5) goto L26;
               d *= 10;
               if (d > .5)         d = .51;
               continue;
            }
            d *= 10;
         }
L26:
         mnwarn("W", "HESSE", (boost::format("Second derivative zero for parameter%d") % iext).str().c_str());
         goto L390;
//*-*-                            SAG is not zero
L30:
         g2bfor    = fG2[i-1];
         fG2[i-1]  = sag*2 / (d*d);
         fGrd[i-1] = (fs1 - fs2) / (d*2);
         if (ldebug) {
            log_info("%4d%2d%12.5g%12.5g%12.5g%12.5g%12.5g",i,idrv,fGstep[i-1],d,fG2[i-1],fGrd[i-1],sag);
         }
         if (fGstep[i-1] > 0) fGstep[i-1] =  std::abs(d);
         else                 fGstep[i-1] = -std::abs(d);
         fDirin[i-1] = d;
         fHESSyy[i-1]= fs1;
         dlast       = d;
         d           = std::sqrt(aimsag*2 / std::abs(fG2[i-1]));
//*-*-        if parameter has limits, max int step size = 0.5
         stpinm = .5;
         if (fGstep[i-1] < 0) d = std::min(d,stpinm);
         if (d < dmin_) d = dmin_;
//*-*-          see if converged
         if (std::abs((d - dlast) / d) < tlrstp ||
            std::abs((fG2[i-1] - g2bfor) / fG2[i-1]) < tlrg2) {
            skip50 = 1;
            break;
         }
         d = std::min(d,dlast*102);
         d = std::max(d,dlast*.1);
      }
//*-*-                      end of step size loop
      if (!skip50)
         mnwarn("D", "MNHESS", (boost::format("Second Deriv. SAG,AIM= %d%g%g") % iext % sag % aimsag).str().c_str());

      ndex = i*(i + 1) / 2;
      fVhmat[ndex-1] = fG2[i-1];
   }
//*-*-                             end of diagonal second derivative loop
   mninex(fX);
//*-*-                                    refine the first derivatives
   if (fIstrat > 0) mnhes1();
   fISW[1] = 3;
   fDcovar = 0;
//*-*-                                       . . . .  off-diagonal elements

   if (fNpar == 1) goto L214;
   for (i = 1; i <= fNpar; ++i) {
      for (j = 1; j <= i-1; ++j) {
         xti     = fX[i-1];
         xtj     = fX[j-1];
         fX[i-1] = xti + fDirin[i-1];
         fX[j-1] = xtj + fDirin[j-1];
         mninex(fX);
         Eval(nparx, fGin, fs1, fU, 4);            ++fNfcn;
         fX[i-1] = xti;
         fX[j-1] = xtj;
         elem = (fs1 + fAmin - fHESSyy[i-1] - fHESSyy[j-1]) / (
                    fDirin[i-1]*fDirin[j-1]);
         ndex = i*(i-1) / 2 + j;
         fVhmat[ndex-1] = elem;
      }
   }
L214:
   mninex(fX);
//*-*-                 verify matrix positive-definite
   mnpsdf();
   for (i = 1; i <= fNpar; ++i) {
      for (j = 1; j <= i; ++j) {
         ndex = i*(i-1) / 2 + j;
         fP[i + j*fMaxpar - fMaxpar-1] = fVhmat[ndex-1];
         fP[j + i*fMaxpar - fMaxpar-1] = fP[i + j*fMaxpar - fMaxpar-1];
      }
   }
   mnvert(fP, fMaxint, fMaxint, fNpar, ifail);
   if (ifail > 0) {
      mnwarn("W", "HESSE", "Matrix inversion fails.");
      goto L390;
   }
//*-*-                                       . . . . . . .  calculate  e d m
   fEDM = 0;

   for (i = 1; i <= fNpar; ++i) {
//*-*-                             off-diagonal elements
      ndex = i*(i-1) / 2;
      for (j = 1; j <= i-1; ++j) {
         ++ndex;
         ztemp = fP[i + j*fMaxpar - fMaxpar-1]*2;
         fEDM += fGrd[i-1]*ztemp*fGrd[j-1];
         fVhmat[ndex-1] = ztemp;
      }
//*-*-                             diagonal elements
      ++ndex;
      fVhmat[ndex-1] = fP[i + i*fMaxpar - fMaxpar-1]*2;
      fEDM += fP[i + i*fMaxpar - fMaxpar-1]*(fGrd[i-1]*fGrd[i-1]);
   }
   if (fISW[4] >= 1 && fISW[1] == 3 && fItaur == 0) {
      log_info(" COVARIANCE MATRIX CALCULATED SUCCESSFULLY");
   }
   goto L900;
//*-*-                             failure to invert 2nd deriv matrix
L390:
   fISW[1] = 1;
   fDcovar = 1;
   fCstatu = "FAILED    ";
   if (fISW[4] >= 0) {
      log_info("  MNHESS FAILS AND WILL RETURN DIAGONAL MATRIX. ");
   }
   for (i = 1; i <= fNpar; ++i) {
      ndex = i*(i-1) / 2;
      for (j = 1; j <= i-1; ++j) {
         ++ndex;
         fVhmat[ndex-1] = 0;
      }
      ++ndex;
      g2i = fG2[i-1];
      if (g2i <= 0) g2i = 1;
      fVhmat[ndex-1] = 2 / g2i;
   }
L900:
   return;
} /* mnhess_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnhes1()
{
//*-*-*-*Calculate first derivatives (GRD) and uncertainties (DGRD)*-*-*-*-*-*
//*-*    ==========================================================
//*-*         and appropriate step sizes GSTEP
//*-*      Called from MNHESS and MNGRAD
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double dmin_, d, dfmin, dgmin=0, change, chgold, grdold=0, epspri;
   double fs1, optstp, fs2, grdnew=0, sag, xtf;
   int32_t icyc, ncyc=0, idrv, i, nparx;
   bool ldebug;

   ldebug = fIdbg[5] >= 1;
   if (fIstrat <= 0) ncyc = 1;
   if (fIstrat == 1) ncyc = 2;
   if (fIstrat > 1)  ncyc = 6;
   idrv = 1;
   nparx = fNpar;
   dfmin = fEpsma2*4*(std::abs(fAmin) + fUp);
//*-*-                                    main loop over parameters
   for (i = 1; i <= fNpar; ++i) {
      xtf    = fX[i-1];
      dmin_  = fEpsma2*4*std::abs(xtf);
      epspri = fEpsma2 + std::abs(fGrd[i-1]*fEpsma2);
      optstp = std::sqrt(dfmin / (std::abs(fG2[i-1]) + epspri));
      d = std::abs(fGstep[i-1])*.2;
      if (d > optstp) d = optstp;
      if (d < dmin_)  d = dmin_;
      chgold = 1e4;
//*-*-                                      iterate reducing step size
      for (icyc = 1; icyc <= ncyc; ++icyc) {
         fX[i-1] = xtf + d;
         mninex(fX);
         Eval(nparx, fGin, fs1, fU, 4);            ++fNfcn;
         fX[i-1] = xtf - d;
         mninex(fX);
         Eval(nparx, fGin, fs2, fU, 4);            ++fNfcn;
         fX[i-1] = xtf;
//*-*-                                      check if step sizes appropriate
         sag    = (fs1 + fs2 - fAmin*2)*.5;
         grdold = fGrd[i-1];
         grdnew = (fs1 - fs2) / (d*2);
         dgmin  = fEpsmac*(std::abs(fs1) + std::abs(fs2)) / d;
         if (ldebug) {
            log_info("%4d%2d%12.5g%12.5g%12.5g%12.5g%12.5g",i,idrv,fGstep[i-1],d,fG2[i-1],grdnew,sag);
         }
         if (grdnew == 0) goto L60;
         change = std::abs((grdold - grdnew) / grdnew);
         if (change > chgold && icyc > 1) goto L60;
         chgold    = change;
         fGrd[i-1] = grdnew;
         if (fGstep[i-1] > 0) fGstep[i-1] =  std::abs(d);
         else                 fGstep[i-1] = -std::abs(d);
//*-*-                 decrease step until first derivative changes by <5%
         if (change < .05) goto L60;
         if (std::abs(grdold - grdnew) < dgmin) goto L60;
         if (d < dmin_) {
            mnwarn("D", "MNHES1", "Step size too small for 1st drv.");
            goto L60;
         }
         d *= .2;
      }
//*-*-                                      loop satisfied = too many iter
      mnwarn("D", "MNHES1", (boost::format("Too many iterations on D1.%g%g") % grdold % grdnew).str().c_str());
L60:
      fDgrd[i-1] = std::max(dgmin,std::abs(grdold - grdnew));
   }
//*-*-                                       end of first deriv. loop
   mninex(fX);
} /* mnhes1_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnimpr()
{
//*-*-*-*-*-*-*Attempts to improve on a good local minimum*-*-*-*-*-*-*-*-*-*
//*-*          ===========================================
//*-*        Attempts to improve on a good local minimum by finding a
//*-*        better one.   The quadratic part of FCN is removed by MNCALF
//*-*        and this transformed function is minimized using the simplex
//*-*        method from several random starting points.
//*-*        ref. -- Goldstein and Price, Math.Comp. 25, 569 (1971)
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Initialized data */

   static double rnum = 0;

   /* Local variables */
   double amax, ycalf, ystar, ystst;
   double pb, ep, wg, xi, sigsav, reg, sig2;
   int32_t npfn, ndex, loop=0, i, j, ifail, iseed=0;
   int32_t jhold, nloop, nparx, nparp1, jh, jl, iswtr;

   if (fNpar <= 0) return;
   if (fAmin == fUndefi) mnamin();
   fCstatu = "UNCHANGED ";
   fItaur  = 1;
   fEpsi   = fUp*.1;
   npfn    = fNfcn;
   nloop   = int32_t(fWord7[1]);
   if (nloop <= 0) nloop = fNpar + 4;
   nparx  = fNpar;
   nparp1 = fNpar + 1;
   wg = 1 / double(fNpar);
   sigsav = fEDM;
   fApsi  = fAmin;
   iswtr   = fISW[4] - 2*fItaur;
   for (i = 1; i <= fNpar; ++i) {
      fXt[i-1]       = fX[i-1];
      fIMPRdsav[i-1] = fWerr[i-1];
      for (j = 1; j <= i; ++j) {
         ndex = i*(i-1) / 2 + j;
         fP[i + j*fMaxpar - fMaxpar-1] = fVhmat[ndex-1];
         fP[j + i*fMaxpar - fMaxpar-1] = fP[i + j*fMaxpar - fMaxpar-1];
      }
   }
   mnvert(fP, fMaxint, fMaxint, fNpar, ifail);
   if (ifail >= 1) goto L280;
//*-*-              Save inverted matrix in VT
   for (i = 1; i <= fNpar; ++i) {
      ndex = i*(i-1) / 2;
      for (j = 1; j <= i; ++j) {
         ++ndex;
         fVthmat[ndex-1] = fP[i + j*fMaxpar - fMaxpar-1];
      }
   }
   loop = 0;

L20:
   for (i = 1; i <= fNpar; ++i) {
      fDirin[i-1] = fIMPRdsav[i-1]*2;
      mnrn15(rnum, iseed);
      fX[i-1] = fXt[i-1] + fDirin[i-1]*2*(rnum - .5);
   }
   ++loop;
   reg = 2;
   if (fISW[4] >= 0) {
      log_info("START ATTEMPT NO.%2d TO FIND NEW MINIMUM",loop);
   }
L30:
   mncalf(fX, ycalf);
   fAmin = ycalf;
//*-*-                                       . . . . set up  random simplex
   jl = nparp1;
   jh = nparp1;
   fIMPRy[nparp1-1] = fAmin;
   amax = fAmin;
   for (i = 1; i <= fNpar; ++i) {
      xi = fX[i-1];
      mnrn15(rnum, iseed);
      fX[i-1] = xi - fDirin[i-1]*(rnum - .5);
      mncalf(fX, ycalf);
      fIMPRy[i-1] = ycalf;
      if (fIMPRy[i-1] < fAmin) {
         fAmin = fIMPRy[i-1];
         jl    = i;
      } else if (fIMPRy[i-1] > amax) {
         amax = fIMPRy[i-1];
         jh   = i;
      }
      for (j = 1; j <= fNpar; ++j) { fP[j + i*fMaxpar - fMaxpar-1] = fX[j-1]; }
      fP[i + nparp1*fMaxpar - fMaxpar-1] = xi;
      fX[i-1] = xi;
   }

   fEDM = fAmin;
   sig2 = fEDM;
//*-*-                                       . . . . . . .  start main loop
L50:
   if (fAmin < 0)   goto L95;
   if (fISW[1] <= 2) goto L280;
   ep = fAmin*.1;
   if (sig2 < ep && fEDM < ep) goto L100;
   sig2 = fEDM;
   if (fNfcn - npfn > fNfcnmx) goto L300;
//*-*-        calculate new point * by reflection
   for (i = 1; i <= fNpar; ++i) {
      pb = 0;
      for (j = 1; j <= nparp1; ++j) { pb += wg*fP[i + j*fMaxpar - fMaxpar-1]; }
      fPbar[i-1]  = pb - wg*fP[i + jh*fMaxpar - fMaxpar-1];
      fPstar[i-1] = fPbar[i-1]*2 - fP[i + jh*fMaxpar - fMaxpar-1]*1;
   }
   mncalf(fPstar, ycalf);
   ystar = ycalf;
   if (ystar >= fAmin) goto L70;
//*-*-        point * better than jl, calculate new point **
   for (i = 1; i <= fNpar; ++i) {
      fPstst[i-1] = fPstar[i-1]*2 + fPbar[i- 1]*-1;
   }
   mncalf(fPstst, ycalf);
   ystst = ycalf;
   if (ystst < fIMPRy[jl-1]) goto L67;
   mnrazz(ystar, fPstar, fIMPRy, jh, jl);
   goto L50;
L67:
   mnrazz(ystst, fPstst, fIMPRy, jh, jl);
   goto L50;
//*-*-        point * is not as good as jl
L70:
   if (ystar >= fIMPRy[jh-1]) goto L73;
   jhold = jh;
   mnrazz(ystar, fPstar, fIMPRy, jh, jl);
   if (jhold != jh) goto L50;
//*-*-        calculate new point **
L73:
   for (i = 1; i <= fNpar; ++i) {
      fPstst[i-1] = fP[i + jh*fMaxpar - fMaxpar-1]*.5 + fPbar[i-1]*.5;
   }
   mncalf(fPstst, ycalf);
   ystst = ycalf;
   if (ystst > fIMPRy[jh-1]) goto L30;
//*-*-    point ** is better than jh
   if (ystst < fAmin) goto L67;
   mnrazz(ystst, fPstst, fIMPRy, jh, jl);
   goto L50;
//*-*-                                       . . . . . .  end main loop
L95:
   if (fISW[4] >= 0) {
      log_info(" AN IMPROVEMENT ON THE PREVIOUS MINIMUM HAS BEEN FOUND");
   }
   reg = .1;
//*-*-                                       . . . . . ask if point is new
L100:
   mninex(fX);
   Eval(nparx, fGin, fAmin, fU, 4);    ++fNfcn;
   for (i = 1; i <= fNpar; ++i) {
      fDirin[i-1] = reg*fIMPRdsav[i-1];
      if (std::abs(fX[i-1] - fXt[i-1]) > fDirin[i-1])     goto L150;
   }
   goto L230;
L150:
   fNfcnmx = fNfcnmx + npfn - fNfcn;
   npfn    = fNfcn;
   mnsimp();
   if (fAmin >= fApsi) goto L325;
   for (i = 1; i <= fNpar; ++i) {
      fDirin[i-1] = fIMPRdsav[i-1]*.1;
      if (std::abs(fX[i-1] - fXt[i-1]) > fDirin[i-1])     goto L250;
   }
L230:
   if (fAmin < fApsi)         goto L350;
   goto L325;
/*                                        . . . . . . truly new minimum */
L250:
   fLnewmn = true;
   if (fISW[1] >= 1) {
      fISW[1] = 1;
      fDcovar = std::max(fDcovar,.5);
   } else fDcovar = 1;
   fItaur  = 0;
   fNfcnmx = fNfcnmx + npfn - fNfcn;
   fCstatu = "NEW MINIMU";
   if (fISW[4] >= 0) {
      log_info(" IMPROVE HAS FOUND A TRULY NEW MINIMUM");
      log_info(" *************************************");
   }
   return;
//*-*-                                       . . . return to previous region
L280:
   if (fISW[4] > 0) {
      log_info(" COVARIANCE MATRIX WAS NOT POSITIVE-DEFINITE");
   }
   goto L325;
L300:
   fISW[0] = 1;
L325:
   for (i = 1; i <= fNpar; ++i) {
      fDirin[i-1] = fIMPRdsav[i-1]*.01;
      fX[i-1]     = fXt[i-1];
   }
   fAmin = fApsi;
   fEDM  = sigsav;
L350:
   mninex(fX);
   if (fISW[4] > 0) {
      log_info(" IMPROVE HAS RETURNED TO REGION OF ORIGINAL MINIMUM");
   }
   fCstatu = "UNCHANGED ";
   mnrset(0);
   if (fISW[1] < 2) goto L380;
   if (loop < nloop && fISW[0] < 1) goto L20;
L380:
   if (iswtr >= 0) mnprin(5, fAmin);
   fItaur = 0;
} /* mnimpr_ */

//______________________________________________________________________________
void lilliput::TMinuit::mninex(double *pint)
{
//*-*-*-*-*Transforms from internal coordinates (PINT) to external (U)*-*-*-*
//*-*      ===========================================================
//*-*        The minimizing routines which work in
//*-*        internal coordinates call this routine before calling FCN.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   int32_t i, j;

   for (j = 0; j < fNpar; ++j) {
      i = fNexofi[j]-1;
      if (fNvarl[i] == 1) {
         fU[i] = pint[j];
      } else {
         fU[i] = fAlim[i] + (std::sin(pint[j]) + 1)*.5*(fBlim[i] - fAlim[i]);
      }
   }
} /* mninex_ */

//______________________________________________________________________________
void lilliput::TMinuit::mninit(int32_t i1, int32_t i2, int32_t i3)
{
//*-*-*-*-*-*Main initialization member function for MINUIT*-*-*-*-*-*-*-*-*
//*-*        ==============================================
//*-*     It initializes some constants
//*-*                (including the logical I/O unit nos.),
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double piby2, epsp1, epsbak, epstry, distnn;
   int32_t i, idb;

//*-*-           I/O unit numbers
   fIsysrd = i1;
   fIsyswr = i2;
   fIstkwr[0] = fIsyswr;
   fNstkwr = 1;
   fIsyssa = i3;
   fNstkrd = 0;
//*-*-              version identifier
   fCvrsn  = "95.03++ ";
//*-*-              some CONSTANT
   fMaxint = fMaxpar;
   fMaxext = 2*fMaxpar;
   fUndefi = -54321;
   fBigedm = 123456;
   fCundef = ")UNDEFINED";
   fCovmes[0] = "NO ERROR MATRIX       ";
   fCovmes[1] = "ERR MATRIX APPROXIMATE";
   fCovmes[2] = "ERR MATRIX NOT POS-DEF";
   fCovmes[3] = "ERROR MATRIX ACCURATE ";
//*-*-               some starting values
   fNblock = 0;
   fIcomnd = 0;
   fCtitl  = fCundef;
   fCfrom  = "INPUT   ";
   fNfcn   = 0;
   fNfcnfr = fNfcn;
   fCstatu = "INITIALIZE";
   fISW[2] = 0;
   fISW[3] = 0;
   fISW[4] = 1;
//*-*-        fISW[5]=0 for batch jobs,  =1 for interactive jobs
//*-*-                     =-1 for originally interactive temporarily batch

   fISW[5] = 0;
//    if (intrac(&dummy)) fISW[5] = 1;
//*-*-       DEBUG options set to default values
   for (idb = 0; idb <= 10; ++idb) { fIdbg[idb] = 0; }
   fLrepor = false;
   fLwarn  = true;
   fLimset = false;
   fLnewmn = false;
   fIstrat = 1;
   fItaur  = 0;
//*-*-       default page dimensions and 'new page' carriage control integer
   fNpagwd = 120;
   fNpagln = 56;
   fNewpag = 1;
   if (fISW[5] > 0) {
      fNpagwd = 80;
      fNpagln = 30;
      fNewpag = 0;
   }
   fUp = 1;
   fUpdflt = fUp;
//*-*-                  determine machine accuracy epsmac
   epstry = .5;
   for (i = 1; i <= 100; ++i) {
      epstry *= .5;
      epsp1 = epstry + 1;
      mntiny(epsp1, epsbak);
      if (epsbak < epstry) goto L35;
   }
   epstry = 1e-7;
   fEpsmac = epstry*4;
   log_info(" MNINIT UNABLE TO DETERMINE ARITHMETIC PRECISION. WILL ASSUME:%g",fEpsmac);
L35:
   fEpsmac = epstry*8;
   fEpsma2 = std::sqrt(fEpsmac)*2;
//*-*-                the vlims are a non-negligible distance from pi/2
//*-*-        used by MNPINT to set variables "near" the physical limits
   piby2   = std::atan(1)*2;
   distnn  = std::sqrt(fEpsma2)*8;
   fVlimhi =  piby2 - distnn;
   fVlimlo = -piby2 + distnn;
   mncler();
//    log_info("  MINUIT RELEASE %s INITIALIZED.   DIMENSIONS 100/50  EPSMAC=%g",(const char*)fCvrsn,fEpsmac);
} /* mninit_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnlims()
{
//*-*-*-*Interprets the SET LIM command, to reset the parameter limits*-*-*-*
//*-*    =============================================================
//*-*       Called from MNSET
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double dxdi, snew;
   int32_t kint, i2, newcod, ifx=0, inu;

   fCfrom  = "SET LIM ";
   fNfcnfr = fNfcn;
   fCstatu = "NO CHANGE ";
   i2 = int32_t(fWord7[0]);
   if (i2 > fMaxext || i2 < 0) goto L900;
   if (i2 > 0) goto L30;
//*-*-                                    set limits on all parameters
   newcod = 4;
   if (fWord7[1] == fWord7[2]) newcod = 1;
   for (inu = 1; inu <= fNu; ++inu) {
      if (fNvarl[inu-1] <= 0) continue;
      if (fNvarl[inu-1] == 1 && newcod == 1) continue;
      kint = fNiofex[inu-1];
//*-*-            see if parameter has been fixed
      if (kint <= 0) {
         if (fISW[4] >= 0) {
            log_info("           LIMITS NOT CHANGED FOR FIXED PARAMETER:%4d",inu);
         }
         continue;
      }
      if (newcod == 1) {
//*-*-           remove limits from parameter
         if (fISW[4] > 0) {
            log_info(" LIMITS REMOVED FROM PARAMETER  :%3d",inu);
         }
         fCstatu = "NEW LIMITS";
         mndxdi(fX[kint-1], kint-1, dxdi);
         snew           = fGstep[kint-1]*dxdi;
         fGstep[kint-1] = std::abs(snew);
         fNvarl[inu-1]  = 1;
      } else {
//*-*-            put limits on parameter
         fAlim[inu-1] = std::min(fWord7[1],fWord7[2]);
         fBlim[inu-1] = std::max(fWord7[1],fWord7[2]);
         if (fISW[4] > 0) {
            log_info(" PARAMETER %3d LIMITS SET TO  %15.5g%15.5g",inu,fAlim[inu-1],fBlim[inu-1]);
         }
         fNvarl[inu-1]  = 4;
         fCstatu        = "NEW LIMITS";
         fGstep[kint-1] = -.1;
      }
   }
   goto L900;
//*-*-                                      set limits on one parameter
L30:
   if (fNvarl[i2-1] <= 0) {
      log_info(" PARAMETER %3d IS NOT VARIABLE.", i2);
      goto L900;
   }
   kint = fNiofex[i2-1];
//*-*-                                      see if parameter was fixed
   if (kint == 0) {
      log_info(" REQUEST TO CHANGE LIMITS ON FIXED PARAMETER:%3d",i2);
      for (ifx = 1; ifx <= fNpfix; ++ifx) {
         if (i2 == fIpfix[ifx-1]) goto L92;
      }
      log_info(" MINUIT BUG IN MNLIMS. SEE F. JAMES");
L92:
      ;
   }
   if (fWord7[1] != fWord7[2]) goto L235;
//*-*-                                      remove limits
   if (fNvarl[i2-1] != 1) {
      if (fISW[4] > 0) {
         log_info(" LIMITS REMOVED FROM PARAMETER  %2d",i2);
      }
      fCstatu = "NEW LIMITS";
      if (kint <= 0) {
         fGsteps[ifx-1] = std::abs(fGsteps[ifx-1]);
      } else {
         mndxdi(fX[kint-1], kint-1, dxdi);
         if (std::abs(dxdi) < .01) dxdi = .01;
         fGstep[kint-1] = std::abs(fGstep[kint-1]*dxdi);
         fGrd[kint-1]  *= dxdi;
      }
      fNvarl[i2-1] = 1;
   } else {
      log_info(" NO LIMITS SPECIFIED.  PARAMETER %3d IS ALREADY UNLIMITED.  NO CHANGE.",i2);
   }
   goto L900;
//*-*-                                       put on limits
L235:
   fAlim[i2-1]  = std::min(fWord7[1],fWord7[2]);
   fBlim[i2-1]  = std::max(fWord7[1],fWord7[2]);
   fNvarl[i2-1] = 4;
   if (fISW[4] > 0) {
      log_info(" PARAMETER %3d LIMITS SET TO  %15.5g%15.5g",i2,fAlim[i2-1],fBlim[i2-1]);
   }
   fCstatu = "NEW LIMITS";
   if (kint <= 0) fGsteps[ifx-1] = -.1;
   else           fGstep[kint-1] = -.1;

L900:
   if (fCstatu != "NO CHANGE ") {
      mnexin(fX);
      mnrset(1);
   }
} /* mnlims_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnline(double *start, double fstart, double *step, double slope, double toler)
{
//*-*-*-*-*-*-*-*-*-*Perform a line search from position START*-*-*-*-*-*-*-*
//*-*                =========================================
//*-*        along direction STEP, where the length of vector STEP
//*-*                   gives the expected position of minimum.
//*-*        FSTART is value of function at START
//*-*        SLOPE (if non-zero) is df/dx along STEP at START
//*-*        TOLER is initial tolerance of minimum in direction STEP
//*-*
//*-* SLAMBG and ALPHA control the maximum individual steps allowed.
//*-* The first step is always =1. The max length of second step is SLAMBG.
//*-* The max size of subsequent steps is the maximum previous successful
//*-*   step multiplied by ALPHA + the size of most recent successful step,
//*-*   but cannot be smaller than SLAMBG.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double xpq[12], ypq[12], slam, sdev, coeff[3], denom, flast;
   double fvals[3], xvals[3], f1, fvmin, xvmin, ratio, f2, f3, fvmax;
   double toler8, toler9, overal, undral, slamin, slamax, slopem;
   int32_t i, nparx=0, nvmax=0, nxypt, kk, ipt;
   bool ldebug;
   std::string cmess;
   // char chpq[13];
   int     l65, l70, l80;

   /* Function Body */
   l65 = 0; l70 = 0; l80 = 0;
   ldebug = fIdbg[1] >= 1;
//*-*-                 starting values for overall limits on total step SLAM
   overal = 1e3;
   undral = -100;
//*-*-                             debug check if start is ok
   if (ldebug) {
      mninex(&start[0]);
      Eval(nparx, fGin, f1, fU, 4);        ++fNfcn;
      if (f1 != fstart) {
         log_info(" MNLINE start point not consistent, F values, parameters=");
         for (kk = 1; kk <= fNpar; ++kk) {
            log_info("  %14.5e",fX[kk-1]);
         }
      }
   }
//*-*-                                     . set up linear search along STEP
   fvmin   = fstart;
   xvmin   = 0;
   nxypt   = 1;
   // chpq[0] = charal[0];
   xpq[0]  = 0;
   ypq[0]  = fstart;
//*-*-              SLAMIN = smallest possible value of ABS(SLAM)
   slamin = 0;
   for (i = 1; i <= fNpar; ++i) {
      if (step[i-1] != 0) {
         ratio = std::abs(start[i-1] / step[i-1]);
         if (slamin == 0)    slamin = ratio;
         if (ratio < slamin) slamin = ratio;
      }
      fX[i-1] = start[i-1] + step[i-1];
   }
   if (slamin == 0) slamin = fEpsmac;
   slamin *= fEpsma2;
   nparx = fNpar;

   mninex(fX);
   Eval(nparx, fGin, f1, fU, 4);    ++fNfcn;
   ++nxypt;
   // chpq[nxypt-1] = charal[nxypt-1];
   xpq[nxypt-1] = 1;
   ypq[nxypt-1] = f1;
   if (f1 < fstart) {
      fvmin = f1;
      xvmin = 1;
   }
//*-*-                        . quadr interp using slope GDEL and two points
   slam   = 1;
   toler8 = toler;
   slamax = 5;
   flast  = f1;
//*-*-                        can iterate on two-points (cut) if no imprvmnt

   do {
      denom = (flast - fstart - slope*slam)*2 / (slam*slam);
      slam  = 1;
      if (denom != 0)    slam = -slope / denom;
      if (slam < 0)      slam = slamax;
      if (slam > slamax) slam = slamax;
      if (slam < toler8) slam = toler8;
      if (slam < slamin) {
         l80 = 1;
         break;
      }
      if (std::abs(slam - 1) < toler8 && f1 < fstart) {
         l70 = 1;
         break;
      }
      if (std::abs(slam - 1) < toler8) slam = toler8 + 1;
      if (nxypt >= 12) {
         l65 = 1;
         break;
      }
      for (i = 1; i <= fNpar; ++i) { fX[i-1] = start[i-1] + slam*step[i-1]; }
      mninex(fX);
      nparx = fNpar;
      Eval(nparx, fGin, f2, fU, 4);    ++fNfcn;
      ++nxypt;
      // chpq[nxypt-1] = charal[nxypt-1];
      xpq[nxypt-1]  = slam;
      ypq[nxypt-1]  = f2;
      if (f2 < fvmin) {
         fvmin = f2;
         xvmin = slam;
      }
      if (fstart == fvmin) {
         flast  = f2;
         toler8 = toler*slam;
         overal = slam - toler8;
         slamax = overal;
      }
   } while (fstart == fvmin);

   if (!l65 && !l70 && !l80) {
//*-*-                                       . quadr interp using 3 points
      xvals[0] = xpq[0];
      fvals[0] = ypq[0];
      xvals[1] = xpq[nxypt-2];
      fvals[1] = ypq[nxypt-2];
      xvals[2] = xpq[nxypt-1];
      fvals[2] = ypq[nxypt-1];
//*-*-                            begin iteration, calculate desired step
      do {
         slamax = std::max(slamax,std::abs(xvmin)*2);
         mnpfit(xvals, fvals, 3, coeff, sdev);
         if (coeff[2] <= 0) {
            slopem = coeff[2]*2*xvmin + coeff[1];
            if (slopem <= 0)  slam = xvmin + slamax;
            else              slam = xvmin - slamax;
         } else {
            slam = -coeff[1] / (coeff[2]*2);
            if (slam > xvmin + slamax) slam = xvmin + slamax;
            if (slam < xvmin - slamax) slam = xvmin - slamax;
         }
         if (slam > 0) {
            if (slam > overal)
               slam = overal;
            else if (slam < undral)
               slam = undral;
         }

//*-*-              come here if step was cut below
         do {
            toler9 = std::max(toler8,std::abs(toler8*slam));
            for (ipt = 1; ipt <= 3; ++ipt) {
               if (std::abs(slam - xvals[ipt-1]) < toler9) {
                  l70 = 1;
                  break;
               }
            }
            if (l70) break;
//*-*-               take the step
            if (nxypt >= 12) {
               l65 = 1;
               break;
            }
            for (i = 1; i <= fNpar; ++i) { fX[i-1] = start[i-1] + slam*step[i-1]; }
            mninex(fX);
            Eval(nparx, fGin, f3, fU, 4);    ++fNfcn;
            ++nxypt;
            // chpq[nxypt-1] = charal[nxypt-1];
            xpq[nxypt-1]  = slam;
            ypq[nxypt-1]  = f3;
//*-*-            find worst previous point out of three
            fvmax = fvals[0];
            nvmax = 1;
            if (fvals[1] > fvmax) {
               fvmax = fvals[1];
               nvmax = 2;
            }
            if (fvals[2] > fvmax) {
               fvmax = fvals[2];
               nvmax = 3;
            }
//*-*-             if latest point worse than all three previous, cut step
            if (f3 >= fvmax) {
               if (nxypt >= 12) {
                  l65 = 1;
                  break;
               }
               if (slam > xvmin) overal = std::min(overal,slam - toler8);
               if (slam < xvmin) undral = std::max(undral,slam + toler8);
               slam = (slam + xvmin)*.5;
            }
         } while (f3 >= fvmax);

//*-*-             prepare another iteration, replace worst previous point
         if (l65 || l70) break;

         xvals[nvmax-1] = slam;
         fvals[nvmax-1] = f3;
         if (f3 < fvmin) {
            fvmin = f3;
            xvmin = slam;
         } else {
            if (slam > xvmin) overal = std::min(overal,slam - toler8);
            if (slam < xvmin) undral = std::max(undral,slam + toler8);
         }
      } while (nxypt < 12);
   }

//*-*-                                           . . end of iteration . . .
//*-*-           stop because too many iterations
   if (!l70 && !l80) {
      cmess = " LINE SEARCH HAS EXHAUSTED THE LIMIT OF FUNCTION CALLS ";
      if (ldebug) {
         log_info(" MNLINE DEBUG: steps=");
         for (kk = 1; kk <= fNpar; ++kk) {
            log_info("  %12.4g",step[kk-1]);
         }
      }
   }
//*-*-           stop because within tolerance
   if (l70) cmess = " LINE SEARCH HAS ATTAINED TOLERANCE ";
   if (l80) cmess = " STEP SIZE AT ARITHMETICALLY ALLOWED MINIMUM";

   fAmin = fvmin;
   for (i = 1; i <= fNpar; ++i) {
      fDirin[i-1] = step[i-1]*xvmin;
      fX[i-1]     = start[i-1] + fDirin[i-1];
   }
   mninex(fX);
   if (xvmin < 0) {
      mnwarn("D", "MNLINE", " LINE MINIMUM IN BACKWARDS DIRECTION");
   }
   if (fvmin == fstart) {
      mnwarn("D", "MNLINE", " LINE SEARCH FINDS NO IMPROVEMENT ");
   }
   if (ldebug) {
      log_info(" AFTER %3d POINTS,%s",nxypt,cmess.c_str());
      // mnplot(xpq, ypq, chpq, nxypt, fNpagwd, fNpagln);
      log_warn("mnplot plotting has been disabled");
   }
} /* mnline_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnmatu(int32_t kode)
{
//*-*-*-*-*-*-*-*Prints the covariance matrix v when KODE=1*-*-*-*-*-*-*-*-*
//*-*            ==========================================
//*-*        always prints the global correlations, and
//*-*        calculates and prints the individual correlation coefficients
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   int32_t ndex, i, j, m, n, ncoef, nparm, id, it, ix;
   int32_t nsofar, ndi, ndj, iso, isw2, isw5;
   std::string ctemp;

   isw2 = fISW[1];
   if (isw2 < 1) {
      log_info("%s",fCovmes[isw2].c_str());
      return;
   }
   if (fNpar == 0) {
      log_info(" MNMATU: NPAR=0");
      return;
   }
//*-*-                                      . . . . .external error matrix
   if (kode == 1) {
      isw5    = fISW[4];
      fISW[4] = 2;
      mnemat(fP, fMaxint);
      if (isw2 < 3) {
         log_info("%s",fCovmes[isw2].c_str());
      }
      fISW[4] = isw5;
   }
//*-*-                                      . . . . . correlation coeffs. .
   if (fNpar <= 1) return;
   mnwerr();
//*-*-    NCOEF is number of coeff. that fit on one line, not to exceed 20
   ncoef = (fNpagwd - 19) / 6;
   ncoef = std::min(ncoef,20);
   nparm = std::min(fNpar,ncoef);
   log_info(" PARAMETER  CORRELATION COEFFICIENTS  ");
   ctemp = "       NO.  GLOBAL";
   for (id = 1; id <= nparm; ++id) {
      ctemp += boost::str(boost::format(" %6d") % fNexofi[id-1]);
   }
   log_info("%s",ctemp.c_str());
   for (i = 1; i <= fNpar; ++i) {
      ix  = fNexofi[i-1];
      ndi = i*(i + 1) / 2;
      for (j = 1; j <= fNpar; ++j) {
         m    = std::max(i,j);
         n    = std::min(i,j);
         ndex = m*(m-1) / 2 + n;
         ndj  = j*(j + 1) / 2;
         fMATUvline[j-1] = fVhmat[ndex-1] / std::sqrt(std::abs(fVhmat[ndi-1]*fVhmat[ndj-1]));
      }
      nparm = std::min(fNpar,ncoef);
      ctemp = boost::str(boost::format("      %3d  %7.5f ") % ix % fGlobcc[i-1]);
      for (it = 1; it <= nparm; ++it) {
         ctemp += boost::str(boost::format(" %6.3f") % fMATUvline[it-1]);
      }
      log_info("%s",ctemp.c_str());
      if (i <= nparm) continue;
      ctemp = "                   ";
      for (iso = 1; iso <= 10; ++iso) {
         nsofar = nparm;
         nparm  = std::min(fNpar,nsofar + ncoef);
         for (it = nsofar + 1; it <= nparm; ++it) {
            ctemp = ctemp + boost::str(boost::format(" %6.3f") % fMATUvline[it-1]);
         }
         log_info("%s",ctemp.c_str());
         if (i <= nparm) break;
      }
   }
   if (isw2 < 3) {
      log_info(" %s",fCovmes[isw2].c_str());
   }
} /* mnmatu_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnmigr()
{
//*-*-*-*-*-*-*-*-*Performs a local function minimization*-*-*-*-*-*-*-*-*-*
//*-*              ======================================
//*-*        Performs a local function minimization using basically the
//*-*        method of Davidon-Fletcher-Powell as modified by Fletcher
//*-*        ref. -- Fletcher, Comp.J. 13,317 (1970)   "switching method"
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double gdel, gami, vlen, dsum, gssq, vsum, d;
   double fzero, fs, ri, delgam, rhotol;
   double gdgssq, gvg, vgi;
   int32_t npfn, ndex, iext, i, j, m, n, npsdf, nparx;
   int32_t iswtr, lined2, kk, nfcnmg, nrstrt,iter;
   bool ldebug;
   double toler = 0.05;

   if (fNpar <= 0) return;
   if (fAmin == fUndefi) mnamin();
   ldebug  = false; if ( fIdbg[4] >= 1) ldebug = true;
   fCfrom  = "MIGRAD  ";
   fNfcnfr = fNfcn;
   nfcnmg  = fNfcn;
   fCstatu = "INITIATE  ";
   iswtr   = fISW[4] - 2*fItaur;
   npfn    = fNfcn;
   nparx   = fNpar;
   vlen    = (double) (fNpar*(fNpar + 1) / 2);
   nrstrt  = 0;
   npsdf   = 0;
   lined2  = 0;
   fISW[3] = -1;
   rhotol  = fApsi*.001;
   if (iswtr >= 1) {
      log_info(" START MIGRAD MINIMIZATION.  STRATEGY %2d.  CONVERGENCE WHEN EDM .LT.%9.2e",fIstrat,rhotol);
   }
//*-*-                                          initialization strategy
   if (fIstrat < 2 || fISW[1] >= 3) goto L2;
//*-*-                               come (back) here to restart completely
L1:
   if (nrstrt > fIstrat) {
      fCstatu = "FAILED    ";
      fISW[3] = -1;
      goto L230;
   }
//*-*-                                     . get full covariance and gradient
   mnhess();
   mnwerr();
   npsdf = 0;
   if (fISW[1] >= 1) goto L10;
//*-*-                                       . get gradient at start point
L2:
   mninex(fX);
   if (fISW[2] == 1) {
      Eval(nparx, fGin, fzero, fU, 2);        ++fNfcn;
   }
   mnderi();
   if (fISW[1] >= 1) goto L10;
//*-*-                                  sometimes start with diagonal matrix
   for (i = 1; i <= fNpar; ++i) {
      fMIGRxxs[i-1]  = fX[i-1];
      fMIGRstep[i-1] = 0;
   }
//*-*-                          do line search if second derivative negative
   ++lined2;
   if (lined2 < (fIstrat + 1)*fNpar) {
      for (i = 1; i <= fNpar; ++i) {
         if (fG2[i-1] > 0) continue;
         if (fGrd[i-1] > 0) fMIGRstep[i-1] = -std::abs(fGstep[i-1]);
         else               fMIGRstep[i-1] =  std::abs(fGstep[i-1]);
         gdel = fMIGRstep[i-1]*fGrd[i-1];
         fs   = fAmin;
         mnline(fMIGRxxs, fs, fMIGRstep, gdel, toler);
         mnwarn("D", "MNMIGR", "Negative G2 line search");
         iext = fNexofi[i-1];
         if (ldebug) {
            log_info(" Negative G2 line search, param %3d %13.3g%13.3g",iext,fs,fAmin);
         }
         goto L2;
      }
   }
//*-*-                          make diagonal error matrix
   for (i = 1; i <= fNpar; ++i) {
      ndex = i*(i-1) / 2;
      for (j = 1; j <= i-1; ++j) {
         ++ndex;
         fVhmat[ndex-1] = 0;
      }
      ++ndex;
      if (fG2[i-1] <= 0) fG2[i-1] = 1;
      fVhmat[ndex-1] = 2 / fG2[i-1];
   }
   fDcovar = 1;
   if (ldebug) {
      log_info(" DEBUG MNMIGR, STARTING MATRIX DIAGONAL,  VHMAT=");
      for (kk = 1; kk <= int32_t(vlen); ++kk) {
         log_info(" %10.2g",fVhmat[kk-1]);
      }
   }
//*-*-                                        ready to start first iteration
L10:
   ++nrstrt;
   if (nrstrt > fIstrat + 1) {
      fCstatu = "FAILED    ";
      goto L230;
   }
   fs = fAmin;
//*-*-                                       . . . get EDM and set up loop
   fEDM = 0;
   for (i = 1; i <= fNpar; ++i) {
      fMIGRgs[i-1]  = fGrd[i-1];
      fMIGRxxs[i-1] = fX[i-1];
      ndex     = i*(i-1) / 2;
      for (j = 1; j <= i-1; ++j) {
         ++ndex;
         fEDM += fMIGRgs[i-1]*fVhmat[ndex-1]*fMIGRgs[j-1];
      }
      ++ndex;
      fEDM += fMIGRgs[i-1]*fMIGRgs[i-1]*.5*fVhmat[ndex-1];
   }
   fEDM = fEDM*.5*(fDcovar*3 + 1);
   if (fEDM < 0) {
      mnwarn("W", "MIGRAD", "STARTING MATRIX NOT POS-DEFINITE.");
      fISW[1] = 0;
      fDcovar = 1;
      goto L2;
   }
   if (fISW[1] == 0) fEDM = fBigedm;
   iter = 0;
   mninex(fX);
   mnwerr();
   if (iswtr >= 1) mnprin(3, fAmin);
   if (iswtr >= 2) mnmatu(0);
//*-*-                                       . . . . .  start main loop
L24:
   if (fNfcn - npfn >= fNfcnmx) goto L190;
   gdel = 0;
   gssq = 0;
   for (i = 1; i <= fNpar; ++i) {
      ri = 0;
      gssq += fMIGRgs[i-1]*fMIGRgs[i-1];
      for (j = 1; j <= fNpar; ++j) {
         m    = std::max(i,j);
         n    = std::min(i,j);
         ndex = m*(m-1) / 2 + n;
         ri  += fVhmat[ndex-1]*fMIGRgs[j-1];
      }
      fMIGRstep[i-1] = ri*-.5;
      gdel += fMIGRstep[i-1]*fMIGRgs[i-1];
   }
   if (gssq == 0) {
      mnwarn("D", "MIGRAD", " FIRST DERIVATIVES OF FCN ARE ALL ZERO");
      goto L300;
   }
//*-*-                if gdel positive, V not posdef
   if (gdel >= 0) {
      mnwarn("D", "MIGRAD", " NEWTON STEP NOT DESCENT.");
      if (npsdf == 1) goto L1;
      mnpsdf();
      npsdf = 1;
      goto L24;
   }
//*-*-                                       . . . . do line search
   mnline(fMIGRxxs, fs, fMIGRstep, gdel, toler);
   if (fAmin == fs) goto L200;
   fCfrom  = "MIGRAD  ";
   fNfcnfr = nfcnmg;
   fCstatu = "PROGRESS  ";
//*-*-                                       . get gradient at new point
   mninex(fX);
   if (fISW[2] == 1) {
      Eval(nparx, fGin, fzero, fU, 2);        ++fNfcn;
   }
   mnderi();
//*-*-                                        . calculate new EDM
   npsdf = 0;
L81:
   fEDM = 0;
   gvg = 0;
   delgam = 0;
   gdgssq = 0;
   for (i = 1; i <= fNpar; ++i) {
      ri  = 0;
      vgi = 0;
      for (j = 1; j <= fNpar; ++j) {
         m    = std::max(i,j);
         n    = std::min(i,j);
         ndex = m*(m-1) / 2 + n;
         vgi += fVhmat[ndex-1]*(fGrd[j-1] - fMIGRgs[j-1]);
         ri  += fVhmat[ndex-1]*fGrd[j-1];
      }
      fMIGRvg[i-1] = vgi*.5;
      gami    = fGrd[i-1] - fMIGRgs[i-1];
      gdgssq += gami*gami;
      gvg    += gami*fMIGRvg[i-1];
      delgam += fDirin[i-1]*gami;
      fEDM   += fGrd[i-1]*ri*.5;
   }
   fEDM = fEDM*.5*(fDcovar*3 + 1);
//*-*-                         . if EDM negative,  not positive-definite
   if (fEDM < 0 || gvg <= 0) {
      mnwarn("D", "MIGRAD", "NOT POS-DEF. EDM OR GVG NEGATIVE.");
      fCstatu = "NOT POSDEF";
      if (npsdf == 1) goto L230;
      mnpsdf();
      npsdf = 1;
      goto L81;
   }
//*-*-                           print information about this iteration
   ++iter;
   if (iswtr >= 3 || (iswtr == 2 && iter % 10 == 1)) {
      mnwerr();
      mnprin(3, fAmin);
   }
   if (gdgssq == 0) {
      mnwarn("D", "MIGRAD", "NO CHANGE IN FIRST DERIVATIVES OVER LAST STEP");
   }
   if (delgam < 0) {
      mnwarn("D", "MIGRAD", "FIRST DERIVATIVES INCREASING ALONG SEARCH LINE");
   }
//*-*-                                       .  update covariance matrix
   fCstatu = "IMPROVEMNT";
   if (ldebug) {
      log_info(" VHMAT 1 =");
      for (kk = 1; kk <= 10; ++kk) {
         log_info(" %10.2g",fVhmat[kk-1]);
      }
   }
   dsum = 0;
   vsum = 0;
   for (i = 1; i <= fNpar; ++i) {
      for (j = 1; j <= i; ++j) {
         if(delgam == 0 || gvg == 0) d = 0;
         else d = fDirin[i-1]*fDirin[j-1] / delgam - fMIGRvg[i-1]*fMIGRvg[j-1] / gvg;
         dsum += std::abs(d);
         ndex  = i*(i-1) / 2 + j;
         fVhmat[ndex-1] += d*2;
         vsum += std::abs(fVhmat[ndex-1]);
      }
   }
//*-*-               smooth local fluctuations by averaging DCOVAR
   fDcovar = (fDcovar + dsum / vsum)*.5;
   if (iswtr >= 3 || ldebug) {
      log_info(" RELATIVE CHANGE IN COV. MATRIX=%5.1f per cent",fDcovar*100);
   }
   if (ldebug) {
      log_info(" VHMAT 2 =");
      for (kk = 1; kk <= 10; ++kk) {
         log_info(" %10.3g",fVhmat[kk-1]);
      }
   }
   if (delgam <= gvg) goto L135;
   for (i = 1; i <= fNpar; ++i) {
      fMIGRflnu[i-1] = fDirin[i-1] / delgam - fMIGRvg[i-1] / gvg;
   }
   for (i = 1; i <= fNpar; ++i) {
      for (j = 1; j <= i; ++j) {
         ndex = i*(i-1) / 2 + j;
         fVhmat[ndex-1] += gvg*2*fMIGRflnu[i-1]*fMIGRflnu[j-1];
      }
   }
L135:
//*-*-                                             and see if converged
   if (fEDM < rhotol*.1) goto L300;
//*-*-                                   if not, prepare next iteration
   for (i = 1; i <= fNpar; ++i) {
      fMIGRxxs[i-1] = fX[i-1];
      fMIGRgs[i-1]  = fGrd[i-1];
   }
   fs = fAmin;
   if (fISW[1] == 0 && fDcovar < .5)  fISW[1] = 1;
   if (fISW[1] == 3 && fDcovar > .1)  fISW[1] = 1;
   if (fISW[1] == 1 && fDcovar < .05) fISW[1] = 3;
   goto L24;
//*-*-                                       . . . . .  end main loop
//*-*-                                        . . call limit in MNMIGR
L190:
   fISW[0] = 1;
   if (fISW[4] >= 0) {
      log_info(" CALL LIMIT EXCEEDED IN MIGRAD.");
   }
   fCstatu = "CALL LIMIT";
   goto L230;
//*-*-                                        . . fails to improve . .
L200:
   if (iswtr >= 1) {
      log_info(" MIGRAD FAILS TO FIND IMPROVEMENT");
   }
   for (i = 1; i <= fNpar; ++i) { fX[i-1] = fMIGRxxs[i-1]; }
   if (fEDM < rhotol) goto L300;
   if (fEDM < std::abs(fEpsma2*fAmin)) {
      if (iswtr >= 0) {
         log_info(" MACHINE ACCURACY LIMITS FURTHER IMPROVEMENT.");
      }
      goto L300;
   }
   if (fIstrat < 1) {
      if (fISW[4] >= 0) {
         log_info(" MIGRAD FAILS WITH STRATEGY=0.   WILL TRY WITH STRATEGY=1.");
      }
      fIstrat = 1;
   }
   goto L1;
//*-*-                                        . . fails to converge
L230:
   if (iswtr >= 0) {
      log_info(" MIGRAD TERMINATED WITHOUT CONVERGENCE.");
   }
   if (fISW[1] == 3) fISW[1] = 1;
   fISW[3] = -1;
   goto L400;
//*-*-                                        . . apparent convergence
L300:
   if (iswtr >= 0) {
      log_info(" MIGRAD MINIMIZATION HAS CONVERGED.");
   }
   if (fItaur == 0) {
      if (fIstrat >= 2 || (fIstrat == 1 && fISW[1] < 3)) {
         if (fISW[4] >= 0) {
            log_info(" MIGRAD WILL VERIFY CONVERGENCE AND ERROR MATRIX.");
         }
         mnhess();
         mnwerr();
         npsdf = 0;
         if (fEDM > rhotol) goto L10;
      }
   }
   fCstatu = "CONVERGED ";
   fISW[3] = 1;
//*-*-                                          come here in any case
L400:
   fCfrom  = "MIGRAD  ";
   fNfcnfr = nfcnmg;
   mninex(fX);
   mnwerr();
   if (iswtr >= 0) mnprin(3, fAmin);
   if (iswtr >= 1) mnmatu(1);
} /* mnmigr_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnmnos()
{
//*-*-*-*-*-*-*-*-*-*-*Performs a MINOS error analysis*-*-*-*-*-*-*-*-*-*-*-*
//*-*                  ===============================
//*-*        Performs a MINOS error analysis on those parameters for
//*-*        which it is requested on the MINOS command by calling
//*-*        MNMNOT for each parameter requested.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double val2mi, val2pl;
   int32_t nbad, ilax, ilax2, ngood, nfcnmi, iin, knt;

   if (fNpar <= 0) goto L700;
   ngood = 0;
   nbad = 0;
   nfcnmi = fNfcn;
//*-*-                                     . loop over parameters requested
   for (knt = 1; knt <= fNpar; ++knt) {
      if (int32_t(fWord7[1]) == 0) {
         ilax = fNexofi[knt-1];
      } else {
         if (knt >= 7) break;
         ilax = int32_t(fWord7[knt]);
         if (ilax == 0) break;
         if (ilax > 0 && ilax <= fNu) {
            if (fNiofex[ilax-1] > 0) goto L565;
         }
         log_info(" PARAMETER NUMBER %3d NOT A VARIABLE. IGNORED.",ilax);
         continue;
      }
L565:
//*-*-                                        calculate one pair of M E s
      ilax2 = 0;
      mnmnot(ilax, ilax2, val2pl, val2mi);
      if (fLnewmn) goto L650;
//*-*-                                         update NGOOD and NBAD
      iin = fNiofex[ilax-1];
      if (fErp[iin-1] > 0) ++ngood;
      else                   ++nbad;
      if (fErn[iin-1] < 0) ++ngood;
      else                   ++nbad;
   }
//*-*-                                          end of loop . . . . . . .
//*-*-                                       . . . . printout final values .
   fCfrom  = "MINOS   ";
   fNfcnfr = nfcnmi;
   fCstatu = "UNCHANGED ";
   if (ngood == 0 && nbad == 0) goto L700;
   if (ngood > 0 && nbad == 0)  fCstatu = "SUCCESSFUL";
   if (ngood == 0 && nbad > 0)  fCstatu = "FAILURE   ";
   if (ngood > 0 && nbad > 0)   fCstatu = "PROBLEMS  ";
   if (fISW[4] >= 0)    mnprin(4, fAmin);
   if (fISW[4] >= 2)    mnmatu(0);
   return;
//*-*-                                       . . . new minimum found . . . .
L650:
   fCfrom  = "MINOS   ";
   fNfcnfr = nfcnmi;
   fCstatu = "NEW MINIMU";
   if (fISW[4] >= 0) mnprin(4, fAmin);
   log_info(" NEW MINIMUM FOUND.  GO BACK TO MINIMIZATION STEP.");
   log_info(" =================================================");
   log_info("                                                  V");
   log_info("                                                  V");
   log_info("                                                  V");
   log_info("                                               VVVVVVV");
   log_info("                                                VVVVV");
   log_info("                                                 VVV");
   log_info("                                                  V\n");
   return;
L700:
   log_info(" THERE ARE NO MINOS ERRORS TO CALCULATE.");
} /* mnmnos_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnmnot(int32_t ilax, int32_t ilax2, double &val2pl, double &val2mi)
{
//*-*-*-*-*-*Performs a MINOS error analysis on one parameter*-*-*-*-*-*-*-*-*
//*-*        ================================================
//*-*        The parameter ILAX is varied, and the minimum of the
//*-*        function with respect to the other parameters is followed
//*-*        until it crosses the value FMIN+UP.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* System generated locals */
   int32_t i__1;

   /* Local variables */
   double delu, aopt, eros;
   double abest, xunit, dc, ut, sigsav, du1;
   double fac, sig, sav;
   int32_t marc, isig, mpar, ndex, imax, indx, ierr, i, j;
   int32_t iercr, it, istrav, nfmxin, nlimit, isw2, isw4;
   std::string csig;

//*-*-                                       . . save and prepare start vals
   isw2    = fISW[1];
   isw4    = fISW[3];
   sigsav  = fEDM;
   istrav  = fIstrat;
   dc      = fDcovar;
   fLnewmn = false;
   fApsi   = fEpsi*.5;
   abest   = fAmin;
   mpar    = fNpar;
   nfmxin  = fNfcnmx;
   for (i = 1; i <= mpar; ++i) { fXt[i-1] = fX[i-1]; }
   i__1 = mpar*(mpar + 1) / 2;
   for (j = 1; j <= i__1; ++j) { fVthmat[j-1] = fVhmat[j-1]; }
   for (i = 1; i <= mpar; ++i) {
      fMNOTgcc[i-1] = fGlobcc[i-1];
      fMNOTw[i-1]   = fWerr[i-1];
   }
   it = fNiofex[ilax-1];
   fErp[it-1] = 0;
   fErn[it-1] = 0;
   mninex(fXt);
   ut = fU[ilax-1];
   if (fNvarl[ilax-1] == 1) {
      fAlim[ilax-1] = ut - fMNOTw[it-1]*100;
      fBlim[ilax-1] = ut + fMNOTw[it-1]*100;
   }
   ndex  = it*(it + 1) / 2;
   xunit = std::sqrt(fUp / fVthmat[ndex-1]);
   marc  = 0;
   for (i = 1; i <= mpar; ++i) {
      if (i == it) continue;
      ++marc;
      imax = std::max(it,i);
      indx = imax*(imax-1) / 2 + std::min(it,i);
      fMNOTxdev[marc-1] = xunit*fVthmat[indx-1];
   }
//*-*-                          fix the parameter in question
   mnfixp(it-1, ierr);
   if (ierr > 0) {
      log_info(" MINUIT ERROR. CANNOT FIX PARAMETER %4d   INTERNAL %3d",ilax,it);
      goto L700;
   }
//*-*-                      . . . . . Nota Bene: from here on, NPAR=MPAR-1
//*-*-     Remember: MNFIXP squeezes IT out of X, XT, WERR, and VHMAT,
//*-*-                                                   not W, VTHMAT
   for (isig = 1; isig <= 2; ++isig) {
      if (isig == 1) {
         sig  = 1;
         csig = "POSI";
      } else {
         sig  = -1;
         csig = "NEGA";
      }
//*-*-                                       . sig=sign of error being calcd
      if (fISW[4] > 1) {
         log_info(" DETERMINATION OF %sTIVE MINOS ERROR FOR PARAMETER %d %s"
                            ,csig.c_str(),ilax
                            ,fCpnam[ilax-1].c_str());
      }
      if (fISW[1] <= 0) {
            mnwarn("D", "MINOS", "NO COVARIANCE MATRIX.");
      }
      nlimit     = fNfcn + nfmxin;
      fIstrat    = std::max(istrav-1,0);
      du1        = fMNOTw[it-1];
      fU[ilax-1] = ut + sig*du1;
      fU[ilax-1] = std::min(fU[ilax-1],fBlim[ilax-1]);
      fU[ilax-1] = std::max(fU[ilax-1],fAlim[ilax-1]);
      delu = fU[ilax-1] - ut;
//*-*-        stop if already at limit with negligible step size
      if (std::abs(delu) / (std::abs(ut) + std::abs(fU[ilax-1])) < fEpsmac) goto L440;
      fac = delu / fMNOTw[it-1];
      for (i = 1; i <= fNpar; ++i) {
         fX[i-1] = fXt[i-1] + fac*fMNOTxdev[i-1];
      }
      if (fISW[4] > 1) {
         log_info(" PARAMETER %4d SET TO%11.3e + %10.3e = %12.3e",ilax,ut,delu,fU[ilax-1]);
      }
//*-*-                                       loop to hit AMIN+UP
      fKe1cr  = ilax;
      fKe2cr  = 0;
      fXmidcr = fU[ilax-1];
      fXdircr = delu;

      fAmin = abest;
      fNfcnmx = nlimit - fNfcn;
      mncros(aopt, iercr);
      if (abest - fAmin > fUp*.01) goto L650;
      if (iercr == 1) goto L440;
      if (iercr == 2) goto L450;
      if (iercr == 3) goto L460;
//*-*-                                       . error successfully calculated
      eros = fXmidcr - ut + aopt*fXdircr;
      if (fISW[4] > 1) {
         log_info("        THE %4sTIVE MINOS ERROR OF PARAMETER %3d  %10s, IS %12.4e"
                           ,csig.c_str(),ilax
                           ,fCpnam[ilax-1].c_str(),eros);
      }
      goto L480;
//*-*-                                       . . . . . . . . failure returns
L440:
      if (fISW[4] >= 1) {
         log_info("    THE %4sTIVE MINOS ERROR OF PARAMETER %3d, %s EXCEEDS ITS LIMIT."
                              ,csig.c_str(),ilax
                              ,fCpnam[ilax-1].c_str());
      }
      eros = fUndefi;
      goto L480;
L450:
      if (fISW[4] >= 1) {
         log_info("       THE %4sTIVE MINOS ERROR %4d REQUIRES MORE THAN %5d FUNCTION CALLS."
                         ,csig.c_str(),ilax,nfmxin);
      }
      eros = 0;
      goto L480;
L460:
      if (fISW[4] >= 1) {
         log_info("                         %4sTIVE MINOS ERROR NOT CALCULATED FOR PARAMETER %d"
                         ,csig.c_str(),ilax);
      }
      eros = 0;

L480:
      if (fISW[4] > 1) {
         log_info("     **************************************************************************");
      }
      if (sig < 0) {
         fErn[it-1] = eros;
         if (ilax2 > 0 && ilax2 <= fNu) val2mi = fU[ilax2-1];
      } else {
         fErp[it-1] = eros;
         if (ilax2 > 0 && ilax2 <= fNu) val2pl = fU[ilax2-1];
      }
   }
//*-*-                                       . . parameter finished. reset v
//*-*-                      normal termination */
   fItaur = 1;
   mnfree(1);
   i__1 = mpar*(mpar + 1) / 2;
   for (j = 1; j <= i__1; ++j) { fVhmat[j-1] = fVthmat[j-1]; }
   for (i = 1; i <= mpar; ++i) {
      fWerr[i-1]   = fMNOTw[i-1];
      fGlobcc[i-1] = fMNOTgcc[i-1];
      fX[i-1]      = fXt[i-1];
   }
   mninex(fX);
   fEDM    = sigsav;
   fAmin   = abest;
   fISW[1] = isw2;
   fISW[3] = isw4;
   fDcovar = dc;
   goto L700;
//*-*-                      new minimum
L650:
   fLnewmn = true;
   fISW[1] = 0;
   fDcovar = 1;
   fISW[3] = 0;
   sav     = fU[ilax-1];
   fItaur  = 1;
   mnfree(1);
   fU[ilax-1] = sav;
   mnexin(fX);
   fEDM = fBigedm;
//*-*-                      in any case
L700:
   fItaur  = 0;
   fNfcnmx = nfmxin;
   fIstrat = istrav;
} /* mnmnot_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnparm(int32_t k1, std::string cnamj, double uk, double wk, double a, double b, int32_t &ierflg)
{
//*-*-*-*-*-*-*-*-*Implements one parameter definition*-*-*-*-*-*-*-*-*-*-*-*
//*-*              ===================================
//*-*        Called from MNPARS and user-callable
//*-*    Implements one parameter definition, that is:
//*-*          K     (external) parameter number
//*-*          CNAMK parameter name
//*-*          UK    starting value
//*-*          WK    starting step size or uncertainty
//*-*          A, B  lower and upper physical parameter limits
//*-*    and sets up (updates) the parameter lists.
//*-*    Output: IERFLG=0 if no problems
//*-*                  >0 if MNPARM unable to implement definition
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double vplu, a_small, gsmin, pinti, vminu, danger, sav, sav2;
   int32_t ierr, kint, in, ix, ktofix, lastin, kinfix, nvl;
   std::string cnamk, chbufi;

   int32_t k = k1+1;
   cnamk   = cnamj;
   kint    = fNpar;
   if (k < 1 || k > fMaxext) {
//*-*-                    parameter number exceeds allowed maximum value
      log_info(" MINUIT USER ERROR.  PARAMETER NUMBER IS %3d  ALLOWED RANGE IS ONE TO %4d",k,fMaxext);
      goto L800;
   }
//*-*-                    normal parameter request
   ktofix = 0;
   if (fNvarl[k-1] < 0) goto L50;
//*-*-        previously defined parameter is being redefined
//*-*-                                    find if parameter was fixed
   for (ix = 1; ix <= fNpfix; ++ix) {
      if (fIpfix[ix-1] == k) ktofix = k;
   }
   if (ktofix > 0) {
      mnwarn("W", "PARAM DEF", "REDEFINING A FIXED PARAMETER.");
      if (kint >= fMaxint) {
         log_info(" CANNOT RELEASE. MAX NPAR EXCEEDED.");
         goto L800;
      }
      mnfree(-k);
   }
//*-*-                      if redefining previously variable parameter
   if (fNiofex[k-1] > 0) kint = fNpar - 1;
L50:

//*-*-                                     . . .print heading
   if (fLphead && fISW[4] >= 0) {
      log_info(" PARAMETER DEFINITIONS:");
      log_info("    NO.   NAME         VALUE      STEP SIZE      LIMITS");
      fLphead = false;
   }
   if (wk > 0) goto L122;
//*-*-                                       . . .constant parameter . . . .
   if (fISW[4] >= 0) {
      log_info(" %5d %-10s %13.5e  constant",k,cnamk.c_str(),uk);
   }
   nvl = 0;
   goto L200;
L122:
   if (a == 0 && b == 0) {
//*-*-                                     variable parameter without limits
      nvl = 1;
      if (fISW[4] >= 0) {
         log_info(" %5d %-10s %13.5e%13.5e     no limits",k,cnamk.c_str(),uk,wk);
      }
   } else {
//*-*-                                        variable parameter with limits
      nvl = 4;
      fLnolim = false;
      if (fISW[4] >= 0) {
         log_info(" %5d %-10s %13.5e%13.5e  %13.5e%13.5e",k,cnamk.c_str(),uk,wk,a,b);
      }
   }
//*-*-                            . . request for another variable parameter
   ++kint;
   if (kint > fMaxint) {
      log_info(" MINUIT USER ERROR.   TOO MANY VARIABLE PARAMETERS.");
      goto L800;
   }
   if (nvl == 1) goto L200;
   if (a == b) {
      log_info(" USER ERROR IN MINUIT PARAMETER");
      log_info(" DEFINITION");
      log_info(" UPPER AND LOWER LIMITS EQUAL.");
      goto L800;
   }
   if (b < a) {
      sav = b;
      b = a;
      a = sav;
      mnwarn("W", "PARAM DEF", "PARAMETER LIMITS WERE REVERSED.");
      if (fLwarn) fLphead = true;
   }
   if (b - a > 1e7) {
      mnwarn("W", "PARAM DEF", (boost::format("LIMITS ON PARAM%d TOO FAR APART.") % k).str().c_str());
      if (fLwarn) fLphead = true;
   }
   danger = (b - uk)*(uk - a);
   if (danger < 0) {
      mnwarn("W", "PARAM DEF", "STARTING VALUE OUTSIDE LIMITS.");
   }
   if (danger == 0) {
      mnwarn("W", "PARAM DEF", "STARTING VALUE IS AT LIMIT.");
   }
L200:
//*-*-                          . . . input OK, set values, arrange lists,
//*-*-                                   calculate step sizes GSTEP, DIRIN
   fCfrom      = "PARAMETR";
   fNfcnfr     = fNfcn;
   fCstatu     = "NEW VALUES";
   fNu         = std::max(fNu,k);
   fCpnam[k-1] = cnamk;
   fU[k-1]     = uk;
   fAlim[k-1]  = a;
   fBlim[k-1]  = b;
   fNvarl[k-1] = nvl;
   mnrset(1);
//*-*-                            K is external number of new parameter
//*-*-          LASTIN is the number of var. params with ext. param. no.< K
   lastin = 0;
   for (ix = 1; ix <= k-1; ++ix) { if (fNiofex[ix-1] > 0) ++lastin; }
//*-*-                KINT is new number of variable params, NPAR is old
   if (kint == fNpar) goto L280;
   if (kint > fNpar) {
//*-*-                         insert new variable parameter in list
      for (in = fNpar; in >= lastin + 1; --in) {
         ix            = fNexofi[in-1];
         fNiofex[ix-1] = in + 1;
         fNexofi[in]   = ix;
         fX[in]        = fX[in-1];
         fXt[in]       = fXt[in-1];
         fDirin[in]    = fDirin[in-1];
         fG2[in]       = fG2[in-1];
         fGstep[in]    = fGstep[in-1];
         fWerr[in]     = fWerr[in-1];
         fGrd[in]      = fGrd[in-1];
      }
   } else {
//*-*-                         remove variable parameter from list
      for (in = lastin + 1; in <= kint; ++in) {
         ix            = fNexofi[in];
         fNiofex[ix-1] = in;
         fNexofi[in-1] = ix;
         fX[in-1]      = fX[in];
         fXt[in-1]     = fXt[in];
         fDirin[in-1]  = fDirin[in];
         fG2[in-1]     = fG2[in];
         fGstep[in-1]  = fGstep[in];
         fWerr[in-1]   = fWerr[in];
         fGrd[in-1]    = fGrd[in];
      }
   }
L280:
   ix = k;
   fNiofex[ix-1] = 0;
   fNpar = kint;
//*-*-                                      lists are now arranged . . . .
   if (nvl > 0) {
      in            = lastin + 1;
      fNexofi[in-1] = ix;
      fNiofex[ix-1] = in;
      sav           = fU[ix-1];
      mnpint(sav, ix-1, pinti);
      fX[in-1]    = pinti;
      fXt[in-1]   = fX[in-1];
      fWerr[in-1] = wk;
      sav2        = sav + wk;
      mnpint(sav2, ix-1, pinti);
      vplu = pinti - fX[in-1];
      sav2 = sav - wk;
      mnpint(sav2, ix-1, pinti);
      vminu = pinti - fX[in-1];
      fDirin[in-1] = (std::abs(vplu) + std::abs(vminu))*.5;
      fG2[in-1] = fUp*2 / (fDirin[in-1]*fDirin[in-1]);
      gsmin = fEpsma2*8*std::abs(fX[in-1]);
      fGstep[in-1] = std::max(gsmin,fDirin[in-1]*.1);
      if (fAmin != fUndefi) {
         a_small      = std::sqrt(fEpsma2*(fAmin + fUp) / fUp);
         fGstep[in-1] = std::max(gsmin,a_small*fDirin[in-1]);
      }
      fGrd[in-1] = fG2[in-1]*fDirin[in-1];
//*-*-                  if parameter has limits
      if (fNvarl[k-1] > 1) {
         if (fGstep[in-1] > .5) fGstep[in-1] = .5;
         fGstep[in-1] = -fGstep[in-1];
      }
   }
   if (ktofix > 0) {
      ierr = 0;
      kinfix = fNiofex[ktofix-1];
      if (kinfix > 0) mnfixp(kinfix-1, ierr);
      if (ierr > 0)   goto L800;
   }
   ierflg = 0;
   return;
//*-*-                  error on input, unable to implement request  . . . .
L800:
   ierflg = 1;
} /* mnparm_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnpars(std::string &crdbuf, int32_t &icondn)
{
//*-*-*-*-*-*-*-*Implements one parameter definition*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*            =========== =======================
//*-*        Called from MNREAD and user-callable
//*-*    Implements one parameter definition, that is:
//*-*       parses the string CRDBUF and calls MNPARM
//*-*
//*-* output conditions:
//*-*        ICONDN = 0    all OK
//*-*        ICONDN = 1    error, attempt to define parameter is ignored
//*-*        ICONDN = 2    end of parameter definitions
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double a=0, b=0, fk=0, uk=0, wk=0, xk=0;
   int32_t ierr, kapo1, kapo2;
   int32_t k, llist, ibegin, lenbuf, istart, lnc, icy;
   std::string cnamk, comand, celmnt, ctemp;
   char stmp[128];

   lenbuf = strlen(crdbuf.c_str());
//*-*-                    find out whether fixed or free-field format
   kapo1 = strspn(crdbuf.c_str(), "'");
   if (kapo1 == 0) goto L150;
   kapo2 = strspn((crdbuf.c_str()) + kapo1, "'");
   if (kapo2 == 0) goto L150;
//*-*-         new (free-field) format
   kapo2 += kapo1;
//*-*-                            skip leading blanks if any
   for (istart = 1; istart <= kapo1-1; ++istart) {
      if (crdbuf[istart-1] != ' ') goto L120;
   }
   goto L210;
L120:
//*-*-                              parameter number integer
   celmnt = crdbuf.substr(istart-1, kapo1-istart);
   if (scanf(celmnt.c_str(),&fk)) {;}
   k = int32_t(fk);
   if (k <= 0) goto L210;
   cnamk = "PARAM " + celmnt;
   if (kapo2 - kapo1 > 1) {
      cnamk = crdbuf.substr(kapo1, kapo2-1-kapo1);
   }
//*-*  special handling if comma or blanks and a comma follow 'name'
   for (icy = kapo2 + 1; icy <= lenbuf; ++icy) {
      if (crdbuf[icy-1] == ',') goto L139;
      if (crdbuf[icy-1] != ' ') goto L140;
   }
   uk = 0;
   wk = 0;
   a  = 0;
   b  = 0;
   goto L170;
L139:
   ++icy;
L140:
   ibegin = icy;
   ctemp = crdbuf.substr(ibegin-1,lenbuf-ibegin);
   mncrck(ctemp, 20, comand, lnc, fMaxpar, fPARSplist, llist, ierr, fIsyswr);
   if (ierr > 0) goto L180;
   uk = fPARSplist[0];
   wk = 0;
   if (llist >= 2) wk = fPARSplist[1];
   a = 0;
   if (llist >= 3) a = fPARSplist[2];
   b = 0;
   if (llist >= 4) b = fPARSplist[3];
   goto L170;
//*-*-         old (fixed-field) format
L150:
   if (scanf(crdbuf.c_str(),&xk,stmp,&uk,&wk,&a,&b)) {;}
   cnamk = stmp;
   k = int32_t(xk);
   if (k == 0)    goto L210;
//*-*-         parameter format cracked, implement parameter definition
L170:
   mnparm(k-1, cnamk, uk, wk, a, b, ierr);
   icondn = ierr;
   return;
//*-*-         format or other error
L180:
   icondn = 1;
   return;
//*-*-       end of data
L210:
   icondn = 2;
} /* mnpars_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnpfit(double *parx2p, double *pary2p, int32_t npar2p, double *coef2p, double &sdev2p)
{
//*-*-*-*-*-*-*-*-*-*To fit a parabola to npar2p points*-*-*-*-*-*-*-*-*-*-*
//*-*                ==================================
//*-*   npar2p   no. of points
//*-*   parx2p(i)   x value of point i
//*-*   pary2p(i)   y value of point i
//*-*
//*-*   coef2p(1...3)  coefficients of the fitted parabola
//*-*   y=coef2p(1) + coef2p(2)*x + coef2p(3)*x**2
//*-*   sdev2p= variance
//*-*   method : chi**2 = min equation solved explicitly
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double a, f, s, t, y, s2, x2, x3, x4, y2, cz[3], xm, xy, x2y;
   x2 = x3 = 0;
   int32_t i;

   /* Parameter adjustments */
   --coef2p;
   --pary2p;
   --parx2p;

   /* Function Body */
   for (i = 1; i <= 3; ++i) { cz[i-1] = 0; }
   sdev2p = 0;
   if (npar2p < 3) goto L10;
   f = (double) (npar2p);
//*-* --- center x values for reasons of machine precision
   xm  = 0;
   for (i = 1; i <= npar2p; ++i) { xm += parx2p[i]; }
   xm /= f;
   x2  = 0;
   x3  = 0;
   x4  = 0;
   y   = 0;
   y2  = 0;
   xy  = 0;
   x2y = 0;
   for (i = 1; i <= npar2p; ++i) {
      s    = parx2p[i] - xm;
      t    = pary2p[i];
      s2   = s*s;
      x2  += s2;
      x3  += s*s2;
      x4  += s2*s2;
      y   += t;
      y2  += t*t;
      xy  += s*t;
      x2y += s2*t;
   }
   a = (f*x4 - x2*x2)*x2 - f*(x3*x3);
   if (a == 0) goto L10;
   cz[2] = (x2*(f*x2y - x2*y) - f*x3*xy) / a;
   cz[1] = (xy - x3*cz[2]) / x2;
   cz[0] = (y - x2*cz[2]) / f;
   if (npar2p == 3) goto L6;
   sdev2p = y2 - (cz[0]*y + cz[1]*xy + cz[2]*x2y);
   if (sdev2p < 0) sdev2p = 0;
   sdev2p /= f - 3;
L6:
   cz[0] += xm*(xm*cz[2] - cz[1]);
   cz[1] -= xm*2*cz[2];
L10:
   for (i = 1; i <= 3; ++i) { coef2p[i] = cz[i-1]; }
} /* mnpfit_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnpint(double &pexti, int32_t i1, double &pinti)
{
//*-*-*-*-*-*-*Calculates the internal parameter value PINTI*-*-*-*-*-*-*-*
//*-*          =============================================
//*-*        corresponding  to the external value PEXTI for parameter I.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double a, alimi, blimi, yy, yy2;
   int32_t igo;
   std::string chbuf2, chbufi;

   int32_t i = i1+1;
   pinti   = pexti;
   igo     = fNvarl[i-1];
   if (igo == 4) {
//*-* --                          there are two limits
      alimi = fAlim[i-1];
      blimi = fBlim[i-1];
      yy = (pexti - alimi)*2 / (blimi - alimi) - 1;
      yy2 = yy*yy;
      if (yy2 >= 1 - fEpsma2) {
         if (yy < 0) {
            a      = fVlimlo;
            chbuf2 = " IS AT ITS LOWER ALLOWED LIMIT.";
         } else {
            a      = fVlimhi;
            chbuf2 = " IS AT ITS UPPER ALLOWED LIMIT.";
         }
         pinti   = a;
         pexti   = alimi + (blimi - alimi)*.5*(std::sin(a) + 1);
         fLimset = true;
         if (yy2 > 1) chbuf2 = " BROUGHT BACK INSIDE LIMITS.";
         mnwarn("W", fCfrom.c_str(), (boost::format("VARIABLE%d%s") % i % chbuf2.c_str()).str().c_str());
      } else {
         pinti = std::asin(yy);
      }
   }
} /* mnpint_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnpout(int32_t iuext1, std::string &chnam, double &val, double &err, double &xlolim, double &xuplim, int32_t &iuint) const
{
//*-*-*-*Provides the user with information concerning the current status*-*-*
//*-*    ================================================================
//*-*          of parameter number IUEXT. Namely, it returns:
//*-*        CHNAM: the name of the parameter
//*-*        VAL: the current (external) value of the parameter
//*-*        ERR: the current estimate of the parameter uncertainty
//*-*        XLOLIM: the lower bound (or zero if no limits)
//*-*        XUPLIM: the upper bound (or zero if no limits)
//*-*        IUINT: the internal parameter number (or zero if not variable,
//*-*           or negative if undefined).
//*-*  Note also:  If IUEXT is negative, then it is -internal parameter
//*-*           number, and IUINT is returned as the EXTERNAL number.
//*-*     Except for IUINT, this is exactly the inverse of MNPARM
//*-*     User-called
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   int32_t iint, iext, nvl;

   int32_t iuext = iuext1 + 1;
   xlolim = 0;
   xuplim = 0;
   err    = 0;
   if (iuext == 0) goto L100;
   if (iuext < 0) {
//*-*-                  internal parameter number specified
      iint  = -(iuext);
      if (iint > fNpar) goto L100;
      iext  = fNexofi[iint-1];
      iuint = iext;
   } else {
//*-*-                   external parameter number specified
      iext = iuext;
      if (iext > fNu) goto L100;
      iint  = fNiofex[iext-1];
      iuint = iint;
   }
//*-*-                    in both cases
   nvl = fNvarl[iext-1];
   if (nvl < 0) goto L100;
   chnam = fCpnam[iext-1];
   val   = fU[iext-1];
   if (iint > 0) err = fWerr[iint-1];
   if (nvl == 4) {
      xlolim = fAlim[iext-1];
      xuplim = fBlim[iext-1];
   }
   return;
//*-*-               parameter is undefined
L100:
   iuint = -1;
   chnam = "undefined";
   val = 0;
} /* mnpout_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnprin(int32_t inkode, double fval)
{
//*-*-*-*Prints the values of the parameters at the time of the call*-*-*-*-*
//*-*    ===========================================================
//*-*        also prints other relevant information such as function value,
//*-*        estimated distance to minimum, parameter errors, step sizes.
//*-*
//*-*         According to the value of IKODE, the printout is:/
//*-*    IKODE=INKODE= 0    only info about function value
//*-*                  1    parameter values, errors, limits
//*-*                  2    values, errors, step sizes, internal values
//*-*                  3    values, errors, step sizes, first derivs.
//*-*                  4    values, parabolic errors, MINOS errors
//*-*    when INKODE=5, MNPRIN chooses IKODE=1,2, or 3, according to fISW[1]
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Initialized data */

   static std::string cblank = "           ";
   static std::string cnambf = "           ";

   /* Local variables */
   double dcmax, x1, x2, x3, dc;
   x2 = x3 = 0;
   int32_t nadd, i, k, l, m, ikode, nc, ntrail, lbl;
   uint32_t ic;
   std::string chedm;
   std::string colhdl[6], colhdu[6], cx2, cx3, cheval;

   if (fNu == 0) {
      log_info(" THERE ARE CURRENTLY NO PARAMETERS DEFINED");
      return;
   }
//*-*-                 get value of IKODE based in INKODE, fISW[1]
   ikode = inkode;
   if (inkode == 5) {
      ikode = fISW[1] + 1;
      if (ikode > 3) ikode = 3;
   }
//*-*-                 set 'default' column headings
   for (k = 1; k <= 6; ++k) {
      colhdu[k-1] = "UNDEFINED";
      colhdl[k-1] = "COLUMN HEAD";
   }
//*-*-             print title if Minos errors, and title exists.
   if (ikode == 4 && fCtitl != fCundef) {
      log_info(" MINUIT TASK: %s",fCtitl.c_str());
   }
//*-*-             report function value and status
   if (fval == fUndefi) cheval = " unknown       ";
   else                 cheval = boost::str(boost::format("%g") % fval);

   if (fEDM == fBigedm) chedm = " unknown  ";
   else                 chedm = boost::str(boost::format("%g") % fEDM);

   nc = fNfcn - fNfcnfr;
   log_info(" FCN=%s FROM %8s  STATUS=%10s %6d CALLS   %9d TOTAL"
               ,cheval.c_str()
               ,fCfrom.c_str()
               ,fCstatu.c_str(),nc,fNfcn);
   m = fISW[1];
   if (m == 0 || m == 2 || fDcovar == 0) {
      log_info("                     EDM=%s    STRATEGY=%2d      %s"
                      ,chedm.c_str(),fIstrat
                      ,fCovmes[m].c_str());
   } else {
      dcmax = 1;
      dc    = std::min(fDcovar,dcmax)*100;
      log_info("                     EDM=%s    STRATEGY=%2d  ERROR MATRIX UNCERTAINTY %5.1f per cent"
                      ,chedm.c_str(),fIstrat,dc);
   }

   if (ikode == 0) return;
//*-*-              find longest name (for Rene!)
   ntrail = 10;
   for (i = 1; i <= fNu; ++i) {
      if (fNvarl[i-1] < 0) continue;
      for (ic = 10; ic >= 1; --ic) {
         if (fCpnam[i-1].size() >= ic && fCpnam[i-1].substr(ic-1,1) != " ") goto L16;
      }
      ic = 1;
L16:
      lbl = 10 - ic;
      if (lbl < ntrail) ntrail = lbl;
   }
   nadd = ntrail / 2 + 1;
   if (ikode == 1) {
      colhdu[0] = "              ";
      colhdl[0] = "      ERROR   ";
      colhdu[1] = "      PHYSICAL";
      colhdu[2] = " LIMITS       ";
      colhdl[1] = "    NEGATIVE  ";
      colhdl[2] = "    POSITIVE  ";
   }
   if (ikode == 2) {
      colhdu[0] = "              ";
      colhdl[0] = "      ERROR   ";
      colhdu[1] = "    INTERNAL  ";
      colhdl[1] = "    STEP SIZE ";
      colhdu[2] = "    INTERNAL  ";
      colhdl[2] = "      VALUE   ";
   }
   if (ikode == 3) {
      colhdu[0] = "              ";
      colhdl[0] = "      ERROR   ";
      colhdu[1] = "       STEP   ";
      colhdl[1] = "       SIZE   ";
      colhdu[2] = "      FIRST   ";
      colhdl[2] = "   DERIVATIVE ";
   }
   if (ikode == 4) {
      colhdu[0] = "    PARABOLIC ";
      colhdl[0] = "      ERROR   ";
      colhdu[1] = "        MINOS ";
      colhdu[2] = "ERRORS        ";
      colhdl[1] = "   NEGATIVE   ";
      colhdl[2] = "   POSITIVE   ";
   }

   if (ikode != 4) {
      if (fISW[1] < 3) colhdu[0] = "  APPROXIMATE ";
      if (fISW[1] < 1) colhdu[0] = " CURRENT GUESS";
   }
   log_info("  EXT PARAMETER              %-14s%-14s%-14s",colhdu[0].c_str()
                                                    ,colhdu[1].c_str()
                                                    ,colhdu[2].c_str());
   log_info("  NO.   NAME      VALUE      %-14s%-14s%-14s",colhdl[0].c_str()
                                                    ,colhdl[1].c_str()
                                                    ,colhdl[2].c_str());
//*-*-                                       . . . loop over parameters . .
   for (i = 1; i <= fNu; ++i) {
      if (fNvarl[i-1] < 0) continue;
      l = fNiofex[i-1];
      cnambf = cblank.substr(0,nadd) + fCpnam[i-1];
      if (l == 0) goto L55;
//*-*-             variable parameter.
      x1  = fWerr[l-1];
      cx2 = "PLEASE GET X..";
      cx3 = "PLEASE GET X..";
      if (ikode == 1) {
         if (fNvarl[i-1] <= 1) {
            log_info("%4d %-11s%14.5e%14.5e",i,cnambf.c_str(),fU[i-1],x1);
            continue;
         } else {
            x2 = fAlim[i-1];
            x3 = fBlim[i-1];
         }
      }
      if (ikode == 2) {
         x2 = fDirin[l-1];
         x3 = fX[l-1];
      }
      if (ikode == 3) {
         x2 = fDirin[l-1];
         x3 = fGrd[l-1];
         if (fNvarl[i-1] > 1 && std::abs(std::cos(fX[l-1])) < .001) {
            cx3 = "** at limit **";
         }
      }
      if (ikode == 4) {
         x2 = fErn[l-1];
         if (x2 == 0)        cx2 = " ";
         if (x2 == fUndefi)  cx2 = "   at limit   ";
         x3 = fErp[l-1];
         if (x3 == 0)        cx3 = " ";
         if (x3 == fUndefi)         cx3 = "   at limit   ";
      }
      if (cx2 == "PLEASE GET X..")  cx2 = boost::str(boost::format("%14.5e") % x2);
      if (cx3 == "PLEASE GET X..")  cx3 = boost::str(boost::format("%14.5e") % x3);
      log_info("%4d %-11s%14.5e%14.5e%-14s%-14s",i
                   ,cnambf.c_str(),fU[i-1],x1
                   ,cx2.c_str(),cx3.c_str());

//*-*-              check if parameter is at limit
      if (fNvarl[i-1] <= 1 || ikode == 3) continue;
      if (std::abs(std::cos(fX[l-1])) < .001) {
         log_info("                                 WARNING -   - ABOVE PARAMETER IS AT LIMIT.");
      }
      continue;

//*-*-                               print constant or fixed parameter.
L55:
      colhdu[0] = "   constant   ";
      if (fNvarl[i-1] > 0)  colhdu[0] = "     fixed    ";
      if (fNvarl[i-1] == 4 && ikode == 1) {
         log_info("%4d %-11s%14.5e%-14s%14.5e%14.5e",i
              ,cnambf.c_str(),fU[i-1]
              ,colhdu[0].c_str(),fAlim[i-1],fBlim[i-1]);
      } else {
         log_info("%4d %-11s%14.5e%s",i
                   ,cnambf.c_str(),fU[i-1],colhdu[0].c_str());
      }
   }

   if (fUp != fUpdflt) {
      log_info("                               ERR DEF= %g",fUp);
   }
   return;
} /* mnprin_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnpsdf()
{
//*-*-*-*-*-*Calculates the eigenvalues of v to see if positive-def*-*-*-*-*
//*-*        ======================================================
//*-*        if not, adds constant along diagonal to make positive.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double dgmin, padd, pmin, pmax, dg, epspdf, epsmin;
   int32_t ndex, i, j, ndexd, ip, ifault;
   std::string chbuff, ctemp;

   epsmin = 1e-6;
   epspdf = std::max(epsmin,fEpsma2);
   dgmin  = fVhmat[0];
//*-*-                       Check if negative or zero on diagonal
   for (i = 1; i <= fNpar; ++i) {
      ndex = i*(i + 1) / 2;
      if (fVhmat[ndex-1] <= 0) {
         mnwarn("W", fCfrom.c_str(), (boost::format("Negative diagonal element %d in Error Matrix") % i).str().c_str());
      }
      if (fVhmat[ndex-1] < dgmin) dgmin = fVhmat[ndex-1];
   }
   if (dgmin <= 0) {
      dg    = epspdf + 1 - dgmin;
      mnwarn("W", fCfrom.c_str(), (boost::format("%g added to diagonal of error matrix") % dg).str().c_str());
   } else {
      dg = 0;
   }
//*-*-                   Store VHMAT in P, make sure diagonal pos.
   for (i = 1; i <= fNpar; ++i) {
      ndex  = i*(i-1) / 2;
      ndexd = ndex + i;
      fVhmat[ndexd-1] += dg;
      if (fVhmat[ndexd-1]==0) {
         fPSDFs[i-1] = 1 / 1e-19; // a totally arbitrary silly small value
      } else {
         fPSDFs[i-1] = 1 / std::sqrt(fVhmat[ndexd-1]);
      }
      for (j = 1; j <= i; ++j) {
         ++ndex;
         fP[i + j*fMaxpar - fMaxpar-1] = fVhmat[ndex-1]*fPSDFs[i-1]*fPSDFs[j-1];
      }
   }
//*-*-     call eigen (p,p,maxint,npar,pstar,-npar)
   mneig(fP, fMaxint, fNpar, fMaxint, fPstar, epspdf, ifault);
   pmin = fPstar[0];
   pmax = fPstar[0];
   for (ip = 2; ip <= fNpar; ++ip) {
      if (fPstar[ip-1] < pmin) pmin = fPstar[ip-1];
      if (fPstar[ip-1] > pmax) pmax = fPstar[ip-1];
   }
   pmax = std::max(std::abs(pmax),double(1));
   if ((pmin <= 0 && fLwarn) || fISW[4] >= 2) {
      log_info(" EIGENVALUES OF SECOND-DERIVATIVE MATRIX:");
      ctemp = "       ";
      for (ip = 1; ip <= fNpar; ++ip) {
         ctemp += boost::str(boost::format(" %11.4e") % fPstar[ip-1]);
      }
      log_info("%s", ctemp.c_str());
   }
   if (pmin > epspdf*pmax) return;
   if (fISW[1] == 3) fISW[1] = 2;
   padd = pmax*.001 - pmin;
   for (ip = 1; ip <= fNpar; ++ip) {
      ndex = ip*(ip + 1) / 2;
      fVhmat[ndex-1] *= padd + 1;
   }
   fCstatu = "NOT POSDEF";
   mnwarn("W", fCfrom.c_str(), (boost::format("MATRIX FORCED POS-DEF BY ADDING %f TO DIAGONAL.") % padd).str().c_str());

} /* mnpsdf_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnrazz(double ynew, double *pnew, double *y, int32_t &jh, int32_t &jl)
{
//*-*-*-*-*Called only by MNSIMP (and MNIMPR) to add a new point*-*-*-*-*-*-*
//*-*      =====================================================
//*-*        and remove an old one from the current simplex, and get the
//*-*        estimated distance to minimum.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double pbig, plit;
   int32_t i, j, nparp1;

   /* Function Body */
   for (i = 1; i <= fNpar; ++i) { fP[i + jh*fMaxpar - fMaxpar-1] = pnew[i-1]; }
   y[jh-1] = ynew;
   if (ynew < fAmin) {
      for (i = 1; i <= fNpar; ++i) { fX[i-1] = pnew[i-1]; }
      mninex(fX);
      fAmin   = ynew;
      fCstatu = "PROGRESS  ";
      jl      = jh;
   }
   jh     = 1;
   nparp1 = fNpar + 1;
   for (j = 2; j <= nparp1; ++j) { if (y[j-1] > y[jh-1]) jh = j; }
   fEDM = y[jh-1] - y[jl-1];
   if (fEDM <= 0) goto L45;
   for (i = 1; i <= fNpar; ++i) {
      pbig = fP[i-1];
      plit = pbig;
      for (j = 2; j <= nparp1; ++j) {
         if (fP[i + j*fMaxpar - fMaxpar-1] > pbig) pbig = fP[i + j*fMaxpar - fMaxpar-1];
         if (fP[i + j*fMaxpar - fMaxpar-1] < plit) plit = fP[i + j*fMaxpar - fMaxpar-1];
      }
      fDirin[i-1] = pbig - plit;
   }
L40:
   return;
L45:
   log_info("  FUNCTION VALUE DOES NOT SEEM TO DEPEND ON ANY OF THE %d VARIABLE PARAMETERS.",fNpar);
   log_info("          VERIFY THAT STEP SIZES ARE BIG ENOUGH AND CHECK FCN LOGIC.");
   log_info(" *******************************************************************************");
   log_info(" *******************************************************************************");
   goto L40;
} /* mnrazz_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnrn15(double &val, int32_t &inseed)
{
//*-*-*-*-*-*-*This is a super-portable random number generator*-*-*-*-*-*-*
//*-*          ================================================
//*-*         It should not overflow on any 32-bit machine.
//*-*         The cycle is only ~10**9, so use with care!
//*-*         Note especially that VAL must not be undefined on input.
//*-*                    Set Default Starting Seed
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Initialized data */

   static int32_t iseed = 12345;

   int32_t k;

   if (val == 3) goto L100;
   inseed = iseed;
   k      = iseed / 53668;
   iseed  = (iseed - k*53668)*40014 - k*12211;
   if (iseed < 0) iseed += 2147483563;
   val = double(iseed*4.656613e-10);
   return;
//*-*               "entry" to set seed, flag is VAL=3
L100:
   iseed = inseed;
} /* mnrn15_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnrset(int32_t iopt)
{
//*-*-*-*-*-*-*-*Resets function value and errors to UNDEFINED*-*-*-*-*-*-*-*
//*-*            =============================================
//*-*    If IOPT=1,
//*-*    If IOPT=0, sets only MINOS errors to undefined
//*-*        Called from MNCLER and whenever problem changes, for example
//*-*        after SET LIMITS, SET PARAM, CALL FCN 6
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   int32_t iext, i;

   fCstatu = "RESET     ";
   if (iopt >= 1) {
      fAmin   = fUndefi;
      fFval3  = std::abs(fAmin)*2 + 1;
      fEDM    = fBigedm;
      fISW[3] = 0;
      fISW[1] = 0;
      fDcovar = 1;
      fISW[0] = 0;
   }
   fLnolim = true;
   for (i = 1; i <= fNpar; ++i) {
      iext = fNexofi[i-1];
      if (fNvarl[iext-1] >= 4) fLnolim = false;
      fErp[i-1] = 0;
      fErn[i-1] = 0;
      fGlobcc[i-1] = 0;
   }
   if (fISW[1] >= 1) {
      fISW[1] = 1;
      fDcovar = std::max(fDcovar,.5);
   }
} /* mnrset_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnsave()
{
//*-*-*-*Writes current parameter values and step sizes onto file ISYSSA*-*-*
//*-*    ===============================================================
//*-*          in format which can be reread by Minuit for restarting.
//*-*       The covariance matrix is also output if it exists.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   log_info("mnsave is dummy in lilliput::TMinuit");

} /* mnsave_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnscan()
{
//*-*-*-*-*Scans the values of FCN as a function of one parameter*-*-*-*-*-*
//*-*      ======================================================
//*-*        and plots the resulting values as a curve using MNPLOT.
//*-*        It may be called to scan one parameter or all parameters.
//*-*        retains the best function and parameter values found.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double step, uhigh, xhreq, xlreq, ubest, fnext, unext, xh, xl;
   int32_t ipar, iint, icall, ncall, nbins, nparx;
   int32_t nxypt, nccall, iparwd;

   xlreq = std::min(fWord7[2],fWord7[3]);
   xhreq = std::max(fWord7[2],fWord7[3]);
   ncall = int32_t((fWord7[1] + .01));
   if (ncall <= 1)  ncall = 41;
   if (ncall > 98) ncall = 98;
   nccall = ncall;
   if (fAmin == fUndefi) mnamin();
   iparwd  = int32_t((fWord7[0] + .1));
   ipar    = std::max(iparwd,0);
   fCstatu = "NO CHANGE";
   if (iparwd > 0) goto L200;

//*-*-        equivalent to a loop over parameters requested
L100:
   ++ipar;
   if (ipar > fNu) goto L900;
   iint = fNiofex[ipar-1];
   if (iint <= 0) goto L100;
//*-*-        set up range for parameter IPAR
L200:
   iint    = fNiofex[ipar-1];
   ubest    = fU[ipar-1];
   fXpt[0]  = ubest;
   fYpt[0]  = fAmin;
   fChpt[0] = ' ';
   fXpt[1]  = ubest;
   fYpt[1]  = fAmin;
   fChpt[1] = 'X';
   nxypt    = 2;
   if (fNvarl[ipar-1] > 1) goto L300;

//*-*-        no limits on parameter
   if (xlreq == xhreq) goto L250;
   unext = xlreq;
   step = (xhreq - xlreq) / double(ncall-1);
   goto L500;
L250:
   xl = ubest - fWerr[iint-1];
   xh = ubest + fWerr[iint-1];
   mnbins(xl, xh, ncall, unext, uhigh, nbins, step);
   nccall = nbins + 1;
   goto L500;
//*-*-        limits on parameter
L300:
   if (xlreq == xhreq) goto L350;
//*-*  Computing MAX
   xl = std::max(xlreq,fAlim[ipar-1]);
//*-*  Computing MIN
   xh = std::min(xhreq,fBlim[ipar-1]);
   if (xl >= xh) goto L700;
   unext = xl;
   step  = (xh - xl) / double(ncall-1);
   goto L500;
L350:
   unext = fAlim[ipar-1];
   step = (fBlim[ipar-1] - fAlim[ipar-1]) / double(ncall-1);
//*-*-        main scanning loop over parameter IPAR
L500:
   for (icall = 1; icall <= nccall; ++icall) {
      fU[ipar-1] = unext;
      nparx = fNpar;
      Eval(nparx, fGin, fnext, fU, 4);        ++fNfcn;
      ++nxypt;
      fXpt[nxypt-1]  = unext;
      fYpt[nxypt-1]  = fnext;
      fChpt[nxypt-1] = '*';
      if (fnext < fAmin) {
         fAmin   = fnext;
         ubest   = unext;
         fCstatu = "IMPROVED  ";
      }
      unext += step;
   }
   fChpt[nccall] = 0;

//*-*-        finished with scan of parameter IPAR
   fU[ipar-1] = ubest;
   mnexin(fX);
   if (fISW[4] >= 1)
   log_info("%dSCAN OF PARAMETER NO. %d,  %s"
         ,fNewpag,ipar,fCpnam[ipar-1].c_str());
   // mnplot(fXpt, fYpt, fChpt, nxypt, fNpagwd, fNpagln);
   log_warn("mnplot plotting has been disabled");
   goto L800;
L700:
   log_info(" REQUESTED RANGE OUTSIDE LIMITS FOR PARAMETER  %d",ipar);
L800:
   if (iparwd <= 0) goto L100;
//*-*-        finished with all parameters
L900:
   if (fISW[4] >= 0) mnprin(5, fAmin);
} /* mnscan_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnseek()
{
//*-*-*-*Performs a rough (but global) minimization by monte carlo search*-*
//*-*    ================================================================
//*-*        Each time a new minimum is found, the search area is shifted
//*-*        to be centered at the best value.  Random points are chosen
//*-*        uniformly over a hypercube determined by current step sizes.
//*-*   The Metropolis algorithm accepts a worse point with probability
//*-*      exp(-d/UP), where d is the degradation.  Improved points
//*-*      are of course always accepted.  Actual steps are random
//*-*      multiples of the nominal steps (DIRIN).
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Local variables */
   double dxdi, rnum, ftry, rnum1, rnum2, alpha;
   double flast, bar;
   int32_t ipar, iext, j, ifail, iseed=0, nparx, istep, ib, mxfail, mxstep;

   mxfail = int32_t(fWord7[0]);
   if (mxfail <= 0) mxfail = fNpar*20 + 100;
   mxstep = mxfail*10;
   if (fAmin == fUndefi) mnamin();
   alpha = fWord7[1];
   if (alpha <= 0) alpha = 3;
   if (fISW[4] >= 1) {
      log_info(" MNSEEK: MONTE CARLO MINIMIZATION USING METROPOLIS ALGORITHM");
      log_info(" TO STOP AFTER %6d SUCCESSIVE FAILURES, OR %7d STEPS",mxfail,mxstep);
      log_info(" MAXIMUM STEP SIZE IS %9.3f ERROR BARS.",alpha);
   }
   fCstatu = "INITIAL  ";
   if (fISW[4] >= 2) mnprin(2, fAmin);
   fCstatu = "UNCHANGED ";
   ifail   = 0;
   rnum    = 0;
   rnum1   = 0;
   rnum2   = 0;
   nparx   = fNpar;
   flast   = fAmin;
//*-*-             set up step sizes, starting values
   for (ipar = 1; ipar <= fNpar; ++ipar) {
      iext = fNexofi[ipar-1];
      fDirin[ipar-1] = alpha*2*fWerr[ipar-1];
      if (fNvarl[iext-1] > 1) {
//*-*-             parameter with limits
         mndxdi(fX[ipar-1], ipar-1, dxdi);
         if (dxdi == 0) dxdi = 1;
         fDirin[ipar-1] = alpha*2*fWerr[ipar-1] / dxdi;
         if (std::abs(fDirin[ipar-1]) > 6.2831859999999997) {
            fDirin[ipar-1] = 6.2831859999999997;
         }
      }
      fSEEKxmid[ipar-1]  = fX[ipar-1];
      fSEEKxbest[ipar-1] = fX[ipar-1];
   }
//*-*-                             search loop
   for (istep = 1; istep <= mxstep; ++istep) {
      if (ifail >= mxfail) break;
      for (ipar = 1; ipar <= fNpar; ++ipar) {
         mnrn15(rnum1, iseed);
         mnrn15(rnum2, iseed);
         fX[ipar-1] = fSEEKxmid[ipar-1] + (rnum1 + rnum2 - 1)*.5*fDirin[ipar-1];
      }
      mninex(fX);
      Eval(nparx, fGin, ftry, fU, 4);        ++fNfcn;
      if (ftry < flast) {
         if (ftry < fAmin) {
            fCstatu = "IMPROVEMNT";
            fAmin = ftry;
            for (ib = 1; ib <= fNpar; ++ib) { fSEEKxbest[ib-1] = fX[ib-1]; }
            ifail = 0;
            if (fISW[4] >= 2) mnprin(2, fAmin);
         }
         goto L300;
      } else {
         ++ifail;
//*-*-                  Metropolis algorithm
         bar = (fAmin - ftry) / fUp;
         mnrn15(rnum, iseed);
         if (bar < std::log(rnum)) continue;
      }
//*-*-                   Accept new point, move there
L300:
      for (j = 1; j <= fNpar; ++j) { fSEEKxmid[j-1] = fX[j-1];        }
      flast = ftry;
   }
//*-*-                              end search loop
   if (fISW[4] > 1) {
      log_info(" MNSEEK: %5d SUCCESSIVE UNSUCCESSFUL TRIALS.",ifail);
   }
   for (ib = 1; ib <= fNpar; ++ib) { fX[ib-1] = fSEEKxbest[ib-1]; }
   mninex(fX);
   if (fISW[4] >= 1) mnprin(2, fAmin);
   if (fISW[4] == 0) mnprin(0, fAmin);
} /* mnseek_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnset()
{
//*-*-*-*-*Interprets the commands that start with SET and SHOW*-*-*-*-*-*-*
//*-*      ====================================================
//*-*        Called from MNEXCM
//*-*        file characteristics for SET INPUT
//*-*       'SET ' or 'SHOW',  'ON ' or 'OFF', 'SUPPRESSED' or 'REPORTED  '
//*-*        explanation of print level numbers -1:3  and strategies 0:2
//*-*        identification of debug options
//*-*        things that can be set or shown
//*-*        options not intended for normal users
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Initialized data */

   static const char *cname[30] = {
      "FCN value ",
      "PARameters",
      "LIMits    ",
      "COVariance",
      "CORrelatio",
      "PRInt levl",
      "NOGradient",
      "GRAdient  ",
      "ERRor def ",
      "INPut file",
      "WIDth page",
      "LINes page",
      "NOWarnings",
      "WARnings  ",
      "RANdom gen",
      "TITle     ",
      "STRategy  ",
      "EIGenvalue",
      "PAGe throw",
      "MINos errs",
      "EPSmachine",
      "OUTputfile",
      "BATch     ",
      "INTeractiv",
      "VERsion   ",
      "reserve   ",
      "NODebug   ",
      "DEBug     ",
      "SHOw      ",
      "SET       "};

   static int32_t nname = 25;
   static int32_t nntot = 30;
   static std::string cprlev[5] = {
      "-1: NO OUTPUT EXCEPT FROM SHOW    ",
      " 0: REDUCED OUTPUT                ",
      " 1: NORMAL OUTPUT                 ",
      " 2: EXTRA OUTPUT FOR PROBLEM CASES",
      " 3: MAXIMUM OUTPUT                "};

   static std::string cstrat[3] = {
      " 0: MINIMIZE THE NUMBER OF CALLS TO FUNCTION",
      " 1: TRY TO BALANCE SPEED AGAINST RELIABILITY",
      " 2: MAKE SURE MINIMUM TRUE, ERRORS CORRECT  "};

   static std::string cdbopt[7] = {
      "REPORT ALL EXCEPTIONAL CONDITIONS      ",
      "MNLINE: LINE SEARCH MINIMIZATION       ",
      "MNDERI: FIRST DERIVATIVE CALCULATIONS  ",
      "MNHESS: SECOND DERIVATIVE CALCULATIONS ",
      "MNMIGR: COVARIANCE MATRIX UPDATES      ",
      "MNHES1: FIRST DERIVATIVE UNCERTAINTIES ",
      "MNCONT: MNCONTOUR PLOT (MNCROS SEARCH) "};

   /* System generated locals */
   //int32_t f_inqu();

   /* Local variables */
   double val;
   int32_t iset, iprm, i, jseed, kname, iseed, iunit, id, ii, kk;
   int32_t ikseed, idbopt, igrain=0, iswsav, isw2;
   std::string  cfname, cmode, ckind,  cwarn, copt, ctemp, ctemp2;
   bool lname=false;

   for (i = 1; i <= nntot; ++i) {
      ctemp  = cname[i-1];
      ckind  = ctemp.substr(0,3);
      ctemp2 = fCword.substr(4,6);
      if (strstr(ctemp2.c_str(),ckind.c_str())) goto L5;
   }
   i = 0;
L5:
   kname = i;

//*-*-          Command could be SET xxx, SHOW xxx,  HELP SET or HELP SHOW
   ctemp2 = fCword.substr(0,3);
   if (ctemp2.find("HEL")!=std::string::npos)  goto L2000;
   if (ctemp2.find("SHO")!=std::string::npos)  goto L1000;
   if (ctemp2.find("SET")==std::string::npos)  goto L1900;
//*-*-                          ---
   ckind = "SET ";
//*-*-                                       . . . . . . . . . . set unknown
   if (kname <= 0) goto L1900;
//*-*-                                       . . . . . . . . . . set known
   switch ((int)kname) {
      case 1:  goto L3000;
      case 2:  goto L20;
      case 3:  goto L30;
      case 4:  goto L40;
      case 5:  goto L3000;
      case 6:  goto L60;
      case 7:  goto L70;
      case 8:  goto L80;
      case 9:  goto L90;
      case 10:  goto L100;
      case 11:  goto L110;
      case 12:  goto L120;
      case 13:  goto L130;
      case 14:  goto L140;
      case 15:  goto L150;
      case 16:  goto L160;
      case 17:  goto L170;
      case 18:  goto L3000;
      case 19:  goto L190;
      case 20:  goto L3000;
      case 21:  goto L210;
      case 22:  goto L220;
      case 23:  goto L230;
      case 24:  goto L240;
      case 25:  goto L3000;
      case 26:  goto L1900;
      case 27:  goto L270;
      case 28:  goto L280;
      case 29:  goto L290;
      case 30:  goto L300;
   }

//*-*-                                       . . . . . . . . . . set param
L20:
   iprm = int32_t(fWord7[0]);
   if (iprm > fNu) goto L25;
   if (iprm <= 0) goto L25;
   if (fNvarl[iprm-1] < 0) goto L25;
   fU[iprm-1] = fWord7[1];
   mnexin(fX);
   isw2 = fISW[1];
   mnrset(1);
//*-*-       Keep approximate covariance matrix, even if new param value
   fISW[1] = std::min(isw2,1);
   fCfrom  = "SET PARM";
   fNfcnfr = fNfcn;
   fCstatu = "NEW VALUES";
   return;
L25:
   log_info(" UNDEFINED PARAMETER NUMBER.  IGNORED.");
   return;
//*-*-                                       . . . . . . . . . . set limits
L30:
   mnlims();
   return;
//*-*-                                       . . . . . . . . . . set covar
L40:
//*-*   this command must be handled by MNREAD, and is not Fortran-callable
   goto L3000;
//*-*-                                       . . . . . . . . . . set print
L60:
   fISW[4] = int32_t(fWord7[0]);
   return;
//*-*-                                       . . . . . . . . . . set nograd
L70:
   fISW[2] = 0;
   return;
//*-*-                                       . . . . . . . . . . set grad
L80:
   mngrad();
   return;
//*-*-                                       . . . . . . . . . . set errdef
L90:
   if (fWord7[0] == fUp) return;
   if (fWord7[0] <= 0) {
      if (fUp == fUpdflt) return;
      fUp = fUpdflt;
   } else {
      fUp = fWord7[0];
   }
   for (i = 1; i <= fNpar; ++i) {
      fErn[i-1] = 0;
      fErp[i-1] = 0;
   }
   mnwerr();
   return;
//*-*-                                       . . . . . . . . . . set input
//*-* This command must be handled by MNREAD. If it gets this far,
//*-*-        it is illegal.
L100:
   goto L3000;
//*-*-                                       . . . . . . . . . . set width
L110:
   fNpagwd = int32_t(fWord7[0]);
   fNpagwd = std::max(fNpagwd,50);
   return;

L120:
   fNpagln = int32_t(fWord7[0]);
   return;
//*-*-                                       . . . . . . . . . . set nowarn

L130:
   fLwarn = false;
   return;
//*-*-                                       . . . . . . . . . . set warn
L140:
   fLwarn = true;
   mnwarn("W", "SHO", "SHO");
   return;
//*-*-                                       . . . . . . . . . . set random
L150:
   jseed = int32_t(fWord7[0]);
   val = 3;
   mnrn15(val, jseed);
   if (fISW[4] > 0) {
      log_info(" MINUIT RANDOM NUMBER SEED SET TO %d",jseed);
   }
   return;
//*-*-                                       . . . . . . . . . . set title
L160:
//*-*   this command must be handled by MNREAD, and is not Fortran-callable
   goto L3000;
//*-*-                                       . . . . . . . . . set strategy
L170:
   fIstrat = int32_t(fWord7[0]);
   fIstrat = std::max(fIstrat,0);
   fIstrat = std::min(fIstrat,2);
   if (fISW[4] > 0) goto L1172;
   return;
//*-*-                                      . . . . . . . . . set page throw
L190:
   fNewpag = int32_t(fWord7[0]);
   goto L1190;
//*-*-                                       . . . . . . . . . . set epsmac
L210:
   if (fWord7[0] > 0 && fWord7[0] < .1) {
      fEpsmac = fWord7[0];
   }
   fEpsma2 = std::sqrt(fEpsmac);
   goto L1210;
//*-*-                                      . . . . . . . . . . set outputfile
L220:
   iunit = int32_t(fWord7[0]);
   fIsyswr = iunit;
   fIstkwr[0] = iunit;
   if (fISW[4] >= 0) goto L1220;
   return;
//*-*-                                       . . . . . . . . . . set batch
L230:
   fISW[5] = 0;
   if (fISW[4] >= 0) goto L1100;
   return;
//*-*-                                      . . . . . . . . . . set interactive
L240:
   fISW[5] = 1;
   if (fISW[4] >= 0) goto L1100;
   return;
//*-*-                                       . . . . . . . . . . set nodebug
L270:
   iset = 0;
   goto L281;
//*-*-                                       . . . . . . . . . . set debug
L280:
   iset = 1;
L281:
   idbopt = int32_t(fWord7[0]);
   if (idbopt > 6) goto L288;
   if (idbopt >= 0) {
      fIdbg[idbopt] = iset;
      if (iset == 1) fIdbg[0] = 1;
   } else {
//*-*-            SET DEBUG -1  sets all debug options
      for (id = 0; id <= 6; ++id) { fIdbg[id] = iset; }
   }
   fLrepor = fIdbg[0] >= 1;
   mnwarn("D", "SHO", "SHO");
   return;
L288:
   log_info(" UNKNOWN DEBUG OPTION %d REQUESTED. IGNORED",idbopt);
   return;
//*-*-                                       . . . . . . . . . . set show
L290:
//*-*-                                       . . . . . . . . . . set set
L300:
   goto L3000;
//*-*-               -----------------------------------------------------
L1000:
//*-*-              at this point, CWORD must be 'SHOW'
   ckind = "SHOW";
   if (kname <= 0) goto L1900;

   switch ((int)kname) {
      case 1:  goto L1010;
      case 2:  goto L1020;
      case 3:  goto L1030;
      case 4:  goto L1040;
      case 5:  goto L1050;
      case 6:  goto L1060;
      case 7:  goto L1070;
      case 8:  goto L1070;
      case 9:  goto L1090;
      case 10:  goto L1100;
      case 11:  goto L1110;
      case 12:  goto L1120;
      case 13:  goto L1130;
      case 14:  goto L1130;
      case 15:  goto L1150;
      case 16:  goto L1160;
      case 17:  goto L1170;
      case 18:  goto L1180;
      case 19:  goto L1190;
      case 20:  goto L1200;
      case 21:  goto L1210;
      case 22:  goto L1220;
      case 23:  goto L1100;
      case 24:  goto L1100;
      case 25:  goto L1250;
      case 26:  goto L1900;
      case 27:  goto L1270;
      case 28:  goto L1270;
      case 29:  goto L1290;
      case 30:  goto L1300;
   }

//*-*-                                       . . . . . . . . . . show fcn
L1010:
   if (fAmin == fUndefi) mnamin();
   mnprin(0, fAmin);
   return;
//*-*-                                       . . . . . . . . . . show param
L1020:
   if (fAmin == fUndefi) mnamin();
   mnprin(5, fAmin);
   return;
//*-*-                                       . . . . . . . . . . show limits
L1030:
   if (fAmin == fUndefi) mnamin();
   mnprin(1, fAmin);
   return;
//*-*-                                       . . . . . . . . . . show covar
L1040:
   mnmatu(1);
   return;
//*-*-                                       . . . . . . . . . . show corre
L1050:
   mnmatu(0);
   return;
//*-*-                                       . . . . . . . . . . show print
L1060:
   if (fISW[4] < -1) fISW[4] = -1;
   if (fISW[4] > 3)  fISW[4] = 3;
   log_info(" ALLOWED PRINT LEVELS ARE:");
   log_info("                           %s",cprlev[0].c_str());
   log_info("                           %s",cprlev[1].c_str());
   log_info("                           %s",cprlev[2].c_str());
   log_info("                           %s",cprlev[3].c_str());
   log_info("                           %s",cprlev[4].c_str());
   log_info(" CURRENT PRINTOUT LEVEL IS %s",cprlev[fISW[4]+1].c_str());
   return;
//*-*-                                       . . . . . . . show nograd, grad
L1070:
   if (fISW[2] <= 0) {
      log_info(" NOGRAD IS SET.  DERIVATIVES NOT COMPUTED IN FCN.");
   } else {
      log_info("   GRAD IS SET.  USER COMPUTES DERIVATIVES IN FCN.");
   }
   return;
//*-*-                                      . . . . . . . . . . show errdef
L1090:
   log_info(" ERRORS CORRESPOND TO FUNCTION CHANGE OF %g",fUp);
   return;
//*-*-                                      . . . . . . . . . . show input,
//*-*-                                               batch, or interactive
L1100:
//    ioin__1.inerr = 0;
//    ioin__1.inunit = fIsysrd;
//    ioin__1.infile = 0;
//    ioin__1.inex = 0;
//    ioin__1.inopen = 0;
//    ioin__1.innum = 0;
//    ioin__1.innamed = &lname;
//    ioin__1.innamlen = 64;
//    ioin__1.inname = cfname;
//    ioin__1.inacc = 0;
//    ioin__1.inseq = 0;
//    ioin__1.indir = 0;
//    ioin__1.infmt = 0;
//    ioin__1.inform = 0;
//    ioin__1.inunf = 0;
//    ioin__1.inrecl = 0;
//    ioin__1.innrec = 0;
//    ioin__1.inblank = 0;
//    f_inqu(&ioin__1);
   cmode = "BATCH MODE      ";
   if (fISW[5] == 1) cmode  = "INTERACTIVE MODE";
   if (! lname)      cfname = "unknown";
   log_info(" INPUT NOW BEING READ IN %s FROM UNIT NO. %d FILENAME: %s"
           ,cmode.c_str(),fIsysrd,cfname.c_str());
   return;
//*-*-                                      . . . . . . . . . . show width
L1110:
   log_info("          PAGE WIDTH IS SET TO %d COLUMNS",fNpagwd);
   return;
//*-*-                                      . . . . . . . . . . show lines
L1120:
   log_info("          PAGE LENGTH IS SET TO %d LINES",fNpagln);
   return;
//*-*-                                      . . . . . . .show nowarn, warn
L1130:
   cwarn = "SUPPRESSED";
   if (fLwarn) cwarn = "REPORTED  ";
   log_info("%s",cwarn.c_str());
   if (! fLwarn) mnwarn("W", "SHO", "SHO");
   return;
//*-*-                                     . . . . . . . . . . show random
L1150:
   val = 0;
   mnrn15(val, igrain);
   ikseed = igrain;
   log_info(" MINUIT RNDM SEED IS CURRENTLY=%d",ikseed);
   val   = 3;
   iseed = ikseed;
   mnrn15(val, iseed);
   return;
//*-*-                                       . . . . . . . . . show title
L1160:
   log_info(" TITLE OF CURRENT TASK IS:%s",fCtitl.c_str());
   return;
//*-*-                                       . . . . . . . show strategy
L1170:
   log_info(" ALLOWED STRATEGIES ARE:");
   log_info("                    %s",cstrat[0].c_str());
   log_info("                    %s",cstrat[1].c_str());
   log_info("                    %s",cstrat[2].c_str());
L1172:
   log_info(" NOW USING STRATEGY %s",cstrat[fIstrat].c_str());
   return;
//*-*-                                         . . . . . show eigenvalues
L1180:
   iswsav = fISW[4];
   fISW[4] = 3;
   if (fISW[1] < 1) {
      log_info("%s",fCovmes[0].c_str());
   } else {
      mnpsdf();
   }
   fISW[4] = iswsav;
   return;
//*-*-                                           . . . . . show page throw
L1190:
   log_info(" PAGE THROW CARRIAGE CONTROL = %d",fNewpag);
   if (fNewpag == 0) {
      log_info(" NO PAGE THROWS IN MINUIT OUTPUT");
   }
   return;
//*-*-                                       . . . . . . show minos errors
L1200:
   for (ii = 1; ii <= fNpar; ++ii) {
      if (fErp[ii-1] > 0 || fErn[ii-1] < 0) goto L1204;
   }
   log_info("       THERE ARE NO MINOS ERRORS CURRENTLY VALID.");
   return;
L1204:
   mnprin(4, fAmin);
   return;
//*-*-                                       . . . . . . . . . show epsmac
L1210:
   log_info(" FLOATING-POINT NUMBERS ASSUMED ACCURATE TO %g",fEpsmac);
   return;
//*-*-                                       . . . . . . show outputfiles
L1220:
   log_info("  MINUIT PRIMARY OUTPUT TO UNIT %d",fIsyswr);
   return;
//*-*-                                       . . . . . . show version
L1250:
   log_info(" THIS IS MINUIT VERSION:%s",fCvrsn.c_str());
   return;
//*-*-                                       . . . . . . show nodebug, debug
L1270:
   for (id = 0; id <= 6; ++id) {
      copt = "OFF";
      if (fIdbg[id] >= 1) copt = "ON ";
      log_info("          DEBUG OPTION %3d IS %3s :%s"
             ,id,copt.c_str(),cdbopt[id].c_str());
   }
   if (! fLrepor) mnwarn("D", "SHO", "SHO");
   return;
//*-*-                                       . . . . . . . . . . show show
L1290:
   ckind = "SHOW";
   goto L2100;
//*-*-                                       . . . . . . . . . . show set
L1300:
   ckind = "SET ";
   goto L2100;
//*-*-               -----------------------------------------------------
//*-*-                             UNKNOWN COMMAND
L1900:
   log_info(" THE COMMAND:%10s IS UNKNOWN.",fCword.c_str());
   goto L2100;
//*-*-               -----------------------------------------------------
//*-*-                   HELP SHOW,  HELP SET,  SHOW SET, or SHOW SHOW
L2000:
   ckind = "SET ";
   ctemp2 = fCword.substr(3,7);
   if (strcmp(ctemp2.c_str(), "SHO")) ckind = "SHOW";
L2100:
   log_info(" THE FORMAT OF THE %4s COMMAND IS:",ckind.c_str());
   log_info(" %s xxx    [numerical arguments if any]",ckind.c_str());
   log_info(" WHERE xxx MAY BE ONE OF THE FOLLOWING:");
   for (kk = 1; kk <= nname; ++kk) {
      log_info(" %s",cname[kk-1]);
   }
   return;
//*-*-               -----------------------------------------------------
//*-*-                              ILLEGAL COMMAND
L3000:
   log_info(" ABOVE COMMAND IS ILLEGAL.   IGNORED");

} /* mnset_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnsimp()
{
//*-*-*-*-*Minimization using the simplex method of Nelder and Mead*-*-*-*-*
//*-*      ========================================================
//*-*        Performs a minimization using the simplex method of Nelder
//*-*        and Mead (ref. -- Comp. J. 7,308 (1965)).
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* Initialized data */

   static double alpha = 1;
   static double beta = .5;
   static double gamma = 2;
   static double rhomin = 4;
   static double rhomax = 8;

   /* Local variables */
   double dmin_, dxdi, yrho, f, ynpp1, aming, ypbar;
   double bestx, ystar, y1, y2, ystst, pb, wg;
   double absmin, rho, sig2, rho1, rho2;
   int32_t npfn, i, j, k, jhold, ncycl, nparx;
   int32_t nparp1, kg, jh, nf, jl, ns;

   if (fNpar <= 0) return;
   if (fAmin == fUndefi) mnamin();
   fCfrom  = "SIMPLEX ";
   fNfcnfr = fNfcn;
   fCstatu = "UNCHANGED ";
   npfn    = fNfcn;
   nparp1  = fNpar + 1;
   nparx   = fNpar;
   rho1    = alpha + 1;
   rho2    = rho1 + alpha*gamma;
   wg      = 1 / double(fNpar);
   if (fISW[4] >= 0) {
      log_info(" START SIMPLEX MINIMIZATION.    CONVERGENCE WHEN EDM .LT. %g",fEpsi);
   }
   for (i = 1; i <= fNpar; ++i) {
      fDirin[i-1] = fWerr[i-1];
      mndxdi(fX[i-1], i-1, dxdi);
      if (dxdi != 0) fDirin[i-1] = fWerr[i-1] / dxdi;
      dmin_ = fEpsma2*std::abs(fX[i-1]);
      if (fDirin[i-1] < dmin_) fDirin[i-1] = dmin_;
   }
//*-* **       choose the initial simplex using single-parameter searches
L1:
   ynpp1 = fAmin;
   jl = nparp1;
   fSIMPy[nparp1-1] = fAmin;
   absmin = fAmin;
   for (i = 1; i <= fNpar; ++i) {
      aming      = fAmin;
      fPbar[i-1] = fX[i-1];
      bestx      = fX[i-1];
      kg         = 0;
      ns         = 0;
      nf         = 0;
L4:
      fX[i-1] = bestx + fDirin[i-1];
      mninex(fX);
      Eval(nparx, fGin, f, fU, 4);        ++fNfcn;
      if (f <= aming) goto L6;
//*-*-        failure
      if (kg == 1) goto L8;
      kg = -1;
      ++nf;
      fDirin[i-1] *= -.4;
      if (nf < 3) goto L4;
      ns = 6;
//*-*-        success
L6:
      bestx        = fX[i-1];
      fDirin[i-1] *= 3;
      aming        = f;
      fCstatu      = "PROGRESS  ";
      kg           = 1;
      ++ns;
      if (ns < 6) goto L4;
//*-*-        local minimum found in ith direction
L8:
      fSIMPy[i-1] = aming;
      if (aming < absmin) jl = i;
      if (aming < absmin) absmin = aming;
      fX[i-1] = bestx;
      for (k = 1; k <= fNpar; ++k) { fP[k + i*fMaxpar - fMaxpar-1] = fX[k-1]; }
   }
   jh    = nparp1;
   fAmin = fSIMPy[jl-1];
   mnrazz(ynpp1, fPbar, fSIMPy, jh, jl);
   for (i = 1; i <= fNpar; ++i) { fX[i-1] = fP[i + jl*fMaxpar - fMaxpar-1]; }
   mninex(fX);
   fCstatu = "PROGRESS  ";
   if (fISW[4] >= 1) mnprin(5, fAmin);
   fEDM  = fBigedm;
   sig2  = fEDM;
   ncycl = 0;
//*-*-                                       . . . . .  start main loop
L50:
   if (sig2 < fEpsi && fEDM < fEpsi) goto L76;
   sig2 = fEDM;
   if (fNfcn - npfn > fNfcnmx) goto L78;
//*-*-        calculate new point * by reflection
   for (i = 1; i <= fNpar; ++i) {
      pb = 0;
      for (j = 1; j <= nparp1; ++j) { pb += wg*fP[i + j*fMaxpar - fMaxpar-1]; }
      fPbar[i-1]  = pb - wg*fP[i + jh*fMaxpar - fMaxpar-1];
      fPstar[i-1] = (alpha + 1)*fPbar[i-1] - alpha*fP[i + jh*fMaxpar - fMaxpar-1];
   }
   mninex(fPstar);
   Eval(nparx, fGin, ystar, fU, 4);    ++fNfcn;
   if (ystar >= fAmin) goto L70;
//*-*-        point * better than jl, calculate new point **
   for (i = 1; i <= fNpar; ++i) {
      fPstst[i-1] = gamma*fPstar[i-1] + (1 - gamma)*fPbar[i-1];
   }
   mninex(fPstst);
   Eval(nparx, fGin, ystst, fU, 4);    ++fNfcn;
//*-*-        try a parabola through ph, pstar, pstst.  min = prho
   y1 = (ystar - fSIMPy[jh-1])*rho2;
   y2 = (ystst - fSIMPy[jh-1])*rho1;
   rho = (rho2*y1 - rho1*y2)*.5 / (y1 - y2);
   if (rho < rhomin) goto L66;
   if (rho > rhomax) rho = rhomax;
   for (i = 1; i <= fNpar; ++i) {
      fPrho[i-1] = rho*fPbar[i-1] + (1 - rho)*fP[i + jh*fMaxpar - fMaxpar-1];
   }
   mninex(fPrho);
   Eval(nparx, fGin, yrho, fU, 4);    ++fNfcn;
   if (yrho  < fSIMPy[jl-1] && yrho < ystst) goto L65;
   if (ystst < fSIMPy[jl-1]) goto L67;
   if (yrho  > fSIMPy[jl-1]) goto L66;
//*-*-        accept minimum point of parabola, PRHO
L65:
   mnrazz(yrho, fPrho, fSIMPy, jh, jl);
   goto L68;
L66:
   if (ystst < fSIMPy[jl-1]) goto L67;
   mnrazz(ystar, fPstar, fSIMPy, jh, jl);
   goto L68;
L67:
   mnrazz(ystst, fPstst, fSIMPy, jh, jl);
L68:
   ++ncycl;
   if (fISW[4] < 2) goto L50;
   if (fISW[4] >= 3 || ncycl % 10 == 0) {
      mnprin(5, fAmin);
   }
   goto L50;
//*-*-        point * is not as good as jl
L70:
   if (ystar >= fSIMPy[jh-1]) goto L73;
   jhold = jh;
   mnrazz(ystar, fPstar, fSIMPy, jh, jl);
   if (jhold != jh) goto L50;
//*-*-        calculate new point **
L73:
   for (i = 1; i <= fNpar; ++i) {
      fPstst[i-1] = beta*fP[i + jh*fMaxpar - fMaxpar-1] + (1 - beta)*fPbar[i-1];
   }
   mninex(fPstst);
   Eval(nparx, fGin, ystst, fU, 4);    ++fNfcn;
   if (ystst > fSIMPy[jh-1]) goto L1;
//*-*-    point ** is better than jh
   if (ystst < fAmin) goto L67;
   mnrazz(ystst, fPstst, fSIMPy, jh, jl);
   goto L50;
//*-*-                                       . . . . . .  end main loop
L76:
   if (fISW[4] >= 0) {
      log_info(" SIMPLEX MINIMIZATION HAS CONVERGED.");
   }
   fISW[3] = 1;
   goto L80;
L78:
   if (fISW[4] >= 0) {
      log_info(" SIMPLEX TERMINATES WITHOUT CONVERGENCE.");
   }
   fCstatu = "CALL LIMIT";
   fISW[3] = -1;
   fISW[0] = 1;
L80:
   for (i = 1; i <= fNpar; ++i) {
      pb = 0;
      for (j = 1; j <= nparp1; ++j) { pb += wg*fP[i + j*fMaxpar - fMaxpar-1]; }
      fPbar[i-1] = pb - wg*fP[i + jh*fMaxpar - fMaxpar-1];
   }
   mninex(fPbar);
   Eval(nparx, fGin, ypbar, fU, 4);    ++fNfcn;
   if (ypbar < fAmin)         mnrazz(ypbar, fPbar, fSIMPy, jh, jl);
   mninex(fX);
   if (fNfcnmx + npfn - fNfcn < fNpar*3) goto L90;
   if (fEDM > fEpsi*2) goto L1;
L90:
   if (fISW[4] >= 0) mnprin(5, fAmin);
} /* mnsimp_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnstat(double &fmin, double &fedm, double &errdef, int32_t &npari, int32_t &nparx, int32_t &istat)
{
//*-*-*-*-*Returns concerning the current status of the minimization*-*-*-*-*
//*-*      =========================================================
//*-*       User-called
//*-*          Namely, it returns:
//*-*        FMIN: the best function value found so far
//*-*        FEDM: the estimated vertical distance remaining to minimum
//*-*        ERRDEF: the value of UP defining parameter uncertainties
//*-*        NPARI: the number of currently variable parameters
//*-*        NPARX: the highest (external) parameter number defined by user
//*-*        ISTAT: a status integer indicating how good is the covariance
//*-*           matrix:  0= not calculated at all
//*-*                    1= approximation only, not accurate
//*-*                    2= full matrix, but forced positive-definite
//*-*                    3= full accurate covariance matrix
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   fmin   = fAmin;
   fedm   = fEDM;
   errdef = fUp;
   npari  = fNpar;
   nparx  = fNu;
   istat  = fISW[1];
   if (fEDM == fBigedm) fedm = fUp;
   if (fAmin == fUndefi) {
      fmin  = 0;
      fedm  = fUp;
      istat = 0;
   }
} /* mnstat_ */

//______________________________________________________________________________
void lilliput::TMinuit::mntiny(double epsp1, double &epsbak)
{
//*-*-*-*-*-*-*-*To find the machine precision*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*            =============================
//*-*        Compares its argument with the value 1.0, and returns
//*-*        the value .TRUE. if they are equal.  To find EPSMAC
//*-*        safely by foiling the Fortran optimizer
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   epsbak = epsp1 - 1;
} /* mntiny_ */

//______________________________________________________________________________
bool lilliput::TMinuit::mnunpt(std::string &cfname)
{
//*-*-*-*-*-*Returns .TRUE. if CFNAME contains unprintable characters*-*-*-*
//*-*        ========================================================
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   int32_t i, l, ic;
   bool ret_val;
   static std::string cpt = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890./;:[]$%*_!@#&+()";

   ret_val = false;
   l       = strlen(cfname.c_str());
   for (i = 1; i <= l; ++i) {
      for (ic = 1; ic <= 80; ++ic) {
         if (cfname[i-1] == cpt[ic-1]) goto L100;
      }
      return true;
L100:
      ;
   }
   return ret_val;
} /* mnunpt_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnvert(double *a, int32_t l, int32_t, int32_t n, int32_t &ifail)
{
//*-*-*-*-*-*-*-*-*-*-*-*Inverts a symmetric matrix*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                    ==========================
//*-*        inverts a symmetric matrix.   matrix is first scaled to
//*-*        have all ones on the diagonal (equivalent to change of units)
//*-*        but no pivoting is done since matrix is positive-definite.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   /* System generated locals */
   int32_t a_offset;

   /* Local variables */
   double si;
   int32_t i, j, k, kp1, km1;

   /* Parameter adjustments */
   a_offset = l + 1;
   a -= a_offset;

   /* Function Body */
   ifail = 0;
   if (n < 1) goto L100;
   if (n > fMaxint) goto L100;
//*-*-                  scale matrix by sqrt of diag elements
   for (i = 1; i <= n; ++i) {
      si = a[i + i*l];
      if (si <= 0) goto L100;
      fVERTs[i-1] = 1 / std::sqrt(si);
   }
   for (i = 1; i <= n; ++i) {
      for (j = 1; j <= n; ++j) {
         a[i + j*l] = a[i + j*l]*fVERTs[i-1]*fVERTs[j-1];
      }
   }
//*-*-                                       . . . start main loop . . . .
   for (i = 1; i <= n; ++i) {
      k = i;
//*-*-                  preparation for elimination step1
      if (a[k + k*l] != 0) fVERTq[k-1] = 1 / a[k + k*l];
      else goto L100;
      fVERTpp[k-1] = 1;
      a[k + k*l] = 0;
      kp1 = k + 1;
      km1 = k - 1;
      if (km1 < 0) goto L100;
      else if (km1 == 0) goto L50;
      else               goto L40;
L40:
      for (j = 1; j <= km1; ++j) {
         fVERTpp[j-1] = a[j + k*l];
         fVERTq[j-1]  = a[j + k*l]*fVERTq[k-1];
         a[j + k*l]   = 0;
      }
L50:
      if (k - n < 0) goto L51;
      else if (k - n == 0) goto L60;
      else                goto L100;
L51:
      for (j = kp1; j <= n; ++j) {
         fVERTpp[j-1] = a[k + j*l];
         fVERTq[j-1]  = -a[k + j*l]*fVERTq[k-1];
         a[k + j*l]   = 0;
      }
//*-*-                  elimination proper
L60:
      for (j = 1; j <= n; ++j) {
         for (k = j; k <= n; ++k) { a[j + k*l] += fVERTpp[j-1]*fVERTq[k-1]; }
      }
   }
//*-*-                  elements of left diagonal and unscaling
   for (j = 1; j <= n; ++j) {
      for (k = 1; k <= j; ++k) {
         a[k + j*l] = a[k + j*l]*fVERTs[k-1]*fVERTs[j-1];
         a[j + k*l] = a[k + j*l];
      }
   }
   return;
//*-*-                  failure return
L100:
   ifail = 1;
} /* mnvert_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnwarn(const char *copt1, const char *corg1, const char *cmes1)
{
//*-*-*-*-*-*-*-*-*-*-*-*Prints Warning messages*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                    =======================
//*-*     If COPT='W', CMES is a WARning message from CORG.
//*-*     If COPT='D', CMES is a DEBug message from CORG.
//*-*         If SET WARnings is in effect (the default), this routine
//*-*             prints the warning message CMES coming from CORG.
//*-*         If SET NOWarnings is in effect, the warning message is
//*-*             stored in a circular buffer of length kMAXMES.
//*-*         If called with CORG=CMES='SHO', it prints the messages in
//*-*             the circular buffer, FIFO, and empties the buffer.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   std::string copt = copt1;
   std::string corg = corg1;
   std::string cmes = cmes1;

   const int32_t kMAXMES = 10;
   int32_t ityp, i, ic, nm;
   std::string englsh, ctyp;

   if (corg.substr(0,3) != "SHO" || cmes.substr(0,3) != "SHO") {

//*-*-            Either print warning or put in buffer
      if (copt == "W") {
         ityp = 1;
         if (fLwarn) {
            log_info(" MINUIT WARNING IN %s",corg.c_str());
            log_info(" ============== %s",cmes.c_str());
            return;
         }
      } else {
         ityp = 2;
         if (fLrepor) {
            log_info(" MINUIT DEBUG FOR %s",corg.c_str());
            log_info(" =============== %s ",cmes.c_str());
            return;
         }
      }
//*-*-                if appropriate flag is off, fill circular buffer
      if (fNwrmes[ityp-1] == 0) fIcirc[ityp-1] = 0;
      ++fNwrmes[ityp-1];
      ++fIcirc[ityp-1];
      if (fIcirc[ityp-1] > 10)         fIcirc[ityp-1] = 1;
      ic = fIcirc[ityp-1];
      fOrigin[ic] = corg;
      fWarmes[ic] = cmes;
      fNfcwar[ic] = fNfcn;
      return;
   }

//*-*-            'SHO WARnings', ask if any suppressed mess in buffer
   if (copt == "W") {
      ityp = 1;
      ctyp = "WARNING";
   } else {
      ityp = 2;
      ctyp = "*DEBUG*";
   }
   if (fNwrmes[ityp-1] > 0) {
      englsh = " WAS SUPPRESSED.  ";
      if (fNwrmes[ityp-1] > 1) englsh = "S WERE SUPPRESSED.";
      log_info(" %5d MINUIT %s MESSAGE%s",fNwrmes[ityp-1]
             ,ctyp.c_str(),englsh.c_str());
      nm = fNwrmes[ityp-1];
      ic = 0;
      if (nm > kMAXMES) {
         log_info(" ONLY THE MOST RECENT 10 WILL BE LISTED BELOW.");
         nm = kMAXMES;
         ic = fIcirc[ityp-1];
      }
      log_info("  CALLS  ORIGIN         MESSAGE");
      for (i = 1; i <= nm; ++i) {
         ++ic;
         if (ic > kMAXMES) ic = 1;
         log_info(" %6d  %s  %s", fNfcwar[ic],fOrigin[ic].c_str(),fWarmes[ic].c_str());
      }
      fNwrmes[ityp-1] = 0;
      log_info(" ");
   }
} /* mnwarn_ */

//______________________________________________________________________________
void lilliput::TMinuit::mnwerr()
{
//*-*-*-*-*-*-*-*Calculates the WERR, external parameter errors*-*-*-*-*-*-*
//*-*            ==============================================
//*-*      and the global correlation coefficients, to be called
//*-*      whenever a new covariance matrix is available.
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   double denom, ba, al, dx, du1, du2;
   int32_t ndex, ierr, i, j, k, l, ndiag, k1, iin;

//*-*-                        calculate external error if v exists
   if (fISW[1] >= 1) {
      for (l = 1; l <= fNpar; ++l) {
         ndex = l*(l + 1) / 2;
         dx = std::sqrt(std::abs(fVhmat[ndex-1]*fUp));
         i = fNexofi[l-1];
         if (fNvarl[i-1] > 1) {
            al = fAlim[i-1];
            ba = fBlim[i-1] - al;
            du1 = al + 0.5*(std::sin(fX[l-1] + dx) + 1)*ba - fU[i-1];
            du2 = al + 0.5*(std::sin(fX[l-1] - dx) + 1)*ba - fU[i-1];
            if (dx > 1) du1 = ba;
            dx = 0.5*(std::abs(du1) + std::abs(du2));
         }
         fWerr[l-1] = dx;
      }
   }
//*-*-                         global correlation coefficients
   if (fISW[1] >= 1) {
      for (i = 1; i <= fNpar; ++i) {
         fGlobcc[i-1] = 0;
         k1 = i*(i-1) / 2;
         for (j = 1; j <= i; ++j) {
            k = k1 + j;
            fP[i + j*fMaxpar - fMaxpar-1] = fVhmat[k-1];
            fP[j + i*fMaxpar - fMaxpar-1] = fP[i + j*fMaxpar - fMaxpar-1];
         }
      }
      mnvert(fP, fMaxint, fMaxint, fNpar, ierr);
      if (ierr == 0) {
         for (iin = 1; iin <= fNpar; ++iin) {
            ndiag = iin*(iin + 1) / 2;
            denom = fP[iin + iin*fMaxpar - fMaxpar-1]*fVhmat[ndiag-1];
            if (denom <= 1 && denom >= 0) fGlobcc[iin-1] = 0;
            else                          fGlobcc[iin-1] = std::sqrt(1 - 1 / denom);
         }
      }
   }
} /* mnwerr_ */

//LCOV_EXCL_STOP