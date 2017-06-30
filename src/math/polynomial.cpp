/*a Documentation
 */
/*a Includes
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "vector.h"
#include "matrix.h"
#include "polynomial.h"

/*a Defines
 */
#define EPSILON (1E-20)
#define PI (M_PI)
#define FORALL_COEFFS(t,x) for (t x=0; x<_ncoeffs; x++)

/*a Infix operator methods for doubles
 */
/*f operator*=
 */
c_polynomial &c_polynomial::operator*=(double real)
{
    this->scale(real);
    return *this;
}

/*f operator/=
 */
c_polynomial &c_polynomial::operator/=(double real)
{
    this->scale(1.0/real);
    return *this;
}

/*a Infix operator methods for c_polynomial's
 */
/*f operator=
 */
c_polynomial &c_polynomial::operator=(const c_polynomial &other)
{
    set_size(other._ncoeffs);
    for (int i=0; i<other._ncoeffs; i++) {
        _coeffs[i] = other._coeffs[i];
    }
    return *this;
}

/*f operator+=
 */
c_polynomial &c_polynomial::operator+=(const c_polynomial &other)
{
    this->add_scaled(&other,1.0);
    return *this;
}

/*f operator-=
 */
c_polynomial &c_polynomial::operator-=(const c_polynomial &other)
{
    this->add_scaled(&other,-1.0);
    return *this;
}

/*a Constructors
 */
/*f c_polynomial::resize_at_least(min_ncoeffs)
 */
void c_polynomial::resize_at_least(int min_ncoeffs)
{
    double *new_coeffs;
    int new_max_dimension;
    if (min_ncoeffs<_max_dimension) return;
    new_max_dimension = min_ncoeffs+16;
    new_coeffs = (double *)malloc(sizeof(double)*new_max_dimension);
    FORALL_COEFFS(int,i) {
        new_coeffs[i] = _coeffs[i];
    }
    _max_dimension = new_max_dimension;
    free_coeffs_if_necessary();
    _coeffs = new_coeffs;
    _coeffs_must_be_freed = 1;
}

/*f c_polynomial::free_coeffs_if_necessary
 */
void c_polynomial::free_coeffs_if_necessary(void)
{
    if (_coeffs_must_be_freed && _coeffs) {
        free(_coeffs);
        _coeffs = NULL;
    }
}

/*f c_polynomial::alloc_support_structures
 */
int c_polynomial::alloc_support_structures(void)
{
    if (!matrix) matrix = new c_matrix(_ncoeffs,_ncoeffs);
    if (!vector) vector = new c_vector(_ncoeffs);
    return 1;
}

/*f c_polynomial::free_support_structures
 */
void c_polynomial::free_support_structures(void)
{
    if (matrix) {
        free(matrix);
        matrix = NULL;
    }
    if (vector) {
        free(vector);
        vector = NULL;
    }
}

/*f c_polynomial::set_size (ncoeffs)
 */
void c_polynomial::set_size(int ncoeffs)
{
    if (ncoeffs>_max_dimension) {
        _max_dimension = (ncoeffs+16);
        free_coeffs_if_necessary();
        _coeffs = (double *) malloc(sizeof(double)*_max_dimension);
        _coeffs_must_be_freed = 1;
    }
    for (int i=0; i<_max_dimension; i++) {
        _coeffs[i] = 0;
    }
    _ncoeffs = ncoeffs;
    free_support_structures();
}

/*f c_polynomial::~c_polynomial
 */
c_polynomial::~c_polynomial(void)
{
    if (_coeffs_must_be_freed) {
        if (_coeffs) { free(_coeffs); }
        _coeffs = NULL;
        _coeffs_must_be_freed = 0;
    }
    _max_dimension = 0;
    _ncoeffs = 0;
    free_support_structures();
}

/*f c_polynomial::base_init
 */
void c_polynomial::base_init(void)
{
    _max_dimension = _POLYNOMIAL_INITIAL_MAX_DIMENSION;
    _ncoeffs = 0;
    _coeffs = &(_initial_coeffs[0]);
    _coeffs_must_be_freed = 0;
    matrix = NULL;
    vector = NULL;
    set_size(0);
}

/*f c_polynomial::c_polynomial (void) - null
 */
c_polynomial::c_polynomial(void)
{
    base_init();
}

/*f c_polynomial::c_polynomial (length) - null
 */
c_polynomial::c_polynomial(int ncoeffs)
{
    base_init();
    set_size(ncoeffs);
}

/*f c_polynomial::c_polynomial (other) - copy from other
 */
c_polynomial::c_polynomial(const c_polynomial &other)
{
    base_init();
    set_size(other._ncoeffs);
    _ncoeffs = other._ncoeffs;
    FORALL_COEFFS(int,i) {
        _coeffs[i] = other._coeffs[i];
    }
}

/*f c_polynomial::c_polynomial (c_polynomial &a, c_polynomial &b) - a * b
 */
c_polynomial::c_polynomial(const c_polynomial &a, const c_polynomial &b)
{
    base_init();
    set_size(a._ncoeffs + b._ncoeffs);
    for (int ai=0; ai<a._ncoeffs; ai++) {
        for (int bi=0; bi<b._ncoeffs; bi++) {
            _coeffs[ai+bi] += a._coeffs[ai]*b._coeffs[bi];
        }
    }
}

/*f c_polynomial::copy
 */
c_polynomial *c_polynomial::copy(void) const
{
    return new c_polynomial(*this);
}

/*f c_polynomial::__str__
 */
void c_polynomial::__str__(char *buffer, int buf_size) const
{
    int pr;
    
    pr = snprintf(buffer, buf_size, "[" );
    if (pr<0) { buffer[buf_size-1] = 0; return; }
    buffer += pr; buf_size-=pr;
    FORALL_COEFFS(int,i) {
        pr = snprintf(buffer, buf_size, "%lf, ", _coeffs[i] );
        if (pr<0) { buffer[buf_size-1] = 0; return; }
        buffer += pr; buf_size-=pr;
    }
    pr = snprintf(buffer, buf_size, "]" );
    buffer[buf_size-1] = 0;
    return;
}

/*f c_polynomial::normalize
 */
c_polynomial *c_polynomial::normalize(void)
{
    while ((_ncoeffs>0) && (fabs(_coeffs[_ncoeffs-1])>EPSILON)) {
        _ncoeffs--;
    }
    return this;
}

/*f c_polynomial::add_scaled
 */
c_polynomial *c_polynomial::add_scaled(const c_polynomial *other, double scale)
{
    resize_at_least(other->_ncoeffs);
    for (int i=0; i<other->_ncoeffs; i++) {
        _coeffs[i] += scale*other->_coeffs[i];
    }
    return this;
}

/*f c_polynomial::scale
 */
c_polynomial *c_polynomial::scale(double scale)
{
    FORALL_COEFFS(int,i) {
        _coeffs[i] *= scale;
    }
    return this;
}

/*f c_polynomial::as_vector
 */
c_vector &c_polynomial::as_vector(void)
{
    c_vector *v = new c_vector(_ncoeffs);
    FORALL_COEFFS(int, i) {
        v->set(i,_coeffs[i]);
    }
    return *v;
}

/*f c_polynomial::evaluate
 */
double c_polynomial::evaluate(double d)
{
    double r = 0.0;
    double di = 1.0;
    FORALL_COEFFS(int,i) {
        r += di*_coeffs[i];
        di *= d;
    }
    return r;
}

/*f c_polynomial::differentiate
 */
c_polynomial *c_polynomial::differentiate(void)
{
    if (_ncoeffs==0) return this;
    FORALL_COEFFS(int,i) {
        if (i==_ncoeffs) break;
        _coeffs[i] = _coeffs[i+1]*(i+1);
    }
    _ncoeffs--;
    return this;
}

/*f c_polynomial::differential_of
 */
c_polynomial *c_polynomial::differential_of(const c_polynomial *f)
{
    resize_at_least(f->_ncoeffs);
    if (f->_ncoeffs==0) {
        _ncoeffs = 0;
        return this;
    }
    _ncoeffs = f->_ncoeffs;
    FORALL_COEFFS(int,i) {
        if (i==_ncoeffs) break;
        _coeffs[i] = f->_coeffs[i+1]*(i+1);
    }
    return this;
}

/*f c_polynomial::best_fit_matrix
 *
 * Find best fit matrix for a given set of 'xs' and mult_poly
 * Uses whatever degree the polynomial is
 *
 * mult_poly is a polynomial that the best_fit polynomial is to be
 * multiplied by to get the actual best_fit polynomial
 * For example, if mult_poly is x^2-1 then effectively the
 * best fit will be for npts points (x,y) to y(x)=(x^2-1)*(best_fit_poly)
 *
 * Hence if there are known conditions that apply to the data, such
 * as y(-1)=y(1)=0 (in the case given) we can still best fit while meeting
 * these conditions.
 *
 * To find the best fit matrix for given xs where there are no known
 * conditions, i.e. mult_poly is y(x)=1, then mult_poly may be NULL.
 * This would be a standard best_fit.
 */
c_matrix *c_polynomial::best_fit_matrix(int npts, double *xs, c_polynomial *mult_poly)
{
    alloc_support_structures();
    FORALL_COEFFS(int,r) {
        double c;
        FORALL_COEFFS(int,i) {
            c = 0.0;
            int pwr = r+i;
            for (int p=0; p<npts; p++) {
                double f=1.0;
                if (mult_poly) {
                    f = mult_poly->evaluate(xs[p]);
                    f *= f;
                }
                c += f*pow(xs[p],pwr); // xs[p]* (mult_poly(xs[p])^2)
            }
            matrix->set(r,i,c); // # Sum(xp^(i+r)) or Sum(xq^(2*i))
        }
    }
    matrix->lup_decompose(&vector);
    matrix->lup_invert(vector);
    return matrix;
}

/*f c_polynomial::best_fit
 *
 * Set coefficients to be best fit for xs and ys
 * Uses whatever degree the polynomial is
 */
c_polynomial *c_polynomial::best_fit(int npts, double *xs, double *ys, c_polynomial *mult_poly)
{
    best_fit_matrix(npts, xs, NULL);
    FORALL_COEFFS(int,r) {
        double c;
        c = 0.0;
        for (int p=0; p<npts; p++) {
            double f=1.0;
            if (mult_poly) {
                f = mult_poly->evaluate(xs[p]);
            }
            c += f * ys[p] * pow(xs[p],r);// * M[p];
        }
        vector->set(r,c);
    }
    matrix->apply(*vector, _coeffs);
    return this;
}

/*f c_polynomial::evaluate_poly
 */
/*f c_polynomial::multiply
 */
/*f c_polynomial::find_root
 */
/*f c_polynomial::divide
 */
