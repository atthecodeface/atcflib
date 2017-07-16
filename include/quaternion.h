/** Copyright (C) 2016-2017,  Gavin J Stark.  All rights reserved.
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
 * @file          quaternion.h
 * @brief         Quaternions and methods thereon
 *
 */

/*a Wrapper
 */
#ifdef __INC_QUATERNION
#else
#define __INC_QUATERNION

/*a Includes
 */

/*a Types
 */
template <typename T> class c_vector;
template <typename T> class c_quaternion
{
    struct
    {
        T r, i, j, k;
    } quat;
public:
    static inline c_quaternion identity(void) { c_quaternion q; q=1.0; return q; }
    static inline c_quaternion rijk(T r, T i, T j, T k) { c_quaternion q=c_quaternion(r,i,j,k); return q; }
    static inline c_quaternion roll( T angle, int degrees) { c_quaternion q; q.from_euler(angle,0,0,degrees); return q; }
    static inline c_quaternion pitch(T angle, int degrees) { c_quaternion q; q.from_euler(0,angle,0,degrees); return q; }
    static inline c_quaternion yaw(  T angle, int degrees) { c_quaternion q; q.from_euler(0,0,angle,degrees); return q; }
    static inline c_quaternion of_euler(T r, T p, T y, int degrees) { c_quaternion q; q.from_euler(r,p,y,degrees); return q; }
    static inline c_quaternion of_rotation(T angle, const T axis[3], int degrees) { c_quaternion q; q.from_rotation(angle, axis, degrees); return q; }
    static inline c_quaternion of_rotation(T cos_angle, T sin_angle, const T *axis, int axis_stride=1) { c_quaternion q; q.from_rotation(cos_angle, sin_angle, axis, axis_stride); return q; }

    c_quaternion &operator=(T real);
    c_quaternion &operator+=(T real);
    c_quaternion &operator-=(T real);
    c_quaternion &operator*=(T real);
    c_quaternion &operator/=(T real);
    inline c_quaternion operator+(T rhs) { c_quaternion lhs=*this; lhs += rhs; return lhs; }
    inline c_quaternion operator-(T rhs) { c_quaternion lhs=*this; lhs -= rhs; return lhs; }
    inline c_quaternion operator*(T rhs) { c_quaternion lhs=*this; lhs *= rhs; return lhs; }
    inline c_quaternion operator/(T rhs) { c_quaternion lhs=*this; lhs /= rhs; return lhs; }

    c_quaternion &operator=(const c_quaternion &other);
    c_quaternion &operator+=(const c_quaternion &other);
    c_quaternion &operator-=(const c_quaternion &other);
    c_quaternion &operator*=(const c_quaternion &other);
    c_quaternion &operator/=(const c_quaternion &other);
    inline c_quaternion operator+(const c_quaternion &rhs) const { c_quaternion lhs=*this; lhs += rhs; return lhs; }
    inline c_quaternion operator-(const c_quaternion &rhs) const { c_quaternion lhs=*this; lhs -= rhs; return lhs; }
    inline c_quaternion operator-(void) const { c_quaternion lhs=*this; lhs.quat.r=-lhs.quat.r;lhs.quat.i=-lhs.quat.i;lhs.quat.j=-lhs.quat.j;lhs.quat.k=-lhs.quat.k; return lhs; }
    inline c_quaternion operator~(void) const { c_quaternion lhs=*this; lhs.quat.i=-lhs.quat.i;lhs.quat.j=-lhs.quat.j;lhs.quat.k=-lhs.quat.k; return lhs; }
    inline c_quaternion operator*(const c_quaternion &rhs) const { c_quaternion lhs=*this; lhs *= rhs; return lhs; }
    inline c_quaternion operator/(const c_quaternion &rhs) const { c_quaternion lhs=*this; lhs /= rhs; return lhs; }

    c_quaternion(const c_quaternion &quat);
    c_quaternion(void);
    c_quaternion(const class c_vector<T> &vector);
    c_quaternion(T r, T i, T j, T k);
    c_quaternion *copy(void) const;

    inline T r(void) const {return quat.r;};
    inline T i(void) const {return quat.i;};
    inline T j(void) const {return quat.j;};
    inline T k(void) const {return quat.k;};
    c_quaternion &conjugate(void);
    c_quaternion &reciprocal(void);
    c_quaternion &add_scaled(const c_quaternion &other, T scale);
    T modulus_squared(void) const;
    T modulus(void) const;
    c_quaternion &scale(T scale);
    c_quaternion &normalize(void);
    c_quaternion &multiply(const c_quaternion &other, int premultiply=0);

    c_quaternion &from_euler(T roll, T pitch, T yaw, int degrees=0);
    c_quaternion &lookat_aeronautic(const c_vector<T> &at, const c_vector<T> &up);
    c_quaternion &lookat_aeronautic(const T at[3], const T up[3]);
    c_quaternion &lookat_graphics(const c_vector<T> &at, const c_vector<T> &up);
    c_quaternion &lookat_graphics(const T xyz[3], const T up[3]);
    c_quaternion &from_rotation(T angle, const T axis[3], int degrees=0);
    c_quaternion &from_rotation(T cos_angle, T sin_angle, const T *axis, int axis_stride);
    c_quaternion &from_rotation(T cos_angle, T sin_angle, const c_vector<T> &vector);
    T as_rotation(T axis[3]) const;
    T as_rotation(class c_vector<T> &vector) const;
    void as_rotation(c_vector<T> &vector, T *cos, T *sin) const;
    void as_euler(T rpy[3]) const;
    void get_rijk(T rijk[4]) const;
    // axis_angle gives the quaternion for a great-circle (minimum rotation angle)
    // required to transform a vector ('this' applied to vector) to another vector
    // ('other' applied to that vector).
    // The axis will be perpendicular to the axes of rotation represented by 'this'
    // and 'other'
    c_quaternion *rotate_vector(const class c_vector<T> &vector) const;
    void          rotate_vector(c_vector<T> *vector) const;
    c_quaternion *angle_axis(const c_quaternion<T> &other, class c_vector<T> &vector) const;
    T distance_to(const c_quaternion<T> &other) const;
    char *__str__(char *buffer, int buf_size) const;
    
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
