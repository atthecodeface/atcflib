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
    inline c_quaternion operator+(const c_quaternion &rhs) { c_quaternion lhs=*this; lhs += rhs; return lhs; }
    inline c_quaternion operator-(const c_quaternion &rhs) { c_quaternion lhs=*this; lhs -= rhs; return lhs; }
    inline c_quaternion operator*(const c_quaternion &rhs) { c_quaternion lhs=*this; lhs *= rhs; return lhs; }
    inline c_quaternion operator/(const c_quaternion &rhs) { c_quaternion lhs=*this; lhs /= rhs; return lhs; }

    c_quaternion(const c_quaternion *quat);
    c_quaternion(void);
    c_quaternion(double r, double i, double j, double k);
    c_quaternion *copy(void);

    inline double r(void) {return quat.r;};
    inline double i(void) {return quat.i;};
    inline double j(void) {return quat.j;};
    inline double k(void) {return quat.k;};
    c_quaternion *conjugate(void);
    c_quaternion *reciprocal(void);
    c_quaternion *add_scaled(const c_quaternion *other, double scale);
    double modulus_squared(void);
    double modulus(void);
    c_quaternion *scale(double scale);
    c_quaternion *normalize(void);
    c_quaternion *multiply(const c_quaternion *other);

    c_quaternion *from_euler(double roll, double pitch, double yaw, int degrees);
    c_quaternion *from_rotation(double angle, double axis[3], int degrees);
    double as_rotation(double axis[3]);
    void get_rijk(double rijk[4]);
};

/*a External functions
 */
/*f filter_from_string
  filter string must be:
  <filter type>:<filename>(<options list>)[&<uniform_name>=<float>]
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
