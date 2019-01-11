/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3ResponseMapSplitter.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

// tools, standard lib stuff
#include "icetray/IcetrayFwd.h"
#include <vector>
#include <string>
#include <utility>
#include <list>
#include <functional>
#include <cmath>

// my header
#include "double-muon/I3ResponseMapSplitter.h"

// IceTray/ROOT stuff
I3_MODULE(I3ResponseMapSplitter);
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "icetray/I3Context.h"
#include "icetray/I3Frame.h"

static const std::string inpulse_optionname = "InputPulseMap";
static const std::string intrack_optionname = "InputTrackName";
static const std::string minnch_optionname  = "MinimumNch";
static const std::string dotres_optionname = "DoTRes";
static const std::string mintres_optionname = "MinTRes";
static const std::string maxtres_optionname = "MaxTRes";
static const std::string dobrightst_optionname = "DoBrightSt";
static const std::string maxdbrightst_optionname = "maxDBrightSt";
static const std::string dokmeans_optionname = "DoKmeans";
static const std::string tmedian_optionname = "DoTMedian";
static const std::string tweight_optionname = "TSplitWeight";
static const std::string wantrotated_optionname = "WantRotated";
static const std::string splitevents_optionname = "SplitEvents";
static const std::string geo_optionname = "I3GeometryName";

// dataclasses & gulliver
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "phys-services/I3Cuts.h"
#include "phys-services/I3Calculator.h"
#include "phys-services/I3RandomService.h"
#include "dataclasses/physics/I3EventHeader.h"


//----------------------------------------------------------------------------

I3ResponseMapSplitter::I3ResponseMapSplitter(const I3Context& ctx) :
  I3ConditionalModule(ctx),
  I3Splitter(configuration_),
  geoName_(I3DefaultName<I3Geometry>::value()),
  minNch_(2),
  doTRes_(0),minTRes_(0),maxTRes_(1000),
  doBrightSt_(0),maxDBrightSt_(150),
  doKmeans_(0), wantRotated_(0),splitEvents_(false),
  doTMedian_(false), tSplitWeightName_("OLD"), tSplitWeight_(SW_OLD),
  nEvents_(0), nBadEvents_(0),
  nHits1_(0), nHits2_(0),
  nMinHits1_(INT_MAX), nMinHits2_(INT_MAX),
  nMaxHits1_(0), nMaxHits2_(0){

    AddOutBox("OutBox");

    // options

    AddParameter( inpulse_optionname,
                  "Name of pulse map (or pulse mask) to split",
                  inPulseMapName_ );

    AddParameter( intrack_optionname,
                  "Name of a trackfit; hit/pulse maps are split "
                  "geometrically, either using the plane perpendicular to the "
                  "track and through the COG as a divider OR the time residual "
                  "of the hit relative to the track (if doTRes is set)."
                  "If no track name is specified, or the fit had failed, "
                  "then the hit/pulses are split by comparing their time to"
                  "the average hit/pulse time.",
                  inTrackName_ );

    AddParameter(dotres_optionname,
                 "Sort hits according to time residual (default FALSE).",
                 doTRes_);

    AddParameter(mintres_optionname,
                 "Minimum time residual relative to fit track (default 0.).",
                 minTRes_);

    AddParameter(maxtres_optionname,
                 "Maximum time residual relative to fit track (default 1000.).",
                 maxTRes_);

    AddParameter( minnch_optionname,
                  "minimum number of DOMs with hits",
                  minNch_ );

    AddParameter(dobrightst_optionname,
                 "Sort hits according to proximity to brightest string (default FALSE).",
                 doBrightSt_);

    AddParameter(maxdbrightst_optionname,
                 "Maximum radial distance from brightest track (default 150.).",
                 maxDBrightSt_);

    AddParameter(dokmeans_optionname,
                 "Sort hits according to kmeans algorithm (default FALSE).",
                 doKmeans_);

    AddParameter(wantrotated_optionname,
                 "Sort hits according to kmeans algorithm, rotated around track (default FALSE).",
                 wantRotated_);

    AddParameter(geo_optionname,
                 "Name of (alternative) I3Geometry object",
                 geoName_ );
    
    AddParameter(splitevents_optionname,
                 "Run as an event splitter. If True, the module will run on DAQ frames and emit "
                 "two Physics frames containing the halves of the event.",
                 splitEvents_);

    AddParameter(tmedian_optionname,
                 "Time-split pulses on the *median* instead of the *mean* time (default FALSE).",
                 doTMedian_ );

    AddParameter(tweight_optionname,
                 "Time-split pulse weighting for split-time (median or mean):\n"
                 "'OLD': use all pulses with equal weight\n"
                 "'Charge': use all pulses, weighted with charge\n"
                 "'DOM': use only first pulse in each DOM, ignore charge.",
                 tSplitWeightName_ );

    AddParameter("SubEventStreamName",
		 "The name of the SubEvent stream.",
		 configuration_.InstanceName());

}

//----------------------------------------------------------------------------
void I3ResponseMapSplitter::Configure(){

    GetParameter( inpulse_optionname, inPulseMapName_ );
    GetParameter( intrack_optionname, inTrackName_ );
    GetParameter( minnch_optionname, minNch_ );
    GetParameter( dotres_optionname,  doTRes_ );
    GetParameter( mintres_optionname,  minTRes_ );
    GetParameter( maxtres_optionname,  maxTRes_ );
    GetParameter( dobrightst_optionname, doBrightSt_ );
    GetParameter( maxdbrightst_optionname, maxDBrightSt_ );
    GetParameter( dokmeans_optionname, doKmeans_ );
    GetParameter( wantrotated_optionname, wantRotated_ );
    GetParameter( geo_optionname, geoName_ );
    GetParameter( splitevents_optionname, splitEvents_ );
    GetParameter( tmedian_optionname, doTMedian_ );
    GetParameter( tweight_optionname, tSplitWeightName_ );
    GetParameter("SubEventStreamName", sub_event_stream_name_);
    
    if(inTrackName_.empty()&&wantRotated_){
      log_info("(%s) You asked to rotate points along track, but the track %s doesn't exist, only doing k-means separation",GetName().c_str(),inTrackName_.c_str());
      wantRotated_=0;
    }

    if ( ! inTrackName_.empty() ){
      if( doTRes_ ){
        log_info( "(%s) Splitting hit/pulse set w.r.t. time residual of %s",
                  GetName().c_str(), inTrackName_.c_str() );
      }
      else if(doKmeans_&&wantRotated_){
        log_info("(%s) Splitting hit/pulse set w.r.t k-means clustering of hits rotated around angles from %s",GetName().c_str(),inTrackName_.c_str());
      }
      else{
        log_info( "(%s) Splitting hit/pulse set w.r.t. COG and %s",
                  GetName().c_str(), inTrackName_.c_str() );
      }
    }

    if(doKmeans_&&!wantRotated_){
      log_info("(%s) Splitting hit/pulse set w.r.t k-means clustering",GetName().c_str());
    }

    if(doBrightSt_){
      log_info("(%s) Splitting hit/pulse set w.r.t proximity to brightest string",
               GetName().c_str());
    }

    if ( inPulseMapName_.empty() ){
        log_fatal( "(%s) You should specify a pulsemap "
                   "with the \"%s\" option",
                   GetName().c_str(),
                   inpulse_optionname.c_str() );
    }
    inResponseMapName_ = inPulseMapName_;
    if (splitEvents_) {
        outResponseMap1Name_ = GetName();
    } else {
        outResponseMap1Name_ = GetName()+"1";
        outResponseMap2Name_ = GetName()+"2";
    }

    std::string weightdoc;
    if (tSplitWeightName_ == "OLD"){
        tSplitWeight_ = SW_OLD;
        weightdoc = "all pulses (unweighted)";
    } else if (tSplitWeightName_ == "Charge"){
        tSplitWeight_ = SW_Charge;
        weightdoc = "all charge-weighted pulses";
    } else if (tSplitWeightName_ == "DOM"){
        tSplitWeight_ = SW_DOM;
        weightdoc = "the first pulse in every DOM (unweighted)";
    } else {
        log_fatal( "(%s) unknown value for option %s: %s (should be 'OLD', 'Charge' or 'DOM')",
                   GetName().c_str(),
                   tweight_optionname.c_str(),
                   tSplitWeightName_.c_str() );
    }

    log_info("(%s) when splitting pulses based on time, using a split time based on the %s of the %s pulses.",
             GetName().c_str(),
             (doTMedian_ ? "median" : "mean"), weightdoc.c_str());
}

template<typename Response>
double I3ResponseMapSplitter::SplitByTime(
        typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        bool want_median,
        I3ResponseMapSplitter::splitweight_t sw,
        const std::string &name){

    typedef std::vector< Response > ResponseSeries;
    typedef I3Map< OMKey, ResponseSeries > ResponseMap;

    nhits1=nhits2=0;
    newmap1->clear();
    newmap2->clear();

    typename ResponseMap::const_iterator iom;
    typename ResponseSeries::const_iterator ipulse;

    // this can probably done more efficiently, but speed is not so important here
    // going to collect data for both the median and the mean
    double splitTime = NAN;
    double qsum = 0.;
    double tsum = 0.;
    double tqsum = 0.;
    // first=time, second=charge; using a multimap does *not* make it faster
    std::vector< std::pair<double,double> > tqvec;
    tqvec.reserve(2*responsemap->size()); // random guess for number of pulses

    for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
        for ( ipulse = iom->second.begin(); ipulse != iom->second.end(); ++ipulse ){
            double t = ipulse->GetTime();
            double q = ipulse->GetCharge();
            tqvec.push_back( std::make_pair(t,q) );
            qsum += q;
            tqsum += q*t;
            tsum += t;
            if (sw == SW_DOM) break; // only first pulse in each DOM
        }
    }
    size_t nresponse = tqvec.size();
    if (nresponse == 0) {
        // DJB: this used to be a log_fatal, that seems excessive
        log_error("(%s) Trying to split empty pulse/hit series!", name.c_str());
        return NAN;
    } else if (nresponse == 1){
        log_error("(%s) Trying to split pulse/hit series with only 1 pulse!", name.c_str());
        splitTime = tqvec[0].first+1*I3Units::ns; // arbitrary: force the only pulse into the first map
    } else {
        if (want_median){
            std::sort(tqvec.begin(),tqvec.end());
            // We do not want the split time to be equal to a hit time
            // so we take the mean of the "median time" and the "prevous time"
            // since SuperDST digitizes hit times, it is very possible that
            // hits in different DOMs have exactly the same time, so some care
            // must be taken to ensure that the "previous time" is indeed earlier
            // than the median time.
            // We could achieve something similar by running std::unique on the
            // tq vector, but that *discards* the copies which might cause us to
            // land at the wrong median.
            double t = tqvec[0].first;
            double prevt = t - 1.*I3Units::ns;
            if (sw == SW_Charge){
                double median = 0.5 * qsum;
                std::vector< std::pair<double,double> >::iterator itq = tqvec.begin();
                qsum = 0.;
                for (itq = tqvec.begin(); itq != tqvec.end(); itq++){
                    qsum += itq->second;
                    t = itq->first;
                    if (qsum>median && t>prevt){
                        break;
                    }
                    prevt = t;
                }
            } else {
                // DOM-weighted or OLD-weighted
                int i = nresponse/2;
                t = tqvec[i].first;
                while (i-->0){
                    if ( tqvec[i].first < t ){
                        prevt = tqvec[i].first;
                        break;
                    }
                }
            }
            // make sure splitTime is in between hit times
            splitTime = 0.5*(t + prevt);
        } else {
            // user does not want median: will compute mean instead
            if (sw == SW_Charge){
                splitTime = tqsum/qsum;
            } else {
                // DOM-weighted or OLD-weighted
                splitTime = tsum/nresponse;
            }
        }
    }
    for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
        const OMKey &om = iom->first;
        for ( ipulse = iom->second.begin(); ipulse != iom->second.end(); ++ipulse ){
            double t = ipulse->GetTime();
            if ( t < splitTime ){
                (*newmap1)[om].push_back( *ipulse );
                ++nhits1;
            } else {
                (*newmap2)[om].push_back( *ipulse );
                ++nhits2;
            }
        }
    }
    return splitTime;
}

// force compilation (for unit test)
template
double I3ResponseMapSplitter::SplitByTime<I3RecoPulse>(
        boost::shared_ptr< const I3Map< OMKey, std::vector< I3RecoPulse > > > responsemap,
        boost::shared_ptr< I3Map< OMKey, std::vector< I3RecoPulse > > > newmap1,
        boost::shared_ptr< I3Map< OMKey, std::vector< I3RecoPulse > > > newmap2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        bool want_median,
        I3ResponseMapSplitter::splitweight_t sw,
        const std::string &name);

//--------- splitting hit/pulse set using a track & COG ------------------------
template<typename Response>
void I3ResponseMapSplitter::SplitByTrackAndCOG(
        I3ParticleConstPtr track,
        I3GeometryConstPtr geoptr,
        typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        const std::string &name ){

    typedef std::vector< Response > ResponseSeries;
    typedef I3Map< OMKey, ResponseSeries > ResponseMap;

    nhits1=nhits2=0;
    newmap1->clear();
    newmap2->clear();

    I3Position cog = I3Cuts::COG( *geoptr, *responsemap );
    log_debug( "(%s) COG = (%.1f,%.1f,%.1f)",
               name.c_str(), cog.GetX(), cog.GetY(), cog.GetZ() );

    double threshold = I3Calculator::DistanceAlongTrack( *track, cog );

    typename ResponseMap::const_iterator iom;
    for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
        const OMKey &om = iom->first;
        I3OMGeoMap::const_iterator igeo = geoptr->omgeo.find(om);
        if ( igeo == geoptr->omgeo.end() ){
            log_warn( "(%s) unknown DOM(%d,%u) in input hitset",
                      name.c_str(),
                      igeo->first.GetString(), igeo->first.GetOM() );
            continue;
        }
        const ResponseSeries &series = iom->second;
        const I3Position &ompos = igeo->second.position;
        if ( threshold > I3Calculator::DistanceAlongTrack( *track, ompos ) ){
            newmap1->insert(make_pair(om,series));
            nhits1 += series.size();
        } else {
            newmap2->insert(make_pair(om,series));
            nhits2 += series.size();
        }
    }
}

// force compilation (for unit test)
template
void I3ResponseMapSplitter::SplitByTrackAndCOG<I3RecoPulse>(
        I3ParticleConstPtr track,
        I3GeometryConstPtr geoptr,
        boost::shared_ptr< const I3Map< OMKey, std::vector< I3RecoPulse > > > responsemap,
        boost::shared_ptr< I3Map< OMKey, std::vector< I3RecoPulse > > > newmap1,
        boost::shared_ptr< I3Map< OMKey, std::vector< I3RecoPulse > > > newmap2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        const std::string &name );

//--------- splitting hit/pulse set using a track & time residuals -------------
template<typename Response>
void I3ResponseMapSplitter::SplitByTimeResidual(
        double minTRes, double maxTRes,
        I3ParticleConstPtr track,
        I3GeometryConstPtr geoptr,
        typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        const std::string &name ){

    typedef std::vector< Response > ResponseSeries;
    typedef I3Map< OMKey, ResponseSeries > ResponseMap;

    nhits1=nhits2=0;
    newmap1->clear();
    newmap2->clear();

    typename ResponseMap::const_iterator iom;
    for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
        const OMKey &om = iom->first;
        I3OMGeoMap::const_iterator igeo = geoptr->omgeo.find(om);
        if ( igeo == geoptr->omgeo.end() ){
            log_warn( "(%s) unknown DOM(%d,%u) in input hitset",
                      name.c_str(),
                      igeo->first.GetString(), igeo->first.GetOM() );
            continue;
        }
        const I3Position &ompos = igeo->second.position;
        typename ResponseSeries::const_iterator ipulse;
        for ( ipulse = iom->second.begin(); ipulse != iom->second.end(); ++ipulse ){
            double t = ipulse->GetTime();
            double dt=I3Calculator::TimeResidual(*track,ompos,t);
            //  log_info("OM pos (%g,%g,%g) time is %g, tres is %g",
            //           igeo->second.position.GetX(),
            //           igeo->second.position.GetY(),
            //           igeo->second.position.GetZ(),t,dt);
            if ( dt >= minTRes && dt<=maxTRes ){
                (*newmap1)[om].push_back( *ipulse );
                //  log_info("putting hit %g into box 1",t);
                ++nhits1;
            } else {
                (*newmap2)[om].push_back( *ipulse );
                ++nhits2;
            }
        }
    }
}

// force compilation (for unit test)
template
void I3ResponseMapSplitter::SplitByTimeResidual<I3RecoPulse>(
        double minTRes, double maxTRes,
        I3ParticleConstPtr track,
        I3GeometryConstPtr geoptr,
        boost::shared_ptr< const I3Map< OMKey, std::vector< I3RecoPulse > > > responsemap,
        boost::shared_ptr< I3Map< OMKey, std::vector< I3RecoPulse > > > newmap1,
        boost::shared_ptr< I3Map< OMKey, std::vector< I3RecoPulse > > > newmap2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        const std::string &name );

//--------------- splitting hit/pulse set using brightness ---------------------
template<typename Response>
void I3ResponseMapSplitter::SplitByBrightness(
        double maxDBrightSt,
        I3GeometryConstPtr geoptr,
        typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        const std::string &name ){

    typedef std::vector< Response > ResponseSeries;
    typedef I3Map< OMKey, ResponseSeries > ResponseMap;

    nhits1=nhits2=0;
    newmap1->clear();
    newmap2->clear();

    I3OMGeoMap sg = geoptr->omgeo;
    I3OMGeoMap::const_iterator iter;
    std::map<int,int> nstringmap;
    std::map<int,int>::iterator nsit;
    std::map<int,double> stringx;
    std::map<int,double> stringy;

    for ( iter=sg.begin(); iter!=sg.end(); iter++) {
        int st = iter->first.GetString();
        nstringmap[st]=0;
    }

    typename ResponseMap::const_iterator iom;
    for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
        const OMKey &om = iom->first;
        I3OMGeoMap::const_iterator igeo = geoptr->omgeo.find(om);
        if ( igeo == geoptr->omgeo.end() ){
            log_warn( "(%s) unknown DOM(%d,%u) in input hitset",
                      name.c_str(),
                      igeo->first.GetString(), igeo->first.GetOM() );
            continue;
        }
        nstringmap[igeo->first.GetString()]++;
        stringx[igeo->first.GetString()]=igeo->second.position.GetX();
        stringy[igeo->first.GetString()]=igeo->second.position.GetY();
    }
    double maxstring=0;
    int nstring=0;
    for (nsit = nstringmap.begin(); nsit != nstringmap.end(); ++nsit){
        if ((*nsit).second>maxstring){
            maxstring=(*nsit).second;
            nstring=(*nsit).first;
        }
    }
    for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
        typename ResponseSeries::const_iterator ipulse;
        for ( ipulse = iom->second.begin(); ipulse != iom->second.end(); ++ipulse ){
            const OMKey &om = iom->first;
            I3OMGeoMap::const_iterator igeo = geoptr->omgeo.find(om);
            if ( igeo == geoptr->omgeo.end() ){
                log_warn("(%s) unknown DOM(%d,%u) in input hitset",
                         name.c_str(),igeo->first.GetString(), igeo->first.GetOM() );
                continue;
            }
            double shortx,shorty,rdistance;
            shortx = igeo->second.position.GetX();
            shorty = igeo->second.position.GetY();
            rdistance = sqrt( (shortx-stringx[nstring])*(shortx-stringx[nstring]) +
                              (shorty-stringy[nstring])*(shorty-stringy[nstring]) );
            if (rdistance<=maxDBrightSt){
                (*newmap1)[om].push_back( *ipulse );
                ++nhits1;
                // log_info( "putting (%g,%g,%g) into box 1",
                //           igeo->second.position.GetX(),
                //           igeo->second.position.GetY(),
                //           igeo->second.position.GetZ());
            } else {
                (*newmap2)[om].push_back( *ipulse );
                ++nhits2;
                // log_info( "putting (%g,%g,%g) into box 2",
                //           igeo->second.position.GetX(),
                //           igeo->second.position.GetY(),
                //           igeo->second.position.GetZ());
            }
        }
    }
    #ifdef DEBUG
    for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
      const OMKey &om = iom->first;
      I3OMGeoMap::const_iterator igeo = geoptr->omgeo.find(om);
      log_info("om %d newmap1 size %d newmap2 size %d",igeo->first.GetOM(),(int)(*newmap1)[om].size(),(int)(*newmap2)[om].size());
    }
    #endif
}

// force compilation (for unit test)
template
void I3ResponseMapSplitter::SplitByBrightness<I3RecoPulse>(
        double maxDBrightSt,
        I3GeometryConstPtr geoptr,
        boost::shared_ptr< const I3Map< OMKey, std::vector< I3RecoPulse > > > responsemap,
        boost::shared_ptr< I3Map< OMKey, std::vector< I3RecoPulse > > > newmap1,
        boost::shared_ptr< I3Map< OMKey, std::vector< I3RecoPulse > > > newmap2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        const std::string &name );


//--------- splitting hit/pulse set using K-Means clustering -------------
template<typename Response>
void I3ResponseMapSplitter::SplitByKmeans(
	bool wantRotated_,
        I3ParticleConstPtr track,
        I3GeometryConstPtr geoptr,
        typename boost::shared_ptr< const I3Map< OMKey, std::vector< Response > > > responsemap,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
        typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2,
        unsigned int &npulses1,
        unsigned int &npulses2,
        unsigned int &nhits1,
        unsigned int &nhits2,
        const std::string &name,
	I3RandomServicePtr rnd ){

  typedef std::vector< Response > ResponseSeries;
  typedef I3Map< OMKey, ResponseSeries > ResponseMap;
  I3OMGeoMap sg = geoptr->omgeo;
  I3OMGeoMap::const_iterator iter;
  int nstring;
  std::map<int,int> nstringmap;
  std::map<int,int>::iterator nsit;
  std::map<int,double> stringx;
  std::map<int,double> stringy;
  std::map<int,double> stringz;
  I3Position rotated,primary;
  double centroid1[3],centroid2[3];
  double savecentroid1[3],savecentroid2[3];
  double sqrderr,savesqrderr=1e9;
  double savemincentroid1[3],savemincentroid2[3];
  double shortx,shorty,shortz;
  double rdistance1,rdistance2;
  typename ResponseMap::const_iterator iom; 


  nhits1=nhits2=0;
  newmap1->clear();
  newmap2->clear();
  //  tempmap1->clear();
  //  tempmap2->clear();
  
  for(int i=0;i<3;i++){
    savemincentroid1[i]=savemincentroid2[i]=0.;
  }
  
  nstring=0;
  for(iter=sg.begin(); iter!=sg.end(); iter++) {
    int st = iter->first.GetString();
    nstringmap[st]=0;
  }
  
  for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
    const OMKey &om = iom->first;
    I3OMGeoMap::const_iterator igeo = geoptr->omgeo.find(om);
    if ( igeo == geoptr->omgeo.end() ){
      log_warn("(%s) unknown DOM(%d,%u) in input hitset",
	       name.c_str(),
	       igeo->first.GetString(), igeo->first.GetOM() );
      continue;
    }
    nstringmap[igeo->first.GetString()]++;
    stringx[igeo->first.GetString()]=igeo->second.position.GetX();
    stringy[igeo->first.GetString()]=igeo->second.position.GetY();
    stringz[igeo->first.GetString()]=igeo->second.position.GetZ();
    /*    rotated=(igeo->second.position);
    rotated.RotateZ(-track->GetAzimuth());
    rotated.RotateY(-track->GetZenith());*/
  }
  double maxstring=0;
  for(nsit = nstringmap.begin(); nsit != nstringmap.end(); ++nsit){
    if((*nsit).second>maxstring){
      maxstring=(*nsit).second;
      nstring=(*nsit).first;
    }
  }
  //  log_debug("Max string found at %d (%g,%g,%g)",nstring,stringx[nstring],stringy[nstring],stringz[nstring]);
  centroid1[0]=stringx[nstring];
  centroid1[1]=stringy[nstring];
  centroid1[2]=0.;
  //  I3RandomService& rnd = context_.Get<I3RandomService>();
  //  I3RandomService& rnd = ctx.Get<I3RandomService>();
  for(int iter=0;iter<1;iter++){
    for(int i=0;i<3;i++){
      savecentroid1[i]=1.e9;
      savecentroid2[i]=1.e9;
    }
    sqrderr=0.;
    /* randomly make a new starting point, except try one centroid at the bright string first */
    if(iter>0){ /* try bright string location first */
      centroid1[0]=rnd->Uniform(-600,600);
      centroid1[1]=rnd->Uniform(-600,600);
      centroid1[2]=rnd->Uniform(-600,600);
    }
    centroid2[0]=rnd->Uniform(-600,600);
    centroid2[1]=rnd->Uniform(-600,600);
    centroid2[2]=rnd->Uniform(-600,600);
    //    centroid2[2]=centroid1[2];
    //    log_debug("Random starting point (%g %g %g)",centroid2[0],centroid2[1],centroid2[2]);
    //    log_debug("size of responsemap %d",(int)responsemap->size());
    int nhits=0;
    int ncentroid1=0,ncentroid2=0;
    double mintol=1.;
    int nloops=0;
    while((fabs(savecentroid1[0]-centroid1[0])>mintol)||(fabs(savecentroid1[1]-centroid1[1])>mintol)||(fabs(savecentroid2[0]-centroid2[0])>mintol)||(fabs(savecentroid2[1]-centroid2[1])>mintol)){
      if(nloops>10000){
	//	log_debug("event not converging, giving up");
	iter=1001;
	break;
      }
      nhits=0;
      /*if one of these is empty, try rethrowing the points*/
      if(nloops>0&&(ncentroid1==0||ncentroid2==0)){
	centroid2[0]=rnd->Uniform(-600,600);
	centroid2[1]=rnd->Uniform(-600,600);
	centroid2[2]=rnd->Uniform(-600,600);
	centroid1[0]=rnd->Uniform(-600,600);
	centroid1[1]=rnd->Uniform(-600,600);
	centroid1[2]=rnd->Uniform(-600,600);
	//	log_debug("empty bin (#1 %d, #2 %d), rethrowing 1 (%g %g) rethrowing 2 (%g %g)",ncentroid1,ncentroid2,centroid1[0],centroid1[1],centroid2[0],centroid2[1]);
      }
      //      log_debug("nloops %d ncentroid1 %d ncentroid2 %d centroid1 (%g %g) savecentroid1 (%g %g) centroid2 (%g %g) savecentroid2 (%g %g)",nloops,ncentroid1,ncentroid2,centroid1[0],centroid1[1],savecentroid1[0],savecentroid1[1],centroid2[0],centroid2[1],savecentroid2[0],savecentroid2[1]);
      nloops++;
      ncentroid1=ncentroid2=0;
      for(int i=0;i<3;i++){
	savecentroid1[i]=centroid1[i];
	savecentroid2[i]=centroid2[i];
	centroid1[i]=centroid2[i]=0.;
      }
      for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
	typename ResponseSeries::const_iterator ipulse;
	for ( ipulse = iom->second.begin(); ipulse != iom->second.end(); ++ipulse ){
	  nhits=nhits+1;
	}
      }
      for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
	//            for ( ipulse = iom->second.begin(); ipulse != iom->second.end(); ++ipulse ){
	const OMKey &om = iom->first;
	I3OMGeoMap::const_iterator igeo = geoptr->omgeo.find(om);
	if ( igeo == geoptr->omgeo.end() ){
	  log_warn("(%s) unknown DOM(%d,%u) in input hitset",
		   name.c_str(),igeo->first.GetString(), igeo->first.GetOM() );
	  continue;
	}
	if(wantRotated_){
	  rotated=(igeo->second.position);
	  rotated.RotateZ(-track->GetAzimuth());
	  rotated.RotateY(-track->GetZenith());
	  shortx=rotated.GetX();
	  shorty=rotated.GetY();
	  shortz=rotated.GetZ();
	}
	else{
	  shortx=igeo->second.position.GetX();
	  shorty=igeo->second.position.GetY();
	  shortz=igeo->second.position.GetZ();
	}
	rdistance1=sqrt((shortx-savecentroid1[0])*(shortx-savecentroid1[0])+(shorty-savecentroid1[1])*(shorty-savecentroid1[1]));
	rdistance2=sqrt((shortx-savecentroid2[0])*(shortx-savecentroid2[0])+(shorty-savecentroid2[1])*(shorty-savecentroid2[1]));
	if(rdistance1<=rdistance2){
	  //              log_info("bin 1 distance %g ( %g , %g , %g )",rdistance1,shortx,shorty,shortz);
	  centroid1[0]+=shortx;
	  centroid1[1]+=shorty;
	  centroid1[2]+=shortz;
	  ncentroid1++;
	}
	else{
	  //log_info("bin 2 distance %g ( %g , %g , %g )",rdistance2,shortx,shorty,shortz);
	  centroid2[0]+=shortx;
	  centroid2[1]+=shorty;
	  centroid2[2]+=shortz;
	  ncentroid2++;
	}
	/*keep track of squared error so we can check if we're in minimum spot*/
	if(rdistance1<=rdistance2) sqrderr+=(rdistance1*rdistance1);
	else sqrderr+=(rdistance2*rdistance2);
      }
      for(int i=0;i<3;i++){
	if(ncentroid1!=0)centroid1[i]=centroid1[i]/(float)ncentroid1;
	if(ncentroid2!=0)centroid2[i]=centroid2[i]/(float)ncentroid2;
      }
      //      log_debug("centroid1 (%g,%g,%g) savecentroid1 (%g,%g,%g)",centroid1[0],centroid1[1],centroid1[2],savecentroid1[0],savecentroid1[1],savecentroid1[2]);
      //      log_debug("centroid2 (%g,%g,%g) savecentroid2 (%g,%g,%g)",centroid2[0],centroid2[1],centroid2[2],savecentroid2[0],savecentroid2[1],savecentroid2[2]);
    }
            
    //    log_debug("final centroid1 (%g,%g,%g) centroid2 (%g,%g,%g)",centroid1[0],centroid1[1],centroid1[2],centroid2[0],centroid2[1],centroid2[2]);
    //    log_debug("on iter %d sqrderr %g savesqrderr %g\n",iter,sqrderr,savesqrderr);
    if(sqrderr<savesqrderr&&ncentroid1>0&&ncentroid2>0){
      savesqrderr=sqrderr;
      for(int i=0;i<3;i++){
	savemincentroid1[i]=centroid1[i];
	savemincentroid2[i]=centroid2[i];
      }
    }
  }
  //  log_debug("after iters sqrderr %g centroid1 (%g,%g,%g) centroid2 (%g,%g,%g)",savesqrderr,savemincentroid1[0],savemincentroid1[1],savemincentroid1[2],savemincentroid2[0],savemincentroid2[1],savemincentroid2[2]);
  npulses1=0;
  npulses2=0;
  for ( iom = responsemap->begin(); iom != responsemap->end(); ++iom ){
    const OMKey &om = iom->first;
    I3OMGeoMap::const_iterator igeo = geoptr->omgeo.find(om);
    if ( igeo == geoptr->omgeo.end() ){
      log_warn("(%s) unknown DOM(%d,%u) in input hitset",
	       name.c_str(),
	       igeo->first.GetString(), igeo->first.GetOM() );
      continue;
    }
    if(wantRotated_){
      rotated=(igeo->second.position);
      rotated.RotateZ(-track->GetAzimuth());
      rotated.RotateY(-track->GetZenith());
      shortx=rotated.GetX();
      shorty=rotated.GetY();
      shortz=rotated.GetZ();
    }
    else{
      shortx=igeo->second.position.GetX();
      shorty=igeo->second.position.GetY();
      shortz=igeo->second.position.GetZ();
    }
    rdistance1=sqrt((shortx-savemincentroid1[0])*(shortx-savemincentroid1[0])+(shorty-savemincentroid1[1])*(shorty-savemincentroid1[1]));
    rdistance2=sqrt((shortx-savemincentroid2[0])*(shortx-savemincentroid2[0])+(shorty-savemincentroid2[1])*(shorty-savemincentroid2[1]));
    if(rdistance1>=rdistance2)npulses1++;
    else npulses2++;
    typename ResponseSeries::const_iterator ipulse;
      for ( ipulse = iom->second.begin(); ipulse != iom->second.end(); ++ipulse ){
      if(rdistance1>=rdistance2){
	//	log_info("bin 1 String %d OM %d distance %g ( %g , %g , %g )",igeo->first.GetString(),igeo->first.GetOM(),rdistance1,shortx,shorty,shortz);
	(*newmap1)[om].push_back( *ipulse );
	++nhits1;
      }
      else {
	//	log_info("bin 2 String %d OM %d distance %g ( %g , %g , %g )",igeo->first.GetString(),igeo->first.GetOM(),rdistance1,shortx,shorty,shortz);
	(*newmap2)[om].push_back( *ipulse );
	++nhits2;
      }
    }
  }
  //  log_info("npulses1 %d npulses2 %d",npulses1,npulses2);
}



//----------------------------------------------------------------------------
// We made a template here in the time when we still used both I3RecoPulse and I3RecoHit.
// Right now we instantiate this method only with I3RecoPulse, so we could consider
// de-templating this method. On the other hand: maybe someone some day would like to
// split an I3DOMLaunchSeriesMap. Or PINGU produces its own special kind of pulse and are
// interested in simple splitting algorithms. So let's keep this code like it is.
template<class Response>
void I3ResponseMapSplitter::SplitMap( I3FramePtr frame ){

    // convenience typedefs
    typedef std::vector< Response > ResponseSeries;
    typedef I3Map< OMKey, ResponseSeries > ResponseMap;
    typedef boost::shared_ptr< ResponseMap > ResponseMapPtr;
    typedef boost::shared_ptr< const ResponseMap > ResponseMapConstPtr;

    // input hits/pulses
    ResponseMapConstPtr responsemap = frame->template Get< ResponseMapConstPtr >( inResponseMapName_ );

    // output hits/pulses
    ResponseMapPtr newmap1( new ResponseMap );
    ResponseMapPtr newmap2( new ResponseMap );

    bool ok = ( responsemap && ( int(responsemap->size()) >= minNch_ ) );

    if ( ok ){
        ++nEvents_;
        unsigned int nhits1=0;
        unsigned int nhits2=0;
        I3ParticleConstPtr track;
        if ( !inTrackName_.empty() ){
            track = frame->Get<I3ParticleConstPtr>( inTrackName_ );
        }
        I3GeometryConstPtr geoptr = frame->Get<I3GeometryConstPtr>(geoName_);
        if ( !geoptr ){
            log_fatal( "(%s) Failed to find geometry in the frame.",
                       GetName().c_str() );
        }
        if ( track && ( track->GetFitStatus() == I3Particle::OK )){
            log_debug( "(%s) using track \"%s\"",
                       GetName().c_str(), inTrackName_.c_str() );
            if( doTRes_ ){ // split according to time residual
	      if(doKmeans_ || doBrightSt_){            
		log_fatal("(%s) Multiple options found. Please only pick one of the following: DoKmeans, DoBrightSt or DoTRes",GetName().c_str());
	      }
	      SplitByTimeResidual(minTRes_, maxTRes_,
                                    track, geoptr, responsemap,
                                    newmap1, newmap2, nhits1, nhits2,
                                    GetName());
            } else if( doKmeans_ && wantRotated_ ){ // split with K-means clustering algorithm
	      if(doTRes_ || doBrightSt_){            
		log_fatal("(%s) Multiple options found. Please only pick one of the following: DoKmeans, DoBrightSt or DoTRes",GetName().c_str());
	      }
	      //  	        I3RandomService& rnd = context_.Get<I3RandomService>();
  	        I3RandomServicePtr rnd = context_.Get<I3RandomServicePtr>();
		unsigned int npulses1=0;
		unsigned int npulses2=0;
	        SplitByKmeans(wantRotated_,track, geoptr, responsemap,
			      newmap1, newmap2, npulses1, npulses2, nhits1, nhits2,
			      GetName(),rnd);
		//		log_info("after pass newmap1 %d newmap2 %d",(int)newmap1->size(),(int)newmap2->size());
		if(npulses2>=npulses1){
		  //		  log_info("before swap newmap1 %d newmap2 %d",(int)(*newmap1).size(),(int)(*newmap2).size());
		  newmap1.swap(newmap2);
		  //		  log_info("after swap newmap1 %d newmap2 %d",(int)(*newmap1).size(),(int)(*newmap2).size());
		}
	    }else { // split according to COG
                SplitByTrackAndCOG(track, geoptr, responsemap,
                                   newmap1, newmap2, nhits1, nhits2, GetName());
            }
        } else if (doBrightSt_){
	  if(doKmeans_ || doTRes_){            
	    log_fatal("(%s) Multiple options found. Please only pick one of the following: DoKmeans, DoBrightSt or DoTRes",GetName().c_str());
	  }
	  SplitByBrightness( maxDBrightSt_, geoptr, responsemap,
                               newmap1, newmap2,
                               nhits1, nhits2, GetName() );
	} else if( doKmeans_ && !wantRotated_ ){ // split with K-means clustering algorithm
	  if(doTRes_ || doBrightSt_){            
	    log_fatal("(%s) Multiple options found. Please only pick one of the following: DoKmeans, DoBrightSt or DoTRes",GetName().c_str());
	  }
	  //  	        I3RandomService& rnd = context_.Get<I3RandomService>();
	  I3RandomServicePtr rnd = context_.Get<I3RandomServicePtr>();
	  unsigned int npulses1=0;
	  unsigned int npulses2=0;
	  SplitByKmeans(wantRotated_,track, geoptr, responsemap,
			newmap1, newmap2, npulses1, npulses2, nhits1, nhits2,
			GetName(),rnd);
	  //		log_info("after pass newmap1 %d newmap2 %d",(int)newmap1->size(),(int)newmap2->size());
	  if(npulses2>=npulses1){
	    //		  log_info("before swap newmap1 %d newmap2 %d",(int)(*newmap1).size(),(int)(*newmap2).size());
	    newmap1.swap(newmap2);
	    //		  log_info("after swap newmap1 %d newmap2 %d",(int)(*newmap1).size(),(int)(*newmap2).size());
	  }
        } else {
            // ------ splitting hit/pulse set using average hit time ---------
            // (because no track was configured, or because the input fit was not OK)
            if ( !inTrackName_.empty() ){
                log_info("(%s) Requested track \"%s\" failed or is not present, "
                         "defaulting to splitting pulse/hit series "
                         "using %s hit time",
                         GetName().c_str(), inTrackName_.c_str(), (doTMedian_?"median":"average") );
            } else {
	        log_info("(%s) Splitting by %s time",GetName().c_str(), (doTMedian_?"median":"average"));
            }
            SplitByTime<Response>(responsemap,newmap1,newmap2,nhits1,nhits2,doTMedian_,tSplitWeight_,GetName());
        }
        if ( nMinHits1_ > nhits1 ) nMinHits1_ = nhits1;
        if ( nMaxHits1_ < nhits1 ) nMaxHits1_ = nhits1;
        nHits1_ += nhits1;
        if ( nMinHits2_ > nhits2 ) nMinHits2_ = nhits2;
        if ( nMaxHits2_ < nhits2 ) nMaxHits2_ = nhits2;
        nHits2_ += nhits2;
        log_debug( "(%s) NCh/NHits(1)=%zu/%u,  NCh/NHits(2)=%zu/%u",
                   GetName().c_str(), newmap1->size(), nhits1,
                                      newmap2->size(), nhits2 );
    } else {
        ++nBadEvents_;
        log_warn( "(%s) event not ok: %s",
                  GetName().c_str(),
                  responsemap ? "too few hits/pulses" : "missing hit/pulse map" );
        log_warn( "(%s) So, return it", GetName().c_str());
	PushFrame( frame, "OutBox" );
	return;
	
    }

    WriteOutput<Response>(frame, newmap1, newmap2);
}

//----------------------------------------------------------------------------

template <typename Response>
void
I3ResponseMapSplitter::WriteOutput(I3FramePtr frame,
    typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap1,
    typename boost::shared_ptr< I3Map< OMKey, std::vector< Response > > > newmap2)
{
    frame->Put( outResponseMap1Name_, newmap1 );
    frame->Put( outResponseMap2Name_, newmap2 );
    PushFrame( frame, "OutBox" );
}

//----------------------------------------------------------------------------
// Specialization for I3RecoPulse: instead of blindly writing a copy of a
// subset of the input to the frame, write a mask that represents that subset
// in many fewer bytes.
template <>
void
I3ResponseMapSplitter::WriteOutput<I3RecoPulse>(I3FramePtr frame,
    I3RecoPulseSeriesMapPtr newmap1,
    I3RecoPulseSeriesMapPtr newmap2)
{
	I3RecoPulseSeriesMapMaskPtr mask1(
	    new I3RecoPulseSeriesMapMask(*frame, inResponseMapName_, *newmap1));
	I3RecoPulseSeriesMapMaskPtr mask2(
	    new I3RecoPulseSeriesMapMask(*frame, inResponseMapName_, *newmap2));
   
	if (splitEvents_) {
		// Emit a new Physics frame for each subevent.
		I3FramePtr subframe;
	
		// Send the DAQ frame down the river
		assert(frame->GetStop() == I3Frame::DAQ);
		PushFrame(frame);
	
		// Send both halves of the event
		subframe = GetNextSubEvent(frame);
		subframe->Put( outResponseMap1Name_, mask1);
		PushFrame(subframe);
	
		subframe = GetNextSubEvent(frame);
		subframe->Put( outResponseMap1Name_, mask2);
		PushFrame(subframe);
	} else {
		// Put the subevents in numbered maps in the same frame.
		frame->Put( outResponseMap1Name_, mask1 );
		frame->Put( outResponseMap2Name_, mask2 );
		PushFrame(frame);
	}
}

//----------------------------------------------------------------------------
void I3ResponseMapSplitter::Physics(I3FramePtr frame){

    // Return instantly if running as a splitter
    if (splitEvents_) {
        PushFrame(frame);
        return;
    }

    log_info( "(%s) Welcome to the physics method of ResponseMapSplitter!",
              GetName().c_str());

    SplitMap<I3RecoPulse>( frame );

    log_debug( "(%s) Leaving I3ResponseMapSplitter Physics().",
               GetName().c_str() );
}

void I3ResponseMapSplitter::DAQ(I3FramePtr frame){
    
    // Return instantly if _not_ running as a splitter
    if (!splitEvents_) {
        PushFrame(frame);
        return;
    }

    log_info( "(%s) Welcome to the DAQ method of ResponseMapSplitter!",
              GetName().c_str());

    SplitMap<I3RecoPulse>( frame );

    log_debug( "(%s) Leaving I3ResponseMapSplitter DAQ().",
               GetName().c_str() );
}

void I3ResponseMapSplitter::Finish(){
    log_info( "(%s) %u events with Nch>=%d in %sset \"%s\"",
              GetName().c_str(), nEvents_, minNch_,
              "pulse", inPulseMapName_.c_str() );
    if ( nBadEvents_ > 0 ){
        log_info( "(%s) %u events with Nch<%d or missing %sset",
                  GetName().c_str(), nBadEvents_, minNch_,
                  "pulse" );
    }
    if ( nEvents_ > 0 ){
        log_info( "(%s) %u %s in set1 (min %u, average %.1f, max %u)",
                  GetName().c_str(), nHits1_,
                  "pulses", nMinHits1_, nHits1_*1.0/nEvents_, nMaxHits1_ );
        log_info( "(%s) %u %s in set2 (min %u, average %.1f, max %u)",
                  GetName().c_str(), nHits2_, "pulses",
                  nMinHits2_, nHits2_*1.0/nEvents_, nMaxHits2_ );
    }
}
