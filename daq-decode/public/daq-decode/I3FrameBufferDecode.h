/**
 * copyright  (C) 2004
 * the icecube collaboration
 * $Id$
 *
 * @file I3FrameBufferDecode.h
 * @version $Revision$
 * @date $Date$
 * @author tschmidt
 */
#ifndef I3FRAMEBUFFERDECODE_H_INCLUDED
#define I3FRAMEBUFFERDECODE_H_INCLUDED

// forward declarations

class I3Context;

// header files

#include <string>

#include <icetray/I3Frame.h>
#include <icetray/I3Logging.h>
#include <icetray/I3ConditionalModule.h>
#include <daq-decode/I3DAQEventDecoder.h>

// definitions


/** Decodes event data from I3 DAQ.
 *
 * I3FrameBufferDecode supports two parameter:
 * <ul>
 * <li><VAR>BufferID</VAR> (the ID of the buffer to decode in the frame
 * (default value is "I3DAQData")) and
 * <li><VAR>ExceptionID</VAR> (this boolean indicates any exception
 * that is thrown during decoding (default value is I3DAQDecodeException)).
 * </ul>
 */
class I3FrameBufferDecode : public I3ConditionalModule
{
 public:
  I3FrameBufferDecode(const I3Context& context);
  virtual ~I3FrameBufferDecode();
  void Configure();
  void DAQ(I3FramePtr frame);

 private:
  std::string bufferID_;
  std::string exceptionID_;
  std::string decoderService_;
  I3DAQEventDecoderPtr decoder_;


  // private copy constructors and assignment
  I3FrameBufferDecode(const I3FrameBufferDecode&);
  I3FrameBufferDecode& operator=(const I3FrameBufferDecode&);


  // logging
  SET_LOGGER ("I3FrameBufferDecode");
};

#endif /*I3FRAMEBUFFERDECODE_H_INCLUDED*/
