extern value caml_atcf_alloc_vector(class c_vector *cv);
extern value caml_atcf_alloc_matrix(class c_matrix *cm);
/*f matrix_of_val c_matrix from an OCAML value
 */
#define matrix_of_val(v) (*((c_matrix **) Data_custom_val(v)))

/*f vector_of_val c_matrix from an OCAML value
 */
#define vector_of_val(v) (*((c_vector **) Data_custom_val(v)))

