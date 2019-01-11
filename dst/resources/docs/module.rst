I3DSTModule
============

Collects the DST information from reconstructed I3Particles as well as
DOMLaunchSeries and HitSeries/PulseSeries and I3TriggerHierarchy and with this
adds an I3DST object into the I3Frame. On configuration, I3DSTModule is given
a vector of strings corresponding to the names of I3Particles for various
reconstructions in descending order of priority. The first two reconstructions
that the module finds in the frame will be assigned to reco1 and reco2 in the
dst and the index of the reco name for each will be encoded in to a single
uint8_t byte 'reco_label' using the nth bit as a boolean to indicate whether
the nth reconstruction was selected. This is accomplished by bit-shifting 1 by
n where n is the index of the reconstruction key.


