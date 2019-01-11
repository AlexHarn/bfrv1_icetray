/**
 * copyright (C) 2006
 * the IceCube collaboration
 * @version $Id$
 * @version $Revision: $
 * @file I3FillRatioModule.h
 * @author Doug Rutledge
 * @date 26Aug2006
 */

#ifndef FILLRATIO_I3FILLRATIOMODULE_H_INCLUDED
#define FILLRATIO_I3FILLRATIOMODULE_H_INCLUDED

#include "icetray/I3ConditionalModule.h"
#include "icetray/I3TrayHeaders.h"

#include "icetray/OMKey.h"

/**
 * @brief An instance of the fill-ratio algorithm.
 */

class I3FillRatioModule : public I3ConditionalModule
{
  public:
    /**
     * Creates an instance of this class.
     * @param context -- the I3Context that will be given to this class by 
     * the IceTray framework, and that gives it access to the services. This
     * module doesn't actually use any services at the moment, but this is
     * how modules are instantiated in IceTray.
     */

    I3FillRatioModule(const I3Context& context);

    /**
     * Destroys an instance of this class.
     */
    virtual ~I3FillRatioModule();
    
    //Overridden methods of I3Module
    /**
     * Fetch any user-defined parameter from the steering file and configure
     * this module.
     */
    void Configure();

    /**
     * Clean up any thing that needs to be done for an entire run, as 
     * opposed to a single event.
     */ 
    void Finish();
    
    /**
     * Operate on one event. Collect a distribution of distances from hit OMs
     * to the vertex. Calculate the mean and RMS. Draw two spheres around the
     * vertex, with radii of some user-defined numbers times the mean and RMS,
     * respectively. Count the fraction of OMs inside this sphere which are 
     * hit.
     *
     * @param frame -- the event to operate on.
     */
    void Physics(I3FramePtr frame);
    
    void DetectorStatus(I3FramePtr frame);

  private:
    //default constructor, copy constructor, and assigment operator.
    //declared private and left un-defined to prevent accidental
    //creation and other bad situations.
    I3FillRatioModule();
    I3FillRatioModule(const I3FillRatioModule&);
    I3FillRatioModule& operator=(const I3FillRatioModule&);

    //private member function
    /**
     * CalculateWeight is a convenience method for determining the
     * weight of a hit, given the charge recorded in the waveform.
     * The weight will be calculated from an exponential distribution,
     * with the scale determined by the user's input. A scale factor of
     * zero will return a weight of 1.0, and a scale factor of 1.0 will
     * return the charge as the weight.
     *
     * @param charge The charge from the pulse series for a given OM.
     * @return The weight used to calculate how the charge is distributed 
     * about the COG.
     */
    double CalculateWeight(double charge);

    /**
     * Estimate the energy from the NCh. This is used by the NCh fill ratio to 
     * find the energy estimate, which is then given to the 
     * "EstimateSPERadiusFromEnergy" method to find the radius if removal. 
     * Diffent detector configurations have different calibrations for this.
     *
     * @param nCh the NChannels from whatever pulse series is being used.
     * 
     * @return The estimated log base 10 of the Energy in GeV.
     */
    double EstimateEnergyFromNCh(int nCh);

    /**
     * Estimate the SPE Radius from the energy, which is either obtained from
     * the NCh or from apreiously-reconstructed vertex. 
     *
     * @param log10E Log base 10 of the energy estimate.
     *
     * @return The estimated SPE Radius, which is then used to define the
     * NCh and energy fill ratios.
     */
    double EstimateSPERadiusFromEnergy(double log10E);
  private:
   
   //the following variables are for the configurable parameters
   /**
    * The name of the (previously reconstructed) vertex
    */
   std::string vertexName_;

   /**
    * The name of the I3FillRatioInfo which will be pushed into the frame.
    */
   std::string resultName_;

   /**
    * The name of the RecoPulseSeriesMap to be used for the calculation.
    */
   std::string recoPulseName_;

   /**
    * A user-defined vector of OMs to exclude from this analysis. This is
    * useful if one is running a single-detector-only analysis. Otherwise,
    * a large number of UnHit-able OMs will be found in the geometry, and
    * skew the result.
    */
   std::string badDOMListName_;
   std::vector<OMKey> staticBadDOMList_;
   std::set<OMKey> badOMs_;

   /**
    * The radius of the rms-defined shere, in units of the RMS.
    */
   double rmsSphereRadius_;

   /**
    * The radius of the mean-defined spere, in units of the mean.
    */
   double meanSphereRadius_;

   /*
    * The radius of the mean plus rms-defined sphere in units of mean + rms.
    */
   double meanRMSSphereRadius_;

   /**
    * The radius of the nCh-defined spere, in units of the SPE Radius.
    */
   double nChSphereRadius_;

   /**
    * The radius of the energy-defined SPE spere, in units of the SPE Radius.
    */
   double energySphereRadius_;

   /**
    * The distance distribution can be weighted by the charge recorded in
    * the hits. The weighting is defined by a exponential. This defines the
    * power of the exponential. Use a value of 0 to refrain from weighting,
    * and a value of 1 to weight each hit by its charge.
    */
   double ampWeightPower_; 

   /**
    * Counter for number of events seen (for use in debug output).
    */
   int eventsSeen_;

   /**
    * Set a minimum radius for the Energy estimate fill ratios 
    * (Energy and NCh fillratios).
    */
   double minRadius_;

   SET_LOGGER("I3FillRatioModule"); 

};

#endif // FILLRATIO_I3FILLRATIOMODULE_H_INCLUDED
