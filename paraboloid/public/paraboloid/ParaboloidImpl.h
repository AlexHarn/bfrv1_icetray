/**
 * @file ParaboloidImpl.h
 * (C) 2006 the icecube collaboration
 * $Id$
 *
 * This code was copied from sieglinde/reconstruction/SLParaboloidAux.hh
 * Written by Marc Hellwig
 * Imported to IceTray/Gulliver by David Boersma
 * Further developed and maintained by Timo Griesel, Chad Finley and Jon Dumm
 *
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 * @author Timo Griesel <griesel@uni-mainz.de>
 * @author Chad Finley <cfinley@icecube.wisc.edu>
 * @author Jon Dumm <jdumm@icecube.wisc.edu>
 */

#ifndef __PARABOLOIDIMPL_HH__
#define __PARABOLOIDIMPL_HH__

// standard library stuff
#include <vector>

// dataclass stuff
#include <dataclasses/I3Direction.h>
#include <dataclasses/I3Quaternion.h>

// boost stuff
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>


namespace bnu=boost::numeric::ublas;

// WARNING
// We now have both std::vector<double> and boost::numeric::ublas::vector<double> in the code.
// That is grammatically fine but requires strict namespace discipline.

namespace ParaboloidImpl {

    /**
     * Surprisingly, boost::numeric::ublas does not have its own determinant calculation.
     * @param[in] input a square matrix
     */
    double determinant(const bnu::matrix<double>& input);

    /**
     * Surprisingly, boost::numeric::ublas does not have its own matrix inverse calculation.
     * But it's relatively easy to get that from the available ublas functions.
     * @param[in] input a square matrix
     * @param[out] inverse a matrix with the same shape as the input
     * @returns true = success
     *          false = failure (LU facturization failed)
     */
    bool invert_matrix(const bnu::matrix<double>& input, bnu::matrix<double>& inverse);

    /// input is passed by value, because we need a modifiable copy
    int solve_linear_equation(bnu::matrix<double> input,
                              const bnu::vector<double>& inhomo,
                              bnu::vector<double>& solution);

    /**
     * Kramer
     * @brief solve a inhomogeneous linear equation using kramer's method (not very efficient for n>3)
     * @param[in] tmatrixd inhomogenous equation system
     * @param[in] inhomo inhomogenous equation system
     * @param[out] det Determinant of matrix
     * @param[out] solution solution of equation system saved in vector
     * @returns -1 = something went wrong (e.g. dimensions of matrix/vector not compatible)
     *           0 = determinant of matrix is zero -> no unique solution
     *           1 = success
     * @todo finished
     */
    int Kramer ( const bnu::matrix<double> &tmatrixd,
                 const bnu::vector<double> &inhomo,
                 double &det,
                 bnu::vector<double> &solution);
    
    /**
     * diagonalize_sym_2_2_matrix
     * @brief diagonalize a symmetric 2x2 matrix, calculate the eigenvalues and the angle between the eigen vectors
    
     * @param[in]  tmatrixd
     * @param[out] eigen1 eigenvalue of the matrix
     * @param[out] eigen2 eigenvalues of the matrix
     * @param[out] angle = angle between eigen vectors in radians
    
     * @returns -1 = something went wrong (e.g. matrix not 2x2)
     *           0 = determinant of matrix < 0 -> no eigenvalues
     *           1 = success
     * @todo finished
     */
    int diagonalize_sym_2_2_matrix  (
            bnu::matrix<double> &tmatrixd, // input
            double &eigen1,
            double &eigen2,
            double &angle); // output
    
    /**
     * @class RotateLocalNet
     * @brief Rotate coordinates in a local coordinate system (theta,phi) to a global coordinate system and vice versa.
     *
     * @todo finished
     *
     * @sa I3ParaboloidFitter, GridStar
     */
    class RotateLocalNet
    {
    public:
        RotateLocalNet() {}
        ~RotateLocalNet() {}
    
        int Init(const I3Direction &origin); 
        int Rotate(double &Azimuth, double &Zenith);
        int Invert(double &Azimuth, double &Zenith);
    
    private:
        I3Quaternion M_Transform_Forward;  // use for rotation
        I3Quaternion M_Transform_Backward; // use for rotation
    
        static const double C_Zero_Zenith;
        static const double C_Zero_Azimuth;
    };
    
    /**
     * @class GridStar
     * @brief Create ellipses in a global spherical coordinate system centered around a given point
     *
     * @todo finished
     *
     * @sa I3ParaboloidFitter, GridStar, ParabolaFit
     */
    class GridStar
    {
    public:
        GridStar() {}
        ~GridStar() {}
    
        int Init(int n_Pts_on_Circle, int n_Number_Circles, double n_Azi_Reach, double n_Zen_Reach, I3Direction &track); 
        int GridNext(double &Azi_Local, double &Zen_Local, double &Azi_Global, double &Zen_Global); 
    
        RotateLocalNet localnet;
    
    private:
        double Azi_Reach, Zen_Reach;
        int Number_Circles, Count_Circles;
        int Pts_on_Circle, Count_Points; 
        double Stretch;
    
        std::vector<double> zenith_table;
        std::vector<double> azimuth_table;
    };
    
    /**
     * @class XYTupel
     * @brief Basic data type for ParabolaFit
     *
     * @todo finished
     *
     * @sa I3ParaboloidFitter, ParabolaFit
     */
    class XYTupel
    {
    public:
        double x[2];
        double y;
    };
    
    class ParabolaTypePol_2;
    
    /**
     * @class ParabolaTypeStd_2
     * @brief Describes a 2 dimensional Parabola using A, b, c as parameters 
     *
     * @todo finished
     *
     * @sa I3ParaboloidFitter, ParabolaFit, ParabolaTypePol_2
     */
    class ParabolaTypeStd_2
    {
    public:
        ParabolaTypeStd_2() : c(0), b(bnu::vector<double>(2)), A(bnu::matrix<double>(2,2)) {}
        ParabolaTypeStd_2(ParabolaTypePol_2 &pol) : c(0), b(bnu::vector<double>(2)), A(bnu::matrix<double>(2,2)) {*this = pol;}
        ~ParabolaTypeStd_2() {}
        ParabolaTypeStd_2 &operator= (ParabolaTypePol_2 &pol);
    
        double get_y(double x[2]);
    
        double c;
        bnu::vector<double> b;
        bnu::matrix<double> A;
    };
    
    /**
     * @class ParabolaTypePol_2
     * @brief Describes a 2 dimensional Parabola using alpha, beta, gamma as parameters 
     *
     * @todo finished
     *
     * @sa I3ParaboloidFitter, ParabolaFit, ParabolaTypeStd_2
     */
    class ParabolaTypePol_2
    {
    public:
        ParabolaTypePol_2() : alpha(0), beta(bnu::vector<double>(2)), gamma(bnu::matrix<double>(2,2)) {}
        ParabolaTypePol_2(ParabolaTypeStd_2 &std) : alpha(0), beta(bnu::vector<double>(2)), gamma(bnu::matrix<double>(2,2)) {*this = std;}
        ~ParabolaTypePol_2() {}
        ParabolaTypePol_2 &operator= (ParabolaTypeStd_2 &std);
    
        double get_y(double x[2]);
    
        double alpha;
        bnu::vector<double> beta;
        bnu::matrix<double> gamma;
    };
    
    /**
     * @class ParabolaFit
     * @brief Fits a two dimensional parabola to a symmetric set of sampling points
     *
     * @todo finished
     *
     * @sa I3ParaboloidFitter, ParabolaFit, ParabolaTypePol_2, ParabolaTypeStd_2, XYTupel
     */
    class ParabolaFit
    {
    public:
        ParabolaFit(size_t n=Standard_Vector_Size)
        {
            data_points.reserve(n);
            Clear();
        }
        ~ParabolaFit() {}
    
        inline void Clear() {data_points.clear();}
        inline void push_back(const XYTupel &add) {data_points.push_back(add);}
        inline size_t size() const {return data_points.size();}
    
        int lin_reg_parabola_2_sym(ParabolaTypePol_2 &para, double *chi2 = NULL, double *detM = NULL);
    
        static const size_t Standard_Vector_Size;
        static const double Zero_Limit;
    
    private:
        class SumType 
        {
        public:
            SumType() {Clear();}
            ~SumType() {}
    
            void BuildManySums2(const std::vector<ParaboloidImpl::XYTupel> &data_points);
    
            bool IsSymmetric();
    
            double sumxi[4][3+2];
            double sumyixi[3][2+1];
    
        private:
            void Clear();
        };
    
        std::vector<ParaboloidImpl::XYTupel> data_points;
        SumType sums;
    };
    
};
    
#endif /* __PARABOLOIDIMPL_HH__ */
