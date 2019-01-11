For the developer of new and improved things
*************************************************

Is this code "done"?  
Of course not!  This reconstruction can always be improved!  Part of the reason for porting this
algorithm into the Gulliver Framework was to make it easier for *anyone* with an idea of
how to improve it to implement and test their improvements.  Go play!  See the following section
for more details on how to muck around in the code and make changes.

I've got a new charge LDF.  How do I add it?
=========================================================

Currently, the only charge LDF option available is "dlp".
Details of the charge likelihood reside (of course) in the LikelihoodService,
and the relevant code is I3LaputopLikelihood.h and .cxx.  Here's the relevant bit::

  double local_ldf;
  if (fLDF == DLP_NAME)
     if ((fCoreCut <= 0.) || (local_r >= core_radius_)) {
       // A normal likelihood:          
       local_ldf = LateralFitFunctions::top_ldf_dlp(local_r, par);
     } else {
       // Let's try this: keeping LLH constant (rather than zero)   
       // inside the core cut                         
       log_debug("I'm using a radius of %f instead of %f", core_radius_, local_r);
       local_ldf = LateralFitFunctions::top_ldf_dlp(core_radius_, par);
     }
  else
    log_fatal("Sorry, no support yet for LDF function %s", fLDF.c_str());


Naturally, your new LDF would go where the existing code says "else log_fatal" at 
the end!
Notice that this bit of code is also where the "11-meter dynamic core cut" is 
done.  If you want Laputop to perform this same behavior for your new LDF, then 
your code will have to have a similar structure.  (I haven't come up with an 
elegant way of making the dynamic core cut "universal" for all LDF's... since
there is only one at the moment and I haven't been motivated to.)

The DLP function shows up again in the treatment of *saturated*
tanks... with a new function, this "if" statement will similarly have to be expanded::

  double local_ldf;
  if (fLDF == DLP_NAME)
     local_ldf = LateralFitFunctions::top_ldf_dlp(local_r, par);
  else
     log_fatal("Sorry, no support yet for LDF function %s", fLDF.c_str());


I'm sure there's a graceful way of defining the LDF function selection at the beginning, 
to alleviate the need for all the "if" statements, but I've been too lazy to do this
(again, because there is only one LDF available at the moment).
Input is welcome.

After this snippet of code, Laputop will correct the expected signal for snow,
and also compute the expected "sigma" of the signal.  If your LDF is associated
with different "sigmas", then you'll want to muck around with that part of the code
as well.


I've got a new timing likelihood.  How do I add it?
===========================================================

The timing likelihood is (of course) also a part of I3LaputopLikelihood.cxx.
Here is the relevant snippet::

  //////////////////////   
  // time-likelihood  //     
  //////////////////////                          
  if(combined_fit){
      double local_delta_t = GetDistToPlane(time,core,dir,inputData_.begin()+i);
      // For now, GAUSSPAR is the only one available. 
      llh += LateralFitFunctions::top_curv_gausspar_llh(local_r, local_delta_t, par); // par is NOT used in this function btw       
  }

Currently, "combined_fit" is just a boolean, which will be true if the user has 
selected "gausspar" as the curvature function (and wants to do a "combined fit" with 
both charge and timing likelihoods) and zero if the user has left a blank string.
If you want to add an additional LDF, this is where you'll want to add it.

I want to fit a new free parameter.  How do I do that?
==========================================================

Free parameters are controlled in the ParametrizationService.  There are two types of parameters
that can be varied by Gulliver: standard things that appear in an I3Particle (such as zenith,
azimuth, energy, time, etc.), and anything else, which are collectively called "non-standard"
parameters.  The non-standard free parameters currently available in Laputop are: S125, Beta, and
the SnowFactor.  

Gulliver can handle standard (I3Particle) parameters easily, but any non-standard parameters
need to be stored in some kind of structure, where Gullliver can fetch their initial values, 
store hypothesis values during minimization,
and eventually put their final values.  In Laputop, the I3LaputopParams structure
is used to store non-standard parameters such as S125, beta, errors, and correlation coefficients.

The purpose of the ParametrizationService is not only to set which parameters are fixed or free
(this is done in "Configure"), 
but also to provide Gulliver with tools for a) fetching free parameters *from* a hypothesis, 
and b) writing free parameters *into* a hypothesis.
The function which does the first is "UpdateParameters", 
and the function which does the second is "UpdatePhysicsParameters".
As you poke around these functions, notice that "standard" parameters are written to/from an 
I3Particle, while "non-standard" parameters are written to/from an I3LaputopParams structure.

Anytime you add a new potential free parameter to the ParametrizationService, you'll have to also add
a way to initialize it in the SeedService.  If there is a mismatch between the two, the code will 
return an error.

If your new free parameter is already part of the I3LaputopParams structure, then you're
in luck!  If not, you will have to *add* it to the structure.  It is part of the "recclasses" 
project.



