"""
Tray segments for compression.
"""

import os
from functools import partial

from icecube import icetray,dataio
from icecube.frame_object_diff import modules

def get_frames(tray,base_filename):
    """Use the file stager to get base GCD frames."""
    frames = {}
    if "I3FileStager" in tray.context:
        stager = tray.context["I3FileStager"]
        base_filename_staged = stager.GetReadablePath(base_filename)
    else:
        if not os.path.isfile(base_filename):
            raise Exception('base_filename %s does not exist'%(base_filename))
        base_filename_staged = base_filename
    for fr in dataio.I3File(str(base_filename_staged)):
        if fr.Stop == fr.Geometry:
            frames['Geometry'] = fr
        elif fr.Stop == fr.Calibration:
            frames['Calibration'] = fr
        elif fr.Stop == fr.DetectorStatus:
            frames['DetectorStatus'] = fr
    return frames

@icetray.traysegment
def compress(tray, name,
             base_filename,
             base_path="",
             base_frames={},
             inline_compression=True,
             frame_callback=None,
             If=lambda f:True):
    """
    A general compressor for GCD frames.

    :param base_filename: The base GCD filename.
    :param base_path: (optional) Will be used as a path to base_filename but not stored in the diff objects.
    :param base_frames: (optional) Pre-existing copy of GCD frames in a dictionary.
    :param inline_compression: (optional) Directly modify the frame stream (default True).
    :param frame_callback: (optional) Function callback which receives the compressed frames.
    :param If: (optional) Callable to decide whether to run on the frame.

    If neither inline_compression or frame_callback is enabled, compressed objects will be
    added into the frame stream without deleting anything else.
    """
    if inline_compression and frame_callback:
        raise Exception('cannot use both inline_compression and frame_callback')

    # Get the base frames from the base file.
    # This is done once, at setup.
    if not base_frames:
        base_frames = get_frames(tray,os.path.join(base_path, base_filename))

    if 'Geometry' not in base_frames:
        raise Exception('Could not find Geometry in base_frames')
    if 'Calibration' not in base_frames:
        raise Exception('Could not find Calibration in base_frames')
    if 'DetectorStatus' not in base_frames:
        raise Exception('Could not find DetectorStatus in base_frames')

    tray.AddModule(modules.GeometryCompressor,name+'_Geometry',
                   base_filename = base_filename,
                   base_frame = base_frames['Geometry'],
                   inline_compression = inline_compression,
                   frame_callback = frame_callback,
                   If = If)

    tray.AddModule(modules.CalibrationCompressor,name+'_Calibration',
                   base_filename = base_filename,
                   base_frame = base_frames['Calibration'],
                   inline_compression = inline_compression,
                   frame_callback = frame_callback,
                   If = If)

    tray.AddModule(modules.DetectorStatusCompressor,name+'_DetectorStatus',
                   base_filename = base_filename,
                   base_frame = base_frames['DetectorStatus'],
                   inline_compression = inline_compression,
                   frame_callback = frame_callback,
                   If = If)

@icetray.traysegment
def uncompress(tray, name,
               base_filename=None,
               base_path="",
               base_frames={},
               keep_compressed=False,
               frame_callback=None,
               If=lambda f:True):
    """
    A general uncompressor for GCD frames. The reverse of compress.

    :param base_filename: (optional) The base GCD filename (default to stored filename).
    :param base_path: (optional) Will be used as a path to the input filename if set.
    :param base_frames: (optional) Pre-existing copy of GCD frames in a dictionary.
    :param keep_compressed: (optional) Keep the compressed objects (default False).
    :param frame_callback: (optional) Function callback which receives the uncompressed frames.
    :param If: (optional) Callable to decide whether to run on the frame.

    If frame_callback is enabled, uncompressed objects will not be written
    to the frame stream, instead they will be sent to the callback. This
    also enables keep_compressed, since we don't want to leave the frames
    empty.
    """
    if frame_callback:
        keep_compressed = True

    # Get the base frames from the base file.
    # This is done once, at setup.
    if base_frames:
        def get(stop):
            return base_frames[stop]
    else:
        def get(stop,filename=None):
            if not get.frames:
                get.frames = get_frames(tray,filename)
                if 'Geometry' not in get.frames:
                    raise Exception('Could not find Geometry in base_frames')
                if 'Calibration' not in get.frames:
                    raise Exception('Could not find Calibration in base_frames')
                if 'DetectorStatus' not in get.frames:
                    raise Exception('Could not find DetectorStatus in base_frames')
            if stop and stop in get.frames:
                return get.frames[stop]
        get.frames = {}
        if base_filename:
            get(None,os.path.join(base_path,base_filename))
        else:
            def get_filename(fr):
                if base_path:
                    f = os.path.join(base_path,os.path.basename(fr['I3GeometryDiff'].base_filename))
                else:
                    f = fr['I3GeometryDiff'].base_filename
                get(None,f)
            tray.AddModule(get_filename,Streams=[icetray.I3Frame.Geometry])

    tray.AddModule(modules.GeometryUncompressor,name+'_Geometry',
                   base_frame=partial(get,'Geometry'),
                   keep_compressed=keep_compressed,
                   frame_callback=frame_callback,
                   If=If)

    tray.AddModule(modules.CalibrationUncompressor,name+'_Calibration',
                   base_frame=partial(get,'Calibration'),
                   keep_compressed=keep_compressed,
                   frame_callback=frame_callback,
                   If=If)

    tray.AddModule(modules.DetectorStatusUncompressor,name+'_DetectorStatus',
                   base_frame=partial(get,'DetectorStatus'),
                   keep_compressed=keep_compressed,
                   frame_callback=frame_callback,
                   If=If)


@icetray.traysegment
def inline_compress(tray, name, If=lambda f:True):
    """
    An inline compressor for GCD frames in the same file.

    :param If: (optional) Callable to decide whether to run on the frame.
    """
    gcd_streams = [icetray.I3Frame.Geometry,
                   icetray.I3Frame.Calibration,
                   icetray.I3Frame.DetectorStatus]

    def get_base(frame):
        if str(frame.Stop) not in get_base.frames:
            get_base.frames[str(frame.Stop)] = [0,frame]
        else:
            get_base.frames[str(frame.Stop)][0] += 1
    get_base.frames = {}
    tray.AddModule(get_base, name+'_base', Streams=gcd_streams)

    def my_If(frame):
        if str(frame.Stop) not in get_base.frames:
            return False
        if get_base.frames[str(frame.Stop)][0] == 0:
            return False
        return If(frame)

    def get(stop):
        return get_base.frames[stop][1]

    tray.AddModule(modules.GeometryCompressor,name+'_Geometry',
                   base_filename = '<self>',
                   base_frame = partial(get,'Geometry'),
                   inline_compression = True,
                   If = my_If)

    tray.AddModule(modules.CalibrationCompressor,name+'_Calibration',
                   base_filename = '<self>',
                   base_frame = partial(get,'Calibration'),
                   inline_compression = True,
                   If = my_If)

    tray.AddModule(modules.DetectorStatusCompressor,name+'_DetectorStatus',
                   base_filename = '<self>',
                   base_frame = partial(get,'DetectorStatus'),
                   inline_compression = True,
                   If = my_If)

@icetray.traysegment
def inline_uncompress(tray, name, If=lambda f:True):
    """
    An inline uncompressor for GCD frames. The reverse of inline_compress.

    :param If: (optional) Callable to decide whether to run on the frame.
    """
    gcd_streams = [icetray.I3Frame.Geometry,
                   icetray.I3Frame.Calibration,
                   icetray.I3Frame.DetectorStatus]

    def get_base(frame):
        if str(frame.Stop) not in get_base.frames:
            get_base.frames[str(frame.Stop)] = [0,frame]
        else:
            get_base.frames[str(frame.Stop)][0] += 1
    get_base.frames = {}
    tray.AddModule(get_base, name+'_base', Streams=gcd_streams)

    def my_If(frame):
        if str(frame.Stop) not in get_base.frames:
            return False
        if get_base.frames[str(frame.Stop)][0] == 0:
            return False
        return If(frame)

    def get(stop):
        return get_base.frames[stop][1]

    tray.AddModule(modules.GeometryUncompressor,name+'_Geometry',
                   base_frame=partial(get,'Geometry'),
                   keep_compressed=False,
                   If=my_If)

    tray.AddModule(modules.CalibrationUncompressor,name+'_Calibration',
                   base_frame=partial(get,'Calibration'),
                   keep_compressed=False,
                   If=my_If)

    tray.AddModule(modules.DetectorStatusUncompressor,name+'_DetectorStatus',
                   base_frame=partial(get,'DetectorStatus'),
                   keep_compressed=False,
                   If=my_If)
