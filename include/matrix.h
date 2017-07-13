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
 * @file          matrix.h
 * @brief         Matrices and methods thereon
 *
 */

/*a Wrapper
 */
#ifdef __INC_MATRIX
#else
#define __INC_MATRIX

/*a Includes
 */
#include "vector.h"

/*a Defines
 */

/*a Types
 */
class c_matrix
{
    double *_values;
    int _nrows;
    int _ncols;
public:
    c_matrix &operator*=(double real);
    c_matrix &operator/=(double real);
    inline c_matrix operator*(double rhs) { c_matrix lhs=*this; lhs *= rhs; return lhs; }
    inline c_matrix operator/(double rhs) { c_matrix lhs=*this; lhs /= rhs; return lhs; }

    c_matrix &operator=(const c_matrix &other);
    c_matrix &operator+=(const c_matrix &other);
    c_matrix &operator-=(const c_matrix &other);
    inline c_matrix operator+(const c_matrix &rhs) const { c_matrix lhs=*this; lhs += rhs; return lhs; }
    inline c_matrix operator-(const c_matrix &rhs) const { c_matrix lhs=*this; lhs -= rhs; return lhs; }
    inline c_matrix operator-(void) const { c_matrix lhs=*this; for (int i=0; i<_nrows*_ncols; i++) lhs._values[i]*=-1; return lhs; }
    inline c_matrix operator*(const c_matrix &rhs) const { c_matrix *r=new c_matrix(*this,rhs); return *r; }

    ~c_matrix(void);
    c_matrix(const c_matrix &matrix);
    c_matrix(int nrows, int ncols);
    c_matrix(const c_matrix &a, const c_matrix &b);
    void set_size(int nrows, int ncols);
    c_matrix(void);
    c_matrix(int nrows, int ncols, const double *values);
    c_matrix *copy(void) const;

    inline int nrows(void) {return _nrows;}
    inline int ncols(void) {return _ncols;}
    inline const double *values(void) const {return _values;};
    inline void set(int r, int c, double v) {_values[r*_ncols+c]=v;};
    inline double get(int r, int c) const {return _values[r*_ncols+c];};

    c_vector *get_row(int row);
    c_vector *get_column(int col);
    c_matrix &set_identity(void);
    c_matrix &transpose(void);
    c_matrix &add_scaled(const c_matrix &other, double scale);
    c_matrix &multiply(const c_matrix &a, const c_matrix &b);
    c_matrix &scale(double scale);
    c_vector *apply(const c_vector &v); // new vector = this * v
    double *apply(const c_vector &v, double *rv, int stride); // rv = this * v, return rv
    c_vector &apply(const c_vector &v, c_vector &rv); // rv = this * v, return rv
    int lup_decompose(c_vector **P); // Decompose this to LU setting pivot *P
    c_matrix &lup_get_l(void); // Update this to be just L of LU decomposition
    c_matrix &lup_get_u(void); // Update this to be just U of LU decomposition
    c_matrix *lup_unpivot(const c_vector &P); // unpivot this matrix with vector P
    int lup_inverse(double *data) const; // find unpivoted inverse of this LU matrix using data, result into data
    c_matrix *lup_inverse(void) const; // return new matrix that is inverse of this (doing LU and pivot)
    int lup_invert(const c_vector &P);

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
