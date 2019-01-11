/**
 * Copyright (C) 2008
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3VEMCalData.cxx
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */

#include <vemcal/I3VEMCalData.h>
#include <icetray/serialization.h>

union SingleChipID {
  struct {
    #if BYTE_ORDER == BIG_ENDIAN
    uint8_t available : 1;
    uint8_t chip      : 1;
    uint8_t channel   : 2;
    uint8_t slop      : 4;   // unused
    #else
    uint8_t slop      : 4;   // unused
    uint8_t channel   : 2;
    uint8_t chip      : 1;
    uint8_t available : 1;
    #endif
  } fields;
  uint8_t bits;

  SingleChipID() {
    fields.available = 0;
    fields.channel = 0;
    fields.chip = 0;
  }
} __attribute((packed));

union ChipPairID {
  struct {
    #if BYTE_ORDER == BIG_ENDIAN
    uint8_t hg_available : 1;
    uint8_t hg_chip      : 1;
    uint8_t hg_channel   : 2;
    uint8_t lg_available : 1;
    uint8_t lg_chip      : 1;
    uint8_t lg_channel   : 2;
    #else
    uint8_t lg_channel   : 2;
    uint8_t lg_chip      : 1;
    uint8_t lg_available : 1;
    uint8_t hg_channel   : 2;
    uint8_t hg_chip      : 1;
    uint8_t hg_available : 1;
    #endif
  } fields;
  uint8_t bits;

  ChipPairID() {
    fields.hg_available = 0;
    fields.hg_chip = 0;
    fields.hg_channel = 0;
    fields.lg_available = 0;
    fields.lg_chip = 0;
    fields.lg_channel = 0;
  }
} __attribute((packed));


I3VEMCalData::MinBiasHit::~MinBiasHit() {}

template <class Archive>
void I3VEMCalData::MinBiasHit::serialize(Archive& ar, unsigned version)
{
    if(version>i3vemcaldata_version_)
    {
        log_fatal("Attempting to read version %u from file but running version %u "
                  "of I3VEMCalData class.", version, i3vemcaldata_version_);
    }
    
    ar & make_nvp("str",          str);
    ar & make_nvp("om",           om);
    ar & make_nvp("charge_dpe",   charge_dpe);
    
    if(version<1)
    {
        // These members only existed in version 0
        // but were never used by any other program 
        int16_t dummy = 0;
        ar & make_nvp("zenith_cdeg",  dummy);
	ar & make_nvp("velocity_ccc", dummy);
    }

    if(version>1)
    {
	SingleChipID chipid;
	if (chip >= 0) {
	  chipid.fields.available = 1;
	  chipid.fields.chip = chip;
	  chipid.fields.channel = channel;
	}
	ar & make_nvp("chipid", chipid.bits);
	if (chipid.fields.available) {
	  chip = chipid.fields.chip;
	  channel = chipid.fields.channel;
	} else {
	  chip = -1;
	  channel = -1;
	}
    }
}

I3_SERIALIZABLE(I3VEMCalData::MinBiasHit);


/////////////////////////////////////////////////////////


I3VEMCalData::HGLGhit::~HGLGhit() {}

template <class Archive>
void I3VEMCalData::HGLGhit::serialize(Archive& ar, unsigned version)
{
    if(version>i3vemcaldata_version_)
    {
        log_fatal("Attempting to read version %u from file but running version %u "
                  "of I3VEMCalData class.", version, i3vemcaldata_version_);
    }
    
    ar & make_nvp("str",          str);
    ar & make_nvp("hg_om",        hg_om);
    ar & make_nvp("hg_charge_pe", hg_charge_pe);
    ar & make_nvp("lg_om",        lg_om);
    ar & make_nvp("lg_charge_pe", lg_charge_pe);
    
    if(version>0) ar & make_nvp("deltat_2ns", deltat_2ns);

    if(version>1)
    {
	ChipPairID chipid;
	if (hg_chip >= 0) {
	  chipid.fields.hg_available = 1;
	  chipid.fields.hg_chip = hg_chip;
	  chipid.fields.hg_channel = hg_channel;
	}
	if (lg_chip >= 0) {
	  chipid.fields.lg_available = 1;
	  chipid.fields.lg_chip = lg_chip;
	  chipid.fields.lg_channel = lg_channel;
	}
	ar & make_nvp("chipid", chipid.bits);
	if (chipid.fields.hg_available) {
	  hg_chip = chipid.fields.hg_chip;
	  hg_channel = chipid.fields.hg_channel;
	} else {
	  hg_chip = hg_channel = -1;
	}
	if (chipid.fields.lg_available) {
	  lg_chip = chipid.fields.lg_chip;
	  lg_channel = chipid.fields.lg_channel;
	} else {
	  lg_chip = lg_channel = -1;
	}
    }
}

I3_SERIALIZABLE(I3VEMCalData::HGLGhit);


/////////////////////////////////////////////////////////


I3VEMCalData::~I3VEMCalData() {}

template <class Archive>
void 
I3VEMCalData::serialize(Archive& ar, unsigned version)
{
    if(version>i3vemcaldata_version_)
    {
        log_fatal("Attempting to read version %u from file but running version %u "
                  "of I3VEMCalData class.", version, i3vemcaldata_version_);
    }
    
    ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
    ar & make_nvp("runID",         runID);
    ar & make_nvp("minBiasHits",   minBiasHits);
    ar & make_nvp("hglgHits",      hglgHits);
}

I3_SERIALIZABLE(I3VEMCalData);
