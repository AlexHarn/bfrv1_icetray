#ifndef READER_H_INCLUDED
#define READER_H_INCLUDED
#ifndef __CINT__

#include <map>
#include <deque>
#include <vector>
#include <iostream>
#include <cstring>

namespace I3FRreco{
  // holds DOM parameters necessary for reconstruction
  struct mydom{
    int num;                // consecutive DOM number
    unsigned long long id;  // DOM mainboard id
    double hv;              // PMT high voltage [V]
    double st;              // saturation in PEs
    double vem;             // VEM for IceTop DOMs
    double x, y, z;         // DOM coordinates
    int om, str;            // DOM position/string

    mydom(){
      st=1.e6;
      vem=160;
    }
  };

  // returns the number of DOMs
  int indoms();
  const std::map<unsigned long long, mydom>& idoms();

  // holds DOM info necessary for calculating nearest neighbors
  struct mykey{
    int om, str;
    bool operator< (const mykey& rhs) const
    {
      return str < rhs.str || (str == rhs.str && om < rhs.om);
    }
  };
  const std::map<mykey, unsigned long long>& mykeys();
  // returns array of strings
  const std::map<int, int>& mystrings();

  // total number of events being considered
  extern unsigned long maxevents;

  // Ties all pulses to a single waveform
  struct wform{
    unsigned long long id;  // DOM mainboard id

    wform(){
      events=0;
      maxevents++;
    }

    void copy(const wform& e){
      memcpy(this, &e, sizeof(wform));
    }

    void destroy(){
      maxevents--;
    }

    wform(const wform& e){
      copy(e);
    }

    const wform& operator=(const wform& e){
      if(this != &e){
	destroy();
	copy(e);
      }
      return *this;
    }

    ~wform(){
      destroy();
    }

    int events;             // number of attached events
  };

  // holds hits/pulses (here called events)
  struct event{
    unsigned long long id;  // DOM mainboard id
    unsigned long long gt;  // global time (DOR) of the event
    double Q;               // charge in the waveform (nC)
    double W;               // width of the waveform (ns)
    char fi;                // pulse type: 0:normal pulse
    bool operator< (const event& rhs) const
    {
      return gt < rhs.gt;
    }

    int ijs;                // peak number in the atwd waveform
    wform *wf;              // waveform event data

    event(wform *wf){
      wf->events++;
      this->wf=wf;
      this->id=wf->id;
    }

    void copy(const event& e){
      memcpy(this, &e, sizeof(event));
      if(wf) wf->events++;
    }

    void destroy(){
      if(wf){
	wf->events--;
	if(wf->events==0) delete wf;
      }
    }

    event(const event& e){
      copy(e);
    }

    const event& operator=(const event& e){
      if(this != &e){
	destroy();
	copy(e);
      }
      return *this;
    }

    ~event(){
      destroy();
    }

    event(){
      wf=NULL;
      // std::cerr << "Error: default constructor for event was called!" << std::endl;
    }
  };

  // returns all pulses
  std::deque<event *>& ievents();

  // initializes the reconstruction parameters
  void amain(unsigned int, int);

  // prints a pulse
  std::ostream& operator<<(std::ostream&, const event&);

  // holds reconstruction results
  struct preco{
    int type;        // reconstruction type 1 = muon 2 = cascade 3 = icetop shower
    int hits;        // number of hits used in the fit
    int wforms;      // number of waveforms
    int hitdoms;     // number of hit DOMs
    int strings;     // number of hit strings
    double rllh;     // reduced log-likelihood
    double t0, z0, th, dd, ph, az, x0, y0, n0, ne, nl;
    double t0_min, z0_min, th_min, dd_min, n0_min, ph_min, az_min, x0_min, y0_min;
    double t0_max, z0_max, th_max, dd_max, n0_max, ph_max, az_max, x0_max, y0_max;
    short dir;       // up:1 down:-1
    short ddf;       // closest approach is negative (opposite side) if -1
    short en;        // 1 if joint reconstruction
    short sp;        // 1 if special reconstruction
  };

  // prints a reconstruction result
  std::ostream& operator<<(std::ostream&, const preco&);

  // initializes lightweight geometry, necessary for uniform access to the in-ice and icetop DOMs
  void setgeo(unsigned long long, int, int, double, double, double, double);

  // number of icetop and inice hits used in the reconstruction
  int get_itop();
  int get_inic();

  // number of icetop stations and inice strings used in the reconstruction
  int get_itsn();
  int get_icsn();

  // reconstruction mask
  unsigned int get_rmsk();

  // string number to use for reconstruction
  int get_strn();

  // whether to interpolate
  int getinterpo();

  // holds simple majority events
  struct multe{
    int evnum;               // event number since the last call to reconfigure
    int max;                 // maximum number of hits from different DOMs in a triggered hit set
    int mlt;                 // total number of hits from different DOMs in the calculated trigger
    int tot;                 // total number of hits in the event
    unsigned long long t0;   // earliest hit time in the event
    unsigned long long ts;   // average hit time in the event
    int inic, itop;          // number of inice and icetop hits in a trigger
    int icsn, itsn;          // number of inice strings and icetop stations participating in a trigger
    std::vector<preco> tracks;  // reconstruction results 
    std::vector<event> events;  // events (pulses)
  };
  std::ostream& operator<<(std::ostream&, const multe&);
  std::deque<multe>& itevents();

}

#endif

#endif  // READER_H_INCLUDED
