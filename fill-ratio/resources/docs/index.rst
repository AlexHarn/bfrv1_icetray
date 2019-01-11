.. _fill-ratio:

==========
Fill Ratio
==========

Fill-ratio is maintained by Michael Larson (mjlarson@nbi.ku.dk).

Overview
========

The fill-ratio is an algorithm that is meant to distinguish track-like
events from point-like events. The main idea is that, given a vertex
hypothesis (e.g. from c-first), hits should be distributed about that
position in some manner. This algorithm first calculates the mean and
width of that distribution. Two spheres are drawn around the vertex position
with radii of a few times the RMS and a few times the mean that were 
previously calculated. For each sphere the number of hit OMs are counted 
contained therein, and the total number of OMs contained therein. The 
fill-ratios for each sphere, as well as the mean and RMS, and the sphere
radii are filled into a frame object, which is then pushed into the frame.


Algorithm
=========

Fill-ratio is a project introduced in the AMANDA days to separate out tracks from cascades at a few hundred GeV. The concept is relatively straightforward and can be described in roughly four steps:

1. Pick a vertex and hit series. This will define all later steps, with a moderately good vertex giving separation between tracks and cascades. The hit series can be cleaned or uncleaned: In general, a cleaned hit series will benefit the variable, although you will need to optimize this for your own analysis.

2. Define a "radius" to use. Fill-ratio automatically attempts to use five different radii:
   a. "FillRatio": The radius is defined by the distance R between the vertex and each hit:
      
      .. math::
         r_{rms} = \sqrt{\left(\sum{\vec{R}\cdot\vec{r}}\right) - \left(\sum{\vec{R}}\right)^2}

   b. "FillRatioFromMean": The radius is defined by the average distance between each hit in and vertex
      
      .. math::
         r_{mean} = \frac{1}{N}\sum{\left|\vec{R}\right|}

   c. "FillRatioFromMeanPlusRMS": Use both of the above definitions to pick the radius:
      
      .. math::
         r_{mean\;rms} = r_{mean} + r_{rms}

   d. "FillRatioFromEnergy": Define an "SPE radius" based on the reconstructed energy of the given particle. This is an OLD calculation and should be considered unreliable/deprecated!
      
      .. math::
         r_{Energy} = -0.6441 + 36.9 * Log_{10}(E)

   e. "FillRatioFromNCh": Define an "SPE radius" based on the number of hit DOMs in the given hit series. First the nChannel is converted to an "energy". The energy is then passed to the same calculation as the "FillRatioFromEnergy". This is an OLD calculation and should be considered unreliable/deprecated!
      
      .. math::
         E_{nCh} = 0.084 + 1.83 Log_{10}(nCh)

3. Once a radius is calculated, find all DOMs within a sphere centered on the given reconstructed vertex with radius given by one of the five methods in (2). 

4. Return the fraction of hit DOMs inside the sphere to unhit DOMs in the sphere. 
   
This supposes that the event is likely to be spherically symmetric if it is cascade-like (ie, close to 1) and relatively empty if track-like (ie, close to 0). 

I3FillRatioMoudle returns a specialized object, the I3FillRatioInfo. This object is legacy and can be converted to a standard I3MapStringDouble using I3FillRatio2StringDoubleMap. This is also done by default using the included tray segment. 

I3FillRatioLite is an updated version of the I3FillRatioModule code and only performs the calculation using the radius defined by the mean distance (ie, option 2.b above). It offers a cleaner interface and should be preferred for new users.

Usage
=====
**I3FillRatioModule** (C++ Module)

  *VertexName*
    Name of the I3Particle in the frame to use as a vertex

  *ResultName*
    The name that the resulting I3FillRatioInfo object will take in the frame

  *RecoPulseName*
    The name of the recopulses to be used
  
  *SphericalRadiusRMS*
    The radius (in units of the RMS) of the sphere to be used to calculate the fill ratio.

  *SphericalRadiusMean*
    The radius (in units of the mean) of the sphere used to calculate the fill ratio.

  *SphericalRadiusMeanPlusRMS*
    The radius (in units of the mean plus rms) of the sphere used to 
    calculate the fill ratio.

  *SphericalRadiusNCh*
    The radius (in units of the SPE Radius) of the sphere used to define the 
    fill-ratio

  *AmplitudeWeightingPower*
    The means and RMSs can be weighted by the charge of the hit. 
    This parameter sets the power for the exponential weighting.

  *BadDOMList*
    Name of the BadDOMList in the DetectorStatus frame

  *BadOMs*
    A user-defined vector of OMKeys that should not 
    contribute to the calculation of the fill-ratio. 

**I3FillRatioLite** (C++ Module)

   *Vertex*
     Name of the particle to use as a vertex

   *Pulses*
     Name of the pulse series in the frame

   *Output*
     Name of the I3Double to put in the frame

   *Scale*
     Factor by which to scale the mean vertex-DOM distance

   *AmplitudeWeightingPower*
     Weight hits by the charge to this power

   *ExcludeDOMs*
     List of DOMs to exclude from the calculation

   *BadDOMListName*
      Name of the dynamic BadDOMList in the DetectorStatus frame


**FillRatio** (Tray Segment)

  *VertexName*
    Name of the I3Particle in the frame to use as a vertex

  *ResultName*
    The name that the resulting I3FillRatioInfo object will take in the frame"

  *MapName*
       The name that the I3MapStringDouble object will take in the frame"

  *RecoPulseName*
    The name of the recopulses to be used

  *SphericalRadiusRMS*
    The radius (in units of the RMS) of the sphere to be used to calculate the fill ratio.

  *SphericalRadiusMean*
    The radius (in units of the mean) of the sphere used to calculate the fill ratio.

  *SphericalRadiusMeanPlusRMS*
    The radius (in units of the mean plus rms) of the sphere used to calculate the fill ratio.

  *SphericalRadiusNCh*
    The radius (in units of the SPE Radius) of the sphere used to define the fill-ratio

  *AmplitudeWeightingPower*
    The means and RMSs can be weighted by the charge of the hit. 
    This parameter sets the power for the exponential wieghting.

 *If*
    the usual python function, makes the segment run conditionally frame by frame. 
        

See Also
========

.. toctree::
   :maxdepth: 1

   release_notes
   Python API Reference </python/icecube.fill_ratio>
   C++ API Reference </doxygen/fill-ratio/index>
   IceTray Inspect Reference </inspect/fill_ratio>

