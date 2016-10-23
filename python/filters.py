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
        r = []
        for t in self.f.times:
            r.append(t/1000/100/27)
            pass
        return r
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

