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
            o["t"] = gjslib_c.texture(filename=o["texture_filename"])
            o["texture"] = o["t"].gl_id
            #o["texture"] = opengl_utils.texture_from_png(o["texture_filename"])
            o["obj"].create_opengl_surface()
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

        n = 0
        for o in self.objects:
            n = n + 1
            if (((self.tick/1)+n) % len(self.objects))!=0: continue
            glBindTexture(GL_TEXTURE_2D, o["texture"])
            self.matrix_push()
            self.shader_use("texture_standard")
            self.matrix_use()
            o["obj"].draw_opengl_surface(self)
            self.matrix_pop()
            pass

        self.matrix_pop()
        self.tick = self.tick+1

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

#f build_add_projected_image_mesh
def build_add_projected_image_mesh(obj,camera,src_w,src_h,n=32):
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
            xyz = src_q.rotate_vector((0,0,8))
            src_xyzs.append(xyz)
            src_uvs.append(((xy[0]+1.0/2)/1,(1.0/2+xy[1]/src_h)/1))
            pass
        obj.add_triangle( xyz_list = src_xyzs,
                          uv_list = src_uvs )
        pass
    return obj

def test_object():

    lens_projection = gjslib_c.lens_projection
    quaternion      = gjslib_c.quaternion

    dst_w = 1024.0
    src_w = 1.0
    src_h = 1.0
    src_ar = 3456/5148.0

    #for rect
    src_ar = 1
    src_h = 3456/5148.0

    src_orientation = quaternion().from_euler(yaw=0,degrees=True) * quaternion().from_euler(pitch=-30,degrees=True)
    src2_orientation = quaternion(r=0.957717, i=0.023461, j=-0.264079, k=-0.111759)
    src2_orientation = src_orientation  * quaternion(r=0.993334,i=-0.006355,j=-0.007261,k=-0.114871)
    src3_orientation = src2_orientation * quaternion(r=0.995947,i=-0.008978,j=-0.006900,k=-0.089229)

    src_camera      = lens_projection(focal_length=35.0, lens_type="rectilinear", frame_width=22.3*src_ar, width=src_w)
    src_camera.orient(src_orientation)

    src2_camera      = lens_projection(focal_length=35.0, lens_type="rectilinear", frame_width=22.3*src_ar, width=src_w)
    src2_camera.orient(src2_orientation)

    src3_camera      = lens_projection(focal_length=35.0, lens_type="rectilinear", frame_width=22.3*src_ar, width=src_w)
    src3_camera.orient(src3_orientation)

    dst_orientation = quaternion().from_euler(yaw=-10,degrees=True) * quaternion().from_euler(pitch=-74,degrees=True)
    dst_camera      = lens_projection(focal_length=80.0, lens_type="stereographic", frame_width=22.3, width=dst_w*3456/5148)
    dst_camera.orient(dst_orientation)

    #src = camera(width=2.0, height=2.0*3456.0/5184.0, focal_length=20.0, projection="rectilinear")
    #src_orientation = quaternion.yaw(-60,degrees=True) * quaternion.pitch(-30,degrees=True)
    #dst = camera(width=1024.0, height=1024.0, focal_length=80.0, projection="rectilinear")
    #dst_orientation = quaternion.yaw(-50,degrees=True) * quaternion.pitch(-60,degrees=True)
    dst_triangles = [ ((-512.0,-512.0), (512.0,-512.0), (-512.0,512.0)),
                          ((512.0,512.0), (512.0,-512.0), (-512.0,512.0)),
                          ]

    obj2 = opengl_obj.c_opengl_obj()
    build_add_projected_image_mesh(obj2, src_camera, src_w, src_h, 64)
    obj3 = opengl_obj.c_opengl_obj()
    build_add_projected_image_mesh(obj3, src2_camera, src_w, src_h, 64)
    obj4 = opengl_obj.c_opengl_obj()
    build_add_projected_image_mesh(obj4, src3_camera, src_w, src_h, 64)

    triangles = []
    n = 32
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

    obj1 = opengl_obj.c_opengl_obj()
    for triangle in triangles:
        dst_xyzs = []
        src_xyzs = []
        src2_xyzs = []
        dst_uvs = []
        src_uvs = []
        src2_uvs = []
        for xy in triangle:
            # points for 'destination' mesh - uses 'src' as the image texture, hence needs uv in 'src' terms
            dst_xy = (dst_w*xy[0],dst_w*xy[1])
            dst_q  = dst_camera.orientation_of_xy(dst_xy)
            src_xy = src_camera.xy_of_orientation(dst_q)
            xyz = dst_q.rotate_vector((0,0,2))
            dst_uvs.append(((src_xy[0]+1.0)/2,(1.0-src_xy[1])/2))
            dst_xyzs.append(xyz)

            pass
        obj1.add_triangle( xyz_list = dst_xyzs,
                          uv_list = dst_uvs )
        pass
        
    objects = [ {"obj":obj1,"texture_filename":"../../gjslib_data/camera/IMG_2159_25.PNG"},
                {"obj":obj2,"texture_filename":"../../gjslib_data/camera/IMG_2159_25.PNG"},
                {"obj":obj3,"texture_filename":"../../gjslib_data/camera/IMG_2159_25.PNG"},
                  ]
    objects = [ {"obj":obj1,"texture_filename":"../images/IMG_1900.PNG"},
                {"obj":obj2,"texture_filename":"../images/IMG_1900.PNG"},
                {"obj":obj3,"texture_filename":"../images/IMG_1901.PNG"},
                  ]
    objects = [ #{"obj":obj1,"texture_filename":"../images/IMG_1900.JPG"},
                {"obj":obj2,"texture_filename":"../images/IMG_1900.JPG"},
                {"obj":obj3,"texture_filename":"../images/IMG_1901.JPG"},
                {"obj":obj4,"texture_filename":"../images/IMG_1902.JPG"},
                  ]

    og = c_view_obj(obj=objects,
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

