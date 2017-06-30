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
from filters import *
vector_z = gjslib_c.vector(vector=(0,0,1))
gjslib_c.lens_projection.add_named_polynomial("canon_20_35_rebel2Ti_20", canon_20_35_rebel2Ti_20_polynomial[0], canon_20_35_rebel2Ti_20_polynomial[1])

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
                c_view_camera_obj.selectable_id += 1
                pass
            self.selectable_id = c_view_camera_obj.selectable_id
            pass
        self.note = note
        print selectable, c_view_camera_obj.selectable_id
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
        if (selection%c_view_camera_obj.selectable_id)==(self.selectable_id%c_view_camera_obj.selectable_id):
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

    objects_selectable =True
    polys = None
    if True:
        image_orientations = [ 

#2181 to 2178
# 21.5 : 1 417.0 (r=0.769131,i=0.092447,j=-0.096866,k=0.624906) (r=0.768204,i=0.092498,j=-0.096972,k=0.626022)  79.61:(0.144482,-0.151470,0.977845)
# 21.4 : 1 233.0 (r=0.769226,i=0.094089,j=-0.097378,k=0.624464) (r=0.768465,i=0.093588,j=-0.097964,k=0.625384)  79.57:(0.146255,-0.153094,0.977329)x
# 21.3 : 1 650.0 (r=0.768453,i=0.094119,j=-0.096986,k=0.625472) (r=0.769359,i=0.093733,j=-0.097299,k=0.624367)  79.41:(0.146729,-0.152311,0.977380)
# 21.2 : 1 368.0 (r=0.767563,i=0.094998,j=-0.098568,k=0.626184) (r=0.768901,i=0.094572,j=-0.098359,k=0.624638)  79.49:(0.147915,-0.153838,0.976962)
# 21.1 : 1 315.0 (r=0.768167,i=0.095541,j=-0.099083,k=0.625279) (r=0.768445,i=0.094912,j=-0.098933,k=0.625057)  79.57:(0.148320,-0.154603,0.976780)
# 21.0 : 1 555.0 (r=0.769232,i=0.095706,j=-0.098276,k=0.624070) (r=0.769276,i=0.095400,j=-0.098753,k=0.623988)  79.42:(0.149315,-0.154564,0.976635)
# 20.9 : 1 341.0 (r=0.769277,i=0.095995,j=-0.099887,k=0.623715) (r=0.767736,i=0.095682,j=-0.099982,k=0.625644)  79.70:(0.149324,-0.156036,0.976399)
# 20.8 : 1 420.0 (r=0.770275,i=0.097004,j=-0.099588,k=0.622374) (r=0.770084,i=0.096780,j=-0.100126,k=0.622559)  79.28:(0.151706,-0.156951,0.975885)
# 20.7 : 1 270.0 (r=0.767516,i=0.095850,j=-0.100451,k=0.625812) (r=0.768147,i=0.096568,j=-0.100550,k=0.624912)  79.62:(0.150823,-0.157042,0.976007)
# 20.6 : 1 468.0 (r=0.767765,i=0.097265,j=-0.100048,k=0.625354) (r=0.768215,i=0.097317,j=-0.100842,k=0.624664)  79.61:(0.152012,-0.157519,0.975746)
# 20.5 : 1 142.0 (r=0.755855,i=0.099231,j=-0.102901,k=0.638943) (r=0.760029,i=0.098960,j=-0.102794,k=0.634032)  81.07:(0.152272,-0.158172,0.975600)
# 20.4 : 1 316.0 (r=0.768317,i=0.098527,j=-0.101057,k=0.624315) (r=0.769024,i=0.098244,j=-0.101465,k=0.623422)  79.47:(0.153694,-0.158732,0.975286)
# 20.3 : 1 229.0 (r=0.768702,i=0.098801,j=-0.103050,k=0.623471) (r=0.768499,i=0.098768,j=-0.102650,k=0.623793)  79.56:(0.154360,-0.160428,0.974903)
# 20.1 : 1 182.0 (r=0.768085,i=0.100267,j=-0.102555,k=0.624079) (r=0.768643,i=0.099773,j=-0.103265,k=0.623354)  79.54:(0.155974,-0.161433,0.974480)

#('../images/IMG_2181.JPG',None,20.8,quaternion(r=0.689734910077,i=0.053184853808,j=-0.134445094474,k=-0.709479838839)),
#('../images/IMG_2178.JPG',0,20.8,quaternion(r=0.770084,i=0.096780,j=-0.100126,k=0.622559)),
#('../images/IMG_2181.JPG',None,21.0,quaternion(r=0.689734910077,i=0.053184853808,j=-0.134445094474,k=-0.709479838839)),
#('../images/IMG_2178.JPG',0,21.0,quaternion(r=0.769276,i=0.095400,j=-0.098753,k=0.623988)),
#('../images/IMG_2181.JPG',None,21.3,quaternion(r=0.689734910077,i=0.053184853808,j=-0.134445094474,k=-0.709479838839)),
#('../images/IMG_2178.JPG',0,21.3,quaternion(r=0.769359,i=0.093733,j=-0.097299,k=0.624367)),


#('../images/IMG_2176.JPG',None,21.3,quaternion(1)),
#('../images/IMG_2175.JPG',0,21.3,quaternion(r=0.983959,i=-0.174295,j=-0.008297,k=0.037103)),
#('../images/IMG_2176.JPG',None,20.8,quaternion(1)),
#('../images/IMG_2175.JPG',0,20.8,quaternion(r=0.983256,i=-0.178408,j=-0.008481,k=0.036141)),

#('../images/IMG_2165.JPG',None,20.0,quaternion(1)),
#('../images/IMG_2164.JPG',0,   20.0,quaternion(r=0.979403,i=-0.048752,j=-0.059300,k=-0.186752)),

#('../images/IMG_2178.JPG',None,quaternion(r=0.954967940233,i=-0.0494948303831,j=-0.277013921209,k=-0.0940732818042)),
#('../images/IMG_2180.JPG',None,quaternion(r=-0.497018799929,i=-0.183674708766,j=0.0281886225961,k=0.847609176114)),
# from 2181

# from 2184
#('../images/IMG_2181.JPG',None,quaternion(r=0.689734910077,i=0.053184853808,j=-0.134445094474,k=-0.709479838839)),
#('../images/IMG_2166.JPG',None,quaternion(r=0.750766257325,i=-0.0626460971721,j=0.136806745474,k=-0.643202462499)),

#('../images/IMG_2184.JPG',None,quaternion(r=1.0,i=0.0,j=0.0,k=0.0)),

# IMG_2167.JPG has no orientation
# IMG_2168.JPG has no orientation
# IMG_2169.JPG has no orientation
# IMG_2170.JPG has no orientation
# IMG_2171.JPG has no orientation
#('../images/IMG_2172.JPG',None,quaternion(r=0.732570424733,i=-0.643857732533,j=-0.195378626968,k=-0.103029050217)),
# IMG_2173.JPG has no orientation
# IMG_2174.JPG has no orientation
#('../images/IMG_2175.JPG',None,quaternion(r=0.809040868112,i=-0.538701192734,j=-0.191710872587,k=-0.136017792963)),
#('../images/IMG_2176.JPG',None,quaternion(r=0.88920148509,i=-0.3770829874,j=-0.243517434789,k=-0.0884782373294)),
#('../images/IMG_2177.JPG',None,quaternion(r=0.941119314314,i=-0.198435713477,j=-0.256436721882,k=-0.0956969775548)),
#('../images/IMG_2178.JPG',None,quaternion(r=0.954967940233,i=-0.0494948303831,j=-0.277013921209,k=-0.0940732818042)),
#('../images/IMG_2179.JPG',None,quaternion(r=-0.940477583524,i=-0.142755006227,j=0.305033383995,k=0.0455802340328)),
#('../images/IMG_2180.JPG',None,quaternion(r=-0.497018799929,i=-0.183674708766,j=0.0281886225961,k=0.847609176114)),
# IMG_2183.JPG has no orientation

#('../images/IMG_2157.JPG',None,quaternion(r=0.876639352422,i=-0.241602264532,j=0.30356149888,k=-0.284573730265)),
#('../images/IMG_2158.JPG',None,quaternion(r=0.894904505137,i=-0.259915398441,j=0.253044499884,k=-0.259919975024)),
#('../images/IMG_2159.JPG',None,quaternion(r=0.911719607084,i=-0.286924204863,j=0.175646724718,k=-0.235775500888)),
#('../images/IMG_2160.JPG',None,quaternion(r=0.919581174185,i=-0.316395558856,j=0.0846978550371,k=-0.21700365843)),
#('../images/IMG_2161.JPG',None,quaternion(r=0.916545400428,i=-0.345241891722,j=-0.00280050241489,k=-0.201853219794)),
#('../images/IMG_2162.JPG',None,quaternion(r=0.525347785312,i=-0.266998017825,j=-0.0625487209466,k=-0.80548707032)),
#('../images/IMG_2163.JPG',None,quaternion(r=0.531357608186,i=-0.266342043416,j=0.0220872374547,k=-0.80388628678)),
#('../images/IMG_2164.JPG',None,quaternion(r=0.517829078641,i=-0.257653057349,j=0.0604959173962,k=-0.813515944116)),
#('../images/IMG_2165.JPG',None,quaternion(r=0.668007514065,i=-0.167959728431,j=0.0979370333883,k=-0.718306221795)),
#('../images/IMG_2166.JPG',None,quaternion(r=0.751674830865,i=-0.0618738477884,j=0.134790754217,k=-0.64264144605)),
#('../images/IMG_2167.JPG',None,quaternion(r=0.868941844498,i=-0.168174942786,j=0.405504491949,k=0.22852432367)),
#('../images/IMG_2168.JPG',None,quaternion(r=0.847649336196,i=-0.257557756599,j=0.419534490739,k=0.197852005151)),
#('../images/IMG_2169.JPG',None,quaternion(r=0.81430168683,i=-0.380443439137,j=0.435046372651,k=0.0539463259871)),
#('../images/IMG_2170.JPG',None,quaternion(r=0.737599348599,i=-0.528529545178,j=-0.00287376648846,k=-0.420232628773)),
#('../images/IMG_2171.JPG',None,quaternion(r=0.74384220092,i=-0.604823704026,j=-0.0908435397158,k=-0.269507919125)),
#('../images/IMG_2172.JPG',None,quaternion(r=0.742179973929,i=-0.630724131935,j=-0.213203732077,k=-0.0768122667446)),
#('../images/IMG_2173.JPG',None,quaternion(r=0.833497801582,i=-0.465458289753,j=-0.0189163588952,k=-0.297106322087)),
#('../images/IMG_2174.JPG',None,quaternion(r=0.832611268653,i=-0.49028516146,j=-0.0663776723968,k=-0.248943648987)),
#('../images/IMG_2175.JPG',None,quaternion(r=0.816975071027,i=-0.527984064514,j=-0.205761315045,k=-0.106989916212)),
#('../images/IMG_2176.JPG',None,quaternion(r=0.894610067226,i=-0.367816994652,j=-0.232798448894,k=-0.100937447244)),
#('../images/IMG_2177.JPG',None,quaternion(r=0.943084748422,i=-0.192756947523,j=-0.247589864534,k=-0.1101597724)),
#('../images/IMG_2178.JPG',None,quaternion(r=0.955298941011,i=-0.0476220319942,j=-0.269932300967,k=-0.110781895034)),
#('../images/IMG_2179.JPG',None,quaternion(r=-0.941226881643,i=-0.139551382749,j=0.300792039072,k=0.0643546274721)),
#('../images/IMG_2180.JPG',None,quaternion(r=-0.68624550572,i=-0.167989351691,j=0.0428979663466,k=0.706403884531)),
#('../images/IMG_2181.JPG',None,quaternion(r=0.688281299561,i=0.0513609985304,j=-0.131630769448,k=-0.711550589234)),
#('../images/IMG_2183.JPG',None,quaternion(r=0.496011193362,i=-0.247619109473,j=-0.365204021126,k=-0.747852723226)),
#('../images/IMG_2184.JPG',None,quaternion(r=1.0,i=0.0,j=0.0,k=0.0)),

('../images/IMG_2157.JPG',None,quaternion(r=0.879815563782,i=-0.244369054073,j=0.298827177338,k=-0.277327707275)),
('../images/IMG_2158.JPG',None,quaternion(r=0.897867640987,i=-0.262277156481,j=0.24793235166,k=-0.252138734545)),
('../images/IMG_2159.JPG',None,quaternion(r=0.914142685503,i=-0.288945793925,j=0.170039764719,k=-0.22790339429)),
('../images/IMG_2160.JPG',None,quaternion(r=0.92150148118,i=-0.317915156536,j=0.0782737615154,k=-0.208897562663)),
('../images/IMG_2161.JPG',None,quaternion(r=0.918249800485,i=-0.345555790309,j=-0.00939771913577,k=-0.193184322781)),
('../images/IMG_2162.JPG',None,quaternion(r=0.531526683461,i=-0.265033762773,j=-0.0678937156658,k=-0.801640151647)),
('../images/IMG_2163.JPG',None,quaternion(r=0.539046589407,i=-0.265322127652,j=0.0175607163384,k=-0.799202455119)),
('../images/IMG_2164.JPG',None,quaternion(r=0.52262463601,i=-0.256168743372,j=0.0556015283734,k=-0.811264158457)),
('../images/IMG_2165.JPG',None,quaternion(r=0.672595046202,i=-0.167193056299,j=0.0938401391295,k=-0.714742201103)),
('../images/IMG_2166.JPG',None,quaternion(r=0.75630346981,i=-0.0614467517138,j=0.131379392259,k=-0.637941073726)),
('../images/IMG_2167.JPG',None,quaternion(r=0.867639112714,i=-0.172359672922,j=0.401277102393,k=0.237636698206)),
('../images/IMG_2168.JPG',None,quaternion(r=0.837709066369,i=-0.235172341318,j=0.41783736869,k=0.261437226364)),
('../images/IMG_2169.JPG',None,quaternion(r=0.815474687186,i=-0.361358054968,j=0.435056455135,k=0.123074252034)),
('../images/IMG_2170.JPG',None,quaternion(r=0.746403631735,i=-0.528748679295,j=-0.00828131136219,k=-0.404027069091)),
('../images/IMG_2171.JPG',None,quaternion(r=0.748802107298,i=-0.603111311534,j=-0.0965518110665,k=-0.25735170057)),
('../images/IMG_2172.JPG',None,quaternion(r=0.746924356424,i=-0.633386435523,j=-0.201379191496,k=-0.0192886056759)),
('../images/IMG_2173.JPG',None,quaternion(r=0.837829557455,i=-0.464973556452,j=-0.0230001619869,k=-0.285152971236)),
('../images/IMG_2174.JPG',None,quaternion(r=0.836394236119,i=-0.489039979226,j=-0.0707723300248,k=-0.237225331297)),
('../images/IMG_2175.JPG',None,quaternion(r=0.82304940363,i=-0.534033058591,j=-0.186498836974,k=-0.051152275832)),
('../images/IMG_2176.JPG',None,quaternion(r=0.896332903909,i=-0.366705957732,j=-0.230948205632,k=-0.0936856032089)),
('../images/IMG_2177.JPG',None,quaternion(r=0.944628234658,i=-0.192372508885,j=-0.244736814141,k=-0.103798882062)),
('../images/IMG_2178.JPG',None,quaternion(r=0.95704521348,i=-0.0472947705337,j=-0.265841273154,k=-0.105622353329)),
('../images/IMG_2179.JPG',None,quaternion(r=-0.943087718169,i=-0.139339075159,j=0.295874252939,k=0.0602378985394)),
('../images/IMG_2180.JPG',None,quaternion(r=-0.691282503845,i=-0.155795073889,j=0.0761040201664,k=0.701473144849)),
('../images/IMG_2181.JPG',None,quaternion(r=0.689483415211,i=0.04986226991,j=-0.128104781415,k=-0.711136793568)),
('../images/IMG_2183.JPG',None,quaternion(r=0.50815564887,i=-0.246445599165,j=-0.363885100632,k=-0.740695643778)),
('../images/IMG_2184.JPG',None,quaternion(r=1.0,i=0.0,j=0.0,k=0.0)),


]
        default_focal_length = 20.0
        vector = gjslib_c.vector
        polys = {
}
        lens_type = "canon_20_35_rebel2Ti_20"
        pass

    orientations = []
    image_objects = []
    n = 0
    for tpo in image_orientations:
        if len(tpo)==3:
            (texture_filename, previous_orientation, orientation) = tpo
            focal_length = default_focal_length
            pass
        if len(tpo)==4:
            (texture_filename, previous_orientation, focal_length, orientation) = tpo
            pass
        if previous_orientation is not None:
            orientation = orientations[previous_orientation] * orientation
            if separate:
                orientation = quaternion().from_euler(pitch=45,degrees=1) * orientations[previous_orientation]
                pass
            pass
        obj = c_view_camera_obj(texture_filename = texture_filename, has_surface=True, selectable=objects_selectable, note="%s %f"%(texture_filename, focal_length))
        obj.camera = lens_projection(focal_length=focal_length, lens_type=lens_type, frame_width=22.3*src_ar, width=src_w, height=src_w)
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

