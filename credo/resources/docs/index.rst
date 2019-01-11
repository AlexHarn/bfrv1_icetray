.. _credo:

Credo
=====

This project is maintained by Joulien Tatar <jtatar@lbl.gov>.
More detailed Credo information can be found in Eike Middell's thesis:
http://www.ifh.de/~middell/_downloads/diplom_final.pdf

.. toctree::
   :maxdepth: 1
   
   release_notes

Introduction
^^^^^^^^^^^^
Credo is a photonics based reconstruction implemented in the gulliver framework.  It is a derivative of photorec-llh specialized to cascades.  In contrast to cscd-llh, it incorporates the ice-model (by using photorec tables), the full timing information (by considering each individual pulse in the llh calculation) and aims at the reconstruction of the direction of the incident neutrino.

For electromagnetic cascades the number of emitted Cherenkov photons scales linearly with the deposited energy.  This allows for a good energy reconstruction.  Hadronic cascades appear similar to electromagnetic ones except they produce 20% fewer photons for the same deposited energy.  Below 10PeV, cascades have characteristic length of several meters.  Compared to the distance between the DOMs, they appear as point-like light sources.  Even though the cherenkov photons originate approximately from one point, they are preferably emitted in the direction of the Cherenkov angle = 41deg.  In principle this anisotropy allows for the reconstruction of the neutrino direction.  Strong light scattering in the ice and large DOMs spacing makes this inherently difficult.  In summary, a cascade is described by 7 parameters; c, the time and location of the interaction (t,x,y,z), the deposited energy E and the zenith and azimuth angle that denote the direction.

Light Propagation
^^^^^^^^^^^^^^^^^

During their propagation photons are scattered and absorbed.  The instrumented volume has depth dependent optical properties.  It is pervaded with dust layers that track historic climatological changes.  Since propagation of light in such a medium cannot be treated analytically, the tabulated results of the Photonics Monte Carlo Package have been used.  They allow to make predictions for the mean expected amplitude and photon arrival distribution.  Scattering in ice can cause delay times up to a few microseconds.


Reconstruction
^^^^^^^^^^^^^^

Credo accounts for inhomogeneities of the ice and tries to reconstruct the direction of the cascade.  It uses all relevant information from the time resolved measurements of the PMT response (waveforms).  The tabulated results of a simulation of light propagation (Photonics) is used to analyse the arrival time and intensity distributions of the recorded photons.  With the Photonics prediction a likelihood description of the measurement is possible.  Given a hypothetical cascade with parameters c and assuming a Poisson process for every non overlapping sample 'i' taken by the ATWD and the FADC in a DOM, one can compare the measured amplitude to the mean expectation by calculating the Binned Poisson Log-Likelihood.  A considerable speed-up in the computation results from the fact that in the sum over samples 'i' only time intervals with values > 0 contribute.  Periods in the waveform with no measured charge can be ignored.  This is implemented in two steps: first the waveform is scanned for pulses, then these pulses are used to calculate the likelihood.  The cascade reconstruction is performed by searching numerically for the minimum of the negative log likelihood (which is a function of the seven cascade parameters).  This minimization is seeded with time, vertex, and direction estimates of first guess algorithms.  These treat the recorded hit pattern as a rigit body for which they calculate the center of gravity and tensor of inertia.  The number of triggered DOMs is a rough estimate for the deposited energy.  The problem can be significantly simplified if the vertex and the time of the interaction are determined by another method and the orientation of the cascade is neglected.  Then the likelihood, which now only depends on the cascade energy, provides a fast energy estimator that benefits from the improved light-propagation model.


Performance
^^^^^^^^^^^

The reconstruction has been tested with a simulated electron neutrino dataset for IC40.  The primary neutrinos have energies in the range from 10^1.7GeV to 10^10GeV and are weighted to an E^-2 spectrum.  In the simulation of the particle showers the same parameterization as in the reconstruction is used.  Lower energetic showers (<PeV) are represented as point-like light sources with an anisotropic emission profile.  At PeV energies, cascades are split up into several showers to simulate the elongation due to the LPM effect.  The event selection requires that an event has to trigger the detector, the algorithm must converge (fulfilled by 79%) and the reconstructed vertex has to be located inside the geometric boundaries of the detector (fullfilled by 38%).  The obtained vertex resolution is about 7m in x and y, and 4m in z.  An energy resolution of :math:`\sigma(log10(E_{reco}/E_{true})) = 0.13` was obtained.  For highly energetic or nearby cascades, the saturation of the PMT limits the recorded charge.  Above 10PeV the reconstructed energy is systematically too low.  As a measure for the angular resolution the median of the cos(phi) distribution was used, where phi is the angle between the true and the reconstructed direction.  This MC study suggests that for the energy interval of 10TeV to 10PeV an angular resolution of 30-35 deg is possible.
