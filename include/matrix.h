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
#include <stdio.h>
#include "vector.h"
#include "quaternion.h"

/*a Defines
 */

/*a Types
 */
template <typename T> class c_vector;
template <typename T> class c_matrix
{
    T *_values;
    int _nrows;
    int _ncols;
    int _row_stride;
    int _col_stride;
    int _values_must_be_freed;
public:
    c_matrix &operator*=(T real);
    c_matrix &operator/=(T real);
    inline c_matrix operator*(T rhs) { c_matrix lhs=*this; lhs *= rhs; return lhs; }
    inline c_matrix operator/(T rhs) { c_matrix lhs=*this; lhs /= rhs; return lhs; }

    c_matrix &operator=(const c_matrix &other);
    c_matrix &operator+=(const c_matrix &other);
    c_matrix &operator-=(const c_matrix &other);
    inline c_matrix operator+(const c_matrix &rhs) const { c_matrix lhs=*this; lhs += rhs; return lhs; }
    inline c_matrix operator-(const c_matrix &rhs) const { c_matrix lhs=*this; lhs -= rhs; return lhs; }
    inline c_matrix operator-(void) const { c_matrix lhs=*this; for (int i=0; i<_nrows*_ncols; i++) lhs._values[i]*=-1; return lhs; }
    inline c_matrix operator*(const c_matrix &rhs) const { c_matrix *r=new c_matrix(*this,rhs); return *r; }

    void init(void);
    ~c_matrix(void);
    c_matrix(const c_matrix &matrix);
    c_matrix(int nrows, int ncols);
    c_matrix(const c_matrix &a, const c_matrix &b);
    void set_size(int nrows, int ncols);
    c_matrix(void);
    c_matrix(int nrows, int ncols, const T *values);
    c_matrix(int nrows, int ncols, T *values, int row_stride, int col_stride);
    c_matrix *copy(void) const;

    inline int nrows(void) {return _nrows;}
    inline int ncols(void) {return _ncols;}
    inline int row_stride(void) {return _row_stride;}
    inline int col_stride(void) {return _col_stride;}
    inline T *set_values(T *values) {_values=values;return _values;}
    inline const T *values(void) const {return _values;};
    inline void set(int r, int c, T v) {_values[r*_row_stride+c*_col_stride]=v;};
    inline T get(int r, int c) const {return _values[r*_row_stride+c*_col_stride];};

    void get_row(int row, c_vector<T> *result) const;
    void get_column(int col, c_vector<T> *result) const;
    c_vector<T> *get_row(int row) const;
    c_vector<T> *get_column(int col) const;
    c_matrix &set_identity(void);
    c_matrix &transpose_data(void);
    c_matrix &transpose_stride(void);
    c_matrix &add_scaled(const c_matrix<T> &other, T scale);
    c_matrix &multiply(const c_matrix<T> &a, const c_matrix &b);
    c_matrix &scale(T scale);
    c_vector<T> *apply(const c_vector<T> &v); // new vector = this * v
    T *apply(const c_vector<T> &v, T *rv, int stride); // rv = this * v, return rv
    c_vector<T> &apply(const c_vector<T> &v, c_vector<T> &rv); // rv = this * v, return rv
    c_matrix<T> &set_from_quaternion(const c_quaternion<T> &q);
    int lup_decompose(c_vector<T> **P); // Decompose this to LU setting pivot *P
    c_matrix &lup_get_l(void); // Update this to be just L of LU decomposition
    c_matrix &lup_get_u(void); // Update this to be just U of LU decomposition
    c_matrix *lup_unpivot(const c_vector<T> &P); // unpivot this matrix with vector P
    int lup_inverse(T *data) const; // find unpivoted inverse of this LU matrix using data, result into data
    c_matrix *lup_inverse(void) const; // return new matrix that is inverse of this (doing LU and pivot)
    int lup_invert(const c_vector<T> &P);

    void __str__(char *buffer, int buf_size) const;
    void __display__(FILE *f) const;
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
