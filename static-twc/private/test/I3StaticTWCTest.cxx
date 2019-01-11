#include "I3Test.h"

#include "icetray/I3Tray.h"
#include "icetray/I3Frame.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/TriggerKey.h"
#include "dataclasses/physics/I3TriggerHierarchy.h"



TEST_GROUP(StaticTWCTest);

namespace testStaticTWC
{
  template <class Tester>
  class TesterSource : public I3Module
  {
  public:
    TesterSource(const I3Context& context): I3Module(context)
    {
      AddOutBox("OutBox");
      called = false;
    }
    void Process()
    {
      if(called)
        {
          RequestSuspension();
          return;
        }
      I3FramePtr frame(new I3Frame(I3Frame::Physics));
      frame->Put("input",tester.GetDirtyHits());
      called = true;
      
      // Put a DC trigger with 0.0 ns trigger time
      TriggerKey tkey(TriggerKey::IN_ICE, TriggerKey::SIMPLE_MULTIPLICITY , 1010 );
      I3TriggerHierarchyPtr trigger(new I3TriggerHierarchy);
      I3Trigger t;
      t.GetTriggerKey() = tkey;
      t.SetTriggerFired(true);
      t.SetTriggerTime(0.0);
      trigger->insert(trigger->begin(),t);
      frame->Put("I3TriggerHierarchy",trigger);

      PushFrame(frame,"OutBox");
    }
  private:
    Tester tester;
    bool called;
  };
  
  
  template <class Tester>
  class TesterClient : public I3Module
  {
  public:
    TesterClient(const I3Context& context): I3Module(context)
    {
      AddOutBox("OutBox");
    }
    
    void Physics(I3FramePtr frame)
    {
      I3RecoPulseSeriesMapConstPtr pulses = 
        frame->template Get<I3RecoPulseSeriesMapConstPtr>("cleaned");
      ENSURE((bool)pulses,"Found the cleaned pulses");
      
      tester.CheckCleanedHits(*pulses);
      
      PushFrame(frame,"OutBox");
    }
    
  private:
    Tester tester;
  };
};

namespace testStaticTWC
{

  // Test when no hits are cleaned
  struct NoCleaningTest
  {
    I3RecoPulseSeriesMapPtr GetDirtyHits()
    {
      I3RecoPulseSeriesMapPtr toReturn(new I3RecoPulseSeriesMap());
      
      I3RecoPulse hit_0_0;
      hit_0_0.SetTime(0);
      I3RecoPulseSeries series_0_0;
      series_0_0.push_back(hit_0_0);
      (*toReturn)[OMKey(0,0)] = series_0_0;
      
      I3RecoPulse hit_0_1;
      hit_0_1.SetTime(400);
      I3RecoPulseSeries series_0_1;
      series_0_1.push_back(hit_0_1);
      (*toReturn)[OMKey(0,1)] = series_0_1;
      
      I3RecoPulse hit_0_2;
      hit_0_2.SetTime(-400);
      I3RecoPulseSeries series_0_2;
      series_0_2.push_back(hit_0_2);
      (*toReturn)[OMKey(0,2)] = series_0_2;
      
      I3RecoPulse hit_0_3;
      hit_0_3.SetTime(0);
      I3RecoPulseSeries series_0_3;
      series_0_3.push_back(hit_0_3);
      (*toReturn)[OMKey(0,3)] = series_0_3;
      
      
      return toReturn;
    }
    
    void CheckCleanedHits(const I3RecoPulseSeriesMap& pulses)
    {
      ENSURE(pulses.size() == 4);
    }
  };
  
  I3_MODULE(TesterSource<NoCleaningTest>);
  I3_MODULE(TesterClient<NoCleaningTest>);
};

TEST(NoCleaningTest)
{
  I3Tray tray;
  
  tray.AddModule("TesterSource<NoCleaningTest>","source");
  
  std::vector<int> triggerConfigIDList(1, 1010);
  
  tray.AddModule("I3StaticTWC<I3RecoPulseSeries>","cleaning")
    ("InputResponse","input")
    ("OutputResponse","cleaned")
    ("TriggerConfigIDs",triggerConfigIDList)
    ("TriggerName","I3TriggerHierarchy")
    ("WindowMinus",500.)
    ("WindowPlus",500.)
    ("FirstTriggerOnly",false);
  
  tray.AddModule("Dump","dump");
  
  tray.AddModule("TesterClient<NoCleaningTest>","client");
  
  
  
  tray.Execute();
  
}

namespace testStaticTWC
{
  // Test with one early hit
  struct EarlyHitTest
  {
    I3RecoPulseSeriesMapPtr GetDirtyHits()
    {
      I3RecoPulseSeriesMapPtr toReturn(new I3RecoPulseSeriesMap());
      
      I3RecoPulse hit_0_0;
      hit_0_0.SetTime(-1200.);
      I3RecoPulseSeries series_0_0;
      series_0_0.push_back(hit_0_0);
      (*toReturn)[OMKey(0,0)] = series_0_0;
      
      I3RecoPulse hit_0_1;
      hit_0_1.SetTime(0);
      I3RecoPulseSeries series_0_1;
      series_0_1.push_back(hit_0_1);
      (*toReturn)[OMKey(0,1)] = series_0_1;
    
      I3RecoPulse hit_0_2;
      hit_0_2.SetTime(0);
      I3RecoPulseSeries series_0_2;
      series_0_2.push_back(hit_0_2);
      (*toReturn)[OMKey(0,2)] = series_0_2;
      
      I3RecoPulse hit_0_3;
      hit_0_3.SetTime(0);
      I3RecoPulseSeries series_0_3;
      series_0_3.push_back(hit_0_3);
      (*toReturn)[OMKey(0,3)] = series_0_3;
      
      
      return toReturn;
    }
    
    void CheckCleanedHits(const I3RecoPulseSeriesMap& pulses)
    {
      ENSURE(pulses.size() == 3);
      ENSURE(pulses.find(OMKey(0,0)) == pulses.end());
    }
  };
  
  I3_MODULE(TesterSource<EarlyHitTest>);
  I3_MODULE(TesterClient<EarlyHitTest>);
  
};

TEST(EarlyHitTest)
{
  I3Tray tray;
  
  tray.AddModule("TesterSource<EarlyHitTest>","source");
  
  std::vector<int> triggerConfigIDList(1, 1010);
  
  tray.AddModule("I3StaticTWC<I3RecoPulseSeries>","cleaning")
    ("InputResponse","input")
    ("OutputResponse","cleaned")
    ("TriggerConfigIDs",triggerConfigIDList)
    ("TriggerName","I3TriggerHierarchy")
    ("WindowMinus",500.)
    ("WindowPlus",500.)
    ("FirstTriggerOnly",false);
  
  tray.AddModule("Dump","dump");
  
  tray.AddModule("TesterClient<EarlyHitTest>","client");
  
  
  
  tray.Execute();
  
}


namespace testStaticTWC
{
  
  // Test with one late hit
  struct LateHitTest
  {
    I3RecoPulseSeriesMapPtr GetDirtyHits()
    {
      I3RecoPulseSeriesMapPtr toReturn(new I3RecoPulseSeriesMap());
      
      I3RecoPulse hit_0_0;
      hit_0_0.SetTime(+1200.);
      I3RecoPulseSeries series_0_0;
      series_0_0.push_back(hit_0_0);
      (*toReturn)[OMKey(0,0)] = series_0_0;
      
      I3RecoPulse hit_0_1;
      hit_0_1.SetTime(0);
      I3RecoPulseSeries series_0_1;
      series_0_1.push_back(hit_0_1);
      (*toReturn)[OMKey(0,1)] = series_0_1;
      
      I3RecoPulse hit_0_2;
      hit_0_2.SetTime(0);
      I3RecoPulseSeries series_0_2;
      series_0_2.push_back(hit_0_2);
      (*toReturn)[OMKey(0,2)] = series_0_2;
      
      I3RecoPulse hit_0_3;
      hit_0_3.SetTime(0);
      I3RecoPulseSeries series_0_3;
      series_0_3.push_back(hit_0_3);
      (*toReturn)[OMKey(0,3)] = series_0_3;
      
      
      return toReturn;
    }
    
    void CheckCleanedHits(const I3RecoPulseSeriesMap& pulses)
    {
      ENSURE(pulses.size() == 3);
    ENSURE(pulses.find(OMKey(0,0)) == pulses.end());
    }
  };
  
  I3_MODULE(TesterSource<LateHitTest>);
  I3_MODULE(TesterClient<LateHitTest>);
  
};

TEST(LateHitTest)
{
  I3Tray tray;
  
  tray.AddModule("TesterSource<LateHitTest>","source");
  
  std::vector<int> triggerConfigIDList(1, 1010);
  
  tray.AddModule("I3StaticTWC<I3RecoPulseSeries>","cleaning")
    ("InputResponse","input")
    ("OutputResponse","cleaned")
    ("TriggerConfigIDs",triggerConfigIDList)
    ("TriggerName","I3TriggerHierarchy")
    ("WindowMinus",500.)
    ("WindowPlus",500.)
    ("FirstTriggerOnly",false);
  
  tray.AddModule("Dump","dump");
  
  tray.AddModule("TesterClient<LateHitTest>","client");
  
  
  
  tray.Execute();
  
}


