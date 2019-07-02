.. _cramer-rao:

==========
Cramer-Rao
==========

.. toctree::
   :maxdepth: 1
   
   release_notes

Maintainer: Chujie Chen


Introduction
============
CramerRao is a module to calculate an estimator for the angular resolution of a reconstructed track. 
As it is an analytical algorithm that does not need a minimization process, it is much faster (factor ~200) than Paraboloid.   

Cramer-Rao relation
===================
The Cramer-Rao relation states that the inverted Fisher information matrix :math:`I^{-1}` provides a lower bound on the variance of the estimator of a parameter.

.. math::
   cov(\theta_m,\theta_k)\geq I(\vec{\theta})^{-1}

The Fisher information matrix is defined as

.. math::
   I_{mk}(\vec{\theta})= -\langle\left[\sum_{i=1}^{modules} \frac{\partial^2}{\partial\theta_k\partial\theta_m} \ln p(t_i;\vec\theta) \right]\rangle

where :math:`\vec{\theta}` represents the track parameters, i.e direction and vertex of the track. :math:`p` is the propability density function. In our case :math:`p(t_i;\vec\theta)` is the Pandel function.

Technical details of the implementation
=======================================

If calculating the covariance matrix for a track in IceCube, it is important to take into account, that only four parameters are independent, as explained in the following: a track can be described by 

.. math::
   \vec{r}=\vec{r}_0+\vec{v}\cdot t

with 

.. math::
   \vec{v}=(\cos\Phi\sin\Theta,\sin\Phi\sin\Theta,\cos\Theta)

Due to the averaging, the CR-inequation doesn't include any time information. That means that the vertex is not uniquely defined: it can be placed anywhere on the track. This leads to the fact that the information matrix is over-determined, thus not invertible. Therefore the number of parameters must be reduced by giving a unique definition of the vertex. We fix the the vertex position at z=0, i.e. the intersection of the track with the x-y plane:

.. math::
   \vec{r^\prime}_0=\vec{r}_0+a\cdot\vec{v}=\vec{r}_0-\frac{z\cdot\vec{v}}{\cos\Theta}

The coordiantes of the moved vertex are:

.. math::
   x^\prime=x-z\cdot\cos\Phi\tan\Theta\\
   y^\prime=y-z\cdot\sin\Phi\tan\Theta\\
   z^\prime=0

With this definition, only four independent parameters remain: the intersection x', y', :math:`Phi` and :math:`Theta`.
The distance between the track and a DOM (r_DOM = x_DOM,y_DOM,z_DOM) can be calculated with: 

.. math::
  d^2=(\vec{v}\times (\vec{r}_{DOM}-\vec{r^\prime}_0))^2\\
  =[(x_{DOM}-x^\prime)\sin\Phi\sin\Theta-(y_{DOM}-y^\prime)\cos\Phi\sin\Theta]^2\\
  +[(y_{DOM}-y^\prime)\cos\Theta-(z_{DOM}-z^\prime)\sin\Phi\sin\Theta]^2\\
  +(z_{DOM}-z^\prime)\cos\Phi\sin\Theta-(x_{DOM}-x^\prime)\cos\Theta]^2 \\
  =[(x_{DOM}-x+z\cdot\cos\Phi\tan\Theta)\sin\Phi\sin\Theta
  -(y_{DOM}-y+z\cdot\sin\Phi\tan\Theta)\cos\Phi\sin\Theta]^2\\
  +\left[(y_{DOM}-y+z\cdot \sin\Phi\tan\Theta)\cos\Theta - z_{DOM}\cdot\sin\Phi\sin\Theta\right]^2\\
  +\left[z_{DOM}\cdot\cos\Phi\sin\Theta-(x_{DOM}-x+z\cdot\cos\Phi\tan\Theta)\cos\Theta\right]^2

The disadvantage of this choice of coordiates is that it cannot describe tracks that are parallel to the x-y plane. These tracks cannot be processed by the cramer-rao module. Fortunately it is very rare, that a track has a zenith of exactly 90 degree. 

The searched parameters :math:`\vec{\theta}` are connected to the propability density :math:`p(t_i,d(\theta))`. With the chain rule

.. math::
  \frac{\partial}{\partial\theta_m}
  =\frac{\partial}{\partial d}\frac{\partial d}{\partial\theta_m}
  =\frac{\partial}{\partial d} \frac{\partial d}{\partial d^2} \frac{\partial d^2}{\partial\theta_m}
  =\frac{1}{2d} \frac{\partial}{\partial d} \frac{\partial d^2}{\partial\theta_m}`

the Fisher information matrix can be written as

.. math::
   I_{mk}(\vec{\theta})=\sum_{i=1}^{NCh}\underbrace{-\left\langle \left(\frac{\partial^2}{(\partial d_i)^2}\ln p(t_i;d(\vec{\theta}))\right)\right\rangle}_{T(d_i)} \underbrace{\frac{1}{4d_i^2}\frac{\partial d_i^2}{\partial\theta_m}\frac{\partial d_i^2}{\partial\theta_k}}_{D_{mk}(\vec{\theta})}

(more information will follow)


References
==========
#. An approach to estimate the optimal resolution was introduced by Marek Kowalski: “Applying information theory to cascade reconstruction”, talk presented at IceCube Collaboration meeting, 6.10.2006, Zeuthen.
 
#. The application on muon tracks by was introduced by Lutz Köepke, Jan Lünemann, Heinz-Georg Sander: "Muon track reconstruction resolution (note in progress)", circulated in the muon group in March 2008.

#. This documentation is mainly translated from chapter 4.4 of Jan Lünemanns thesis: "Suche nach Dunkler Materie in Galaxien und Galaxienhaufen mit dem Neutrinoteleskop IceCube" (2013).
