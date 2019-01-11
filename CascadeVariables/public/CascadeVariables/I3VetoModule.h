/**
 *  @file I3VetoModule.h
 *  @version $Id$
 *  @date $Date$
 *  @author $Author$
*/

#ifndef I3VETOMODULE_H
#define I3VETOMODULE_H


//#include "icetray/IcetrayFwd.h"
#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "icetray/OMKey.h"



/**
 * @class I3VetoModule
 * @brief IceTray module to fill an I3Veto object from a given response map
 *       
 */
 
 
namespace I3VetoDetail {
    struct DOMInfo {
        OMKey key;
        double charge;
        double firstPulseTime;

        bool operator<(const DOMInfo& other) const{
            return this->firstPulseTime < other.firstPulseTime;
        }
    };
    
    struct VetoParams {
        VetoParams() : nUnhitTopLayers(std::numeric_limits<short>::min()),
                       nLayer(std::numeric_limits<short>::min()),
                       earliestLayer(std::numeric_limits<short>::min()),
                       earliestOM(std::numeric_limits<short>::min()),
                       earliestContainment(std::numeric_limits<short>::min()),
                       latestLayer(std::numeric_limits<short>::min()),
                       latestOM(std::numeric_limits<short>::min()),
                       latestContainment(std::numeric_limits<short>::min()),
                       mostOuterLayer(std::numeric_limits<short>::min()),
                       depthHighestHit(NAN),
                       depthFirstHit(NAN),
                       maxDomChargeLayer(std::numeric_limits<short>::min()),
                       maxDomChargeString(std::numeric_limits<short>::min()),
                       maxDomChargeOM(std::numeric_limits<short>::min()),
                       nDomsBeforeMaxDOM(std::numeric_limits<short>::min()),
                       maxDomChargeLayer_xy(std::numeric_limits<short>::min()),
                       maxDomChargeLayer_z(std::numeric_limits<short>::min()),
                       maxDomChargeContainment(std::numeric_limits<short>::min()) {}

        short nUnhitTopLayers;
        short nLayer;
        short earliestLayer;
        short earliestOM;
        short earliestContainment;
        short latestLayer;
        short latestOM;
        short latestContainment;
        short mostOuterLayer;
        double depthHighestHit;
        double depthFirstHit;
        short maxDomChargeLayer;
        short maxDomChargeString;
        short maxDomChargeOM;
        short nDomsBeforeMaxDOM;
        short maxDomChargeLayer_xy;
        short maxDomChargeLayer_z;
        short maxDomChargeContainment;
        };

}

/*****************************************************************************/



class I3VetoModule : public I3ConditionalModule {
    public:
        I3VetoModule(const I3Context& ctx);
        virtual ~I3VetoModule();
        
        /**
        * Load the configuration parameters.
        */
        void Configure();

        /**
        * The actual work is performed in the Physics method.
        */
        void Physics(I3FramePtr frame);

    private:
        
        // hidden constructors
        I3VetoModule();
        I3VetoModule(const I3VetoModule&);
        I3VetoModule& operator=(const I3VetoModule&);
        
        // private member variables
        std::string hitmapName_;
        std::string outputName_;
	    bool useAMANDA_;
	    short detectorGeometry_;
	    bool writeFullOutput_;

        std::vector<std::vector<short> > layerStrings_;
        
        // this does the actual work of the Physics method
        I3VetoDetail::VetoParams CalcFromPulseMap(I3RecoPulseSeriesMapConstPtr pulsemap, 
			I3GeometryConstPtr geometry,
			const std::vector<std::vector<short> >& layers,
			bool useAMANDA);

        // logger 
        SET_LOGGER("I3VetoModule");
};

#endif 
