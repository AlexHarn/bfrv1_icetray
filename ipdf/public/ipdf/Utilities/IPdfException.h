#ifndef IPdfException_H
#define IPdfException_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file IPdfException.h
    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "AssertX.h"

namespace IPDF {

  /**
    @brief Base class for IPDF exceptions

    Exception class hierachy for the IPDF project.  
    The AssertX.h set of compiler macros is used for 
    checking assertions; on failure of an AssertX 
    assertion an exception is thrown, the type of which 
    can be chosen when calling AssertX.
   */
class IPdfException : public AssertX::AssertFailed {
public:
  ASSERT_FAILED_CTOR(IPdfException);
};

class PSIFailure : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(PSIFailure,IPdfException);

  const char* error() const {
    return "\n PSInterface produced an error! ";
  }
};

/// @brief Implementation of IPdfException
class ReceiverTypeNotImplemeted : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(ReceiverTypeNotImplemeted,IPdfException);

  const char* error() const {
    return "\n Receiver/Emiter types are not implemented! ";
  }
};

/// @brief Implementation of IPdfException
class EmitterTypeNotImplemeted : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(EmitterTypeNotImplemeted,IPdfException);

  const char* error() const {
    return "\n Receiver/Emiter types are not implemented! ";
  }
};

/// @brief Implementation of IPdfException
class CombinationReceiverEmitterNotImplemented : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(CombinationReceiverEmitterNotImplemented,IPdfException);

  const char* error() const {
    return "\n Receiver/Emiter types are not implemented! ";
  }
};

/// @brief Implementation of IPdfException
class IceModelTypeNotImplemented : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(IceModelTypeNotImplemented,IPdfException);

  const char* error() const {
    return "\n Ice Parameter is not implemented! ";
  }
};

/// @brief Implementation of IPdfException
class ParameterValueIsNotFinite : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(ParameterValueIsNotFinite,IPdfException);

  const char* error() const {
    return "\n Received Parameter value is not finite! ";
  }
};

/// @brief Implementation of IPdfException
class IllegalInputParameter : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(IllegalInputParameter,IPdfException);

  const char* error() const {
    return "\n Input Parameters have Illegal values!";
  }
};

/// @brief Implementation of IPdfException
class NumericalError : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(NumericalError,IPdfException);

  const char* error() const {
    return "\n Numerical Error!";
  }
};

/// @brief Implementation of IPdfException
class NotImplemented : public IPdfException {
public:
  ASSERT_FAILED_CTOR_BASE(NotImplemented,IPdfException);

  const char* error() const {
    return "\n Requested Feature is not implemented!";
  }
};

} //namespace IPDF

#endif // PdfException_H
