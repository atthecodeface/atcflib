#!/usr/bin/env python
# PYTHONPATH=`pwd`/../python:$PYTHONPATH ./view_camera.py
# PYTHONPATH=`pwd`/../python:`pwd`/..:`pwd`/gjslib/../python::`pwd`/../../gjslib/python:$PYTHONPATH ./view_camera.py

#a Imports
from gjslib.graphics import opengl_app, opengl_utils, opengl_obj

import math
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import python_wrapper

#a Classes
#c c_view_obj
class c_view_obj(opengl_app.c_opengl_camera_app):
    #f __init__
    def __init__(self, obj, texture_filename, **kwargs):
        opengl_app.c_opengl_camera_app.__init__(self, **kwargs)
        self.obj = obj
        self.xxx = 0.0
        self.yyy = 0.0
        self.window_title = "Viewing object"
        self.texture_filename = texture_filename
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
        self.texture = opengl_utils.texture_from_png(self.texture_filename)
        self.obj.create_opengl_surface()
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
                xyz = (z,y,x)
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
                xyz = (z,y,x)
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
        self.shader_set_attributes( t=3, v=0, C=(1.0,1.0,1.0) ) # calls bind_vbo with args for selected shader
        #glEnableVertexAttribArray(self.attrib_ids[k])
        #glVertexAttribPointer(self.attrib_ids[k], s, GL_FLOAT, GL_FALSE, t*sizeof(c_float), c_void_p(d*sizeof(c_float)) )
        glDrawArrays( GL_LINES, 0, len(self.opengl_lines)/3 )
        self.opengl_lines.unbind()

        glBindTexture(GL_TEXTURE_2D, self.texture)
        self.matrix_push()
        self.shader_use("texture_standard")
        self.matrix_use()
        self.obj.draw_opengl_surface(self)
        self.matrix_pop()
        self.matrix_pop()

        glutSwapBuffers()
        return
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
def test_object():
    obj = opengl_obj.c_opengl_obj()

    lens_projection = python_wrapper.lens_projection
    quaternion      = python_wrapper.quaternion

    dst_w = 1024.0
    src_w = 2.0

    src_orientation = quaternion().from_euler(yaw=-60,degrees=True) * quaternion().from_euler(pitch=-30,degrees=True)
    src_camera      = lens_projection(focal_length=20.0, lens_type="rectilinear", frame_width=36.0, width=src_w)
    src_camera.orient(src_orientation)

    dst_orientation = quaternion().from_euler(yaw=-70,degrees=True) * quaternion().from_euler(pitch=-40,degrees=True)
    dst_camera      = lens_projection(focal_length=80.0, lens_type="rectilinear", frame_width=36.0, width=dst_w)
    dst_camera.orient(dst_orientation)

    #src = camera(width=2.0, height=2.0*3456.0/5184.0, focal_length=20.0, projection="rectilinear")
    #src_orientation = quaternion.yaw(-60,degrees=True) * quaternion.pitch(-30,degrees=True)
    #dst = camera(width=1024.0, height=1024.0, focal_length=80.0, projection="rectilinear")
    #dst_orientation = quaternion.yaw(-50,degrees=True) * quaternion.pitch(-60,degrees=True)
    dst_triangles = [ ((-512.0,-512.0), (512.0,-512.0), (-512.0,512.0)),
                          ((512.0,512.0), (512.0,-512.0), (-512.0,512.0)),
                          ]

    triangles = []
    n = 8
    for x in range(n):
        for y in range(n):
            x0 = (x/float(n)-0.5)
            x1 = ((x+1)/float(n)-0.5)
            y0 = (y/float(n)-0.5)
            y1 = ((y+1)/float(n)-0.5)
            triangles.append( ( (x0,y0), (x1,y0), (x0,y1) ) )
            triangles.append( ( (x1,y1), (x1,y0), (x0,y1) ) )
            pass
        pass

    for triangle in triangles:
        dst_xyzs = []
        src_xyzs = []
        dst_uvs = []
        src_uvs = []
        for xy in triangle:
            # points for 'destination' mesh - uses 'src' as the image texture, hence needs uv in 'src' terms
            dst_xy = (dst_w*xy[0],dst_w*xy[1])
            dst_q  = dst_camera.orientation_of_xy(dst_xy)
            src_xy = src_camera.xy_of_orientation(dst_q)
            rxyz = dst_q * quaternion(0,0,0,2) * ~dst_q
            xyz = rxyz.rijk[1:]
            dst_uvs.append(((src_xy[0]+1.0)/2,(1.0-src_xy[1])/2))
            dst_xyzs.append(xyz)

            # points for 'src' mesh - also uses 'src' as the image texture, hence needs uv in 'src' terms
            src_xy = (xy[0]*src_w, xy[1]*src_w)
            src_q = src_camera.orientation_of_xy(src_xy)
            rxyz = src_q * quaternion(0,0,0,8) * ~src_q
            xyz = rxyz.rijk[1:]
            src_xyzs.append(xyz)
            src_uvs.append(((src_xy[0]+1.0)/2,(1.0-src_xy[1])/2))
            pass
        obj.add_triangle( xyz_list = dst_xyzs,
                          uv_list = dst_uvs )
        obj.add_triangle( xyz_list = src_xyzs,
                          uv_list = src_uvs )
        pass
        
    texture_filename = "../../gjslib_data/camera/IMG_2159_25.PNG"

    og = c_view_obj(obj=obj,
                    texture_filename=texture_filename,
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

