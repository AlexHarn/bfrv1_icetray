/**
    copyright  (C) 2011
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author boersma
*/


// generic stuff
#include "TestFrame.h"
#include "icetray/I3Units.h"
#include "icetray/I3Logging.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3OMGeo.h"

#include "gsl/gsl_randist.h"
#include <math.h>

I3Direction TestFrame::GetPerpDir(const I3Direction &p,double azimuth ){
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

TestFrame::TestFrame(std::string pulsename): I3Frame('P'),
                                   testPulses_ (new I3RecoPulseSeriesMap),
                                   geo_ (new I3Geometry){
    this->Put(pulsename,testPulses_);
    this->Put(geo_);
}

void TestFrame::AddOM(OMKey om, double x, double y, double z ){
    I3OMGeo omgeo;
    omgeo.position = I3Position(x,y,z);
    omgeo.orientation = I3Orientation(0.,0.,-1., 1.,0.,0.);
    omgeo.area = M_PI*5*5*2.54*2.54*I3Units::cm*I3Units::cm;
    omgeo.omtype = I3OMGeo::IceCube;
    geo_->omgeo[om] = omgeo;
}

// adds a pulse to the pulseseriesmap
void TestFrame::AddPulse(OMKey om, double t, double c, double w ){
    // STL map takes care to create the series if it does not yet exist
    I3RecoPulseSeries &pulses = (*testPulses_)[om];
    I3RecoPulse newpulse;
    newpulse.SetCharge(c);
    newpulse.SetWidth(w);
    newpulse.SetTime(t);
    newpulse.SetFlags(I3RecoPulse::ATWD);
    pulses.push_back(newpulse);
}

// returns a random OM key which is not yet in the Geometry
OMKey TestFrame::GetUnusedOMKey(){
    OMKey newkey;
    for ( int ntry = 0; ntry < 10000; ++ntry ){
        newkey.SetString( 1 + (rand() % 100 ) );
        newkey.SetOM( 1 + (rand() % 100 ) );
        if ( geo_->omgeo.count(newkey) > 0 ) continue; // try again
        return newkey;
    }
    log_fatal( "can't generate new OMKey" );
    return newkey; // still return something (make compiler happy)
}

// Adding a pulse with a given time delay w.r.t. a given track
double TestFrame::AddPulse( I3ParticleConstPtr p, double tdelay, double perpdist,
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


I3TestFrameSource::I3TestFrameSource(const I3Context& ctx) : I3Module(ctx){
    AddOutBox("OutBox");
    rng_ = gsl_rng_alloc( gsl_rng_knuthran2 );
    trackName_ = "bogotrack";
    pulsesName_ = "bogopulses";
    eMin_ = 1.e2*I3Units::GeV;
    eMax_ = 1.e10*I3Units::GeV;
    AddParameter("TrackName","Name of track",trackName_);
    AddParameter("PulsesName","Name of pulses",pulsesName_);
    AddParameter("MinEnergy","Minimum energy",eMin_);
    AddParameter("MaxEnergy","Maximum energy",eMax_);
}

void I3TestFrameSource::Configure(){
    GetParameter("TrackName",trackName_);
    GetParameter("PulsesName",pulsesName_);
    GetParameter("MinEnergy",eMin_);
    GetParameter("MaxEnergy",eMax_);
}

I3TestFrameSource::~I3TestFrameSource(){
    gsl_rng_free( rng_ );
}

void I3TestFrameSource::Process(){
    TestFramePtr frame(new TestFrame(pulsesName_));
    I3ParticlePtr p(new I3Particle( I3Particle::InfiniteTrack, I3Particle::MuPlus));
    p->SetTime( 1.e4*I3Units::ns );
    p->SetDir( acos(gsl_ran_flat(rng_,-0.999,+0.999)), gsl_ran_flat(rng_,0.,360.*I3Units::degree));
    p->SetPos( gsl_ran_flat(rng_,-500.*I3Units::m,+500.*I3Units::m),
               gsl_ran_flat(rng_,-500.*I3Units::m,+500.*I3Units::m),
               gsl_ran_flat(rng_,-500.*I3Units::m,+500.*I3Units::m) );
    /**
     * pdf = N energie^-2
     * 1 := int(pdf)E_0,E_1 = N * -1 (energy^-1)(E_0,E_1)
     *                      = N * (1./E_0 - 1./E_1)
     * N = E_0 * E_1 / (E_1 - E_0)
     * cumulative(energy) = N * ( E_0^-1 - energy^-1 )
     *                    = E_1 * (1 - E_0/energy)/ (E_1 - E_0)
     * cumulative^-1(x) = E_1 * E_0 / (E_1 - (E_1 - E_0) * x )
     */
    p->SetEnergy(eMin_*eMax_/(eMax_-(eMax_-eMin_)*gsl_ran_flat(rng_,0.,1.)));
    int nch=10+gsl_ran_poisson(rng_,20);
    for (int ch=0; ch<nch;++ch){
        double perpdist = gsl_ran_exponential(rng_,30.*I3Units::m);
        double tdelay = (2.+gsl_ran_landau(rng_))*perpdist/(30.*I3Units::m);
        double chpoint = gsl_ran_gaussian(rng_,100.*I3Units::m);
        double chazi = gsl_ran_flat(rng_,0.,360.*I3Units::degree);
        double charge = 0.5*(1+gsl_ran_poisson(rng_,3.));
        double width = 2.5*sqrt(charge);
        frame->AddPulse(p,tdelay,perpdist,chpoint,chazi,charge,width);
    }
    log_trace( "generating event with nch=%d and zenith=%g",
              nch,p->GetDir().GetZenith()/I3Units::degree );
    frame->Put(trackName_,p);
    // Each event has a different geometry, so we have to push
    // a new Geoemtry frame for each Physics frame
    I3FramePtr geo_frame(new I3Frame(I3Frame::Geometry));
    const I3GeometryConstPtr& geo = frame->Get<I3GeometryConstPtr>("I3Geometry");
    geo_frame->Put("I3Geometry",geo);
    PushFrame(geo_frame);
    PushFrame(frame);
}

I3_MODULE(I3TestFrameSource);
