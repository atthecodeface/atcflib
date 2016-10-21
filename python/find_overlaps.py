#!/usr/bin/env python
#
#a Imports
import OpenGL.GLUT
import OpenGL.GL
import gjslib_c
import math
import sys
img_png_n=0
vector_z = gjslib_c.vector(vector=(0,0,1))
vector_x = gjslib_c.vector(vector=(1,0,0))

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
    """
    def __init__(self, p0, p1):
        self.axis = p0.cross_product(p1)
        pass
    def intersect(self, other):
        """
        All points a the GC have p.n=0
        Points where two great circles meet have p.n1 = p.n2 = 0

        A clear solution to this is p = k * (n1 x n2),
        as p.n1 = k*(n1 x n2).n1 = k*0 = 0, and similarly for p.n2
        k can be any real number.

        For points that are on the GC, they are distance 1 from the
        origin. Hence k = +-1, if n1 and n2 are unit vectors.

        Hence the intersection is +- n1 x n2
        """
        return self.axis.cross_product(other.axis)

from octagon_image_maps import image_map_data
class c_image_data(object):
    def __init__(self, image_filename):
        self.image_filename = image_filename
        self.map_data = {}
        self.base_orientation = None
        pass
    def set_base_orientation(self, orientation):
        self.base_orientation = orientation
        pass
    def add_map_data(self, other, direction, image_mapping):
        self.map_data[(other, direction)] = image_mapping
        pass
    def propagate(self, ids_propagated_to):
        """
        for every other
        if that other is not in the set ids_propagated_to
        then propagate the top optimized orientation to that other
        and return list of others that were propagated to
        """
        if self.base_orientation is None: return
        result = []
        for od in self.map_data:
            (other, direction) = od
            if other in ids_propagated_to: continue
            bqs = self.map_data[od]['best_qs']
            if len(bqs)==0: continue
            (iteration, score, opt_q, start_q) = bqs[-1]
            if (iteration==0) and (score<20):
                print "Ignoring poor match %s => %s"%(self.image_filename, other.image_filename)
                continue

            opt_q = gjslib_c.quaternion(r=opt_q[0],i=opt_q[1],j=opt_q[2],k=opt_q[3])
            if od[1]==0: opt_q = ~opt_q
            other.set_base_orientation(self.base_orientation * opt_q)
            print self.image_filename, '-->',   other.image_filename
            result.append(other)
            pass
        return result
    def __str__(self):
        r = self.image_filename +" => "
        for od in self.map_data:
            (other, direction)=od
            r += " "+other.image_filename
            pass
        return r
    pass

ids = {}
for (i0, i1) in image_map_data:
    if i0 not in ids: ids[i0] = c_image_data(i0)
    if i1 not in ids: ids[i1] = c_image_data(i1)
    ids[i0].add_map_data( other=ids[i1], direction=1, image_mapping=image_map_data[(i0, i1)] )
    ids[i1].add_map_data( other=ids[i0], direction=0, image_mapping=image_map_data[(i0, i1)] )
    pass

image_names = ids.keys()
image_names.sort()
key_image = ids[image_names[0]]
key_image.set_base_orientation(gjslib_c.quaternion(r=1))

ids_to_propagate=[key_image]
ids_propagated_to=set()
while len(ids_to_propagate)>0:
    next_id = ids_to_propagate.pop(0)
    ids_propagated_to.add(next_id)
    ids_to_propagate.extend(next_id.propagate(ids_propagated_to))
    pass

for i in image_names:
    print i, ids[i].base_orientation, str(ids[i])
    pass

for i in image_names:
    q = ids[i].base_orientation
    if q is None:
        print "# %s has no orientation"%(i,)
        pass
    else:
        print "('../images/%s',None,quaternion(r=%s,i=%s,j=%s,k=%s)),"%(i, q.r,q.i,q.j,q.k)
        pass
    pass


