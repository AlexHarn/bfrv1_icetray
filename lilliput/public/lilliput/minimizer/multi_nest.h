/**
 *-------------------------------------------------------------------------------
 *  copyright (C) 2014
 *  Farhan Feroz & Mike Hobson
 *
 * Permission to include in application software or to make digital or
 * hard copies of part or all of this work is subject to the following
 * licensing agreement.
 *
 * MultiNest License Agreement
 *
 * MultiNest (both binary and source) is copyrighted by Farhan Feroz and
 * Mike Hobson (hereafter, FF&MH) and ownership of all right, title and
 * interest in and to the Software remains with FF&MH. By using or
 * copying the Software, User agrees to abide by the terms of this
 * Agreement.
 *
 * Non-commercial Use
 *
 * FF&MH grant to you (hereafter, User) a royalty-free, non-exclusive
 * right to execute, copy, modify and distribute both the binary and
 * source code solely for academic, research and other similar
 * non-commercial uses, subject to the following
 *
 * 1. User acknowledges that the Software is still in the development
 * stage and that it is being supplied "as is," without any support
 * services from FF&MH. FF&MH do not make any representations or
 * warranties, express or implied, including, without limitation, any
 * representations or warranties of the merchantability or fitness for
 * any particular purpose, or that the application of the software, will
 * not infringe on any patents or other proprietary rights of others.
 *
 * 2. FF&MH shall not be held liable for direct, indirect, incidental or
 * consequential damages arising from any claim by User or any third
 * party with respect to uses allowed under this Agreement, or from any
 * use of the Software.
 *
 * 3. User agrees to fully indemnify and hold harmless FF&MH of the
 * original work from and against any and all claims, demands, suits,
 * losses, damages, costs and expenses arising out of the User's use of
 * the Software, including, without limitation, arising out of the User's
 * modification of the Software.
 *
 * 4. User may modify the Software and distribute that modified work to
 * third parties provided that: (a) if posted separately, it clearly
 * acknowledges that it contains material copyrighted by FF&MH (b) no
 * charge is associated with such copies, (c) User agrees to notify FF&MH
 * of the distribution, and (d) User clearly notifies secondary users
 * that such modified work is not the original Software.
 *
 * 5. User agrees that the authors of the original work and others may
 * enjoy a royalty-free, non-exclusive license to use, copy, modify and
 * redistribute these modifications to the Software made by the User and
 * distributed to third parties as a derivative work under this
 * agreement.
 *
 * 6. This agreement will terminate immediately upon User's breach of, or
 * non-compliance with, any of its terms. User may be held liable for any
 * copyright infringement or the infringement of any other proprietary
 * rights in the Software that is caused or facilitated by the User's
 * failure to abide by the terms of this agreement.
 *
 * Commercial Use
 *
 * Any User wishing to make a commercial use of the Software must contact
 * FF&MH at f.feroz@mrao.cam.ac.uk to arrange an appropriate license.
 * Commercial use includes (1) integrating or incorporating all or part
 * of the source code into a product for sale or license by, or on behalf
 * of, User to third parties, or (2) distribution of the binary or source
 * code to third parties for use with a commercial product sold or
 * licensed by, or on behalf of, User.
 * ------------------------------------------------------------------------------
 *  @file 		multi_nest.h
 *  @date       $Date$
 *  @version    $Revision$
 *  @author     Farhan Feroz
 *  @author		Mike Hobson
 *  @author		Ryan Eagan <reagan@phys.psu.edu>
 *              Joao Pedro Athayde Marcondes de Andre <jpa14@psu.edu>
 *
 *  @brief Provides C++ to Fortran interface.
 */


#define NESTRUN __nested_MOD_nestrun

#ifndef MULTI_NEST_H
#define MULTI_NEST_H

#ifdef __cplusplus

/***************************************** C++ Interface to MultiNest **************************************************/

#include <string>

namespace nested
{
	/**
	* @namespace nested
	*
	* map the Fortran 90 entry points of libnest3.a to C++ functions
	*
	* module nested, function nestRun maps to nested::run
	*
	* the pass-by-reference nature of most of the Fortran is translated away
	* *apart* from the callbacks. The provided call back functions must still accept
	* references rather than values. There is also some confusion as to the type
	* of the first argument of LogLike.
	* Should it be a double * or an farray<double, 1> *? The former seems to
	* work and is simpler.
	*
	* This structure is reverse engineered from looking
	* at gfortran stack traces. It is probably wrong
	*/
	template<typename type, int ndims> class farray_traits;
	
	template<> class farray_traits<double, 1> { public: static const int id = 537; };
	template<> class farray_traits<double, 2> { public: static const int id = 538; };
	template<> class farray_traits<int, 1> { public: static const int id = 265; };
	template<> class farray_traits<int, 2> { public: static const int id = 266; };

	/// the extra data for f90 that defines how arrays are arranged.
	template<typename T, int ndim> class farray
	{
		public:
			farray(T *_data, int w, int h = 0) : data(_data), offset(0), type(farray_traits<T, ndim>::id), 
			x_stride(1), x_lbound(1), x_ubound(w), y_stride(w), y_lbound(1), y_ubound(h) {};
			
			T *data;
			int offset;
			int type;
			int x_stride, x_lbound, x_ubound;
			int y_stride, y_lbound, y_ubound;
	};
	
	/// Calls the NESTRUN function in the fortran library.
	extern "C" {
		void NESTRUN(int &IS, bool &mmodal, int &ceff, int &nlive, double &tol, double &efr, int &ndims,
			int &nPar, int &nClsPar, int &maxModes, int &updInt, double &Ztol, char *root, int &seed,
			int *pWrap, int &fb, int &resume, int &outfile, int &initMPI, double &logZero, int &maxiter, int &miniter,
			void (*Loglike)(double *Cube, int &n_dim, int &n_par, double &lnew, void *),
			void (*dumper)(int &, int &, int &, double **, double **, double **, double &, double &, double &, void *),
			void *context, int &root_len);
	}
    
	static inline void run(bool IS, bool mmodal, bool ceff, int nlive, double tol, double efr, int ndims, int nPar, int nClsPar, int maxModes,
		int updInt, double Ztol, std::string& root, int seed, int *pWrap, bool fb, bool resume, bool outfile, 
		bool initMPI, double logZero, int maxiter, int miniter, void (*LogLike)(double *Cube, int &n_dim, int &n_par, double &lnew, void *),
		void (*dumper)(int &, int &, int &, double **, double **, double **, double &, double &, double &, void *), void *context)
	{
		char t_root[100];
		std::fill(t_root, t_root + 100, ' ');
		snprintf(t_root, 99, "%s", root.c_str());
		int root_len = strlen(t_root);
		t_root[strlen(t_root)] = ' ';
	
		int t_fb = fb;
		int t_resume = resume;
		int t_outfile = outfile;
		int t_initMPI = initMPI;
		bool t_mmodal = mmodal;
		int t_ceff = ceff;
		int t_IS = IS;
		
		NESTRUN(t_IS, t_mmodal, t_ceff, nlive, tol, efr, ndims, nPar, nClsPar, maxModes, updInt, Ztol, t_root, seed, pWrap, t_fb, 
		t_resume, t_outfile, t_initMPI, logZero, maxiter, miniter, LogLike, dumper, context, root_len);
	}
}

/***********************************************************************************************************************/

#else // ifdef __cplusplus

/***************************************** C Interface to MultiNest **************************************************/
/*
/// Calls the NESTRUN function in the fortran library.
extern void NESTRUN(int *, bool *, int *, int *, double *, double *, int *, int *, int *, int *, int *, double *, 
char *, int *, int *, int *, int *, int *, int *, double *, int *, void (*Loglike)(double *, int *, int *, 
double *, void *), void (*dumper)(int *, int *, int *, double **, double **, double **, double *, 
double *, double *, void *), void *context);

void run(int IS, bool mmodal, int ceff, int nlive, double tol, double efr, int ndims, int nPar, int nClsPar, 
int maxModes, int updInt, double Ztol, char root[], int seed, int *pWrap, int fb, int resume, int outfile, 
int initMPI, double logZero, int maxiter, int miniter, void (*LogLike)(double *, int *, int *, double *, void *), 
void (*dumper)(int *, int *, int *, double **, double **, double **, double *, double *, double *, void *), 
void *context)
{
	int i;
	for (i = strlen(root); i < 100; i++) root[i] = ' ';

        NESTRUN(&IS, &mmodal, &ceff, &nlive, &tol, &efr, &ndims, &nPar, &nClsPar, &maxModes, &updInt, &Ztol,
        root, &seed, pWrap, &fb, &resume, &outfile, &initMPI, &logZero, &maxiter, &miniter, LogLike, dumper, context);
}
*/
/***********************************************************************************************************************/

#endif // ifdef __cplusplus

#endif // MULTI_NEST_H
