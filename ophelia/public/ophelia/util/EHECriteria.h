#ifndef EHECRITERIA_H_INCLUDED
#define EHECRITERIA_H_INCLUDED

#include "icetray/I3Logging.h"

#include <string>
#include <boost/assign/list_of.hpp>
#include <boost/assign.hpp>
#include <map>

using namespace boost::assign;

//typedef map<double,double>::value_type double_pair;
typedef std::map<double,double> double_map;

class EHECriteria {

public :

  EHECriteria(const std::string& filename = "");
  virtual ~EHECriteria(){}

  static const double GetShallowDeepBoundaryDepth() { return shallowDeepBoundaryDepth_;};
  static void SetShallowDepthBoundary(double depth) { shallowDeepBoundaryDepth_ = depth;};

  static const int GetNDOMThreshold() { return nDOMThreshold_;};
  static void SetNDOMThreshld(int ndom) { nDOMThreshold_ = ndom;};


  /**
     Judge whether it satisfies the EHE criteria.

     nDOM is the threshold of number of launched DOMs.
     keyForNPE can be cos(zenith) for shallow events, t1st-tLND for deep events if IC40
     npe is the npe threshold for evenys with keyForNPE
     In other words, the criteia should be defined on NPE-keyForNPE plane.
   */
  bool PassEHECriteria(double keyForNPE, double npe, int nDOM, bool isThisEventShallow = true);

  /**
      Return true if this event belongs to "shallow event" category
   */
  bool IsThisEventShallow(double zDepthOfThisEvent);

  /**
     Set the NPE criteria for shallow events. The criteria must be described in
     map(keyForNPE, npe_threshold) which constrcutes the criteria line on NPE-KeyForNPE plane
   */
  virtual void SetNPECriteriaForShallowEvents(double_map& npeMap){ shallowNPECriteria_ = npeMap;};
  /**
     Set the NPE criteria for deep events. The criteria must be described in
     map(keyForNPE, npe_threshold) which constrcutes the criteria line on NPE-KeyForNPE plane
   */
  virtual void SetNPECriteriaForDeepEvents(double_map& npeMap){ deepNPECriteria_ = npeMap;};

  /**
     Read the criteria (keyForNPE, thresholdNPE) from filename in the directory criteria_data_directory_
     which should be $I3_BUILD/ophelia/resources/.

     File fname should store the criteria in the following format

     shallow  keyForNPE NPEThreshold
     shallow  keyForNPE NPEThereshold
        ...
     deep     keyForNPE NPEThesold
        ...

     Important Note: keyForNPE must be placed in ascending order.
   */
  virtual void ReadEHECriteriaSetting(const std::string& filename); 

  /**
     Print the Criteria values stored in the maps.
     Mainly for debugging puroposes.
   */
  virtual void PrintEHECriteria();


 private:
  /**       boundary depth in z to define shallow/deep   */
  static double shallowDeepBoundaryDepth_;

  /* NDOM threshold */
  static int nDOMThreshold_;

  /* the directory to store the file of the criteria. You may use it for non-default setting */
  static std::string criteria_data_directory_;

  /* EHE Criteria map<double,double>  for shallow events  */
  static double_map shallowNPECriteria_;// default: IC40 Shallow (coszenith, NPE)

  /* EHE Criteria map<double,double>  for deep events  */
  static double_map deepNPECriteria_; // default: IC40 Deep (dT, NPE)


  SET_LOGGER("EHECriteria");

};

#endif //EHECRITERIA_H_INCLUDED
