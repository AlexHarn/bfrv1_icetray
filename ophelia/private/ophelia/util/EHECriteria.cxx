#include <iostream>
#include <fstream>
#include "ophelia/util/EHECriteria.h"

#include "icetray/I3Units.h"

using namespace std;

double EHECriteria::shallowDeepBoundaryDepth_ = -300.0*I3Units::m; // IC40 default
int EHECriteria::nDOMThreshold_ = 200; // IC40 default
string EHECriteria::criteria_data_directory_ = string(getenv("I3_BUILD"));

//
//  The default critera definition in the map<dounle,double>
//  Note: The method map_list_of here initialized the map with
//        the values below in ascending order of key: The smallest key element comes firstest.
//
double_map EHECriteria::shallowNPECriteria_ =  // IC40 EHE shallow event cut (cosZ,NPE)
map_list_of
  (1.0, 2.5118864e6) // NPE 1e6.4
  (0.8, 1.5848932e6) // NPE 1e6.2
  (0.6, 1.0000000e6) // NPE 1e6
  (0.4, 6.3095734e5) // NPE 1e5.8
  (0.3, 6.3095734e4) // NPE 1e4.8
  (0.2, 2.5118864e4);  // NPE 1e4.4

double_map EHECriteria::deepNPECriteria_ =  // IC40 EHE deep event cut (dT,NPE)
map_list_of
  (1.0*I3Units::second, 1.9952623e6) // NPE 1e6.3
  (2500*I3Units::ns, 1.0000000e6) // NPE 1e6
  (1500*I3Units::ns, 7.9432823e5) // NPE 1e5.9
  (500*I3Units::ns,  7.9432823e4);  // NPE 1e4.9

//_________________________________________________________________________
EHECriteria::EHECriteria(const string& filename) 
{

  if(filename != ""){ // Then readout the NPE threshold setting from the file
    log_info(" not using the default setting. Read data");
    criteria_data_directory_.append("/ophelia/resources/");
    ReadEHECriteriaSetting(filename);
  }

  PrintEHECriteria();

}

//_________________________________________________________________________
bool EHECriteria::IsThisEventShallow(double zDepthOfThisEvent)
{

  if(zDepthOfThisEvent<GetShallowDeepBoundaryDepth()) return false;
  else return true;

}

//_________________________________________________________________________
bool EHECriteria::PassEHECriteria(double keyForNPE, double npe, int nDOM, bool isThisEventShallow)
{

  bool passThisEvent = false;
  if(nDOM< nDOMThreshold_) return passThisEvent;

  double_map* npeCriteriaPtr;
  if(isThisEventShallow) npeCriteriaPtr = &shallowNPECriteria_;
  else npeCriteriaPtr = &deepNPECriteria_;

  double_map::iterator criteria_iter = npeCriteriaPtr->begin();
  double thresholdNPE = criteria_iter->second;
  for(;criteria_iter!= npeCriteriaPtr->end(); criteria_iter++){
    double key = criteria_iter->first;
    thresholdNPE = criteria_iter->second;
    if(key> keyForNPE){ // This event is subject to the key (such as cos(zenith))
      break;            // in the previous loop
    }
  }

  log_debug(" This event with NPE of %e Key %f is compared to threshold of %e ",npe, keyForNPE,thresholdNPE);
  if(thresholdNPE<= npe) passThisEvent = true;

  return passThisEvent;
  
}

//_________________________________________________________________________
void EHECriteria::PrintEHECriteria()
{

  // Printing the Shallow event criteria
  double_map::iterator criteria_iter = shallowNPECriteria_.begin();
  for(;criteria_iter!= shallowNPECriteria_.end(); criteria_iter++){
    double keyForNPE = criteria_iter->first;
    double thresholdNPE = criteria_iter->second;

    log_debug(" Shallow: key for NPE %f  NPEthreshold = %e", keyForNPE,thresholdNPE);

  }

  // Printing the Deep event criteria
  criteria_iter = deepNPECriteria_.begin();
  for(;criteria_iter!= deepNPECriteria_.end(); criteria_iter++){
    double keyForNPE = criteria_iter->first;
    double thresholdNPE = criteria_iter->second;

    log_debug(" Deep: key for NPE %f  NPEthreshold = %e", keyForNPE,thresholdNPE);

  }
}
//_________________________________________________________________________
void EHECriteria::ReadEHECriteriaSetting(const string& filename)
{

  string criteriaFileName = criteria_data_directory_ + filename;
  log_debug(" reading the criteria from %s",criteriaFileName.c_str());

  // Clearing the criteria maps
  shallowNPECriteria_.clear();
  deepNPECriteria_.clear();

  // Open the file 
  ifstream cfile(criteriaFileName.c_str());
  if (!cfile.good()) {
    cerr << "failed to open " << criteriaFileName << endl;
  }

  // Reading the file
  double key;
  double npe;
  string option;
  int numberOfCriteria = 0;
  while(!cfile.eof()){
    cfile >> option >> key >> npe;

    if(option == "shallow") shallowNPECriteria_[key] = npe;
    else deepNPECriteria_[key] = npe;
    numberOfCriteria++;
    if(numberOfCriteria>=20){
      cerr << " You set too much sets of criteria. We break here to avoid leaks." 
	   << endl;
      break;
    }
  }
  cfile.close();



}

