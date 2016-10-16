#!/usr/bin/env python
# PYTHONPATH=`pwd`/../python:$PYTHONPATH ./view_camera.py
# PYTHONPATH=`pwd`/../python:`pwd`/..:`pwd`/gjslib/../python::`pwd`/../../gjslib/python:$PYTHONPATH ./view_camera.py

#a Imports
from gjslib.graphics import opengl_app, opengl_utils, opengl_obj

import math
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import gjslib_c
vector_z = gjslib_c.vector(vector=(0,0,1))

#a Classes
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
                xyz = (z*1.1,y*1.1,x*1.1)
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
                xyz = (z*1.1,y*1.1,x*1.1)
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
            return True
        if key=='2':
            if self.selection is None:
                self.selection = 0
                pass
            else:
                self.selection = self.selection-1
                pass
            return True
        return opengl_app.c_opengl_camera_app.keypress(self, key, m, x, y)
    #f All done
    pass

#a Top level
#f test_object
def conjugation(q,p):
    qc = q.copy().conjugate()
    #print p[0], p[1], p[2]
    pq = quaternion(r=0,i=p[0],j=p[1],k=p[2])
    r = q*pq*qc
    #print r
    return r.get()[1:]

#c c_view_camera_obj
class c_view_camera_obj(opengl_obj.c_opengl_obj):
    color = None
    selectable_id = -1
    def __init__(self, has_surface=False, texture_filename=None, color=None, selectable=None):
        opengl_obj.c_opengl_obj.__init__(self)
        self.color = color
        self.has_surface = has_surface
        self.texture_filename = texture_filename
        self.is_textured = False
        if texture_filename is not None:
            self.is_textured = True
            pass
        self.selectable = False
        if selectable is not None:
            self.selectable = True
            if selectable:
                c_view_camera_obj.selectable_id += 1
                pass
            self.selectable_id = c_view_camera_obj.selectable_id
        pass
    pass
    def should_display(self, selection, n, tick):
        if not self.selectable:
            if (n%2) == (tick%2):
                return False
            return True
        if selection is None:
            return True
        if (selection%c_view_camera_obj.selectable_id)==self.selectable_id:
            return True
        return False

#f build_add_projected_image_mesh
def build_add_projected_image_mesh(obj,camera,src_w,src_h,n=32,z=8):
    triangles = []
    for x in range(n):
        for y in range(n):
            x0 = (x/float(n)-0.5)
            x1 = ((x+1)/float(n)-0.5)
            y0 = src_h*(y/float(n)-0.5)
            y1 = src_h*((y+1)/float(n)-0.5)
            triangles.append( ( (x0,y0), (x1,y0), (x0,y1) ) )
            triangles.append( ( (x1,y1), (x1,y0), (x0,y1) ) )
            pass
        pass

    for triangle in triangles:
        src_xyzs = []
        src_uvs = []
        for xy in triangle:
            # points for 'src' mesh - also uses 'src' as the image texture, hence needs uv in 'src' terms
            src_xy = (xy[0]*src_w, xy[1]*src_w)
            src_q = camera.orientation_of_xy(src_xy)
            xyz = src_q.rotate_vector(gjslib_c.vector((0,0,z)))
            src_xyzs.append(xyz.coords)
            src_uvs.append(((xy[0]+1.0/2)/1,(1.0/2+xy[1]/src_h)/1))
            pass
        obj.add_triangle( xyz_list = src_xyzs,
                          uv_list = src_uvs )
        pass
    return obj

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

#f rgb_of_hue
def rgb_of_hue(hue):
    i = int(hue/60.0) % 6
    o = (hue-i*60.0)/60.0
    rgb = ( (o, 1, 1, 1-o, 0, 0, o, 1, 1, 1-0)[i],
            (o, 1, 1, 1-o, 0, 0, o, 1, 1, 1-0)[i+2],
            (o, 1, 1, 1-o, 0, 0, o, 1, 1, 1-0)[i+4])
    return rgb

#f test_object
def test_object():

    lens_projection = gjslib_c.lens_projection
    quaternion      = gjslib_c.quaternion

    dst_w = 1024.0
    src_w = 1.0
    src_h = 1.0
    src_ar = 3456/5148.0

    separate = False

    #for rect
    src_ar = 1
    src_h = 3456/5148.0

    image_orientations = [ ("../images/IMG_1900.JPG", None, ~quaternion(r=0.360925,i=0.049321,j=0.126091,k=0.922714)), # focus on muzzy...
                           ("../images/IMG_1901.JPG", 0, quaternion(r=0.994394,i=-0.008970,j=-0.008219,k=-0.105034)
                            ),
                           #(r=0.993401,i=-0.006273,j=-0.007141,k=-0.114297)),
                           # Perfect: (r=0.993378,i=-0.006266,j=-0.007135,k=-0.114499)
                           #(r=0.993378,i=-0.006266,j=-0.007135,k=-0.114499)#(r=0.993450,i=-0.006199,j=-0.007088,k=-0.113879)#(r=0.993624,i=-0.005610,j=-0.006695,k=-0.112407)
#                           ("../images/IMG_1902.JPG", 1, quaternion(r=0.995856,i=-0.009064,j=-0.006932,k=-0.090231)),
                           #(r=0.995798,i=-0.009073,j=-0.006975,k=-0.090860)#(r=0.995947,i=-0.008978,j=-0.006900,k=-0.089229)
                           ]
    focal_length = 35.0
    lens_type = "rectilinear"

    if False:

        image_orientations = [ ("../images/IMG_2173.JPG", None, quaternion(r=1)),
                               ("../images/IMG_2174.JPG", 0, quaternion(r=0.997352,i=-0.005048,j=-0.070954,k=0.015113)
#(r=0.996421,i=0.033653,j=-0.074828,k=0.020332)
                                ),
#(r=0.993533,i=-0.080915,j=0.073880,k=-0.029767))
#(r=0.998303,i=-0.017093,j=-0.055670,k=0.000131)),
                               ]
# Good at 20.7: quaternion(r=0.997465,i=-0.007825,j=-0.069781,k=0.011491)
# Better at 20.6: quaternion(r=0.997539,i=-0.007213,j=-0.067968,k=0.015646)
# Approximately: (r=0.997425,i=-0.007999,j=-0.070465,k=0.010689)
#(r=0.998548,i=-0.004666,j=-0.053508,k=0.004202)
#(r=0.997590,i=-0.021140,j=-0.061052,k=0.025295)
#~quaternion(r=0.997465,i=0.011834,j=0.061485,k=-0.033796)#(r=0.997499,i=0.006470,j=0.067833,k=-0.018762)
#(r=0.997465,i=0.011834,j=0.061485,k=-0.033796)
# given (r=0.997484,i=0.011707,j=0.069910,k=0.000644)...        
# best was (r=0.997583,i=0.006527,j=0.067192,k=-0.016425)
#derived from 24 hits on center match (r=0.997333,i=-0.006595,j=0.060502,k=-0.040275)
        focal_length = 20.0
        focal_length = 20.5
        lens_type = "rectilinear"
        pass

    orientations = []
    image_objects = []
    n = 0
    for (texture_filename, previous_orientation, orientation) in image_orientations:
        if previous_orientation is not None:
            orientation = orientations[previous_orientation] * orientation
            if separate:
                orientation = quaternion().from_euler(pitch=45,degrees=1) * orientations[previous_orientation]
                pass
            pass
        obj = c_view_camera_obj(texture_filename = texture_filename, has_surface=True)
        obj.camera = lens_projection(focal_length=focal_length, lens_type=lens_type, frame_width=22.3*src_ar, width=src_w)
        obj.camera.orient(orientation)
        build_add_projected_image_mesh(obj, obj.camera, src_w, src_h, 64, z=8+n*2)
        image_objects.append(obj)
        orientations.append(orientation)
        #n += 1
        pass
    for i in range(len(orientations)-1):
        src_q = orientations[i]
        tgt_q = orientations[i+1]
        points = line_between_qs(src_q, tgt_q)
        obj = c_view_camera_obj(has_surface=True, color=(1.0,1.0,1.0))
        obj.add_line(points)
        image_objects.append(obj)
        pass
    mappings = None
    try:
        from image_mappings import mappings
        pass
    except:
        pass
    if mappings is not None:
        src_q = orientations[0]
        tgt_q = orientations[1]

        for src_q0 in mappings:
            m = mappings[src_q0]
            src_q0 = src_q * src_q0
            hue = 0
            first_of_mapping = True
            for tgt_q0 in m['tgts']:
                src_tgt_mapping = m['tgts'][tgt_q0]
                tgt_q0 = tgt_q * tgt_q0
                obj = None
                if len(src_tgt_mapping['mappings'])<4: continue
                for (src_q1, tgt_q1, src_from_tgt_orient) in src_tgt_mapping['mappings']:
                    if obj is None:
                        obj = c_view_camera_obj(has_surface=True, color=rgb_of_hue(hue), selectable=first_of_mapping)
                        image_objects.append(obj)
                        hue = (hue+44)%360
                        #first_of_mapping = False
                        pass

                    src_q1 = src_q * src_q1
                    tgt_q1 = tgt_q * tgt_q1
                    obj.add_line(line_between_qs(src_q0, src_q1))
                    obj.add_line(line_between_qs(src_q1, tgt_q1))
                    obj.add_line(line_between_qs(tgt_q0, tgt_q1))
                    pass
                pass
            pass
        pass

    og = c_view_obj(obj=image_objects,
                    window_size=(1000,1000))
    og.init_opengl()
    og.seal_hack = True
    og.camera["fov"] = 90

    og.zFar = 100.0
    #og.create_menus(menus)
    #og.attach_menu("main_menu")
    og.main_loop()

#f Main
if __name__ == '__main__':
    test_object()

