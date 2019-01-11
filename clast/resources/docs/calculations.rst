.. _calculations:

Calculations
============

CLast calculations are called to return an ``I3Particle`` as an early cascade fit, and to return ``I3CLastParams`` which details features of the Tensor of Inertia which are not encapsulated by the ``I3Particle``.

The position of the I3Particle and the point about which the Tensor of Inertia is calculated is the Center of Gravity (CoG), called as ``I3Cuts::COG``. The center of gravity is defined as

:math:`\vec{x} = \sum_i {a_i}^p \vec{r}_i`

where :math:`\vec{r}_i` is the position vector to a given DOM, and :math:`a_i` is the amplitude of the DOM (the total charge) and :math:`p` is the amplitude weight power which here is :math:`p = 1.0`.

The following calculations determine the direction via the Tensor of Inertia and also add how spherical the event is:

* Tensor of Intertia (ToI) calculation:
    ``I3CLastCalculator::CalculateTOI`` takes the following variables: pulse series, IceCube geometry, and center of gravity (CoG). It then returns an I3Matrix containing the ToI.

    The formula for ToI is

    :math:`I^{ij} = \{\sum_k^N m_k (\delta^{ij}{R_k}^2 - {R_{k}}^{i} {R_{k}}^{j})\}/\sum_k^N m_k`

    Where :math:`R_{ik}` is the vector from the CoG to the kth mass, in our case the position of the kth DOM; and :math:`{R_k}^2` is the square of it's length; :math:`m_k` is the kth mass, in our case the total charge observed on the DOM. The calculation is for the :math:`I^{ij}` component of the Tensor of Inertia which in our case is a 3x3 matrix.

    There are some variables which can modify the calculation: ``ampOpt_`` and ``ampWeight_`` default to 1.0, which results in predicable behaviour. If ``ampOpt_=0.`` then pulse charges between 0 and 2 are cast to 1.0. If ``ampWeight_`` is set, each pulse is scaled to ``pow(charge,ampWeight_)`` (for example, ``ampWeight_=0.`` would cast all pulses regardless of  charge to 1.0).

* Diagonalising the ToI:
    ``I3CLastCalculator::diagonaliseToI`` is used as a means to access the eigenvalues of the tensor of intertia, and returns the eigenvector as a ``std::vector`` corresponding to the minimum eigenvalue. It uses the gsl library of matrix operations to perform this task.
    
    Note that the smallest eigenvalue will correspond to the long axis of the 'object' as this is the axis about which the 'object' could most freely rotate (i.e. has the least inertia).

    This calculation both dermines the eigenvector corresponding to the long axis, and also adds `I3CLastParams` to the frame which includes all 3 eigenvalues as well as the ratio ``evalratio`` :math:`e_1 / (e_1 + e_2 + e_3)` which describes how spherical the event is.

* Correct Direction
    ``I3CLastCalculator::CorrectDirection`` is used to remove the degeneracy (correct the calculation) of the two possible directions for the result eigenvector (forward or backward) by pointing the vector towards the direction where the OM hit time is latest from the CoG. To do this the covariance of the times projected along the vector :math:`l_i` for DOM i is calculated weighted by the square of the charge. A negative covariance results in reversing the vector.

    The covariance calculation is done in two parts: a divisor :math:`C` which is purely geometric:
    
    :math:`C = \sum_i {q_i}^2 \sum_i (l_i^2 {q_i}^2) - (\sum_i l_i {q_i}^2)^2`
   
    and the result which is dependent on time :math:`t_i`:

    :math:`\sigma = [ \sum_i {q_i}^2 \sum_i (t_i l_i {q_i}^2) - \sum_i (l_i {q_i}^2) \sum_i(t_i {q_i}^2) ] / C`

    If :math:`\sigma < 0` then the eigenvector is reversed in order to point in the direction of later times.

In addition, to complete the seed requires a calculation of the time and energy of the event:

* Time Calculation
    ``I3CLastCalculator::CalculateTimes`` is used to determine the time of the reconstruction particle where we currently have the CoG position and ToI eigenvector for direction.

    Basics of calculation: first we look at only DOMs within a radius ``rMax_``, with a default of 300.0m get a "vertex time". The vertex time is the time of an event at the CoG assuming the first hit observed on the given DOM was a direct photon from the CoG; i.e.

    :math:`t_{vertex} = t_{hit} - r/c_{ice}`

    where :math:`r` is the distance from the DOM to the CoG.

    Then, iterate over all other DOMs to check how many DOMs have a later vertex time than your current DOM (``nDirect``). To be a direct hit, the conditions are that the new hit trial vertex time :math:`t_{proj} = t_{hit2} - r_2 /c_{ice}` must satisfy:
    
    :math:`t_{proj} > t_{vertex}`

    and 

    :math:`t_{proj} < t_{vertex} + t_{window}`.

    Here, :math:`t_{window}` is the setting ``directHitWindow_`` with a default of 200.0ns.

    To cover all bases, three times are stored:
    ``earliestThresholdTime`` for the earliest overall vertex time with :math:`n_{direct}` times above the predefined threshold ``directHitThreshold_`` with a default of 3 hits. (if satisfied, this will take precedence)

    ``timeOfMaxNDirect`` for the vertex time with the most :math:`n_{direct}` times overall.

    ``earliestVertexTime`` as a fallback for the earliest vertex time regardless of threshold.

* Energy Calculation
    ``I3CLastCalculator::CalculateEnergy_From_I3Hits`` and ``I3CLastCalculator::CalculateEnergy_From_AMHits`` are functions that covert the total charge observed to a corresponding energy given their own parameterisations.
