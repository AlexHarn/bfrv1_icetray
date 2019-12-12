#include <I3Test.h>
#include <queue>
#include <icetray/I3Int.h>
#include <icetray/I3Tray.h>
#include <icetray/I3Units.h>
#include <dataclasses/physics/I3MCTreeUtils.h>
#include <simclasses/I3MCPE.h>
#include <simclasses/I3Photon.h>
#include <simclasses/I3CompressedPhoton.h>
#include <simclasses/I3MMCTrack.h>
#include <phys-services/I3GSLRandomService.h>
#include <polyplopia/PoissonPEMerger.h>
#include <pybindings.hpp>
#include <boost/python/init.hpp>

I3_DEFAULT_NAME(I3MCPE);
I3_DEFAULT_NAME(I3Photon);
I3_DEFAULT_NAME(I3CompressedPhoton);

namespace{

template<typename PhotonType>
struct photon_traits{};

template<>
struct photon_traits<I3MCPE>{
  using container_type=I3MCPESeriesMap;
  
  static I3MCPE make_with_time(double t){
    return(I3MCPE(1,t));
  }
  static double get_time(const I3MCPE& p){
    return(p.time);
  }
};

template<>
struct photon_traits<I3Photon>{
  using container_type=I3PhotonSeriesMap;
  
  static I3Photon make_with_time(double t){
    I3Photon p;
    p.SetTime(t);
    return(p);
  }
  static double get_time(const I3Photon& p){
    return(p.GetTime());
  }
};

template<>
struct photon_traits<I3CompressedPhoton>{
  using container_type=I3CompressedPhotonSeriesMap;
  
  static I3CompressedPhoton make_with_time(double t){
    I3CompressedPhoton p;
    p.SetTime(t);
    return(p);
  }
  static double get_time(const I3CompressedPhoton& p){
    return(p.GetTime());
  }
};
  
///Make a copy of a tree which is identical, except that all particles are
///copied to get new IDs.
boost::shared_ptr<I3MCTree> tree_copy_new_particles(const I3MCTree& tree){
  boost::shared_ptr<I3MCTree> new_tree(new I3MCTree);
  std::unordered_map<I3ParticleID,I3ParticleID,i3hash<I3ParticleID>> id_map;
  //this relies on the default iteration being in pre-order!
  for(const I3Particle& p : tree){
    I3Particle pnew(p.GetShape(),p.GetType());
    pnew.SetTime(p.GetTime());
    id_map[p]=pnew;
    auto parent=tree.parent(p);
    if(parent)
      new_tree->append_child(id_map[*parent],pnew);
    else
      new_tree->insert(pnew);
  }
  return(new_tree);
}

///Trvial background event generator which produces data at a single, set instant
template<typename PhotonType>
class TestBackgroundService : public I3GeneratorService{
private:
  using ptraits=photon_traits<PhotonType>;
  using HitContainerType=typename ptraits::container_type;
  using KeyType=typename HitContainerType::key_type;
public:
  TestBackgroundService(double rate, I3MCTree tree, double time,
                        std::string hitsName,
                        std::initializer_list<KeyType> hitKeys,
                        I3MMCTrackList tracks):
  rate(rate),tree(tree),time(time),hitsName(hitsName),tracks(tracks){
    for(auto key : hitKeys)
      hits[key].push_back(ptraits::make_with_time(time));
    for(I3Particle& p : this->tree)
      p.SetTime(time);
    for(I3MMCTrack& track : this->tracks)
      track.ti=time;
  }
  
  double GetRate() override{ return(rate); }
  
  boost::shared_ptr<I3MCTree> GetNextEvent() override{
    log_fatal("Not valid");
  }
  
  boost::shared_ptr<I3Frame> GetNextFrame() override{
    boost::shared_ptr<I3Frame> frame(new I3Frame(I3Frame::DAQ));
    frame->Put("I3MCTree",tree_copy_new_particles(tree));
    frame->Put(hitsName,boost::make_shared<HitContainerType>(hits));
    frame->Put("MMCTrackList",boost::make_shared<I3MMCTrackList>(tracks));
    return(frame);
  }
  
private:
  double rate;
  I3MCTree tree;
  double time;
  std::string hitsName;
  HitContainerType hits;
  I3MMCTrackList tracks;
};
  
class GeneratorWrapper : public I3Module{
public:
  GeneratorWrapper(const I3Context& ctx):I3Module(ctx){
    AddParameter("Generator","The generator");
    AddOutBox("OutBox");
  }
  void Configure(){
    GetParameter("Generator",generator);
    assert(generator);
  }
  void Process(){
    auto next=generator->GetNextFrame();
    if(!next)
      RequestSuspension();
    else
      PushFrame(next);
  }
private:
  boost::shared_ptr<I3GeneratorService> generator;
};
  
I3_MODULE(GeneratorWrapper);
  
class Collector : public I3Module{
public:
  Collector(const I3Context& ctx):I3Module(ctx){
    AddParameter("Queue","Results");
  }
  void Configure(){
    GetParameter("Queue",queue);
    assert(queue);
  }
  void Process(){
    boost::shared_ptr<I3Frame> frame;
    while((frame=PopFrame()))
      queue->push(frame);
  }
private:
  std::queue<boost::shared_ptr<I3Frame>>* queue;
};
  
I3_MODULE(Collector);

template<typename PType>
void do_test(const std::string hitsOptionName){
  const double timeWindow=10*I3Units::ns;
  const double baseTime=22000;
  
  using ptraits=photon_traits<PType>;
  using Container=typename photon_traits<PType>::container_type;
  using Key=typename Container::key_type;
  
  boost::shared_ptr<TestBackgroundService<PType>> gen1, gen2;
  {
    I3Particle p1(I3Particle::Primary,I3Particle::NuMu);
    I3Particle p2(I3Particle::StartingTrack,I3Particle::MuMinus);
    I3Particle p3(I3Particle::Cascade,I3Particle::Hadrons);
    I3MCTree tree;
    tree.insert(p1);
    tree.append_child(p1,p2);
    tree.append_child(p1,p3);
    I3MMCTrackList tracks(1,I3MMCTrack());
    gen1.reset(new TestBackgroundService<PType>(1*I3Units::hertz,tree,baseTime,"Hits",
      {Key{7,6},Key{12,2},Key{19,54},Key{45,16},Key{79,36}},tracks));
  }
  {
    I3Particle p1(I3Particle::Primary,I3Particle::PPlus);
    I3Particle p2(I3Particle::StartingTrack,I3Particle::MuMinus);
    I3Particle p3(I3Particle::StartingTrack,I3Particle::MuPlus);
    I3Particle p4(I3Particle::StartingTrack,I3Particle::MuPlus);
    I3MCTree tree;
    tree.insert(p1);
    tree.append_child(p1,p2);
    tree.append_child(p1,p3);
    tree.append_child(p1,p4);
    I3MMCTrackList tracks(3,I3MMCTrack());
    gen2.reset(new TestBackgroundService<PType>(1e8*I3Units::hertz,tree,6000,"Hits",
      {Key{13,1},Key{14,2},Key{19,54},Key{20,56}},tracks));
  }
  boost::shared_ptr<I3GSLRandomService> rng(new I3GSLRandomService(52,false));
  boost::shared_ptr<std::queue<boost::shared_ptr<I3Frame>>> results(new std::queue<boost::shared_ptr<I3Frame>>);
  
  //deal with python retardation
  namespace bp=boost::python;
  bp::import("icecube.phys_services");
  bp::import("icecube.sim_services");
  bp::class_<TestBackgroundService<PType>,bp::bases<I3GeneratorService>,
             boost::shared_ptr<TestBackgroundService<PType>>,
             boost::noncopyable>((std::string("TestBackgroundService")+I3DefaultName<PType>::value()).c_str(),bp::no_init);
  bp::class_<std::queue<boost::shared_ptr<I3Frame>>,bp::bases<>,
             boost::shared_ptr<std::queue<boost::shared_ptr<I3Frame>>>,
             boost::noncopyable>("FrameQueue",bp::no_init);
  
  I3Tray tray;
  tray.AddModule("GeneratorWrapper")("Generator",gen1);
  tray.AddModule("PoissonPEMerger")
    ("BaseIsBackground",false)
    ("CoincidentEventService",gen2)
    ("TimeWindow",timeWindow)
    ("RandomService",rng)
    ("MCTreeName","I3MCTree")
    (hitsOptionName,"Hits")
    ("MMCTrackName","MMCTrackList");
  tray.AddModule("Collector")("Queue",results);
  tray.Execute(1000);
  
  boost::shared_ptr<I3Frame> frame;
  while(!results->empty()){
    frame=results->front();
    results->pop();
    uint32_t count=frame->Get<I3Int>("PolyplopiaCount").value;
    
    //check the MCTree
    auto tree=frame->Get<I3MCTree>();
    auto primaries = I3MCTreeUtils::GetPrimaries(tree);
    ENSURE_EQUAL(count+1,primaries.size(),"There should be one primary per merged event, plus the base primary");
    ENSURE_EQUAL(count*4+3,tree.size(),"All particles from each merged event, plus the base event should be present");
    for(const I3Particle& p : tree)
      ENSURE_DISTANCE(p.GetTime(),baseTime,timeWindow/2,"All particles must be in the time window");
    
    //check the hits
    Container hits=frame->Get<Container>("Hits");
    unsigned int nHits=0;
    for(const auto& module_entry : hits){
      double lastTime=NAN;
      for(const PType& p : module_entry.second){
        ENSURE_DISTANCE(ptraits::get_time(p),baseTime,timeWindow/2,"All hits must be in the time window");
        ENSURE(!(ptraits::get_time(p)<lastTime),"Hits must be time ordered");
        lastTime=ptraits::get_time(p);
        nHits++;
      }
    }
    ENSURE_EQUAL(count*4+5,nHits,"All hits from each merged event, plus the base event should be present");
    
    //check the tracks
    auto tracks=frame->Get<I3MMCTrackList>("MMCTrackList");
    ENSURE_EQUAL(count*3+1,tracks.size(),"All muon tracks from each merged event, plus the base event should be present");
    for(const I3MMCTrack& track : tracks)
      ENSURE_DISTANCE(track.ti,baseTime,timeWindow/2,"All muon tracks must be in the time window");
    
  }
}
  
} //end anonymous namespace

TEST_GROUP(PoissonPEMerger);

TEST(MergeMCPE){
  do_test<I3MCPE>("MCPEsToMerge");
}

TEST(MergePhoton){
  do_test<I3Photon>("PhotonsToMerge");
}

TEST(MergeCompressedPhoton){
  do_test<I3CompressedPhoton>("PhotonsToMerge");
}
