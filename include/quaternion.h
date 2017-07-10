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
struct quat
{
    double r, i, j, k;
};

class c_quaternion
{
    struct quat quat;
public:
    static inline c_quaternion identity(void) { c_quaternion q; q=1.0; return q; }
    static inline c_quaternion rijk(double r, double i, double j, double k) { c_quaternion q=c_quaternion(r,i,j,k); return q; }
    static inline c_quaternion roll( double angle, int degrees) { c_quaternion q; q.from_euler(angle,0,0,degrees); return q; }
    static inline c_quaternion pitch(double angle, int degrees) { c_quaternion q; q.from_euler(0,angle,0,degrees); return q; }
    static inline c_quaternion yaw(  double angle, int degrees) { c_quaternion q; q.from_euler(0,0,angle,degrees); return q; }
    static inline c_quaternion of_euler(double r, double p, double y, int degrees) { c_quaternion q; q.from_euler(r,p,y,degrees); return q; }
    static inline c_quaternion of_rotation(double angle, const double axis[3], int degrees) { c_quaternion q; q.from_rotation(angle, axis, degrees); return q; }
    static inline c_quaternion of_rotation(double cos_angle, double sin_angle, const double *axis) { c_quaternion q; q.from_rotation(cos_angle, sin_angle, axis); return q; }

    c_quaternion &operator=(double real);
    c_quaternion &operator+=(double real);
    c_quaternion &operator-=(double real);
    c_quaternion &operator*=(double real);
    c_quaternion &operator/=(double real);
    inline c_quaternion operator+(double rhs) { c_quaternion lhs=*this; lhs += rhs; return lhs; }
    inline c_quaternion operator-(double rhs) { c_quaternion lhs=*this; lhs -= rhs; return lhs; }
    inline c_quaternion operator*(double rhs) { c_quaternion lhs=*this; lhs *= rhs; return lhs; }
    inline c_quaternion operator/(double rhs) { c_quaternion lhs=*this; lhs /= rhs; return lhs; }

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
    c_quaternion(const class c_vector &vector);
    c_quaternion(double r, double i, double j, double k);
    c_quaternion *copy(void) const;

    inline double r(void) const {return quat.r;};
    inline double i(void) const {return quat.i;};
    inline double j(void) const {return quat.j;};
    inline double k(void) const {return quat.k;};
    c_quaternion &conjugate(void);
    c_quaternion &reciprocal(void);
    c_quaternion &add_scaled(const c_quaternion &other, double scale);
    double modulus_squared(void) const;
    double modulus(void) const;
    c_quaternion &scale(double scale);
    c_quaternion &normalize(void);
    c_quaternion &multiply(const c_quaternion &other, int premultiply=0);

    c_quaternion &from_euler(double roll, double pitch, double yaw, int degrees=0);
    c_quaternion &lookat_aeronautic(const double at[3], const double up[3]);
    c_quaternion &lookat_graphics(const double xyz[3], const double up[3]);
    c_quaternion &from_rotation(double angle, const double axis[3], int degrees=0);
    c_quaternion &from_rotation(double cos_angle, double sin_angle, const double *axis);
    double as_rotation(double axis[3]) const;
    double as_rotation(class c_vector &vector) const;
    void as_rotation(c_vector &vector, double *cos, double *sin) const;
    void as_euler(double rpy[3]) const;
    void get_rijk(double rijk[4]) const;
    // axis_angle gives the quaternion for a great-circle (minimum rotation angle)
    // required to transform a vector ('this' applied to vector) to another vector
    // ('other' applied to that vector).
    // The axis will be perpendicular to the axes of rotation represented by 'this'
    // and 'other'
    c_quaternion *rotate_vector(const class c_vector &vector) const;
    void          rotate_vector(c_vector *vector) const;
    c_quaternion *angle_axis(const c_quaternion &other, class c_vector &vector) const;
    double distance_to(const c_quaternion &other) const;
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
