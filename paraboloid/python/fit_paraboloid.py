import sys
if __name__ == '__main__':
    sys.exit(0)

# TODO: move documentation into the RST docs (makes the formulae more readable)

from icecube.paraboloid import I3ParaboloidFitParams
from icecube.dataclasses import I3Direction
from icecube.icetray import logging, I3Units
import scipy.optimize as optimization
import numpy as np

# polynomial form of paraboloid (used for doing the actual fit)
def paraboloid(f0,x0,y0,a,b,c):
    return lambda x,y: f0+0.5*a*(x-x0)**2+b*(x-x0)*(y-y0)+0.5*c*(y-y0)**2

"""
We use the polynomial form to do the fit to the likelihood landscape and then
we transform the result to what Till Neunhoefer called the 'standard form':
rotation angle of the error ellipse and the lengths of its long and short
axis.
(We are not using that form for doing the fit because in case of err1=err2 the
parametrization becomes degenerate (rotation angle can be any arbitrary
value, because circles are round) and then the minimizer gets confused.)

Near an extremum L0=L(x0,y0) of L(x,y) we may write
(using the polynomial form like above):

 dL = L-L0 = 0.5*a*(x-x0)**2+b*(x-x0)*(y-y0)+0.5*c*(y-y0)**2

For brevity, let's introduce dx=x-x0 and dx=y0:

 (Eq. I) dL = 0.5*a*dx**2 + b*dx*dy + 0.5*c*dy**2

The form that makes it easy to extract the error ellips looks like this:

 dL = A*((cosfi,sinfi)*(dx,dy))**2 + B*((-sinfi,cosfi)*(dx,dy))**2

This describes (for some fixed positive value of dL) an ellipse with long
axes dL/sqrt(A) and dL/sqrt(B), respectively, and the tangent of the rotation
angle of the ellipse is sinfi/cosfi. For dL=0.5 (the 1-sigma error ellipse)
these axes are going to be err1 and err2, respectively.

So our fit gives us (a,b,c) but we want (A,B,fi).
In order to get the conversion formulas that we can use in our code,
first we reshuffle the standard form back to polynomial form:

 dL = A*(dx*cosfi+dy*sinfi)**2 + B*(-dx*sinfi+dy*cosfi)**2
    = A*((dx*cosfi)**2+(dy*sinfi)**2 + 2*dx*dy*cosfi*sinfi) + B*((dx*sinfi)**2+(dy*cosfi)**2-2*dx*dy*sinfi*cosfi)
    = (A*cosfi**2+B*sinfi**2)*dx*dx + 2*cosfi*sinfi*(A-B)*dx*dy + (A*sinfi**2+B*cosfi**2)*dy**2

Requiring that this is identical to (Eq. I) for all dx,dy we have
 (Eq. II) a = 2*(A*cosfi**2+B*sinfi**2)
          b = 2*cosfi*sinfi*(A-B)
          c = 2*(A*sinfi**2+B*cosfi**2)

So let's solve this. First notice that
 a + c = 2*(A + B)
so:
 (Eq. III) A = (a + c)/2 - B

If you insert this in the expression for b:
 b = 2*cosfi*sinfi*(a + c - 2*B)

Then it easy to express B (and A, using Eq. III) in terms of a,b,c and fi:
 (Eq. IV) B = 0.25*[ a + c - b/(cosfi*sinfi) ]
          A = (a + c)/2 - B = 0.25 * [ b/(cosfi*sinfi) + a + c ]

Now if we insert IV back into IIa:
 a = 0.5*b*cosfi/sinfi + 0.5*(a+c)*cosfi**2 + 0.5*(a+c)*sinfi**2 - 0.5*sinfi/cosfi
   = 0.5 * [ b*(cosfi/sinfi - sinfi/cosfi) + a + c ]

Write tanfi=sinfi/cosfi, rearrange some more:
 (Eq. V) b*tanfi**2 + (a-c) * tanfi - b = 0

Special cases:
    If b==0 and a==c then:
        tanfi is undefined/undetermined (we choose to set it to 0.)
        A=B=0.25*(a+c)==0.5*a==0.5*c.
    If b==0 and a!=c then:
        tanfi=0 (so: sinfi=0,cosfi=1)
        inserting that back into Eq. I gives A=0.5*a and B=0.5*c.

So for the case b==0 (which in numerical reality will be formulated as abs(b)<epsilon)
we simply put A=a/2 and B=c/2.

In the generic case (abs(b)>epsilon) we just solve tanfi from Eq. V
using the standard formula for quadratic equations:
    tanfi = (-(a-c) +/- sqrt((a-c)**2 + 4 * b**2))/2*b
          = (c-a)/(2*b) +/- sqrt( ((c-a)/(4*b))**2 + 1))
The two solutions differ exactly by 90 degrees, which can be verified
by checking that the product of the two tanfi solutions is equal to -1.
So the fact that we have two solutions corresponds to the choice that we
have to which of the two elliptical axes we let A and B, resp., correspond.

Using cosfi*sinfi = tanfi/(1+tanfi**2) we can now use Eq. IV to compute A and
B, and from that err1 and err2. Both tanfi solutions are equivalent, we use
the one for which A<=B, so that err1>=err2.
"""

def fit_paraboloid(axyt,allh,inputdir,name):
    """
    The formulae in this function are explained in the comments above.
    These will move to RST. Then *this* comment should be updated.
    """
    llh0=np.min(allh)
    allh-=llh0
    deg1invsqr = (1./I3Units.degree)**2
    parinit=[0.,0.,0.,deg1invsqr,0.,deg1invsqr] # could be more fancy, using moments
    f=lambda params: paraboloid(*params)(*axyt)-allh
    pars,covmat,info,mesg,ier = optimization.leastsq(f,parinit,full_output=True)
    success = ier in [1,2,3,4] # see scipy.optimize.leastsq help page
    f0,x0,y0,a,b,c = pars
    logging.log_debug("f0=%f x0=%f y0=%f a=%f b=%f c=%f" % (f0,x0,y0,a,b,c),unit=name)
    logging.log_debug("ier=%d mesg=%s" % (ier,mesg),unit=name)
    logging.log_debug("success=%s" % success,unit=name)
    if abs(b)<1e-7:
        logging.log_debug("abs(b=%f)<1e-7, neglecting correlation" % b,unit=name)
        A=a/2.
        tanfi=0.
        B=c/2.
    else:
        tanfi = (c-a + np.sqrt((c-a)**2 + 4*b**2))/2./b
        cosfi_sinfi = tanfi/(1+tanfi**2)
        A = 0.25*( a + c + b/cosfi_sinfi )
        B = 0.25*( a + c - b/cosfi_sinfi )
    logging.log_debug("A=%f B=%f tanfi=%f" % (A,B,tanfi),unit=name)
    #sinfi = tanfi/np.sqrt(1+tanfi**2)
    #cosfi = 1/np.sqrt(1+tanfi**2)
    #aa = A*cosfi**2+B*sinfi**2
    #bb = 2*cosfi*sinfi*(A-B)
    #cc = A*sinfi**2+B*cosfi**2
    #logging.log_notice("check aa=%f=a=%f bb=%f=b=%f cc=%f=c=%f" % (aa,a,bb,b,cc,c),unit=name)
    sig1 = np.sign(A)/np.sqrt(abs(A))
    sig2 = np.sign(B)/np.sqrt(abs(B))
    rotang = np.arctan2(tanfi,1)
    if np.abs(sig1) < np.abs(sig2):
        (sig1,sig2) = (sig2,sig1)
        rotang = np.arctan2(1,tanfi)
    # now saving the results, using the original params class for now
    # several of its data members are actually not interesting
    fitpar = I3ParaboloidFitParams()
    fitpar.pbfRotAng =  rotang
    fitpar.pbfErr1 = sig1 # actually, maybe we should do np.arcsin(sig1) instead
    fitpar.pbfErr2 = sig2 # actually, maybe we should do np.arcsin(sig2) instead
    xy02=x0**2+y0**2
    if sig1<1 and sig2<1 and sig1>0 and sig2>0 and xy02<1:
        fitpar.pbfErr1 = np.arcsin(sig1)
        fitpar.pbfErr2 = np.arcsin(sig2)
        fitpar.pbfStatus = I3ParaboloidFitParams.PBF_SUCCESS
    elif fitpar.pbfErr1 < 0 and fitpar.pbfErr2 > 0:
        fitpar.pbfStatus = I3ParaboloidFitParams.PBF_NON_POSITIVE_ERR_1
    elif fitpar.pbfErr1 > 0 and fitpar.pbfErr2 < 0:
        fitpar.pbfStatus = I3ParaboloidFitParams.PBF_NON_POSITIVE_ERR_2
    elif fitpar.pbfErr1 < 0 and fitpar.pbfErr2 < 0:
        fitpar.pbfStatus = I3ParaboloidFitParams.PBF_NON_POSITIVE_ERRS
    else:
        fitpar.pbfStatus = I3ParaboloidFitParams.PBF_FAILED_PARABOLOID_FIT
    fitpar.pbfCenterLlh = f0+llh0
    fitpar.pbfStatus = I3ParaboloidFitParams.PBF_SUCCESS
    fitpar.pbfCurv11 = a
    fitpar.pbfCurv12 = b
    fitpar.pbfCurv22 = c
    # get the direction corresponding to paraboloids minimum
    if xy02<1:
        zdir = inputdir
        xdir = I3Direction(zdir.zenith+90.*I3Units.degree,zdir.azimuth)
        ydir = zdir.cross(xdir)
        z0=np.sqrt(1-xy02)
        dir0=I3Direction(x0*xdir+y0*ydir+z0*zdir)
        fitpar.pbfZen = dir0.zenith
        fitpar.pbfAzi = dir0.azimuth
    fitpar.pbfZenOff = x0
    fitpar.pbfAziOff = y0
    fitpar.pbfChi2 = np.sum(f(pars)**2)
    return fitpar
