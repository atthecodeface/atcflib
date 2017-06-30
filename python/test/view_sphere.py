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
from gjslib_c import vector, quaternion, lens_projection
from filters import *

vector_x = vector((1,0,0))
vector_y = vector((0,1,0))
vector_z = vector((0,0,1))
lens_projection.add_named_polynomial("canon_20_35_rebel2Ti_20", canon_20_35_rebel2Ti_20_polynomial[0], canon_20_35_rebel2Ti_20_polynomial[1])

#a Classes
#c spherical_angle
def spherical_angle(p, p0, p1):
    """
    A spherical angle is formed from three points on the sphere, p, p0 and p1

    Two great circles (p,p0) and (p,p1) have an angle between them (the rotation
    anticlockwise about p that it takes to move the p,p0 great circle p,p1).
    Hence there is an axis through (p, 0, -p) about which p0 rotates to p1

    Consider the great circle for (p, p0); this is a rotation around
    an axis p x p0. Similarly for (p, p1).  If unit rotation axes are
    chosen (p_p0_r and p_p1_r) then the angle can be determined from
    p_p0_r.p_p1_r (cos of angle) and p.(p_p0_r x p_p1_r) (sin of angle)
    """
    p_p0_r = p.cross_product(p0).normalize()
    p_p1_r = p.cross_product(p1).normalize()
    ps = p_p0_r.cross_product(p_p1_r).dot_product(p)
    pc = p_p0_r.dot_product(p_p1_r)
    return math.atan2(ps, pc)

#f vector_coord_list
def vector_coord_list(vl):
    return [v.coords for v in vl]

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


#a Polyhedra classes
class c_polyhedron(object):
    """
    Want an area<>area mapping for each triangle, from sphere to texture
    plot as triangles where color of triangle = green for 1.0, red for area_texture/area_sphere<1, and blue for area_texture/areas_sphere>1
    Also try pentakis dodecahedron
    """
    vertices = []
    faces = []
    edges = []
    class c_face(object):
        def __init__(self, edge_dirns):
            self.edge_dirns = edge_dirns
            self.midpoints = []
            for (d,e) in self.edge_dirns:
                self.midpoints.append(e.midpoint())
                pass
            self.corners = [ self.edge_dirns[0][1].p0, self.edge_dirns[0][1].p1, self.edge_dirns[2][1].p1 ]
            if self.edge_dirns[0][0]: self.corners[0]=self.edge_dirns[0][1].p1
            if self.edge_dirns[0][0]: self.corners[1]=self.edge_dirns[0][1].p0
            if not self.edge_dirns[2][0]: self.corners[2]=self.edge_dirns[2][1].p0
            #self.corners = [self.corners[0], self.corners[2], self.corners[1]]
            self.subfaces = None
            pass
        def area(self):
            area = -math.pi
            for i in range(3):
                area += spherical_angle( self.corners[(i+2)%3],
                                         self.corners[i],
                                         self.corners[(i+1)%3] )
                pass
            return area
        def midpoint(self, edge):
            return self.midpoints[edge]
        def subdivide(self):
            if self.subfaces is not None:
                return self.subfaces
            subedges = ( c_great_circle_arc(self.corners[0], self.midpoints[0]),
                         c_great_circle_arc(self.midpoints[0], self.corners[1]),
                         c_great_circle_arc(self.corners[1], self.midpoints[1]),
                         c_great_circle_arc(self.midpoints[1], self.corners[2]),
                         c_great_circle_arc(self.corners[2], self.midpoints[2]),
                         c_great_circle_arc(self.midpoints[2], self.corners[0]),

                         c_great_circle_arc(self.midpoints[0], self.midpoints[1]),
                         c_great_circle_arc(self.midpoints[1], self.midpoints[2]),
                         c_great_circle_arc(self.midpoints[2], self.midpoints[0]),
                         )
            self.subfaces = []
            for f in ( (1,-9,6), (3,-7,2), (5,-8,4), (7,8,9) ):
                face = []
                for fn in f:
                    face.append((fn<0, subedges[abs(fn)-1]) )
                    pass
                self.subfaces.append(c_polyhedron.c_face(face))
                pass
            return self.subfaces
        pass
    @staticmethod
    def faces_from_vertices(faces, vertices, edges):
        faces_of_edges = []
        for f in faces:
            fe = []
            faces_of_edges.append(fe)
            ab = vertices[f[0]-1] - vertices[f[1]-1]
            cb = vertices[f[2]-1] - vertices[f[1]-1]
            print f,vertices[f[1]-1].dot_product(ab.cross_product(cb))
            if vertices[f[1]-1].dot_product(ab.cross_product(cb))>0:
                raise Exception, "Clockwise face %s"%str(f)
            for i in range(len(f)):
                a = f[i]
                b = f[(i+1)%len(f)]
                if (a,b) in edges:
                    fe.append(1+edges.index((a,b)))
                    pass
                else:
                    fe.append(-1-edges.index((b,a)))
                    pass
                pass
            pass
        return faces_of_edges
    def __init__(self):
        for i in range(len(self.vertices)):
            self.vertices[i].normalize()
            pass
        self.edge_gcs = []
        for e in self.edges:
            self.edge_gcs.append( c_great_circle_arc(self.vertices[e[0]-1], self.vertices[e[1]-1]) )
            pass
        self.face_gcs = []
        for f in self.faces:
            face = []
            for fn in f:
                face.append((fn<0, self.edge_gcs[abs(fn)-1]) )
                pass
            self.face_gcs.append(c_polyhedron.c_face(face))
            pass
        self.all_faces = {}
        pass
    def subface(self, subdivisions):
        if type(subdivisions)==int: return self.face_gcs[subdivisions]
        if len(subdivisions)==1:    return self.face_gcs[subdivisions[0]]
        sd = tuple(subdivisions)
        if sd in self.all_faces:
            return self.all_faces[sd]
        parent = self.subface(subdivisions[:-1])
        subfaces = parent.subdivide()
        return subfaces[subdivisions[-1]]

class c_tetrahedron(c_polyhedron):
    vertices = ( vector((1,1,1)), vector((1,-1,-1)), vector((-1,1,-1)), vector((-1,-1,1)) )
    edges = ( (1,2), (1,3), (1,4), (2,3), (2,4), (3,4) )
    faces = ( (1,2,3), (1,4,2), (1,3,4), (2,4,3) )
    faces = c_polyhedron.faces_from_vertices( faces, vertices, edges )

class c_octahedron(c_polyhedron):
    vertices = ( vector(( 1,0,0)),
                 vector((-1,0,0)),
                 vector((0, 1,0)),
                 vector((0,-1,0)),
                 vector((0,0, 1)),
                 vector((0,0,-1)),
                 )
    edges = ( (1,3), (1,4), (1,5), (1,6),
              (2,3), (2,4), (2,5), (2,6),
              (3,5), (5,4), (4,6), (6,3) )
    faces = ( (1,3,5), (1,5,4), (1,4,6), (1,6,3),
              (3,2,5), (5,2,4), (4,2,6), (6,2,3) )
    faces = c_polyhedron.faces_from_vertices( faces, vertices, edges )
    pass

class c_icosahedron(c_polyhedron):
    """
    A 20mm lens on a 22.3mm sensor is 58 degree horizontal, 40 degree vertical; this is about 1/20th of a sphere
    At 30Mpixel there would be 600Mpixel for the sphere

    subdivision of length 5 (i.e. each face subdivided 4 times)
    has a center that is about .1% away from 1.0 length
    and each subdivision gets it a factor of 4 closer
    subdivison of length 8 closest to a vertex has 0.0014% (1 part in 70,000)
    subdivison of length 8 closest to center has 0.0018% (1 part in 56,000)
    The earth has a radius of 6,000km; so 1 part in 70,000 is about 80m
    Subdivision length 8 is each face divided 7 times, so 4^7 faces per triangle,
    or 2^14 = 16384 faces  - total of 320k faces for the sphere
    Each edge is 1.1755 long, or 7,053km
    So subdivision length 8 is divide each side by 128, or 55km

    Subdivision length 6 is 80k faces for the sphere, and a part in 4,500 (at vertex) or 3,500 (center of face)
    This is an error of 2km at the center.
    Each edge is 1.1755 long, or 7,053km
    So subdivision length 6 is divide each side by 32, or 221km

    At 80k faces, with 600Mpixels for a sphere, that is 8k pixels per face - as half of a square, that is 128 pixels per side of each face
    At 600Mpixels that is 30Mpixels per triangular face, 60Mpixels per face pair - or 8kx8k images

    The base image could be 10Mpixels - or 1Mpixel per face pair, or 1k by 1k for each face edge, 32 pixels per each side of each face
    """
    gr = (math.sqrt(5)-1)/2
    vertices = ( vector(( 1,0, gr)),
                 vector(( 1,0,-gr)),
                 vector((-1,0, gr)),
                 vector((-1,0,-gr)),
                 vector(( gr, 1,0)),
                 vector((-gr, 1,0)),
                 vector(( gr,-1,0)),
                 vector((-gr,-1,0)),
                 vector((0, gr, 1)),
                 vector((0,-gr, 1)),
                 vector((0, gr,-1)),
                 vector((0,-gr,-1)),
                 )
    edges = ( (1,2), (1,5), (1,7), (1,9), (1,10),    #  1 top
              (2,5), (5,9), (9,10), (10,7), (7,2),   #  6 ring 2,5,9,10,7
              (4,3), (4,6), (4,8), (4,11), (4,12),   # 11 bottom
              (3,6), (6,11), (11,12), (12,8), (8,3), # 16 ring 11,6,3,8,12
              (12,2), (2,11), (11,5), (5,6), (6,9),
              (9,3), (3,10), (10,8), (8,7), (7,12),
              )
    faces = ( (1,2,5), (1,5,9), (1,9,10), (1,10,7), (1,7,2),
              (4,3,6), (4,6,11), (4,11,12), (4,12,8), (4,8,3),
              (2,11,5), (11,6,5), (5,6,9), (6,3,9), (9,3,10),
              (3,8,10), (10,8,7), (8,12,7), (7,12,2), (12,11,2),
              )
    faces = c_polyhedron.faces_from_vertices( faces, vertices, edges )
    pass

#f add_shape_subdivision
def add_shape_subdivision(shape, sd, hue, fill=False, color_by_area=True):
    f = shape.subface(sd)
    center = vector((0,0,0))
    for v in f.corners:
        center += v
        pass
    center.scale(1/3.0)
    center_displacement = 1/(1.0-center.modulus())
    rel_area = f.area()*(4**len(sd))/shape.subface(0).area()/4
    if color_by_area:
        if rel_area<0: rel_area=-1/rel_area
        hue = 120 * (1+rel_area/2)
    obj = c_view_sphere_obj( has_surface=True, color=rgb_of_hue(hue), selectable=True,
                             note="%d : %s : %f : %f : %f"%(len(sd),str(sd),f.area(),rel_area,center_displacement) )
    if fill:
        obj.add_triangle(vector_coord_list(f.corners),[(0,0)]*3)
        return obj

    n = 0
    for v in f.corners:
        add_blob(obj, v=v, style=["triangle","diamond","star"][n], scale=0.03/len(sd) )
        n += 1
        pass
    
    for (d,g) in f.edge_dirns:
        obj.add_line( vector_coord_list(g.interpolate()) )
        obj.add_line(( g.p0.coords, g.p1.coords))
        pass
    return obj

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
        rot_q = quaternion.of_rotation(axis=diff_axis, angle=angle)
        points.append( (rot_q*src_q).rotate_vector(vector_z).coords )
        pass
    return points

#f add_blob
def add_blob(obj, src_q=None, v=None, scale=0.001, style="star", distance=1.0):
    angle_rs = {"diamond":      [(i,1) for i in range(0,450,90)],
                "star":         [(i,[1,0.5][((i/36)&1)]) for i in range(0,396,36)],
                "triangle":     [(i,1) for i in range(0,450,120)],
                "inv_triangle": [(i,1) for i in range(60,450,120)],
                }
    if v is not None:
        cxyz = v.coords
        dv = vector_z
        if (cxyz[0]==0) and (cxyz[0]==0): dv=vector_y
        src_q = quaternion().lookat(v, dv)
        pass
    else:
        cxyz= src_q.rotate_vector(vector((0,0,distance))).coords
        pass
    lxyz = None
    for (angle,r) in angle_rs[style]:
        vector_z_sc = vector((0,scale*r,distance))
        xyz = (src_q*quaternion().from_euler(roll=angle,degrees=1)).rotate_vector(vector_z_sc).coords
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

    dst_w = 1024.0
    src_w = 1.0
    src_h = 1.0
    src_ar = 3456/5148.0

    separate = False
    #separate = True

    #for rect
    src_ar = 1
    src_h = 3456/5148.0

    s60 = math.sin(math.radians(60))

    image_orientations = []
    orientations = []
    image_objects = []
    n = 0
    hue = 0

    obj = c_view_sphere_obj(has_surface=True, color=rgb_of_hue(hue), selectable=True,
                            note="stars")
    image_objects.append(obj)

    shape = c_tetrahedron()
    #shape = c_octahedron()
    shape = c_icosahedron()
    for v in shape.vertices:
        add_blob(obj, v=v, style="star", scale=0.02 )
        pass

    for g in shape.edge_gcs:
        cs = []
        for p in g.interpolate():
            cs.append(p.coords)
            pass
        obj.add_line(cs)
        cs = ( g.p0.coords, g.p1.coords)
        obj.add_line(cs)
        pass

    subdivisions_to_show = ( (0,), (0,0,0,0,0,0), (0,1), (0,1,1), (0,3), (0,3,0), (0,3,0,3), (0,3,0,3,3), (3,3,3,3,3,3,3) )
    subdivisions_to_show = ( (0,0,0,0,0,0,0,0), (0,3,0,3), (0,3,0,3,3), (0,3,3,3,3,3,3,3) )
    subdivisions_to_show = []
    subdivisions_to_show2 = []
    subdivisions_to_show3 = []
    for i in [0,3]:
        for j in range(4):
            for k in range(4):
                #if (i,j,k) not in [(0,0,0),(3,3,3)]:
                #    subdivisions_to_show.append( (0,i,j,k) )
                #    pass
                subdivisions_to_show2.append( (0,0,0,0,i,j,k) )
                subdivisions_to_show3.append( (0,3,3,3,i,j,k) )
            pass
        pass
    subdivisions_to_show.extend(subdivisions_to_show2)
    subdivisions_to_show.extend(subdivisions_to_show3)
    subdivisions_to_show = ( (0,), )
    subdivisions_to_show = []
    for i in range(len(shape.faces)):
        #subdivisions_to_show.append((i,))
        subdivisions_to_show.append((i,3,3,3,3,3))
        subdivisions_to_show.append((i,0,3))
        #subdivisions_to_show.append((i,1,3))
        #subdivisions_to_show.append((i,2,3))
        subdivisions_to_show.append((i,0,0,3))
        subdivisions_to_show.append((i,0,0,0,3))
        subdivisions_to_show.append((i,0,0,0,0,3))
        subdivisions_to_show.append((i,0,0,0,0,0))
        #subdivisions_to_show.append((i,0,1,3))
        #subdivisions_to_show.append((i,0,2,3))
        pass
    for sd in subdivisions_to_show:
        obj = add_shape_subdivision(shape, sd, hue, fill=True)
        image_objects.append(obj)
        hue += 44
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

