/** Copyright (C) 2016-2017,  Gavin J Stark.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          vector.cpp
 * @brief         Vector class and operations
 *
 */

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

/*a Infix operator methods for Ts
 */
/*f operator*=
 */
template <typename T>
c_vector<T> &c_vector<T>::operator*=(T real)
{
    this->scale(real);
    return *this;
}

/*f operator/=
 */
template <typename T>
c_vector<T> &c_vector<T>::operator/=(T real)
{
    this->scale(1.0/real);
    return *this;
}

/*a Constructors
 */
/*f c_vector<T>::init
 */
template <typename T>
void c_vector<T>::init(void)
{
    _length = 0;
    _max_length = VECTOR_MAX_LENGTH;
    _stride = 1;
    _coords = _internal_coords;
    _coords_must_be_freed = 0;
}

/*f c_vector<T>::set_length
 */
template <typename T>
int c_vector<T>::set_length(int length, int allow_reallocate)
{
    if (length<=_max_length) {
        _length = length;
        return 0;
    }
    if (!allow_reallocate) {
        return -1;
    }
    if (length<=VECTOR_MAX_LENGTH) {
        if (_coords_must_be_freed) {
            free(_coords);
            _coords_must_be_freed = 0;
        }
        _coords = _internal_coords;
        _length = length;
        _max_length = VECTOR_MAX_LENGTH;
        _stride = 1;
        return 0;
    }
    if (_coords_must_be_freed) {
        free(_coords);
    }
    _coords = (T *)malloc(sizeof(T)*length);
    if (_coords==NULL) {
        _coords_must_be_freed = 0;
        _length = 0;
        _max_length = 0;
        _stride = 1;
        return -1;
    }
    _coords_must_be_freed = 1;
    _length = length;
    _max_length = length;
    _stride = 1;
    return 0;
}

/*f c_vector<T>::c_vector (void) - null
 */
template <typename T>
c_vector<T>::c_vector(void)
{
    _length = 0;
    _stride = 1;
    _coords_must_be_freed = 0;
    _coords = _internal_coords;
}

/*f c_vector<T>::c_vector (length) - null
 */
template <typename T>
c_vector<T>::c_vector(int length)
{
    init();
    set_length(length, 1);
    for (int i=0; i<_length; i++) {
        COORD(i) = 0;
    }
}

/*f c_vector<T>::c_vector(other) - copy from other
 */
template <typename T>
c_vector<T>::c_vector(const c_vector<T> &other)
{
    init();
    set_length(other._length, 1);
    for (int i=0; i<_length; i++) {
        COORD(i) = OCOORD(other,i);
    }
}

/*f c_vector<T>::c_vector (length, int stride, T *coords)
 */
template <typename T>
c_vector<T>::c_vector(int length, int stride, T *coords)
{
    init();
    _length = length;
    _max_length = length;
    _stride = stride;
    _coords = coords;
    _coords_must_be_freed = 0;
}

/*f c_vector<T>::c_vector (quaternion<T>)
 */
template <typename T>
c_vector<T>::c_vector(const c_quaternion<T> &quat)
{
    init();
    set_length(3,1);
    COORD(0) = quat.i();
    COORD(1) = quat.j();
    COORD(2) = quat.k();
}

/*f c_vector<T>::~c_vector<T>
 */
template <typename T>
c_vector<T>::~c_vector<T>(void)
{
    if (_coords_must_be_freed) {
        free(_coords);
        _coords_must_be_freed = 0;
        _coords = _internal_coords;
    }
}

/*f c_vector<T>::copy
 */
template <typename T>
c_vector<T> *c_vector<T>::copy(void) const
{
    return new c_vector<T>(*this);
}

/*f c_vector<T>::__str__
 */
template <typename T>
void c_vector<T>::__str__(char *buffer, int buf_size) const
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

/*f c_vector<T>::assign( vector )
 */
template <typename T>
c_vector<T> &c_vector<T>::assign(const c_vector<T> &other)
{
    set_length(other._length, 0); // Don't reallocate vector data
    for (int i=0; i<_length; i++) {
        COORD(i) = OCOORD(other,i);
    }
    return *this;
}

/*f c_vector<T>::assign (length, const T *coords)
 */
template <typename T>
c_vector<T> &c_vector<T>::assign(int length, int stride, const T *coords)
{
    set_length(length, 0); // Don't reallocate vector data
    for (int i=0; i<length; i++) {
        COORD(i) = coords[i*stride];
    }
    return *this;
}

/*a Interrogation methods */
/*f c_vector<T>::modulus_squared
 */
template <typename T>
T c_vector<T>::modulus_squared(void) const
{
    T l=0;
    for (int i=0; i<_length; i++) {
        l += COORD(i)*COORD(i);
    }
    return l;
}

/*f c_vector<T>::modulus
 */
template <typename T>
T c_vector<T>::modulus(void) const
{
    return sqrt(modulus_squared());
}

/*a In-place vector operations */
/*f c_vector<T>::add_scaled
 */
template <typename T>
c_vector<T> &c_vector<T>::add_scaled(const c_vector<T> &other, T scale)
{
    for (int i=0; i<_length; i++) {
        COORD(i) += OCOORD(other,i)*scale;
    }
    return *this;
}

/*f c_vector<T>::scale
 */
template <typename T>
c_vector<T> &c_vector<T>::scale(T scale)
{
    for (int i=0; i<_length; i++) {
        COORD(i) *= scale;
    }
    return *this;
}

/*f c_vector<T>::normalize
 */
template <typename T>
c_vector<T> &c_vector<T>::normalize(void)
{
    T l = this->modulus();
    if ((l>-EPSILON) && (l<EPSILON))
        return this->scale(0.0);
    return this->scale(1.0/l);
}

/*a Vector operations */
/*f c_vector<T>::dot_product
 */
template <typename T>
T c_vector<T>::dot_product(const c_vector<T> &other) const
{
    T l=0;
    for (int i=0; i<_length; i++) {
        l += COORD(i) * OCOORD(other,i);
    }
    return l;
}

/*f c_vector<T>::cross_product3
 */
template <typename T>
c_vector<T> *c_vector<T>::cross_product3(const c_vector<T> &other, c_vector<T> *result) const
{
    OCOORD(*result,0) = COORD(1)*OCOORD(other,2) - COORD(2)*OCOORD(other,1);
    OCOORD(*result,1) = COORD(2)*OCOORD(other,0) - COORD(0)*OCOORD(other,2);
    OCOORD(*result,2) = COORD(0)*OCOORD(other,1) - COORD(1)*OCOORD(other,0);
    return result;
}

/*f c_vector<T>::cross_product3
 */
template <typename T>
c_vector<T> *c_vector<T>::cross_product3(const c_vector<T> &other) const
{
    c_vector<T> *result=new c_vector<T>(_length);
    return cross_product3(other, result);
}

/*f c_vector<T>::angle_axis_to_v3
 */
template <typename T>
c_vector<T> *c_vector<T>::angle_axis_to_v3(const c_vector<T> &other, T *cos_angle, T *sin_angle, c_vector<T> *result) const
{
    T tl, ol;
    tl = this->modulus();
    ol = other.modulus();

    //fprintf(stderr,"tl,ol: %lf, %lf\n",tl, ol);
    c_vector<T> *axis=new c_vector<T>(3);
    cross_product3(other, axis);
    *axis /= (tl*ol);
    //fprintf(stderr,"axb:%lf,%lf,%lf\n",axis->_coords[0],axis->_coords[1],axis->_coords[2]);
    *cos_angle = dot_product(other) / (tl*ol);
    *sin_angle = axis->modulus();
    //fprintf(stderr,"angles:%lf,%lf\n",*cos_angle,*sin_angle);
    axis->normalize();
    //fprintf(stderr,"axis:%lf,%lf,%lf\n",axis->_coords[0],axis->_coords[1],axis->_coords[2]);
    return axis;
}

/*f c_vector<T>::angle_axis_to_v3
 */
template <typename T>
c_vector<T> *c_vector<T>::angle_axis_to_v3(const c_vector<T> &other, T *cos_angle, T *sin_angle) const
{
    T tl, ol;
    tl = this->modulus();
    ol = other.modulus();

    //fprintf(stderr,"tl,ol: %lf, %lf\n",tl, ol);
    c_vector<T> *axis = cross_product3(other);
    *axis /= (tl*ol);
    //fprintf(stderr,"axb:%lf,%lf,%lf\n",axis->_coords[0],axis->_coords[1],axis->_coords[2]);
    *cos_angle = dot_product(other) / (tl*ol);
    *sin_angle = axis->modulus();
    //fprintf(stderr,"angles:%lf,%lf\n",*cos_angle,*sin_angle);
    axis->normalize();
    //fprintf(stderr,"axis:%lf,%lf,%lf\n",axis->_coords[0],axis->_coords[1],axis->_coords[2]);
    return axis;
}

/*f c_vector<T>::angle_axis_to_v3 (to quaternion)
 */
template <typename T>
c_quaternion<T> *c_vector<T>::angle_axis_to_v3(const c_vector<T> &other) const
{
    c_quaternion<T> *r;
    T cos_angle, sin_angle;
    c_vector<T> *axis = this->angle_axis_to_v3(other, &cos_angle, &sin_angle);
    r = new c_quaternion<T>();
    r->from_rotation(cos_angle, sin_angle, *axis);
    return r;
}

/*a Explicit instantiations of the template
 */
template class c_vector<double>;
template class c_vector<float>;
