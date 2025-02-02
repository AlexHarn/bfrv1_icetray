/**
 * Copyright (c) 2011, 2012
 * Claudio Kopper <claudio.kopper@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * $Id$
 *
 * @file I3CLSimFunction.h
 * @version $Revision$
 * @date $Date$
 * @author Claudio Kopper
 */

#ifndef I3CLSIMFUNCTION_H_INCLUDED
#define I3CLSIMFUNCTION_H_INCLUDED

#include "icetray/serialization.h"
#include "icetray/I3TrayHeaders.h"

#include <string>

/**
 * @brief A function value dependent on photon wavelength (or anything else)
 */
static const unsigned i3clsimfunction_version_ = 1;

I3_FORWARD_DECLARATION(I3CLSimFunction);

class I3CLSimFunction : public I3FrameObject
{
public:
    
    I3CLSimFunction();
    virtual ~I3CLSimFunction();

    /**
     * If this is true, it is assumed that GetValue() and GetDerivative() return
     * meaningful values. If not, GetValue will not be called;
     * only the OpenCL implementation will be used.
     */
    virtual bool HasNativeImplementation() const = 0;

    /**
     * If this is true, derivatives can be used.
     */
    virtual bool HasDerivative() const = 0;
    
    /**
     * Shall return the value at a requested wavelength (n)
     */
    virtual double GetValue(double wlen) const = 0;

    /**
     * Shall return the derivative at a requested wavelength (dn/dlambda)
     */
    virtual double GetDerivative(double wlen) const {return NAN;}
    
    /**
     * Shall return the minimal supported wavelength (possibly -inf)
     */
    virtual double GetMinWlen() const = 0;

    /**
     * Shall return the maximal supported wavelength (possibly +inf)
     */
    virtual double GetMaxWlen() const = 0;
    
    /**
     * Shall return an OpenCL-compatible function named
     * functionName with a single float argument (float wlen)
     */
    virtual std::string GetOpenCLFunction(const std::string &functionName) const = 0;

    /**
     * Shall return an OpenCL-compatible function named
     * functionName with a single float argument (float wlen)
     */
    virtual std::string GetOpenCLFunctionDerivative(const std::string &functionName) const {return std::string();}
    
    /**
     * Shall compare to another I3CLSimFunction object
     */
    virtual bool CompareTo(const I3CLSimFunction &other) const = 0;

    /*
     * NB: since shared_ptr<B> is unrelated to shared_ptr<A> even if B inherits
     * from A, we can't return shared_ptr to derived types from overrides.
     * Instead, we emulate the desired behavior by implementing a non-virtual
     * method in each subclass that hides the superclass method and wraps a raw
     * pointer in shared_ptr. This allows the caller to receive either a
     * shared_ptr<I3CLSimFunction> or shared_ptr<Derived>, depending on the
     * type of the pointer Scale() is called on.
     */
    I3CLSimFunctionPtr Scale(double coefficient) const { return I3CLSimFunctionPtr(this->ScaleImpl(coefficient)); };

private:
    /**
     * Shall return a function whose evaluates are scaled by coeffiecient
     */
    virtual I3CLSimFunction* ScaleImpl(double coefficient) const = 0;

    friend class icecube::serialization::access;
    template <class Archive> void serialize(Archive & ar, unsigned version);
};

inline bool operator==(const I3CLSimFunction& a, const I3CLSimFunction& b)
{
    return a.CompareTo(b);
}

inline bool operator!=(const I3CLSimFunction& a, const I3CLSimFunction& b)
{
    return (!a.CompareTo(b));
}

inline I3CLSimFunctionPtr operator*(const I3CLSimFunction& a, double b)
{
    return a.Scale(b);
}

inline I3CLSimFunctionPtr operator*(double b, const I3CLSimFunction& a)
{
    return a.Scale(b);
}


I3_CLASS_VERSION(I3CLSimFunction, i3clsimfunction_version_);

#endif //I3CLSIMFUNCTION_H_INCLUDED
