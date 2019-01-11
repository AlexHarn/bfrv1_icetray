#ifndef SMALLSHOWER_FILTER_SMALLSHOWERGEOMETRIES_H_INCLUDED
#define SMALLSHOWER_FILTER_SMALLSHOWERGEOMETRIES_H_INCLUDED

#include <set>
#include <string>
#include <vector>


namespace smallshower_filter {

  typedef std::vector< std::set<int> > StationList;

  const StationList* getStation3(const std::string &name);
  const StationList* getStation4(const std::string &name);

}

#endif
