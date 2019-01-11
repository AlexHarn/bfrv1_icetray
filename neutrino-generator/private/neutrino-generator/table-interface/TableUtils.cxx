/**
 *   Copyright  (C) 2006
 *   The IceCube collaboration
 *   $Id: $
 *   @version $Revision: $
 *   @date    $Date:     $
 *   @author Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *   @brief reads cross section table list
 */

#include "neutrino-generator/table-interface/TableUtils.h"
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

namespace nugen {

std::string
guess_table_path(const std::string &tablepath)
{
    // if tablepath is given, just use it.
    if (tablepath != "")
        return tablepath + "/";

    // if cvmfs exist, just use it.
    std::string path = "/cvmfs/icecube.opensciencegrid.org/data/neutrino-generator/cross_section_data/";
    if (fs::exists(path)) {
       return path;
    }

    // check I3_DATA
    if (getenv("I3_DATA") != NULL) {
        path = std::string(getenv("I3_DATA"))
            + std::string("/neutrino-generator/cross_section_data/");
        if (fs::exists(path))
            return path;
    }

    // check I3_BUILD
    if (getenv("I3_BUILD") != NULL) {
        path = std::string(getenv("I3_BUILD"))
            + std::string("/neutrino-generator/cross_section_data/");
        if (fs::exists(path))
            return path;
    }
    
    path = ".";
    return path;
}

void TableUtils::ReadTableList(const string &tablepath,
                       const string &modelname, 
                       std::map<string, string> &xsecs)

{
   xsecs.clear();

   string path = guess_table_path(tablepath);

   string modelfile = modelname;
   string suffix = ".list";
   
   if (modelfile.find(suffix) == string::npos) modelfile += suffix;

   //
   // check file
   //
   string fname1 = path + modelfile;
   log_debug("open file %s", fname1.c_str());
   std::ifstream in(fname1.c_str());

   // if the cross section list file doesn't exist, stop simulation
   if (in.fail()) {
      log_fatal("failed to open %s. Set correct CrossSectionTablePath.",
                 fname1.c_str());
   }

   //
   // read the file
   //

   const int bufsize = 8196;
   char buf[bufsize];
   string label, file;
   int nread = 0;

   while(!in.eof()) {

      in.getline(buf, bufsize);
      nread = in.gcount();

      if (nread == -1) {
         log_error("getline failed");

      } else if (nread == 1 || buf[0] == ' ' || buf[0] == '#') {
         // new line, start from white space, or comment line.
         continue;

      } else {
         // density data
         stringstream ss(buf);
         ss >> label >> file;

         // the "file" could be relative path or full path.
         // try relative path first.
         string filepathcandidate = path + file;
         std::ifstream in2(filepathcandidate.c_str());
         if (in2.fail()) {
             // path + file doesn't exist. file could be a full path.
             filepathcandidate = file;
             std::ifstream in3(filepathcandidate.c_str());
             if (in3.fail()) {
                 log_error("tried to open cross section file %s or %s, but not found.", (path+file).c_str(), file.c_str());
             }
         }

         xsecs[label] = filepathcandidate;
         log_trace("label %s, filepath %s", 
                   label.c_str(), xsecs[label].c_str());

      }
   } // end of the while loop
   in.close();
} 

}
