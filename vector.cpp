/*a Documentation
 */
/*a Includes
 */
#include <math.h>
#include <stdio.h>
#include "quaternion.h"
#include "vector.h"

/*a Defines
 */
#define EPSILON (1E-20)
#define PI (M_PI)

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
    for (int i=0; i<VECTOR_MAX_LENGTH; i++) {
        _coords[i] = other._coords[i];
    }
    return *this;
}

/*f operator+=
 */
c_vector &c_vector::operator+=(const c_vector &other)
{
    this->add_scaled(&other,1.0);
    return *this;
}

/*f operator-=
 */
c_vector &c_vector::operator-=(const c_vector &other)
{
    this->add_scaled(&other,-1.0);
    return *this;
}

/*a Constructors
 */
/*f c_vector::c_vector (length) - null
 */
c_vector::c_vector(int length)
{
    _length = length;
    for (int i=0; i<VECTOR_MAX_LENGTH; i++) {
        _coords[i] = 0;
    }
}

/*f c_vector::c_vector(other) - copy from other
 */
c_vector::c_vector(const c_vector &other)
{
    _length = other._length;
    for (int i=0; i<VECTOR_MAX_LENGTH; i++) {
        _coords[i] = other._coords[i];
    }
}

/*f c_vector::c_vector (length, double *coords)
 */
c_vector::c_vector(int length, double *coords)
{
    _length = length;
    for (int i=0; i<VECTOR_MAX_LENGTH; i++) {
        _coords[i] = coords[i];
    }
}

/*f c_vector::c_vector (quaternion)
 */
c_vector::c_vector(const c_quaternion &quat)
{
    _length = 3;
    _coords[0] = quat.i();
    _coords[1] = quat.j();
    _coords[2] = quat.k();
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
             _coords[0], _coords[1], _coords[2], _coords[3],
             _coords[4], _coords[5], _coords[6], _coords[7] );
    buffer[buf_size-1] = 0;
}

/*f c_vector::add_scaled
 */
c_vector *c_vector::add_scaled(const c_vector *other, double scale)
{
    for (int i=0; i<_length; i++) {
        _coords[i] += other->_coords[i]*scale;
    }
    return this;
}

/*f c_vector::modulus_squared
 */
double c_vector::modulus_squared(void) const
{
    double l=0;
    for (int i=0; i<_length; i++) {
        l += _coords[i]*_coords[i];
    }
    return l;
}

/*f c_vector::modulus
 */
double c_vector::modulus(void) const
{
    return sqrt(modulus_squared());
}

/*f c_vector::scale
 */
c_vector *c_vector::scale(double scale)
{
    for (int i=0; i<_length; i++) {
        _coords[i] *= scale;
    }
    return this;
}

/*f c_vector::normalize
 */
c_vector *c_vector::normalize(void)
{
    double l = this->modulus();
    if ((l>-EPSILON) && (l<EPSILON))
        return this->scale(0.0);
    return this->scale(1.0/l);
}

/*f c_vector::dot_product
 */
double c_vector::dot_product(const c_vector &other)
{
    double l=0;
    for (int i=0; i<_length; i++) {
        l += _coords[i] * other._coords[i];
    }
    return l;
}

/*f c_vector::cross_product
 */
c_vector &c_vector::cross_product(const c_vector &other) const
{
    c_vector *r=new c_vector(_length);
    r->_coords[0] = _coords[1]*other._coords[2] - _coords[2]*other._coords[1];
    r->_coords[1] = _coords[2]*other._coords[0] - _coords[0]*other._coords[2];
    r->_coords[2] = _coords[0]*other._coords[1] - _coords[1]*other._coords[0];
    return *r;
}

/*f c_vector::angle_axis_to_v
 */
c_vector &c_vector::angle_axis_to_v(const c_vector &other, double *cos_angle, double *sin_angle)
{
    double tl, ol;
    tl = this->modulus();
    ol = other.modulus();

    //fprintf(stderr,"tl,ol: %lf, %lf\n",tl, ol);
    c_vector *axis = new c_vector(cross_product(other)/ (tl*ol));
    //fprintf(stderr,"axb:%lf,%lf,%lf\n",axis->_coords[0],axis->_coords[1],axis->_coords[2]);
    *cos_angle = dot_product(other) / (tl*ol);
    *sin_angle = axis->modulus();
    //fprintf(stderr,"angles:%lf,%lf\n",*cos_angle,*sin_angle);
    axis->normalize();
    //fprintf(stderr,"axis:%lf,%lf,%lf\n",axis->_coords[0],axis->_coords[1],axis->_coords[2]);
    return *axis;
}

