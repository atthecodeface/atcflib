/*a Documentation
 */
/*a Includes
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "lens_projection.h"
#include "test.h"

/*a Support functions
 */
static void display(const char *msg, c_lens_projection &a)
{
    char buffer[256];
    a.__str__(buffer, sizeof(buffer));
    fprintf(stderr, "%s : %s\n",msg, buffer);
}

static void display(const char *msg, c_quaternion &a)
{
    char buffer[256];
    a.__str__(buffer, sizeof(buffer));
    fprintf(stderr, "%s : %s\n",msg, buffer);
}

/*a Basic tests
 */
static void
test_init(void)
{
    c_lens_projection lp;
    c_quaternion q;
    double l;

    assert_dbeq(lp.get_orientation().r(), 1, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().i(), 0, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().j(), 0, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().k(), 0, WHERE, "LP init");

    lp.orient(c_quaternion::rijk(2,3,4,5));
    l = sqrt(2*2+3*3+4*4+5*5);

    display("Init", lp);
    assert_dbeq(lp.get_orientation().r(), 2/l, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().i(), 3/l, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().j(), 4/l, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().k(), 5/l, WHERE, "LP init");

    lp.orient(c_quaternion::identity());
    assert_dbeq(lp.get_orientation().r(), 1, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().i(), 0, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().j(), 0, WHERE, "LP init");
    assert_dbeq(lp.get_orientation().k(), 0, WHERE, "LP init");

    static const double xy[2] = {0.0,0.0};
    q = lp.orientation_of_xy(xy);
    assert_dbeq(q.r(), 1, WHERE, "LP init");
    assert_dbeq(q.i(), 0, WHERE, "LP init");
    assert_dbeq(q.j(), 0, WHERE, "LP init");
    assert_dbeq(q.k(), 0, WHERE, "LP init");
}

static void
test_map_unmap(c_lens_projection &lp)
{
    c_quaternion q;
    double xy[2];
    double xy2[2];

    for (int i=0; i<100; i++) {
        int j=i;
        xy[0] = (j%10)/6.0-0.4; j/=10;
        xy[1] = (j%10)/6.0-0.4; j/=10;
        q = lp.orientation_of_xy(xy);
        lp.xy_of_orientation(&q, xy2);
        assert_dbeq(xy2[0], xy[0], WHERE, "XY to and from orientation");
        assert_dbeq(xy2[1], xy[1], WHERE, "XY to and from orientation");
    }
}

static void
test_map(void)
{
    c_lens_projection lp;
    display("Test map/unmap",lp);
    test_map_unmap(lp);

    lp.orient(c_quaternion::roll(30.0,1));
    display("Test map/unmap",lp);
    test_map_unmap(lp);

    lp.orient(c_quaternion::yaw(30.0,1));
    display("Test map/unmap",lp);
    test_map_unmap(lp);

    lp.orient(c_quaternion::pitch(30.0,1));
    display("Test map/unmap",lp);
    test_map_unmap(lp);

    lp.orient(c_quaternion::of_euler(10.0,20.0,30.0,1));
    display("Test map/unmap",lp);
    test_map_unmap(lp);
    
}

/*a Toplevel
 */
extern int main(int argc, char **argv)
{
    test_init();
    test_map();
    if (failures>0) {
        exit(4);
    }
}
