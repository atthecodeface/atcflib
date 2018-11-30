template <typename T> class c_vector;
template <typename T> class c_quaternion;
template <typename T> class c_matrix;
//extern value caml_atcf_alloc_vector(class c_vector<double> *cv);
//extern value caml_atcf_alloc_matrix(class c_matrix<double> *cm);
extern value caml_atcf_alloc_quaternion(class c_quaternion<double> *cq);

/*t t_math_type - doubles MUST be odd */
typedef enum {
    MT_V_FLOAT=0,
    MT_V_DOUBLE=1,
    MT_M_FLOAT=2,
    MT_M_DOUBLE=3,
} t_math_type;

/*t t_math_obj - Object stored in Ocaml value */
typedef struct {
    t_math_type mt;
    value ba;
    int   ofs;
    union {
        void *v;
        c_vector<double> *vd;
        c_vector<float>  *vf;
        c_matrix<double> *md;
        c_matrix<float>  *mf;
        c_quaternion<double> *qd;
        c_quaternion<float>  *qf;
        const c_vector<double> *cvd;
        const c_vector<float>  *cvf;
        const c_matrix<double> *cmd;
        const c_matrix<float>  *cmf;
        const c_quaternion<double> *cqd;
        const c_quaternion<float>  *cqf;
    } ptr;
} t_math_obj;

/*f caml_atcf_alloc_math_obj */
extern void
caml_atcf_alloc_math_obj(value *v, t_math_type mt, value *ba, int ofs, void *ptr);
extern int
math_obj_validate(t_math_obj *m, t_math_type mt, ...);
extern int math_obj_validate_is_m(t_math_obj *m);
extern int math_obj_validate_is_v(t_math_obj *m);

/*f math_obj_of_val c_vector from an OCAML value
 */
#define math_obj_of_double(m) ((m->mt)&1)
#define math_obj_of_val(v) (*((t_math_obj **) Data_custom_val(v)))

/*f quaternion_of_val c_quaternion from an OCAML value
 */
#define quaternion_of_val(v) (*((c_quaternion<double> **) Data_custom_val(v)))

/*f lens_projection_of_val c_lens_projection from an OCAML value
 */
#define lens_projection_of_val(v) (*((c_lens_projection **) Data_custom_val(v)))

/*f bunzip_of_val c_bunzip from an OCAML value
 */
#define bunzip_of_val(v) (*((c_bunzip **) Data_custom_val(v)))

/*f 't'_of_val c_t from an OCAML value
 */
#define cobj_of_val(t, v) (*((c_ ## t <double> **) Data_custom_val(v)))

/*a Defines for function wrappers
 */
/*f FN_MO_C_TO_FLOAT : ct -> float
 *
 * Function to return ct->fn() as a float
 *
 */
#define FN_MO_C_TO_FLOAT(st, fn)                                        \
    extern "C" CAMLprim value atcf_ ## st ## _ ## fn (value v) { \
        CAMLparam1(v);                      \
        CAMLlocal1(result);                 \
        t_math_obj *m = math_obj_of_val(v); \
        VERBOSE(stderr,#fn " of " #st " %p\n", m);     \
        if (math_obj_of_double(m)) { result = caml_copy_double(m->ptr.st ## d->fn()); } \
        else  { result = caml_copy_double(m->ptr.st ## f->fn());}       \
        CAMLreturn(result);   \
    }

/*f FN_MO_C_TO_INT : ct -> int
 *
 * Function to return ct->fn() as an int
 *
 */
#define FN_MO_C_TO_INT(st, fn)                                        \
    extern "C" CAMLprim value atcf_ ## st ## _ ## fn (value v) { \
        CAMLparam1(v);                      \
        CAMLlocal1(result);                 \
        t_math_obj *m = math_obj_of_val(v); \
        VERBOSE(stderr,#fn " of math obj " #st " %p ba data %p\n", \
                m, Caml_ba_data_val(m->ba));                            \
        math_obj_validate_is_ ## st (m); \
        if (math_obj_of_double(m)) { result = Val_long(m->ptr.st ## d->fn());} \
        else  { result = Val_long(m->ptr.st ## f->fn());}                      \
        CAMLreturn(result);   \
    }

/*f FN_MO_C_TO_UNIT : ct -> unit
 *
 * Function to invoke ct->fn()
 *
 */
#define FN_MO_C_TO_UNIT(st, fn)                                        \
    extern "C" CAMLprim void atcf_ ## st ## _ ## fn (value v) { \
        CAMLparam1(v);                      \
        t_math_obj *m = math_obj_of_val(v); \
        VERBOSE(stderr,#fn " of math obj " #st " %p ba data %p\n", \
                m, Caml_ba_data_val(m->ba));                            \
        math_obj_validate_is_ ## st (m); \
        if (math_obj_of_double(m)) { m->ptr.st ## d->fn();} \
        else  { m->ptr.st ## f->fn();}                      \
        CAMLreturn0;   \
    }

/*f FN_MO_C_INT_TO_FLOAT : ct -> int -> float
 *
 * Function to perform ct->fn(int) returning a float
 *
 */
#define FN_MO_C_INT_TO_FLOAT(st, fn)                                          \
    extern "C" CAMLprim value atcf_ ## st ## _ ## fn (value v, value v2) { \
        CAMLparam2(v,v2);                                             \
        CAMLlocal1(result);                 \
        t_math_obj *m = math_obj_of_val(v); \
        VERBOSE(stderr,#fn " of math obj " #st " %p ba data %p and %ld\n", \
                m, Caml_ba_data_val(m->ba),                             \
                Long_val(v2));                                          \
        math_obj_validate_is_ ## st (m); \
        if (math_obj_of_double(m)) { result = caml_copy_double(m->ptr.st ## d->fn(Long_val(v2))); } \
        else  { result = caml_copy_double(m->ptr.st ## f->fn(Long_val(v2)));}       \
        CAMLreturn(result); \
    }

/*f FN_MO_C_CR_TO_UNIT : ct -> ct -> unit
 *
 * Function to perform ct->fn(*ct) with no return value
 *
 */
#define FN_MO_C_CR_TO_UNIT(st, fn)                                           \
    extern "C" CAMLprim void atcf_ ## st ## _ ## fn (value v, value v2) {  \
        CAMLparam2(v, v2);                                              \
        t_math_obj *m = math_obj_of_val(v); \
        t_math_obj *m2 = math_obj_of_val(v2); \
        VERBOSE(stderr,#fn " of math objs " #st " %p ba data %p and %p ba data %p\n", \
                m, Caml_ba_data_val(m->ba),                             \
                m2, Caml_ba_data_val(m2->ba));                          \
        math_obj_validate_is_ ## st (m); \
        math_obj_validate_is_ ## st (m2); \
        if (math_obj_of_double(m)) { m->ptr.st ## d->fn(*m2->ptr.st ## d); } \
        else  { m->ptr.st ## f->fn(*m2->ptr.st ## f); }       \
        CAMLreturn0;                                                    \
    }

/*f FN_MO_C_CR_CR_TO_UNIT : ct -> ct -> ct -> unit
 *
 * Function to perform ct->fn(*ct) with no return value
 *
 */
#define FN_MO_C_CR_CR_TO_UNIT(st, fn)                                           \
    extern "C" CAMLprim void atcf_ ## st ## _ ## fn (value v, value v2, value v3) { \
        CAMLparam3(v, v2, v3);                                             \
        t_math_obj *m = math_obj_of_val(v); \
        t_math_obj *m2 = math_obj_of_val(v2); \
        t_math_obj *m3 = math_obj_of_val(v3); \
        VERBOSE(stderr,#fn " of math objs " #st " %p ba data %p and %p ba data %p and %p ba data %p\n", \
                m, Caml_ba_data_val(m->ba), \
                m2, Caml_ba_data_val(m2->ba), \
                m3, Caml_ba_data_val(m3->ba)); \
        math_obj_validate_is_ ## st (m); \
        math_obj_validate_is_ ## st (m2); \
        math_obj_validate_is_ ## st (m3); \
        if (math_obj_of_double(m)) { m->ptr.st ## d->fn(*m2->ptr.st ## d, *m3->ptr.st ## d); } \
        else  { m->ptr.st ## f->fn(*m2->ptr.st ## f, *m3->ptr.st ## f ); }                \
        CAMLreturn0;                                                    \
    }

/*f FN_MO_C_INT_FLOAT_TO_UNIT : ct -> int -> float -> unit
 *
 * Function to perform ct->fn(int,double) with no return value
 *
 */
#define FN_MO_C_INT_FLOAT_TO_UNIT(st, fn)                                    \
    extern "C" CAMLprim void atcf_ ## st ## _ ## fn (value v, value n, value f) { \
        CAMLparam3(v,n,f);                                              \
        t_math_obj *m = math_obj_of_val(v); \
        VERBOSE(stderr,#fn " of math obj " #st " %p ba data %p and %ld, %f\n", \
                m, Caml_ba_data_val(m->ba),                             \
                Long_val(n),                                            \
                Double_val(f));                                         \
        math_obj_validate_is_ ## st (m); \
        if (math_obj_of_double(m)) { m->ptr.st ## d->fn(Long_val(n), Double_val(f)); } \
        else  { m->ptr.st ## f->fn(Long_val(n), Double_val(f));}        \
        CAMLreturn0;                                                    \
    }

/*f FN_MO_C_INT_INT_FLOAT_TO_UNIT : ct -> int -> int -> float -> unit
 *
 * Function to perform ct->fn(int,int,double) with no return value
 *
 */
#define FN_MO_C_INT_INT_FLOAT_TO_UNIT(st, fn)                                \
    extern "C" CAMLprim void atcf_ ## st ## _ ## fn (value v, value r, value c, value f) { \
        CAMLparam4(v,r,c,f);                                            \
        t_math_obj *m = math_obj_of_val(v); \
        VERBOSE(stderr,#fn " of math obj " #st " %p ba data %p and %ld, %ld, %f\n", \
                m, Caml_ba_data_val(m->ba),                             \
                Long_val(r),                                            \
                Long_val(c),                                            \
                Double_val(f));                                         \
        math_obj_validate_is_ ## st (m); \
        if (math_obj_of_double(m)) { m->ptr.st ## d->fn(Long_val(r), Long_val(c), Double_val(f)); } \
        else  { m->ptr.st ## f->fn(Long_val(r), Long_val(c), Double_val(f));} \
        CAMLreturn0;                                                    \
    }

/*f FN_MO_C_FLOAT_TO_UNIT : ct -> float -> unit
 *
 * Function to perform ct->fn(double) with no return value
 *
 */
#define FN_MO_C_FLOAT_TO_UNIT(st, fn)                                        \
    extern "C" CAMLprim void atcf_ ## st ## _ ## fn (value v, value f) {   \
        CAMLparam2(v,f);                                                \
        t_math_obj *m = math_obj_of_val(v);                            \
        VERBOSE(stderr,#fn " of math obj " #st " %p ba data %p and %f\n", \
                m, Caml_ba_data_val(m->ba),                             \
                Double_val(f));                                         \
        math_obj_validate_is_ ## st (m); \
        if (math_obj_of_double(m)) { m->ptr.st ## d ->fn(Double_val(f)); } \
        else  {m->ptr.st ## d ->fn(Double_val(f)); }  \
                                          \
        CAMLreturn0;                                                   \
    }

/*f FN_MO_C_CR_FLOAT_TO_UNIT : ct -> ct -> float -> unit
 *
 * Function to perform ct->fn(*ct,double) with no return value
 *
 */
#define FN_MO_C_CR_FLOAT_TO_UNIT(st, fn)                                     \
    extern "C" CAMLprim void atcf_ ## st ## _ ## fn (value v, value v2, value f) { \
        CAMLparam3(v,v2,f);                                             \
        t_math_obj *m = math_obj_of_val(v); \
        t_math_obj *m2 = math_obj_of_val(v2); \
        VERBOSE(stderr,#fn " of math objs " #st " %p ba data %p and %p ba data %p and %f\n", \
                m, Caml_ba_data_val(m->ba),                             \
                m2, Caml_ba_data_val(m2->ba),                           \
                Double_val(f));                                         \
        math_obj_validate_is_ ## st (m); \
        math_obj_validate_is_ ## st (m2); \
        if (math_obj_of_double(m)) { m->ptr.st ## d->fn(*m2->ptr.st ## d, Double_val(f)); } \
        else  { m->ptr.st ## f->fn(*m2->ptr.st ## f, Double_val(f)); }       \
        CAMLreturn0;                                                    \
    }

/*f FN_MO_C_CR_TO_FLOAT : ct -> ct -> float
 *
 * Function to perform ct->fn(*ct) returning a float
 *
 */
#define FN_MO_C_CR_TO_FLOAT(st, fn)                                          \
    extern "C" CAMLprim value atcf_ ## st ## _ ## fn (value v, value v2) { \
        CAMLparam2(v,v2);                                             \
        CAMLlocal1(result);                 \
        double result_f; \
        t_math_obj *m = math_obj_of_val(v); \
        t_math_obj *m2 = math_obj_of_val(v2); \
        VERBOSE(stderr,#fn " of math objs " #st " %p ba data %p and %p ba data %p\n", \
                m, Caml_ba_data_val(m->ba),                             \
                m2, Caml_ba_data_val(m2->ba));                          \
        math_obj_validate_is_ ## st (m); \
        math_obj_validate_is_ ## st (m2); \
        if (math_obj_of_double(m)) { result_f = m->ptr.st ## d->fn(*m2->ptr.st ## d); } \
        else  { result_f = m->ptr.st ## f->fn(*m2->ptr.st ## f); }       \
        result = caml_copy_double(result_f); \
        CAMLreturn(result); \
    }

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

/*f FN_C_INT_TO_FLOAT : ct -> int -> float
 *
 * Function to perform ct->fn(int) returning a float
 *
 */
#define FN_C_INT_TO_FLOAT(t, fn)                                          \
    extern "C" CAMLprim value atcf_ ## t ## _ ## fn (value v, value v2) { \
        CAMLparam2(v,v2);                                             \
        VERBOSE(stderr,#fn " of " #t " %p %ld\n", cobj_of_val(t,v),  Long_val(v2)); \
        CAMLreturn(caml_copy_double(cobj_of_val(t,v)->fn(Long_val(v2)))); \
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

