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
 * @file          lens_projection.cpp
 * @brief         Mappings of XY image positions to/from quaternions through parametrized lens projections
 *
 */

/*a Documentation
    Camera projection is from world to (-aspect_ratio*width/2,-width/2) to (aspect_ratio*width/2,width/2) such that
    the width*aspect_ratio is the horizontal field-of-view

        offset is 0.0 to 1.0 indicating center to right-hand edge of frame
        for an 'equidistant' lens projection the result is offset/focal_length
        offset*atan(frame_width/(2*focal_length))
        Stereographic angle=2*atan(offset/(2*focal_length))
        Equidistant/equiangular angle=offset/focal_length
        for rectilinear lenses there has to be a distortion applied
        See also http://michel.thoby.free.fr/Fisheye_history_short/Projections/Models_of_classical_projections.html

  canon 20-35mm lens on rebel t2i (aps-c)
  image taken from 57.5 inches
  center should be on 72 inches
  Pictures taken at f/22, shutter about 2 seconds
  img_2333, 2334 focus at 1m
  img_2335, 2336 focus at 3m
  At 35mm img_2333 about 906.462mm across 35.6875" 91 9/16 - 55 14/16 at distance of 1460.5mm
    tan(fov/2) = 0.3103 => fov = 34.47 degrees
  At 35mm img_2336 about 92 13/16 - 56 14/16 = 912.825mm at distance of 1460.5mm
    tan(fov/2) = 0.3125 => fov = 34.70 degrees
  At 20mm img_2335 105 21/74 - (47-271/84) = (middle is at 73 10/16) (right half is 31.658", left is 29.85" - trust right more)
    tan(fov/2) = 0.550 => fov = 57.57 degrees
  These figures tie in well with Bob Atkins calculate (to within half a degree)
  At 20mm img_2335, (x,y) of (0.500,0) = (2592/5184,0) is (105 21/74 - 73 10/16) = 31.658" = 57.57 degrees
  At 20mm img_2335, (x,y) of (0.423,0) = (2194/5184,0) is (100  0/16 - 73 10/16) = 26.375" = 49.28 degrees
  At 20mm img_2335, (x,y) of (0.362,0) = (1876/5184,0) is ( 95 15/16 - 73 10/16) = 22.313" = 42.42 degrees
  At 20mm img_2335, (x,y) of (0.174,0) = ( 900/5184,0) is ( 84  0/16 - 73 10/16) = 10.375" = 20.46 degrees
  At 20mm img_2335, (x,y) of (0.107,0) = ( 555/5184,0) is ( 80  0/16 - 73 10/16) =  6.375" = 12.65 degrees
 */
/*a Includes
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "quaternion.h"
#include "lens_projection.h"
#include <string>

/*a Defines
 */
#define MAX_POLY_COEFFS 8

/*a Types
 */
/*t t_polynomial
 */
typedef struct
{
    int length;
    double coeffs[MAX_POLY_COEFFS];
} t_polynomial;

/*t t_named_polynomial
 */
typedef struct t_named_polynomial
{
    std::string name;
    t_polynomial poly;
    t_polynomial inv_poly;
} t_named_polynomial;

/*a Polynomial functions
 */
/*f polynomial_calc
 */
static double
polynomial_calc(const t_polynomial *poly, double x)
{
    double r = 0;
    for (int i=poly->length-1; i>=0; i--) {
        r = r*x + poly->coeffs[i];
    }
    return r;
}

/*f add_named_polynomial
 */
std::map<std::string, struct t_named_polynomial *>c_lens_projection::named_polynomials;
int
c_lens_projection::add_named_polynomial(const char *name,
                                        int poly_length, const double poly_coeffs[],
                                        int inv_poly_length, const double inv_poly_coeffs[])
{
    std::string name_str = name;
    t_named_polynomial *np;
    if (named_polynomials.count(name_str)>0)
        return -1;

    np = (t_named_polynomial *)malloc(sizeof(t_named_polynomial));
    if (!np) return -1;

    named_polynomials[name] = np;

    np->poly.length = poly_length;
    for (int i=0; (i<poly_length) && (i<MAX_POLY_COEFFS); i++) {
        np->poly.coeffs[i] = poly_coeffs[i];
    }

    np->inv_poly.length = inv_poly_length;
    for (int i=0; (i<inv_poly_length) && (i<MAX_POLY_COEFFS); i++) {
        np->inv_poly.coeffs[i] = inv_poly_coeffs[i];
    }
    return 0;
}

/*a Constructors/destructors/string representation for c_lens_projection
 */
/*f c_lens_projection::c_lens_projection constructor - default is 35mm lens, 36mm image size ('width'), image 1.0x1.0, equidistant lens, pointed at (0,0,1)
 */
c_lens_projection::c_lens_projection(void)
{
    width = 1.0;
    height = 1.0;
    frame_width = 36.0;
    focal_length = 35.0;
    offset_to_angle = &c_lens_projection::offset_to_angle_equidistant;
    angle_to_offset = &c_lens_projection::angle_to_offset_equidistant;
    orientation = c_quaternion<double>::identity();
    if (named_polynomials.count("__linear")==0) {
        static double linear_poly[1]={1.0};
        add_named_polynomial("__linear", 1, linear_poly, 1, linear_poly);
    }
}

/*f c_lens_projection::~c_lens_projection
 */
c_lens_projection::~c_lens_projection()
{
}

/*f c_lens_projection::__str__
 */
void c_lens_projection::__str__(char *buffer, int buf_size) const
{
    snprintf(buffer, buf_size, "(%lf, %lf, %lf, %lf) : ",
             width, height, frame_width, focal_length );
    buffer[buf_size-1] = 0;
    orientation.__str__(buffer+strlen(buffer), buf_size-strlen(buffer) );
}


/*a Methods to configure a projection (orient, set lens,
 */
/*f c_lens_projection::orient - align the lens projection along a quaternion (which provides camera direction and 'up' orientation)
 */
void
c_lens_projection::orient(const c_quaternion<double> &orientation)
{
    this->orientation = orientation;
    this->orientation.normalize();
}

/*f c_lens_projection::set_lens - set frame width, lens focal length, and the projection type of the lens (e.e. rectilinear, stereographic, equidistant, polynomial)
 */
void
c_lens_projection::set_lens(double frame_width, double focal_length, t_lens_projection_type lens_type)
{
    this->frame_width = frame_width;
    this->focal_length = focal_length;
    this->lens_type = lens_type;
    switch (lens_type) {
    case lens_projection_type_rectilinear: {
        offset_to_angle = &c_lens_projection::offset_to_angle_rectilinear;
        angle_to_offset = &c_lens_projection::angle_to_offset_rectilinear;
        break;
    }
    case lens_projection_type_stereographic: {
        offset_to_angle = &c_lens_projection::offset_to_angle_stereographic;
        angle_to_offset = &c_lens_projection::angle_to_offset_stereographic;
        break;
    }
    case lens_projection_type_polynomial: {
        offset_to_angle = &c_lens_projection::offset_to_angle_polynomial;
        angle_to_offset = &c_lens_projection::angle_to_offset_polynomial;
        polynomial = named_polynomials["__linear"];
        break;
    }
    default: {
        offset_to_angle = &c_lens_projection::offset_to_angle_equidistant;
        angle_to_offset = &c_lens_projection::angle_to_offset_equidistant;
        break;
    }
    }
}

/*f c_lens_projection::set_sensor - sets sensor with and height
 */
void
c_lens_projection::set_sensor(double width, double height)
{
    this->width = width;
    this->height = height;
}

/*f c_lens_projection::set_polynomial - select a named polynomial as the projection type for the lens
 */
int
c_lens_projection::set_polynomial(const char *name)
{
    if (named_polynomials.count(name)==0)
        return -1;
    offset_to_angle = &c_lens_projection::offset_to_angle_polynomial;
    angle_to_offset = &c_lens_projection::angle_to_offset_polynomial;
    polynomial = named_polynomials[name];
    return 0;
}

/*a Mapping methods - XY to/from angle through various projections
 */
/*f c_lens_projection::offset_to_angle_equidistant
  fraction_from_center is 0.0 to 1.0 of the frame width (i.e. right-hand edge is 0.5)

  Equidistant/equiangular angle=offset/focal_length
 */
double
c_lens_projection::offset_to_angle_equidistant(double fraction_from_center) const
{
    return fraction_from_center*frame_width/focal_length;
}

/*f c_lens_projection::offset_to_angle_rectilinear
  fraction_from_center is 0.0 to 1.0 of the frame width (i.e. right-hand edge is 0.5)

  Rectilinear angle = atan(offset,focal_length)
 */
double c_lens_projection::offset_to_angle_rectilinear(double fraction_from_center) const
{
    return atan2(fraction_from_center*frame_width, focal_length);
}

/*f c_lens_projection::offset_to_angle_stereographic
  fraction_from_center is 0.0 to 1.0 of the frame width (i.e. right-hand edge is 0.5)

  Stereographic angle=2*atan(offset/(2*focal_length))
 */
double c_lens_projection::offset_to_angle_stereographic(double fraction_from_center) const
{
    return 2*atan2(fraction_from_center*frame_width, 2*focal_length);
}

/*f c_lens_projection::offset_to_angle_polynomial
  fraction_from_center is 0.0 to 1.0 of the frame width (i.e. right-hand edge is 0.5)

  Polynomial angle=sum(ai.x^i)(offset/(frame_width/focal_length))
 */
double c_lens_projection::offset_to_angle_polynomial(double fraction_from_center) const
{
    double x = fraction_from_center*frame_width / focal_length;
    double angle = polynomial_calc(&(polynomial->poly), x);
    return angle;
}

/*f c_lens_projection::angle_to_offset_equidistant
  Must be the inverse of offset_to_angle
 */
double c_lens_projection::angle_to_offset_equidistant(double angle) const
{
    return angle*focal_length/frame_width;
}

/*f c_lens_projection::angle_to_offset_rectilinear
  Must be the inverse of offset_to_angle
 */
double c_lens_projection::angle_to_offset_rectilinear(double angle) const
{
    return tan(angle)*focal_length/frame_width;
}

/*f c_lens_projection::angle_to_offset_stereographic
  Must be the inverse of offset_to_angle
 */
double c_lens_projection::angle_to_offset_stereographic(double angle) const
{
    return 2*tan(angle/2)*focal_length/frame_width;
}

/*f c_lens_projection::angle_to_offset_polynomial
  Must be the inverse of offset_to_angle
 */
double c_lens_projection::angle_to_offset_polynomial(double angle) const
{
    double x= polynomial_calc(&(polynomial->inv_poly), angle)*focal_length/frame_width;
    return x;
}

/*a Mapping methods - XY to/from quaternion ('up' vector is unpredictable)
 */
/*f c_lens_projection::xy_to_roll_yaw
  Convert from (x,y) in (frame_width, frame_height) to [roll(angle) yaw(angle)][0 0 1]
 */
void c_lens_projection::xy_to_roll_yaw(const double xy[2], double ry[2]) const
{
    double r = sqrt(xy[0]*xy[0]/width/width+xy[1]*xy[1]/height/height);
    double roll = atan2(xy[1]*width, xy[0]*height);
    double yaw  = (this->*offset_to_angle)(r); 
    ry[0] = roll;
    ry[1] = yaw;
}

/*f c_lens_projection::roll_yaw_to_xy
  Convert [roll(angle) yaw(angle)][0 0 1] to (x,y) in lens units
 */
void c_lens_projection::roll_yaw_to_xy(const double ry[2], double xy[2]) const
{
    double r = (this->*angle_to_offset)(ry[1]);
    xy[0] = width  * r * cos(ry[0]);
    xy[1] = height * r * sin(ry[0]);
}

/*f c_lens_projection::orientation_of_xy - return a quaternion that maps (0,0,1) to where XY is on the image given this projection
  Convert (x,y) in image units (e.g. pixels) to full orientation including camera orientation
 */
c_quaternion<double> c_lens_projection::orientation_of_xy(const double xy[2]) const
{
    double ry[2];
    xy_to_roll_yaw(xy, ry);
    return this->orientation * c_quaternion<double>::roll(ry[0],0) * c_quaternion<double>::yaw(ry[1],0);
}

/*f c_lens_projection::xy_of_orientation - find XY on image that is 
  Convert orientation quaternion (which maps (0,0,1) to the (x,y) in image units (e.g. pixels)
 */
void c_lens_projection::xy_of_orientation(const c_quaternion<double> *orientation, double xy[2]) const
{
    c_quaternion<double> q = c_quaternion<double>(this->orientation);
    c_quaternion<double> qc, mapped_001;
    double rxyz[4];
    double roll, yaw;
    double ry[2];

    q.conjugate();          // invert this camera orientation
    q = q * (*orientation); // add the input orientation

    q.normalize();
    // q is now input orientation - camera orientation
    // Find how (0,0,1) is mapped through q
    qc = c_quaternion<double>(q);
    qc.conjugate();
    mapped_001 = q * c_quaternion<double>::rijk(0,0,0,1) * qc;

    // recover as rxyz[1..3]
    mapped_001.get_rijk(rxyz);

    yaw  = acos(rxyz[3]); // yaw that gets mapped z back to 1
    roll = atan2(rxyz[1], rxyz[2]); // to get (x,y) mapped to (0,0) after z is mapped back to 1
    ry[0] = -roll;
    ry[1] = -yaw;
    roll_yaw_to_xy(ry, xy);
    return;
}

/*a External static functions
 */
/*f c_lens_projection::xy_b_of_a - map XY in projection A to XY in projection B
 */
void
c_lens_projection::xy_b_of_a(const c_lens_projection *a, const c_lens_projection *b, const double xy_a[2], double xy_b[2])
{
    auto wq = a->orientation_of_xy(xy_a);
    b->xy_of_orientation(&wq, xy_b);
}

/*f lens_projection_type
 */
t_lens_projection_type
c_lens_projection::lens_projection_type(const char *name)
{
    t_lens_projection_type lp_type;
    lp_type = lens_projection_type_equidistant;
    if (name) {
        if (!strcmp(name,"rectilinear")) {
            lp_type = lens_projection_type_rectilinear;
        }
        if (!strcmp(name,"stereographic")) {
            lp_type = lens_projection_type_stereographic;
        }
        if (!strcmp(name,"polynomial")) {
            lp_type = lens_projection_type_polynomial;
        }
    }
    return lp_type;
}

