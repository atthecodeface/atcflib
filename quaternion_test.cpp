/*a Documentation
Ported from gjslib.math.quaternion python
    # +y to right, +x up, +z in front
    # roll + = roll clkwise inside looking forward
    # pitch + = nose up inside looking forward
    # yaw + = nose left inside looking forward
    # when given roll, pitch, yaw the order applies is roll(pitch(yaw())) - i.e. yaw is applied first
 */
/*a Includes
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "quaternion.h"
#include "test.h"

/*a Support functions
 */
static void display(const char *msg, c_quaternion &a)
{
    char buffer[256];
    a.__str__(buffer, sizeof(buffer));
    fprintf(stderr, "%s : %s\n",msg, buffer);
}

/*a Arithmetic tests
 */
/*f test_init
  tests initial values, constructors and accessors for simple values
 */
static void
test_init(void)
{
    c_quaternion a, b;
    assert( (a.modulus()==0), WHERE, "Initial quaternion should be 0,0,0,0");
    assert( (b.modulus()==0), WHERE, "Initial quaternion should be 0,0,0,0");

    a = c_quaternion(1.0,0.0,0.0,0.0);
    assert( (a.modulus()==1.0), WHERE, "Quaternion initialization to 1,0,0,0");
    assert( (a.r()==1.0), WHERE, "Quaternion access");
    assert( (a.i()==0.0), WHERE, "Quaternion access");
    assert( (a.j()==0.0), WHERE, "Quaternion access");
    assert( (a.k()==0.0), WHERE, "Quaternion access");

    a = c_quaternion(1.0,2.0,3.0,4.0);
    assert( (a.r()==1.0), WHERE, "Quaternion access");
    assert( (a.i()==2.0), WHERE, "Quaternion access");
    assert( (a.j()==3.0), WHERE, "Quaternion access");
    assert( (a.k()==4.0), WHERE, "Quaternion access");

    a = c_quaternion(4.0,3.0,2.0,1.0);
    assert( (a.r()==4.0), WHERE, "Quaternion access");
    assert( (a.i()==3.0), WHERE, "Quaternion access");
    assert( (a.j()==2.0), WHERE, "Quaternion access");
    assert( (a.k()==1.0), WHERE, "Quaternion access");

    a = c_quaternion::roll(0, 0);
    assert( (a.r()==1.0), WHERE, "Quaternion init roll");
    assert( (a.i()==0.0), WHERE, "Quaternion init roll");
    assert( (a.j()==0.0), WHERE, "Quaternion init roll");
    assert( (a.k()==0.0), WHERE, "Quaternion init roll");

    a = c_quaternion::roll(90, 1);
    assert_dbeq(a.r(), sqrt(0.5), WHERE, "Quaternion init roll");
    assert_dbeq(a.i(), 0,         WHERE, "Quaternion init roll");
    assert_dbeq(a.j(), 0,         WHERE, "Quaternion init roll");
    assert_dbeq(a.k(), sqrt(0.5), WHERE, "Quaternion init roll");

    a = c_quaternion::roll(45, 1);
    assert( (a.modulus()==1.0),   WHERE, "Quaternion init roll");
    assert_dbeq(a.i(), 0,         WHERE, "Quaternion init roll");
    assert_dbeq(a.j(), 0,         WHERE, "Quaternion init roll");

    a = c_quaternion::roll(45, 1) * c_quaternion::roll(-45, 1);
    assert( (a.r()==1.0), WHERE, "Quaternion access");
    assert_dbeq(a.i(), 0,         WHERE, "Quaternion init roll");
    assert_dbeq(a.j(), 0,         WHERE, "Quaternion init roll");
    assert_dbeq(a.k(), 0,         WHERE, "Quaternion init roll");

    a = c_quaternion::pitch(90, 1);
    assert_dbeq(a.r(), sqrt(0.5), WHERE, "Quaternion init pitch");
    assert_dbeq(a.i(), 0,         WHERE, "Quaternion init pitch");
    assert_dbeq(a.j(), sqrt(0.5), WHERE, "Quaternion init pitch");
    assert_dbeq(a.k(), 0,         WHERE, "Quaternion init pitch");

    a = c_quaternion::pitch(45, 1);
    assert( (a.modulus()==1.0),   WHERE, "Quaternion init pitch");
    assert_dbeq(a.i(), 0,         WHERE, "Quaternion init pitch");
    assert_dbeq(a.k(), 0,         WHERE, "Quaternion init pitch");

    a = c_quaternion::pitch(45, 1) * c_quaternion::pitch(-45, 1);
    assert( (a.r()==1.0), WHERE, "Quaternion access");
    assert_dbeq(a.i(), 0,         WHERE, "Quaternion init pitch");
    assert_dbeq(a.j(), 0,         WHERE, "Quaternion init pitch");
    assert_dbeq(a.k(), 0,         WHERE, "Quaternion init pitch");

    a = c_quaternion::yaw(90, 1);
    assert_dbeq(a.r(), sqrt(0.5), WHERE, "Quaternion init yaw");
    assert_dbeq(a.i(), sqrt(0.5), WHERE, "Quaternion init yaw");
    assert_dbeq(a.j(), 0,         WHERE, "Quaternion init yaw");
    assert_dbeq(a.k(), 0,         WHERE, "Quaternion init yaw");

    a = c_quaternion::yaw(45, 1);
    assert( (a.modulus()==1.0),   WHERE, "Quaternion init yaw");
    assert_dbeq(a.j(), 0,         WHERE, "Quaternion init yaw");
    assert_dbeq(a.k(), 0,         WHERE, "Quaternion init yaw");

    a = c_quaternion::yaw(45, 1) * c_quaternion::yaw(-45, 1);
    assert( (a.r()==1.0), WHERE, "Quaternion access");
    assert_dbeq(a.i(), 0,         WHERE, "Quaternion init yaw");
    assert_dbeq(a.j(), 0,         WHERE, "Quaternion init yaw");
    assert_dbeq(a.k(), 0,         WHERE, "Quaternion init yaw");

    a = c_quaternion::rijk(6,5,4,3);
    assert_dbeq(a.r(), 6,         WHERE, "Quaternion init rijk");
    assert_dbeq(a.i(), 5,         WHERE, "Quaternion init rijk");
    assert_dbeq(a.j(), 4,         WHERE, "Quaternion init rijk");
    assert_dbeq(a.k(), 3,         WHERE, "Quaternion init rijk");

    a = c_quaternion::identity();
    assert_dbeq(a.r(), 1,         WHERE, "Quaternion init rijk");
    assert_dbeq(a.i(), 0,         WHERE, "Quaternion init rijk");
    assert_dbeq(a.j(), 0,         WHERE, "Quaternion init rijk");
    assert_dbeq(a.k(), 0,         WHERE, "Quaternion init rijk");
}

/*f test_reals
  tests =, +, -, *, /, +=, -=, *=, /= for reals
 */
static void
test_reals(void)
{
    c_quaternion a, b, c;
    for (int i=0; i<25; i++) {
        int j=i;
        int a_r = (i%5)-2; j/=5;
        int b_r = (i%5)-2; j/=5;

        a = (double)a_r;
        b = (double)b_r;
        assert_dbeq(a.modulus(), abs(a_r), WHERE, "Real initialization");
        assert_dbeq(b.modulus(), abs(b_r), WHERE, "Real initialization");

        b = a;
        b -= (double)b_r;
        assert_dbeq(b.modulus(), abs(a_r-b_r), WHERE, "Real initialization");

        b = a;
        b += (double)b_r;
        assert_dbeq(b.modulus(), abs(a_r+b_r), WHERE, "Real initialization");

        b = a;
        b *= (double)b_r;
        assert_dbeq(b.modulus(), abs(a_r*b_r), WHERE, "Real initialization");

        b = a;
        if (b_r!=0) {
            b /= (double)b_r;
            assert_dbeq(b.modulus(), abs(a_r/b_r), WHERE, "Real initialization");
        }
        if (a_r!=0) {
            b = ((( a * a_r + a_r ) / a_r) - a_r ) * a_r;
            assert_dbeq(b.modulus(), a.modulus(), WHERE, "Real initialization");
        }
        b = a + 1;
        assert_dbeq(b.r(), 1+a.r(), WHERE, "Real initialization");
    }
}


/*f test_arithmetic
  tests infix +, -, *, /, and +=, -=, *=, /=
 */
static void
test_arithmetic(void)
{
    c_quaternion a, b, c;
    for (int i=0; i<81*81; i++) {
        double am, bm;

        int j=i;
        int a_r = j%3; j/=3;
        int a_i = j%3; j/=3;
        int a_j = j%3; j/=3;
        int a_k = j%3; j/=3;
        int b_r = j%3; j/=3;
        int b_i = j%3; j/=3;
        int b_j = j%3; j/=3;
        int b_k = j%3; j/=3;

        a = c_quaternion((double)a_r, (double)a_i, (double)a_j, (double)a_k);
        b = c_quaternion((double)b_r, (double)b_i, (double)b_j, (double)b_k);
        c = a + b;
        b += a;
        c = c - b;
        assert( (a.r()==a_r), WHERE, "Basic add");
        assert( (a.i()==a_i), WHERE, "Basic add");
        assert( (a.j()==a_j), WHERE, "Basic add");
        assert( (a.k()==a_k), WHERE, "Basic add");
        assert( (b.r()==b_r+a_r), WHERE, "Basic add");
        assert( (b.i()==b_i+a_i), WHERE, "Basic add");
        assert( (b.j()==b_j+a_j), WHERE, "Basic add");
        assert( (b.k()==b_k+a_k), WHERE, "Basic add");
        assert_dbeq( c.modulus(), 0, WHERE, "Basic add/subtract");

        a = c_quaternion((double)a_r, (double)a_i, (double)a_j, (double)a_k);
        b = c_quaternion((double)b_r, (double)b_i, (double)b_j, (double)b_k);
        c = a - b;
        b -= a;
        c = b + c;
        assert( (a.r()==a_r), WHERE, "Basic sub");
        assert( (a.i()==a_i), WHERE, "Basic sub");
        assert( (a.j()==a_j), WHERE, "Basic sub");
        assert( (a.k()==a_k), WHERE, "Basic sub");
        assert( (b.r()==b_r-a_r), WHERE, "Basic sub");
        assert( (b.i()==b_i-a_i), WHERE, "Basic sub");
        assert( (b.j()==b_j-a_j), WHERE, "Basic sub");
        assert( (b.k()==b_k-a_k), WHERE, "Basic sub");
        assert_dbeq( c.modulus(), 0, WHERE, "Basic add/subtract");

        a = c_quaternion((double)a_r, (double)a_i, (double)a_j, (double)a_k);
        b = c_quaternion((double)b_r, (double)b_i, (double)b_j, (double)b_k);
        am = a.modulus();
        bm = b.modulus();
        c = a * b;
        b *= a;
        if ((bm>0.001) && (am>0.001)) {
            c = c / b;
        }
        assert( (a.r()==a_r), WHERE, "Basic mult");
        assert( (a.i()==a_i), WHERE, "Basic mult");
        assert( (a.j()==a_j), WHERE, "Basic mult");
        assert( (a.k()==a_k), WHERE, "Basic mult");
        assert_dbeq( b.modulus(), am*bm, WHERE, "Basic mult %d.%d.%d.%d %d.%d.%d.%d %lf %lf %lf",
                     a_r, a_i, a_j, a_k,
                     b_r, b_i, b_j, b_k,
                     am, bm, b.modulus() );
        if ((bm>0.001) && (am>0.001)) {
            assert_dbeq( c.modulus(), 1.0, WHERE, "Basic mult/div %d.%d.%d.%d %d.%d.%d.%d %lf %lf %lf %lf.%lf.%lf.%lf",
                         a_r, a_i, a_j, a_k,
                         b_r, b_i, b_j, b_k,
                         am, bm, c.modulus(),
                         c.r(), c.i(), c.j(), c.k());
        } else {
            assert_dbeq( c.modulus(), 0.0, WHERE, "Basic mult/div");
        }

        a = c_quaternion((double)a_r, (double)a_i, (double)a_j, (double)a_k);
        b = c_quaternion((double)b_r, (double)b_i, (double)b_j, (double)b_k);
        am = a.modulus();
        bm = b.modulus();
        if (am>0.00001) {
            c = b / a;
            b /= a;
            if (bm>0.001) {
                c = c / b;
            }
            assert( (a.r()==a_r), WHERE, "Basic divide");
            assert( (a.i()==a_i), WHERE, "Basic divide");
            assert( (a.j()==a_j), WHERE, "Basic divide");
            assert( (a.k()==a_k), WHERE, "Basic divide");
            assert_dbeq( b.modulus(), bm/am, WHERE, "Basic divide %d.%d.%d.%d %d.%d.%d.%d %lf %lf %lf",
                         a_r, a_i, a_j, a_k,
                         b_r, b_i, b_j, b_k,
                         am, bm, b.modulus() );
            if (bm>0.001) {
                assert_dbeq( c.modulus(), 1.0, WHERE, "Basic divide/mult");
            }
        }
    }

}

/*f test_scaling
  tests scale, normalize, modulus, modulus_squared
 */
static void
test_scaling(void)
{
    c_quaternion a, b, c;
    for (int i=0; i<81*81; i++) {
        double am;

        int j=i;
        int a_r = j%3; j/=3;
        int a_i = j%3; j/=3;
        int a_j = j%3; j/=3;
        int a_k = j%3; j/=3;
        int b_r = j%3; j/=3;
        int b_i = j%3; j/=3;
        int b_j = j%3; j/=3;
        int b_k = j%3; j/=3;

        a = c_quaternion((double)a_r, (double)a_i, (double)a_j, (double)a_k);
        b = c_quaternion((double)b_r, (double)b_i, (double)b_j, (double)b_k);

        am = a.modulus_squared();
        a.scale(2.0);
        assert_dbeq( a.modulus_squared(), am*4, WHERE, "Basic scale");
        a /= 2.0;
        assert_dbeq( a.modulus_squared(), am, WHERE, "Basic scale");

        a *= 2.0;
        assert_dbeq( a.modulus_squared(), am*4, WHERE, "Basic scale");
    }

}

static void
test_euler(void)
{
    c_quaternion a, b;
    a.from_euler(0,0,0,0);
    assert_dbeq( a.modulus(), 1, WHERE, "Modulus of from_euler should be 1");
}

/*a Toplevel
 */
extern int main(int argc, char **argv)
{
    test_init();
    test_reals();
    test_arithmetic();
    test_scaling();
    test_euler();
    if (failures>0) {
        exit(4);
    }
}
