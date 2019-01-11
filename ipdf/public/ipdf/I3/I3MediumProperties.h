/**
 * Definition of I3MediumProperties class
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id:  $
 *
 * @file I3MediumProperties.h
 * @version $Revision:  $
 * @date $Date:  $
 * @author tschmidt
 */

#ifndef I3_MEDIUM_PROPERTIES_H_INCLUDED
#define I3_MEDIUM_PROPERTIES_H_INCLUDED

// forward declarations

class I3MediumProperties;

// header files

#include <functional>
#include <vector>

// definitions

typedef boost::shared_ptr<I3MediumProperties> I3MediumPropertiesPtr;

/**
 * @brief This class provides an interface to the ice properties file, formerly used by the @c I3MediumService, now by @c IceTableInterpolator.
 */
class I3MediumProperties
{
 public:
  /**
   * @brief This class describes an ice layer.
   */
  class Layer
  {
   public:
    /**
     * @brief This class describes an ice property of an ice layer.
     */
    class Property
    {
     private:
      double lowestWavelength_;
      double highestWavelength_;
      std::vector<double> properties_;

     public:
     /**
      * @brief Returns the lowest wavelength, for which the property is known.
      * 
      * @return The lowest wavelength.
      */
     double LowestWavelength() const{ return lowestWavelength_; }
     /**
      * @brief Sets the lowest wavelength, for which the property is known.
      * 
      * @param lowestWavelength The lowest wavelength.
      */
     void SetLowestWavelength(double lowestWavelength)
     {
       lowestWavelength_ = lowestWavelength;
     }
     /**
      * @brief Returns the highest wavelength, for which the property is known.
      * 
      * @return The highest wavelength.
      */
     double HighestWavelength() const{ return highestWavelength_; }
     /**
      * @brief Sets the highest wavelength, for which the property is known.
      * 
      * @param highestWavelength The highest wavelength.
      */
     void SetHighestWavelength(double highestWavelength)
     {
       highestWavelength_ = highestWavelength;
     }
     /**
      * @brief Returns the property.
      * 
      * @return Returns the property per wavelength bin.
      */
     const std::vector<double>& Get() const{ return properties_; }
     /**
      * @brief Returns the property.
      * 
      * @return Returns the property per wavelength bin.
      */
     std::vector<double>& Get(){ return properties_; }
    };

   private:
    double lowerEdge_;
    double upperEdge_;
    Property abs_;
    Property scat_;

   public:
    /**
     * @brief Returns the lower edge of the layer.
     * 
     * @return The lower edge of the layer.
     */
    double LowerEdge() const{ return lowerEdge_; }
    /**
     * @brief Sets the lower edge of the layer.
     * 
     * @param lowerEdge The lower edge of the layer.
     */
    void SetLowerEdge(double lowerEdge){ lowerEdge_ = lowerEdge; }
    /**
     * @brief Returns the upper edge of the layer.
     * 
     * @return The upper edge of the layer.
     */
    double UpperEdge() const{ return upperEdge_; }
    /**
     * @brief Sets the upper edge of the layer.
     * 
     * @param upperEdge The upper edge of the layer.
     */
    void SetUpperEdge(double upperEdge){ upperEdge_ = upperEdge; }
    /**
     * @brief Returns the absorption coefficents.
     * 
     * @return The absorption coefficents per wavelength bin.
     */
    const Property& AbsorptionCoefficents() const{ return abs_; }
    /**
     * @brief Returns the absorption coefficents.
     * 
     * @return The absorption coefficents per wavelength bin.
     */
    Property& AbsorptionCoefficents(){ return abs_; }
    /**
     * @brief Returns the scattering coefficents.
     * 
     * @return The scattering coefficents per wavelength bin.
     */
    const Property& ScatteringCoefficents() const{ return scat_; }
    /**
     * @brief Returns the scattering coefficents.
     * 
     * @return The scattering coefficents per wavelength bin.
     */
    Property& ScatteringCoefficents(){ return scat_; }
  };


  /**
   * @brief Functor for sort algorithm.
   */
  struct LayerLower : public std::binary_function<Layer, Layer, bool>
  {
    bool operator()(const Layer & x, const Layer & y) const
    {
      return x.LowerEdge() < y.LowerEdge();
    }
  };


  /**
   * @brief Default constructor.
   */
  I3MediumProperties(){}
  /**
   * @brief Destructor.
   */
  virtual ~I3MediumProperties(){}
  /**
   * @brief Clone ice properties.
   * 
   * @return A clone of this ice properties.
   */
  virtual I3MediumPropertiesPtr Clone() const = 0;
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
  virtual const std::vector<Layer>& Layers() const = 0;

private:

  // private copy constructors and assignment
  I3MediumProperties(const I3MediumProperties&);
  I3MediumProperties& operator= (const I3MediumProperties&);
};

#endif // I3_MEDIUM_PROPERTIES_H_INCLUDED
