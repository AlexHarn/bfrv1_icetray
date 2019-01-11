#include <icetray/I3ConditionalModule.h>

#include <dataclasses/I3Double.h>
#include <dataclasses/physics/I3MCTree.h>
#include <dataclasses/physics/I3MCTreeUtils.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/physics/I3Particle.h>

#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/calibration/I3Calibration.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_hyperg.h>
#include <gsl/gsl_sf_lambert.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_multimin.h>

#include <fstream>
#include <limits.h>

using namespace std;

#include "llhreco.cxx"

using namespace MUEX;

typedef std::vector<OMKey> OMKeySeries;
I3_POINTER_TYPEDEFS(OMKeySeries);

namespace {

bool is_inice(I3OMGeo::OMType omtype)
{
    return (omtype == I3OMGeo::IceCube || omtype == I3OMGeo::UnknownType);
}

}

class muex : public I3ConditionalModule{

  bool ini;
  std::string pulses;
  std::string badoms;
  std::string rectrk;
  std::string result;
  std::string elistn;
  std::string icedir;
  unsigned int lcsp, repeat;
  bool rectyp, usempe, detail, energy, compat;

  SET_LOGGER("muex");

public:

  muex(const I3Context& ctx) : I3ConditionalModule(ctx), ini(false){
    pulses = "InPulses";
    AddParameter("pulses", "input pulse series", pulses);

    badoms = "";
    AddParameter("badoms", "list of clipped/saturated OMs", badoms);

    rectrk = "";
    AddParameter("rectrk", "input track name", rectrk);

    result = "MuEx";
    AddParameter("result", "name of the result particle", result);

    elistn = result+"_list";

    lcsp = 0;
    AddParameter("lcspan", "lc span in input data", lcsp);

    repeat = 0;
    AddParameter("repeat", "number of iterations for uncertainty estimatation", repeat);

    rectyp = true;
    AddParameter("rectyp", "track(True) or cascade(False) reconstruction", rectyp);

    usempe = false;
    AddParameter("usempe", "mpe(True) or spe(False) reconstruction", usempe);

    detail = false;
    AddParameter("detail", "detailed energy losses", detail);

    energy = false;
    AddParameter("energy", "estimate energy", energy);

    compat = false;
    AddParameter("compat", "compatibility with older pre-wreco trunk", compat);

    icedir = getenv("I3_BUILD")+string("/mue/resources/ice/mie");
    AddParameter("icedir", "icemodel directory", icedir);

    AddOutBox("OutBox");
  }

  // transitions
  void Configure(){
    log_info("Configuring muex");

    GetParameter("pulses", pulses);
    GetParameter("badoms", badoms);
    GetParameter("rectrk", rectrk);
    GetParameter("result", result);
    elistn = result+"_list";
    GetParameter("lcspan", lcsp);
    GetParameter("repeat", repeat);
    GetParameter("rectyp", rectyp);
    GetParameter("usempe", usempe);
    GetParameter("detail", detail);
    GetParameter("energy", energy);
    GetParameter("compat", compat);
    GetParameter("icedir", icedir);

    log_info("pulses=\"%s\"", pulses.c_str());
    log_info("badoms=\"%s\"", badoms.c_str());
    log_info("rectrk=\"%s\"", rectrk.c_str());
    log_info("result=\"%s\"", result.c_str());
    log_info("lcspan=%u", lcsp);
    log_info("repeat=%u", repeat);
    log_info("rectyp=%s", rectyp ? "True" : "False");
    log_info("usempe=%s", usempe ? "True" : "False");
    log_info("detail=%s", detail ? "True" : "False");
    log_info("energy=%s", energy ? "True" : "False");
    log_info("compat=%s", compat ? "True" : "False");
    log_info("icedir=\"%s\"", icedir.c_str());

    if(detail && !energy) log_warn("Do not forget to set energy=True with detail=True!");

    if(result.length()==0){
      stringstream aux;
      aux << rectrk << "_e";
      result = aux.str();
    }

    icedir+="/";
    ice.ini(icedir);
    tilt.ini(icedir);
  }

  void Physics(I3FramePtr frame){
    log_trace("Entering muex::Physics()");

    MUEX::compat=compat;

    const I3OMGeoMap& geomap = frame->Get<I3Geometry>().omgeo;
    if(!ini){
      doms.clear();
      typedef map<OMKey,I3DOMStatus> I3DOMStatusMap;
      typedef map<OMKey,I3DOMCalibration> I3DOMCalibrationMap;


      const I3DOMStatusMap& stat = frame->Get<I3DetectorStatus>().domStatus;
      const I3DOMCalibrationMap& cal = frame->Get<I3Calibration>().domCal;

      for(I3OMGeoMap::const_iterator i = geomap.begin(); i != geomap.end(); ++i){
	const I3OMGeo& geo = i->second;	
	if(is_inice(geo.omtype)){	  
	  const OMKey& om = i->first;
	  const I3Position& pos = geo.position;

	  double hv=0, eff=1;

	  const I3DOMStatusMap::const_iterator omstat = stat.find(om);
	  if(omstat!=stat.end()) hv = omstat->second.pmtHV/I3Units::volt;

	  const I3DOMCalibrationMap::const_iterator omcal = cal.find(om);
	  if(omcal!=cal.end()) eff=omcal->second.GetRelativeDomEff();
	  if(!isfinite(eff)) eff=1; // for DB problems

	  mydom dom;
	  dom.x=pos.GetX()/I3Units::m;
	  dom.y=pos.GetY()/I3Units::m;
	  dom.z=pos.GetZ()/I3Units::m;
	  dom.hv=hv; dom.eff=eff; dom.st=7500;
	  doms[mykey(om.GetString(), om.GetOM())]=dom;
	}
      }

      tilt.set_r0();
      ini=true;
    }

    const OMKeySeriesConstPtr vbad = frame->Get<OMKeySeriesConstPtr>(badoms);
    if(vbad){
      for(OMKeySeries::const_iterator i = vbad->begin(); i != vbad->end(); ++i)
	boms.insert(mykey(i->GetString(), i->GetOM()));
    }

    const I3RecoPulseSeriesMapConstPtr hits = frame->Get<I3RecoPulseSeriesMapConstPtr>(pulses);

    if(hits){
      
      //clean out non-inice hits
      I3RecoPulseSeriesMapPtr inicehits(new I3RecoPulseSeriesMap());
      for(I3RecoPulseSeriesMap::const_iterator i = hits->begin(); i != hits->end(); ++i){
	//for(auto pulseseries: *hits){
	if (is_inice(geomap.find(i->first)->second.omtype)){
	  (*inicehits)[i->first]=i->second;	  
	}
      }
      
      if(rectrk.empty()){
	vtemp v(inicehits, rectyp, lcsp, repeat, usempe, energy);
	if(v.reconstruct(inicehits, frame, result)){
	  frame->Put(result, I3ParticlePtr(new I3Particle(v.muon)));
	  if(detail) frame->Put(result+"_r", I3DoublePtr(new I3Double(v.dlle(!v.type))));
	  if(!v.elist.empty()) frame->Put(elistn, I3VectorI3ParticlePtr(new I3VectorI3Particle(v.elist)));
	}
      }
      else{
	I3ParticleConstPtr track = frame->Get<I3ParticleConstPtr>(rectrk);
	if(!track){
	  const I3MCTreeConstPtr stree = frame->Get<I3MCTreeConstPtr>(rectrk);
	  if(stree){
	    I3MCTree::const_iterator st = stree->begin();
	    if(st!=stree->end()) track = I3ParticlePtr(new I3Particle(*st));
	  }
	}
	if(track){
	  vtemp v(inicehits, track->GetShape()!=I3Particle::Cascade, lcsp, repeat, usempe, energy);
	  if(v.set_track(track)){
	    v.rlle(rectyp);
	    frame->Put(result, I3ParticlePtr(new I3Particle(v.muon)));
	    if(detail) frame->Put(result+"_r", I3DoublePtr(new I3Double(v.dlle(!v.type))));
	    if(!v.elist.empty()) frame->Put(elistn, I3VectorI3ParticlePtr(new I3VectorI3Particle(v.elist)));
	  }
	}
      }
    }

    boms.clear();

    PushFrame(frame,"OutBox");
    log_trace("Leaving muex::Physics()");
  }

  void Geometry(I3FramePtr frame){
    ini=false;
    PushFrame(frame,"OutBox");
  }

  void Calibration(I3FramePtr frame){
    ini=false;
    PushFrame(frame,"OutBox");
  }

  void DetectorStatus(I3FramePtr frame){
    ini=false;
    PushFrame(frame,"OutBox");
  }
};

I3_MODULE(muex);
