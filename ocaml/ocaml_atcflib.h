extern value caml_atcf_alloc_vector(class c_vector<double> *cv);
extern value caml_atcf_alloc_matrix(class c_matrix<double> *cm);
extern value caml_atcf_alloc_quaternion(class c_quaternion<double> *cq);

/*f matrix_of_val c_matrix from an OCAML value
 */
#define matrix_of_val(v) (*((c_matrix<double> **) Data_custom_val(v)))

/*f vector_of_val c_vector from an OCAML value
 */
#define vector_of_val(v) (*((c_vector<double> **) Data_custom_val(v)))

/*f quaternion_of_val c_quaternion from an OCAML value
 */
#define quaternion_of_val(v) (*((c_quaternion<double> **) Data_custom_val(v)))

/*f 't'_of_val c_t from an OCAML value
 */
#define cobj_of_val(t, v) (*((c_ ## t <double> **) Data_custom_val(v)))

/*a Defines for function wrappers
 */
/*f FN_C_TO_FLOAT : ct -> float
 *
 * Function to return ct->fn() as a float
 *
 */
#define FN_C_TO_FLOAT(t,fn)                                            \
    extern "C" CAMLprim value atcf_ ## t ## _ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of " #t " %p\n", cobj_of_val(t,v));     \
        CAMLreturn(caml_copy_double(cobj_of_val(t,v)->fn()));        \
    }

/*f FN_C_TO_INT : ct -> int
 *
 * Function to return ct->fn() as an int
 *
 */
#define FN_C_TO_INT(t, fn)                                               \
    extern "C" CAMLprim value atcf_ ## t ## _ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of " #t " %p\n", cobj_of_val(t,v));     \
        CAMLreturn(Val_long(cobj_of_val(t,v)->fn()));   \
    }

/*f FN_C_TO_UNIT : ct -> unit
 *
 * Function to perform ct->fn() with no return value
 *
 */
#define FN_C_TO_UNIT(t, fn)                                             \
    extern "C" CAMLprim void atcf_ ## t ## _ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of " #t " %p\n", cobj_of_val(t,v));        \
        cobj_of_val(t,v)->fn();                                      \
        CAMLreturn0;                                                 \
    }

/*f FN_C_CR_TO_UNIT : ct -> ct -> unit
 *
 * Function to perform ct->fn(*ct) with no return value
 *
 */
#define FN_C_CR_TO_UNIT(t, fn)                                           \
    extern "C" CAMLprim void atcf_ ## t ## _ ## fn (value v, value v2) {  \
        CAMLparam2(v, v2);                                              \
        VERBOSE(stderr,#fn " of " #t " %p %p\n", cobj_of_val(t,v), cobj_of_val(t,v2));     \
        cobj_of_val(t,v)->fn(*cobj_of_val(t,v2));                        \
        CAMLreturn0;                                                    \
    }

/*f FN_C_FLOAT_TO_UNIT : ct -> float -> unit
 *
 * Function to perform ct->fn(double) with no return value
 *
 */
#define FN_C_FLOAT_TO_UNIT(t, fn)                                        \
    extern "C" CAMLprim void atcf_ ## t ## _ ## fn (value v, value f) {   \
        CAMLparam2(v,f);                                                \
        VERBOSE(stderr,#fn " of " #t " %p %f\n", cobj_of_val(t,v), Double_val(f)); \
        cobj_of_val(t,v)->fn(Double_val(f));                            \
        CAMLreturn0;                                                    \
    }

/*f FN_C_INT_FLOAT_TO_UNIT : ct -> int -> float -> unit
 *
 * Function to perform ct->fn(int,double) with no return value
 *
 */
#define FN_C_INT_FLOAT_TO_UNIT(t, fn)                                    \
    extern "C" CAMLprim void atcf_ ## t ## _ ## fn (value v, value n, value f) { \
        CAMLparam3(v,n,f);                                              \
        VERBOSE(stderr,#fn " of " #t " %p %ld %f\n", cobj_of_val(t,v), Long_val(n), Double_val(f)); \
        cobj_of_val(t,v)->fn(Long_val(n), Double_val(f));                \
        CAMLreturn0;                                                    \
    }

/*f FN_C_INT_INT_FLOAT_TO_UNIT : ct -> int -> int -> float -> unit
 *
 * Function to perform ct->fn(int,int,double) with no return value
 *
 */
#define FN_C_INT_INT_FLOAT_TO_UNIT(t, fn)                                \
    extern "C" CAMLprim void atcf_ ## t ## _ ## fn (value v, value r, value c, value f) { \
        CAMLparam4(v,r,c,f);                                            \
        VERBOSE(stderr,#fn " of " #t " %p %ld %ld %f\n", cobj_of_val(t,v), Long_val(r), Long_val(c), Double_val(f)); \
        cobj_of_val(t,v)->fn(Long_val(r), Long_val(c), Double_val(f));                \
        CAMLreturn0;                                                    \
    }

/*f FN_C_CR_FLOAT_TO_UNIT : ct -> ct -> float -> unit
 *
 * Function to perform ct->fn(*ct,double) with no return value
 *
 */
#define FN_C_CR_FLOAT_TO_UNIT(t, fn)                                     \
    extern "C" CAMLprim void atcf_ ## t ## _ ## fn (value v, value v2, value f) { \
        CAMLparam3(v,v2,f);                                             \
        VERBOSE(stderr,#fn " of " #t " %p %p %f\n", cobj_of_val(t,v),  cobj_of_val(t,v2), Double_val(f)); \
        cobj_of_val(t,v)->fn(*cobj_of_val(t,v2), Double_val(f));        \
        CAMLreturn0;                                                    \
    }

/*f FN_C_CR_TO_FLOAT : ct -> ct -> float
 *
 * Function to perform ct->fn(*ct) returning a float
 *
 */
#define FN_C_CR_TO_FLOAT(t, fn)                                          \
    extern "C" CAMLprim value atcf_ ## t ## _ ## fn (value v, value v2) { \
        CAMLparam2(v,v2);                                             \
        VERBOSE(stderr,#fn " of " #t " %p %p\n", cobj_of_val(t,v),  cobj_of_val(t,v2)); \
        CAMLreturn(caml_copy_double(cobj_of_val(t,v)->fn(*cobj_of_val(t,v2)))); \
    }

/*f FN_C_CR_TO_C : ct -> ct -> ct
 *
 * Function to perform ct->fn(*ct) returning a NEW ct
 *
 */
#define FN_C_CR_TO_C(t, fn)                                              \
    extern "C" CAMLprim value atcf_ ## t ## _ ## fn (value v, value v2) {  \
        CAMLparam2(v,v2);                                               \
        CAMLreturn(caml_atcf_alloc_ ## t(cobj_of_val(t,v)->fn(*cobj_of_val(t,v2)))); \
    }

