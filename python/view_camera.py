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
#('../images/IMG_2157.JPG',None,quaternion(r=0.876071075236,i=-0.234743211406,j=0.303596644011,k=-0.291931796109)),
#('../images/IMG_2158.JPG',None,quaternion(r=0.895229549788,i=-0.254409052802,j=0.250791990382,k=-0.266352143974)),
#('../images/IMG_2159.JPG',None,quaternion(r=0.912925963735,i=-0.283005185206,j=0.170388109334,k=-0.239670903703)),
#('../images/IMG_2160.JPG',None,quaternion(r=0.920812320628,i=-0.313956965385,j=0.0756936184312,k=-0.218646221543)),
#('../images/IMG_2161.JPG',None,quaternion(r=0.915790458069,i=-0.343489005666,j=-0.0144685487151,k=-0.207686785796)),
#('../images/IMG_2162.JPG',None,quaternion(r=0.521505895067,i=-0.258142615924,j=-0.0647172229906,k=-0.810682226463)),
#('../images/IMG_2163.JPG',None,quaternion(r=0.530466112881,i=-0.256659664471,j=0.0245468920898,k=-0.807545026489)),
#('../images/IMG_2164.JPG',None,quaternion(r=0.51360147274,i=-0.247130904281,j=0.0645458815901,k=-0.819129826412)),
#('../images/IMG_2165.JPG',None,quaternion(r=0.664630038585,i=-0.153302953544,j=0.0996026953723,k=-0.724461468486)),
#('../images/IMG_2166.JPG',None,quaternion(r=0.74879472528,i=-0.042264528764,j=0.134884618484,k=-0.64755409712)),
#('../images/IMG_2167.JPG',None,quaternion(r=0.870999435719,i=-0.168410581851,j=0.403891403891,k=0.223315007916)),
#('../images/IMG_2168.JPG',None,quaternion(r=0.848646103391,i=-0.260287303362,j=0.418461092524,k=0.192199440559)),
#('../images/IMG_2169.JPG',None,quaternion(r=0.811365685033,i=-0.383423410832,j=0.438250685397,k=0.0510739652486)),
#('../images/IMG_2170.JPG',None,quaternion(r=0.735081029854,i=-0.537199502831,j=-0.00448860346975,k=-0.413584847578)),
#('../images/IMG_2171.JPG',None,quaternion(r=0.730852730643,i=-0.616925057672,j=-0.10511130956,k=-0.272413971615)),
#('../images/IMG_2172.JPG',None,quaternion(r=0.721905616568,i=-0.64820640783,j=-0.223618129783,k=-0.0931432533635)),
#('../images/IMG_2173.JPG',None,quaternion(r=0.829966032604,i=-0.472327318691,j=-0.0252580996473,k=-0.295677725138)),
('../images/IMG_2174.JPG',None,quaternion(r=0.82729049789,i=-0.498196141488,j=-0.0760701396181,k=-0.24820227752)),
#('../images/IMG_2175.JPG',None,quaternion(r=0.802079119548,i=-0.543042247185,j=-0.217365013855,k=-0.120526571798)),
#('../images/IMG_2176.JPG',None,quaternion(r=0.885671006452,i=-0.380921075139,j=-0.243370769526,k=-0.106097461732)),
#('../images/IMG_2177.JPG',None,quaternion(r=0.938994591171,i=-0.199516490009,j=-0.258022668256,k=-0.109117508364)),
#('../images/IMG_2178.JPG',None,quaternion(r=0.952574273516,i=-0.0499309867915,j=-0.281304449263,k=-0.104770973168)),
#('../images/IMG_2179.JPG',None,quaternion(r=-0.935896301688,i=-0.146899956362,j=0.315113527273,k=0.0567624896955)),
#('../images/IMG_2180.JPG',None,quaternion(r=-0.710461312188,i=-0.166421836125,j=0.0507686106141,k=0.681887853332)),
#('../images/IMG_2181.JPG',None,quaternion(r=0.689801666071,i=0.0541954852482,j=-0.136461862513,k=-0.708953221972)),
#('../images/IMG_2183.JPG',None,quaternion(r=0.499941016482,i=-0.244775961786,j=-0.376087705139,k=-0.740744049328)),
('../images/IMG_2184.JPG',None,quaternion(r=1.0,i=0.0,j=0.0,k=0.0)),
]
        vector = gjslib_c.vector
        polys = {
'IMG_2179.JPG': [vector((-0.340950,0.205465,0.917353,)), vector((-0.308765,-0.463147,0.830758,)), vector((-0.288963,-0.466168,0.836175,)), vector((-0.252071,0.180071,0.950807,))] ,
'IMG_2166.JPG': [vector((0.336577,0.258115,0.905589,)), vector((-0.269161,0.137336,0.953253,)), vector((-0.191995,-0.468567,0.862313,)), vector((0.312618,-0.441341,0.841123,))] ,
'IMG_2164.JPG': [vector((0.342775,-0.178700,0.922264,)), vector((0.308765,0.463147,0.830758,)), vector((0.071131,0.485707,0.871223,)), vector((-0.091652,0.068339,0.993443,))] ,
'IMG_2162.JPG': [vector((0.348382,-0.001416,0.937351,)), vector((0.308765,0.463147,0.830758,)), vector((-0.145482,0.481760,0.864143,)), vector((-0.215124,0.267435,0.939255,))] ,
'IMG_2159.JPG': [vector((0.347856,-0.054945,0.935936,)), vector((0.308765,0.463147,0.830758,)), vector((0.081071,0.485337,0.870560,)), vector((-0.042224,0.190566,0.980766,))] ,
'IMG_2160.JPG': [vector((0.348380,-0.003727,0.937346,)), vector((0.308765,0.463147,0.830758,)), vector((-0.133859,0.482558,0.865575,)), vector((-0.210322,0.264281,0.941233,)), vector((0.341088,-0.013727,0.939931,))] ,
'IMG_2180.JPG': [vector((-0.342309,0.185908,0.921012,)), vector((-0.308765,-0.463147,0.830758,)), vector((-0.054235,-0.486224,0.872150,)), vector((0.212482,-0.436523,0.874242,)), vector((0.172402,0.172243,0.969850,))] ,
'IMG_2173.JPG': [vector((0.314357,0.431038,0.845805,)), vector((0.308765,0.463147,0.830758,)), vector((0.001969,0.486939,0.873434,)), vector((0.213228,0.344057,0.914417,))] ,
'IMG_2178.JPG': [vector((-0.323201,-0.373278,0.869600,)), vector((-0.288870,-0.375632,0.880599,)), vector((-0.092408,0.484857,0.869698,)), vector((-0.308765,0.463147,0.830758,))] ,
'IMG_2177.JPG': [vector((-0.347263,-0.080095,0.934341,)), vector((-0.248677,-0.088672,0.964519,)), vector((-0.077286,0.485484,0.870823,)), vector((-0.308765,0.463147,0.830758,))] ,
'IMG_2165.JPG': [vector((0.318462,0.405456,0.856847,)), vector((-0.115876,0.400951,0.908741,)), vector((-0.158813,-0.212587,0.964150,)), vector((0.333104,-0.292893,0.896245,))] ,
'IMG_2163.JPG': [vector((0.346439,-0.105474,0.932124,)), vector((0.308765,0.463147,0.830758,)), vector((-0.000538,0.486940,0.873435,)), vector((-0.119274,0.126788,0.984733,))] ,
'IMG_2161.JPG': [vector((-0.330540,0.315927,0.889344,)), vector((0.197731,0.099358,0.975208,)), vector((0.331553,0.307058,0.892070,)), vector((0.308765,0.463147,0.830758,)), vector((-0.308765,0.463147,0.830758,))] ,
'IMG_2158.JPG': [vector((0.347286,-0.079265,0.934403,)), vector((0.308765,0.463147,0.830758,)), vector((0.271766,0.468613,0.840562,)), vector((0.097741,0.116599,0.988358,))] ,
'IMG_2181.JPG': [vector((-0.309945,0.456606,0.833933,)), vector((-0.338794,-0.232995,0.911554,)), vector((0.212788,-0.213076,0.953583,)), vector((0.239016,0.400049,0.884778,))] ,
'IMG_2183.JPG': [vector((0.316516,0.417819,0.851613,)), vector((0.308765,0.463147,0.830758,)), vector((0.093270,0.484818,0.869628,)), vector((0.299745,0.393746,0.868975,))] ,
'IMG_2157.JPG': [vector((0.330852,0.313218,0.890186,)), vector((0.185673,0.063217,0.980576,)), vector((0.347018,-0.088422,0.933681,))] ,
'IMG_2174.JPG': [vector((0.050739,0.486313,0.872310,)), vector((0.142733,0.436594,0.888264,)), vector((0.183246,0.478695,0.858645,))] ,
'IMG_2176.JPG': [vector((-0.332890,0.294891,0.895669,)), vector((-0.198660,0.284355,0.937911,)), vector((-0.122874,0.483250,0.866817,)), vector((-0.308765,0.463147,0.830758,))] ,
'IMG_2170.JPG': [vector((0.311762,0.446297,0.838823,)), vector((0.308765,0.463147,0.830758,)), vector((0.254997,0.470843,0.844561,)), vector((0.282948,0.435552,0.854538,))] ,
}
        focal_length = 20.0
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

