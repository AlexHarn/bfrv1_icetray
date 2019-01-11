/**
 * class: I3mue.C
 *
 * Version $Id$
 *
 * Date: Tue November 1 2005
 *
 * (c) 2005 IceCube Collaboration
 */

#include "mue/I3mue.h"
#include "reader.h"
#include "llhreco.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3Particle.h"

#include <sstream>

using namespace std;
using namespace I3FRreco;

I3_MODULE(I3mue);

I3mue::I3mue(const I3Context& ctx) : I3ConditionalModule(ctx){
  fRecoPulseSeriesNames.push_back("InitialPulseSeriesRecoNames");
  AddParameter("RecoPulseSeriesNames","Reco Pulses to process",fRecoPulseSeriesNames);

  fDataReadoutName = "RawData";
  AddParameter("DataReadoutName","name of the recopulses vector",fDataReadoutName);

  fRecoResult = "RecoResult";
  AddParameter("RecoResult","name of the reconstruction result to use",fRecoResult);

  fRecoRnum = 8;
  AddParameter("RecoRnum","required trigger multiplicity",fRecoRnum);

  fRecoIntr = 0;
  AddParameter("RecoIntr","cpandel parameterization mode",fRecoIntr);

  fRecoIntr = 0;
  AddParameter("OutputParticle","name of the output particle",fOutputParticle);

  fVerbose = 0;
  AddParameter("Verbose","Enable event printout on cout",fVerbose);

  AddOutBox("OutBox");
}

I3mue::~I3mue(){
  pp_stop();
}

// transitions
void I3mue::Configure(){
  log_info("Configuring the reco");

  GetParameter("RecoPulseSeriesNames",fRecoPulseSeriesNames);
  GetParameter("DataReadoutName",fDataReadoutName);
  GetParameter("RecoResult",fRecoResult);
  GetParameter("RecoRnum",fRecoRnum);
  GetParameter("RecoIntr",fRecoIntr);
  GetParameter("OutputParticle",fOutputParticle);
  GetParameter("Verbose",fVerbose);

  log_info("Running I3mue with parameters:");
  for(uint32_t i=0; i<fRecoPulseSeriesNames.size(); i++) 
    log_info("RecoPulseSeriesNames[%d]=%s",i,fRecoPulseSeriesNames[i].c_str());
  log_info("DataReadoutName=%s",fDataReadoutName.c_str());
  log_info("RecoResult=%s",fRecoResult.c_str());
  log_info("RecoRnum=%d",fRecoRnum);
  log_info("RecoIntr=%d",fRecoIntr);
  log_info("OutputParticle=%s",fOutputParticle.c_str());
  log_info("Verbose=%d",fVerbose);

  fInitialized=false;
  fDetCenterDepth=-1940.93;

  amain(fRecoRnum, fRecoIntr);

  if( fOutputParticle.length() == 0 )
  {
    stringstream ss;
    ss << fRecoResult << "_e";
    fOutputParticle = ss.str();
  }
  
  pp_start(1);
}

double res(double x, double mod){
  return x-mod*int(x/mod);
}

void I3mue::Physics(I3FramePtr frame)
{
   log_trace("Entering I3mue::Physics()");

   //  Retrieve geometry and status information

   if(!fInitialized){
     const I3OMGeoMap& geo = frame->Get<I3Geometry>().omgeo;
     const map<OMKey,I3DOMStatus>& detectorStatus = frame->Get<I3DetectorStatus>().domStatus;

     for(I3OMGeoMap::const_iterator OMiter = geo.begin(); OMiter != geo.end(); ++OMiter){

       OMKey omkey = OMiter->first;
       unsigned long long domid;

       domid=1000*omkey.GetString()+omkey.GetOM();
       fIds[omkey]=domid;

       const I3OMGeo& thisomgeo = OMiter->second;
       I3Position pos = thisomgeo.position;

       double hv=0;
       if(detectorStatus.count(omkey)){
	 const I3DOMStatus& status = detectorStatus.find(omkey)->second;
	 hv = status.pmtHV/I3Units::volt;
       }
       setgeo(domid, omkey.GetOM(), omkey.GetString(),
	      pos.GetX()/I3Units::m, pos.GetY()/I3Units::m, pos.GetZ()/I3Units::m+fDetCenterDepth, hv);
     }

     fEvent=0;
     fToffset=1000000;
     fInitialized=true;
   }
   fEvent++;

   multe events;

   // get the data from the event

   for(uint32_t i=0; i<fRecoPulseSeriesNames.size(); i++){
     I3RecoPulseSeriesMapConstPtr omdata = frame->Get<I3RecoPulseSeriesMapConstPtr>(fRecoPulseSeriesNames[i]);
     if(omdata) for(I3RecoPulseSeriesMap::const_iterator OMiter = omdata->begin(); OMiter != omdata->end(); ++OMiter){
       // get the recopulse data
       const I3RecoPulseSeries& pseries=OMiter->second;

       if(pseries.size()>0){
	 OMKey omkey = OMiter->first;
	 unsigned long long domid;

	 // get the DOM MB id
	 domid=fIds[omkey];

	 // fill wform with pulse data (this speeds up access to the pulse data)

	 wform *wf = new wform();
	 wf->id=domid;

	 int i=0;
	 for(I3RecoPulseSeries::const_iterator hit=pseries.begin(); hit!=pseries.end(); ++hit){

	   // fill event objects with pulse data
	   event ev(wf);
	   double T=hit->GetTime()/I3Units::ns;
	   ev.gt=(unsigned long long)((hit->GetTime()/I3Units::ns+fToffset)*10);
	   ev.Q=hit->GetCharge();
	   ev.W=hit->GetWidth();
	   log_debug("hit at time=%g, width=%g ns", T, ev.W);

	   if(ev.Q!=ev.Q || ev.Q+1==ev.Q){ // monolith bug work-around
	     log_warn("I3mue: charge is a %g, setting to 1", ev.Q);
	     ev.Q=1;
	   }

	   ev.ijs=i++;
	   ev.fi=0;

	   events.events.push_back(ev);
	   // cout<<fEvent<<" "<<i<<" "<<*ev;
	 }
       }
     }
   }

   // Get the track
   I3ParticleConstPtr track = frame->Get<I3ParticleConstPtr>(fRecoResult);
   if(track){
     preco tr;

     tr.type=-1;
     tr.th=180-track->GetZenith()/I3Units::degree;
     tr.ph=res(180+track->GetAzimuth()/I3Units::degree, 360);
     tr.t0=track->GetTime()/I3Units::ns;
     tr.x0=track->GetX()/I3Units::m;
     tr.y0=track->GetY()/I3Units::m;
     tr.z0=track->GetZ()/I3Units::m-1940.93;

     inimctrack(events, tr);
     I3ParticlePtr new_track = I3ParticlePtr(new I3Particle(*track));
     
     // M. Merck: Conversion to energy. 
     // See Dima's ICRC paper http://arxiv.org/ps/0711.0353v1 p. 63
     double energy = tr.n0 / ( 20.2e-4 * 32240 * 1.36e-3 );
     new_track->SetEnergy( energy * I3Units::GeV );

     frame->Put(fOutputParticle, new_track);

     if(fVerbose) cout << tr << endl << flush;
   }

   PushFrame(frame,"OutBox");
   log_trace("Exiting I3mue::Physics()");
}

void I3mue::Geometry(I3FramePtr frame){
  fInitialized=false;
  PushFrame(frame,"OutBox");
}

void I3mue::Calibration(I3FramePtr frame){
  fInitialized=false;
  PushFrame(frame,"OutBox");
}

void I3mue::DetectorStatus(I3FramePtr frame){
  fInitialized=false;
  PushFrame(frame,"OutBox");
}
