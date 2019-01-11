/**
 * Definition of I3MediumPropertiesFile class
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id:  $
 *
 * @file I3MediumPropertiesFile.h
 * @version $Revision:  $
 * @date $Date:  $
 * @author tschmidt
 */

#ifndef I3_MEDIUM_PROPERTIES_FILE_H_INCLUDED
#define I3_MEDIUM_PROPERTIES_FILE_H_INCLUDED

// forward declarations


// header files

#include <string>
#include <fstream>

#include <boost/shared_ptr.hpp>

#include <ipdf/I3/I3MediumProperties.h>
#include <icetray/I3Logging.h>

// definitions


/**
 * @brief This class holds ice properties. It uses an ice properties file as input.
 * 
 * The format of an ice properties file is the following:
 * - Comment lines must start with #
 * 
 * Header consists of:
 * - NLAYER <VAR>number of layers specified in the file</VAR>
 * - NWVL <VAR>wavelength bins</VAR>, <VAR>lowest wavlength</VAR> and
 * <VAR>wavelength step</VAR>
 * 
 * After header comes list of layers. Each layer starts with:
 * - LAYER <VAR>lower edge</VAR> and <VAR>upper edge</VAR> where distances are
 * wrt to some arbitrary <VAR>origin</VAR>
 * 
 * In layer section accepted commands are:
 * - ABS <VAR>list of absorption coefficients per wavelength bin</VAR>
 * - SCAT <VAR>list of effective scattering coefficients per wavelength bin</VAR>
 * 
 * Each list has to have correct number of entries.
 * Layer description is considered complete when new LAYER line is encountered.
 * Layers don't have to come or be described in any specific order. 
 */
class I3MediumPropertiesFile : public I3MediumProperties
{
 public:	
  /**
   * @brief Default constructor.
   */
  I3MediumPropertiesFile();
  /**
   * @brief Constructor.
   * 
   * @param propInFilename Path/name of an ice properties file.
   */
  explicit I3MediumPropertiesFile(const std::string& propInFilename);
  /**
   * @brief Destructor.
   */ 
  virtual ~I3MediumPropertiesFile();
  /**
   * @brief Clone ice properties.
   * 
   * @return A clone of this ice properties.
   */
  virtual I3MediumPropertiesPtr Clone() const;
  /**
   * @brief Configure.
   * 
   * @param propInFilename Path/name of an ice properties file.
   */
  void Configure(const std::string& propInFilename);
  /**
   * @brief Returns all ice layers including ice properties.
   * 
   * @return The collection of all ice layers including ice properties.
   * The collection is ordered, e. g. Layers()[0] will hold the lowest layer.
   * Layers are guaranteed to be disjoint and contiguous.
   * 
   * Within each layer: LowerEdge() < UpperEdge is guaranteed.
   * For each property: LowestWavelength <= HighestWavelength is guaranteed.
   */
  virtual const std::vector<Layer>& Layers() const;

private:
  const static char COMMENT_TOKEN;
  const static std::string NLAYER_TOKEN;
  const static std::string NWVL_TOKEN;
  const static std::string LAYER_TOKEN;
  const static std::string ABS_TOKEN;
  const static std::string SCAT_TOKEN;
  const static double FP_CMP_TOLERANCE;


  // private copy constructors and assignment
  I3MediumPropertiesFile(const I3MediumPropertiesFile&);
  I3MediumPropertiesFile& operator=(const I3MediumPropertiesFile&);

  bool Contiguous(const std::vector<Layer>& layers);
  unsigned int GetNextToken(std::istream& f, std::string& token);
  unsigned int GetNLayer(std::vector<Layer>& layers, std::ifstream& propInFile);
  void GetNWL(int& nwl, double& minwl, double& maxwl, std::ifstream& propInFile);
  unsigned int GetLayers(std::vector<Layer>& layers,
                         int nwl, double minwl, double maxwl,
                         std::ifstream& propInFile);


  // instance member data
  std::vector<Layer> layers_;

  	
  // logging
  SET_LOGGER("I3MediumPropertiesFile");
};

typedef boost::shared_ptr<I3MediumPropertiesFile> I3MediumPropertiesFilePtr;


inline
const std::vector<I3MediumPropertiesFile::Layer>&
I3MediumPropertiesFile
::Layers() const
{
  return layers_;
}

#endif // I3_MEDIUM_PROPERTIES_FILE_H_INCLUDED
