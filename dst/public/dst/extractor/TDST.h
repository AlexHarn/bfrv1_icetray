/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: dst.h 16031 2006-02-20 12:38:45Z troy $

    @version $Revision: 1.2 $
    @date $Date: 2006-02-20 06:38:45 -0600 (lun, 20 feb 2006) $
    @author juancarlos@icecube.wisc.edu
    @brief TOject dataclass for storing DST data in a ROOT tree
*/

#ifndef TDST_H_INCLUDED
#define TDST_H_INCLUDED

#include <icetray/I3Units.h>
#include "dataclasses/I3Direction.h"
#include "dataclasses/I3Position.h"
#include <icetray/I3FrameObject.h>
#include <map>
//#include "TFormula.h"
//#include "TTree.h"


using namespace std;
static const unsigned tdst_version_  = 1;

class TDST: public I3FrameObject
{
	private:
		friend class icecube::serialization::access;
		template <class Archive> void save(Archive & ar, unsigned version) const;
		template <class Archive> void load(Archive & ar, unsigned version);

	public:

		TDST();
		virtual ~TDST();

		//void AddBranches(TTree*);
        template <typename Archive>
               void serialize(Archive&,unsigned);

		bool cut_nan();
					  
		// Event Weight  (for Simulated events)
		double weight;   
		double diplopiaweight;   
		double TimeScale;   
		float dt;   

        double mjd, usec, mjdTime;
        double localMST;
        double localAntiS;
        double localExtS;

        float llhAzimuth;
        float llhZenith;

        float lfAzimuth;
        float lfZenith;

        float linllhOpeningAngle;

        // RA, Dec in current epoch
        float RA;
        float Dec;

        // RA, Dec calculated with anti-sidereal time
        float RAAntiS;
        float DecAntiS;

        // RA, Dec calculated with solar time
        float RASolar;
        float DecSolar;

        // RA calculated with extended-sidereal time
        float RAExtS;

        // Position of the sun
        float RASun;
        float DecSun;

        // Position of the moon
        float RAMoon;
        float DecMoon;

        // Muon energy
        float logMuE;

        // Other geometrical quantities: impact parameter, center of gravity, etc.
        float rlogl;

        float sdcog;

        float cogx;
        float cogy;
        float cogz;

        uint32_t ldir;

        // Run information
        uint32_t runId;
        uint32_t eventId;
        uint32_t subEventId;
		uint32_t triggertag;   
		double time;   

        // Number of hits and strings
        uint32_t ndir;
        uint16_t nchan;
        uint16_t nstring;

        // Sub-run information
        uint16_t subrunId;
        bool isGoodLineFit;
        bool isGoodLLH;


};

I3_POINTER_TYPEDEFS(TDST);

#endif
