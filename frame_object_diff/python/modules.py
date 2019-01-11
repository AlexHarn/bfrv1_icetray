"""
Modules for compressing and uncompressing GCD frames.
"""

from icecube import icetray
from icecube import frame_object_diff

def get_compressor(obj):
    """
    Return the compressor class for an object, if available.
    Raise an exception otherwise.
    """
    return getattr(frame_object_diff,obj.__class__.__name__+'Diff')


class _AbstractCompressor(icetray.I3ConditionalModule):
    """
    Abstract frame object compressor. Do not use this directly.

    :param base_filename: Base filename to compress against
    :param base_frame: (optional) Frame from base filename to compress against
                       Can be an actual frame object, or a callable that returns a frame object.
    :param inline_compression: (optional) Enable/disable inline compression (default enabled)
    :param frame_callback: (optional) Callback to receive compressed frames
    :param If: (optional) Callable to decide whether to run on the frame.

    If ``base_frame`` is not supplied, it is looked up from ``base_filename``.

    Either ``inline_compression`` or ``frame_callback`` can be enabled, not both.
    """
    def __init__(self,context,frame_type):
        super(_AbstractCompressor,self).__init__(context)
        self.frame_type = frame_type
        self.Register(self.frame_type, self.ProcessRegisteredFrameType)
        self.AddParameter('base_filename','Base filename to compress against',None)
        self.AddParameter('base_frame','Frame from base filename to compress against',None)
        self.AddParameter('inline_compression','Enable/disable inline compression (default enabled)',True)
        self.AddParameter('frame_callback','Callback to receive compressed frames',None)
        self.AddOutBox('OutBox')

    def Configure(self):
        self.base_filename = self.GetParameter('base_filename')
        if not self.base_filename:
            raise Exception('base_filename required')
        self.base_frame = self.GetParameter('base_frame')
        if not self.base_frame:
            raise Exception('base_frame required')
        self.inline_compression = self.GetParameter('inline_compression')
        self.frame_callback = self.GetParameter('frame_callback')
        if self.frame_callback and not callable(self.frame_callback):
            raise Exception('frame_callback is not callable')
        if self.inline_compression and self.frame_callback:
            raise Exception('cannot do both inline_compression and frame_callback')
        self.if_ = self.GetParameter('If')
        if self.if_ and not callable(self.if_):
            raise Exception('If is not callable')

    def ProcessRegisteredFrameType(self, frame):
        icetray.logging.log_debug('Process() for %s, received %s'%(
            str(self.frame_type),str(frame.Stop)))
        if frame.Stop == self.frame_type and (not self.if_ or
           self.if_(frame)):
            base_frame = self.base_frame
            if callable(base_frame):
                base_frame = base_frame()
            # attempt to compress everything in the frame
            if self.frame_callback:
                new_frame = icetray.I3Frame(frame.Stop)
            for k in frame.keys():
                if frame.get_stop(k) == self.frame_type:
                    try:
                        comp = get_compressor(frame[k])
                        obj = comp(self.base_filename,base_frame[k],frame[k])
                        if self.inline_compression:
                            del frame[k]
                            frame[k+'Diff'] = obj
                        elif self.frame_callback:
                            new_frame[k+'Diff'] = obj
                        else:
                            frame[k+'Diff'] = obj
                    except Exception as e:
                        icetray.logging.log_info('compression failed: %r'%e)
                        if self.frame_callback:
                            new_frame[k] = frame[k]

            if self.frame_callback:
                self.frame_callback(new_frame)

        self.PushFrame(frame)

class _AbstractUncompressor(icetray.I3ConditionalModule):
    """
    Abstract frame object uncompressor. Do not use this directly.

    :param base_frame: (optional) Frame from base filename to uncompress against.
                       Can be an actual frame object, or a callable that returns a frame object.
    :param keep_compressed: (optional) Enable/disable keeping compressed objects
                            in frame (default disabled)
    :param frame_callback: (optional) Callback to receive uncompressed frames
                           instead of writing them in the frame stream.
    :param If: (optional) Callable to decide whether to run on the frame.

    If ``base_frame`` is not supplied, it is looked up from ``base_filename``.
    """
    def __init__(self,context,frame_type):
        super(_AbstractUncompressor,self).__init__(context)
        self.frame_type = frame_type
        self.Register(self.frame_type, self.ProcessRegisteredFrameType)
        self.AddParameter('base_frame','Frame from base filename to uncompress against',None)
        self.AddParameter('keep_compressed','Enable/disable keeping compressed objects in frame (default disabled)',True)
        self.AddParameter('frame_callback','Callback to receive uncompressed frames',None)
        self.AddOutBox('OutBox')

    def Configure(self):
        self.base_frame = self.GetParameter('base_frame')
        if not self.base_frame:
            raise Exception('base_frame required')
        self.keep_compressed = self.GetParameter('keep_compressed')
        self.frame_callback = self.GetParameter('frame_callback')
        if self.frame_callback and not callable(self.frame_callback):
            raise Exception('frame_callback is not callable')
        self.if_ = self.GetParameter('If')
        if self.if_ and not callable(self.if_):
            raise Exception('If is not callable')

    def ProcessRegisteredFrameType(self, frame):
        icetray.logging.log_debug('Process() for %s, received %s'%(
            str(self.frame_type),str(frame.Stop)))
        if frame.Stop == self.frame_type and (not self.if_ or
           self.if_(frame)):
            base_frame = self.base_frame
            if callable(base_frame):
                base_frame = base_frame()
            # attempt to uncompress everything in the frame
            if self.frame_callback:
                new_frame = icetray.I3Frame(frame.Stop)
            for k in frame.keys():
                if frame.get_stop(k) == self.frame_type:
                    if k.endswith('Diff'):
                        try:
                            new_k = k.replace('Diff','')
                            obj = frame[k].unpack(base_frame[new_k])
                            if self.frame_callback:
                                new_frame[new_k] = obj
                            else:
                                frame[new_k] = obj
                            if not self.keep_compressed:
                                del frame[k]
                        except Exception as e:
                            raise
                            icetray.logging.log_info('uncompression failed: %r'%e)
                            if self.frame_callback:
                                new_frame[k] = frame[k]
                    elif self.frame_callback:
                        new_frame[k] = frame[k]

            if self.frame_callback:
                self.frame_callback(new_frame)

        self.PushFrame(frame)

class GeometryCompressor(_AbstractCompressor):
    __doc__ = '\n'.join(_AbstractCompressor.__doc__.split('\n')[2:])
    def __init__(self,context):
        super(GeometryCompressor,self).__init__(context,icetray.I3Frame.Geometry)

class CalibrationCompressor(_AbstractCompressor):
    __doc__ = '\n'.join(_AbstractCompressor.__doc__.split('\n')[2:])
    def __init__(self,context):
        super(CalibrationCompressor,self).__init__(context,icetray.I3Frame.Calibration)

class DetectorStatusCompressor(_AbstractCompressor):
    __doc__ = '\n'.join(_AbstractCompressor.__doc__.split('\n')[2:])
    def __init__(self,context):
        super(DetectorStatusCompressor,self).__init__(context,icetray.I3Frame.DetectorStatus)

class GeometryUncompressor(_AbstractUncompressor):
    __doc__ = '\n'.join(_AbstractUncompressor.__doc__.split('\n')[2:])
    def __init__(self,context):
        super(GeometryUncompressor,self).__init__(context,icetray.I3Frame.Geometry)

class CalibrationUncompressor(_AbstractUncompressor):
    __doc__ = '\n'.join(_AbstractUncompressor.__doc__.split('\n')[2:])
    def __init__(self,context):
        super(CalibrationUncompressor,self).__init__(context,icetray.I3Frame.Calibration)

class DetectorStatusUncompressor(_AbstractUncompressor):
    __doc__ = '\n'.join(_AbstractUncompressor.__doc__.split('\n')[2:])
    def __init__(self,context):
        super(DetectorStatusUncompressor,self).__init__(context,icetray.I3Frame.DetectorStatus)

