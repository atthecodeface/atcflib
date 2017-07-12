/*a Documentation
 */
/*a Includes
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "quaternion.h"
#include "vector.h"

/*a Defines
 */
#define EPSILON (1E-20)
#define PI (M_PI)
#define COORD(n) (_coords[(n)*_stride])
#define OCOORD(o,n) ((o)._coords[(n)*(o)._stride])

/*a Infix operator methods for doubles
 */
/*f operator*=
 */
c_vector &c_vector::operator*=(double real)
{
    this->scale(real);
    return *this;
}

/*f operator/=
 */
c_vector &c_vector::operator/=(double real)
{
    this->scale(1.0/real);
    return *this;
}

/*a Infix operator methods for c_vector's
 */
/*f operator=
 */
c_vector &c_vector::operator=(const c_vector &other)
{
    _length = other._length;
    _stride = 1;
    _coords = _internal_coords;
    _coords_must_be_freed = 0;
    for (int i=0; i<VECTOR_MAX_LENGTH; i++) {
        COORD(i) = OCOORD(other,i);
    }
    return *this;
}

/*f operator+=
 */
c_vector &c_vector::operator+=(const c_vector &other)
{
    this->add_scaled(other,1.0);
    return *this;
}

/*f operator-=
 */
c_vector &c_vector::operator-=(const c_vector &other)
{
    this->add_scaled(other,-1.0);
    return *this;
}

/*a Constructors
 */
/*f c_vector::c_vector (void) - null
 */
c_vector::c_vector(void)
{
    _length = 0;
    _stride = 1;
    _coords_must_be_freed = 0;
    _coords = _internal_coords;
}

/*f c_vector::c_vector (length) - null
 */
c_vector::c_vector(int length)
{
    _length = length;
    _stride = 1;
    _coords = _internal_coords;
    _coords_must_be_freed = 0;
    for (int i=0; i<VECTOR_MAX_LENGTH; i++) {
        COORD(i) = 0;
    }
}

/*f c_vector::c_vector(other) - copy from other
 */
c_vector::c_vector(const c_vector &other)
{
    _length = other._length;
    _stride = 1;
    _coords = _internal_coords;
    _coords_must_be_freed = 0;
    for (int i=0; i<VECTOR_MAX_LENGTH; i++) {
        COORD(i) = OCOORD(other,i);
    }
}

/*f c_vector::c_vector (length, const double *coords)
 */
c_vector::c_vector(int length, const double *coords)
{
    _length = length;
    _stride = 1;
    _coords = _internal_coords;
    _coords_must_be_freed = 0;
    for (int i=0; i<VECTOR_MAX_LENGTH; i++) {
        COORD(i) = coords[i];
    }
}

/*f c_vector::c_vector (quaternion)
 */
c_vector::c_vector(const c_quaternion &quat)
{
    _length = 3;
    _stride = 1;
    _coords = _internal_coords;
    _coords_must_be_freed = 0;
    COORD(0) = quat.i();
    COORD(1) = quat.j();
    COORD(2) = quat.k();
}

/*f c_vector::~c_vector
 */
c_vector::~c_vector(void)
{
    if (_coords_must_be_freed) {
        free(_coords);
        _coords_must_be_freed = 0;
        _coords = _internal_coords;
    }
}

/*f c_vector::copy
 */
c_vector *c_vector::copy(void) const
{
    return new c_vector(*this);
}

/*f c_vector::__str__
 */
void c_vector::__str__(char *buffer, int buf_size) const
{
    static const char *formats[9] = {"",
                                     "(%lf,)",
                                     "(%lf, %lf)",
                                     "(%lf, %lf, %lf)",
                                     "(%lf, %lf, %lf, %lf)",
                                     "(%lf, %lf, %lf, %lf, %lf)",
                                     "(%lf, %lf, %lf, %lf, %lf, %lf)",
                                     "(%lf, %lf, %lf, %lf, %lf, %lf, %lf)",
                                     "(%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)"};
    snprintf(buffer, buf_size, formats[_length],
             COORD(0), COORD(1), COORD(2), COORD(3),
             COORD(4), COORD(5), COORD(6), COORD(7) );
    buffer[buf_size-1] = 0;
}

/*f c_vector::assign
 */
c_vector &c_vector::assign(const c_vector &other)
{
    for (int i=0; i<_length; i++) {
        COORD(i) = OCOORD(other,i);
    }
    return *this;
}

/*a Interrogation methods */
/*f c_vector::modulus_squared
 */
double c_vector::modulus_squared(void) const
{
    double l=0;
    for (int i=0; i<_length; i++) {
        l += COORD(i)*COORD(i);
    }
    return l;
}

/*f c_vector::modulus
 */
double c_vector::modulus(void) const
{
    return sqrt(modulus_squared());
}

/*a In-place vector operations */
/*f c_vector::add_scaled
 */
c_vector &c_vector::add_scaled(const c_vector &other, double scale)
{
    for (int i=0; i<_length; i++) {
        COORD(i) += OCOORD(other,i)*scale;
    }
    return *this;
}

/*f c_vector::scale
 */
c_vector &c_vector::scale(double scale)
{
    for (int i=0; i<_length; i++) {
        COORD(i) *= scale;
    }
    return *this;
}

/*f c_vector::normalize
 */
c_vector &c_vector::normalize(void)
{
    double l = this->modulus();
    if ((l>-EPSILON) && (l<EPSILON))
        return this->scale(0.0);
    return this->scale(1.0/l);
}

/*a Vector operations */
/*f c_vector::dot_product
 */
double c_vector::dot_product(const c_vector &other) const
{
    double l=0;
    for (int i=0; i<_length; i++) {
        l += COORD(i) * OCOORD(other,i);
    }
    return l;
}

/*f c_vector::cross_product3
 */
c_vector *c_vector::cross_product3(const c_vector &other) const
{
    c_vector *r=new c_vector(_length);
    OCOORD(*r,0) = COORD(1)*OCOORD(other,2) - COORD(2)*OCOORD(other,1);
    OCOORD(*r,1) = COORD(2)*OCOORD(other,0) - COORD(0)*OCOORD(other,2);
    OCOORD(*r,2) = COORD(0)*OCOORD(other,1) - COORD(1)*OCOORD(other,0);
    return r;
}

/*f c_vector::angle_axis_to_v3
 */
c_vector *c_vector::angle_axis_to_v3(const c_vector &other, double *cos_angle, double *sin_angle) const
{
    double tl, ol;
    tl = this->modulus();
    ol = other.modulus();

    //fprintf(stderr,"tl,ol: %lf, %lf\n",tl, ol);
    c_vector *axis = cross_product3(other);
    *axis /= (tl*ol);
    //fprintf(stderr,"axb:%lf,%lf,%lf\n",axis->_coords[0],axis->_coords[1],axis->_coords[2]);
    *cos_angle = dot_product(other) / (tl*ol);
    *sin_angle = axis->modulus();
    //fprintf(stderr,"angles:%lf,%lf\n",*cos_angle,*sin_angle);
    axis->normalize();
    //fprintf(stderr,"axis:%lf,%lf,%lf\n",axis->_coords[0],axis->_coords[1],axis->_coords[2]);
    return axis;
}

/*f c_vector::angle_axis_to_v3 (to quaternion)
 */
c_quaternion *c_vector::angle_axis_to_v3(const c_vector &other) const
{
    c_quaternion *r;
    double cos_angle, sin_angle;
    c_vector *axis = this->angle_axis_to_v3(other, &cos_angle, &sin_angle);
    r = new c_quaternion();
    r->from_rotation(cos_angle, sin_angle, axis->coords());
    return r;
}
