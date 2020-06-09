#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/I3Matrix.h>
#include <millipede/Millipede.h>

#include <vector>
#include <cmath>

#include <cholmod.h>
#include <SuiteSparseQR_C.h>

#include "linalg_solver.h"

using namespace std; // Get C99 math functions back

MillipedeFitParams::MillipedeFitParams() : I3LogLikelihoodFitParams(),
    qtotal(0), predicted_qtotal(0), squared_residuals(0), chi_squared(0),
    chi_squared_dof(0), logl_ratio(0) { }

static cholmod_dense *
FillMillipedeDataVector(const MillipedeDOMCacheMap &datamap, cholmod_common *c)
{
	cholmod_dense *data;
	int j = 0;

	for (MillipedeDOMCacheMap::const_iterator i = datamap.begin();
	    i != datamap.end(); i++)
		j += i->second.valid.count();
	
	data = cholmod_l_zeros(j, 1, CHOLMOD_REAL, c);

	j = 0;
	for (MillipedeDOMCacheMap::const_iterator i = datamap.begin();
	    i != datamap.end(); i++) {
		for (int k = 0; k < i->second.nbins; k++) {
			if (!i->second.valid[k])
				continue;
			((double *)(data->x))[j] = i->second.charges[k];
			j++;
		}
	}

	return data;
}

static cholmod_dense *
FillMillipedeNoiseVector(const MillipedeDOMCacheMap &datamap, cholmod_common *c)
{
	cholmod_dense *data;
	int j = 0;

	for (MillipedeDOMCacheMap::const_iterator i = datamap.begin();
	    i != datamap.end(); i++) 
		j += i->second.valid.count();

	data = cholmod_l_zeros(j, 1, CHOLMOD_REAL, c);

	j = 0;
	for (MillipedeDOMCacheMap::const_iterator i = datamap.begin();
	    i != datamap.end(); i++) {
		for (int k = 0; k < i->second.nbins; k++)
			if (i->second.valid[k])
				((double *)(data->x))[j++] =
				    i->second.noise[k];
	}
	assert(j == int(data->nrow));

	return data;
}

static cholmod_sparse *
GetMillipedeStochasticPenaltyMatrix(const std::vector<I3Particle> &sources,
    cholmod_common *c)
{
	cholmod_triplet *pen_trip = cholmod_l_allocate_triplet(
	    sources.size() - 1, sources.size(), sources.size(), 0,
	    CHOLMOD_REAL, c);
	pen_trip->nnz = 0;

	for (unsigned i = 0; i < pen_trip->nrow; i++) {
		if (sources[i].GetShape() == I3Particle::ContainedTrack)
			continue;

		((long *)(pen_trip->i))[pen_trip->nnz] = i;
		((long *)(pen_trip->j))[pen_trip->nnz] = i;
		((double *)(pen_trip->x))[pen_trip->nnz] = 1;
		pen_trip->nnz++;
	}

	cholmod_sparse *penalty = cholmod_l_triplet_to_sparse(pen_trip, 0, c);
	cholmod_l_free_triplet(&pen_trip, c);

	return penalty;
}

static cholmod_sparse *
GetMillipedeContinuousPenaltyMatrix(const std::vector<I3Particle> &sources,
    cholmod_common *c)
{
	cholmod_triplet *pen_trip = cholmod_l_allocate_triplet(
	    sources.size() - 1, sources.size(), sources.size()*2, 0,
	    CHOLMOD_REAL, c);
	pen_trip->nnz = 0;

	for (unsigned i = 0, j = 0; i < pen_trip->nrow; i = j) {
		if (sources[i].GetShape() != I3Particle::ContainedTrack) {
			j++;
			continue;
		}

		((long *)(pen_trip->i))[pen_trip->nnz] = i;
		((long *)(pen_trip->j))[pen_trip->nnz] = i;
		((double *)(pen_trip->x))[pen_trip->nnz] = 1;
		pen_trip->nnz++;

		for (j = i + 1; j < pen_trip->ncol && 
		    sources[j].GetShape() != I3Particle::ContainedTrack; j++);

		if (j == pen_trip->ncol)
			break;

		((long *)(pen_trip->i))[pen_trip->nnz] = i;
		((long *)(pen_trip->j))[pen_trip->nnz] = j;
		((double *)(pen_trip->x))[pen_trip->nnz] = -1;
		pen_trip->nnz++;
	}
		
	cholmod_sparse *penalty = cholmod_l_triplet_to_sparse(pen_trip, 0, c);
	cholmod_l_free_triplet(&pen_trip, c);

	return penalty;
}

static cholmod_sparse *
ElementwiseTensorContract(cholmod_sparse *grad, cholmod_dense *vec,
    bool from_left, cholmod_common *c)
{
	/*
	 * This is a special routine for handling contraction of tensors
	 * like dB_ij/dJ^k x_i (or x_j, depending on from_left), where dB_ij
	 * is a modified form of grad, with all the individual sources
	 * shifted away from one another.
	 *
	 * In the case of multiplication from the right (i.e. by energies)
	 * this transforms a <data> x <nsources> x <6 nsources> tensor (stored
	 * as a <data> x <6 nsources> matrix) tensor into a real
	 * <data> x <6 nsources> matrix. This is accomplished by 
	 * multiplying it element-wise (mod 6) by vec.
	 *
	 * For multiplication from the left, this is reversed. Beginning with
	 * the same data format, we end with a <nsources> x <6 nsources> matrix.
	 * This is accomplished by concatenating the gradients (as above),
	 * a matrix multiplication by vec from the left, and subsequent
	 * downward shifting of the columns by the particle index with that
	 * parameter (col / 6).
	 */

	cholmod_sparse *result;
	if (from_left) {
		// Step 1: multiply by vec from the left
		double alpha[] = {1,0}, beta[] = {0,0};
		cholmod_dense *dense = cholmod_l_zeros(grad->ncol, 1,
		    CHOLMOD_REAL, c);
		cholmod_l_sdmult(grad, true /* transpose */, alpha, beta,
		    vec, dense, c);

		// Step 2: Convert to <nsources> x <6 nsources>
		size_t nnz = grad->ncol;
		result = cholmod_l_allocate_sparse(grad->ncol,
		    grad->ncol/6, nnz, true, true, 0, CHOLMOD_REAL, c);
		memcpy(result->x, dense->x, sizeof(double)*nnz);
		cholmod_l_free_dense(&dense, c);
		for (unsigned i = 0; i < nnz; i++)
			((long *)(result->i))[i] = i;
		for (unsigned i = 0; i <= result->ncol; i++)
			((long *)(result->p))[i] = 6*i;
	} else {
		// Element-wise multiplication by right-multiplication
		// by vec along the diagonal.
		cholmod_dense *scale = cholmod_l_allocate_dense(1,
		    grad->ncol, grad->ncol, CHOLMOD_REAL, c);
		for (unsigned i = 0; i < grad->ncol; i++)
			((double *)(scale->x))[i] = ((double *)(vec->x))[i/6];
		result = cholmod_l_copy_sparse(grad, c);
		cholmod_l_scale(scale, CHOLMOD_COL, result, c);
		cholmod_l_free_dense(&scale, c);
	}

	return result;
}

void
Millipede::SolveEnergyLosses(MillipedeDOMCacheMap &datamap,
    std::vector<I3Particle> &sources, cholmod_sparse *basis,
    cholmod_sparse *gradients, double muonRegularization,
    double stochasticRegularization, cholmod_common *c)
{
	if (sources.size() == 0)
		return;

	cholmod_dense *data = FillMillipedeDataVector(datamap, c);
	cholmod_dense *noise = FillMillipedeNoiseVector(datamap, c);

	double alpha[2] = {1,0}, beta[2] = {1,0};
	cholmod_sparse *extra_terms = NULL;

	if (muonRegularization > 0) {
		cholmod_sparse *penalty, *penalty_t, *ptp;

		penalty = GetMillipedeContinuousPenaltyMatrix(sources, c);
		penalty_t = cholmod_l_transpose(penalty, 1, c);
		ptp = cholmod_l_ssmult(penalty_t, penalty, 0, true, true, c);
		cholmod_l_free_sparse(&penalty, c);
		cholmod_l_free_sparse(&penalty_t, c);

		if (extra_terms != NULL) {
			cholmod_sparse *tmp = extra_terms;
			beta[0] = muonRegularization;
			extra_terms = cholmod_l_add(tmp, ptp, alpha, beta,
			    true, true, c);
			cholmod_l_free_sparse(&tmp, c);
			cholmod_l_free_sparse(&ptp, c);
		} else {
			extra_terms = ptp;
			cholmod_dense *scalar = cholmod_l_zeros(1, 1,
			    CHOLMOD_REAL, c);
			((double *)(scalar->x))[0] = muonRegularization;
			cholmod_l_scale(scalar, CHOLMOD_SCALAR, extra_terms,
			    c);
		}
	}

	if (stochasticRegularization > 0) {
		cholmod_sparse *penalty, *penalty_t, *ptp;

		penalty = GetMillipedeStochasticPenaltyMatrix(sources, c);
		penalty_t = cholmod_l_transpose(penalty, 1, c);
		ptp = cholmod_l_ssmult(penalty_t, penalty, 0, true, true, c);
		cholmod_l_free_sparse(&penalty, c);
		cholmod_l_free_sparse(&penalty_t, c);

		if (extra_terms != NULL) {
			cholmod_sparse *tmp = extra_terms;
			beta[0] = stochasticRegularization;
			extra_terms = cholmod_l_add(tmp, ptp, alpha, beta,
			    true, true, c);
			cholmod_l_free_sparse(&tmp, c);
			cholmod_l_free_sparse(&ptp, c);
		} else {
			extra_terms = ptp;
			cholmod_dense *scalar = cholmod_l_zeros(1, 1,
			    CHOLMOD_REAL, c);
			((double *)(scalar->x))[0] = stochasticRegularization;
			cholmod_l_scale(scalar, CHOLMOD_SCALAR, extra_terms,
			    c);
		}
	}

	cholmod_dense *unfolded = pcg_poisson(basis, extra_terms, data, noise,
	    1e-10, 0, 100, c);
	
	cholmod_l_free_sparse(&extra_terms, c);
	cholmod_l_free_dense(&noise, c);
	cholmod_l_free_dense(&data, c);

	for (unsigned i = 0; i < sources.size(); i++)
		sources[i].SetEnergy(((double *)(unfolded->x))[i]);
	cholmod_l_free_dense(&unfolded, c);
}

double
Millipede::FitStatistics(const MillipedeDOMCacheMap &datamap,
    const std::vector<I3Particle> &sources, double energy_epsilon,
    cholmod_sparse *basis, MillipedeFitParams *params, cholmod_common *c)
{
	double alpha[] = {1,0};
	double llh = 0;
	cholmod_dense *data, *unfolded, *refolded;

	if (params != NULL) {
		params->qtotal = 0;
		params->predicted_qtotal = 0;
		params->squared_residuals = 0;
		params->chi_squared = 0;
		params->logl_ratio = 0;
		if (sources.size() == 0) {
			params->chi_squared_dof = 0;
			params->ndof_ = 0;
			params->logl_ = 0;
			params->rlogl_ = 0;
			params->nmini_ = 0;
			return 0;
		}
	}

	data = FillMillipedeDataVector(datamap, c);
	refolded = FillMillipedeNoiseVector(datamap, c);
	if (sources.size() > 0) {
		unfolded = cholmod_l_zeros(sources.size(), 1, CHOLMOD_REAL, c);
		for (unsigned i = 0; i < sources.size(); i++) {
			((double *)(unfolded->x))[i] = sources[i].GetEnergy() +
			    energy_epsilon;
			if (!isfinite(((double *)(unfolded->x))[i]))
			((double *)(unfolded->x))[i] = 0;
		}
		cholmod_l_sdmult(basis, false, alpha, alpha, unfolded,
		    refolded, c);
		cholmod_l_free_dense(&unfolded, c);
	}

	double *xx = ((double *)(data->x));
	double *mu = ((double *)(refolded->x));
	for (unsigned i = 0; i < refolded->nrow; i++) {
		llh += xx[i] * log(mu[i]) - mu[i] - lgamma(xx[i] + 1);
		if (params != NULL) {
			params->qtotal += xx[i];
			params->predicted_qtotal += mu[i];
			params->squared_residuals +=
			    pow(xx[i] - mu[i], 2);
			params->chi_squared += pow(xx[i] - mu[i], 2)/
			    ((xx[i] > 1) ? xx[i] : 1);
			params->logl_ratio += xx[i] *
			    (xx[i] > 0 ? log(xx[i]/mu[i]) : 0) + mu[i] - xx[i];
		}
	}

	cholmod_l_free_dense(&data, c);
	cholmod_l_free_dense(&refolded, c);

	if (params != NULL) {
		params->chi_squared_dof = basis->ncol;
		params->logl_ratio *= 2;
		params->ndof_ = basis->nrow;
		params->logl_ = -llh;
		params->rlogl_ = -llh/basis->nrow;
		params->nmini_ = 0;
	}

	return llh;
}

I3MatrixPtr
Millipede::FisherMatrix(const MillipedeDOMCacheMap &datamap,
    const std::vector<I3Particle> &sources, cholmod_sparse *basis,
    cholmod_common *c)
{
	cholmod_dense *data = FillMillipedeDataVector(datamap, c);
	cholmod_dense *lambda = FillMillipedeNoiseVector(datamap, c);
	cholmod_dense *solution = cholmod_l_zeros(sources.size(), 1,
	    CHOLMOD_REAL, c);
	for (unsigned i = 0; i < sources.size(); i++) {
		((double *)(solution->x))[i] = sources[i].GetEnergy();
		if (!isfinite(((double *)(solution->x))[i]))
			((double *)(solution->x))[i] = 0;
	}

	double alpha[2] = {1,0};
	cholmod_l_sdmult(basis, false, alpha, alpha, solution, lambda, c);

	// The second derivative matrix is the matrix
	// -d2LLH/dE_idE_j = B|x/lambda><1/lambda|B
	// XXX: regularization
	
	// Repurpose lambda to be x/lambda^2
	for (unsigned i = 0; i < lambda->nrow; i++) {
		((double *)(lambda->x))[i] = ((double *)(data->x))[i]/
		    pow(((double *)(lambda->x))[i],2);
	}

	cholmod_sparse *basis_t = cholmod_l_transpose(basis, 1, c);
	cholmod_l_scale(lambda, CHOLMOD_COL, basis_t, c);
	cholmod_sparse *lhs = cholmod_l_ssmult(basis_t, basis, 0, true, true,
	    c);
	cholmod_dense *lhs_d = cholmod_l_sparse_to_dense(lhs, c);
	cholmod_l_free_sparse(&basis_t, c);
	cholmod_l_free_sparse(&lhs, c);

	I3MatrixPtr fisher(new I3Matrix(solution->nrow, solution->nrow, 0));
	memcpy(fisher->data().begin(), lhs_d->x,
	    sizeof(double)*solution->nrow*solution->nrow);
	cholmod_l_free_dense(&lhs_d, c);
	
	cholmod_l_free_dense(&data, c);
	cholmod_l_free_dense(&lambda, c);
	cholmod_l_free_dense(&solution, c);
	
	return fisher;
}

void
Millipede::LLHGradient(const MillipedeDOMCacheMap &datamap,
    const std::vector<I3Particle> &sources, std::vector<I3Particle> &gradsrcs,
    double energy_epsilon, double weight, cholmod_sparse *basis,
    cholmod_sparse *gradients, cholmod_common *c)
{
	double alpha[] = {1,0}, beta[] = {0,0};
	cholmod_dense *data, *unfolded, *refolded, *gradient;

	// A note on math:
	// dlnL/dx = ((data - predict)/predict)^T (dB/dx E + B dE/dx)
	//
	// For dlnL/dE, dE/dE in the second term is trivial and is simply B
	//  B is also independent of E and so that term drops out entirely,
	//  leaving dlnL/dE = ((data - predict)/predict)^T B
	//
	// For anything else, this term of course recurs multiplying dE/dx
	//  (which is quite difficult to evaluate). However, dlnL/dE is an
	//  invariant of our algorithm: it is always 0. As such, the term
	//  containing dE/dx when computing any non-E derivative is always
	//  multiplied by dlnL/dE = 0 and thus drops out.
	//
	// Therefore, for any non-E variable:
	// dlnL/dx = ((data - predict)/predict)^T dB/dx E

	data = FillMillipedeDataVector(datamap, c);
	refolded = FillMillipedeNoiseVector(datamap, c);
	unfolded = cholmod_l_zeros(sources.size(), 1, CHOLMOD_REAL, c);

	// Step 1: Make a vector equal to (observed - predicted)/predicted,
	// and put it back in data
	for (unsigned i = 0; i < sources.size(); i++) {
		((double *)(unfolded->x))[i] = sources[i].GetEnergy() +
		    energy_epsilon;
		if (!isfinite(((double *)(unfolded->x))[i]))
			((double *)(unfolded->x))[i] = 0;
	}
	cholmod_l_sdmult(basis, false, alpha, alpha, unfolded, refolded, c);
	for (unsigned i = 0; i < data->nrow; i++) {
		((double *)(data->x))[i] =
		    (((double *)(data->x))[i] - ((double *)(refolded->x))[i])/
		    ((double *)(refolded->x))[i];
	}
	cholmod_l_free_dense(&refolded, c);

	// Step 2: Compute dB/dx E
	cholmod_sparse *dBdxE = ElementwiseTensorContract(gradients, unfolded,
            false, c);
	cholmod_l_free_dense(&unfolded, c);

	// Step 3: Multiply them
	gradient = cholmod_l_zeros(6*sources.size(), 1, CHOLMOD_REAL, c);
	cholmod_l_sdmult(dBdxE, true, alpha, beta, data, gradient, c);
	cholmod_l_free_dense(&data, c);
	cholmod_l_free_sparse(&dBdxE, c);

	// The gradient is now in blocks of var_1 (all particles), var_2(all)
	for (unsigned i = 0; i < gradsrcs.size(); i++) {
		gradsrcs[i].SetPos(
		    gradsrcs[i].GetPos().GetX() +
		      weight*((double *)(gradient->x))[6*i + 0],
		    gradsrcs[i].GetPos().GetY() +
		      weight*((double *)(gradient->x))[6*i + 1],
		    gradsrcs[i].GetPos().GetZ() +
		      weight*((double *)(gradient->x))[6*i + 2]
		);
		gradsrcs[i].SetTime(
		    gradsrcs[i].GetTime() +
		      weight*((double *)(gradient->x))[6*i + 3]
		);
		gradsrcs[i].SetDir(
		    gradsrcs[i].GetDir().GetZenith() +
		      weight*((double *)(gradient->x))[6*i + 4],
		    gradsrcs[i].GetDir().GetAzimuth() +
		      weight*((double *)(gradient->x))[6*i + 5]
		);
		// gradsrcs[i].SetEnergy(0); // Always at maximum
	}

	cholmod_l_free_dense(&gradient, c);
}

