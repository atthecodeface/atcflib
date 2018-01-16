/*a Documentation
 */
/*a Includes
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "vector.h"
#include "matrix.h"

/*a Defines
 */
#define EPSILON (1E-20)
#define PI (M_PI)
#define MATRIX_VALUE(r,c) _values[(c)*_col_stride+(r)*_row_stride]
#define MATRIX_VALUE_M(M,r,c) (M)._values[(c)*(M)._col_stride+(r)*(M)._row_stride]
#define SWAP(a,b) { int t; t = (a); (a) = (b); (b) = t; }

/*a Infix operator methods for doubles
 */
/*f operator*=
 */
template <typename T>
c_matrix<T> &c_matrix<T>::operator*=(T real)
{
    this->scale(real);
    return *this;
}

/*f operator/=
 */
template <typename T>
c_matrix<T> &c_matrix<T>::operator/=(T real)
{
    this->scale(((T)1)/real);
    return *this;
}

/*a Infix operator methods for c_matrix's
 */
/*f operator=
 */
template <typename T>
c_matrix<T> &c_matrix<T>::operator=(const c_matrix<T> &other)
{
    set_size(other._nrows, other._ncols);
    for (int i=0; i<other._nrows*other._ncols; i++) {
        _values[i] = other._values[i];
    }
    return *this;
}

/*f operator+=
 */
template <typename T>
c_matrix<T> &c_matrix<T>::operator+=(const c_matrix<T> &other)
{
    this->add_scaled(other,(T)1);
    return *this;
}

/*f operator-=
 */
template <typename T>
c_matrix<T> &c_matrix<T>::operator-=(const c_matrix<T> &other)
{
    this->add_scaled(other,(T)-1);
    return *this;
}

/*a Constructors
 */
/*f c_matrix<T>::set_size (nrows, ncols)
 */
template <typename T>
void c_matrix<T>::set_size(int nrows, int ncols)
{
    if (_values!=NULL) {
        if (_values_must_be_freed) {
            free(_values);
        }
        _values = NULL;
    }
    _values = (T *) malloc(sizeof(T)*nrows*ncols);
    if (_values == NULL) {
        _nrows = 0;
        _ncols = 0;
        _row_stride = 0;
        _col_stride = 0;
        _values_must_be_freed = 0;
        return;
    }
    _values_must_be_freed = 1;
    _ncols = ncols;
    _nrows = nrows;
    _col_stride = 1;
    _row_stride = _ncols;
    for (int i=0; i<_nrows*_ncols; i++) {
        _values[i] = 0;
    }
}

/*f c_matrix<T>::~c_matrix<T>
 */
template <typename T>
c_matrix<T>::~c_matrix(void)
{
    if (_values!=NULL) {
        free(_values);
        _values = NULL;
    }
}

/*f c_matrix<T>::init (void) - null
 */
template <typename T>
void c_matrix<T>::init(void)
{
    _nrows = 0;
    _ncols = 0;
    _row_stride = 0;
    _col_stride = 0;
    _values_must_be_freed = 0;
    _values = NULL;
}

/*f c_matrix<T>::c_matrix (void) - null
 */
template <typename T>
c_matrix<T>::c_matrix(void)
{
    init();
}

/*f c_matrix<T>::c_matrix (nrows, ncols) - null
 */
template <typename T>
c_matrix<T>::c_matrix(int nrows, int ncols)
{
    init();
    set_size(nrows, ncols);
}

/*f c_matrix<T>::c_matrix (other) - copy from other
 */
template <typename T>
c_matrix<T>::c_matrix(const c_matrix<T> &other)
{
    init();
    set_size(other._nrows, other._ncols);
    for (int r=0; r<_nrows; r++) {
        for (int c=0; c<_ncols; c++) {
            MATRIX_VALUE(r,c) = MATRIX_VALUE_M(other,r,c);
        }
    }
}

/*f c_matrix<T>::c_matrix (rows, cols, const double *values)
 */
template <typename T>
c_matrix<T>::c_matrix(int nrows, int ncols, const T *values)
{
    init();
    set_size(nrows, ncols);
    for (int r=0; r<_nrows; r++) {
        for (int c=0; c<_ncols; c++) {
            MATRIX_VALUE(r,c) = values[r*_ncols+c];
        }
    }
}

/*f c_matrix<T>::c_matrix (c_matrix<T> &a, c_matrix<T> &b) - a * b
 */
template <typename T>
c_matrix<T>::c_matrix(const c_matrix<T> &a, const c_matrix<T> &b)
{
    init();
    if (a._ncols != b._nrows) {
        return;
    }
    set_size(a._nrows, b._ncols);
    this->multiply(a,b);
}

/*f c_matrix<T>::copy
 */
template <typename T>
c_matrix<T> *c_matrix<T>::copy(void) const
{
    return new c_matrix<T>(*this);
}

/*f c_matrix<T>::__str__
 */
template <typename T>
void c_matrix<T>::__str__(char *buffer, int buf_size) const
{
    int pr;
    
    pr = snprintf(buffer, buf_size, "[" );
    if (pr<0) { buffer[buf_size-1] = 0; return; }
    buffer += pr; buf_size-=pr;
    for (int r=0; r<_nrows; r++) {
        pr = snprintf(buffer, buf_size, "[" );
        if (pr<0) { buffer[buf_size-1] = 0; return; }
        buffer += pr; buf_size-=pr;
        for (int c=0; c<_ncols; c++) {
            pr = snprintf(buffer, buf_size, "%lf, ", MATRIX_VALUE(r,c) );
            if (pr<0) { buffer[buf_size-1] = 0; return; }
            buffer += pr; buf_size-=pr;
        }
        pr = snprintf(buffer, buf_size, "]," );
        if (pr<0) { buffer[buf_size-1] = 0; return; }
        buffer += pr; buf_size-=pr;
    }
    pr = snprintf(buffer, buf_size, "]" );
    buffer[buf_size-1] = 0;
    return;
}

/*a Linear operations
 */
/*f c_matrix<T>::multiply
 */
template <typename T>
c_matrix<T> &c_matrix<T>::multiply(const c_matrix<T> &a, const c_matrix<T> &b)
{
    if (a._ncols != b._nrows) return *this;
    if (_nrows != a._nrows) return *this;
    if (_ncols != b._ncols) return *this;
    for (int ar=0; ar<a._nrows; ar++) {
        for (int bc=0; bc<b._ncols; bc++) {
            double d;
            d = 0.0;
            for (int br_ac=0; br_ac<b._nrows; br_ac++) {
                d += MATRIX_VALUE_M(a,ar,br_ac) * MATRIX_VALUE_M(b,br_ac,bc);
            }
            MATRIX_VALUE(ar,bc) = d;
        }
    }
    return *this;
}

/*f c_matrix<T>::add_scaled
 */
template <typename T>
c_matrix<T> &c_matrix<T>::add_scaled(const c_matrix<T> &other, T scale)
{
    if ((other._nrows!=_nrows) || (other._ncols!=_ncols)) return *this;
    for (int r=0; r<_nrows; r++) {
        for (int c=0; c<_ncols; c++) {
            MATRIX_VALUE(r,c) += MATRIX_VALUE_M(other,r,c)*scale;
        }
    }
    return *this;
}

/*f c_matrix<T>::scale
 */
template <typename T>
c_matrix<T> &c_matrix<T>::scale(T scale)
{
    for (int r=0; r<_nrows; r++) {
        for (int c=0; c<_ncols; c++) {
            MATRIX_VALUE(r,c) *= scale;
        }
    }
    return *this;
}

/*f c_matrix<T>::set_identity
 */
template <typename T>
c_matrix<T> &c_matrix<T>::set_identity(void)
{
    for (int r=0; r<_nrows; r++) {
        for (int c=0; c<_ncols; c++) {
            MATRIX_VALUE(r,c) = (r==c)?1.0 : 0.0;
        }
    }
    return *this;
}

/*f c_matrix<T>::transpose_stride
 */
template <typename T>
c_matrix<T> &c_matrix<T>::transpose_stride(void)
{
    if (_nrows==_ncols) {
        SWAP(_nrows, _ncols);
        SWAP(_row_stride, _col_stride);
    }
    return *this;
}

/*f c_matrix<T>::transpose_data
 *
 * Run through all the entries of the matrix and see where they map from
 * after transposition
 *
 * Some map to the same index - leave them
 *
 * Others map from a different index, and that from another, and so on,
 * forming a loop. If the index is not the 'lowest' of the loop then
 * just move on - so each loop is done once.
 *
 * In a loop, remember the first value, and copy around the loop
 *
 * Finally, swap nrows/ncols
 *
 */
template <typename T>
c_matrix<T> &c_matrix<T>::transpose_data(void)
{
    if ( (_nrows != _ncols) &&
         (_row_stride!=(_ncols *_col_stride)) &&
         (_col_stride!=(_nrows *_row_stride)) )
    {
        return *this; // cannot transpose the data
    }

    for (int i=0; i<_nrows*_ncols; i++) {
        int n=0;
        int start_of_loop=1;
        double x;
        for (int j=i;; n++) {
            int r,c;
            r = j/_nrows;
            c = j%_nrows;
            j = c*_ncols + r; // j is after transposition
            start_of_loop = (j==i);
            if (j<=i) break;
        }
        if (n==0) continue;
        if (!start_of_loop) continue;
        x = _values[i];
        for (int j=i;;) {
            int r,c,pj;
            pj = j;
            r = j/_nrows;
            c = j%_nrows;
            j = c*_ncols + r; // j is after transposition
            //fprintf(stderr,"%d:%d,%d (%lf):",j,r,c,_values[j]);
            _values[pj] = _values[j];
            if (j==i) {
                _values[pj] = x;
                break;
            }
        }
        //fprintf(stderr,"\n");
    }
    SWAP(_nrows, _ncols);
    if (_col_stride<_row_stride) { // i.e. elements are _col_stride apart throughout
        _row_stride = _ncols * _col_stride;
    } else {
        _col_stride = _nrows * _row_stride;
    }
    return *this;
}

/*f c_matrix<T>::get_row
 */
template <typename T>
c_vector<T> *c_matrix<T>::get_row(int row)
{
    c_vector<T> *v = new c_vector<T>(_ncols);
    for (int c=0; c<_ncols; c++) {
        v->set(c,MATRIX_VALUE(row,c));
    }
    return v;
}

/*f c_matrix<T>::get_column
 */
template <typename T>
c_vector<T> *c_matrix<T>::get_column(int col)
{
    c_vector<T> *v = new c_vector<T>(_nrows);
    for (int r=0; r<_nrows; r++) {
        v->set(r,MATRIX_VALUE(r,col));
    }
    return v;
}

/*a Vector operations
 */
/*f c_matrix<T>::apply(const vector)
 */
template <typename T>
c_vector<T> *c_matrix<T>::apply(const c_vector<T> &v)
{
    if (v.length() != _ncols) return NULL;
    c_vector<T> *rv = new c_vector<T>(_nrows);
    for (int r=0; r<_nrows; r++) {
        double d=0.0;
        for (int c=0; c<_ncols; c++) {
            d += MATRIX_VALUE(r,c) * v.get(c);
        }
        rv->set(r,d);
    }
    return rv;
}

/*f c_matrix<T>::apply(const vector, double *, int)
 */
template <typename T>
T *c_matrix<T>::apply(const c_vector<T> &v, T *rv, int stride)
{
    if (v.length() != _ncols) return NULL;
    for (int r=0; r<_nrows; r++) {
        double d=0.0;
        for (int c=0; c<_ncols; c++) {
            d += MATRIX_VALUE(r,c) * v.get(c);
        }
        rv[r*stride] = d;
    }
    return rv;
}

/*f c_matrix<T>::apply(const vector, vector)
 */
template <typename T>
c_vector<T> &c_matrix<T>::apply(const c_vector<T> &v, c_vector<T> &rv)
{
    apply(v, rv.coords_to_set(NULL), rv.stride() );
    return rv;
}

/*a Quaternion operations
 */
/*f c_matrix<T>::set_from_quaternion
 Maths from http://www.gamasutra.com/view/feature/131686/rotating_objects_using_quaternions.php?page=2
 */
template <typename T>
c_matrix<T> &c_matrix<T>::set_from_quaternion(const c_quaternion<T> &q)
{
    set_identity();
    if (_nrows != _ncols) return *this;
    if (_nrows < 3) return *this;

    T l = q.modulus();

    T x2 = q.i() + q.i();
    T y2 = q.j() + q.j();
    T z2 = q.k() + q.k();
    T xx = q.i() * x2;
    T xy = q.i() * y2;
    T xz = q.i() * z2;
    T yy = q.j() * y2;
    T yz = q.j() * z2;
    T zz = q.k() * z2;
    T wx = q.r() * x2;
    T wy = q.r() * y2;
    T wz = q.r() * z2;

    MATRIX_VALUE(0,0) = l - (yy + zz)/l;
    MATRIX_VALUE(1,0) = (xy - wz)/l;
    MATRIX_VALUE(2,0) = (xz + wy)/l;

    MATRIX_VALUE(0,1) = (xy + wz)/l;
    MATRIX_VALUE(1,1) = l - (xx + zz)/l;
    MATRIX_VALUE(2,1) = (yz - wx)/l;

    MATRIX_VALUE(0,2) = (xz - wy)/l;
    MATRIX_VALUE(1,2) = (yz + wx)/l;
    MATRIX_VALUE(2,2) = l - (xx + yy)/l;
    return *this;
}

/*a LU methods */
/*f c_matrix<T>::lup_decompose
 */
template <typename T>
int c_matrix<T>::lup_decompose(c_vector<T> **P)
{
    if (_nrows != _ncols) return -1;
    
    if (P!=NULL) {
        *P = new c_vector<T>(_nrows);
        for (int i=0; i<_nrows; i++) {
            (*P)->set(i,i);
        }
    }

    for (int d=0; d<_nrows-1; d++) { // for each element of the diagonal except the last
        double v_max=0.0;
        int r_max=-1;
        for (int r=d; r<_nrows; r++) {
            double t=MATRIX_VALUE(r,d);
            if (t<0) t=-t;
            if (t>=v_max) {
                v_max = t;
                r_max = r;
            }
        }
        if (r_max<0) {
            return -2;// not invertible
        }

        // Swap row i with r_max and update the pivot list
        if (r_max != d) {
            if (P) {
                double p_r_max;
                p_r_max = (*P)->get(r_max);
                (*P)->set(r_max, (*P)->get(d));
                (*P)->set(d, p_r_max);
            }
            for (int c=0; c<_ncols; c++) {
                double v;
                v = MATRIX_VALUE(r_max,c);
                MATRIX_VALUE(r_max,c) = MATRIX_VALUE(d,c);
                MATRIX_VALUE(d,c) = v;
            }
        }

        // Subtract out from rows below scaling down by LU[d][d] (in p) and up by LU[r][r]
        for (int r=d+1; r<_nrows; r++) {
            double scale = MATRIX_VALUE(r,d)/MATRIX_VALUE(d,d);
            MATRIX_VALUE(r,d) = scale;
            for (int c=d+1; c<_ncols; c++) {
                MATRIX_VALUE(r,c) -= scale*MATRIX_VALUE(d,c);
            }
        }
    }
    return 0;
}

/*f c_matrix<T>::lup_get_l
 *
 * The LUP has L in the lower, with U in the upper
 * The actual L has 1 on the diagonals, 0 on the upper
 */
template <typename T>
c_matrix<T> &c_matrix<T>::lup_get_l(void)
{
    for (int r=0; r<_nrows; r++) {
        for (int c=r; c<_ncols; c++) {
            if (c==r) {
                MATRIX_VALUE(r,c)=1.0;
            } else {
                MATRIX_VALUE(r,c)=0.0;
            }
        }
    }
    return *this;
}

/*f c_matrix<T>::lup_get_u
 *
 * The LUP has L in the lower, with U in the upper
 * The actual U has 0 on the lower
 */
template <typename T>
c_matrix<T> &c_matrix<T>::lup_get_u(void)
{
    for (int r=1; r<_nrows; r++) {
        for (int c=0; (c<r) && (c<_ncols); c++) {
            MATRIX_VALUE(r,c)=0.0;
        }
    }
    return *this;
}

/*f c_matrix<T>::lup_unpivot
 *
 * P is a vector of row numbers
 * P[0] is the row of the matrix that must be returned in row 0 after unpivot
 * P[1] is the row of the matrix that must be returned in row 1 after unpivot
 * 
 * Since this is messy (and unpivot is not common), a new matrix is returned
 */
template <typename T>
c_matrix<T> *c_matrix<T>::lup_unpivot(const c_vector<T> &P)
{
    c_matrix<T> *UP = new c_matrix<T>(_nrows, _ncols);
    for (int r=0; r<_nrows; r++) {
        int Pr = (int) P.get(r);
        if (Pr<0) Pr=0;
        if (Pr>=_nrows) Pr=_nrows;
        for (int c=0; c<_ncols; c++) {
            MATRIX_VALUE_M((*UP),r,c) = MATRIX_VALUE(Pr,c);
        }
    }
    return UP;
}

/*f c_matrix<T>::lup_inverse
 *
 * this should be an LU matrix
 * Note that LUP decomposition of M has M = P.L.U
 *
 * If L.U.x(c) (for a column vector x(c)) = I(c) (for the c'th column of the identity matrix)
 * then we can put together the x(c) according to the pivot vector P to generate the inverse.
 *
 * Now, if L.U.x(c) = I(c), then L.y(c) = I(c), where y(c) = U.x(c)
 *
 * Since L is a lower matrix, for column c can construct y with top c-1 elements are 0,
 * whose next element is 1; this will ensure that the top c-1 elements of L.y will be 0,
 * and the c'th element will be 1. Then the c+k'th element of L.y is:
 *  sum(L(c+k,l)*y(l)), which needs to be 0
 * (l=c,c+1,...c+k since y(l<c)=0 and L(c+k,>c+k)=0)
 * Also, L(c+k,c+k)=1
 * Hence y(c+k) = 1/L(c+k,c+k) * -(sum(L(c+k,l)*y(l))) for l=c,c+1,..c+k-1
 * and since L(c+k,c+k) is 1, this simplifies to:
 * y(c+k) = -(sum(L(c+k,l)*y(l))) for l=c,c+1,..c+k-1
 * 
 * Now if we have a vector y(c) as above, we know that y(c) = U.x(c), and we need
 * to construct x(c)
 *
 * Note that y(c)(r) = sum(U(l,r)*x(c)(l)) (0<=l<n)
 * with U(<n-1,n-1) being 0, for example, for n by n matrices
 * Hence x(c)(n-1) = y(c)(n-1)/U(c,n-1)
 *
 * Hence again x(c)(r) = (y(c)(r)-sum(U(l,r)*x(c)(l))/U(c,r),
 * for l=r+1..n
 */
template <typename T>
int c_matrix<T>::lup_inverse(T *data) const
{
    for (int c=0; c<_ncols; c++) {
        T *y, *x;
        y = data;
        x = y;
        // Find y(c) such that L.y = c'th column of I
        for (int r=0; r<_nrows; r++) {
            y[r] = 0.0;
        }
        y[c] = 1.0;
        for (int r=0; r<_nrows; r++) {
            for (int k=r+1; k<_ncols; k++) {
                y[k] -= MATRIX_VALUE(k,r) * y[r];
            }
        }

        // Find x(c) such that U.x(c) = y(c)
        for (int r=0; r<_nrows; r++) {
            x[r] = y[r];
        }
        for (int r=_nrows-1; r>=0; r--) {
            if (MATRIX_VALUE(r,r)==0.0) {
                return -1;
            }
            x[r] = x[r]/MATRIX_VALUE(r,r);
            // For the rest of the column remove multiples of x[r] (Uir, r>i>=0)
            for (int i=0; i<r; i++) {
                x[i] -= MATRIX_VALUE(i,r)*x[r];
            }
        }
        data += _ncols;
    }
    return 0;
}

/*f c_matrix<T>::lup_inverse
 *
 */
template <typename T>
c_matrix<T> *c_matrix<T>::lup_inverse(void) const
{
    if (_nrows != _ncols) return NULL;

    c_matrix<T> *R = this->copy();
    c_vector<T> *P = new c_vector<T>(_ncols);

    T stack_data[16];
    T *data = stack_data;
    T *free_me = NULL;
    if (_nrows>4) {
        data = (T *)malloc(sizeof(T)*_nrows*_ncols);
        free_me = data;
    }

    if (lup_inverse(data)!=0) {
        if (free_me) { free(free_me); }
        delete P;
        return R;
    }
    for (int c=0; c<_ncols; c++) {
        // L.U.x(c) = c'th column of I; hence R[P[c]] = x(c)
        int p_c = (int)P->get(c);
        for (int r=0; r<_nrows; r++) {
            MATRIX_VALUE_M(*R,r,p_c) = data[r];
        }
        data += _ncols;
    }
    if (free_me) { free(free_me); }
    delete P;
    return R;
}


/*f c_matrix<T>::lup_invert
 *
 */
template <typename T>
int c_matrix<T>::lup_invert(const c_vector<T> &P)
{
    if (_nrows != _ncols) return 0;
    T stack_data[16];
    T *data = stack_data;
    T *free_me = NULL;
    if (_nrows>4) {
        data = (T *)malloc(sizeof(T)*_nrows*_ncols);
        free_me = data;
    }
    if (lup_inverse(data)!=0) {
        if (free_me) { free(free_me); }
        return -1;
    }
    for (int c=0; c<_ncols; c++) {
        // L.U.x(c) = c'th column of I; hence R[P[c]] = x(c)
        int p_c = (int)P.get(c);
        for (int r=0; r<_nrows; r++) {
            MATRIX_VALUE(r,p_c) = data[r];
        }
        data += _ncols;
    }
    if (free_me) { free(free_me); }
    return 0;
}

/*a Explicit instantiations of the template
 */
template class c_matrix<double>;
template class c_matrix<float>;
