#!/usr/bin/env python
#PYTHONPATH=`pwd`/../python:`pwd`/..:`pwd`/gjslib/../python::`pwd`/../../gjslib/python:$PYTHONPATH ./lens_projection.py
import math
import OpenGL.GL
import gjslib_c
quaternion = gjslib_c.quaternion

def test():
    lp_20 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=20.0, lens_type="rectilinear")
    lp_20.orient(quaternion().lookat((0,0,1),(1,0,0)))
    ##At 20mm img_2335, (x,y) of (0.500,0) = (2592/5184,0) is (105 21/74 - 73 10/16) = 31.658" = 28.79 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((2592,0)).r)*2
    ##At 20mm img_2335, (x,y) of (0.423,0) = (2194/5184,0) is (100  0/16 - 73 10/16) = 26.375" = 24.64 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((2194,0)).r)*2
    ##At 20mm img_2335, (x,y) of (0.362,0) = (1876/5184,0) is ( 95 15/16 - 73 10/16) = 22.313" = 21.21 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((1876,0)).r)*2
    ##At 20mm img_2335, (x,y) of (0.174,0) = ( 900/5184,0) is ( 84  0/16 - 73 10/16) = 10.375" = 10.23 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((900,0)).r)*2
    ##At 20mm img_2335, (x,y) of (0.107,0) = ( 555/5184,0) is ( 80  0/16 - 73 10/16) =  6.375" =  6.32 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((555,0)).r)*2

    focal_length = 20.4
    lp_20 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=focal_length, lens_type="rectilinear")
    lp_20.orient(quaternion().lookat((0,0,1),(1,0,0)))
    print lp_20.xy_of_orientation(quaternion.of_euler(yaw=6.32,degrees=True))
    print lp_20.xy_of_orientation(quaternion.of_euler(yaw=10.23,degrees=True))
    print lp_20.xy_of_orientation(quaternion.of_euler(yaw=21.21,degrees=True))
    print lp_20.xy_of_orientation(quaternion.of_euler(yaw=24.64,degrees=True))
    print lp_20.xy_of_orientation(quaternion.of_euler(yaw=28.79,degrees=True))
    print math.atan2(31.658,57.5) * 180.0/math.pi
    print math.tan(29.13/180.0*math.pi)
    print 24.64/28.79*2592
    print 21.21/28.79*2592
    # distance d from center of image in mm
    # frame width is 22.3mm
    # pixels per mm is 5184/22.3
    # mm from center = (img pixels from center/5184)*22.3
    # r = Distance from center of image / focal length = (img pixels from center/5184)*22.3/20.0
    # r = f.tan(theta) for rectilinear, implies that theta = arctan2(r,f)
    def r(px,f): return px/5184.0*22.3/f
    def rectilinear_angle(r,f): return math.atan2(r,1) * 180.0/math.pi
    f = focal_length
    for px in (555,900,1876,2194,2592):
        angle = rectilinear_angle(r(px,f),f)
        xy = lp_20.xy_of_orientation(quaternion.of_euler(yaw=angle,degrees=True))
        print px, r(px,f), angle, xy
        pass

    focal_length=35
    lp_35 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=focal_length, lens_type="rectilinear")
    lp_35.orient(quaternion().lookat((0,0,1),(1,0,0)))
    q0 = lp_35.orientation_of_xy((5184/2,3456/2))
    q1 = lp_35.orientation_of_xy((-5184/2,-3456/2))
    rq = q0*~q1
    print "(r=%f,i=%f,j=%f,k=%f)"%(rq.r,rq.i,rq.j,rq.k)
    pass

test()

vector = gjslib_c.vector
v1 = vector((1,2,3))
print v1
#v1 = v1+v1
v1 += v1
print v1.dot_product(v1)
print abs(v1)
v1.normalize()
print abs(v1.scale(2))
v2 = vector((0,0,1))
v3 = v2.cross_product(v1)
v4 = v3.cross_product(v2)
v5 = v4 - v3
print str(vector(length=3)), str(v1), str(v2), str(v3), str(v4), str(v5)

print v5.coords

def axis_angle_from_v0_to_v1(v0,v1):
    axis = v0.cross_product(v1)
    angle = math.atan2(abs(axis),v0.dot_product(v1))
    axis.normalize()
    return (axis,angle)

#f axis_angle_of_quaternion_diff
def axis_angle_of_quaternion_diff(q0,q1,pt=(0,0,1)):
    a=vector(q0.rotate_vector(pt))
    b=a.axis_angle_to_v(vector(q1.rotate_vector(pt)))
    return b

#41.9012082851 (-0.832050, -0.554700, 0.000000)
#138.098791715 (0.832050, 0.554700, 0.000000)
#180.0 (0.000000, -0.000000, 0.000000)

focal_length=35
lp_35 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=focal_length, lens_type="rectilinear")
lp_35.orient(quaternion().lookat((0,0,1),(1,0,0)))
q0 = lp_35.orientation_of_xy((5184/2,3456/2))
q1 = lp_35.orientation_of_xy((-5184/2,-3456/2))
aa = axis_angle_of_quaternion_diff(q0,q1,(0,1,0))
print aa[1]*180.0/math.pi,aa[0]
aa = axis_angle_of_quaternion_diff(q0,q1,(1,0,0))
print aa[1]*180.0/math.pi,aa[0]

aa = axis_angle_of_quaternion_diff(q0,q1,(0,0,1))
print aa[1]*180.0/math.pi,aa[0]

print "\nin gjslib_c"
a = q1.rotate_vector((0,0,1))
b = q0.rotate_vector((0,0,1))
print a,b,vector(a).cross_product(vector(b))
aa = q0.axis_angle(q1,vector((0,0,1))).to_rotation()
print aa[0]*180.0/math.pi,aa[1]
die
for angle in [20.0*(i-10) for i in range(21)]:
    sin_angle = math.sin(angle/180.0*math.pi)
    cos_angle = math.cos(angle/180.0*math.pi)

    cos_angle_d2 = math.sqrt(1+cos_angle)/math.sqrt(2)
    sin_angle_d2 = math.sqrt(1-cos_angle_d2*cos_angle_d2)
    if (sin_angle<0): sin_angle_d2=-sin_angle_d2

    print angle, 180.0/math.pi * math.acos(cos_angle_d2), 180.0/math.pi * math.asin(sin_angle_d2),  180.0/math.pi * math.atan2(sin_angle_d2, cos_angle_d2)
    pass

