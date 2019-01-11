/**
 *Author: Mark Wellons
 */

#include<cmath>
#include<cstring>
#include<cassert>
#include<vector>
#include<iostream>
#include<algorithm>

#include "HuberFitPhysics.h"

using std::memcpy;
using std::memset;


//The vector MUST be an array of size 3
double array_norm_2(double *vect)
{
	double norm = 0;
	
	for (size_t j = 0; j<3; j++)
	{
		norm += vect[j]*vect[j]; 
	}
	
	return std::sqrt(norm);
}

/*returns the highest 2-norm error of the z_i's with the arguments as regular C
arrays*/
double maxError_wArray(double const *para, double z[][3], std::vector<double>
&delta_ts)
{
	const size_t numHits = delta_ts.size();
	
	double max = 0;
	double errorVect[3];
	
	for(size_t i = 0; i < numHits; i++)
	{
		double t = delta_ts[i];
				
		for (size_t j =0; j<3; j++)
		{
			errorVect[j] = para[j] + t*para[j+3] - z[i][j];
		}
		
		double norm = array_norm_2( errorVect ); 
		if(norm > max)
		{
			max = norm;
		}	
	}
	return max;
}

/*This function takes in the data as 4 vectors.  The first three are positions
of the DOM hits, and the fourth is the times.  The six track parameters are
returned in the para array (which MUST be of size 6 or larger), and the t_0 of
the track is returned in it's own double.  The function itself returns the
number of iterations required to converge. */  
size_t computeHuberFit_wArray(std::vector<double> const &xs, std::vector<double>
const &ys, std::vector<double> const &zs, std::vector<double> &delta_ts, double
para[], size_t const maxIter, double const distance, double &t_0)
{	
	assert((xs.size() == ys.size()) && (ys.size() == zs.size()));
	size_t numHits = xs.size();
	assert(numHits > 0);
	
	//Get earliest time
	t_0 = *std::min_element(delta_ts.begin(), delta_ts.end());
	
	//Calculate all the delta t's.  
	for(size_t i = 0; i< numHits; i++)
	{
		delta_ts[i] -= t_0;
	}
	
	//Initialize z,d, and z_old
	//z and d are of the form[x_0, y_0, z_0] ,[x_1,  y_1, z_1], ...
	double z[numHits][3];  
	double z_old[numHits][3];
	double d[numHits][3];
	for (size_t i = 0; i< numHits; i++) 
	{
		//memset(d[i], 0, numHits);
		d[i][0] = 0;
		d[i][1] = 0;
		d[i][2] = 0;
	}
	//Compute Huber fit
	size_t step_counter = 0;	
	
	
	double tMean = 0;
	double t_squaredMean = 0;
	double sigmaSquared;
	//Get initial values
	for(size_t i = 0; i < numHits; i++)
	{
		z[i][0] = xs[i];
		z[i][1] = ys[i];
		z[i][2] = zs[i];
		tMean += delta_ts[i];
		t_squaredMean += delta_ts[i]*delta_ts[i];
	}
	tMean /= numHits;
	t_squaredMean /= numHits;
	sigmaSquared = t_squaredMean - tMean*tMean;
	
	//Arrays used in the parameter updating calculations
	double s[3];
	double q[3]; 
	
	double delta[3];
	double r_i[3];
	
	do{
		//compute para_{k+1}
		
		memset(s, 0, 3*sizeof(double));
		memset(q, 0, 3*sizeof(double));
		for(size_t i =0; i < numHits; i++)
		{
			for (size_t j =0; j <3; j++) 
			{
				s[j] += z[i][j] + d[i][j];
				q[j] += delta_ts[i]*(z[i][j] + d[i][j]);
			}
		}
		for (size_t j = 0; j <3; j++)//Get means for s and q
		{
			s[j] /= numHits;
			q[j] /= numHits;
		}
		for (size_t j = 0; j < 3; j++) 
		{
		//Recall that para is of the form(r_0, v)
			para[j] = t_squaredMean*s[j]/sigmaSquared - tMean*q[j]/sigmaSquared;
			para[j+3] = q[j]/sigmaSquared - tMean*s[j]/sigmaSquared;
			
		}
				
		//Record old z
		for (size_t i = 0; i< numHits; i++) {
			memcpy(z_old[i], z[i], 3*sizeof(double));
		}
			
		
		//compute z_{k+1}		
		for(size_t i = 0; i < numHits; i++)
		{
			r_i[0] = xs[i];
			r_i[1] = ys[i];
			r_i[2] = zs[i];
			
			const double t = delta_ts[i];
			for (size_t j =0; j< 3; j++)
			{
				delta[j] = rho*(para[j]+t*para[3+j]) +
				(1-rho)*z[i][j] -  d[i][j] - r_i[j];
			
			}
			const double norm = array_norm_2(delta) ;
			if(norm <= distance*(1 + mu)/mu )
			{
				for (size_t j =0; j< 3; j++)
				{
					z[i][j] = r_i[j] + mu/(1 + mu) * delta[j];
				}
				
			}
			else
			{
				for (size_t j =0; j< 3; j++)
				{
					z[i][j] = r_i[j] + (1 - distance/(mu*norm )) * delta[j];
				}
			}
		}
		
		//compute d_{k+1}
		for(size_t i = 0; i < numHits; i++)
		{
			const double t = delta_ts[i];
			for(size_t j = 0; j<3; j++)
			{
				d[i][j] = d[i][j] - (para[j] + para[j+3]*t - z[i][j] ) +
				(1-rho)*(z[i][j] - z_old[i][j]);
			}
		}
		
		//compute k+1
		step_counter++;
		
		if(step_counter >= maxIter)
		{
			break;
		}
		
	}while(maxError_wArray(para, z, delta_ts) > epsilon);
	
	return step_counter;
}


