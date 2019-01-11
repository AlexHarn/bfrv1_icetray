#ifndef TESTFRAME_H_INCLUDED
#define TESTFRAME_H_INCLUDED
/**
    copyright  (C) 2008
    the icecube collaboration
    $Id$

    @version $Revision: 30607 $
    @date $Date: 2007-03-23 16:18:39 -0500 (Fri, 23 Mar 2007) $
    @author boersma
*/

// NOTE: this is a modified copy of lilliput/private/test/TestFrame.h

#include <cmath>
#include <gsl/gsl_qrng.h>

#include "icetray/I3Frame.h"
#include "icetray/I3Units.h"
#include "icetray/I3Logging.h"
#include "icetray/OMKey.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/geometry/I3OMGeo.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "phys-services/I3Calculator.h"

I3Direction GetPerpDir(const I3Direction &p,double azimuth ){
    double px = p.GetX();
    double py = p.GetY();
    double pz = p.GetZ();
    double pxy = sqrt( px*px + py*py );
    I3Direction pd1, pd2;
    if      ( px == 0 ){ pd1 = I3Direction(0,1,0); pd2 = I3Direction(0,0,1); }
    else if ( py == 0 ){ pd1 = I3Direction(0,0,1); pd2 = I3Direction(1,0,0); }
    else if ( pz == 0 ){ pd1 = I3Direction(1,0,0); pd2 = I3Direction(0,1,0); }
    else {
        pd1 = I3Direction(py/pxy,-px/pxy,0);
        pd2 = I3Direction(pd1.GetY()*pz-pd1.GetZ()*py, 
                    pd1.GetZ()*px-pd1.GetX()*pz, 
                    pd1.GetX()*py-pd1.GetY()*px );
    }
    double cosazi = cos(azimuth);
    double sinazi = sin(azimuth);
    return I3Direction( cosazi*pd1.GetX() + sinazi*pd2.GetX(), 
                        cosazi*pd1.GetY() + sinazi*pd2.GetY(), 
                        cosazi*pd1.GetZ() + sinazi*pd2.GetZ() );
}

class TestFrame : public I3Frame {
private:
    std::string pulseName;
    std::string hitName;
    I3RecoPulseSeriesMapPtr testpulses;
    I3RecoHitSeriesMapPtr testhitses;
    I3GeometryPtr geo;
    std::vector<OMKey> omList_;
protected:
    int hitid;
public:
    TestFrame(std::string pulsename="testpulses",
              std::string hitname="testhits",
              unsigned int Nstring = 4,
              unsigned int Ndom = 60 ):
                      I3Frame(),
                      pulseName(pulsename),hitName(hitname),
                      testpulses (new I3RecoPulseSeriesMap),
                      testhitses (new I3RecoHitSeriesMap),
                      geo (new I3Geometry),
                      hitid(0){
        this->Put(pulseName,testpulses);
        this->Put(hitName,testhitses);
        this->Put(geo);
        int Ndiv = int(ceil(0.5+sqrt(1.0*Nstring)));
        for ( unsigned int istring = 0; istring < Nstring; ++istring ){
            double omx = 100.*(istring%Ndiv);
            double omy = 100.*(istring/Ndiv);
            for ( unsigned int idom = 0; idom < Ndom; ++idom ){
                double omz = 16.*idom;
                OMKey omkey(istring+1,idom+1);
                this->AddOM(omkey,omx,omy,omz);
                omList_.push_back(omkey);
            }
        }
    }

    void AddOM(OMKey om, double x, double y, double z ){
        I3OMGeo omgeo;
        omgeo.position = I3Position(x,y,z);
        omgeo.orientation = I3Orientation(0.,0.,-1., 1.,0.,0.);
        omgeo.area = M_PI*5*5*2.54*2.54*I3Units::cm*I3Units::cm;
        omgeo.omtype = I3OMGeo::IceCube;
        geo->omgeo[om] = omgeo;
    }

    /// add @c n pulses to the pulseseriesmap, with time residuals
    /// between tmin and tmax w.r.t. a given a particle p
    void AddPulses( I3ParticleConstPtr p, int n, int nseed=0 ){
        const size_t ndom = omList_.size();
        const int nsobol = 4; // OMKey, tres, width, charge
        gsl_qrng* generator = gsl_qrng_alloc( gsl_qrng_sobol, nsobol );
        double val[nsobol];
        while (nseed-->0) gsl_qrng_get(generator,val);
        while (n-->0){
            gsl_qrng_get(generator,val);
            const OMKey &om = omList_[size_t(val[0]*ndom)];
            const I3OMGeo &igeo = geo->omgeo[om];
            const I3Position &pos = igeo.position;
            double toff = p->GetTime();
            double tdirect = toff - I3Calculator::TimeResidual(*p,pos,toff);
            // generate some arbitrary hits
            double dist = I3Calculator::ClosestApproachDistance(*p,pos);
            double tmin = -10;
            double tmax = 5.0*dist;
            double tpulse = tdirect+val[1]*tmin+(1.0-val[1])*tmax;
            double charge = 0.5+val[2]*10.0;
            double width = 5*val[3]+sqrt(charge)*10.0;
            this->AddPulse(om,tpulse,charge,width);
        }
    }

    /// adds a pulse to the pulseseriesmap
    void AddPulse(const OMKey &om, double t, double c, double w ){
        // STL map takes care to create the series if it does not yet exist
        I3RecoPulseSeries &pulses = (*testpulses)[om];
        I3RecoPulse newpulse;
        newpulse.SetCharge(c);
        newpulse.SetWidth(w);
        newpulse.SetTime(t);
        pulses.push_back(newpulse);
    }

    /**
     * @returns a random OM key which is not yet in the Geometry
     */
    OMKey GetUnusedOMKey(){
        OMKey newkey;
        for ( int ntry = 0; ntry < 10000; ++ntry ){
            newkey.SetString( 1 + (rand() % 100 ) );
            newkey.SetOM( 1 + (rand() % 100 ) );
            if ( geo->omgeo.count(newkey) > 0 ) continue; // try again
            return newkey;
        }
        log_fatal( "can't generate new OMKey" );
        return newkey; // still return something (make compiler happy)
    }

    /**
     * @brief Adding a pulse with a given time delay w.r.t. a given track

     * @param p the given track
     * @param tdelay the time delay
     * @param perpdist the perpendicular distance from the track
     * @param chpoint the cherenkov emission point (distance from vertex, can be negative)
     * @param chazi arbitrary azimuth around the track
     * @param charge the pulse charge
     * @param width the pulse width
     *
     * @returns the cosine of the direction under which the downlooking OM is hit
     */
    double AddPulse( I3ParticleConstPtr p, double tdelay, double perpdist,
                     double chpoint, double chazi, double charge,
                     double width ){
        static const double nG = I3Constants::n_ice_group;
        static const double nP = I3Constants::n_ice_phase;
        static const double cvac = I3Constants::c;
        static const double cosC = 1.0/nP;
        static const double sinC = sqrt( 1.0 - 1.0/(nP*nP) );
        double chdist = perpdist / sinC;
        double direct_hittime = p->GetTime() + (chpoint + chdist*nG)/cvac;
        double rlong = chpoint + chdist / nP;
        I3Position pos = p->GetPos();
        I3Direction d = p->GetDir();
        I3Direction pd = GetPerpDir(d,chazi);

        double omx = pos.GetX() + d.GetX() * rlong + pd.GetX() * perpdist;
        double omy = pos.GetY() + d.GetY() * rlong + pd.GetY() * perpdist;
        double omz = pos.GetZ() + d.GetZ() * rlong + pd.GetZ() * perpdist;

        OMKey myom(GetUnusedOMKey());
        AddOM( myom, omx, omy, omz );
        AddPulse( myom, direct_hittime + tdelay, charge, width );

        // return cs_ori (z-component of photon direction at OM)
        return cosC * d.GetZ() + sinC * pd.GetZ();
    }

    void AddHit(OMKey om, double t ){
        I3RecoHitSeries &hitses = (*testhitses)[om];
        I3RecoHit newhit;
        newhit.SetTime(t);
        hitses.push_back(newhit);
    }

};

I3_POINTER_TYPEDEFS(TestFrame);

#endif  // TESTFRAME_H_INCLUDED
