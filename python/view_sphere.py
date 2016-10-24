#!/usr/bin/env python
# PYTHONPATH=`pwd`/../python:$PYTHONPATH ./view_camera.py
# PYTHONPATH=`pwd`/../python:`pwd`/..:`pwd`/gjslib/../python::`pwd`/../../gjslib/python:$PYTHONPATH ./view_sphere.py

#a Imports
from gjslib.graphics import opengl_app, opengl_utils, opengl_obj

import math
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import gjslib_c
from filters import *
gjslib_c.lens_projection.add_named_polynomial("canon_20_35_rebel2Ti_20", canon_20_35_rebel2Ti_20_polynomial[0], canon_20_35_rebel2Ti_20_polynomial[1])

#a Classes
#c c_great_circle_arc
class c_great_circle_arc(object):
    """
    Points on a great circle are p such that p.n=0, where |p| =
    1. Effectively the points on the plane through the origin that has
    the great circle axis as its normal, where the points are distance
    1 from the origin (i.e. a circle radius one on the plane centered
    at the origin).

    An arc on the great circle is between two points p0 and p1, both
    on the great circle. These points can be described by the axis
    and, for each, and angle.

    As the are two halves to the great circle split by the points p0
    and p1, we have to define 'between'. We do this by defining the
    points segment 'between' to be the anticlockwise turn from p0 to
    p1; this reinforces the axis as p0 x p1.

    There is thus a set of three vectors gx, gz, gy: p0, axis, and
    axis x p0.  These are mutually orthogonal. p1 is then a linear
    combination of gx and gy (p1.gx, and p1.gy).

    If p1 is not in the first quadrant, then p can first be checked to
    see if it is in any quadrant between p1 and p0 except the first.
    The quadrant is the first if p.gx and p.gy are both positive; the
    second if p.gx is negative and p.gy is positive; the third if p.gx
    is negative and p.gy is negative; the fourth if p.gx is positive
    and p.gy is negative. (1,1, -1,1, -1,-1, 1,-1)

    Hence the quadrant of p is (1-sign(p.gx)*sign(p.gy))/2+(1-sign(p.gy)),
    or (3-sign(p.gy)*(2+sign(p.gx)))/2
    [ (3-1*(2+1))/2, (3-1*(2-1))/2, (3+1*(2-1))/2,  (3+1*(2+1))/2 ]
    [ 0, 1, 2, 3 ]

    So, find out the quadrant of p; if p is in the same quadrant as
    p1, then rotate both back into the first quadrant.
    In quadrant 1, map (p.gx,p.gy) to (p.gy,-p.gx)
    In quadrant 2, map (p.gx,p.gy) to (-p.gx,-p.gy)
    In quadrant 3, map (p.gx,p.gy) to (-p.gy,p.gx)

    If p1 is in the first quadrant (p0->p1 is < 90 degrees) then
    points between p0 and p1 will have p0.gx==1>=p.gx>=p1.gx,
    and p0.gy(==0)<p.gy<p1.gy
    """
    #f __init__
    def __init__(self, p0, p1):
        self.p0 = p0
        self.p1 = p1
        self.axis = p0.cross_product(p1)
        self.axis.normalize()
        self.gx = p0
        self.gy    = (self.axis.cross_product(self.gx)).normalize()
        self.gy_p1 = (self.axis.cross_product(self.p1)).normalize()
        self.p1_quad_gx_gy = self.quad_coords(p1)
        pass
    #f midpoint
    def midpoint(self):
        return (self.p0+self.p1).normalize()
        return (self.gy - self.gy_p1).scale(0.5)
    #f interpolate
    def interpolate(self, num_points=20):
        r = []
        sin_angle = self.p1_quad_gx_gy
        q = math.floor(sin_angle)
        sin_angle -= q
        angle = math.asin(sin_angle) + q/2*math.pi
        for i in range(num_points):
            ia = (angle*i)/(num_points-1)
            r.append( self.gx.copy().scale(math.cos(ia)) + self.gy.copy().scale(math.sin(ia)) )
            pass
        return r
    #f quad_coords
    def quad_coords(self, p):
        """
        Calculate the quadrant of p, and gx/gy given p
        rotated back by 90 degree increments to get it
        into the first quadrant
        Then return a measure of how far round;
        this can be gy in the first quadrant (0<gy<1),
        and therefore quadrant+gy
        """
        gx = p.dot_product(self.gx)
        gy = p.dot_product(self.gy)
        if gy<0:
            if gx>0: return 3+gx
            return 2-gy
        if gx<0: return 1-gx
        return gy
    #f is_on_arc
    def is_on_arc(self, quad_gx_gy):
        """
        p0 is at quad=0, gx=1, gy=0
        p1 is at self.p1_quad_gx_gy

        a point is on the arc if its quad < p1's quad
        OR
        if its quad == p1's quad AND its gx > p1's gx
        """
        if quad_gx_gy>self.p1_quad_gx_gy: return False
        return True
    #f intersect
    def intersect(self, other):
        """
        All points a the GC have p.n=0
        Points where two great circles meet have p.n1 = p.n2 = 0

        A clear solution to this is p = k * (n1 x n2),
        as p.n1 = k*(n1 x n2).n1 = k*0 = 0, and similarly for p.n2
        k can be any real number.

        For points that are on the GC, they are distance 1 from the
        origin. Hence k = +-1, if n1 and n2 are unit vectors.

        Hence the intersection of two great circles is +- n1 x n2

        The questions then are: which of the two intersection points
        are we interested in, and does the intersection point lie
        'between' p0 and p1 for each great circle.

        Since an intersection point is +n1 x n2, we can find (pa.gx,
        pa.gy) for great circle A and (pb.gx, pb.gy) for great circle
        B. Note that -n1 x n2 is (-pa.gx, -pb.gy) and (-pb.gx, -pb.gy):
        or quadrant+2 of +n1 x n2.

        Also, it is good to know the sense of the crossings at an
        intersection. The 'sense' is whether pb0 is on the 'left' as
        one travels from pa0 to pa1; i.e. whether pb0-pb1.(pan) is +ve
        or -ve
        """
        p = self.axis.cross_product(other.axis)
        p.normalize()
        pa_quad_gx_gy = self.quad_coords(p)
        pb_quad_gx_gy = other.quad_coords(p)
        pb_sense_across_pa = cmp(self.axis.dot_product(other.p1-other.gx),0.0)
        pa_sense_across_pb = cmp(other.axis.dot_product(self.p1-self.gx),0.0)
        if (self.is_on_arc(pa_quad_gx_gy) and
            other.is_on_arc(pb_quad_gx_gy)): return (p, pa_quad_gx_gy, pb_quad_gx_gy, pb_sense_across_pa, pa_sense_across_pb)
        pa_quad_gx_gy += 2
        if pa_quad_gx_gy>4: pa_quad_gx_gy-=4
        pb_quad_gx_gy += 2
        if pb_quad_gx_gy>4: pb_quad_gx_gy-=4
        if (self.is_on_arc(pa_quad_gx_gy) and
            other.is_on_arc(pb_quad_gx_gy)): return (-p, pa_quad_gx_gy, pb_quad_gx_gy, pb_sense_across_pa, pa_sense_across_pb)
        return None
        

#c c_view_obj
class c_view_obj(opengl_app.c_opengl_camera_app):
    #f __init__
    def __init__(self, obj, **kwargs):
        opengl_app.c_opengl_camera_app.__init__(self, **kwargs)
        self.objects = obj
        self.xxx = 0.0
        self.yyy = 0.0
        self.window_title = "Viewing object"
        self.selection=None
        self.camera["position"] = [0,0,0]
        self.zNear=0.2
        #self.camera["facing"]   = quaternion.pitch(90,degrees=True) * self.camera["facing"]
        from gjslib.math.quaternion import quaternion
        self.camera["facing"]   = quaternion.pitch(-90,degrees=True) * quaternion.roll(90,degrees=True) * self.camera["facing"]
        pass
    #f opengl_post_init
    def opengl_post_init(self):
        import OpenGL.arrays.vbo as vbo
        import numpy
        self.tick = 0
        
        for o in self.objects:
            if o.is_textured:
                o.gjslib_texture = gjslib_c.texture(filename=o.texture_filename)
                o.texture_gl_id = o.gjslib_texture.gl_id
                pass
            if o.has_surface:
                o.create_opengl_surface()
                pass
            pass
        vector_list = []
        for xyz in [(0,0,4), (0,4,0), (4,0,0)]:
            vector_list.extend( [0.0, 0.0, 0.0] )
            vector_list.append( float(xyz[0]) )
            vector_list.append( float(xyz[1]) )
            vector_list.append( float(xyz[2]) )
            pass
        import gjslib.math.spherical_coords
        sph = gjslib.math.spherical_coords.c_spherical_coord()
        latitudes = range(-80,90,10)
        latitudes.append(1)
        latitudes.append(-1)
        for lat in latitudes:
            last_xyz = None
            for ang in range(0,361,10):
                sph.from_spherical((math.radians(ang),math.radians(lat)))
                (x,y,z) = sph.xyz()
                xyz = (z*0.99,y*0.99,x*0.99)
                if last_xyz is not None:
                    vector_list.extend(last_xyz)
                    vector_list.extend(xyz)
                    pass
                last_xyz = xyz
                pass
            pass
        longitudes = range(0,180,10)
        longitudes.append(1)
        longitudes.append(-1)
        for longitude in longitudes:
            last_xyz = None
            for ang in range(0,361,10):
                sph.from_spherical((math.radians(longitude),math.radians(ang)))
                (x,y,z) = sph.xyz()
                xyz = (z*0.99,y*0.99,x*0.99)
                if last_xyz is not None:
                    vector_list.extend(last_xyz)
                    vector_list.extend(xyz)
                    pass
                last_xyz = xyz
                pass
            pass
        self.opengl_lines = vbo.VBO( data=numpy.array(vector_list, dtype=numpy.float32), target=GL_ARRAY_BUFFER )
        pass
    #f display
    def display(self):
        glClearColor(0.4,0.4,0.4,1.0)
        opengl_app.c_opengl_camera_app.display(self)

        self.yyy += 0.03
        lightZeroPosition = [4.+3*math.sin(self.yyy),4.,4.-3*math.cos(self.yyy),1.]
        lightZeroColor = [0.7,1.0,0.7,1.0] #white
        ambient_lightZeroColor = [1.0,1.0,1.0,1.0] #green tinged

        #glPushMatrix()
        #color = [1.0,0.,0.,1.]
        #glTranslate(lightZeroPosition[0],lightZeroPosition[1],lightZeroPosition[2])
        #glScale(0.3,0.3,0.3)
        #glutSolidSphere(2,40,40)
        #glPopMatrix()

        color = [0.5,0,0.,0.,1.]

        self.matrix_push()
        self.matrix_rotate(95,(1,0,0))
        #self.xxx += 0.2#0.1
        brightness = 0.4
        self.matrix_rotate(self.xxx,(0,0,1))

        self.shader_use("color_standard")
        self.matrix_use()
        self.opengl_lines.bind()
        self.shader_set_attributes( t=3, v=0, C=(0.5,0.5,0.5) ) # calls bind_vbo with args for selected shader
        #glEnableVertexAttribArray(self.attrib_ids[k])
        #glVertexAttribPointer(self.attrib_ids[k], s, GL_FLOAT, GL_FALSE, t*sizeof(c_float), c_void_p(d*sizeof(c_float)) )
        glDrawArrays( GL_LINES, 0, len(self.opengl_lines)/3 )
        self.opengl_lines.unbind()

        n = 0
        for o in self.objects:
            n = n + 1
            if not o.should_display(self.selection, n, self.tick): continue
            self.matrix_push()
            if o.is_textured:
                glBindTexture(GL_TEXTURE_2D, o.texture_gl_id)
                self.shader_use("texture_standard")
                pass
            else:
                self.shader_use("color_standard")
                pass
            self.matrix_use()
            if o.has_surface:
                o.draw_opengl_surface(self, C=o.color)
                pass
            self.matrix_pop()
            pass

        self.matrix_pop()
        self.tick = self.tick+1

        glutSwapBuffers()
        return
    #f keypress
    def keypress(self, key,m,x,y):
        if key=='0':
            self.selection = None
            return True
        if key=='1':
            if self.selection is None:
                self.selection = 0
                pass
            else:
                self.selection = self.selection+1
                pass
            print "Selected",self.selection
            return True
        if key=='2':
            if self.selection is None:
                self.selection = 0
                pass
            else:
                self.selection = self.selection-1
                pass
            print "Selected",self.selection
            return True
        return opengl_app.c_opengl_camera_app.keypress(self, key, m, x, y)
    #f All done
    pass

#a Top level
#c c_view_sphere_obj
class c_view_sphere_obj(opengl_obj.c_opengl_obj):
    color = None
    selectable_id = 0
    last_note_shown = None
    def __init__(self, has_surface=False, texture_filename=None, color=None, selectable=None, note=None):
        opengl_obj.c_opengl_obj.__init__(self)
        self.color = color
        self.has_surface = has_surface
        self.texture_filename = texture_filename
        self.is_textured = False
        if texture_filename is not None:
            self.is_textured = True
            pass
        self.selectable = False
        self.selectable_id = None
        if selectable is not None:
            self.selectable = True
            if selectable:
                c_view_sphere_obj.selectable_id += 1
                pass
            self.selectable_id = c_view_sphere_obj.selectable_id
            pass
        self.note = note
        print selectable, c_view_sphere_obj.selectable_id
        #if note is not None:
        #    print self.selectable_id,note
        pass
    pass
    def should_display(self, selection, n, tick):
        return True
        if not self.selectable:
            return True
            if (n%2) == (tick%2):
                return False
            return True
        if selection is None:
            return True
        if c_view_sphere_obj.selectable_id==0: return True
        if (selection%c_view_sphere_obj.selectable_id)==(self.selectable_id%c_view_sphere_obj.selectable_id):
            if ((c_view_sphere_obj.last_note_shown!=self.selectable_id) and
                self.note is not None):
                print self.selectable_id,self.note
                c_view_sphere_obj.last_note_shown = self.selectable_id
                pass
            return True
        return False

#f line_between_qs
def line_between_qs(src_q, tgt_q, angle_steps = 20):
    diff_q = src_q.angle_axis(tgt_q, vector_z)
    (diff_angle, diff_axis) = diff_q.to_rotation()

    points = []
    for j in range(angle_steps+1):
        angle = diff_angle/angle_steps * j
        rot_q = gjslib_c.quaternion.of_rotation(axis=diff_axis, angle=angle)
        points.append( (rot_q*src_q).rotate_vector(vector_z).coords )
        pass
    return points

#f add_blob
def add_blob(obj, src_q, scale=0.001, style="star", distance=1.0):
    angle_rs = {"diamond":      [(i,1) for i in range(0,450,90)],
                "star":         [(i,[1,0.5][((i/36)&1)]) for i in range(0,396,36)],
                "triangle":     [(i,1) for i in range(0,450,120)],
                "inv_triangle": [(i,1) for i in range(60,450,120)],
                }
    cxyz= src_q.rotate_vector(gjslib_c.vector((0,0,distance))).coords
    lxyz = None
    for (angle,r) in angle_rs[style]:
        vector_z_sc = gjslib_c.vector((0,scale*r,distance))
        xyz = (src_q*gjslib_c.quaternion().from_euler(roll=angle,degrees=1)).rotate_vector(vector_z_sc).coords
        if lxyz is not None:
            obj.add_triangle([cxyz,lxyz,xyz],[(0,0)]*3)
            pass
        lxyz = xyz
        pass
    pass

#f rgb_of_hue
def rgb_of_hue(hue):
    """
    0   -> red
    60  -> yellow
    120 -> green
    180 -> cyan
    240 -> blue
    300 -> magenta
    """
    i = int(hue/60.0) % 6
    o = (hue-i*60.0)/60.0
    rgb = ( (1, 1-o, 0, 0,   o, 1  )[i],
            (o, 1,   1, 1-o, 0, 0  )[i],
            (0, 0,   o, 1,   1, 1-o)[i])
    return rgb

#f test_object 
def test_object():

    lens_projection = gjslib_c.lens_projection
    quaternion      = gjslib_c.quaternion
    vector          = gjslib_c.vector

    dst_w = 1024.0
    src_w = 1.0
    src_h = 1.0
    src_ar = 3456/5148.0

    separate = False
    #separate = True

    #for rect
    src_ar = 1
    src_h = 3456/5148.0

    vector_x = vector(vector=(1,0,0))
    vector_y = vector(vector=(0,1,0))
    vector_z = vector(vector=(0,0,1))

    s60 = math.sin(math.radians(60))
    tetrahedron = []
    tetrahedron.append( vector((1,1,1)).normalize() )
    tetrahedron.append( vector((1,-1,-1)).normalize() )
    tetrahedron.append( vector((-1,1,-1)).normalize() )
    tetrahedron.append( vector((-1,-1,1)).normalize() )

    t_gcs = []
    t_gcs.append( c_great_circle_arc(tetrahedron[2], tetrahedron[3]) )
    t_gcs.append( c_great_circle_arc(tetrahedron[3], tetrahedron[0]) )
    t_gcs.append( c_great_circle_arc(tetrahedron[0], tetrahedron[2]) )
    t_gcs.append( c_great_circle_arc(tetrahedron[1], tetrahedron[2]) )
    t_gcs.append( c_great_circle_arc(tetrahedron[0], tetrahedron[1]) )
    t_gcs.append( c_great_circle_arc(tetrahedron[3], tetrahedron[1]) )

    t_faces = ( (0,1,2), (0,5,3), (1,4,-5), (2,-3,-4) )

    image_orientations = []
    orientations = []
    image_objects = []
    n = 0
    hue = 0

    obj = c_view_sphere_obj(has_surface=True, color=rgb_of_hue(hue), selectable=True,
                            note="stars")
    image_objects.append(obj)
    for p in tetrahedron:
        tq = quaternion(1).lookat(p, vector_y)
        print tq, tq.rotate_vector(vector_z)
        add_blob(obj, tq, style="star", scale=0.1 )
        pass
    for g in t_gcs:
        cs = []
        for p in g.interpolate():
            cs.append(p.coords)
            pass
        obj.add_line(cs)
        cs = ( g.p0.coords, g.p1.coords)
        obj.add_line(cs)
        pass

    t_subfaces = []
    for f in t_faces[0:1]:
        hue += 44
        obj = c_view_sphere_obj(has_surface=True, color=rgb_of_hue(hue), selectable=True,
                                note="centers")
        image_objects.append(obj)
        midpoints = ( t_gcs[abs(f[0])].midpoint(), t_gcs[abs(f[1])].midpoint(), t_gcs[abs(f[2])].midpoint())
        for m in midpoints:
            tq = quaternion(1).lookat(m, vector_y)
            add_blob(obj, tq, style="triangle", scale=0.1 )
            pass
        gcs = ( c_great_circle_arc(midpoints[0], midpoints[1]),
                c_great_circle_arc(midpoints[1], midpoints[2]),
                c_great_circle_arc(midpoints[2], midpoints[0]) )
        t_subfaces.append(gcs)
        for g in gcs:
            cs = []
            for p in g.interpolate():
                cs.append(p.coords)
                pass
            obj.add_line(cs)
            cs = ( g.p0.coords, g.p1.coords)
            obj.add_line(cs)
            pass
        pass

    for f in t_subfaces:
        hue += 44
        obj = c_view_sphere_obj(has_surface=True, color=rgb_of_hue(hue), selectable=True,
                                note="centers")
        image_objects.append(obj)
        midpoints = ( f[0].midpoint(), f[1].midpoint(), f[2].midpoint())
        for m in midpoints:
            tq = quaternion(1).lookat(m, vector_y)
            add_blob(obj, tq, style="diamond", scale=0.03 )
            pass
        gcs = ( c_great_circle_arc(midpoints[0], midpoints[1]),
                c_great_circle_arc(midpoints[1], midpoints[2]),
                c_great_circle_arc(midpoints[2], midpoints[0]) )
        #t_subfaces.append(gcs)
        for g in gcs:
            cs = []
            for p in g.interpolate():
                cs.append(p.coords)
                pass
            obj.add_line(cs)
            cs = ( g.p0.coords, g.p1.coords)
            obj.add_line(cs)
            pass
        pass

    og = c_view_obj(obj=image_objects,
                    window_size=(1000,1000))
    og.init_opengl()
    og.camera["fov"] = 90

    og.zFar = 100.0
    og.main_loop()

#f Main
if __name__ == '__main__':
    test_object()

