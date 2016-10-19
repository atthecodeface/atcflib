/*a Documentation
Ported from gjslib.math.quaternion python
    # +y to right, +x up, +z in front
    # roll + = roll clkwise inside looking forward
    # pitch + = nose up inside looking forward
    # yaw + = nose left inside looking forward
    # when given roll, pitch, yaw the order applies is roll(pitch(yaw())) - i.e. yaw is applied first

Note that quaternion q = w + xi + yj + zk maps (0,0,1) to (2*(z*x-w*y), 2*(w*x+y*z), w*w-x*x-y*y+z*z)

Note that a quaternion can be represented also as q0 + q_  (i.e. real and 'ijk' parts)
A unit quatnernion has q0 = cos(theta/2), |q_| as sin(theta/2), and the axis of rotation is q_.

Now p * q = (p0 + p_) * (q0 + q_) = p0q0-p_.q_ + p0q_ + q0p_ + p_ x q_

Consider q . (0,0,1) . q* = (-qz + q0.(0,0,1) + q_ x (0,0,1)) . q*
 = (-qz + (0,0,q0) + q_ x (0,0,1) ) . (q0 - q_)
q_ = (qx,qy,qz)
q_ x (0,0,1) = (qy, -qx, 0)
q . (0,0,1) . q* = (-qz + (qy, -qx, q0)) . (q0 - q_)
                 = -q0.qz+q_.(qy, -qx, q0) + "p0q_+q0p_+p+xq_"
                 = (-q0.qz+qx.qy-qx.qy+qz.q0) + "p0q_+q0p_+p+xq_"
                 = "p0q_+q0p_+p+xq_"
                 = qz.q_ + q0.(qy, -qx, q0) - (qy, -qx, q0)x(qx,qy,qz)
(qy, -qx, q0)x(qx,qy,qz) = (-qx.qz-q0.qy, q0.qx-qy.qz, qy.qy+qx.qx)
Hence
q . (0,0,1) . q* = (qx.qz, qy.qz, qz.qz) + (q0.qy, -q0.qx, q0.q0) - (-qx.qz-q0.qy, q0.qx-qy.qz, qy.qy+qx.qx) 
                 = (2qx.qz + 2q0.qy,   2qy.qz - 2q0.qx, q0.q0-qx.qx-qy.qy+qz.qz)
                 = (2qx.qz + 2q0.qy,   2qy.qz - 2q0.qx, 2q0.q0 + 2qz.qz - 1)

Note that if we do the inverse rotation, we have q0,-qx,-qy,-qz and hence
q* . (0,0,1) . q = (2qx.qz - 2q0.qy,   2qy.qz + 2q0.qx, 2q0.q0 + 2qz.qz - 1)


for w in range(4):
    for x in range(4):
        for y in range(4):
           for z in range(4):
               q = quaternion(r=w,i=x,j=y,k=z)
               r = q.rotate_vector((0,0,1))
               print r, (2*(z*x-w*y), 2*(w*x+y*z), w*w-x*x-y*y+z*z)
               pass
           pass
        pass
    pass
pass

 */
/*a Includes
 */
#include <math.h>
#include <stdio.h>
#include "vector.h"
#include "quaternion.h"

/*a Defines
 */
#define EPSILON (1E-20)
#define PI (M_PI)

/*a Infix operator methods for doubles
 */
c_quaternion &c_quaternion::operator=(double real)
{
    quat.r = real;
    quat.i = quat.j = quat.k = 0.0;
    return *this;
}

c_quaternion &c_quaternion::operator+=(double real)
{
    quat.r += real;
    return *this;
}

c_quaternion &c_quaternion::operator-=(double real)
{
    quat.r -= real;
    return *this;
}

c_quaternion &c_quaternion::operator*=(double real)
{
    this->scale(real);
    return *this;
}

c_quaternion &c_quaternion::operator/=(double real)
{
    this->scale(1.0/real);
    return *this;
}

/*a Infix operator methods for c_quaternion's
 */
c_quaternion &c_quaternion::operator=(const c_quaternion &other)
{
    quat = other.quat;
    return *this;
}

c_quaternion &c_quaternion::operator+=(const c_quaternion &other)
{
    this->add_scaled(&other,1.0);
    return *this;
}

c_quaternion &c_quaternion::operator-=(const c_quaternion &other)
{
    this->add_scaled(&other,-1.0);
    return *this;
}

c_quaternion &c_quaternion::operator*=(const c_quaternion &other)
{
    this->multiply(&other);
    return *this;
}

c_quaternion &c_quaternion::operator/=(const c_quaternion &other)
{
    c_quaternion r;
    r = other;
    this->multiply(r.reciprocal());
    return *this;
}

/*a Constructors
 */
/*f c_quaternion::c_quaternion (void) - null
 */
c_quaternion::c_quaternion(void)
{
    quat.r = 0.0;
    quat.i = 0.0;
    quat.j = 0.0;
    quat.k = 0.0;
}

/*f c_quaternion::c_quaternion(other) - copy from other
 */
c_quaternion::c_quaternion(const c_quaternion *other)
{
    quat = other->quat;
}

/*f c_quaternion::c_quaternion(r,j,k,k)
 */
c_quaternion::c_quaternion(double r, double i, double j, double k)
{
    quat.r = r;
    quat.i = i;
    quat.j = j;
    quat.k = k;
}

/*f c_quaternion::c_quaternion(vector)
 */
c_quaternion::c_quaternion(const c_vector &vector)
{
    quat.r = 0;
    quat.i = vector.coords()[0];
    quat.j = vector.coords()[1];
    quat.k = vector.coords()[2];
}

/*f c_quaternion::copy
 */
c_quaternion *c_quaternion::copy(void) const
{
    c_quaternion *quat;
    quat = new c_quaternion(this);
    return quat;
}

/*f c_quaternion::__str__
 */
char *
c_quaternion::__str__(char *buffer, int buf_size) const
{
    snprintf(buffer, buf_size, "(%lf, %lf, %lf, %lf)",
             quat.r, quat.i, quat.j, quat.k );
    buffer[buf_size-1] = 0;
    return buffer;
}

/*f c_quaternion::get_rijk
 */
void c_quaternion::get_rijk(double rijk[4]) const
{
    rijk[0] = quat.r;
    rijk[1] = quat.i;
    rijk[2] = quat.j;
    rijk[3] = quat.k;
}

/*f c_quaternion::from_euler
 */
c_quaternion *c_quaternion::from_euler(double roll, double pitch, double yaw, int degrees)
{
    double cr, cp, cy;
    double sr, sp, sy;
    double crcp, srsp;

    if (degrees) {
        roll  *= PI/180;
        yaw   *= PI/180;
        pitch *= PI/180;
    }

    cr = cos(roll/2);
    cp = cos(pitch/2);
    cy = cos(yaw/2);
    sr = sin(roll/2);
    sp = sin(pitch/2);
    sy = sin(yaw/2);

    crcp = cr * cp;
    srsp = sr * sp;
    quat.r = cy * crcp + sy * srsp;
    quat.i = sy * crcp - cy * srsp;
    quat.j = cy * cr * sp + sy * sr * cp;
    quat.k = cy * sr * cp - sy * cr * sp;
    return this;
}

/*f c_quaternion::lookat
 */
c_quaternion *c_quaternion::lookat(const double xyz[3], const double up[3])
{
    double len_xyz = sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
    double pitch =  asin(xyz[0] / len_xyz);
    double yaw   = -atan2(xyz[1] , xyz[2]);

    double cy = cos(yaw);
    double sy = sin(yaw);
    double cp = cos(pitch);
    double sp = sin(pitch);
    double roll = atan2( up[1]*cy    + up[2]*sy,
                         up[1]*sy*sp - up[2]*cy*sp + up[0]*cp );
    return this->from_euler(-roll, -pitch, -yaw, 0)->conjugate();
}

/*f c_quaternion::as_euler
 */
void c_quaternion::as_euler(double rpy[3]) const
{
    double l=modulus();
    double r=quat.r;
    double i=quat.i;
    double j=quat.j;
    double k=quat.k;
    double roll, pitch, yaw;
    if (l>1E-9) {
        r/=l; i/=l; j/=l; k/=l;
    }

    yaw = atan2(2*(r*i+j*k), 1-2*(i*i+j*j));
    if ((2*(r*j-i*k)<-1) || (2*(r*j-i*k)>1)) {
        pitch = asin(1.0);
    } else {
        pitch = asin(2*(r*j-i*k));
    }
    roll  = atan2(2*(r*k+i*j), 1-2*(j*j+k*k));
    rpy[0] = roll;
    rpy[1] = pitch;
    rpy[2] = yaw;
}

/*f More
 */
/*
    #f get_matrix_values
    def get_matrix_values( self ):
        if self.matrix is None: self.create_matrix()
        return self.matrix

    #f get_matrix
    def get_matrix( self, order=3 ):
        self.create_matrix()
        m = self.matrix
        if order==3:
            return matrix(data=(m[0][0], m[0][1], m[0][2],
                                     m[1][0], m[1][1], m[1][2],
                                     m[2][0], m[2][1], m[2][2],))
        if order==4:
            return matrix(data=(m[0][0], m[0][1], m[0][2], 0.0,
                                     m[1][0], m[1][1], m[1][2], 0.0,
                                     m[2][0], m[2][1], m[2][2], 0.0,
                                     0.0,0.0,0.0,1.0))
        raise Exception("Get matrix of unsupported order")

    #f create_matrix
    def create_matrix( self ):
        # From http://www.gamasutra.com/view/feature/131686/rotating_objects_using_quaternions.php?page=2
        # calculate coefficients
        l = self.modulus()

        x2 = self.quat["i"] + self.quat["i"]
        y2 = self.quat["j"] + self.quat["j"] 
        z2 = self.quat["k"] + self.quat["k"]
        xx = self.quat["i"] * x2
        xy = self.quat["i"] * y2
        xz = self.quat["i"] * z2
        yy = self.quat["j"] * y2
        yz = self.quat["j"] * z2
        zz = self.quat["k"] * z2
        wx = self.quat["r"] * x2
        wy = self.quat["r"] * y2
        wz = self.quat["r"] * z2
        m = [[0,0,0,0.],[0,0,0,0.],[0,0,0,0.],[0.,0.,0.,1.]]

        m[0][0] = l - (yy + zz)/l
        m[1][0] = (xy - wz)/l
        m[2][0] = (xz + wy)/l

        m[0][1] = (xy + wz)/l
        m[1][1] = l - (xx + zz)/l
        m[2][1] = (yz - wx)/l

        m[0][2] = (xz - wy)/l
        m[1][2] = (yz + wx)/l
        m[2][2] = l - (xx + yy)/l

        self.matrix = m
        pass
    #f from_matrix
    def from_matrix( self, matrix, epsilon=1E-6 ):
        """
        """
        d = matrix.determinant()
        if (d>-epsilon) and (d<epsilon):
            raise Exception("Singular matrix supplied")
        m = matrix.copy()
        if d<0: d=-d
        m.scale(1.0/math.pow(d,1/3.0))

        yaw   = math.atan2(m[1,2],m[2,2])
        roll  = math.atan2(m[0,1],m[0,0])
        if m[0,2]<-1 or m[0,2]>1:
            pitch=-math.asin(1)
        else:
            pitch = -math.asin(m[0,2])
        q0 = quaternion.of_euler(roll=roll, pitch=pitch, yaw=yaw, degrees=False)

        yaw   = math.atan2(m[2,1],m[2,2])
        roll  = math.atan2(m[1,0],m[0,0])
        if m[2,0]<-1 or m[2,0]>1:
            pitch=-math.asin(1)
        else:
            pitch = -math.asin(m[2,0])
        q1 = quaternion.of_euler(roll=roll, pitch=pitch, yaw=yaw, degrees=False)
        self.quat["r"] = (q0.quat["r"] + q1.quat["r"])/2.0
        self.quat["i"] = (q0.quat["i"] - q1.quat["i"])/2.0
        self.quat["j"] = (q0.quat["j"] - q1.quat["j"])/2.0
        self.quat["k"] = (q0.quat["k"] - q1.quat["k"])/2.0
        self.normalize()
        self.matrix = None
        return self
*/

/*f c_quaternion::from_rotation
 */
c_quaternion *c_quaternion::from_rotation(double angle, const double axis[3], int degrees)
{
    if (degrees) {
        angle *= PI/180;
    }
    double s = sin(angle/2);
    double c = cos(angle/2);
    quat.r = c;
    quat.i = s*axis[0];
    quat.j = s*axis[1];
    quat.k = s*axis[2];
    return this;
}

/*f c_quaternion::from_rotation
 */
c_quaternion *c_quaternion::from_rotation(double cos_angle, double sin_angle, const double *axis)
{
    double c, s;
    // cos(2x) = 2(cos(x)^2)-1 = cos(x)^2 - sin(x)^2
    // sin(2x) = 2sin(x)cos(x)
    // cos(x) = +-sqrt(1+cos(2x))/sqrt(2)
    // sin(x) = +-sin(2x)/sqrt(1+cos(2x))/sqrt(2)
    // chose +ve for cos(x) (-90<x<90), and sin(x) same segment as sin(2x)
    if (cos_angle>=1){
        quat.r = 1;
        quat.i = 0;
        quat.j = 0;
        quat.k = 0;
        return this;
    }
    if (cos_angle<=-1){
        quat.r = 0; // rotate by 180 degrees around _any_ axis
        quat.i = 1;
        quat.j = 0;
        quat.k = 0;
        return this;
    }
    c = sqrt((1+cos_angle)/2);
    s = sqrt(1-c*c);
    if (sin_angle<0) {
        s = -s;
    }
    quat.r = c;
    quat.i = s*axis[0];
    quat.j = s*axis[1];
    quat.k = s*axis[2];
    return this;
}

/*f c_quaternion::as_rotation
 */
double c_quaternion::as_rotation(double axis[3]) const
{
    double m=this->modulus();
    double angle = 2*acos(quat.r/m);

    double sm = m*sin(angle/2);
    if (fabs(sm)>EPSILON) {
        axis[0] = quat.i/sm;
        axis[1] = quat.j/sm;
        axis[2] = quat.k/sm;
    } else {
        axis[0] = 0;
        axis[1] = 0;
        axis[2] = 0;
    }
    return angle;
}

/*f c_quaternion::as_rotation
 */
double c_quaternion::as_rotation(c_vector &vector) const
{
    double axis[3];
    double m=this->modulus();
    double angle = 2*acos(quat.r/m);

    double sm = m*sin(angle/2);
    if (fabs(sm)>EPSILON) {
        axis[0] = quat.i/sm;
        axis[1] = quat.j/sm;
        axis[2] = quat.k/sm;
    } else {
        axis[0] = 0;
        axis[1] = 0;
        axis[2] = 0;
    }
    vector = c_vector(3,axis);
    return angle;
}

/*f c_quaternion::conjugate
 */
c_quaternion *c_quaternion::conjugate(void)
{
    quat.r = quat.r;
    quat.i = -quat.i;
    quat.j = -quat.j;
    quat.k = -quat.k;
    return this;
}

/*f c_quaternion::reciprocal
 */
c_quaternion *c_quaternion::reciprocal(void)
{
    this->conjugate();
    this->scale(1.0/this->modulus_squared());
    return this;
}


/*f c_quaternion::add_scaled
 */
c_quaternion *c_quaternion::add_scaled(const c_quaternion *other, double scale)
{
    quat.r += other->quat.r*scale;
    quat.i += other->quat.i*scale;
    quat.j += other->quat.j*scale;
    quat.k += other->quat.k*scale;
    return this;
}

/*f c_quaternion::modulus_squared
 */
double c_quaternion::modulus_squared(void) const
{
    return (quat.r*quat.r + 
            quat.i*quat.i + 
            quat.j*quat.j + 
            quat.k*quat.k);
}

/*f c_quaternion::modulus
 */
double c_quaternion::modulus(void) const
{
    return sqrt( quat.r*quat.r + 
                 quat.i*quat.i + 
                 quat.j*quat.j + 
                 quat.k*quat.k);
}

/*f c_quaternion::scale
 */
c_quaternion *c_quaternion::scale(double scale)
{
    quat.r *= scale;
    quat.i *= scale;
    quat.j *= scale;
    quat.k *= scale;
    return this;
}

/*f c_quaternion::normalize
 */
c_quaternion *c_quaternion::normalize(void)
{
    double l = this->modulus();
    if ((l>-EPSILON) && (l<EPSILON))
        return this;
    return this->scale(1.0/l);
}

/*f c_quaternion::multiply
 */
c_quaternion *c_quaternion::multiply(const c_quaternion *other, int premultiply)
{
    double r1, i1, j1, k1;
    double r2, i2, j2, k2;
    const c_quaternion *a, *b;
    a=this; b=other;
    if (premultiply) {
        a=other; b=this;
    }

    r1 = a->quat.r;
    i1 = a->quat.i;
    j1 = a->quat.j;
    k1 = a->quat.k;

    r2 = b->quat.r;
    i2 = b->quat.i;
    j2 = b->quat.j;
    k2 = b->quat.k;

    this->quat.r = r1*r2 - i1*i2 - j1*j2 - k1*k2;
    this->quat.i = r1*i2 + i1*r2 + j1*k2 - k1*j2;
    this->quat.j = r1*j2 + j1*r2 + k1*i2 - i1*k2;
    this->quat.k = r1*k2 + k1*r2 + i1*j2 - j1*i2;
    return this;
}

/*f c_quaternion::rotate_vector
 */
c_quaternion &c_quaternion::rotate_vector(const c_vector &vector) const
{
    c_quaternion *r = new c_quaternion();
    *r = (*this) * c_quaternion(vector) * this->copy()->conjugate();
    return *r;
}

/*f c_quaternion::angle_axis
 */
c_quaternion &c_quaternion::angle_axis(const c_quaternion &other, c_vector &vector) const
{
    c_quaternion a, b;
    c_quaternion *r = new c_quaternion();
    double cos_angle, sin_angle;
    //fprintf(stderr,"vector:%lf,%lf,%lf\n",vector.coords()[0],vector.coords()[1],vector.coords()[2]);
    a = this->rotate_vector(vector);
    //fprintf(stderr,"a:%lf,%lf,%lf,%lf\n",a.r(),a.i(),a.j(),a.k());
    b = other.rotate_vector(vector);
    //fprintf(stderr,"b:%lf,%lf,%lf,%lf\n",b.r(),b.i(),b.j(),b.k());
    c_vector axis = c_vector(&a).angle_axis_to_v(c_vector(&b), &cos_angle, &sin_angle);
    //fprintf(stderr,"axis:%lf,%lf,%lf - %lf,%lf\n",axis.coords()[0],axis.coords()[1],axis.coords()[2],cos_angle,sin_angle);
    *r = c_quaternion::of_rotation(cos_angle, sin_angle, axis.coords());
    return *r;
}

/*f c_quaternion::distance_to
 */
double c_quaternion::distance_to(const c_quaternion &other) const
{
    c_quaternion q;
    q = (*this / other);
    return 1-((q*q).r());
}

/*a Others
 */

/*
    #f rotation_multiply
    def rotation_multiply( self, other ):
        A = (self.quat["r"] + self.quat["i"])*(other.quat["r"] + other.quat["i"])
        B = (self.quat["k"] - self.quat["j"])*(other.quat["j"] - other.quat["k"])
        C = (self.quat["r"] - self.quat["i"])*(other.quat["j"] + other.quat["k"]) 
        D = (self.quat["j"] + self.quat["k"])*(other.quat["r"] - other.quat["i"])
        E = (self.quat["i"] + self.quat["k"])*(other.quat["i"] + other.quat["j"])
        F = (self.quat["i"] - self.quat["k"])*(other.quat["i"] - other.quat["j"])
        G = (self.quat["r"] + self.quat["j"])*(other.quat["r"] - other.quat["k"])
        H = (self.quat["r"] - self.quat["j"])*(other.quat["r"] + other.quat["k"])
        r = B + (-E - F + G + H) /2
        i = A - (E + F + G + H)/2 
        j = C + (E - F + G - H)/2 
        k = D + (E - F - G + H)/2
        return quaternion( quat={"r":r, "i":i, "j":j, "k":k } )
    #f interpolate
    def interpolate( self, other, t ):
        cosom = ( self.quat["i"] * other.quat["i"] +
                  self.quat["j"] * other.quat["j"] +
                  self.quat["k"] * other.quat["k"] +
                  self.quat["r"] * other.quat["r"] )
        abs_cosom = cosom
        sgn_cosom = 1
        if (cosom <0.0): 
            abs_cosom = -cosom
            sgn_cosom = -1
            pass

        # calculate coefficients
        if ( (1.0-abs_cosom) > epsilon ):
            #  standard case (slerp)
            omega = math.acos(abs_cosom);
            sinom = math.sin(omega);
            scale0 = math.sin((1.0 - t) * omega) / sinom;
            scale1 = math.sin(t * omega) / sinom;
            pass
        else:
            # "from" and "to" quaternions are very close 
            #  ... so we can do a linear interpolation
            scale0 = 1.0 - t;
            scale1 = t;
            pass

        # calculate final values
        i = scale0 * self.quat["i"] + scale1 * sgn_cosom * other.quat["i"]
        j = scale0 * self.quat["j"] + scale1 * sgn_cosom * other.quat["j"]
        k = scale0 * self.quat["k"] + scale1 * sgn_cosom * other.quat["k"]
        r = scale0 * self.quat["r"] + scale1 * sgn_cosom * other.quat["r"]
        return quaternion( quat={"r":r, "i":i, "j":j, "k":k } )

                }
}
*/

