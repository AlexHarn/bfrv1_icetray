/**
 *
 * Definition of I3CascadeFitCoreRemovalTester
 *
 * (c) 2009
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3CascadeFitCoreRemovalTester.cxx
 * @date $Date$
 * @author panknin
 *
 */

#include "icetray/I3Context.h"
#include "icetray/I3Module.h"
#include <icetray/I3Frame.h>
#include "I3CascadeFitCoreRemovalTester.h"

I3CascadeFitCoreRemovalTester::I3CascadeFitCoreRemovalTester () {
  I3Context context;
  I3ConfigurationPtr config (new I3Configuration ());
  config->InstanceName ("test");
  config->ClassName ("I3CascadeFitCoreRemoval");  
  context.Put (config);
  boost::shared_ptr<std::map<std::string, 
                             std::pair<FrameFifoPtr, I3ModulePtr> > > 
    connections (new std::map<std::string, 
		              std::pair<FrameFifoPtr, I3ModulePtr> >);

  (*connections)["OutBox"] = make_pair(FrameFifoPtr(), I3ModulePtr());
  context.Put (connections, "OutBoxes");

  coreRemover_ = boost::shared_ptr<I3CascadeFitCoreRemoval > 
    (new I3CascadeFitCoreRemoval (context));

  //config->Set();
  
  coreRemover_->Configure ();

}
