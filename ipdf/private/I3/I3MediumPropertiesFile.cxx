/**
 * Implementation of I3MediumPropertiesFile class
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id:  $
 *
 * @file I3MediumPropertiesFile.cxx
 * @version $Revision:  $
 * @date $Date:  $
 * @author tschmidt
 */

// class header file

#include <ipdf/I3/I3MediumPropertiesFile.h>

// other header files

#include <algorithm>
#include <fstream>

#include <boost/version.hpp>
#if BOOST_VERSION >= 105900
#include <boost/test/tools/floating_point_comparison.hpp>
#else
#include <boost/test/floating_point_comparison.hpp>
#endif

// namespace declarations

using namespace std;

// implementation

const char I3MediumPropertiesFile::COMMENT_TOKEN = '#';
const string I3MediumPropertiesFile::NLAYER_TOKEN = "NLAYER";
const string I3MediumPropertiesFile::NWVL_TOKEN = "NWVL";
const string I3MediumPropertiesFile::LAYER_TOKEN = "LAYER";
const string I3MediumPropertiesFile::ABS_TOKEN = "ABS";
const string I3MediumPropertiesFile::SCAT_TOKEN = "SCAT";
const double I3MediumPropertiesFile::FP_CMP_TOLERANCE = 1e-6;


I3MediumPropertiesFile::I3MediumPropertiesFile(){
	layers_.clear();
}


I3MediumPropertiesFile::I3MediumPropertiesFile(const string& propInFilename){
	Configure(propInFilename);
}


I3MediumPropertiesFile::~I3MediumPropertiesFile(){
}


I3MediumPropertiesPtr
I3MediumPropertiesFile::Clone() const{
	I3MediumPropertiesFilePtr clone =
		I3MediumPropertiesFilePtr(new I3MediumPropertiesFile);
	clone->layers_ = layers_;

	return clone;
}


void
I3MediumPropertiesFile::Configure(const string& propInFilename){
	if(propInFilename.empty()) log_fatal("undefined path/file name");

  log_debug("using ice properties file: %s", propInFilename.c_str());
  ifstream propInFile(propInFilename.c_str());
  if (propInFile.fail())
    log_fatal("cannot open file: %s", propInFilename.c_str());

	//////////////////////////////////////////////////////////////

  vector<Layer> layers;
  double minwl, maxwl;
  int nwl;

  // reading header NLAYER
  if(GetNLayer(layers, propInFile)){
		// reading header NWL
    GetNWL(nwl, minwl, maxwl, propInFile);
		// reading layers
		if(GetLayers(layers, nwl, minwl, maxwl, propInFile) != layers.size())
      log_fatal("to less layers defined");

		sort(layers.begin(), layers.end(), LayerLower());
		if(!Contiguous(layers)) log_fatal("discontiguous layers");
	} // if(GetNLayer(layers, propInFile))

	//////////////////////////////////////////////////////////////

  propInFile.close();

	layers_ = layers;
}


bool
I3MediumPropertiesFile::Contiguous(const std::vector<Layer>& layers){
	for(unsigned int i = 1; i < layers.size(); ++i)
    // layers are continous,
    // unless upper and lower edge differ more than 1 micrometer
#if BOOST_VERSION >= 105900
    if(!boost::math::fpc::is_small(layers[i - 1].UpperEdge() - layers[i].LowerEdge(),
                       FP_CMP_TOLERANCE))
#else
    if(!boost::test_tools::check_is_small(layers[i - 1].UpperEdge() - layers[i].LowerEdge(),
                       FP_CMP_TOLERANCE))
#endif
      return false;
	return true;
}


unsigned int
I3MediumPropertiesFile::GetNextToken(istream& f, string& token){
  token.clear();
  while(token.empty() && f.good() && !f.eof()){
    f >> token;
    if(!token.empty() && (token[0] == COMMENT_TOKEN)){
    	getline(f, token);
    	token.clear();
    } // if(!token.empty() && (token[0] == COMMENT_TOKEN))
  } // while(f.good() && !f.eof() && token.empty())

  return token.size();
}


unsigned int
I3MediumPropertiesFile::GetNLayer(vector<Layer>& layers,
                                  ifstream& propInFile){
  int n;
  string token;

  if(GetNextToken(propInFile, token) && (token == NLAYER_TOKEN)){
    propInFile >> n;
    if(propInFile.fail() || (n < 0))
      log_fatal("undefined %s", NLAYER_TOKEN.c_str());
    if(n) layers.resize(n);
  } // if(GetNextToken(propInFile, token) && ...
  else log_fatal("cannot read ice properties file, token %s",
                 NLAYER_TOKEN.c_str());

  return layers.size();
}


void
I3MediumPropertiesFile::GetNWL(int& nwl, double& minwl, double& maxwl,
                               ifstream& propInFile){
  double stepwl;
  string token;

  if(GetNextToken(propInFile, token) && (token == NWVL_TOKEN)){
    propInFile >> nwl >> minwl >> stepwl;
    if(propInFile.fail() || (nwl < 0) || (minwl <= 0) || (stepwl < 0)
       || (!stepwl && nwl))
      log_fatal("undefined %s", NWVL_TOKEN.c_str());
    maxwl = minwl + stepwl * nwl;
  } // if(token == NWVL_TOKEN)
  else log_fatal("cannot read ice properties file, token %s",
                 NWVL_TOKEN.c_str());
}

unsigned int
I3MediumPropertiesFile::GetLayers(vector<Layer>& layers,
                                  int nwl, double minwl, double maxwl,
                                  ifstream& propInFile){
  unsigned int currentLayer = 0;
  string token;

  while(GetNextToken(propInFile, token)){
    if(token == LAYER_TOKEN){
      if(++currentLayer > layers.size())
        log_fatal("to much layers defined");
      double le, ue;
      propInFile >> le >> ue;
      if(propInFile.fail() || (le >= ue))
        log_fatal("undefined %s", LAYER_TOKEN.c_str());
      layers[currentLayer - 1].SetLowerEdge(le);
      layers[currentLayer - 1].SetUpperEdge(ue);
      layers[currentLayer - 1].AbsorptionCoefficents().SetLowestWavelength(minwl);
      layers[currentLayer - 1].AbsorptionCoefficents().SetHighestWavelength(maxwl);
      layers[currentLayer - 1].ScatteringCoefficents().SetLowestWavelength(minwl);
      layers[currentLayer - 1].ScatteringCoefficents().SetHighestWavelength(maxwl);
    } // if(token == LAYER_TOKEN)
    else if(token == ABS_TOKEN){
      if(!currentLayer)
        log_fatal("cannot read ice properties file, token %s",
                  LAYER_TOKEN.c_str());
      if(layers[currentLayer - 1].AbsorptionCoefficents().Get().size())
        log_fatal("duplicated %s", ABS_TOKEN.c_str());
      for(int i = 1; i <= nwl; ++i){
        double val;
        propInFile >> val;
        if(propInFile.fail())
          log_fatal("undefined %s", ABS_TOKEN.c_str());
        layers[currentLayer - 1].AbsorptionCoefficents().Get().push_back(val);
      } // for(int i = 1; i <= nwl; ++i)
    } // else if(token == ABS_TOKEN)
    else if(token == SCAT_TOKEN){
      if(!currentLayer)
        log_fatal("cannot read ice properties file, token %s",
                  LAYER_TOKEN.c_str());
      if(layers[currentLayer - 1].ScatteringCoefficents().Get().size())
        log_fatal("duplicated %s", SCAT_TOKEN.c_str());
      for(int i = 1; i <= nwl; ++i){
        double val;
        propInFile >> val;
        if(propInFile.fail())
          log_fatal("undefined %s", SCAT_TOKEN.c_str());
        layers[currentLayer - 1].ScatteringCoefficents().Get().push_back(val);
      } // for(int i = 1; i <= nwl; ++i)
    } // else if(token == SCAT_TOKEN)
  } // while(GetNextToken(propInFile, token))

  return currentLayer;
}
