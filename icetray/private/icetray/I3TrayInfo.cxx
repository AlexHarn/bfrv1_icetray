/**
 *  $Id$
 *  
 *  Copyright (C) 2007
 *  Troy D. Straszheim  <troy@icecube.umd.edu>
 *  and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *  
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *  
 */

#include <icetray/I3TrayInfo.h>
#include <icetray/I3Logging.h>
#include <icetray/serialization.h>
#include <boost/foreach.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>

using namespace std;

I3TrayInfo::I3TrayInfo() : svn_revision(0)
{ }


template <class Archive>
void
I3TrayInfo::serialize(Archive & ar, unsigned version)
{
  if (version <= 1)
    throw icecube::archive::archive_exception(icecube::archive::archive_exception::unsupported_version);

  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
  ar & make_nvp("host_info", host_info);
  ar & make_nvp("svn_url", svn_url);
  ar & make_nvp("svn_externals", svn_externals);
  ar & make_nvp("svn_revision", svn_revision);
  ar & make_nvp("modules_in_order", modules_in_order);
  ar & make_nvp("factories_in_order", factories_in_order);
  ar & make_nvp("module_configs", module_configs);
  ar & make_nvp("factory_configs", factory_configs);
}

template <typename K, typename V, typename K2>
const V& at(const map<K,V>& m, const K2& k)
{
  log_trace("Looking for <<<%s>>>", std::string(k).c_str()); // we get either char* or string
  typename map<K,V>::const_iterator ci = m.find(k);
  if (ci == m.end())
    log_fatal("Could not find key %s in map", std::string(k).c_str()); // we get either char* or string

  return ci->second;
}

ostream& 
operator<<(ostream& os, const I3TrayInfo& config)
{
  return(config.Print(os));
}

std::ostream& I3TrayInfo::Print(std::ostream& os) const
{
  os << "============================= configuration ==============================\n";
  os << "      Icetray run started:  " << at(host_info, "start_time") // ctime has a newline already
     << "                  on host:  " << at(host_info, "hostname")
     << " (" << at(host_info, "platform") << ")\n"
     << "                  as user:  " << at(host_info,"username") << "\n"
     << "          and gcc version:  " << at(host_info, "gcc_version")  << "\n"
     << "                  svn url:  " << svn_url << "\n"
     << "             svn revision:  " << svn_revision << "\n\n";

  os << "===========================   svn:externals   ============================\n"
     << svn_externals << "\n";

  os << "=============================   services    ==============================\n";

  for (const auto& factory : factories_in_order)
    os << *at(factory_configs,factory) << "\n";

  os << "=============================    modules    ==============================\n";

  for (const auto& module : modules_in_order)
    os << *at(module_configs,module) << "\n\n";

  os << "==========================================================================\n";

  return os;
}

I3_SERIALIZABLE(I3TrayInfo);
