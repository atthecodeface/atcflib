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
template <typename T> class c_quaternion;
template <typename T> class c_vector
{
    T _internal_coords[VECTOR_MAX_LENGTH];
    T *_coords;
    int _coords_must_be_freed;
    int _length;
    int _max_length;
    int _stride;
public:
    c_vector &operator*=(T real);
    c_vector &operator/=(T real);
    inline c_vector operator*(T rhs) { c_vector lhs=*this; lhs *= rhs; return lhs; }
    inline c_vector operator/(T rhs) { c_vector lhs=*this; lhs /= rhs; return lhs; }

    inline c_vector &operator=(const c_vector &other)  {return assign(other);}
    inline c_vector &operator+=(const c_vector &other) {return add_scaled(other, 1.0); }
    inline c_vector &operator-=(const c_vector &other) {return add_scaled(other, -1.0); }
    inline c_vector operator+(const c_vector &rhs) const { c_vector lhs=*this; lhs += rhs; return lhs; }
    inline c_vector operator-(const c_vector &rhs) const { c_vector lhs=*this; lhs -= rhs; return lhs; }
    inline c_vector operator-(void) const { c_vector lhs=*this; for (int i=0; i<lhs._length; i++) lhs._coords[i*lhs._stride]*=-1; return lhs; }

    void init(void);
    int set_length(int length, int allow_reallocate);
    c_vector(const c_vector &vector);
    c_vector(int length);
    c_vector(void);
    c_vector(const class c_quaternion<T> &quat);
    c_vector(int length, int stride, T *coords); // refer to data
    ~c_vector(void);
    c_vector *copy(void) const;

    inline int length(void) const {return _length;}
    inline int stride(void) const {return _stride;}
    inline const T *coords(int *stride) const {if (stride) {*stride=_stride;} return &(_coords[0]);};
    inline T *coords_to_set(int *stride) {if (stride) {*stride=_stride;} return &(_coords[0]);};
    inline void set(int n, T v) {_coords[n*_stride]=v;};
    inline T get(int n) const {return _coords[n*_stride];};
    c_vector<T> &assign(const c_vector<T> &other);
    c_vector<T> &assign(int length, int stride, const T *coords); // copy data
    c_vector<T> &add_scaled(const c_vector<T> &other, T scale);
    T modulus_squared(void) const;
    T modulus(void) const;
    c_vector<T> &scale(T scale);
    c_vector<T> &normalize(void);
    T dot_product(const c_vector<T> &other) const;
    c_vector<T> *cross_product3(const c_vector<T> &other) const;
    c_vector<T> *cross_product3(const c_vector<T> &other, c_vector<T> *result) const;

    // axis_angle_to_v works for 3-vectors.
    // It creates a new vector to be the axis of rotation required to
    // get from 'this' to 'other' (i.e. this <= unit(this x other))
    // and sets the angle; it returns 'this'
    c_vector<T> *angle_axis_to_v3(const c_vector<T> &other, T *cos_angle, T *sin_angle) const;
    c_vector<T> *angle_axis_to_v3(const c_vector<T> &other, T *cos_angle, T *sin_angle, c_vector<T> *result) const;
    class c_quaternion<T> *angle_axis_to_v3(const c_vector<T> &other) const;
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
