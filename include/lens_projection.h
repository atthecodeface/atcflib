/** Copyright (C) 2016-17,  Gavin J Stark.  All rights reserved.
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
 * @file          lens_projection.h
 * @brief         Camera lens projection handling including orientation
 *
 */

/*a Wrapper
 */
#ifdef __INC_LENS_PROJECTION
#else
#define __INC_LENS_PROJECTION

/*a Includes
 */
#include <map>
#include <string>
#include "quaternion.h"

/*t t_lens_projection_type
 */
typedef enum
{
    lens_projection_type_equidistant   = 0,
    lens_projection_type_equiangular   = 0, // same as equidistant
    lens_projection_type_stereographic = 1,
    lens_projection_type_rectilinear   = 2,
    lens_projection_type_polynomial    = 3
} t_lens_projection_type;

/*f c_lens_projection
 * Lens projection
 */
class c_lens_projection
{
private:
    typedef double (c_lens_projection::*f_offset_to_angle)(double fraction_from_center) const;
    typedef double (c_lens_projection::*f_angle_to_offset)(double angle) const;
    double offset_to_angle_equidistant(double fraction_from_center) const;
    double offset_to_angle_rectilinear(double fraction_from_center) const;
    double offset_to_angle_stereographic(double fraction_from_center) const;
    double offset_to_angle_polynomial(double fraction_from_center) const;
    double angle_to_offset_equidistant(double angle) const;
    double angle_to_offset_rectilinear(double angle) const;
    double angle_to_offset_stereographic(double angle) const;
    double angle_to_offset_polynomial(double angle) const;

    double width, height; // in 'sensor' or 'image' units (e.g. pixels)
    double frame_width;   // in lens units, same as focal length
    double focal_length;  // in lens units, same as frame_width
    t_lens_projection_type lens_type;
    c_quaternion<double> orientation;
    f_offset_to_angle offset_to_angle;
    f_angle_to_offset angle_to_offset;
    struct t_named_polynomial *polynomial;

    static std::map<std::string, struct t_named_polynomial *>named_polynomials;
public:
    c_lens_projection(void);
    ~c_lens_projection();

    static t_lens_projection_type lens_projection_type(const char *name);
    static void xy_b_of_a(const c_lens_projection *a, const c_lens_projection *b, const double xy_a[2], double xy_b[2]);
    static int add_named_polynomial(const char *name,
                                    int poly_length, const double poly_coeffs[],
                                    int inv_poly_length, const double inv_poly_coeffs[]);
    static int remove_named_polynomial(const char *name);

    inline double get_frame_width(void) { return frame_width; }
    inline double get_focal_length(void) { return focal_length; }
    inline t_lens_projection_type get_lens_type(void) { return lens_type; }
    inline void   get_sensor(double wh[2]) { wh[0]=width; wh[1]=height; }
    inline double get_sensor_width(void)  { return width; }
    inline double get_sensor_height(void) { return height; }
    inline c_quaternion<double> &get_orientation(void) { return orientation; }
    void orient(const c_quaternion<double> &orientation);
    void set_lens(double frame_width, double focal_length, t_lens_projection_type lens_type);
    void set_sensor(double width, double height);
    int set_polynomial(const char *name);
    void xy_to_roll_yaw(const double xy[2], double ry[2]) const;
    void roll_yaw_to_xy(const double ry[2], double xy[2]) const;
    c_quaternion<double> orientation_of_xy(const double xy[2]) const;
    void xy_of_orientation(const c_quaternion<double> *orientation, double xy[2]) const;

    void __str__(char *buffer, int buf_size) const;
};

/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
