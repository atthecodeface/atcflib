(** ATCFLIB  module
 *)

(**/**)

(*a Types *)
type c_vector
type c_matrix
type c_quaternion

(*a Atcflib ocaml wrapper C functions *)
(*b vector functions *)
external v_create  : int -> c_vector   = "atcf_vector_create"
external v_clone   : c_vector -> c_vector   = "atcf_vector_clone"
external v_destroy : c_vector -> unit  = "atcf_vector_destroy"
external v_modulus : c_vector -> float = "atcf_vector_modulus"
external v_modulus_squared : c_vector -> float = "atcf_vector_modulus_squared"
external v_assign  : c_vector -> c_vector -> unit = "atcf_vector_assign"
external v_assign_m_v  : c_vector -> c_matrix -> c_vector -> unit = "atcf_vector_assign_m_v"
external v_normalize : c_vector -> unit = "atcf_vector_normalize"
external v_length  : c_vector -> int = "atcf_vector_length"
external v_coords  : c_vector -> float array  = "atcf_vector_coords"
external v_set     : c_vector -> int -> float -> unit  = "atcf_vector_set"
external v_scale   : c_vector -> float -> unit  = "atcf_vector_scale"
external v_add_scaled    : c_vector -> c_vector -> float -> unit  = "atcf_vector_add_scaled"
external v_dot_product   : c_vector -> c_vector -> float  = "atcf_vector_dot_product"
external v_cross_product : c_vector -> c_vector -> c_vector  = "atcf_vector_cross_product3"
external v_angle_axis_to : c_vector -> c_vector -> (c_vector * float * float)  = "atcf_vector_angle_axis_to3"

(*b matrix functions *)
external m_create  : int -> int -> c_matrix   = "atcf_matrix_create"
external m_clone   : c_matrix -> c_matrix   = "atcf_matrix_clone"
external m_destroy : c_matrix -> unit  = "atcf_matrix_destroy"
external m_nrows   : c_matrix -> int = "atcf_matrix_nrows"
external m_ncols   : c_matrix -> int = "atcf_matrix_ncols"
external m_row_vector  : c_matrix -> int -> c_vector  = "atcf_matrix_row_vector"
external m_col_vector  : c_matrix -> int -> c_vector  = "atcf_matrix_col_vector"
external m_apply   : c_matrix -> c_vector -> c_vector  = "atcf_matrix_apply"
external m_identity      : c_matrix -> unit  = "atcf_matrix_set_identity"
external m_assign_m_m    : c_matrix -> c_matrix -> c_matrix -> unit  = "atcf_matrix_assign_m_m"
external m_set           : c_matrix -> int -> int -> float -> unit  = "atcf_matrix_set"
external m_transpose     : c_matrix -> unit  = "atcf_matrix_transpose"
external m_scale         : c_matrix -> float -> unit  = "atcf_matrix_scale"
external m_add_scaled    : c_matrix -> c_matrix -> float -> unit  = "atcf_matrix_add_scaled"
external m_lup_decompose : c_matrix -> c_vector = "atcf_matrix_lup_decompose"
external m_lup_get_l     : c_matrix -> unit = "atcf_matrix_lup_get_l"
external m_lup_get_u     : c_matrix -> unit = "atcf_matrix_lup_get_u"
external m_lup_invert    : c_matrix -> unit  = "atcf_matrix_lup_invert"
external m_lup_inverse   : c_matrix -> c_matrix  = "atcf_matrix_lup_inverse"

(*b quaternion functions *)
external q_create  : unit -> c_quaternion   = "atcf_quaternion_create"
external q_create_rijk  : float -> float -> float -> float -> c_quaternion   = "atcf_quaternion_create_rijk"
external q_clone   : c_quaternion -> c_quaternion   = "atcf_quaternion_clone"
external q_destroy : c_quaternion -> unit  = "atcf_quaternion_destroy"
external q_get_rijk  : c_quaternion -> float array  = "atcf_quaternion_rijk"
external q_modulus : c_quaternion -> float = "atcf_quaternion_modulus"
external q_modulus_squared : c_quaternion -> float = "atcf_quaternion_modulus_squared"
external q_normalize : c_quaternion -> unit  = "atcf_quaternion_normalize"
external q_conjugate : c_quaternion -> unit  = "atcf_quaternion_conjugate"
external q_scale         : c_quaternion -> float -> unit  = "atcf_quaternion_scale"
external q_add_scaled    : c_quaternion -> c_quaternion -> float -> unit  = "atcf_quaternion_add_scaled"
(**/**)

let log = Printf.printf

(*a Vector, matrix and quaternion classes - mutually recursive *)
(*b vector *)
class vector (cv:c_vector) =
  object (self)
         val v = cv
         initializer
           Gc.finalise (fun self -> self#destroy ()) self
         method destroy () =
           log "destroying vector %d\n" (Oo.id self) ;
           v_destroy v
         method get_cv      = v
         method copy       = new vector(v_clone v)
         method coords     = v_coords v
         method length     = v_length v
         method set n f    = v_set v n f ; self
         method assign (v2:vector)  = v_assign v v2#get_cv ; self
         method assign_m_v (m:matrix) (v2:vector)  = v_assign_m_v v m#get_cm v2#get_cv ; self
         method scale f    = v_scale v f ; self
         method modulus    = v_modulus v
         method modulus_squared   = v_modulus_squared v
         method add (v2:vector)    = v_add_scaled v v2#get_cv 1.0 ; self
         method add_scaled (v2:vector) f = v_add_scaled v v2#get_cv f ; self
         method normalize     = v_normalize v ; self
         method dot_product (v2:vector) = v_dot_product v v2#get_cv
         method cross_product3 (v2:vector) = new vector(v_cross_product v v2#get_cv)
         method angle_axis_to3 (v2:vector) = let (va,c,s) = v_angle_axis_to v v2#get_cv in (new vector(va),c,s)
         method repr = let f i c = (Printf.printf "%d:%f " i c) in Array.iteri f (v_coords v)  ; self
  end
and
(*b matrix *)
 matrix (cm:c_matrix) =
  object (self)
         val m = cm
         initializer
           Gc.finalise (fun self -> self#destroy ()) self
         method destroy () =
           log "destroying matrix %d\n" (Oo.id self) ;
           m_destroy m
         method get_cm      = m
         method set r c f  = m_set m r c f ; self
         method copy       = new matrix(m_clone m)
         method identity   = m_identity m ; self
         method nrows      = m_nrows m
         method ncols      = m_ncols m
         method row_vector n     = new vector(m_row_vector m n)
         method col_vector n     = new vector(m_col_vector m n)
         method scale f      = (m_scale m f) ; self
         method transpose    = (m_transpose m) ; self
         method add_scaled (m2:matrix) f = (m_add_scaled m m2#get_cm f) ; self
         method apply (v:vector) = new vector(m_apply m v#get_cv)
         method assign_m_m (m1:matrix) (m2:matrix) = m_assign_m_m m m1#get_cm m2#get_cm ; self
         method lup_decompose = new vector(m_lup_decompose m)
         method lup_get_l     = (m_lup_get_l m) ; self
         method lup_get_u     = (m_lup_get_u m) ; self
         method lup_invert    = (m_lup_invert m) ; self
         method lup_inverse   = new matrix(m_lup_inverse m)
         method repr = let rec show_row r l =
                         if r==l then ()
                         else begin
                             let f c d = (Printf.printf "%d,%d:%f " r c d) in
                             Array.iteri f (self#row_vector r)#coords ;
                             Printf.printf "\n" ;
                             show_row (r+1) l ;
                           end
                       in
                       show_row 0 (self#nrows) ; self
  end
(*b quaternion *)
and
 quaternion (cq:c_quaternion) =
  object (self)
         val q = cq
         initializer
           Gc.finalise (fun self -> self#destroy ()) self
         method destroy () =
           log "destroying matrix %d\n" (Oo.id self) ;
           q_destroy q
         method get_cq      = q
         method get_rijk    = q_get_rijk q
         method copy       = new quaternion(q_clone q)
         method scale f      = (q_scale q f) ; self
         method add_scaled (q2:quaternion) f = (q_add_scaled q q2#get_cq f) ; self
         method modulus = q_modulus q
         method modulus_squared = q_modulus_squared q
         method repr = Printf.printf "%f,%f,%f,%f " 0. 1. 2. 3. ; self
  end

(*a Vector constructors *)
let mkvector n =
  new vector (v_create n)
let mkvector2 c0 c1 =
  ((new vector (v_create 2))#set 0 c0)#set 1 c1
let mkvector3 c0 c1 c2 =
  (((new vector (v_create 3))#set 0 c0)#set 1 c1)#set 2 c2
let mkvector4 c0 c1 c2 c3 =
  ((((new vector (v_create 4))#set 0 c0)#set 1 c1)#set 2 c2)#set 3 c3

let matrix_x_vector m v = (v#copy)#assign_m_v m v

(*a Matrix *)

let mkmatrix r c =
  new matrix (m_create r c)
let matrix_x_matrix m1 m2 = (m1#copy)#assign_m_m m1 m2

(*a Quaternion *)

let mkquaternion =
  new quaternion (q_create ())

let mkquaternion_rijk r i j k =
  new quaternion (q_create_rijk r i j k)
