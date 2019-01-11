#ifndef ASSERTX_ASSERTX_H
#define ASSERTX_ASSERTX_H

#include <exception>
#include <iostream>
#include <sstream>

/**
 * @file AssertX.h
 * @author Simon Robbins
 * @date $Date$
 *
 * @brief Assert that throws an exception (with line number, etc)
 *
 * Usage: (basic version)
 * 
 * AssertExp( _assertion_ );   // throws AssertFailed exception
 *
 * where _assertion_ evaluates to bool,
 * e.g.   Assert(ptr!=NULL)
 *
 *
 * usage: (extended version)
 *
 * AssertX( _assertion_, _ExceptionClass_ );
 *    throws _ExceptionClass_ exception (which derives from AssertFailed)
 *
 * where _ExceptionClass_ is a user defined class deriving 
 * from AssertFailed (see below).  This class must define 
 * a constructor the same as AssertFailed and pass the 
 * arguments to its base (AssertFailed).
 * _ExceptionClass_ can override the method "error()" to 
 * return a message to be printed at the end of the 
 * exception, in much the same way as "what()" for 
 * std::exception.
 * "what()" itself should not be overriden since this is 
 * used by AssertFailed to print the file and line info.
 * "m_assertion" is presently unused, but is available for 
 * use by derived classes.
 *
 * The _ExceptionClass_ exception can then be caught by:
 * catch(std::exception&)
 * catch(AssertFailed&)
 * catch(_ExceptionClass_&)
 * 
 *
 * These assertions can be disabled at compile-time
 * by defining ASSERTX_DISABLED
 *
 */

namespace AssertX {

/**
 * @brief Base class for all classes thrown by AssertX
 */
class AssertFailed : public std::exception {
public:
    AssertFailed(const char* assertion, const char* file,
           unsigned int line, const char* function);

    virtual ~AssertFailed() throw()                         { }

    /// @brief Template Method Pattern (override this)
    virtual const char* error() const            { return ""; }
    /// @brief Don't override this, calls this->error()
    const char* what() const throw();

protected:
    /// @brief Copy only available to derived classes
    AssertFailed(const AssertFailed& rhs);
    /// @brief Copy only available to derived classes
    AssertFailed& operator=(const AssertFailed& rhs);
    const char*          m_assertion;
private:
    mutable std::string  m_what;
    const char*          m_file;
    unsigned int         m_line;
    const char*          m_function;
};

inline AssertFailed::AssertFailed(const char* assertion, 
       const char* file, unsigned int line, const char* function)
: std::exception(), m_assertion(assertion), m_what(""),
  m_file(file), m_line(line), m_function(function)
{
  try {
    m_what += " *** ERROR *** AssertFailed in file ";
    m_what += m_file;
    m_what += ", line ";
    std::ostringstream str_out;
    str_out<<m_line;
    m_what += str_out.str();
    m_what += ",\n in function: ";
    m_what += m_function;
    m_what += "\n assertion (";
    m_what += m_assertion;
    m_what += ") failed.\n\t";
  } catch(...) {}   // don't allow exceptions to propogate
}

inline AssertFailed::AssertFailed(const AssertFailed& rhs)
: std::exception(rhs), m_assertion(rhs.m_assertion), m_what(rhs.m_what),
  m_file(rhs.m_file), m_line(rhs.m_line), m_function(rhs.m_function)
{
}

inline AssertFailed& AssertFailed::operator=(const AssertFailed& rhs)
{
  m_assertion=rhs.m_assertion;
  m_what=rhs.m_what;
  m_file=rhs.m_file;
  m_line=rhs.m_line;
  m_function=rhs.m_function;
  return *this;
}

inline const char* AssertFailed::what() const throw() {
  try {
    m_what += this->error();
  } catch(...) {}   // don't allow exceptions to propogate
  return m_what.c_str();
}

template<typename X>
inline void funcAssertFailed(const char* assertion, 
  const char* file, unsigned int line, const char* function) {
  throw X(assertion,file,line,function);
}

} // namespace AssertX
#endif // ASSERTX_ASSERTX_H

// Adapted from standard assert.h:
//=================================


//   If ASSERTX_DISABLED is defined, do nothing.
//   If not, and EXPRESSION is zero, throw an exception.

# define ASSERT_FAILED_CTOR(X) \
  X(const char* assertion, const char* file, \
                unsigned int line, const char* function) \
   : AssertX::AssertFailed(assertion,file,line,function)            { }

# define ASSERT_FAILED_CTOR_BASE(X,B) \
  X(const char* assertion, const char* file, \
                unsigned int line, const char* function) \
   : B(assertion,file,line,function)            { }

// Actual macros that do the work

#ifdef ASSERTX_DISABLED

# define AssertExp(expr)		((void) (0))
# define AssertX(expr,X)		((void) (0))

#else // ASSERTX_DISABLED
   
#define __ASSERTX_FUNCTION __PRETTY_FUNCTION__

/// Throw an "AssertFailed" if the assert fails
# define AssertExp(expr) \
  ((void) ((expr) ? 0 : 			   \
    (throw AssertFailed (__STRING(expr), __FILE__,  \
       __LINE__, __ASSERTX_FUNCTION), 0)))

/// Throw exception "X" if the assert fails
# define AssertX(expr,X) \
  ((void) ((expr) ? 0 : 			   \
    (AssertX::funcAssertFailed<X>(__STRING(expr), __FILE__,  \
       __LINE__, __ASSERTX_FUNCTION), 0)))

#endif // else ASSERTX_DISABLED
