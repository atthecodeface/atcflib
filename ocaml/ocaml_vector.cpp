#define CAML_NAME_SPACE 
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/intext.h>
#include <caml/threads.h>

#include <atcf/vector.h>

#include <stdio.h>

//#define __OCAML_VECTOR_VERBOSE
#ifdef __OCAML_VECTOR_VERBOSE
#define VERBOSE fprintf
#else
#define VERBOSE (void)
#endif

#define vector_of_val(v) (*((c_vector **) Data_custom_val(v)))

static struct custom_operations custom_ops = {
    (char *)"atcf.vector",
    custom_finalize_default,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

extern "C"
CAMLprim value
atcf_vector_create(value n)
{
    CAMLparam1(n);
    VERBOSE(stderr,"Create vector %ld\n",Long_val(n));
    c_vector *cv = new c_vector(Long_val(n));
    value v = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    vector_of_val(v) = cv;
    VERBOSE(stderr,"Created vector %p\n", cv);
    CAMLreturn(v);
}

extern "C"
CAMLprim value
atcf_vector_create2(value c0, value c1)
{
    CAMLparam2(c0,c1);
    VERBOSE(stderr,"Create vector %f %f\n",Double_val(c0),Double_val(c1));
    c_vector *cv = new c_vector(2);
    cv->set(0,Double_val(c0));
    cv->set(1,Double_val(c1));
    value v = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    vector_of_val(v) = cv;
    VERBOSE(stderr,"Created vector %p\n", cv);
    CAMLreturn(v);
}

extern "C"
CAMLprim value
atcf_vector_create3(value c0, value c1, value c2)
{
    CAMLparam3(c0,c1,c2);
    VERBOSE(stderr,"Create vector %f %f %f\n",Double_val(c0),Double_val(c1),Double_val(c2));
    c_vector *cv = new c_vector(3);
    cv->set(0,Double_val(c0));
    cv->set(1,Double_val(c1));
    cv->set(2,Double_val(c2));
    value v = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    vector_of_val(v) = cv;
    VERBOSE(stderr,"Created vector %p\n", cv);
    CAMLreturn(v);
}

extern "C"
CAMLprim value
atcf_vector_create4(value c0, value c1, value c2, value c3)
{
    CAMLparam4(c0,c1,c2,c3);
    VERBOSE(stderr,"Create vector %f %f %f %f\n",Double_val(c0),Double_val(c1),Double_val(c2),Double_val(c3));
    c_vector *cv = new c_vector(4);
    cv->set(0,Double_val(c0));
    cv->set(1,Double_val(c1));
    cv->set(2,Double_val(c2));
    cv->set(3,Double_val(c3));
    value v = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    vector_of_val(v) = cv;
    VERBOSE(stderr,"Created vector %p\n", cv);
    CAMLreturn(v);
}

extern "C"
CAMLprim void
atcf_vector_destroy(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Destroy vector %p\n", vector_of_val(v));
    delete vector_of_val(v);
    vector_of_val(v) = NULL;
    CAMLreturn0;
}

extern "C"
CAMLprim value
atcf_vector_clone(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Clone vector %p\n", vector_of_val(v));
    c_vector *cv = new c_vector(*vector_of_val(v));
    v = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    vector_of_val(v) = cv;
    VERBOSE(stderr,"Created vector %p\n", cv);
    CAMLreturn(v);
}

extern "C"
CAMLprim value
atcf_vector_coords(value v)
{
    CAMLparam1(v);
    c_vector *cv = vector_of_val(v);
    int n = cv->length();
    v = caml_alloc_float_array(n);
    for (int i=0; i<n; i++) {
        Store_double_field(v,i,cv->value(i));
    }
    CAMLreturn(v);
}

#define FN_CV_TO_FLOAT(fn) \
    extern "C" CAMLprim value atcf_vector_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of vector %p\n", vector_of_val(v));     \
        CAMLreturn(caml_copy_double(vector_of_val(v)->fn()));        \
    }
#define FN_CV_TO_INT(fn) \
    extern "C" CAMLprim value atcf_vector_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of vector %p\n", vector_of_val(v));     \
        CAMLreturn(Val_long(vector_of_val(v)->fn()));   \
    }
#define FN_CV_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v) {         \
        CAMLparam1(v);                                               \
        VERBOSE(stderr,#fn " of vector %p\n", vector_of_val(v));     \
        vector_of_val(v)->fn();                                      \
        CAMLreturn0;                                                 \
    }
#define FN_CV_CVR_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v, value v2) {  \
        CAMLparam2(v, v2);                                              \
        VERBOSE(stderr,#fn " of vector %p %p\n", vector_of_val(v), vector_of_val(v2));     \
        vector_of_val(v)->fn(*vector_of_val(v2));                        \
        CAMLreturn0;                                                    \
    }
#define FN_CV_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v, value f) {   \
        CAMLparam2(v,f);                                                \
        VERBOSE(stderr,#fn " of vector %p %f\n", vector_of_val(v), Double_val(f)); \
        vector_of_val(v)->fn(Double_val(f));                            \
        CAMLreturn0;                                                    \
    }
#define FN_CV_INT_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v, value n, value f) { \
        CAMLparam3(v,n,f);                                              \
        VERBOSE(stderr,#fn " of vector %p %ld %f\n", vector_of_val(v), Long_val(n), Double_val(f)); \
        vector_of_val(v)->fn(Long_val(n), Double_val(f));                \
        CAMLreturn0;                                                    \
    }
#define FN_CV_CVR_FLOAT_TO_UNIT(fn) \
    extern "C" CAMLprim void atcf_vector_ ## fn (value v, value v2, value f) { \
        CAMLparam3(v,v2,f);                                             \
        VERBOSE(stderr,#fn " of vector %p %p %f\n", vector_of_val(v),  vector_of_val(v2), Double_val(f)); \
        vector_of_val(v)->fn(*vector_of_val(v2), Double_val(f));        \
        CAMLreturn0;                                                    \
    }

#define FN_CV_CVR_TO_FLOAT(fn) \
    extern "C" CAMLprim value atcf_vector_ ## fn (value v, value v2) { \
        CAMLparam2(v,v2);                                             \
        VERBOSE(stderr,#fn " of vector %p %p\n", vector_of_val(v),  vector_of_val(v2)); \
        CAMLreturn(caml_copy_double(vector_of_val(v)->fn(*vector_of_val(v2)))); \
    }
#define FN_CV_CVR_TO_CV(fn) \
    extern "C" CAMLprim value atcf_vector_ ## fn (value v, value v2) { \
        CAMLparam2(v,v2);                                             \
        value vr = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1); \
        vector_of_val(vr) = vector_of_val(v)->fn(*vector_of_val(v2));   \
        VERBOSE(stderr,"Created vector due to cross product %p\n", vector_of_val(vr));       \
        CAMLreturn(vr);                                                 \
    }


FN_CV_TO_INT(length)
FN_CV_INT_FLOAT_TO_UNIT(set)
FN_CV_CVR_TO_UNIT(assign)
FN_CV_FLOAT_TO_UNIT(scale)
FN_CV_TO_FLOAT(modulus)
FN_CV_TO_FLOAT(modulus_squared)
FN_CV_CVR_FLOAT_TO_UNIT(add_scaled)
FN_CV_TO_UNIT(normalize)
FN_CV_CVR_TO_FLOAT(dot_product)
FN_CV_CVR_TO_CV(cross_product)

extern "C"
CAMLprim value 
atcf_vector_angle_axis_to(value v, value v2)
{
    CAMLparam2(v,v2);
    value vr = caml_alloc_tuple(3);
    value vrv = caml_alloc_custom(&custom_ops, sizeof(c_vector *), 0, 1);
    Store_field(vr,0,vrv);
    double cos, sin;
    c_vector *cv = vector_of_val(v)->angle_axis_to_v(*vector_of_val(v2), &cos, &sin);
    vector_of_val(vrv) = cv;
    Store_field(vr,1,caml_copy_double(cos));
    Store_field(vr,2,caml_copy_double(sin));
    VERBOSE(stderr,"Created vector %p\n", vector_of_val(vr));
    CAMLreturn(vr);
}

