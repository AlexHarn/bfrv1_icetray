
Algorithm
---------

The NoiseEngine algorithm is modeled after the TrackEngine algorithm (from which the name is derived) used to identify muons. The concept is similar to a Hough transform from the three-dimension (x,y,z) space to a two dimension (azimuth, zenith) space mapped onto a binned unit sphere. 

The process begins by grabbing the specified hits from the frame. The hits are expected to be cleaned, although optimizations of the cleaning level and threshold can be performed. 

The hits are reduced using a sliding time window to identify the window during with the largest number of hits occurs. At this point, there is no attempt to optimize based on the identification of "physics" vs "noise" except the assumption that the observation of more hits implies that a particle interaction is more likely.

Hits accepted based on the time window are then connected to all other hits using the HitPair class included in NoiseEngine. Hits are connected from one DOM to another and are not allowed between pulses on the same DOM. Each pair of hits is assigned a weight. This is typically 1, although enabling the ChargeWeight option will instead assign a weight equal to the average charge of the two hits. In addition, the pair of hits is assigned an azimuth and zenith angle calculated as a unit vector pointing from the earlier hit DOM to the later hit DOM.

At this point, all possible combinations of pairs of hits are binned following a HEALPix gridding scheme with the number of bins determined by the HealpixOrder configurable option.

  .. math::
     N_{bins} = 12 \times 4^{Order}

This is generally set intentionally low: the NoiseEngine module attempts to be accepting of low-energy physics interactions while rejecting noise triggers. Fewer bins increases the statistics of each bin and makes a physics interaction clearer above the noise background.

Once all hit pairs are binned, the resulting binned unit sphere is checked for any bin above the threshold value. If any single bin is at or above the threshold, the event "passes" NoiseEngine and is likely to be caused by a particle interaction. If all bins are below the threshold, the even fails and is likely to be caused solely by detector noise.
