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
#define MATRIX_VALUE(r,c) _values[(c)+(r)*_ncols]
#define MATRIX_VALUE_M(M,r,c) (M)._values[(c)+(r)*(M)._ncols]

/*a Infix operator methods for doubles
 */
/*f operator*=
 */
c_matrix &c_matrix::operator*=(double real)
{
    this->scale(real);
    return *this;
}

/*f operator/=
 */
c_matrix &c_matrix::operator/=(double real)
{
    this->scale(1.0/real);
    return *this;
}

/*a Infix operator methods for c_matrix's
 */
/*f operator=
 */
c_matrix &c_matrix::operator=(const c_matrix &other)
{
    set_size(other._nrows, other._ncols);
    for (int i=0; i<other._nrows*other._ncols; i++) {
        _values[i] = other._values[i];
    }
    return *this;
}

/*f operator+=
 */
c_matrix &c_matrix::operator+=(const c_matrix &other)
{
    this->add_scaled(other,1.0);
    return *this;
}

/*f operator-=
 */
c_matrix &c_matrix::operator-=(const c_matrix &other)
{
    this->add_scaled(other,-1.0);
    return *this;
}

/*a Constructors
 */
/*f c_matrix::set_size (nrows, ncols)
 */
void c_matrix::set_size(int nrows, int ncols)
{
    if (_values!=NULL) {
        free(_values);
        _values = NULL;
    }
    _values = (double *) malloc(sizeof(double)*nrows*ncols);
    if (_values == NULL) {
        _nrows = 0;
        _ncols = 0;
        return;
    }
    _nrows = nrows;
    _ncols = ncols;
    for (int i=0; i<_nrows*_ncols; i++) {
        _values[i] = 0;
    }
}

/*f c_matrix::~c_matrix
 */
c_matrix::~c_matrix(void)
{
    if (_values!=NULL) {
        free(_values);
        _values = NULL;
    }
}

/*f c_matrix::c_matrix (void) - null
 */
c_matrix::c_matrix(void)
{
    _nrows = 0;
    _ncols = 0;
    _values = NULL;
}

/*f c_matrix::c_matrix (length) - null
 */
c_matrix::c_matrix(int nrows, int ncols)
{
    _values = NULL;
    set_size(nrows, ncols);
}

/*f c_matrix::c_matrix (other) - copy from other
 */
c_matrix::c_matrix(const c_matrix &other)
{
    _values = NULL;
    set_size(other._nrows, other._ncols);
    for (int i=0; i<_nrows*_ncols; i++) {
        _values[i] = other._values[i];
    }
}

/*f c_matrix::c_matrix (rows, cols, const double *values)
 */
c_matrix::c_matrix(int nrows, int ncols, const double *values)
{
    _values = NULL;
    set_size(nrows, ncols);
    for (int i=0; i<nrows*ncols; i++) {
        _values[i] = values[i];
    }
}

/*f c_matrix::c_matrix (c_matrix &a, c_matrix &b) - a * b
 */
c_matrix::c_matrix(const c_matrix &a, const c_matrix &b)
{
    _values = NULL;
    if (a._ncols != b._nrows) {
        set_size(0,0);
        return;
    }
    set_size(a._nrows, b._ncols);
    this->multiply(a,b);
}

/*f c_matrix::copy
 */
c_matrix *c_matrix::copy(void) const
{
    return new c_matrix(*this);
}

/*f c_matrix::__str__
 */
void c_matrix::__str__(char *buffer, int buf_size) const
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
/*f c_matrix::multiply
 */
c_matrix &c_matrix::multiply(const c_matrix &a, const c_matrix &b)
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

/*f c_matrix::add_scaled
 */
c_matrix &c_matrix::add_scaled(const c_matrix &other, double scale)
{
    if ((other._nrows!=_nrows) || (other._ncols!=_ncols)) return *this;
    for (int i=0; i<_nrows*_ncols; i++) {
        _values[i] += other._values[i]*scale;
    }
    return *this;
}

/*f c_matrix::scale
 */
c_matrix &c_matrix::scale(double scale)
{
    for (int i=0; i<_nrows*_ncols; i++) {
        _values[i] *= scale;
    }
    return *this;
}

/*f c_matrix::set_identity
 */
c_matrix &c_matrix::set_identity(void)
{
    for (int i=0, j=0; i<_nrows*_ncols; i++, j=(j+1)%(_ncols+1)) {
        _values[i] = (j==0)?1.0:0.0;
    }
    return *this;
}

/*f c_matrix::transpose
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
c_matrix &c_matrix::transpose(void)
{
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
    int t;
    t = _nrows; _nrows = _ncols; _ncols = t;
    return *this;
}

/*f c_matrix::get_row
 */
c_vector *c_matrix::get_row(int row)
{
    c_vector *v = new c_vector(_ncols);
    for (int i=0; i<_ncols; i++) {
        v->set(i,MATRIX_VALUE(row,i));
    }
    return v;
}

/*f c_matrix::get_column
 */
c_vector *c_matrix::get_column(int col)
{
    c_vector *v = new c_vector(_nrows);
    for (int i=0; i<_nrows; i++) {
        v->set(i,MATRIX_VALUE(i,col));
    }
    return v;
}

/*a Vector operations
 */
/*f c_matrix::apply(const vector)
 */
c_vector *c_matrix::apply(const c_vector &v)
{
    if (v.length() != _ncols) return NULL;
    c_vector *rv = new c_vector(_nrows);
    for (int r=0; r<_nrows; r++) {
        double d=0.0;
        for (int c=0; c<_ncols; c++) {
            d += MATRIX_VALUE(r,c) * v.value(c);
        }
        rv->set(r,d);
    }
    return rv;
}

/*f c_matrix::apply(const vector, double *, int)
 */
double *c_matrix::apply(const c_vector &v, double *rv, int stride)
{
    if (v.length() != _ncols) return NULL;
    for (int r=0; r<_nrows; r++) {
        double d=0.0;
        for (int c=0; c<_ncols; c++) {
            d += MATRIX_VALUE(r,c) * v.value(c);
        }
        rv[r*stride] = d;
    }
    return rv;
}

/*f c_matrix::apply(const vector, vector)
 */
c_vector &c_matrix::apply(const c_vector &v, c_vector &rv)
{
    apply(v, rv.coords_to_set(NULL), rv.stride() );
    return rv;
}

/*a LU methods */
/*f c_matrix::lup_decompose
 */
int c_matrix::lup_decompose(c_vector **P)
{
    if (_nrows != _ncols) return -1;
    
    if (P!=NULL) {
        *P = new c_vector(_nrows);
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
                p_r_max = (*P)->value(r_max);
                (*P)->set(r_max, (*P)->value(d));
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

/*f c_matrix::lup_get_l
 *
 * The LUP has L in the lower, with U in the upper
 * The actual L has 1 on the diagonals, 0 on the upper
 */
c_matrix &c_matrix::lup_get_l(void)
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

/*f c_matrix::lup_get_u
 *
 * The LUP has L in the lower, with U in the upper
 * The actual U has 0 on the lower
 */
c_matrix &c_matrix::lup_get_u(void)
{
    for (int r=1; r<_nrows; r++) {
        for (int c=0; (c<r) && (c<_ncols); c++) {
            MATRIX_VALUE(r,c)=0.0;
        }
    }
    return *this;
}

/*f c_matrix::lup_unpivot
 *
 * P is a vector of row numbers
 * P[0] is the row of the matrix that must be returned in row 0 after unpivot
 * P[1] is the row of the matrix that must be returned in row 1 after unpivot
 * 
 * Since this is messy (and unpivot is not common), a new matrix is returned
 */
c_matrix *c_matrix::lup_unpivot(const c_vector &P)
{
    c_matrix *UP = new c_matrix(_nrows, _ncols);
    for (int r=0; r<_nrows; r++) {
        int Pr = (int) P.value(r);
        if (Pr<0) Pr=0;
        if (Pr>=_nrows) Pr=_nrows;
        for (int c=0; c<_ncols; c++) {
            MATRIX_VALUE_M((*UP),r,c) = MATRIX_VALUE(Pr,c);
        }
    }
    return UP;
}

/*f c_matrix::lup_inverse
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
int c_matrix::lup_inverse(double *data) const
{
    for (int c=0; c<_ncols; c++) {
        double *y, *x;
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

/*f c_matrix::lup_inverse
 *
 */
c_matrix *c_matrix::lup_inverse(void) const
{
    if (_nrows != _ncols) return NULL;

    c_matrix *R = this->copy();
    c_vector *P = new c_vector(_ncols);

    double stack_data[16];
    double *data = stack_data;
    double *free_me = NULL;
    if (_nrows>4) {
        data = (double *)malloc(sizeof(double)*_nrows*_ncols);
        free_me = data;
    }

    if (lup_inverse(data)!=0) {
        if (free_me) { free(free_me); }
        delete P;
        return NULL;
    }
    for (int c=0; c<_ncols; c++) {
        // L.U.x(c) = c'th column of I; hence R[P[c]] = x(c)
        int p_c = (int)P->value(c);
        for (int r=0; r<_nrows; r++) {
            MATRIX_VALUE_M(*R,r,p_c) = data[r];
        }
        data += _ncols;
    }
    if (free_me) { free(free_me); }
    delete P;
    return R;
}


/*f c_matrix::lup_invert
 *
 */
int c_matrix::lup_invert(const c_vector &P)
{
    if (_nrows != _ncols) return 0;
    double stack_data[16];
    double *data = stack_data;
    double *free_me = NULL;
    if (_nrows>4) {
        data = (double *)malloc(sizeof(double)*_nrows*_ncols);
        free_me = data;
    }
    if (lup_inverse(data)!=0) {
        if (free_me) { free(free_me); }
        return -1;
    }
    for (int c=0; c<_ncols; c++) {
        // L.U.x(c) = c'th column of I; hence R[P[c]] = x(c)
        int p_c = (int)P.value(c);
        for (int r=0; r<_nrows; r++) {
            MATRIX_VALUE(r,p_c) = data[r];
        }
        data += _ncols;
    }
    if (free_me) { free(free_me); }
    return 0;
}

