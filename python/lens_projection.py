#!/usr/bin/env python
#PYTHONPATH=`pwd`/../python:`pwd`/..:`pwd`/gjslib/../python::`pwd`/../../gjslib/python:$PYTHONPATH ./lens_projection.py
#
# Notes
# 20mm shots at 44 1/16th (or possible 42 1/16th) - those at f22 are at infinity, 
# bars from the red channel - using a shot with the diagonal containing regular bars (of roughly 14/16th apart) at infinity using 20-35mm lens at 20mm - Nones where the image is blocked
# 40 bar widths is 31.5 inches
import math

#a Support functions
#f poly_calc
def poly_calc(poly, x):
    v = 0
    for p in poly:
        v=v*x + p
        pass
    return v

#f find_lens_polynomial
def find_lens_polynomial(pixel_width, equispaced_data, data_width_mm, camera_distance_mm, poly_degree=6, weight_of_zero=10, x_scale=1, do_plot=False):
    """
    A projection provides a function theta(r), where r is the distance from the center of the image
    So create a (pixel distance, physical distance) map
    Hence for each data entry we need a theta
    """
    cr_index = None
    cx_pix = pixel_width / 2.0
    for i in range(len(equispaced_data)):
        b = equispaced_data[i]
        if b>cx_pix:
            cr_index=i
            break
        pass
    cl_index=cr_index-1

    cl_pix = equispaced_data[cl_index]
    cr_pix = equispaced_data[cr_index]

    ofs_pix_closest_halfway_bars_to_center = (cr_pix+cl_pix)/2.0 - cx_pix
    data_width_pix_at_center = float(cr_pix-cl_pix)
    ofs_mm_center = ofs_pix_closest_halfway_bars_to_center / data_width_pix_at_center * data_width_mm

    sample_data = []
    for i in range(len(equispaced_data)):
        b_pix = equispaced_data[i]
        if b_pix is None: continue
        b_mm  = (i-cl_index-0.5)*data_width_mm + ofs_mm_center
        b_r_pix = abs(b_pix-cx_pix)
        b_r_mm  = abs(b_mm)
        b_theta = math.atan2(b_r_mm,camera_distance_mm) # for viewing only
        sample_data.append( (i, b_pix, b_mm, b_r_pix, b_r_mm, b_theta) )
        pass
    sample_data.sort(cmp=lambda x,y:cmp(x[3],y[3])) # Sort by pixels, in case a plots is needed

    # y = x.B where x is the row vector (x, x^2, x^3, x^4, x^5, x^6), and y is a scalar, hence B is a 6x1 matrix
    # We find best B through
    # B = (Xt.X)-1.Xt.y
    # Where y is an n x 1 (column) vector, and X is the n rows each of 6 columns (n x 6) matrix of n samples with their 6 values x, x^2, ...
    # Xt is then 6 x n; Xt.X is then 6 x 6, and its inverse is also 6 x 6; (Xt.X)-1.Xt is 6 x n, and hence B is 6 x 1.
    #
    # We can use numpy though
    import numpy

    xs = []
    ys = []
    nxs = []
    nys = []
    for i in range(weight_of_zero):
        xs.append(0)
        ys.append(0)
        nxs.append(0)
        nys.append(0)
        pass
    for d in sample_data:
        if d[0]<cl_index:
            nxs.append(d[3]/float(pixel_width/2.0) * x_scale)
            nys.append(d[5])
            pass
        else:
            xs.append(d[3]/float(pixel_width/2.0) * x_scale)
            ys.append(d[5])
            pass
        pass
    pass
    poly = numpy.polyfit(nxs, nys, poly_degree)
    inv_poly = numpy.polyfit(nys, nxs, poly_degree)

    #print xs
    #print nxs
    if do_plot:
        dx = 1.0/float(pixel_width/2.0) * x_scale
        import matplotlib.pyplot as plt
        yrs = []
        nyrs = []
        for x in xs:
            yrs.append(poly_calc(poly,x))
            pass
        for x in nxs:
            nyrs.append(poly_calc(poly,x))
            pass
        plt.plot(xs,ys)
        plt.plot(xs,yrs)
        plt.plot(nxs,nys)
        plt.plot(nxs,nyrs)
        plt.show()
        pass
    return (poly, inv_poly)

#a Generate poly for 20mm lens
width_20 = 6230
bars_20 = [112, 199.5, 284, 370, 456, 543, 629.5, 715.5, 804, 890.5, 979, 1064.5, 1151, 1230, 1317, 1406.5,
           1493.5, 1586, 1672, 1764.5, 1855.5, 1945.5, 2034.5, 2125.5, 2218.5, 2305.5, 2401.5, 2493, 2585, 2675.5, 2768, 2857.5, 2955, 3044.5,
           3139.5, 3229.5, 3324.5, 3413.5, 3504.5, 3598, 3689.5,
           3784.5, 3874, 3967, 4056.5, 4148, 4238.5, 4330, 4418, 4508, 4598, 4686.5, 4776.5,
           None, None, None, None, 5213, 5298, 5384.5, 5471, 5556, 5639.5, 5726.7, 5812.5, 5894, 5979.5, 6063, 6148.5]
camera_distance_mm = (42+1/16.0) * 25.4
bar_width_mm = 31.5*25.4/40 # 20mm

bar_diff = []
last = None
for b in bars_20:
    if b is not None and last is not None:
        bar_diff.append(b-last)
        pass
    last=b
    pass
#print bar_diff

x_scale_20 = 5184.0/width_20 * 20/22.3 * 2
x_scale_20 = 1/x_scale_20
for pd in range(3,8):
    (poly, inv_poly) = find_lens_polynomial(width_20, bars_20, bar_width_mm, camera_distance_mm, poly_degree=pd, x_scale=x_scale_20)
    p =list(poly)
    ip =list(inv_poly)
    p.reverse()
    ip.reverse()
    print pd, p, ip
    pass
# looking at this, the top coefficient keeps dropping into degrees=5, when it goes up - implying that things are starting to get a bit less stable
(poly_20, inv_poly_20) = find_lens_polynomial(width_20, bars_20, bar_width_mm, camera_distance_mm, poly_degree=5, do_plot=True, x_scale=x_scale_20)
for px in (555,900,1876,2194,2592):
    angle = poly_calc(poly_20,px/(width_20/2.0))
    print px, angle, math.degrees(angle)
    pass


# 15mm shots at 25 15/16th
import math
import OpenGL.GL
import gjslib_c
quaternion = gjslib_c.quaternion
vector     = gjslib_c.vector

gjslib_c.lens_projection.add_named_polynomial("canon_20_35_rebel2Ti_20",
#                                              (-7.9469597546159262e-05, 0.63541956275542499, -0.032610447960875623, 0.023507522557797155, -0.05312904706370946, 0.0086771578923328565),
#                                              (0.00010900649390522783, 1.5781376854050142, 0.047343710875515428, 0.37431719328904872, -0.8303111092968789, 1.3036418413439992)
#[-7.9469597546303475e-05, 0.85144873103827767, -0.058553465905638334, 0.056558857112973038, -0.17128672710793724, 0.037485827814715662],
#[8.1349417962357022e-05, 1.1777333402154477, 0.035331687002521039, 0.27934561250860773, -0.61964496837882266, 0.97288245154393993]
#[-7.9469597546248994e-05, 0.47420121260847303, -0.01816188503551705, 0.0097704169605461882, -0.016479360015559729, 0.0020085742660049949],
#[0.0001460663888093956, 2.1146710117248575, 0.06343956798054573, 0.50157709632004632, -1.1125992679738443, 1.7468524052478904]
#[-7.9469597546248994e-05, 0.23710060630423652, -0.0045404712588792626, 0.0012213021200682735, -0.0010299600009724831, 6.276794581265609e-05],
#[0.00029213277761879121, 4.229342023449715, 0.12687913596109146, 1.0031541926400926, -2.2251985359476887, 3.4937048104957809],
[-7.9469597546248994e-05, 0.94840242521694607, -0.072647540142068201, 0.078163335684369506, -0.26366976024895566, 0.064274376512159836],
[7.3033194404697801e-05, 1.0573355058624287, 0.031719783990272865, 0.25078854816002316, -0.55629963398692217, 0.87342620262394521],

                                              )

def test():
    lp_20 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=20.0, lens_type="rectilinear")
    lp_20.orient(quaternion().lookat(vector((0,0,1)),vector((1,0,0))))
    ##At 20mm img_2335, (x,y) of (0.500,0) = (2592/5184,0) is (105 21/74 - 73 10/16) = 31.658" = 28.79 degrees (possibly 18/74, 
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((2592,0)).r)*2
    ##At 20mm img_2335, (x,y) of (0.423,0) = (2194/5184,0) is (100  0/16 - 73 10/16) = 26.375" = 24.64 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((2194,0)).r)*2
    ##At 20mm img_2335, (x,y) of (0.362,0) = (1876/5184,0) is ( 95 15/16 - 73 10/16) = 22.313" = 21.21 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((1876,0)).r)*2
    ##At 20mm img_2335, (x,y) of (0.174,0) = ( 900/5184,0) is ( 84  0/16 - 73 10/16) = 10.375" = 10.23 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((900,0)).r)*2
    ##At 20mm img_2335, (x,y) of (0.107,0) = ( 555/5184,0) is ( 80  0/16 - 73 10/16) =  6.375" =  6.32 degrees
    #print 180.0/math.pi*math.acos(lp_20.orientation_of_xy((555,0)).r)*2
    c_in = 73+10/16.0
    data = { 555 :(80+0/16.0-c_in),
             900 :(84+1/16.0-c_in),
             1876:(95+15/16.0-c_in),
             2194:(100+0/16.0-c_in),
             2592:(105+18/74.0-c_in),
             }
    distance_to_center = 57.5
    distance_to_center = 56.4
    focal_length = 20.4
    lp_20 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=focal_length, lens_type="rectilinear")
    lp_20.orient(quaternion().lookat(vector((0,0,1)),vector((1,0,0))))
    lp_20_poly = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=20.0, lens_type="polynomial")
    lp_20_poly.set_polynomial("canon_20_35_rebel2Ti_20")
    lp_20_poly.orient(quaternion().lookat(vector((0,0,1)),vector((1,0,0))))

    # distance d from center of image in mm
    # frame width is 22.3mm
    # pixels per mm is 5184/22.3
    # mm from center = (img pixels from center/5184)*22.3
    # r = Distance from center of image / focal length = (img pixels from center/5184)*22.3/20.0
    # r = f.tan(theta) for rectilinear, implies that theta = arctan2(r,f)
    def r(px,f): return px/5184.0*22.3/f
    def rectilinear_angle(r,f): return math.degrees(math.atan2(r,1))
    f = focal_length
    for px in data.keys():
        d_px = data[px]
        # d_px of -29 should map to -30.5; d_px of 32.0 should map to 30.5
        # we can do this with the cos rule - a*a = b*b+c*c-2.b.c.cos(90+epsilon), a = distance to point
        # then sin(theta)=sin(90+epsilon)*b/a [b is reading, c=distance from camera]
        epsilon = 1.26
        epsilon = 3
        sin_epsilon = math.sin(math.radians(90+epsilon))
        cos_epsilon = math.cos(math.radians(90+epsilon))
        d_pt = d_px*d_px + distance_to_center*distance_to_center-2*d_px*distance_to_center*cos_epsilon
        d_pt = math.sqrt(d_pt)
        theta = math.degrees(math.asin(d_px/d_pt*sin_epsilon))
        d_px = math.tan(math.radians(theta))*distance_to_center
        rect_angle = rectilinear_angle(r(px,f),f)
        read_angle = math.degrees(math.atan2(d_px,distance_to_center))
        #read_angle = theta
        poly_angle = math.degrees(poly_calc(poly_20,px/(width_20/2.0)))
        poly_angle = lp_20_poly.orientation_of_xy((px,0)).rotate_vector(vector((0,0,1)))
        print poly_angle
        poly_angle = -math.degrees(math.atan2(poly_angle.coords[1],poly_angle.coords[2]))

        rect_xy = lp_20.xy_of_orientation(quaternion.of_euler(yaw=rect_angle,degrees=True))
        read_xy = lp_20.xy_of_orientation(quaternion.of_euler(yaw=read_angle,degrees=True))
        test_px = px
        for i in range(10):
            poly_angle_tx = math.degrees(poly_calc(poly_20,(test_px)/(width_20/2.0)))
            poly_angle_dx = math.degrees(poly_calc(poly_20,(test_px+1)/(width_20/2.0))) - poly_angle_tx
            test_px += (read_angle - poly_angle_tx)/poly_angle_dx
            pass
        
        print px, d_px, read_angle, rect_angle, poly_angle, rect_xy[0], read_xy[0], test_px, poly_angle_tx, theta
        pass
    print "px, d_px, read_angle, rect_angle, poly_angle, rect_xy[0], read_xy[0], test_px, poly_angle_tx, theta"
    focal_length=35
    lp_35 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=focal_length, lens_type="rectilinear")
    lp_35.orient(quaternion().lookat(vector((0,0,1)),vector((1,0,0))))
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

def angle_axis_from_v0_to_v1(v0,v1):
    axis = v0.cross_product(v1)
    angle = math.atan2(abs(axis),v0.dot_product(v1))
    axis.normalize()
    return (axis,angle)

#f angle_axis_of_quaternion_diff
def angle_axis_of_quaternion_diff(q0,q1,pt=(0,0,1)):
    a=q0.rotate_vector(vector(pt))
    b=a.angle_axis_to_v(q1.rotate_vector(vector(pt)))
    return b

#41.9012082851 (-0.832050, -0.554700, 0.000000)
#138.098791715 (0.832050, 0.554700, 0.000000)
#180.0 (0.000000, -0.000000, 0.000000)

focal_length=35
lp_35 = gjslib_c.lens_projection(width=5184, frame_width=22.3, focal_length=focal_length, lens_type="rectilinear")
lp_35.orient(quaternion().lookat(vector((0,0,1)),vector((1,0,0))))
q0 = lp_35.orientation_of_xy((5184/2,3456/2))
q1 = lp_35.orientation_of_xy((-5184/2,-3456/2))
aa = angle_axis_of_quaternion_diff(q0,q1,(0,1,0)).to_rotation()

print aa[0]*180.0/math.pi,aa[1]
aa = angle_axis_of_quaternion_diff(q0,q1,(1,0,0)).to_rotation()
print aa[0]*180.0/math.pi,aa[1]

aa = angle_axis_of_quaternion_diff(q0,q1,(0,0,1)).to_rotation()
print aa[0]*180.0/math.pi,aa[1]

print "\nin gjslib_c"
a = q1.rotate_vector(vector((0,0,1)))
b = q0.rotate_vector(vector((0,0,1)))
print a,b,a.cross_product(b)
aa = q0.angle_axis(q1,vector((0,0,1))).to_rotation()
print aa[0]*180.0/math.pi,aa[1]

src_axis = vector((1,0,0))
tgt_axis = vector((1.0000000000,0,0))

print src_axis.angle_axis_to_v(tgt_axis)

print quaternion(1).distance_to(quaternion(1))
print quaternion(1).distance_to(quaternion(0.99999,0,0,0.00001).normalize())
print quaternion(1).distance_to(quaternion(0.99999,0,0,0.0001).normalize())
print quaternion(1).distance_to(quaternion(0.99999,0,0,0.001).normalize())
print quaternion(1).distance_to(quaternion(0.99999,0,0,0.01).normalize())
print quaternion(1).distance_to(quaternion(0.99999,0,0,0.014).normalize())
print quaternion(0.999999,0,0,0.14).normalize()
print quaternion(0.992888, -0.023600, -0.020715, -0.114839).distance_to(quaternion(r=0.993378,i=-0.006266,j=-0.007135,k=-0.114499))

die
for angle in [20.0*(i-10) for i in range(21)]:
    sin_angle = math.sin(angle/180.0*math.pi)
    cos_angle = math.cos(angle/180.0*math.pi)

    cos_angle_d2 = math.sqrt(1+cos_angle)/math.sqrt(2)
    sin_angle_d2 = math.sqrt(1-cos_angle_d2*cos_angle_d2)
    if (sin_angle<0): sin_angle_d2=-sin_angle_d2

    print angle, 180.0/math.pi * math.acos(cos_angle_d2), 180.0/math.pi * math.asin(sin_angle_d2),  180.0/math.pi * math.atan2(sin_angle_d2, cos_angle_d2)
    pass

