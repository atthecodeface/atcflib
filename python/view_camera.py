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
                c_view_camera_obj.selectable_id += 1
                pass
            self.selectable_id = c_view_camera_obj.selectable_id
            pass
        self.note = note
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
        if c_view_camera_obj.selectable_id==0: return True
        if (selection%c_view_camera_obj.selectable_id)==self.selectable_id:
            if ((c_view_camera_obj.last_note_shown!=self.selectable_id) and
                self.note is not None):
                print self.selectable_id,self.note
                c_view_camera_obj.last_note_shown = self.selectable_id
                pass
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

#f add_blob
def add_blob(obj, src_q, scale=0.001, style="star"):
    angle_rs = {"diamond":      [(i,1) for i in range(0,450,90)],
                "star":         [(i,[1,0.5][((i/36)&1)]) for i in range(0,396,36)],
                "triangle":     [(i,1) for i in range(0,450,120)],
                "inv_triangle": [(i,1) for i in range(60,450,120)],
                }
    cxyz= src_q.rotate_vector(gjslib_c.vector((0,0,0.8))).coords
    lxyz = None
    for (angle,r) in angle_rs[style]:
        vector_z_sc = gjslib_c.vector((0,scale*r,0.8))
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

    dst_w = 1024.0
    src_w = 1.0
    src_h = 1.0
    src_ar = 3456/5148.0

    separate = False
    #separate = True

    #for rect
    src_ar = 1
    src_h = 3456/5148.0

    image_orientations = [ ("../images/IMG_1900.JPG", None, quaternion(1)),
#~quaternion(r=0.360925,i=0.049321,j=0.126091,k=0.922714)), # focus on muzzy...
                           ("../images/IMG_1901.JPG", 0,
quaternion(r=0.993800,i=-0.006155,j=-0.006950,k=-0.110793)
                            ),
                           ("../images/IMG_1902.JPG", 0, 
quaternion(r=0.979007,i=-0.015306,j=-0.013493,k=-0.202803)
                            ),
                           ]
    focal_length = 35.0
    lens_type = "rectilinear"

    if True:
        image_orientations = [ ("../images/IMG_2173.JPG", None, quaternion(r=1)),
                               ("../images/IMG_2174.JPG", 0, 
quaternion(r=0.997179,i=-0.006334,j=-0.072740,k=0.017404) # 20.0
                               )]
        focal_length = 20.0
        lens_type = "rectilinear"
        pass

    polys = None
    if True:
        image_orientations = [ 
('../images/IMG_2157.JPG',None,quaternion(r=0.873898774365,i=-0.246081826269,j=0.305201494129,k=-0.287396442089)),
('../images/IMG_2158.JPG',None,quaternion(r=0.892784552449,i=-0.264867489793,j=0.253806538552,k=-0.26146356677)),
('../images/IMG_2159.JPG',None,quaternion(r=0.910483968879,i=-0.291998851595,j=0.1759145837,k=-0.234114656361)),
('../images/IMG_2160.JPG',None,quaternion(r=0.918690115993,i=-0.32169570652,j=0.0831829597875,k=-0.213543762225)),
('../images/IMG_2161.JPG',None,quaternion(r=0.91589028353,i=-0.349602855255,j=-0.00450915065685,k=-0.197237166107)),
('../images/IMG_2162.JPG',None,quaternion(r=0.527730580496,i=-0.269725029024,j=-0.0653566053232,k=-0.802793471117)),
('../images/IMG_2163.JPG',None,quaternion(r=0.534512456235,i=-0.269629177505,j=0.0209328366776,k=-0.800723645908)),
('../images/IMG_2164.JPG',None,quaternion(r=0.51917209409,i=-0.260508164002,j=0.0590637211748,k=-0.811854241873)),
('../images/IMG_2165.JPG',None,quaternion(r=0.67073482571,i=-0.170223420862,j=0.0985984975621,k=-0.715134334827)),
('../images/IMG_2166.JPG',None,quaternion(r=0.749742364737,i=-0.0634208638941,j=0.136764098165,k=-0.644328923761)),
('../images/IMG_2167.JPG',None,quaternion(r=0.866822609662,i=-0.175305843172,j=0.409835573036,k=0.223430588334)),
('../images/IMG_2168.JPG',None,quaternion(r=0.843890267114,i=-0.26680402536,j=0.423860352236,k=0.192372635592)),
('../images/IMG_2169.JPG',None,quaternion(r=0.807564742665,i=-0.389884237204,j=0.439683335774,k=0.0500802578585)),
('../images/IMG_2170.JPG',None,quaternion(r=0.736191240676,i=-0.534793054189,j=-0.00738080430692,k=-0.414685869147)),
('../images/IMG_2171.JPG',None,quaternion(r=0.735414316769,i=-0.61355396828,j=-0.100171921474,k=-0.269597657348)),
('../images/IMG_2172.JPG',None,quaternion(r=0.730259536265,i=-0.643433736771,j=-0.211299384429,k=-0.0898142873818)),
('../images/IMG_2173.JPG',None,quaternion(r=0.830215513667,i=-0.471180110505,j=-0.0249964109349,k=-0.29682770048)),
('../images/IMG_2174.JPG',None,quaternion(r=0.828163031052,i=-0.497064041297,j=-0.0747143578401,k=-0.247973985692)),
('../images/IMG_2175.JPG',None,quaternion(r=0.807886677848,i=-0.539585473086,j=-0.204945799795,k=-0.119011983166)),
('../images/IMG_2176.JPG',None,quaternion(r=0.889229936659,i=-0.375936493017,j=-0.234865753678,k=-0.113048444118)),
('../images/IMG_2177.JPG',None,quaternion(r=0.939692251518,i=-0.197393266242,j=-0.25191352033,k=-0.120639749479)),
('../images/IMG_2178.JPG',None,quaternion(r=0.953003284357,i=-0.0491502477046,j=-0.275606785531,k=-0.115801092072)),
('../images/IMG_2179.JPG',None,quaternion(r=-0.93845586036,i=-0.141921192717,j=0.307480129629,k=0.067933372481)),
# IMG_2180.JPG has no orientation
('../images/IMG_2181.JPG',None,quaternion(r=0.685251979233,i=0.0522552416841,j=-0.134074211778,k=-0.713949032081)),
('../images/IMG_2183.JPG',None,quaternion(r=0.498191662482,i=-0.246596921173,j=-0.375057152117,k=-0.741840386166)),
('../images/IMG_2184.JPG',None,quaternion(r=1.0,i=0.0,j=0.0,k=0.0)),
]
        focal_length = 20.5
        vector = gjslib_c.vector
        polys = {
}
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
        n += 1
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
        #from image_mappings import mappings, best_matches, src_quaternions, tgt_quaternions, base_mapping
        pass
    except:
        pass
    if mappings is not None:
        src_q = orientations[0]
        # Use tgt_q = orientations[1]
        # if using mappings; because orientations[1] is 'null' to 'where we want to put tgt'
        # and image_mappings, for example, has src and target with the same 'null' mapping
        tgt_q = orientations[1]
        # Use tgt_q = orientations[0]
        # if using mappings2; because orientations[0] is 'null' to 'where we want to put src'
        # and the points are supposed to be where we want them
        # This is absolutely true if orientations[1]=orientations[0]*mapping used to generate mappings2
        # If the latter is not the case, then we should use
        # tgt_q = orientations[1]*~mapping_used_to_generate_mappins2
        # and image_mappings, for example, has src and target with the same 'null' mapping
        if base_mapping is not None:
            tgt_q = orientations[1] * ~base_mapping
            pass

        hue = 0
        for src_q_id in src_quaternions:
            obj = c_view_camera_obj(has_surface=True, color=rgb_of_hue(hue), selectable=(hue==0))
            hue = (hue+15)%360
            image_objects.append(obj)
            add_blob(obj, src_q * src_quaternions[src_q_id] )
            pass
        hue = 0
        for bm in best_matches:
            (score, min_cos_sep_score, score_q, src_tgt_maps, src_tgts, average_q) = bm
            obj = c_view_camera_obj(has_surface=True, color=rgb_of_hue(hue), selectable=True,
                                    note="score %f, src_tgts %s, avg %s"%(score,src_tgts,average_q.to_rotation_str(1)),
                                    )
            image_objects.append(obj)
            hue = (hue+44)%360
            obj.add_line(line_between_qs(src_q,tgt_q*~score_q))
            for src_qx in src_tgts:
                for tgt_qx in src_tgts[src_qx]:
                    src_q0 = src_q * src_quaternions[src_qx]
                    tgt_q0 = tgt_q * tgt_quaternions[tgt_qx]
                    obj.add_line(line_between_qs(src_q0,tgt_q0))
                    add_blob(obj, src_q0, style="triangle" )
                    add_blob(obj, tgt_q0, style="inv_triangle" )
                    pass
                pass
            pass
        if False: #for src_q0 in mappings: # 
            src_q0_id = src_q0
            m = mappings[src_q0]
            src_q0 = src_q * src_quaternions[src_q0]
            hue = 0
            first_of_mapping = True
            for tgt_q0 in m['tgts']:
                tgt_q0_id = tgt_q0
                src_tgt_mapping = m['tgts'][tgt_q0]
                tgt_q0 = tgt_q * tgt_quaternions[tgt_q0]
                obj = None
                if len(src_tgt_mapping['mappings'])<4: continue
                for (src_q1, tgt_q1, src_from_tgt_orient) in src_tgt_mapping['mappings']:
                    if obj is None:
                        tgt_xy,fft_rot,fft_power = src_tgt_mapping['data']
                        hue = fft_power*100
                        obj = c_view_camera_obj(has_surface=True, color=rgb_of_hue(hue), selectable=first_of_mapping,
                                                note="%d,%d,%s"%(src_q0_id,tgt_q0_id,str(src_tgt_mapping['data'])),
                                                )
                        image_objects.append(obj)
                        hue = (hue+44)%360
                        #first_of_mapping = False
                        pass

                    src_q1 = src_q * src_quaternions[src_q1]
                    tgt_q1 = tgt_q * tgt_quaternions[tgt_q1]
                    obj.add_line(line_between_qs(src_q0, src_q1))
                    obj.add_line(line_between_qs(src_q1, tgt_q1))
                    obj.add_line(line_between_qs(tgt_q0, tgt_q1))
                    pass
                pass
            pass
        pass
    if polys is not None:
        hue = 0
        for p_id in polys:
            p = polys[p_id]
            obj = c_view_camera_obj(has_surface=True, color=rgb_of_hue(hue), selectable=True, note=p_id)
            image_objects.append(obj)
            hue = (hue+44)%360
            (a,b,d) = (0,len(p)-1,True)
            while b>a+1:
                if d:
                    (a,vs) = (a+1,(a,a+1,b))
                    pass
                else:
                    (b,vs) = (b-1,(a,b-1,b))
                    pass
                tri_vs = []
                for v in vs:
                    tri_vs.append(p[v].coords)
                    pass
                obj.add_triangle(tri_vs,[(0,0)]*3)
                pass
            pass
        pass

    og = c_view_obj(obj=image_objects,
                    window_size=(1000,1000))
    og.init_opengl()
    #og.seal_hack = True
    og.camera["fov"] = 90

    og.zFar = 100.0
    #og.create_menus(menus)
    #og.attach_menu("main_menu")
    og.main_loop()

#f Main
if __name__ == '__main__':
    test_object()

