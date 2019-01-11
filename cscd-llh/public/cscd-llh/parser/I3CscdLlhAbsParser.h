/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhAbsParser.h
 * @version $Revision: $
 * @date $Date$
 * @author mggreene
 */


#ifndef I3_CSCD_LLH_ABS_PARSER
#define I3_CSCD_LLH_ABS_PARSER

//#include "icetray/impl/I3ModuleImpl.h"
#include <icetray/I3Logging.h>
#include <icetray/I3Context.h>
#include <icetray/I3Configuration.h>

/**
 * @brief I3CscdLlhAbsParser is a base class for classes that extract parameters from cscd-llh steering files.
 */

class I3CscdLlhFitter;

class I3CscdLlhAbsParser 
{
  public:

    // Constructor
//    I3CscdLlhAbsParser(I3ModuleImplPtr impl) {impl_ = impl;}
    I3CscdLlhAbsParser(I3Configuration &conf) : configuration_(conf){}
    
    // Destructor
    virtual ~I3CscdLlhAbsParser() {}

  public:

    /**
     * Add parameters.
     * 
     */
    virtual void AddParameters() = 0;

    /**
     * DLR Stolen from J. Pretz's IcePick code
     * 
     * @brief Just like the I3Module 'AddParameter' method this is used
     * in an identical way.  
     * @param name is the name that the new parameter should have
     * @param description is a string description of the parameter
     * @param defaultValue is the default value of your parameter
     */
    template <class ParamType>
    void AddParameter(const std::string& name,
        const std::string& description,
        const ParamType& defaultValue)
    {
      GetConfiguration().Add(name, description, defaultValue);
    }

    /**
     * DLR Stolen from J. Pretz's IcePick code
     * 
     * @brief Just like the I3Module 'GetParameter' method it is used
     * in an identical way.
     * @param name the name of the parameter you want to retrieve
     * @param value a reference to the thing you're setting
     */
   template <class ParamType>
     void GetParameter(const std::string& name, ParamType& value)
   {
     // for some reason this has to be split up into two steps
     // or gcc4 gets confused.
     const I3Configuration& config = GetConfiguration();
     value = config.template Get<ParamType>(name);
   }

    /**
     * Set the fitter.
     *
     * @param fitter The fitter.
     */
    void SetFitter(I3CscdLlhFitter* fitter)
    {
      fitter_ = fitter;
    }

    /**
     * Get parameters from the steering file.
     * Parse parameter std::strings if necessary.
     * Tell the fitter what to do with the parameters.
     * 
     * @return true iff successful.
     */
    virtual bool Configure() = 0;

   protected:

     //DLR These two methods are taken from J. Pretz's IcePick code
    
     I3Configuration& GetConfiguration() const
     {
       return configuration_;
     }
     
   protected:
     
    /** The I3Module implementation that handles steering file parameters
     * (among other things.)
     */
//    I3ModuleImplPtr impl_;
    I3Configuration& configuration_;

    I3CscdLlhFitter* fitter_;
};

typedef boost::shared_ptr<I3CscdLlhAbsParser> I3CscdLlhAbsParserPtr;
#endif
