/** Copyright (C) 2016,  Gavin J Stark.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          polynomial.h
 * @brief         Polynomials and methods thereon
 *
 */

/*a Wrapper
 */
#ifdef __INC_POLYNOMIAL
#else
#define __INC_POLYNOMIAL

/*a Includes
 */
#include "vector.h"
#include "matrix.h"

/*a Defines
 */
#define _POLYNOMIAL_INITIAL_MAX_DIMENSION 8

/*a Types
 */
class c_polynomial
{
    double *_coeffs;
    int _ncoeffs;
    int _max_dimension;
    int _coeffs_must_be_freed;
    double _initial_coeffs[_POLYNOMIAL_INITIAL_MAX_DIMENSION];
    c_matrix *matrix;
    c_vector *vector;
    void base_init(void);
public:
    c_polynomial &operator*=(double real);
    c_polynomial &operator/=(double real);
    inline c_polynomial operator*(double rhs) { c_polynomial lhs=*this; lhs *= rhs; return lhs; }
    inline c_polynomial operator/(double rhs) { c_polynomial lhs=*this; lhs /= rhs; return lhs; }

    c_polynomial &operator=(const c_polynomial &other);
    c_polynomial &operator+=(const c_polynomial &other);
    c_polynomial &operator-=(const c_polynomial &other);
    inline c_polynomial operator+(const c_polynomial &rhs) const { c_polynomial lhs=*this; lhs += rhs; return lhs; }
    inline c_polynomial operator-(const c_polynomial &rhs) const { c_polynomial lhs=*this; lhs -= rhs; return lhs; }
    inline c_polynomial operator-(void) const { c_polynomial lhs=*this; for (int i=0; i<_ncoeffs; i++) lhs._coeffs[i]*=-1; return lhs; }
    inline c_polynomial operator*(const c_polynomial &rhs) const { c_polynomial *r=new c_polynomial(*this,rhs); return *r; }

    ~c_polynomial(void);
    c_polynomial(void);
    c_polynomial(const c_polynomial &polynomial);
    c_polynomial(int length);
    c_polynomial(const c_polynomial &a, const c_polynomial &b);
    int alloc_support_structures(void);
    c_matrix *best_fit_matrix(int npts, double *xs, c_polynomial *mult_poly);
    c_polynomial *best_fit(int npts, double *xs, double *ys, c_polynomial *mult_poly);
    void free_coeffs_if_necessary(void);
    void free_support_structures(void);
    void resize_at_least(int min_ncoeffs);
    void set_size(int ncoeffs);
    c_polynomial *copy(void) const;

    inline int ncoeffs(void) {return _ncoeffs;}
    inline const double *coeffs(void) const {return _coeffs;};
    inline void set(int n, double v) {_coeffs[n]=v;};
    //inline double get(int r, int c) const {return _values[r*_ncols+c];};
    c_polynomial *normalize(void);
    c_polynomial *add_scaled(const c_polynomial *other, double scale);

    c_polynomial *scale(double scale);
    c_vector     &as_vector(void);
    double       evaluate(double x);
    c_polynomial *differentiate(void);
    c_polynomial *differential_of(const c_polynomial *other);
    c_polynomial *best_fit(int npts, double *xs, double *ys);

    void __str__(char *buffer, int buf_size) const;
};

/*a External functions
 */

/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
