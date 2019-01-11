/**
 *Author: Mark Wellons
 */

#ifndef I3HUBERFIT_PHYSICS_H_INCLUDED
#define I3HUBERFIT_PHYSICS_H_INCLUDED

double static const rho = 1.5;
double static const mu = 1;
double static const epsilon = 0.1; 


//The vector MUST be an array of size 3
double array_norm_2(double *vect);

//returns the highest 2-norm error of the z_i's with the arguments as C arrays
double maxError_wArray(double const *para, double z[][3], std::vector<double>
&delta_ts);

/*This function takes in the data is 4 vectors.  The first three are positions
of the DOM hits, and the fourth is the times.  The six track parameters are
returned in the para array (which MUST be of size 6 or larger), and the t_0 of
the track is returned in it's own double.  The function itself returns the
number of iterations required to converge.  */
size_t computeHuberFit_wArray(std::vector<double> const &xs, std::vector<double>
const &ys, std::vector<double> const &zs, std::vector<double> &delta_ts, double
para[], size_t const maxIter, double const distance, double &t_0);


#endif
