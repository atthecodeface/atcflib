/*a Defines
 */
#define EPSILON (1E-10)
#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define WHERE ( __FILE__  STRINGIFY(__LINE__) )

/*a Test infrastructure
 */
static int failures=0;
static void
assert(int should_be_true, const char *where, const char *error_fmt, va_list ap)
{
    char buffer[256];
    if (!should_be_true) {
        vsnprintf(buffer, sizeof(buffer), error_fmt, ap);
        printf("Failure at %s: %s\n", where, buffer);
        failures++;
    }
}

static void
assert(int should_be_true, const char *where, const char *error_fmt, ...)
{
    va_list ap;
    va_start(ap, error_fmt);
    assert(should_be_true, where, error_fmt, ap);
    va_end(ap);
}

static void
assert_dbeq(double a, double b, const char *where, const char *error_fmt, ...)
{
    va_list ap;
    va_start(ap, error_fmt);
    double diff = fabs(a-b);
    assert(diff<EPSILON, where, error_fmt, ap);
    va_end(ap);
}

