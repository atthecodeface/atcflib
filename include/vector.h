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
 * @file          vector.h
 * @brief         Vectors and methods thereon
 *
 */

/*a Wrapper
 */
#ifdef __INC_VECTOR
#else
#define __INC_VECTOR

/*a Defines
 */
#define VECTOR_MAX_LENGTH 8

/*a Types
 */
class c_vector
{
    double _internal_coords[VECTOR_MAX_LENGTH];
    double *_coords;
    int _coords_must_be_freed;
    int _length;
    int _stride;
public:
    c_vector &operator*=(double real);
    c_vector &operator/=(double real);
    inline c_vector operator*(double rhs) { c_vector lhs=*this; lhs *= rhs; return lhs; }
    inline c_vector operator/(double rhs) { c_vector lhs=*this; lhs /= rhs; return lhs; }

    c_vector &operator=(const c_vector &other);
    c_vector &operator+=(const c_vector &other);
    c_vector &operator-=(const c_vector &other);
    inline c_vector operator+(const c_vector &rhs) const { c_vector lhs=*this; lhs += rhs; return lhs; }
    inline c_vector operator-(const c_vector &rhs) const { c_vector lhs=*this; lhs -= rhs; return lhs; }
    inline c_vector operator-(void) const { c_vector lhs=*this; for (int i=0; i<lhs._length; i++) lhs._coords[i*lhs._stride]*=-1; return lhs; }

    c_vector(const c_vector &vector);
    c_vector(int length);
    c_vector(void);
    c_vector(const class c_quaternion &quat);
    c_vector(int length, const double *coords);
    ~c_vector(void);
    c_vector *copy(void) const;

    inline int length(void) const {return _length;}
    inline const double *coords(int *stride=NULL) const {if (stride) {*stride=_stride;} return &(_coords[0]);};
    inline double *coords_to_set(int *stride=NULL) {if (stride) {*stride=_stride;} return &(_coords[0]);};
    inline void set(int n, double v) {_coords[n*_stride]=v;};
    inline double value(int n) const {return _coords[n*_stride];};
    c_vector &assign(const c_vector &other);
    c_vector &add_scaled(const c_vector &other, double scale);
    double modulus_squared(void) const;
    double modulus(void) const;
    c_vector &scale(double scale);
    c_vector &normalize(void);
    double dot_product(const c_vector &other) const;
    c_vector *cross_product3(const c_vector &other) const;

    // axis_angle_to_v works for 3-vectors.
    // It creates a new vector to be the axis of rotation required to
    // get from 'this' to 'other' (i.e. this <= unit(this x other))
    // and sets the angle; it returns 'this'
    c_vector *angle_axis_to_v3(const c_vector &other, double *cos_angle, double *sin_angle) const;
    class c_quaternion *angle_axis_to_v3(const c_vector &other) const;
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
