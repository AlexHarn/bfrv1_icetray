/*
 * Copyright (c) 2014 The IceCube Collaboration. All rights reserved.
 *
 * Authors:
 *    Nathan Whitehorn <nwhitehorn@icecube.wisc.edu>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <time.h>
#include <float.h>
#include <assert.h>

#include <cholmod.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_roots.h>

#include "linalg_solver.h"

/*
 * Preconditioned Conjugate Gradient
 *
 * Angelis et al., Phys. Med. Biol. 56 (2011) 3895-3917
 * DOI 10.1088/0031-9155/56/13/010
 *
 * Algorithm is in Appendix A
 * 
 */

static inline void
zero_negative_elements(cholmod_dense *x)
{
	// Truncation operator from paper
	unsigned i;

	for (i = 0; i < x->ncol*x->nrow; i++)
		if (((double *)(x->x))[i] < 0)
			((double *)(x->x))[i] = 0;
}

/* Likelihood function */
struct llh_params {
	cholmod_sparse *B;
	cholmod_sparse *extra_terms;
	cholmod_dense *data;
	cholmod_dense *y;
	cholmod_dense *y_work;
	cholmod_dense *mu;
	cholmod_dense *x;
	cholmod_dense *x_work;
	cholmod_dense *x_scratch;
	cholmod_dense *d;
	cholmod_common *c;
};
static void
poisson_llh_fdfd2f(double alpha, struct llh_params *params, double *val,
    double *grad, double *doublegrad)
{
	double one[2] = {1,0}, zero[2] = {0,0};
	unsigned i;
	double c;

	assert(alpha >= 0);
	for (i = 0; i < params->B->nrow; i++) {
		((double *)(params->y_work->x))[i] =
		    ((double *)(params->y->x))[i] +
		    alpha*((double *)(params->mu->x))[i];
		assert(((double *)(params->y_work->x))[i] > 0);
	}
	if (params->extra_terms != NULL) {
		for (i = 0; i < params->B->ncol; i++)
			((double *)(params->x_work->x))[i] =
			    ((double *)(params->x->x))[i] +
			    alpha*((double *)(params->d->x))[i];
		cholmod_l_sdmult(params->extra_terms, 1, one, zero,
		    params->x_work, params->x_scratch, params->c);
	}

	#define KAHAN_SUM(bound, out, term) \
		for (i = 0; i < bound; i++) { \
			double tmp1, tmp2; \
			tmp1 = term - c; \
			tmp2 = out + tmp1; \
			c = (tmp2 - out) - tmp1; \
			out = tmp2; \
		}

	if (val != NULL) {
		*val = 0; c = 0;

		KAHAN_SUM(params->B->nrow, *val,
		    -(((double *)(params->data->x))[i]*
		    log(((double *)(params->y_work->x))[i]) -
		    ((double *)(params->y_work->x))[i]));

		if (params->extra_terms != NULL)
			KAHAN_SUM(params->B->ncol, *val,
			    ((double *)(params->x_work->x))[i]*
			    ((double *)(params->x_scratch->x))[i]);
	}

	if (grad != NULL) {
		*grad = 0; c = 0;
		KAHAN_SUM(params->B->nrow, *grad, 
		    -(((double *)(params->data->x))[i] -
		     ((double *)(params->y_work->x))[i])/
		    ((double *)(params->y_work->x))[i]*
		    ((double *)(params->mu->x))[i]);
		if (params->extra_terms != NULL)
			KAHAN_SUM(params->B->ncol, *grad, 
			    2*((double *)(params->d->x))[i]*
			    ((double *)(params->x_scratch->x))[i]);
	}
		
	if (doublegrad != NULL) {
		*doublegrad = 0; c = 0;
		KAHAN_SUM(params->B->nrow, *doublegrad, 
		    ((double *)(params->data->x))[i]*
		    pow(((double *)(params->mu->x))[i]/
		     ((double *)(params->y_work->x))[i], 2));
		if (params->extra_terms != NULL) {
			cholmod_l_sdmult(params->extra_terms, 1, one, zero,
			    params->d, params->x_scratch, params->c);
			KAHAN_SUM(params->B->ncol, *doublegrad, 
			    2*((double *)(params->d->x))[i]*
			    ((double *)(params->x_scratch->x))[i]);
		}
	}

	#undef KAHAN_SUM
}

/*
 * Wrap the LLH function with a catch for negative alpha, needed for unbounded
 * root finding.  Jumping to negative alpha should only occur when the LLH function
 * reaches numerical precision limits.
 * 
 * f(alpha) = {alpha >= 0: f(alpha)
 *            {alpha < 0: f(0)
 */ 
static void
poisson_llh_fdfd2f_wrapper(double alpha, struct llh_params *params,
    double *val, double *grad, double *doublegrad)
{

	if (alpha >= 0.) {
		poisson_llh_fdfd2f(alpha, params, val, grad, doublegrad);

	} else {

		poisson_llh_fdfd2f(0., params, val, NULL, NULL);

		if (grad != NULL) {
			*grad = 0.;
		}

		if (doublegrad != NULL) {
			*doublegrad = 0.;
		}
	}
}

static double
poisson_llh_df(double alpha, void *xparams)
{
	double ret;
	poisson_llh_fdfd2f_wrapper(alpha, (struct llh_params *)xparams,
	    NULL, &ret, NULL);
	return ret;
}
static double
poisson_llh_d2f(double alpha, void *xparams)
{
	double ret;
	poisson_llh_fdfd2f_wrapper(alpha, (struct llh_params *)xparams,
	    NULL, NULL, &ret);
	return ret;
}
static void
poisson_llh_dfd2f(double alpha, void *xparams, double *y, double *dy)
{
	poisson_llh_fdfd2f_wrapper(alpha, (struct llh_params *)xparams,
	    NULL, y, dy);
}

static double
alpha_opt_unbounded(struct llh_params *params, int maxiter)
{
	static gsl_root_fdfsolver *s = NULL;
	gsl_function_fdf FDF;
	int status = 0;
	int iter = 0;
	int err = 0;
	double x0, x = 0;

	if (s == NULL)
		s  = gsl_root_fdfsolver_alloc(gsl_root_fdfsolver_newton);

	FDF.params = params;
	FDF.f = &poisson_llh_df;
	FDF.df = &poisson_llh_d2f;
	FDF.fdf = &poisson_llh_dfd2f;

	gsl_root_fdfsolver_set(s, &FDF, x);

	do {
		iter++;
		// N.B. If x < 0, the derivative is zero and newton will return
		// an error.  Watch for the error and quit if we see it.
		err = gsl_root_fdfsolver_iterate(s);
		x0 = x;
		x = gsl_root_fdfsolver_root(s);
		status = gsl_root_test_delta(x, x0, 0, 1e-3);
	} while (status == GSL_CONTINUE && iter < maxiter && !err);

	if (x < 0.) {
		x = 0.;
	}
	return x;
}
		
static double
alpha_opt_bounded(struct llh_params *params, int maxiter, double bound)
{
	static gsl_root_fsolver *s = NULL;
	gsl_function F;
	int status = 0;
	int iter = 0;
	int err = 0;
	double x_lo, x_hi, x = 1.0;
	double left, right, leftgrad, rightgrad;

	if (s == NULL)
		s  = gsl_root_fsolver_alloc(gsl_root_fsolver_brent);

	F.function = &poisson_llh_df;
	F.params = params;

	poisson_llh_fdfd2f(0, params, &left, &leftgrad, NULL);
	poisson_llh_fdfd2f(bound, params, &right, &rightgrad, NULL);

	if ((leftgrad < 0 && rightgrad < 0) || (leftgrad > 0 && rightgrad > 0))
		return (left < right) ? 0 : bound;
	
	gsl_root_fsolver_set(s, &F, 0, bound);

	do {
		iter++;
		err = gsl_root_fsolver_iterate(s);
		x = gsl_root_fsolver_root(s);
		x_lo = gsl_root_fsolver_x_lower(s);
		x_hi = gsl_root_fsolver_x_upper(s);
		status = gsl_root_test_interval(x_lo, x_hi, 0, 1e-3);
	} while (status == GSL_CONTINUE && iter < maxiter && !err);

	return x;
}

#define min(a,b) ((a < b) ? a : b)

cholmod_dense *
pcg_poisson(cholmod_sparse *B, cholmod_sparse *extra_terms,
    cholmod_dense *data, cholmod_dense *noise, double tolerance,
    int min_iterations, int max_iterations, cholmod_common *c)
{
	cholmod_dense *x, *g, *v, *y, *mu, *d;
	cholmod_dense *x_work, *y_work;
	cholmod_dense *last_x, *last_g, *last_v;
	cholmod_dense *colsum;
	double eps = 1.e-16;
	double mtwo[2] = {-2,0}, one[2] = {1,0}, zero[2] = {0,0};
	struct llh_params root_solver_params;
	double alpha, last_alpha, beta_pr;
	unsigned i;
	int k;

	/* Turn off the GSL error handler to avoid the default
	 * abort-on-error behavior
	 */
	gsl_error_handler_t* old_handler = gsl_set_error_handler_off();

	/* Initialize vectors */
	x = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);
	last_x = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);
	g = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);
	last_g = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);
	v = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);
	last_v = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);
	d = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);
	mu = cholmod_l_zeros(B->nrow, 1, CHOLMOD_REAL, c);
	y = cholmod_l_zeros(B->nrow, 1, CHOLMOD_REAL, c);
	assert(y->nrow == data->nrow);
	assert(y->nrow == noise->nrow);

	y_work = cholmod_l_zeros(B->nrow, 1, CHOLMOD_REAL, c);
	x_work = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);

	last_alpha = INFINITY;

	/*
	 * Note: step numbers that follow match those in the paper. However,
	 * the loop has been rearranged to increase code reuse. Where numbers
	 * are separated by slashes, it is because one of the steps from the
	 * initialization sequence (first number) and inner loop (second)
	 * were actually the same.
	 */

	/* Sum columns of response matrix for preconditioner */
	colsum = cholmod_l_zeros(B->ncol, 1, CHOLMOD_REAL, c);
	for (i = 0; i < B->ncol; i++) {
		for (k = ((long*)(B->p))[i]; k < ((long*)(B->p))[i+1]; k++)
			((double*)(colsum->x))[i] += ((double*)(B->x))[k];

		/* If basis function has no support, set to 1 */
		if (((double*)(colsum->x))[i] == 0)
			((double*)(colsum->x))[i] = 1;
	}

	/* Set up root solver params */
	root_solver_params.B = B;
	root_solver_params.extra_terms = extra_terms;
	root_solver_params.data = data;
	root_solver_params.y_work = y_work;
	root_solver_params.x_work = x_work;
	if (extra_terms)
		root_solver_params.x_scratch = cholmod_l_zeros(B->ncol, 1,
		    CHOLMOD_REAL, c);
	root_solver_params.c = c;

	/* Step 1: Initialize first-guess solution to a constant vector
	 * that conserves total charge. */
	{
		double data_qtot = 0, model_qtot = 0, noise_qtot = 0;
		double scale;

		for (i = 0; i < B->ncol; i++)
			model_qtot += ((double *)(colsum->x))[i];
		for (i = 0; i < B->nrow; i++) {
			noise_qtot += ((double *)(noise->x))[i];
			data_qtot += ((double *)(data->x))[i];
		}

		scale = (data_qtot - noise_qtot)/model_qtot;
		if (scale < 0)
			scale = 0;

		for (i = 0; i < B->ncol; i++)
			((double *)(x->x))[i] = scale;
	}

	for (k = 0; k < max_iterations || max_iterations == 0; k++) { 
		/* Step 2/13: calculate the expectations vector, y */
		memcpy(y->x, noise->x, sizeof(double)*y->nrow);
		cholmod_l_sdmult(B, 0, one, one, x, y, c);
		
		/* Step 3/14: compute the gradient of the likelihood, g */
		for (i = 0; i < y->nrow; i++) {
			assert(((double *)(y->x))[i] > 0);
			((double *)(y_work->x))[i] =
			    (((double *)(data->x))[i] -
			     ((double *)(y->x))[i])/
			    ((double *)(y->x))[i];
		}
		cholmod_l_sdmult(B, 1, one, zero, y_work, g, c);

		/* Add penalty terms */
		if (extra_terms != NULL)
			cholmod_l_sdmult(extra_terms, 1, mtwo, one, x, g, c);

		/* Step C: check for convergence */
		{
			double truncated_grad = 0;
			for (i = 0; i < g->nrow; i++) {
				if (((double *)(x->x))[i] == 0 &&
				    ((double *)(g->x))[i] <= 0)
					continue;
				truncated_grad += pow(((double *)(g->x))[i], 2);
			}

			if (truncated_grad < tolerance)
				break;

			/* If two non-steps made in a row, we've hit numerical
			 * precision */
			if (last_alpha == 0 && alpha == 0)
				break;
		}

		/* Step 4/15: calculate gradient direction vector, v */
		for (i = 0; i < g->nrow; i++) {
			((double *)(v->x))[i] = ((double *)(g->x))[i] *
			    (((double *)(x->x))[i] + eps)/
			    ((double *)(colsum->x))[i];
			assert(isfinite(((double *)(v->x))[i]));
		}

		/* Step 16: calculate Polak-Ribiere coefficient, beta_pr */
		beta_pr = 0;
		if (k > 0) {
			for (i = 0; i < g->nrow; i++)
				beta_pr += ((double *)(g->x))[i]*
				    (((double *)(v->x))[i] -
				    ((double *)(last_v->x))[i]);
			beta_pr /= cblas_ddot(x->nrow, (double *)(last_g->x),
			    1, (double *)(last_v->x), 1);
			if (beta_pr < 0)
				beta_pr = 0;
		}

		/* Step 5/17: set search direction, d */
		for (i = 0; i < g->nrow; i++)
			((double *)(d->x))[i] = beta_pr*((double *)(d->x))[i] +
			    ((double *)(v->x))[i];

		/* Step 6: calculate mu */
		cholmod_l_sdmult(B, 0, one, zero, d, mu, c);

		/* Step 18: check if ascent direction */
		root_solver_params.y = y;
		root_solver_params.mu = mu;
		root_solver_params.x = x;
		root_solver_params.d = d;

		/* Note: for numerical reasons, do *exactly* the calculation
		 * from the solver rather than simply projecting d onto g */
		poisson_llh_fdfd2f(0, &root_solver_params, NULL, &alpha, NULL);
		if (alpha > 0) {
			memcpy(d->x, g->x, sizeof(double)*x->nrow);
			cholmod_l_sdmult(B, 0, one, zero, d, mu, c);
		}

		/* Step 7: calculate first step size, alpha */
		{
			double maxalpha = INFINITY;
			for (i = 0; i < mu->nrow; i++) {
				if (((double *)(mu->x))[i] < 0)
					maxalpha = min(-((double *)(y->x))[i]/
					    ((double *)(mu->x))[i], maxalpha);
			}
	
			if (isfinite(maxalpha))
				alpha = alpha_opt_bounded(&root_solver_params,
				    max_iterations, maxalpha*(1. - 1e-9));
			else
				alpha = alpha_opt_unbounded(&root_solver_params,
				    max_iterations);
		}
		assert(alpha >= 0);

		/* Step 8: new x in x_work */
		for (i = 0; i < B->ncol; i++)
			((double *)(x_work->x))[i] =
			    ((double *)(x->x))[i] + alpha*((double *)(d->x))[i];
		zero_negative_elements(x_work);

		/* Step 9: Calculate difference from last values in d */
		for (i = 0; i < B->ncol; i++) {
			((double *)(d->x))[i] =
			    ((double *)(x_work->x))[i] - ((double *)(x->x))[i];
			assert(isfinite(((double *)(d->x))[i]));
		}

		/* Step 10: recalculate mu */
		cholmod_l_sdmult(B, 0, one, zero, d, mu, c);

		/* Step 11: next part of the line search, calculate a step size
		 * between last solution and the current one  */
		alpha = alpha_opt_bounded(&root_solver_params, max_iterations,
		    1);
		assert(alpha >= 0);

		/* Step 12: new coefficients */
		{
			cholmod_dense *temp;
			temp = last_x; last_x = x; x = temp;
			temp = last_g; last_g = g; g = temp;
			temp = last_v; last_v = v; v = temp;
		}

		for (i = 0; i < x->nrow; i++) {
			((double *)(x->x))[i] = ((double *)(last_x->x))[i] +
			    alpha*((double *)(d->x))[i];
			assert(((double *)(x->x))[i] >= 0);
		}
	}

	/* Step 8: return */
	cholmod_l_free_dense(&last_g, c);
	cholmod_l_free_dense(&last_x, c);
	cholmod_l_free_dense(&last_v, c);
	cholmod_l_free_dense(&g, c);
	cholmod_l_free_dense(&v, c);
	cholmod_l_free_dense(&y, c);
	cholmod_l_free_dense(&mu, c);
	cholmod_l_free_dense(&d, c);
	cholmod_l_free_dense(&x_work, c);
	cholmod_l_free_dense(&y_work, c);
	if (extra_terms)
		cholmod_l_free_dense(&root_solver_params.x_scratch, c);

	/* Restore the original GSL error handler */
	gsl_set_error_handler(old_handler);

	return (x);
}

