Use Cases (Examples)
====================

The initial use-case for full_event_followup is to be able to send full event
information from the South Pole to the North in a message that is as small as
possible, but which is fully self-contained. Thus the full geometry,
calibration and detector status (GCD) information need to be sent. The GCD
information usually uses several megabytes of data, even in compressed form.
Most of this data, however, is constant from run to run and thus from event
to event. By only storing differences between GCD information from a baseline
file to the GCD information for a particular event, the storage necessary space
can often  be reduced to well below a kilobyte (only several hundred bytes in
the best case).

The segments provided with this project can be used to create "Diff" objects
for items in the GCD frames which will be created at SPS when writing the
full-event follow-up message. They will be based on a known "baseline" GCD file
which also exists as a copy in the north. Once the message has been received,
the decompression segment can be used to re-create the original frame objects
and thus restore the full GCD set of frames.

A very similar use-case exists in simulation production and/or L2/L3 processing.
Instead of shipping large GCD frames to cluster worker nodes which might not
necessarily share a common file system, "Diff" objects can be transmitted,
significantly reducing the data transfer to and from worker nodes and thus
allowing jobs to start up much faster without causing network congestion.

In the future, the functionality can be easily extended to reduce the storage
size require for an analysis final event samples where events from different
runs are stored in the same .i3 file. At the moment, GCD frames need to be
repeatedly stored in the file, leading to very large file sizes. By only storing
the first GCD set uncompressed and then storing only differences, the file
sizes can be significantly reduced. This is similar to video compression codecs
where I-frames are stored in full and P-frames are stored as differences to
previous frames.
