#!/usr/bin/env python
#a Imports
from OpenGL.GLUT import *
from OpenGL.GL import *
import gjslib_c
import math
import unittest
quaternion = gjslib_c.quaternion
epsilon = 1E-6
sqrt2 = math.sqrt(2)
pi = math.pi

#a Test
#c Simple quaternion tests
class SimpleQuatTests(unittest.TestCase):
    def check_quat(self,q,value):
        (r,i,j,k) = q.get()
        d = (r,i,j,k)
        self.assertEqual(len(value),len(d), 'BUG: Length of quaternion test value is not 4!!')
        for i in range(len(d)):
            self.assertTrue(abs(value[i]-d[i])<epsilon, 'Coordinate %d mismatches (%s, %s, %s)'%(i,str(value),str(q), str(d)))
            pass
        pass
    def check_rotation(self,q,value_angle,value_axis):
        (angle, axis) = q.to_rotation(degrees=True)
        self.assertTrue(abs(angle-value_angle)<epsilon, 'Angle mismatches (%s, %s)'%(str(angle),str(value_angle)))
        self.assertEqual(len(value_axis),len(axis), 'BUG: Length of axis test value is not 3!!')
        for i in range(len(axis)):
            self.assertTrue(abs(value_axis[i]-axis[i])<epsilon, 'Coordinate %d mismatches (%s, %s)'%(i,str(axis),str(value_axis)))
            pass
        pass
    def test_make(self):
        q = quaternion({"r":1,"i":2,"j":3,"k":4})
        self.check_quat(q,(1,2,3,4))
        self.check_quat(quaternion.identity(),(1,0,0,0))
        self.check_quat(quaternion.pitch(90,degrees=True),(1/sqrt2,0,1/sqrt2,0))
        self.check_quat(quaternion.roll(90,degrees=True),(1/sqrt2,0,0,1/sqrt2))
        self.check_quat(quaternion.yaw(90,degrees=True),(1/sqrt2,1/sqrt2,0,0))
        self.check_quat(quaternion.pitch(-pi/2),(1/sqrt2,0,-1/sqrt2,0))
        self.check_quat(quaternion.roll(-pi/2),(1/sqrt2,0,0,-1/sqrt2))
        self.check_quat(quaternion.yaw(-pi/2),(1/sqrt2,-1/sqrt2,0,0))
        self.check_quat(quaternion.of_sequence([("roll",pi/2)]),(1/sqrt2,0,0,1/sqrt2))
        self.check_quat(quaternion.of_sequence([("pitch",pi/2)]),(1/sqrt2,0,1/sqrt2,0))
        self.check_quat(quaternion.of_sequence([("yaw",pi/2)]),(1/sqrt2,1/sqrt2,0,0))
        self.check_quat(quaternion.of_euler(roll=90,degrees=True),(1/sqrt2,0,0,1/sqrt2))
        self.check_quat(quaternion.of_euler(pitch=90,degrees=True),(1/sqrt2,0,1/sqrt2,0))
        self.check_quat(quaternion.of_euler(yaw=90,degrees=True),(1/sqrt2,1/sqrt2,0,0))
        self.check_quat(quaternion.of_euler(roll=90,yaw=90,degrees=True),(0.5,0.5,0.5,0.5))
        self.check_quat(quaternion.of_sequence([("yaw",pi/2),("roll",pi/2)]),(0.5,0.5,0.5,0.5))
        self.check_quat(quaternion.of_sequence([("roll",pi/2),("yaw",pi/2)]),(0.5,0.5,-0.5,0.5))
        return
    def test_rotation(self):
        # +y to right, +x up, +z in front
        # roll + = roll clkwise inside looking forward
        # pitch + = nose up inside looking forward
        # yaw + = nose left inside looking forward
        self.check_rotation(quaternion.of_euler(roll=90,degrees=True), 90, (0,0,1))
        self.check_rotation(quaternion.of_euler(pitch=90,degrees=True), 90, (0,1,0))
        self.check_rotation(quaternion.of_euler(yaw=90,degrees=True),  90, (1,0,0))
        self.check_rotation(quaternion.of_euler(roll=30,degrees=True), 30, (0,0,1))
        self.check_rotation(quaternion.of_euler(pitch=30,degrees=True), 30, (0,1,0))
        self.check_rotation(quaternion.of_euler(yaw=30,degrees=True),  30, (1,0,0))
        self.check_rotation(quaternion.of_euler(roll=90,pitch=90,yaw=90,degrees=True), 90, (0,1,0))
        pass
    def test_infix(self):
        qr1 = quaternion(r=1)
        qi1 = quaternion(r=0,i=1)
        qj1 = quaternion(r=0,j=1)
        qk1 = quaternion(r=0,k=1)
        self.check_quat(qr1+qi1,(1,1,0,0))
        self.check_quat(qr1+qj1,(1,0,1,0))
        self.check_quat(qr1+qk1,(1,0,0,1))
        self.check_quat(qr1+qr1,(2,0,0,0))
        self.check_quat(qr1-qi1,(1,-1,0,0))
        self.check_quat(qr1-qj1,(1,0,-1,0))
        self.check_quat(qr1-qk1,(1,0,0,-1))
        self.check_quat(qr1-qr1,(0,0,0,0))
        self.check_quat(qr1*qi1,(0,1,0,0))
        self.check_quat(qr1*qj1,(0,0,1,0))
        self.check_quat(qr1*qk1,(0,0,0,1))
        self.check_quat(qr1*qr1,(1,0,0,0))
        self.check_quat(qr1*(qi1+qj1+qk1),(0,1,1,1))
        self.check_quat(qi1*(qi1+qj1+qk1),(-1,0,-1,1))
        self.check_quat(qj1*(qi1+qj1+qk1),(-1,1,0,-1))
        self.check_quat(qk1*(qi1+qj1+qk1),(-1,-1,1,0))
        self.check_quat((qi1+qj1+qk1)*(qi1+qj1+qk1),(-3,0,0,0))
        self.check_quat((qi1+qj1+qk1)*(-qi1-qj1-qk1),(3,0,0,0))
        self.assertEqual(abs((qi1+qj1+qk1)*(-qi1-qj1-qk1)),3)
        self.assertEqual(abs(qr1),1)
        self.assertEqual(abs(qi1),1)
        self.assertEqual(abs(qj1),1)
        self.assertEqual(abs(qk1),1)
        self.assertTrue(bool(qr1))
        self.assertTrue(bool(qi1))
        self.assertTrue(bool(qj1))
        self.assertTrue(bool(qk1))
        self.assertTrue(bool(qr1+qr1))
        self.assertTrue(bool(qi1+qr1))
        self.assertTrue(bool(qj1+qr1))
        self.assertTrue(bool(qk1+qr1))
        self.assertFalse(bool(qr1-qr1))
        self.assertTrue(bool(qi1-qr1))
        self.assertTrue(bool(qj1-qr1))
        self.assertTrue(bool(qk1-qr1))
        self.assertFalse(bool(qi1-qi1))
        self.assertFalse(bool(qj1-qj1))
        self.assertFalse(bool(qk1-qk1))
        pass
    def dont_test_other_stuff(self):
        print "Identity, roll30, pitch30, yaw30..."
        i = c_quaternion.identity()
        print i, i.get_matrix(), i.to_euler(degrees=True)

        roll30 = c_quaternion.roll(math.radians(30))
        print roll30, roll30.get_matrix(), roll30.to_euler(degrees=True)

        pitch30 = c_quaternion.pitch(math.radians(30))
        print pitch30, pitch30.get_matrix(), pitch30.to_euler(degrees=True)

        yaw30 = c_quaternion.yaw(math.radians(30))
        print yaw30, yaw30.get_matrix(), yaw30.to_euler(degrees=True)

        print
        print "To/from matrix"
        print "Roll30"
        roll30_m = roll30.get_matrixn()
        print c_quaternion.identity().from_matrix(roll30_m)
        print "Pitch30"
        pitch30_m = pitch30.get_matrixn()
        print c_quaternion.identity().from_matrix(pitch30_m)
        print "Yaw30"
        yaw30_m = yaw30.get_matrixn()
        print c_quaternion.identity().from_matrix(yaw30_m)
        print "Roll30 of Pitch30"
        q = roll30.multiply(pitch30)
        print q, c_quaternion.identity().from_matrix(q.get_matrixn())
        print "Roll30 of Pitch30 of Yaw30"
        q = roll30.multiply(pitch30.multiply(yaw30))
        print q, c_quaternion.identity().from_matrix(q.get_matrixn())

        print
        print "90 degrees roll, pitch, yaw from 3*roll30 etc"
        roll90 = roll30.multiply(roll30.multiply(roll30))
        print roll90, roll90.get_matrix(), roll90.to_euler(degrees=True)

        yaw90 = yaw30.multiply(yaw30.multiply(yaw30))
        print yaw90, yaw90.get_matrix(), yaw90.to_euler(degrees=True)

        pitch90 = pitch30.multiply(pitch30.multiply(pitch30))
        print pitch90, pitch90.get_matrix(), pitch90.to_euler(degrees=True)

        print
        print "roll 180, 360..."
        roll180 = roll90.multiply(roll90)
        print roll180, roll180.get_matrix(), roll180.to_euler(degrees=True)
        roll360 = roll180.multiply(roll180)
        print roll360, roll360.get_matrix(), roll360.to_euler(degrees=True)

        print
        print "yaw90 of pitch90"
        k = yaw90.multiply(pitch90)
        print k, k.get_matrix(), k.to_euler(degrees=True)

        print
        print "roll90 of yaw90 of pitch90, and then repeated, and then repeated; second should be 180,0,180, last should be 0,0,0"
        k = roll90.multiply(yaw90.multiply(pitch90))
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = k.multiply(k)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = k.multiply(k)
        print k, k.get_matrix(), k.to_euler(degrees=True)

        print
        print "roll30 OF a yaw30..."
        k = roll30.multiply(yaw30)
        print k, k.get_matrix(), k.to_euler(degrees=True)

        print
        print "roll -90 of yaw -90 of pitch 90 of yaw 90, should be the identity"
        x = roll90.multiply(roll90.multiply(roll90.multiply(yaw90.multiply(yaw90.multiply(yaw90.multiply(pitch90.multiply(yaw90)))))))
        print x, x.get_matrix(), x.to_euler(degrees=True)

        print
        print "roll90, pitch90, yaw90, to/from euler"
        k = c_quaternion(euler=roll90.to_euler(degrees=True), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = c_quaternion(euler=pitch90.to_euler(degrees=True), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = c_quaternion(euler=yaw90.to_euler(degrees=True), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)

        print
        print "from euler of rpy 90,90,0 in all the possible orders"
        k = c_quaternion(euler=(0,90,90), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = c_quaternion(euler=(90,0,90), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = c_quaternion(euler=(90,90,0), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)

        print
        print "from euler of rpy 10,20,30 in all the possible orders"
        k = c_quaternion(euler=(10,20,30), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = c_quaternion(euler=(20,30,10), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = c_quaternion(euler=(30,10,20), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)

        k = c_quaternion(euler=(10,30,20), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = c_quaternion(euler=(30,20,10), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)
        k = c_quaternion(euler=(20,10,30), degrees=True)
        print k, k.get_matrix(), k.to_euler(degrees=True)

        print
        print "Roll 30 of yaw 20 of pitch 10"
        k = ( c_quaternion.roll(math.radians(30)).multiply( 
                c_quaternion.yaw(math.radians(20)).multiply( 
                    c_quaternion.pitch(math.radians(10)))) )
        print k, k.get_matrix(), k.to_euler(degrees=True)

        print
        print "Pitch 30 of yaw 20 of roll 10"
        k = ( c_quaternion.pitch(math.radians(30)).multiply( 
                c_quaternion.yaw(math.radians(20)).multiply( 
                    c_quaternion.roll(math.radians(10)))) )
        print k, k.get_matrix(), k.to_euler(degrees=True)

        print c_quaternion({'r': 0.6330, 'i': 0.7226, 'j':-0.2194, 'k':-0.1705})
        print c_quaternion({'r': 0.7607, 'i': 0.5670, 'j': 0.2328, 'k': 0.2133})

#a Toplevel
loader = unittest.TestLoader().loadTestsFromTestCase
suites = [ loader(SimpleQuatTests),
           ]

if __name__ == '__main__':
    unittest.main()
