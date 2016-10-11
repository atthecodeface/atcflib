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

/*a Types
 */

/*a c_lens_projection methods
 */
/*f c_lens_projection::c_lens_projection
 */
c_lens_projection::c_lens_projection(void)
{
    width = 1.0;
    height = 1.0;
    frame_width = 36.0;
    focal_length = 35.0;
    offset_to_angle = &c_lens_projection::offset_to_angle_equidistant;
    angle_to_offset = &c_lens_projection::angle_to_offset_equidistant;
    orientation = c_quaternion::identity();
}

/*f c_lens_projection::~c_lens_projection
 */
c_lens_projection::~c_lens_projection()
{
}

/*f c_lens_projection::orient
 */
void c_lens_projection::orient(const c_quaternion &orientation)
{
    this->orientation = orientation;
    this->orientation.normalize();
}

/*f c_lens_projection::set_lens
 */
void c_lens_projection::set_lens(double frame_width, double focal_length, t_lens_projection_type lens_type)
{
    this->frame_width = frame_width;
    this->focal_length = focal_length;
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
    default: {
        offset_to_angle = &c_lens_projection::offset_to_angle_equidistant;
        angle_to_offset = &c_lens_projection::angle_to_offset_equidistant;
        break;
    }
    }
}

void c_lens_projection::set_sensor(double width, double height)
{
    this->width = width;
    this->height = height;
}

/*f c_lens_projection::offset_to_angle_equidistant
  fraction_from_center is 0.0 to 1.0 of the frame width (i.e. right-hand edge is 0.5)

  Equidistant/equiangular angle=offset/focal_length
 */
double c_lens_projection::offset_to_angle_equidistant(double fraction_from_center) const
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

/*f c_lens_projection::xy_to_roll_yaw
  Convert from (x,y) in (frame_width, frame_height) to [roll(angle) yaw(angle)][0 0 1]
 */
void c_lens_projection::xy_to_roll_yaw(const double xy[2], double ry[2]) const
{
    double r = sqrt(xy[0]*xy[0]+xy[1]*xy[1]);
    double roll = atan2(xy[1], xy[0]);
    double yaw  = (this->*offset_to_angle)(r/width); 
    ry[0] = roll;
    ry[1] = yaw;
}

/*f c_lens_projection::roll_yaw_to_xy
  Convert [roll(angle) yaw(angle)][0 0 1] to (x,y) in lens units
 */
void c_lens_projection::roll_yaw_to_xy(const double ry[2], double xy[2]) const
{
    double r = width * (this->*angle_to_offset)(ry[1]);
    xy[0] = r*cos(ry[0]);
    xy[1] = r*sin(ry[0]);
}

/*f c_lens_projection::orientation_of_xy
  Convert (x,y) in image units (e.g. pixels) to full orientation including camera orientation
 */
c_quaternion c_lens_projection::orientation_of_xy(const double xy[2]) const
{
    double ry[2];
    xy_to_roll_yaw(xy, ry);
    return this->orientation * c_quaternion::roll(ry[0],0) * c_quaternion::yaw(ry[1],0);
}

/*f c_lens_projection::xy_of_orientation
  Convert orientation quaternion (which maps (0,0,1) to the (x,y) in image units (e.g. pixels)
 */
void c_lens_projection::xy_of_orientation(const c_quaternion *orientation, double xy[2]) const
{
    c_quaternion q = c_quaternion(this->orientation);
    c_quaternion qc;
    c_quaternion mapped_001;
    double rxyz[4];
    double roll, yaw;
    double ry[2];

    q.conjugate();          // invert this camera orientation
    q = q * (*orientation); // add the input orientation

    q.normalize();
    // q is now input orientation - camera orientation
    // Find how (0,0,1) is mapped through q
    qc = c_quaternion(q);
    qc.conjugate();
    mapped_001 = q * c_quaternion::rijk(0,0,0,1) * qc;

    // recover as rxyz[1..3]
    mapped_001.get_rijk(rxyz);

    yaw  = acos(rxyz[3]); // yaw that gets mapped z back to 1
    roll = atan2(rxyz[1], rxyz[2]); // to get (x,y) mapped to (0,0) after z is mapped back to 1
    ry[0] = -roll;
    ry[1] = -yaw;
    roll_yaw_to_xy(ry, xy);
    return;
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


/*f c_lens_projection::xy_b_of_a
 */
void c_lens_projection::xy_b_of_a(const c_lens_projection *a, const c_lens_projection *b, const double xy_a[2], double xy_b[2])
{
    c_quaternion wq = a->orientation_of_xy(xy_a);
    b->xy_of_orientation(&wq, xy_b);
}


/*f lens_projection_type
 */
t_lens_projection_type c_lens_projection::lens_projection_type(const char *name)
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
    }
    return lp_type;
}

/*a Rest
 */
/*
src = camera(width=5184.0, height=3456.0, focal_length=20.0)
dst = camera(width=1024.0, height=1024.04, focal_length=100.0)
dst = camera(width=5184.0, height=3456.0, focal_length=20.0)

if False:
    print src.xy_to_roll_yaw((0,0))
    print src.xy_to_roll_yaw((2592,0))
    print src.xy_to_roll_yaw((0,1728))
    print src.roll_yaw_to_xy((math.radians(90),0.1))
    print src.roll_yaw_to_xy((math.radians(180),0.1))
    print src.roll_yaw_to_xy((math.radians(45),0.1))
    print src.roll_yaw_to_xy(src.xy_to_roll_yaw((100,200)))

# Want src_orientation and dst_orientation to be quaternions giving the center of the image
# Then I want to know src xy for any dst xy
# Note that dst xy is (roll,yaw)=dst.xy_to_roll_yaw(xy)
# How about we want to know the (x,y,z) of dst xy.
# This should be something like dst_orientation*roll(yaw(dst.xy_to_roll_yaw(xy))) applied to (0,0,1)
# Then we can apply src_orientation' to this to get an (x,y,z) relative to the source
# This has to be converted to a roll,yaw, then to an (x,y)

def conjugation(q,p):
    qc = q.copy().conjugate()
    print p[0], p[1], p[2]
    pq = quaternion(r=0,i=p[0],j=p[1],k=p[2])
    r = q*pq*qc
    print r
    return r.get()[1:]

# Orientation assumes that the camera axis is the z-axis with x-axis up, and orientation is applied to that
dst_orientation = quaternion.yaw(-60,degrees=True) * quaternion.pitch(-30,degrees=True)
src_orientation = quaternion.yaw(-60,degrees=True) * quaternion.pitch(-30,degrees=True)
#dst_orientation = quaternion.of_spherical_polar(0,10,degrees=True) # looking at 10 degrees 'to the right'
#src_orientation = quaternion.of_spherical_polar(0,0,degrees=True)  # looking straight on
xy=(100.0,0) # 100 pixels to the right of center out of 512.0 to the right of center - i.e. center+20%
xy=(0,100.0) # 100 pixels above the center out of 512.0
dst_ry = dst.xy_to_roll_yaw(xy)
print dst_ry
q = dst_orientation * quaternion.roll(dst_ry[0]) * quaternion.yaw(dst_ry[1])
q.repr_fmt = "euler"
print q
q = src_orientation.copy().conjugate() * q
print q
mapped_xyz = conjugation(q,(0,0,1))
print mapped_xyz
yaw = math.acos(mapped_xyz[2])
roll = math.atan2(mapped_xyz[0], mapped_xyz[1]) # [0],[1] because X is up in quaternion universe
src_ry = (-roll,-yaw) # - because we are inverting the transformation
print src_ry
print src.roll_yaw_to_xy(src_ry)

#q=quaternion.roll(30,degrees=True)
#q=quaternion.pitch(30,degrees=True) * quaternion.roll(30,degrees=True)
#q=quaternion.pitch(30,degrees=True)
#conjugation(q,(0,0,1))
*/
