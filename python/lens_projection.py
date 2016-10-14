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
    lp_20 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=20, lens_type="rectilinear")
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
    f = 20
    for px in (555,900,1876,2194,2592):
        angle = rectilinear_angle(r(px,f),f)
        xy = lp_20.xy_of_orientation(quaternion.of_euler(yaw=angle,degrees=True))
        print px, r(px,f), angle, xy
        pass
    pass

test()
