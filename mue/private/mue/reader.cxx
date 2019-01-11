#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
// #include <expat.h>

#include <cmath>
#include <fstream>
#include <algorithm>

#include "reader.h"

#include "llhreco.h"
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_multimin.h>

using namespace std;

namespace I3FRreco{
  // number of initialized DOMs
  static int ndoms=0;
  int indoms(){
    return ndoms;
  }

  // array of DOMs
  static map<unsigned long long, mydom> doms;
  const map<unsigned long long, mydom>& idoms(){
    return doms;
  }

  // sequential DOM keys
  static map<mykey, unsigned long long> ikeys;
  const map<mykey, unsigned long long>& mykeys(){
    return ikeys;
  }

  // array of strings
  static map<int, int> istrings;
  const map<int, int>& mystrings(){
    return istrings;
  }

  // new DOM initializer
  static void new_dom(unsigned long long cid){
    mydom adom;
    adom.num=ndoms;
    ndoms++;
    adom.id=cid;
    adom.hv=0;
    adom.x=0;
    adom.y=0;
    adom.z=0;
    adom.om=0;
    adom.str=0;
    doms.insert(make_pair(cid, adom));
  }

  static unsigned long long ev_t0=0;  // time of the first hit in the triggered set

  ostream& operator<<(ostream& out, const event& it)  // prints one pulse
  {
    mydom& cdom=doms.find(it.id)->second;
    out << "dom=(" << cdom.str << "," << cdom.om << ")" << "  at=(" << cdom.x << "," << cdom.y << "," << cdom.z << ")"
	<< "  le=" << ((long long)(it.gt-ev_t0))/10. << "  w=" << it.W << "  q=" << it.Q << endl;
    return out;
  }

  static const long long ONESEC=10000000000LL;  // in [0.1 ns]
  static long long TSTEP=ONESEC;                // all data must fit in this interval

  // array of pulses
  static deque<event *> events;
  deque<event *>& ievents(){
    return events;
  }

  // iterators necessary for building of event sets
  static deque<event *>::iterator eit=events.begin();
  static deque<event *>::iterator eiti=eit;
  static deque<event *>::iterator eitf=eit;

  deque<event *>::iterator get_eitf(){
    return eitf;
  }

  static bool nnd=false;  // true if within a call to *trigger functions
  unsigned long maxevents=0;  // total number of events being considered

  // delete the pulses deque
  static void everase(deque<event *>::iterator eiti, deque<event *>::iterator eitf){
    for (deque<event *>::iterator it=eiti; it != eitf ; ++it ) delete (*it);
    events.erase(eiti, eitf);
  }

  // pick one
  inline bool evsort(const event* eva, const event* evb){
    return eva->gt < evb->gt;
  }

  struct evless{
    bool operator()(const event* lhs, const event* rhs) const
    {
      return lhs->gt < rhs->gt;
    }
  };

  static unsigned long evoutcount, fullevents;  // pulses counters

  static vector<unsigned long long> dreq;          // IDs of DOMs that must be present in reconstructed events
  static unsigned long xnum=(unsigned long) 1.e6;  // maximum number of unsorted entried to be held in memory
  static unsigned long long gt0=0;                 // global time 0; later than any hit in a triggered set

  // collects hits for triggering; prints data if not called from a *trigger function
  static void output(){
    sort (events.begin(), events.end(), evless());
    unsigned long count=0;
    deque<event *>::iterator it;
    it = events.begin();
    eit=it;
    unsigned long numevents=events.size();
    for (eiti=it ; it != events.end() ; ++it ){
      numevents--;
      if((*it)->gt<gt0 || numevents>=xnum/2){
	if(!nnd) cout << *(*it);
	count++;
      }
      else break;
    }
    eitf=it;
    if(!nnd) everase(eiti, eitf);
    evoutcount=count;
  }

  // modify this to add external hits (e.g., from other detector streams)
  int mdata(){
    return -1;
  }

  static unsigned long long Ggt=0;  // must be recalculated if mdata is used

  // assembles data in blocks of size TSTEP
  int data(){
    int i;
    evoutcount=0;
    while(evoutcount==0){  // iterators set in the output() can otherwise get invalidated
      fullevents=0;
      while((i=mdata())==0) if(fullevents>0) break;

      if(i!=0){
	gt0=Ggt+TSTEP;
	output();
      }
      else if(Ggt>gt0+2*TSTEP || maxevents>=xnum){
	while(Ggt>gt0+2*TSTEP) gt0+=TSTEP;
	output();
      }
      if(i!=0) break;
    }

    return i;
  }

  // needed for iterator arithmetic
  static bool efl=false;

  // gets next pulse from available data
  deque<event *>::iterator next(){
    nnd=true;
    while(true){
      if(eit==eitf){
	if(eiti!=eitf){
	  everase(eiti, eitf);
	  eiti=events.begin();
	  eitf=eiti;
	}
	if(efl) return eitf;
	if(data()!=0) efl=true;
      }
      if(eit!=eitf){ return eit++; }
    }
    nnd=false;
  }

  // returns triggered sets
  static deque<multe> tevents;
  deque<multe>& itevents(){
    return tevents;
  }

  // minimum hit DOM number and window parameters for simple majority trigger
  static unsigned int rnum;

  // prints the triggered event
  ostream& operator<<(ostream& out, const multe& it)
  {
    ev_t0=it.t0;
    out << "! event " << it.evnum << " " << it.max << " " << it.mlt << " " << it.tot << " " << it.t0 << " " << it.ts;
    out << " ic=" << it.inic << " it=" << it.itop << " sc=" << it.icsn << " st=" << it.itsn; 
    out << endl;
    for(vector<preco>::const_iterator t = it.tracks.begin(); t!=it.tracks.end(); ++t) out << "\t" << *t;
    out << "hit data:" << endl;
    for(vector<event>::const_iterator t = it.events.begin(); t!=it.events.end(); ++t) out << "\t" << *t;
    return out;
  }

  // whether to interpolate
  static int interpo=0;
  int getinterpo(){
    return interpo;
  }

  // sets the reconstruction parameters
  void amain(unsigned int rnuma, int intra){
    rnum=rnuma;
    interpo=intra;
  }

  // creates the lightweight structure to hold DOM geometry
  void setgeo(unsigned long long domid, int om, int str, double x, double y, double z, double hv){
    map<unsigned long long, mydom>::iterator it_doms=doms.find(domid);
    if(it_doms==doms.end()){ new_dom(domid); it_doms=doms.find(domid); }
    mydom& cdom=it_doms->second;
    cdom.x=x;
    cdom.y=y;
    cdom.z=z;
    cdom.om=om;
    cdom.str=str;
    mykey key;
    key.om=om;
    key.str=str;
    ikeys[key]=domid;
    istrings[str]++;
    if(hv>0) cdom.hv=hv;
  }

  int get_itop(){return 0;}
  int get_inic(){return 6;}
  int get_itsn(){return 0;}
  int get_icsn(){return 1;}
  unsigned int get_rmsk(){return 1;}
  int get_strn(){return 0;}

}
