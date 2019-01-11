/**
 * Copyright (C) 2008
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3VEMCalData.h
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */


#ifndef _VEMCAL_I3VEMCALDATA_H_
#define _VEMCAL_I3VEMCALDATA_H_


#include <vector>
#include <stdint.h>

#include <icetray/I3FrameObject.h>
#include <icetray/I3DefaultName.h>
#include <dataclasses/I3Time.h>


/**
 * @brief This class is the main data container which
 * will be written to the frame. It stores all the information which
 * is needed to perform the muon calibration of the IceTop DOMs.
 */

static const unsigned i3vemcaldata_version_ = 2;

struct I3VEMCalData : public I3FrameObject
{

    /**
     * This structure stores the information of an IceTop
     * minimum bias hit.
     */
    struct MinBiasHit
    {
        /// String/Station number
        int8_t str; 
         
        /// DOM number
        int8_t om;

        /// Chip ID
        int8_t chip;

        /// Channel ID
        int8_t channel;

        /// Total charge of the hit in deci pe
        int16_t charge_dpe;
                
        MinBiasHit():
           str(-1),
           om(-1),
           chip(-1),
           channel(-1),
           charge_dpe(-10)   // -1 pe (unphysical value)
        {};
        
        virtual ~MinBiasHit();

        friend class icecube::serialization::access;
        template <class Archive> void serialize(Archive& ar, unsigned version);
    };
     
    
    /**
     * This structure stores the data to do the charge
     * correlation between the high gain (HG) and low gain (LG)
     * DOMs in the same IceTop tank.
     */
    struct HGLGhit
    {
        /// String/Station number
        int8_t str;
        
        /// Number of the HIGH gain DOM
        int8_t hg_om;

        /// Chip ID of the HIGH gain DOM
        int8_t hg_chip;

        /// Channel of the HIGH gain DOM
        int8_t hg_channel;
         
        /// Total charge of the HIGH gain hit
        uint16_t hg_charge_pe;
        
        /// Number of the LOW gain DOM
        int8_t lg_om;
        
        /// Chip ID of the LOW gain DOM
        int8_t lg_chip;

        /// Channel of the LOW gain DOM
        int8_t lg_channel;

        /// Total charge of the LOW gain hit
        uint16_t lg_charge_pe;
      
        /// Time difference between HG and LG signal in units of 2ns 
        int8_t deltat_2ns;
      
        HGLGhit():
          str(-1),
          hg_om(-1),
          hg_charge_pe(0),
          lg_om(-1),
          lg_charge_pe(0),
          deltat_2ns(0)
        {};
        
        virtual ~HGLGhit();
        
        friend class icecube::serialization::access;
        template <class Archive> void serialize(Archive& ar, unsigned version);
    };

    
    /// Run number
    uint32_t runID;
    
    /// Collection of minimum bias hits
    std::vector<MinBiasHit> minBiasHits;
    
    /// Collection of HG-LG correlation hits  
    std::vector<HGLGhit>    hglgHits;
    
    I3VEMCalData():
      runID(0)
    {};
      
    virtual ~I3VEMCalData();

    friend class icecube::serialization::access;
    template <class Archive> void serialize(Archive& ar, unsigned version);
};

I3_CLASS_VERSION(I3VEMCalData::MinBiasHit, i3vemcaldata_version_);
I3_CLASS_VERSION(I3VEMCalData::HGLGhit, i3vemcaldata_version_);
I3_CLASS_VERSION(I3VEMCalData, i3vemcaldata_version_);

I3_POINTER_TYPEDEFS(I3VEMCalData);
I3_DEFAULT_NAME(I3VEMCalData);

#endif
