#ifndef _NR_UTIL_H_
#define _NR_UTIL_H_

#include <string>
#include <cmath>
#include <complex>
#include <iostream>

//Definitions of functions and classes for use with the Numerical Recipes 
//algorithms.

/**
 * Calculate the square of two classes that have a defined * operator.
 * @param a --An object of type const T.
 * @return --An object of type T that is a*a.
 */ 
template<class T>
inline const T SQR(const T a) {return a*a;}

/**
 * Compare the sign of two objects, and return the first such that it has the 
 * same sign as the second.
 * @param &a --A class of type T.
 * @param &b --A class of type T.
 * @return --An object of type T that is either a or -a, depending on the 
 * sign of b.
 */
template<class T>
inline const T SIGN(const T &a, const T &b)
	{return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);}

/* @brief A home-grown vector type for use with Numerical Recipes algorithms.
 * Vector, in this context takes the mathematical definition, rather than 
 * meaning a dynamic container.
 */
template <class T>
class NRVec {
private:
	int nn;	// size of array. upper index is nn-1
	T *v;
public:
	/**
	 * Default constructor.
	 */
	NRVec();

	/**
	 * Create a vector of dimension n, located at the origin.
	 * @param n --The dimension of this vector.
	 */
	explicit NRVec(int n);		
	
	/*
	 * Create a vector of dimension n, that has a constant value.
	 * @param T &a --A value to assign to every coordinate of this vector.
	 * @param n --The dimension of this array.
	 */
	NRVec(const T &a, int n);	
	
	/*
	 * Create a vector of dimension n, from an existing array.
	 * @param T *a --An array of n elements of type T. 
	 * @param n --The dimension of this array.
	 */
	NRVec(const T *a, int n);

	/*
	 * Copy constructor.
	 */
	NRVec(const NRVec &rhs);

	/*
	 * Overloaded assignment operator.
	 */
	NRVec & operator=(const NRVec &rhs);

	/**
	 * Overloaded '=' operator that assigns a value to each element of the
	 * array.
	 * @param &a --A value to assign to each element.
	 */
	NRVec & operator=(const T &a);

	/**
	 * Coordinate access operator.
	 */
	inline T & operator[](const int i);

	/** 
	 * Const version of the coordinate access operator.
	 */
	inline const T & operator[](const int i) const;

	/**
	 * The dimension of the vector.
	 */
	inline int size() const;

	/**
	 * Destructor.
	 */
	~NRVec();
};

template <class T>
NRVec<T>::NRVec() : nn(0), v(0) {}

template <class T>
NRVec<T>::NRVec(int n) : nn(n), v(new T[n]) {}

template <class T>
NRVec<T>::NRVec(const T& a, int n) : nn(n), v(new T[n])
{
	for(int i=0; i<n; i++)
		v[i] = a;
}

template <class T>
NRVec<T>::NRVec(const T *a, int n) : nn(n), v(new T[n])
{
	for(int i=0; i<n; i++)
		v[i] = *a++;
}

template <class T>
NRVec<T>::NRVec(const NRVec<T> &rhs) : nn(rhs.nn), v(new T[nn])
{
	for(int i=0; i<nn; i++)
		v[i] = rhs[i];
}

template <class T>
NRVec<T> & NRVec<T>::operator=(const NRVec<T> &rhs)
// postcondition: normal assignment via copying has been performed;
//		if vector and rhs were different sizes, vector
//		has been resized to match the size of rhs
{
	if (this != &rhs)
	{
		if (nn != rhs.nn) {
			if (v != 0) delete [] (v);
			nn=rhs.nn;
			v= new T[nn];
		}
		for (int i=0; i<nn; i++)
			v[i]=rhs[i];
	}
	return *this;
}

template <class T>
NRVec<T> & NRVec<T>::operator=(const T &a)	//assign a to every element
{
	for (int i=0; i<nn; i++)
		v[i]=a;
	return *this;
}

template <class T>
inline T & NRVec<T>::operator[](const int i)	//subscripting
{
	return v[i];
}

template <class T>
inline const T & NRVec<T>::operator[](const int i) const	//subscripting
{
	return v[i];
}

template <class T>
inline int NRVec<T>::size() const
{
	return nn;
}

template <class T>
NRVec<T>::~NRVec()
{
	if (v != 0)
		delete[] (v);
}

/**
 * @brief A home-grown Matrix class for use with the Numerical Recipes 
 * algorithms.
 */
template <class T>
class NRMat {
private:
	int nn;
	int mm;
	T **v;
public:
	/**
	 * Default constructor.
	 */
	NRMat();
	
	/**
	 * Create a Matrix full of zeros.
	 * @param n --The number of rows of the new matrix.
	 * @param m --The number of columns of the matrix.
	 */
	NRMat(int n, int m);
	
	/**
	 * Create a Matrix each element of which is initialized with 
	 * some value.
	 * @param &a --A value of type T.
	 * @param n --The number of rows.
	 * @param m --The number of columns.
	 */
	NRMat(const T &a, int n, int m);

	/**
	 * Create a Matrix by copying the contents of one array.
	 * @param *a --An array of type T's, of length m*n.
	 * @param n --The number of rows.
	 * @param m --The number of columns.
	 */
	NRMat(const T *a, int n, int m);

	/**
	 * Copy constructor.
	 */
	NRMat(const NRMat &rhs);

	/**
	 * Assignment operator.
	 */
	NRMat & operator=(const NRMat &rhs);

	/**
	 * Overloaded '=' operator that assigns a value of a to every element.
	 */
	NRMat & operator=(const T &a);

	/**
	 * Element access operator.
	 */
	inline T* operator[](const int i);

	/**
	 * Const version of the element access operator.
	 */
	inline const T* operator[](const int i) const;
	
	/**
	 * The number of rows.
	 * @return --The number of rows for this matrix.
	 */
	inline int nrows() const;
	
	/**
	 * The number of columns.
	 * @return --The number of columns for this matrix.
	 */
	inline int ncols() const;

	/**
	 * The destructor.
	 */
	~NRMat();
};

template <class T>
NRMat<T>::NRMat() : nn(0), mm(0), v(0) {}

template <class T>
NRMat<T>::NRMat(int n, int m) : nn(n), mm(m), v(new T*[n])
{
	v[0] = new T[m*n];
	for (int i=1; i< n; i++)
		v[i] = v[i-1] + m;
}

template <class T>
NRMat<T>::NRMat(const T &a, int n, int m) : nn(n), mm(m), v(new T*[n])
{
	int i,j;
	v[0] = new T[m*n];
	for (i=1; i< n; i++)
		v[i] = v[i-1] + m;
	for (i=0; i< n; i++)
		for (j=0; j<m; j++)
			v[i][j] = a;
}

template <class T>
NRMat<T>::NRMat(const T *a, int n, int m) : nn(n), mm(m), v(new T*[n])
{
	int i,j;
	v[0] = new T[m*n];
	for (i=1; i< n; i++)
		v[i] = v[i-1] + m;
	for (i=0; i< n; i++)
		for (j=0; j<m; j++)
			v[i][j] = *a++;
}

template <class T>
NRMat<T>::NRMat(const NRMat &rhs) : nn(rhs.nn), mm(rhs.mm), v(new T*[nn])
{
	int i,j;
	v[0] = new T[mm*nn];
	for (i=1; i< nn; i++)
		v[i] = v[i-1] + mm;
	for (i=0; i< nn; i++)
		for (j=0; j<mm; j++)
			v[i][j] = rhs[i][j];
}

template <class T>
NRMat<T> & NRMat<T>::operator=(const NRMat<T> &rhs)
// postcondition: normal assignment via copying has been performed;
//		if matrix and rhs were different sizes, matrix
//		has been resized to match the size of rhs
{
	if (this != &rhs) {
		int i,j;
		if (nn != rhs.nn || mm != rhs.mm) {
			if (v != 0) {
				delete[] (v[0]);
				delete[] (v);
			}
			nn=rhs.nn;
			mm=rhs.mm;
			v = new T*[nn];
			v[0] = new T[mm*nn];
		}
		for (i=1; i< nn; i++)
			v[i] = v[i-1] + mm;
		for (i=0; i< nn; i++)
			for (j=0; j<mm; j++)
				v[i][j] = rhs[i][j];
	}
	return *this;
}

template <class T>
NRMat<T> & NRMat<T>::operator=(const T &a)	//assign a to every element
{
	for (int i=0; i< nn; i++)
		for (int j=0; j<mm; j++)
			v[i][j] = a;
	return *this;
}

template <class T>
inline T* NRMat<T>::operator[](const int i)	//subscripting: pointer to row i
{
	return v[i];
}

template <class T>
inline const T* NRMat<T>::operator[](const int i) const
{
	return v[i];
}

template <class T>
inline int NRMat<T>::nrows() const
{
	return nn;
}

template <class T>
inline int NRMat<T>::ncols() const
{
	return mm;
}

template <class T>
NRMat<T>::~NRMat()
{
	if (v != 0) {
		delete[] (v[0]);
		delete[] (v);
	}
}

/**
 * @brief A 3D matrix for use with the Numerical Recipes algorithms.
 */
template <class T>
class NRMat3d {
private:
	int nn;
	int mm;
	int kk;
	T ***v;
public:
	/**
	 * Default constructor.
	 */
	NRMat3d();
	/**
	 * Create a 3D Matrix.
	 * @param n --The number of rows.
	 * @param m --The number of columns.
	 * @param k --The size of the third dimension.
	 */
	NRMat3d(int n, int m, int k);

	/**
	 * Element access operator.
	 */
	inline T** operator[](const int i);

	/**
	 * Const version of the access operator.
	 */
	inline const T* const * operator[](const int i) const;

	/**
	 * The size of dimension 1.
	 * @return --The number of elements in dimension 1.
	 */
	inline int dim1() const;
	
	/**
	 * The size of dimension 2.
	 * @return --The number of elements in dimension 2.
	 */
	inline int dim2() const;
	
	/**
	 * The size of dimension 2.
	 * @return --The number of elements in dimension 2.
	 */
	inline int dim3() const;

	/**
	 * Destructor.
	 */
	~NRMat3d();
};

template <class T>
NRMat3d<T>::NRMat3d(): nn(0), mm(0), kk(0), v(0) {}

template <class T>
NRMat3d<T>::NRMat3d(int n, int m, int k) : nn(n), mm(m), kk(k), v(new T**[n])
{
	int i,j;
	v[0] = new T*[n*m];
	v[0][0] = new T[n*m*k];
	for(j=1; j<m; j++)
		v[0][j] = v[0][j-1] + k;
	for(i=1; i<n; i++) {
		v[i] = v[i-1] + m;
		v[i][0] = v[i-1][0] + m*k;
		for(j=1; j<m; j++)
			v[i][j] = v[i][j-1] + k;
	}
}

template <class T>
inline T** NRMat3d<T>::operator[](const int i) //subscripting: pointer to row i
{
	return v[i];
}

template <class T>
inline const T* const * NRMat3d<T>::operator[](const int i) const
{
	return v[i];
}

template <class T>
inline int NRMat3d<T>::dim1() const
{
	return nn;
}

template <class T>
inline int NRMat3d<T>::dim2() const
{
	return mm;
}

template <class T>
inline int NRMat3d<T>::dim3() const
{
	return kk;
}

template <class T>
NRMat3d<T>::~NRMat3d()
{
	if (v != 0) {
		delete[] (v[0][0]);
		delete[] (v[0]);
		delete[] (v);
	}
}

#endif /* _NR_UTIL_H_ */
