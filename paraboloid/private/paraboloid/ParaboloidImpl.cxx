/**
 * @file ParaboloidImpl.cxx
 * (C) copyright 2006 the icecube collaboration
 * $Id$
 *
 * This code was almost literally copied from
 * sieglinde/reconstruction/SLParaboloidAux.(hh|cc). The code in
 * SLParaboloidAux.(hh|cc) was written by Marc Hellwig (with minor edits
 * by DJB), as a thorough rewrite of the original parabola fit by Till
 * Neunhoeffer in siegmund/recoos.
 *
 * Imported/Copied/Adapted to IceTray/Gulliver by David Boersma
 * Furthere eveloped and maintained by Timo Griesel (and DJB)
 *
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 * @author Timo Griesel <timo.griesel@uni-mainz.de>
 *
 */

#include <cmath>

// boost stuff
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

// dataclass stuff
#include <dataclasses/I3Direction.h>
#include <dataclasses/I3Quaternion.h>
#include <icetray/I3Logging.h>

// paraboloid stuff
#include "paraboloid/ParaboloidImpl.h"

namespace bnu=boost::numeric::ublas;

// static stuff
const double ParaboloidImpl::RotateLocalNet::C_Zero_Zenith=M_PI/2.0;
const double ParaboloidImpl::RotateLocalNet::C_Zero_Azimuth=M_PI;
const size_t    ParaboloidImpl::ParabolaFit::Standard_Vector_Size=100;
const double ParaboloidImpl::ParabolaFit::Zero_Limit=1.e-11;

double ParaboloidImpl::determinant(const bnu::matrix<double>& input)
{
    std::size_t dim = input.size1();
    if (dim != input.size2()){
        // maybe better pull the plug here
        return NAN;
    }
    double det = 0.;
    if (dim==2){
        det = input(0,0)*input(1,1) - input(0,1)*input(1,0);
    } else if (dim==3){
        det  = input(0,0)*(input(1,1)*input(2,2) - input(1,2)*input(2,1));
        det += input(0,1)*(input(1,2)*input(2,0) - input(1,0)*input(2,2));
        det += input(0,2)*(input(1,0)*input(2,1) - input(1,1)*input(2,0));
    } else {
        // NOT IMPLEMENTED
        det = NAN;
    }
    return det;
}

 /* Matrix inversion routine.
  * shamelessly copied from:
  * http://www.crystalclearsoftware.com/cgi-bin/boost_wiki/wiki.pl?LU_Matrix_Inversion
  * Uses lu_factorize and lu_substitute in uBLAS to invert a matrix
  */
bool ParaboloidImpl::invert_matrix(const bnu::matrix<double>& input, bnu::matrix<double>& inverse)
{
    std::size_t dim = input.size1();

    // create a working copy of the input
    bnu::matrix<double> A(input);

    // create a permutation matrix for the LU-factorization
    bnu::permutation_matrix<std::size_t> pm(dim);

    // perform LU-factorization
    int res = lu_factorize(A, pm);
    if (res != 0)
            return false;

    // create identity matrix of "inverse"
    inverse.assign(bnu::identity_matrix<double> (dim));

    // backsubstitute to get the inverse
    bnu::lu_substitute(A, pm, inverse);

    return true;
}

int ParaboloidImpl::solve_linear_equation(bnu::matrix<double> input,
                                          const bnu::vector<double>& inhomo,
                                          bnu::vector<double>& solution){
    if((input.size1()!=input.size2())||(inhomo.size()!=input.size1())) {
        // maybe log_fatal would be more appropriate
        // this is following the conventions of the original code
        return -1;
    }
    solution=inhomo;
    bnu::permutation_matrix<std::size_t> pm(input.size1());
    if (bnu::lu_factorize(input, pm)==0){
        bnu::lu_substitute(input, pm, solution);
        return 1;
    }
    return 0;
}

// solve a inhomogeneous linear equation using kramer's method (not very efficient for n>3)
//int ParaboloidImpl::Kramer( const bnu::matrix<double> &tmatrixd,
//                            const bnu::vector<double> &inhomo,
//                            double &det,
//                            bnu::vector<double> &solution) {
//    if((tmatrixd.size1()!=tmatrixd.size2())||(inhomo.size()!=tmatrixd.size1())) {
//        /*
//        log_error("ERROR: Kramer got inconsistent input"); 
//        log_error("tmatrixd.Ncols=%d tmatrixd.Nrows=%d inhomo.Nrows=%d",
//                   tmatrixd.size1(),
//                   tmatrixd.size2(),
//                   inhomo.size()); 
//        */
//        return -1;
//    }
//
//    det=determinant(tmatrixd);                         // calculate determinant
//    if(det==0){
//        /* log_error("ERROR: Kramer with zero determinant, no unique solution");  */
//        log_debug("ERROR: Kramer with zero determinant, no unique solution");
//        log_debug_stream(tmatrixd);
//        log_debug_stream(inhomo);
//        return 0;                                // no unique solution if det=0
//    }
//
//    int par,col;
//    int n=tmatrixd.size1();
//    bnu::matrix<double> kramer(n,n);  // Kramer matrix
//
//    // loop over the three parameters
//    for(par=0; par<n; par++) {
//        // loop over the columns to fill the Kramer matrix
//        for(col=0; col<n; col++) {
//            // create Kramer matrix for Parameter "par"
//            if(col==par) {
//                // replace column "par" of the original matrix with the
//                // inhomogenouity
//                TMatrixDColumn(kramer, col)=inhomo;
//            } else {
//                // copy column from original matrix
//                TMatrixDColumn(kramer, col)=TMatrixDColumn(tmatrixd, col);
//            }
//        }
//        // calculate determinant of Kramer matrix and fill solution vector
//        solution[par]=kramer.Determinant()/det;
//    }
//    return 1;
//}

// invert a 2x2 matrix and calculate its determinant
int ParaboloidImpl::diagonalize_sym_2_2_matrix (bnu::matrix<double> &matrix,                //  input
    double &eigen1, double &eigen2, double &angle)         // output
{
    double D;                        // determinant of input matrix
    double p, bsquare, myroot;       // temporary values

    // check size of matrix
    if((matrix.size1()!=2)||(matrix.size2()!=2)) {
        /* log_error("diagonalize_sym_2_2_matrix: error: matrix dimension not 2x2");  */
        return -1;
    }
    
    // if our matrix isn't symmetric (within some accuracy), we go home
    if(fabs(matrix(0,1)-matrix(1,0))>ParaboloidImpl::ParabolaFit::Zero_Limit) {
        /*
        log_error("diagonalize_sym_2_2_matrix: error: matrix not symmetric"); 
        log_error("off-diagonal elements are %g and %g", matrix[0][1], matrix[1][0]); 
        log_error("difference is %g",matrix[0][1]-matrix[1][0]); 
        */
        log_debug_stream("diagonalize_sym_2_2_matrix: error: matrix not symmetric"); 
        log_debug_stream(matrix);
        return -1;
    }

    // if our matrix is diagonal, we are done
    if(matrix(0,1)==0) {
        eigen1=matrix(0,0);
        eigen2=matrix(1,1);
        angle=0;
        return 1;
    }

    // we solve the characteristic polynomial with the pq-formula
    p=(matrix(0,0)+matrix(1,1))/2.0;
    // D is the term below the square root
    // D=p^2-q
    D=p*p-((matrix(0,0)*matrix(1,1))-(matrix(0,1)*matrix(1,0)));
    if(D<0) {
        log_debug("diagonalize_sym_2_2_matrix no solution D=%f", D);
        return 0;
    }
    myroot=sqrt(D);
    eigen1=p+myroot;        // x1 = p + sqrt(D)
    eigen2=p-myroot;        // x2 = p - sqrt(D)
    bsquare=matrix(0,1)*matrix(0,1);  // calculate angle between eigen vectors
    // angle=acos(sqrt(bsquare/(bsquare+(eigen1-matrix(0,0))*(eigen1-matrix(0,0)))));
    // this is a bugfix: Alexander Kappes says that you can send bombs to him if it's wrong.
    angle=acos(matrix(0,1)/sqrt(bsquare+(eigen1-matrix(0,0))*(eigen1-matrix(0,0))));

    return 1;
}

/*******************************************************************************
 * class RotateLocalNet
 * Rotate coordinates in a local coordinate system (theta,phi) to a global
 * coordinate system and vice versa.
 ******************************************************************************/

int ParaboloidImpl::RotateLocalNet::Init(const I3Direction &origin) 
{
    // initialize internal forward transformation matrix
    M_Transform_Forward.set(0,0,0,1);
    // Rotate around Y / Z axis. 
    // The resulting Matrix rotates the vector (C_Zero_Zenith, C_Zero_Azimuth)
    // into Vector "origin"
    // // NOTE (DJB2015):
    // // C_Zero_Zenith=M_PI/2.0;
    // // C_Zero_Azimuth=M_PI;
    // // so C_Zero points along the *negative* x-axis!
    // M_Transform_Forward.RotateY(C_Zero_Zenith-origin.GetZenith());
    // M_Transform_Forward.RotateZ(-C_Zero_Azimuth+origin.GetAzimuth());
    double dz = (C_Zero_Zenith-origin.GetZenith());
    double da = -C_Zero_Azimuth+origin.GetAzimuth();
    log_trace("dz=%g da=%g", dz, da);
    // The order in which we do rotations is important.
    // Note that *= multiplies from the right, and that the right-most factor
    // gets applied first when doing a rotation.
    M_Transform_Forward *= I3Quaternion(0,0,sin(da/2),cos(da/2)); // rotate around z
    M_Transform_Forward *= I3Quaternion(0,sin(dz/2),0,cos(dz/2)); // rotate around y
    // the other way around: from global to local
    M_Transform_Backward=M_Transform_Forward.inverse();

    return 1;
}

// transform local coordinates into global coordinates
// The origin of the local coordinate system gets translated to Vector "origin"    
int ParaboloidImpl::RotateLocalNet::Rotate(double &Azimuth, double &Zenith) 
{
    I3Direction work(Zenith+C_Zero_Zenith, Azimuth+C_Zero_Azimuth);

    // use matrix multiplication to rotate
    work=M_Transform_Forward.rotate(work);

    // return global coordinates
    Azimuth=work.GetAzimuth();
    Zenith=work.GetZenith();

    // normalize coordinates
    if(Zenith<0.) {
        Zenith=-Zenith;
        Azimuth+=M_PI;
    }
    while(Azimuth>2.*M_PI) Azimuth-=2.*M_PI;
    while(Azimuth<0.) Azimuth+=2.*M_PI;

    return 1;
}

// transform global coordinates into local coordinates
// Vector "origin" gets translated into the origin of the
// local coordinate system
int ParaboloidImpl::RotateLocalNet::Invert(double &Azimuth, double &Zenith) 
{
    I3Direction work(Zenith, Azimuth);
    work=M_Transform_Backward.rotate(work);

    // return local coordinates
    Azimuth=work.GetAzimuth()-C_Zero_Azimuth;
    Zenith=work.GetZenith()-C_Zero_Zenith;

    // normalize coordinates
    while(Azimuth>M_PI) Azimuth-=2.*M_PI;
    while(Azimuth<-M_PI) Azimuth+=2.*M_PI;

    return 1;
}        
        
/***************************************************************************************
 * class GridStar
 ***************************************************************************************/

// initialize internal data structures according to the given parameters
int ParaboloidImpl::GridStar::Init( int n_Pts_on_Circle, int n_Number_Circles,
            double n_Azi_Reach, double n_Zen_Reach,
            I3Direction &track)
{
    // number of points on circle has to be multiple of 4
    // (used parabola fit only works with symmetric sampling points)
    if((n_Pts_on_Circle%4)!=0) {
        // return error code
        // log_error("ERROR: number of pointts on circle) must be multiple of 4 (got %d)", n_Pts_on_Circle); 
        return -1;
    }

    // fill member variables with given values
    Number_Circles=n_Number_Circles;
    Pts_on_Circle=n_Pts_on_Circle;

    // reserve memory for sampling point coordinate tables
    azimuth_table.resize(Pts_on_Circle);
    zenith_table.resize(Pts_on_Circle);
    Count_Circles=0;
    Count_Points=Pts_on_Circle-1;
    Azi_Reach=n_Azi_Reach/2.0;
    Zen_Reach=n_Zen_Reach/2.0;

    // Initialize local coordinate system. Origin in global system is given
    // by "track"
    localnet.Init(track);

    // calculate tables with sampling point coordinates 
    for(int i=0; i<Pts_on_Circle; i++) {
        // (ellipse with diameters azi_reach, zen_reach)
        azimuth_table[i]=cos(i*2.*M_PI/Pts_on_Circle)*Azi_Reach;
        zenith_table[i]=sin(i*2.*M_PI/Pts_on_Circle)*Zen_Reach;            
    }

    return 1;
}

// calculate local and global coordinates for next point on circles
// returns number of completed circles or zero if all the points
// have been returned
int ParaboloidImpl::GridStar::GridNext( double &Azi_Local, double &Zen_Local,
                    double &Azi_Global, double &Zen_Global)
{
    // last point on circle done ?
    if(++Count_Points==Pts_on_Circle) {
        // increase number of process circles
        Count_Circles++;

        // last circle done ? yes, then return 0
        if(Count_Circles>Number_Circles){
            return 0;
        }

        // calculate stretch value for next circle
        Stretch=(double)Count_Circles/(double)Number_Circles;
        Count_Points=0;
    }

    // calculate position of next sampling point in local coordinate system
    Azi_Local=Azi_Global=azimuth_table[Count_Points]*Stretch;
    Zen_Local=Zen_Global=zenith_table[Count_Points]*Stretch;

    // rotate local vector into global system
    localnet.Rotate(Azi_Global, Zen_Global);

    log_trace("circle=%d point=%d zenloc=%g zenglob=%g aziloc%g aziglob=%g",
               Count_Circles, Count_Points,
               Zen_Local, Zen_Global, Azi_Local, Azi_Global);

    return Count_Circles;
}

/*****************************************************************************
 * classes ParabolaTypeStd_2, ParabolaTypePol_2
 *****************************************************************************/

// return the llh-value at position x for a given parabola in standard
// representation 
double ParaboloidImpl::ParabolaTypeStd_2::get_y(double x[2])
{
    double result;
    result=c ; 
    for(int i=0; i<2; i++) {
        for(int j=0; j<2; j++) {
            result+=0.5*(x[i]-b[i])*A(i,j)*(x[j]-b[j]);
        }
    }
    return result;
}

// assign parameters of a standard parabola representation using a given polar
// representation parameter c is returned as NAN if the parabola is a flat
// surface. Use std::isnan(foo.c) to check for this exception
ParaboloidImpl::ParabolaTypeStd_2 &ParaboloidImpl::ParabolaTypeStd_2::operator= (ParabolaTypePol_2 &pol)
{
    double det;

    A=pol.gamma;
    det=determinant(A);
    bool ok = invert_matrix(pol.gamma,A);
    if((det==0)||std::isnan(det)||!ok){
        c=NAN;
        return *this;
    }

    b[0]=-1.0*(A(0,0)*pol.beta[0]+A(0,1)*pol.beta[1]);
    b[1]=-1.0*(A(1,0)*pol.beta[0]+A(1,1)*pol.beta[1]); 
    c=pol.alpha+0.5*(pol.beta[0]*b[0]+pol.beta[1]*b[1]);
    A=pol.gamma;

    return *this;    
}

// return the llh-value at position x for a given parabola in
// polar representation 
double ParaboloidImpl::ParabolaTypePol_2::get_y(double x[2])
{
    double result;
    
    result=alpha;
    for(int i=0; i<2; i++) {
        result+=beta[i]*x[i];
        for(int j=0; j<2; j++) {
            result+=0.5*x[i]*gamma(i,j)*x[j];
        }
    }

    return result;
}
    
// assign parameters of a polar parabola representation using a given
// standard representation
ParaboloidImpl::ParabolaTypePol_2 &ParaboloidImpl::ParabolaTypePol_2::operator= (ParaboloidImpl::ParabolaTypeStd_2 &std)
{
    beta[0]=-(std.A(0,0)*std.b[0]+std.A(0,1)*std.b[1]);
    beta[1]=-(std.A(1,0)*std.b[0]+std.A(1,1)*std.b[1]);
 
    alpha=-0.5*(std.b[0]*beta[0]+std.b[1]*beta[1])+std.c;
    gamma=std.A;
    
    return *this;
}

/*****************************************************************************
 * class ParabolaFit
 *****************************************************************************/

// Fit a parabola in polar representation to a given set of symmetric
// sampling points. To get the values for chi^2 and the determinant of the
// matrix, call method with pointers to valid double variables, otherwise
// just call method with parameter para (chi2 and detM preinitialized
// with NULL).
// For further details about the used fitting algorithm write to
// Marc.Hellwig@uni-mainz.de or Till.Neunhoeffer@uni-mainz.de to get a
// copy of a paper discussing this fit.
int ParaboloidImpl::ParabolaFit::lin_reg_parabola_2_sym( ParaboloidImpl::ParabolaTypePol_2 &para,
                                 double *chi2, double *detM)
{
    double y;
    bnu::matrix<double> tmatrixd(3,3);
    bnu::vector<double> inhomo(3);
    bnu::vector<double> solution(3);

    // Calculate some sums using the sampling points
    sums.BuildManySums2(data_points);

    // Use sums to check the symmetry condition
    if(!sums.IsSymmetric()) {
        /*
        log_error("ERROR: ParabolaFit::lin_reg_parabola_2_sym sums are not symmetric !"); 
        log_error("Values: sumxi[0][0]=%f, sumxi[0][1]=%f, sumxi[2][0]=%f, "
                   "sumxi[2][1]=%f, sumxi[2][2]=%f, sumxi[2][3]=%f",
                   sums.sumxi[0][0],
                   sums.sumxi[0][1],
                   sums.sumxi[2][0],
                   sums.sumxi[2][1],
                   sums.sumxi[2][2],
                   sums.sumxi[2][3]); 
        */

        // return error value, data points are not symmetric
        return -1;
    }

    // fill matrix with the sums calculated a few code lines above
    // (azimuth, zenith sums)    
    tmatrixd(0,0)=(double)data_points.size();
    tmatrixd(0,1)=0.5*sums.sumxi[1][0];
    tmatrixd(0,2)=0.5*sums.sumxi[1][2];

    tmatrixd(1,0)=sums.sumxi[1][0];
    tmatrixd(1,1)=0.5*sums.sumxi[3][0];
    tmatrixd(1,2)=0.5*sums.sumxi[3][2];

    tmatrixd(2,0)=sums.sumxi[1][2];
    tmatrixd(2,1)=0.5*sums.sumxi[3][2];
    tmatrixd(2,2)=0.5*sums.sumxi[3][4];

    // the inhomogenity of the equation system is given by the likelihood sums
    inhomo[0]=sums.sumyixi[0][0];
    inhomo[1]=sums.sumyixi[2][0];
    inhomo[2]=sums.sumyixi[2][2];

    // use Kramer method to solve linear equation system
    if(ParaboloidImpl::solve_linear_equation(tmatrixd, inhomo, solution)==0) {
        // log_error("ERROR: ParabolaFit::lin_reg_parabola_2_sym determinant of matrix==0"); 

        // linear equation has no unique solution
        return 0;
    }

    // return determinant of matrix if requested
    if(detM){
        *detM=ParaboloidImpl::determinant(tmatrixd);
    }

    // calculate polar parabola representation parameters out of solution
    para.alpha=solution[0];
    para.gamma(0,0)=solution[1];
    para.gamma(1,1)=solution[2];

    // curvature and direction of parabola can't be calculated in some
    // exceptional cases (e.g. likelihood around seed behaves like a
    // saddle or a plane instead of a parabola)
    if((sums.sumxi[1][0]==0.)||(sums.sumxi[1][2]==0.)||(sums.sumxi[3][2]==0.)) {
        /* log_error("ERROR: ParabolaFit::lin_reg_parabola_2_sym "
                   "avoiding division by 0 error.\n"
                   "beta and gamma(1,0) not defined"); */
        return -1;
    }

    para.beta[0]=sums.sumyixi[1][0]/sums.sumxi[1][0];
    para.beta[1]=sums.sumyixi[1][1]/sums.sumxi[1][2];
    para.gamma(0,1)=para.gamma(1,0)=sums.sumyixi[2][1]/sums.sumxi[3][2];

    // calculate chi^2 in the case it was requested    
    if(chi2) {
        *chi2 = 0;
        // loop over all sample points
        for(std::vector<ParaboloidImpl::XYTupel>::iterator p=data_points.begin(); p!=data_points.end(); p++) {
            // Get likelihood of Parabola fit for sample point p
            y=para.get_y(p->x);
            // p->y is "real" likelihood of sample point p
            *chi2+=(y-p->y)*(y-p->y);
        }
    }
    
    return 1;
}        

// use all datapoints to calculate some internal sums used for further calculations
void ParaboloidImpl::ParabolaFit::SumType::BuildManySums2(const std::vector<ParaboloidImpl::XYTupel> &data_points)
{
    double xi[2][5], yi;
    int i, j; 

    // Clear Sum Variables
    Clear();

    // loop over all sampling points
    for(std::vector<ParaboloidImpl::XYTupel>::const_iterator p=data_points.begin(); p!=data_points.end(); p++) {
        xi[0][0]=xi[1][0]=1.;
        xi[0][1]=p->x[0];
        xi[1][1]=p->x[1];
        yi=p->y;
        for(i=2; i<5; i++) {
            xi[0][i]=xi[0][i-1]*xi[0][1];
            xi[1][i]=xi[1][i-1]*xi[1][1];
        }
        for(i=0; i<4; i++) {
            for(j=0; j<i+2; j++)
                sumxi[i][j]+=xi[0][i+1-j]*xi[1][j];
        }
        for(i=0; i<3; i++) {
            for(j=0; j<i+1; j++)
                sumyixi[i][j]+=yi*xi[0][i-j]*xi[1][j];
        }
    }
}

// check if the sample points fulfill the necessary symmetry conditions
bool ParaboloidImpl::ParabolaFit::SumType::IsSymmetric()
{
    if(   (fabs(sumxi[0][0])>Zero_Limit) || (fabs(sumxi[0][1])>Zero_Limit)
       || (fabs(sumxi[2][0])>Zero_Limit) || (fabs(sumxi[2][1])>Zero_Limit)
       || (fabs(sumxi[2][2])>Zero_Limit) || (fabs(sumxi[2][3])>Zero_Limit))
            return false;
    return true;
}

// clear all internal sums
void ParaboloidImpl::ParabolaFit::SumType::Clear()
{
    int i,j;
    for(i=0; i<4; i++) {
        for(j=0; j<i+2; j++)
            sumxi[i][j]=0;
    }
    for(i=0; i<3; i++) {
        for (j=0; j<i+1; j++)
            sumyixi[i][j]=0;
    }
}
