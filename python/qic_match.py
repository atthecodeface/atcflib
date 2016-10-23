#!/usr/bin/env python
#
# PYTHONPATH=`pwd`/../python:`pwd`/..:`pwd`/gjslib/../python::`pwd`/../../gjslib/python:$PYTHONPATH ./qic_match.py
#
# Useful cosines / min q dists:
# 0.04     : 0.99    => 8.1   degrees
# 0.02     : 0.995   => 5.76  degrees
# 0.004    : 0.999   => 2.56  degrees
# 0.002    : 0.9995  => 1.8   degrees
# 0.0004   : 0.9999  => 0.81  degrees
# 0.0002   : 0.99995 => 0.576 degrees
# 0.00004  : 0.99999 => 0.256 degrees
# 0.00002  : 0.999995 => 0.18 degrees
# 0.000004 : 0.999999 => 0.081 degrees
# Note FOV for 35mm is about 45 degrees
# So 5k pixels in 45 degress is about 0.01 degrees per pixel
# 
max_q_dists = {
    "1.8deg"   :2,
    "200pix35" :2,
    "80pix35"  :2,
    "0.18deg"  :2,
    "20pix35"  :0.002,
    "0.081deg" :0.002,
    "8pix35"   :0.002,
}
min_cos_seps = {
    "1.8deg"   :0.9995,
    "200pix35" :0.9995,
    "80pix35"  :0.9999,
    "0.18deg"  :0.999995,
    "20pix35"  :0.999995,
    "0.081deg" :0.999999,
    "8pix35"   :0.999999,
}
min_q_dists = {
    "5.76deg"  :0.02,
    "1.8deg"   :0.002,
    "200pix35" :0.002,
    "80pix35"  :0.0004,
    "0.18deg"  :0.00002,
    "20pix35"  :0.00002,
    "0.081deg" :0.000004,
    "8pix35"   :0.000004,
}

#a Imports
import OpenGL.GLUT
import OpenGL.GL
import gjslib_c
import math
import sys
img_png_n=0
vector_z = gjslib_c.vector(vector=(0,0,1))
vector_x = gjslib_c.vector(vector=(1,0,0))

#a Support functions
#f init_opengl
def init_opengl():
    OpenGL.GLUT.glutInit(sys.argv)
    OpenGL.GLUT.glutInitDisplayMode(OpenGL.GLUT.GLUT_3_2_CORE_PROFILE)
    #|GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
    OpenGL.GLUT.glutInitWindowSize(64,64)
    OpenGL.GLUT.glutCreateWindow("self.window_title")

#f save_as_png
def save_as_png(texture, filename):
    b = gjslib_c.filter(filter="save:%s(0)"%filename)
    b.textures([texture])
    b.compile()
    b.execute()
    pass

#f quaternion_average
def quaternion_average(qs):
    vf = gjslib_c.vector(length=3)
    vu = gjslib_c.vector(length=3)
    for q in qs:
        vf += q.rotate_vector(vector_z)
        vu += q.rotate_vector(vector_x)
        pass
    return gjslib_c.quaternion().lookat(vf,vu)

#f initialize
def initialize(num_textures=12, size=1024):
    print "init opengl"        
    init_opengl()
    print "Create textures"
    tb = []
    for i in range(num_textures):
        tb.append(gjslib_c.texture(width=size, height=size))
        pass
    return tb

#f quaternion_image_correlate
def quaternion_image_correlate(ipqm, images, focal_length=50.0, accuracy="80pix35", max_n=10):

    print "Quaternion_Image_Correlate"

    ci0 = ipqm.camera_images[images[0]]
    ci1 = ipqm.camera_images[images[1]]

    ipqm.qic = gjslib_c.quaternion_image_correlator()
    ipqm.qic.min_cos_angle_src_q = min_cos_seps[accuracy]
    ipqm.qic.min_cos_angle_tgt_q = min_cos_seps[accuracy]
    ipqm.qic.min_cos_sep_score   = min_cos_seps[accuracy]
    ipqm.qic.max_q_dist_score    = min_q_dists[accuracy]

    for initial_orientation in [gjslib_c.quaternion().from_euler(yaw=-12.0,degrees=1),
                                gjslib_c.quaternion().from_euler(yaw=+12.0,degrees=1),
                                ]:
        src_img_lp_to = gjslib_c.lens_projection(width=2.0, height=2.0, frame_width=36.0, focal_length=focal_length, lens_type="rectilinear")
        dst_img_lp_to = gjslib_c.lens_projection(width=2.0, height=2.0, frame_width=36.0, focal_length=focal_length, lens_type="rectilinear")
        src_img_lp_to.orient(initial_orientation)
        dst_img_lp_to.orient(initial_orientation)
        projections = (src_img_lp_to, dst_img_lp_to)
        if not ipqm.overlap_projections((images[0], images[1]), (src_img_lp_to, dst_img_lp_to)):
            continue

        print "Using src projection focal length",src_img_lp_to.focal_length,"centered on",(src_img_lp_to.orientation).rotate_vector(vector_z)
        ipqm.find_matches((images[0], images[1]), projections=projections)
        pass

    ipqm.qic.create_mappings()
    best_matches = ipqm.find_best_target_matches_qic(max_q_dist        = max_q_dists[accuracy],
                                                     min_q_dist        = min_q_dists[accuracy]/10.0,
                                                     min_cos_sep       = min_cos_seps[accuracy],
                                                     min_cos_sep_score = min_cos_seps[accuracy],
                                                     max_q_dist_score  = min_q_dists[accuracy],
                                                     )

    print ipqm.times()

    #b Do clusters
    def cmp_matches(x,y):
        return cmp(y.max_distance,x.max_distance)
        if x.max_distance/len(x.mappings)<y.max_distance/len(y.mappings): return -1
        return 1

    best_matches.sort(cmp=lambda x,y:cmp(y.max_distance,x.max_distance))
    print "Best matches for whole image"
    n = len(best_matches)
    if n>max_n: n=max_n
    best_matches = best_matches[:n]
    for bm in best_matches:
        bm.calculate( ipqm.qic)
        pass
    return best_matches

#a Filter classes
#c c_filter
class c_filter(object):
    filter_text = 'glsl:yuv_from_rgb(1,2)'
    defines = {}
    parameters = {}
    def __init__(self, extra_parameters={}, extra_defines={}):
        self.f = gjslib_c.filter(filter=self.filter_text)
        for u in extra_parameters:
            self.f.parameter(u,extra_parameters[u])
        for u in self.parameters:
            if u not in extra_parameters:    
                self.f.parameter(u,self.parameters[u])
        for d in extra_defines:
            self.f.define(d,str(extra_defines[d]))
            pass
        for d in self.defines:
            if d not in extra_defines:
                self.f.define(d,str(self.defines[d]))
                pass
            pass
        self.f.compile()
        pass
    def set_parameters(self, parameters={}):
        for p in parameters:
            self.f.parameter(p,parameters[p])
            pass
        pass
    def set_projections(self, projections=(None,None), num_x_divisions=None, num_y_divisions=None):
        if projections[0] and projections[1]: self.f.projections(projections[0], projections[1])
        if num_x_divisions: self.f.parameter("num_x_divisions",num_x_divisions)
        if num_y_divisions: self.f.parameter("num_y_divisions",num_y_divisions)
        pass
    def execute(self,textures):
        self.f.textures(textures)
        self.f.execute()
        pass
    def times(self):
        return self.f.times
    pass

#c c_mandelbrot_filter
class c_mandelbrot_filter(c_filter):
    filter_text = 'glsl:mandelbrot(0,1)'

#c c_alu_filter
class c_alu_filter(c_filter):
    filter_text = 'glsl:alu_buffers(0,0,0)'

#c c_yuv_from_rgb
class c_yuv_from_rgb(c_filter):
    filter_text = 'glsl:yuv_from_rgb(1,2)'

#c c_gauss_filter_y
class c_gauss_filter_y(c_filter):
    filter_text = 'glsl:gauss(1,2)'
    defines = {"X_NOT_Y":"false", "NUM_WEIGHTS":"9", "WEIGHTS":"gauss_offset_weights9"}

#c c_gauss_filter_x
class c_gauss_filter_x(c_filter):
    filter_text = 'glsl:gauss(1,2)'
    defines = {"X_NOT_Y":"true", "NUM_WEIGHTS":"9", "WEIGHTS":"gauss_offset_weights9"}
        
#c c_sobel_filter
class c_sobel_filter(c_filter):
    filter_text = 'glsl:convolve_2d(1,2)'
    defines = {"NUM_WEIGHTS":"9", "OFFSET_WEIGHTS":"sobel_weights"}

#c c_harris_filter
class c_harris_filter(c_filter):
    filter_text = 'glsl:harris(1,2)'
    defines = {"NUM_OFFSETS":"25", "OFFSETS":"offsets_2d_25"}

#c c_circle_dft_filter
class c_circle_dft_filter(c_filter):
    filter_text = "glsl:circle_dft(2,4)"
    defines = { "NUM_CIRCLE_STEPS":"8", 
                "DFT_CIRCLE_RADIUS":"8",
                "CIRCLE_COMPONENT":"r" }

#c c_circle_dft_diff_filter
class c_circle_dft_diff_filter(c_filter):
    filter_text = "glsl:circle_dft_diff(2,4,5)"
    parameters = {"uv_base_x":0.0, "uv_base_y":0.0}

#c c_circle_dft_diff_combine_filter
class c_circle_dft_diff_combine_filter(c_filter):
    filter_text = "glsl:circle_dft_diff_combine(1,2,3,4,0)"
    defines = {"DISCRETE_CIRCLE_OFS":"discrete_circle_offsets_4_32",
               "NUM_OFFSETS":32}

#c c_find_filter
class c_find_filter(c_filter):
    filter_text = "find:a(1)"

#c c_windowed_equalization_filter
class c_windowed_equalization_filter(c_filter):
    filter_text = 'glsl:windowed_equalization(2,4)'
    defines = {"NUM_OFFSETS":81, "OFFSETS":"offsets_2d_81"}

#a Match classes
#c c_camera_image
class c_camera_image(object):
    def __init__(self, image_filename, frame_width=22.3, focal_length=35.0, lens_type="rectilinear", orientation=None):
        self.texture = gjslib_c.texture(filename=image_filename)
        # The lp has to have width/height such that a uv of 0 to 1 maps from left to right, and bottom to top
        # The uv is derived from the (-1,1) range, i.e. uv can also be conceived as -1=left, -1=bottom, +1=right, +1=top
        # Hence for images that are wider than high, we want -1=left, +1=right; but the same angle rotated by 90 is off the top
        # So the uv for that must be = width/height. Hence, for wider than high, width=2.0, height=2.0*width/height
        # For example, an image that is 300 wide and 200 high will have width=2.0, height=3.0
        self.lp      = gjslib_c.lens_projection(width=2.0,
                                                height=2.0*self.texture.width/self.texture.height,
                                                frame_width=frame_width,
                                                focal_length=focal_length,
                                                lens_type=lens_type)
        if orientation is None:
            orientation = gjslib_c.quaternion(r=1)
            pass
        self.lp.orient(orientation)
        pass
    def orientation(self):
        return self.lp.orientation
    def orientation_of_xy(self,xy):
        return self.lp.orientation_of_xy(xy)
    def orientation_of_xy_list(self, xy_list):
        q_list = []
        for xy in xy_list:
            q_list.append(self.lp.orientation_of_xy(xy))
            pass
        return q_list
    def xy_of_orientation_list(self, q_list):
        xy_list = []
        for q in q_list:
            xy_list.append(self.lp.xy_of_orientation(q))
            pass
        return xy_list

#c c_image_match
class c_image_match(object):
    radius=4
    windowed_equalization = False
    max_corners=3
    max_matches_per_corner=3
    min_corner_distance = 5.0
    min_corner_distance = 60.0
    min_match_distance = 2.5
    def __init__(self, radius=4):
        # radius must be 4 at the moment as circle_dft_diff_combine_filter is fixed    
        self.copy_img = c_alu_filter(extra_defines={"OP":"src_a"})
        self.equalize = c_windowed_equalization_filter()
        self.harris = c_harris_filter()
        self.find_corners = c_find_filter(extra_parameters={"min_distance":self.min_corner_distance, "minimum":0.05, "max_elements":1250})
        self.find_matches = c_find_filter(extra_parameters={"min_distance":self.min_match_distance, "minimum":0.04,  "max_elements":250})
        self.circle_dft = c_circle_dft_filter(extra_defines={"DFT_CIRCLE_RADIUS":self.radius,
                                                             "CIRCLE_COMPONENT":"r",
                                                             })
        self.circle_dft_diff         = c_circle_dft_diff_filter()
        self.circle_dft_diff_combine = c_circle_dft_diff_combine_filter()
        pass
    def get_matches(self, tb):
        """tb must be at least 10 textures, and the first is the source image, second is the target image"""
        self.harris.execute( (tb[0],tb[4]) )

        if self.windowed_equalization: # do windowed equalization to remove brightness and contrastiness dependence - with loss of information
                self.copy_img.execute((tb[0],tb[0],tb[2]))
                self.equalize.execute((tb[2],tb[0]))
                self.copy_img.execute((tb[1],tb[1],tb[2]))
                self.equalize.execute((tb[2],tb[1]))
                pass
        self.circle_dft.execute((tb[0],tb[2]))
        self.circle_dft.execute((tb[1],tb[3]))
        self.find_corners.execute( (tb[4],) )

        print "Found %d corners (will restrict to max %d)"%(self.find_corners.f.num_points, self.max_corners)
        corners = self.find_corners.f.points[:self.max_corners]
        matches = {}
        for i in range(len(corners)):
            pt = corners[i]
            xy = (pt[0],pt[1])
            for i, dxy in [(0,(1,0)), (1,(0,1)), (2,(-1,0)), (3,(0,-1))]:
                self.circle_dft_diff.set_parameters( {"uv_base_x":xy[0]+dxy[0]*self.radius,
                                                      "uv_base_y":xy[1]+dxy[1]*self.radius} )
                self.circle_dft_diff.execute( (tb[2], tb[3], tb[5+i]) )
                pass
            self.circle_dft_diff_combine.execute( (tb[5], tb[6], tb[7], tb[8], tb[9]) )
            self.find_matches.execute( (tb[9],) )
            matches[xy] = self.find_matches.f.points[0:self.max_matches_per_corner]
            pass
        return matches
    def times(self):
        return {"copy_img":self.copy_img.times(),
                "equalize":self.equalize.times(),
                "harris":self.harris.times(),
                "circle_dft":self.circle_dft.times(),
                "find_corners":self.find_corners.times(),
                "circle_dft_diff":self.circle_dft_diff.times(),
                "circle_dft_diff_combine":self.circle_dft_diff_combine.times(),
                "find_matches":self.find_matches.times(),
                }
    pass

#c c_image_pair_quaternion_match
class c_image_pair_quaternion_match(object):
    #f __init__
    def __init__(self, filenames=[]):
        self.camera_images = {}
        for f in filenames:
            self.add_image(f)
            pass
        self.im = c_image_match()
        self.im.max_corners=40
        self.im.max_matches_per_corner=10
        self.im.max_corners=20
        self.im.max_matches_per_corner=10 # Not too many matches, as a good match is a good match
        self.to_yuv = c_yuv_from_rgb(extra_parameters={"xsc":2.0,"ysc":2.0,"xc":1.0,"yc":1.0},
                                extra_defines={"EXTRA_VERTEX_UNIFORMS":"uniform float xsc, ysc, xc, yc;",
                                               "GL_POSITION":"vec4(xsc*x+xc,ysc*y+yc,0,1)",
                                               })
        self.src_qs = []
        self.qic = None
        pass
    #f xy_from_texture
    def xy_from_texture(self,texture,texture_xy):
        return ( (2*(float(texture_xy[0])/texture.width)-1.0),
                 (2*(float(texture_xy[1])/texture.height)-1.0) )
    #f orient
    def orient(self, image, orientation):
        self.camera_images[image].lp.orient(orientation)
        pass
    #f xy_of_orientation
    def xy_of_orientation(self, image, q):
        return self.camera_images[image].lp.xy_of_orientation(q)
    #f add_image
    def add_image(self, image_filename="", **kwargs):
        if image_filename not in self.camera_images:
            self.camera_images[image_filename] = c_camera_image(image_filename=image_filename, **kwargs)
            pass
        pass
    #f find_overflows_for_projections
    def find_overflows_for_projections(self, images, projections):
        def check_bounds(in_bounds, off_edge, edge, image, xy):
            # An image that is 300 wide and 200 high will have width=2.0, height=3.0 for its scaling
            # This means that the xy positions returned are truly just within the image
            # independent of the aspect ratio
            err = None
            for e in (abs(xy[0])-1, abs(xy[1])-1 ):
                if e<=0: continue
                if e > off_edge[edge]:
                    off_edge[edge] = e
                    in_bounds = False
                    pass
                pass
            return (in_bounds, off_edge)
        ci0 = self.camera_images[images[0]]
        ci1 = self.camera_images[images[1]]
        ci0_tp = projections[0]
        ci1_tp = projections[1]
        q0_list = []
        q1_list = []
        for i in range(4):
            for j in range(21):
                xy = j/10.0-1.0 # in range -1<=xy<1
                xy = [ (xy,-1.0), (1.0,xy), (-xy,1.0), (-1.0,-xy) ][i] # (-1,-1)<=xy<=(1,1)
                q0_list.append(ci0_tp.orientation_of_xy(xy))
                q1_list.append(ci1_tp.orientation_of_xy(xy))
                pass
            pass
        ci0_q0_xy_list = ci0.xy_of_orientation_list(q0_list)
        ci1_q0_xy_list = ci1.xy_of_orientation_list(q0_list)
        ci0_q1_xy_list = ci0.xy_of_orientation_list(q1_list)
        ci1_q1_xy_list = ci1.xy_of_orientation_list(q1_list)
        off_edge = [0]*4
        in_bounds = True
        for i in range(4):
            for j in range(21):
                (in_bounds, off_edge) = check_bounds(in_bounds, off_edge, i, ci0, ci0_q0_xy_list[i*21+j])
                (in_bounds, off_edge) = check_bounds(in_bounds, off_edge, i, ci1, ci1_q0_xy_list[i*21+j])
                (in_bounds, off_edge) = check_bounds(in_bounds, off_edge, i, ci0, ci0_q1_xy_list[i*21+j])
                (in_bounds, off_edge) = check_bounds(in_bounds, off_edge, i, ci1, ci1_q1_xy_list[i*21+j])
                pass
            pass
        if in_bounds: return None
        return off_edge
    #f overlap_projections
    def overlap_projections(self, images, projections, scale=0.9, max_iter=80):
        ci0 = self.camera_images[images[0]]
        ci1 = self.camera_images[images[1]]
        ci0_tp = projections[0]
        ci1_tp = projections[1]
        for i in range(max_iter):
            overflow = self.find_overflows_for_projections(images, projections)
            size = None
            dxy = [0,0]
            if overflow is None: return projections
            overflow_bits = ( ((overflow[0]>0) and 1) |
                              ((overflow[1]>0) and 2) |
                              ((overflow[2]>0) and 4) |
                              ((overflow[3]>0) and 8) )
            # Edges are 0 => y=-1, 1=> x=1, 2=> y=1, 3=>x=-1
            if overflow_bits in [1, 3,  9, 11]: dxy[1]=1-scale
            if overflow_bits in [4, 6, 12, 14]: dxy[1]=scale-1
            if overflow_bits in [2, 3,  6,  7]: dxy[0]=scale-1
            if overflow_bits in [8, 9, 12, 13]: dxy[0]=1-scale
            if overflow_bits in [5, 7, 13, 10, 11, 14, 15]: size = scale
            if (dxy[0]!=0) or (dxy[1]!=0):
                dxy = tuple(dxy)
                q0 = ci0_tp.orientation_of_xy(dxy)
                q1 = ci0_tp.orientation_of_xy((0,0))
                dq = q1.angle_axis(q0,vector_z)
                ci0_tp.orient(dq*ci0_tp.orientation)
                # now ci0_tp.orientation_of_xy((0,0)).rotate_vector(vector_z) == q0.rotate_vector(vector_z)

                q0 = ci1_tp.orientation_of_xy(dxy)
                q1 = ci1_tp.orientation_of_xy((0,0))
                dq = q1.angle_axis(q0,vector_z)
                ci1_tp.orient(dq*ci1_tp.orientation)

                pass
            if size is not None:
                ci0_tp.focal_length = ci0_tp.focal_length / scale
                ci1_tp.focal_length = ci1_tp.focal_length / scale
                pass
            if False:
                print overflow_bits, size, dxy, ci0_tp.orientation_of_xy((0,0)).rotate_vector(vector_z)
                pass
            pass
        return None
    #f find_matches
    def find_matches(self, images, cxy0=None, size=None, min_cos_sep=min_cos_seps["8pix35"],
                     projections = None):
        #f globals
        global img_png_n

        #b Set to/from projections
        ci0 = self.camera_images[images[0]]
        ci1 = self.camera_images[images[1]]
        if projections is None:
            # 'size' maps to frame_width;
            # fov = 2atan2(frame_width/2focal_length) = 2atan2(size/2)
            # hence size = frame_width/focal_length
            # hence focal_length = frame_width/size
            focal_length = 36.0 / size
            #if img_png_n>0:focal_length=200.0 # GJS T

            src_img_lp_to = gjslib_c.lens_projection(width=2.0, height=2.0, frame_width=36.0, focal_length=focal_length, lens_type="rectilinear")
            dst_img_lp_to = gjslib_c.lens_projection(width=2.0, height=2.0, frame_width=36.0, focal_length=focal_length, lens_type="rectilinear")

            # Currently we can change ci0 orientation and src_to_orientation works great with world_cxy0_q
            # (but presumably not with ci0.orientation() * world_cxy0_q...)
            # The dest projection camera needs to point at world_cxy0_q relative to the destination image
            # as the destination image is accessed 'destination image local'
            # Not sure I buy this logic actually...
            print "Centering on image 0",cxy0, (cxy0[0]+0.5)*5184, (cxy0[1]+0.5)*3456
            world_cxy0_q = ci0.orientation_of_xy(cxy0)

            src_to_orientation = world_cxy0_q
            dst_to_orientation = world_cxy0_q

            dst_img_lp_to.orient(dst_to_orientation)
            src_img_lp_to.orient(src_to_orientation)
            pass

        if projections is not None:
            (src_img_lp_to, dst_img_lp_to) = projections
            pass

        #b Find matches
        self.to_yuv.set_parameters( {"ysc":2.0, "yc":-1.0, "xsc":2.0, "xc":-1.0} )
        self.to_yuv.set_projections(projections=(ci0.lp,src_img_lp_to), num_x_divisions=20, num_y_divisions=20)
        self.to_yuv.execute((ci0.texture,tb[0]))
        self.to_yuv.set_projections(projections=(ci1.lp,dst_img_lp_to), num_x_divisions=20, num_y_divisions=20)
        self.to_yuv.execute((ci1.texture,tb[1]))
        matches = self.im.get_matches(tb)

        if False:
            save_as_png(tb[0],"a%d.png"%img_png_n)
            save_as_png(tb[1],"b%d.png"%img_png_n)
            img_png_n+=1
            pass

        #b Add source -> target matches for qic
        for m in matches:
            src_xy = self.xy_from_texture(tb[0],m)
            src_q = src_img_lp_to.orientation_of_xy(src_xy)
            for mm in matches[m]:
                tgt_xy = self.xy_from_texture(tb[1],mm)
                tgt_q = dst_img_lp_to.orientation_of_xy(tgt_xy)
                self.qic.add_match(src_q, tgt_q, mm[2], mm[3], mm[4])
                pass
            pass
        pass
    #c c_best_match
    class c_best_match(object):
        unique_id = 0
        qic_qs = {}
        #f qid_from_q
        @classmethod
        def qid_from_q(cls, q, why="none"):
            r = "%f:%f:%f:%f"%(q.r,q.i,q.j,q.k)
            if r in cls.qic_qs: return cls.qic_qs[r][1]
            qic_qid = cls.unique_id
            cls.qic_qs[r]       = (q,qic_qid,why)
            cls.qic_qs[qic_qid] = (q,qic_qid,why)
            cls.unique_id += 1
            return qic_qid
        #f __init__
        def __init__(self, src_from_tgt_q, score, qic):
            self.max_distance = score # measure of how good it is - smaller the better
            self.src_from_tgt_q = src_from_tgt_q
            self.min_cos_sep = 0
            self.max_q_dist  = 0
            self.mappings = [] # list of (src/tgt) pairs
            self.best_mappings = [] # list of (src,tgt,n) pairs
            self.qs = {} # map from src,tgt,src,tgt to orientation, distance
            self.qic_scores = qic.scores()
            self.qic_mappings = qic.src_tgt_mappings_of_best_matches(min_score=0, min_count=0)
            self.optimized_src_from_tgt_q = gjslib_c.quaternion(1)
            pass
        #f calculate
        def calculate(self, qic=None):
            qic_bm_qs = []
            qs = []
            for (src_q0, src_q1, tgt_q0, tgt_q1, src_from_tgt_orient) in self.qic_mappings:
                src_q0_id = self.qid_from_q(src_q0,"src")
                tgt_q0_id = self.qid_from_q(tgt_q0,"tgt")
                src_q1_id = self.qid_from_q(src_q1,"src")
                tgt_q1_id = self.qid_from_q(tgt_q1,"tgt")
                qic_bm_qs.append((src_q0_id, tgt_q0_id, src_q1_id, tgt_q1_id, src_from_tgt_orient))
                qs.append(src_from_tgt_orient)
                pass
            self.optimized_src_from_tgt_q = quaternion_average(qs)
            # For save_mappings
            self.qs = {}
            self.best_mappings = []
            self.mappings = []
            for (src_q0, src_q1, tgt_q0, tgt_q1, src_from_tgt_orient) in self.qic_mappings:
                src_q0 = self.qid_from_q(src_q0,"src")
                tgt_q0 = self.qid_from_q(tgt_q0,"tgt")
                src_q1 = self.qid_from_q(src_q1,"src")
                tgt_q1 = self.qid_from_q(tgt_q1,"tgt")
                self.mappings.append((src_q0,tgt_q0))
                bm = (src_q0,tgt_q0,0)
                if bm not in self.best_mappings:
                    self.best_mappings.append(bm)
                    pass
                dq = self.src_from_tgt_q.distance_to(src_from_tgt_orient)
                self.qs[(src_q0, tgt_q0, src_q1, tgt_q1)] = (src_from_tgt_orient, dq)
                pass
            pass
    #f find_mappings_to_try_qic
    def find_mappings_to_try_qic(self, min_q_dist=0.0002, max_q_dist=0.5): # default of 0.5 degrees min sep
        print "Building from qic list of src_from_tgt_q (min_q_dist %f, max_q_dist %f)"%(min_q_dist,max_q_dist)
        mappings_to_try = []
        mappings_to_try.append(gjslib_c.quaternion(1)) # Ensure identity is on the list
        for src_qx in self.qic.src_qs():
            for tgt_qx in self.qic.tgt_qs_of_src_q(src_qx):
                #print src_qx, tgt_qx
                src_tgt_mappings = self.qic.src_tgt_mappings(src_qx, tgt_qx)
                for (src_q0, src_q1, tgt_q0, tgt_q1, src_from_tgt_q) in src_tgt_mappings:
                    #print src_q0, tgt_q0, src_q1, tgt_q1, src_from_tgt_q
                    add_to_list = True
                    for src_from_tgt_qx in mappings_to_try:
                        dq = src_from_tgt_q.distance_to(src_from_tgt_qx)
                        if dq<min_q_dist:
                            add_to_list = False
                            break
                        if dq>max_q_dist:
                            add_to_list = False
                            break
                        pass
                    if not add_to_list: continue
                    mappings_to_try.append(src_from_tgt_q)
                    if (len(mappings_to_try)%500)==0:
                        print "Have %d mappings, continuing"%len(mappings_to_try)
                        pass
                    pass
                pass
            pass
        return mappings_to_try
    #f find_best_target_matches_qic
    def find_best_target_matches_qic(self,
                                 min_q_dist=0.00004, # minimum separation betwen src_from_tgt_q's to try
                                 max_q_dist=0.5,
                                 min_cos_sep_score=0.9995,
                                 max_q_dist_score=0.02,
                                 min_cos_sep="Not used", min_used_factor="not used"):
        cbm = self.c_best_match
        result = []
        mappings_to_try = self.find_mappings_to_try_qic(min_q_dist, max_q_dist)
        print "Trying %d mappings"%(len(mappings_to_try))
        for src_from_tgt_q in mappings_to_try:
            score = self.qic.score_orient(src_from_tgt_q)
            if (score>2):
                result.append(self.c_best_match(src_from_tgt_q, score, self.qic))
                pass
            pass
        return result
    #f times
    def times(self):
        return {"image_match":self.im.times()}
    #f ALL DONE

#a Do it
#f do it
def do_it(images, focal_length, lens_type, max_iteration_depth=2, reverse=0, initial_dest_orientation=None):

    ipqm = c_image_pair_quaternion_match()
    ipqm.add_image(image_filename=images[0], orientation=gjslib_c.quaternion(r=1), focal_length=focal_length, lens_type=lens_type )
    ipqm.add_image(image_filename=images[1], orientation=gjslib_c.quaternion(r=1), focal_length=focal_length, lens_type=lens_type )

    iq = gjslib_c.quaternion(r=1)
    if reverse:
        iq = iq.from_euler(roll=180,degrees=1)
        pass
    trial_orientations = [(iq, 0)]
    if initial_dest_orientation is not None:
        trial_orientations = [(initial_dest_orientation,0)]
        pass
    orientations_attempted = set()
    results = []
    while len(trial_orientations)>0:
        (initial_dest_orientation, iteration_depth) = trial_orientations.pop(0)
        s = (str(initial_dest_orientation), iteration_depth)
        if s in orientations_attempted:
            print "Skipping as already attempted"
            continue
        orientations_attempted.add(s)

        ipqm.orient(images[1], initial_dest_orientation)

        best_matches = quaternion_image_correlate(ipqm, images, focal_length=50.0, accuracy=["80pix35", "20pix35", "8pix35"][iteration_depth] )

        for bm in best_matches:
            bm.src_from_tgt_q *= initial_dest_orientation
            bm.optimized_src_from_tgt_q *= initial_dest_orientation
            rq0 = bm.src_from_tgt_q
            rq1 = bm.optimized_src_from_tgt_q
            print bm.max_distance, "(r=%f,i=%f,j=%f,k=%f)"%(rq0.r,rq0.i,rq0.j,rq0.k), "(r=%f,i=%f,j=%f,k=%f)"%(rq1.r,rq1.i,rq1.j,rq1.k), rq1.to_rotation_str(1)

            if (len(results)==0) or (bm.max_distance>[10,20,30][iteration_depth]):
                results.append((bm,iteration_depth,(iteration_depth+1)*100*bm.max_distance))
                if iteration_depth<max_iteration_depth:
                    trial_orientations.append( (bm.optimized_src_from_tgt_q, iteration_depth+1) )
                    pass
                pass
            pass
        pass
    results.sort(cmp=lambda x,y:cmp(y[2],x[2]))
    for (bm, iteration_depth, score) in results:
        rq0 = bm.src_from_tgt_q
        rq1 = bm.optimized_src_from_tgt_q
        print iteration_depth, bm.max_distance, "(r=%f,i=%f,j=%f,k=%f)"%(rq0.r,rq0.i,rq0.j,rq0.k), "(r=%f,i=%f,j=%f,k=%f)"%(rq1.r,rq1.i,rq1.j,rq1.k), rq1.to_rotation_str(1)
        pass
    return results

#a Toplevel
import getopt
print sys.argv
long_opts = [ 'image_dir=', 'focal_length=', 'fine', 'initial_dest_orientation=', 'lens_type=', 'max_iteration_depth=', 'output=', 'reverse=' ]
optlist,args = getopt.getopt(sys.argv[1:], '', long_opts)
image_dir = ""
focal_length = 35.0
lens_type = 'rectilinear'
max_iteration_depth = 2
output_filename = None
reverse = 0
initial_dest_orientation = None
for (opt, value) in optlist:
    if opt in ["--image_dir"]:
        image_dir = value
        pass
    if opt in ["--focal_length"]:
        focal_length = float(value)
        pass
    if opt in ["--lens_type"]:
        lens_type = value
        pass
    if opt in ["--max_iteration_depth"]:
        max_iteration_depth = int(value)
        pass
    if opt in ["--reverse"]:
        reverse = int(value)
        pass
    if opt in ["--initial_dest_orientation"]:
        q = eval(value)
        initial_dest_orientation = gjslib_c.quaternion(r=q[0],i=q[1],j=q[2],k=q[3])
        pass
    if opt in ["--output"]:
        output_filename = value
        pass
    pass
if len(args)!=2:
    print >>sys.stderr, "Expected two image names"
    sys.exit(4)

images = args
tb = initialize(size=1024, num_textures=12)
full_image_filenames = (image_dir+images[0], image_dir+images[1])
print "Running QIC on",full_image_filenames,"at focal length",focal_length,"type",lens_type,"max_iter",max_iteration_depth
results = do_it( images=full_image_filenames, focal_length=focal_length, lens_type=lens_type, max_iteration_depth=max_iteration_depth, reverse=reverse, initial_dest_orientation=initial_dest_orientation)

f=sys.stdout
if output_filename is not None:    
    f = open(output_filename,"a")
    pass
iteration_depths_written = set()
optimized_qs_written = set()
output_data = {}
output_data["best_qs"]=[]
output_data["optimized_qs"]=[]

for (bm, iteration_depth, score) in results:
    best_qs      = output_data["best_qs"]
    optimized_qs = output_data["optimized_qs"]
    if iteration_depth not in iteration_depths_written:
        iteration_depths_written.add(iteration_depth)
        while len(best_qs)<=iteration_depth:
            best_qs.append(None)
            pass
        best_qs[iteration_depth] = (iteration_depth,
                                    bm.max_distance,
                                    bm.optimized_src_from_tgt_q.rijk,
                                    bm.src_from_tgt_q.rijk)
        pass
    rq0 = bm.optimized_src_from_tgt_q
    rq0s = "(r=%f,i=%f,j=%f,k=%f)"%(rq0.r,rq0.i,rq0.j,rq0.k)
    if rq0s not in optimized_qs_written:
        optimized_qs_written.add(rq0s)
        optimized_qs.append( (iteration_depth,
                              bm.max_distance,
                              bm.optimized_src_from_tgt_q.rijk,
                              bm.src_from_tgt_q.rijk)
                             )
        pass
    pass
print >>f, "image_map_data[('%s','%s')] = "%(images[0],images[1]),repr(output_data)

