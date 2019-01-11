/**
 * copyright  (C) 2005
 * the IceCube collaboration
 * Version $Id$
 */

#include <algorithm>

#include <boost/foreach.hpp>

#include <dataclasses/physics/I3DOMLaunch.h>
#include <dataclasses/I3Vector.h>
#include <icetray/I3Frame.h>

#include <DomTools/I3DOMLaunchCleaning.h>

I3_MODULE(I3DOMLaunchCleaning);

I3DOMLaunchCleaning::I3DOMLaunchCleaning(const I3Context& context) 
  : I3ConditionalModule(context),
    cleanedKeysList_(""),
    inIceInput_("InIceRawData"),
    iceTopInput_("IceTopRawData"),
    inIceOutput_("CleanInIceRawData"),
    iceTopOutput_("CleanIceTopRawData"),
    firstLaunchCleaning_(false)
{
  AddOutBox("OutBox");

  AddParameter("CleanedKeys",
	       "OMKeys to clean out of the launch map",
	       cleanedKeys_);

  AddParameter("CleanedKeysList",
	       "Name of frame vector containing the list of OMKeys to clean out of the launch map",
	       cleanedKeysList_);

  AddParameter("InIceInput",
	       "input inice DOMLaunches",
	       inIceInput_);

  AddParameter("InIceOutput",
	       "output inice DOMLaunches",
	       inIceOutput_);

  AddParameter("IceTopInput",
	       "input icetop DOMLaunches",
	       iceTopInput_);

  AddParameter("IceTopOutput",
	       "output icetop DOMLaunches",
	       iceTopOutput_);

  AddParameter("FirstLaunchCleaning",
	       "True if you want to clean out all but the first DOM launch",
	       firstLaunchCleaning_);
}

void I3DOMLaunchCleaning::Configure()
{
  GetParameter("CleanedKeys",
	       cleanedKeys_);

  GetParameter("CleanedKeysList",
	       cleanedKeysList_);

  GetParameter("InIceInput",
	       inIceInput_);

  GetParameter("InIceOutput",
	       inIceOutput_);

  GetParameter("IceTopInput",
	       iceTopInput_);

  GetParameter("IceTopOutput",
	       iceTopOutput_);

  GetParameter("FirstLaunchCleaning",
	       firstLaunchCleaning_);
           
  if( cleanedKeysList_.length() != 0 ){
    log_debug
      ( "Will use frameobject %s to built list of OMKeys to clean if present.",
        cleanedKeysList_.c_str() );
  }
}

void I3DOMLaunchCleaning::DAQ(I3FramePtr frame)
{

  // By default we will use the list of OMs configured by the
  // the CleanedKeys parameter. If the CleanedKeysList parameter
  // is given we will try to get the list from the frame and
  // overwrite the pointer to the list of OMKeys with the list
  // found in the frame. If we do not find the object in the
  // frame we will fall back to the list configured by the
  // CleanedKeys parameters. This will allow as to use the
  // BadDomList from verification, but use a static list of
  // known bad DOMs if this is list not available.
  std::vector<OMKey> badOmKeys(cleanedKeys_);

  if( cleanedKeysList_.length() != 0 ){
    if( frame->Has( cleanedKeysList_ ) ){
      I3VectorOMKeyConstPtr cleanedKeysListPtr = 
        frame->Get<I3VectorOMKeyConstPtr>(cleanedKeysList_);
      badOmKeys.clear();
      std::copy( cleanedKeysListPtr->begin(), 
                 cleanedKeysListPtr->end(), 
                 std::back_insert_iterator<std::vector<OMKey> >(badOmKeys) );
    }else{
      log_warn
        ( "Configured CleanedKeysList with name %s was not found in frame. "
          "We will revert to the static list.",
          cleanedKeysList_.c_str() );      
    }
  }

  typedef std::pair<std::string, std::string> string_pair;
  std::vector<string_pair> io_name_pairs;
  io_name_pairs.push_back(string_pair(inIceInput_, inIceOutput_));
  io_name_pairs.push_back(string_pair(iceTopInput_, iceTopOutput_));
  
  BOOST_FOREACH(const string_pair& p, io_name_pairs){
    const std::string input_name = p.first;
    const std::string output_name = p.second;

    I3DOMLaunchSeriesMapConstPtr input_launch_map = 
      frame->Get<I3DOMLaunchSeriesMapConstPtr>(input_name);

    if(!input_launch_map){
      // add an empty map and move on
      frame->Put(output_name, I3DOMLaunchSeriesMapPtr(new I3DOMLaunchSeriesMap));
      continue;
    }
      
    I3DOMLaunchSeriesMapPtr output_launch_map =
      I3DOMLaunchSeriesMapPtr(new I3DOMLaunchSeriesMap(*input_launch_map));    

    BOOST_FOREACH(const OMKey& omkey, badOmKeys){
      output_launch_map->erase(omkey);
    }

    if(firstLaunchCleaning_){
      BOOST_FOREACH(I3DOMLaunchSeriesMap::value_type& key_series_pair, *output_launch_map){
        key_series_pair.second.resize(1);
      }
    }
    
    frame->Put(output_name, output_launch_map);
  }
  PushFrame(frame,"OutBox");
}
