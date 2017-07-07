(** ATCFLIB module - OCaml wrapper for the prototyping atcflib C++ library

 The C++ library provides objects such as vector, quaternion,
 etc. This OCaml library wraps those objects.

 This library aims to provide a near one-to-one mapping to the C++
 library. A consequence of this is that an OCaml thing has to map
 cleanly to a C++ object, in a manner that supports allocation and
 destruction of the C objects (which are created using new/delete in
 the library C++ wrapper).

 It seems that the OCaml world is split amongst those who believe
 objects and classes should be removed from OCaml, and those who think
 they are useful. As implementation choices for this library there is
 a need to map from an OCaml thing onto a C++ object. The OCaml thing
 has to map neatly to the C++ constructor/destructor, i.e. when the
 OCaml thing is deallocated (is garbage collected) the C++ object is
 or has been deallocated.

 The first choice was to use instances of class objects for them;
 this requires three classes declared class vector (cv:c_vector) = object and
 matrix (cm:_cmatrix) and quaternion (cq:c_quaternion); then the mutual object
 can create objects with new vector, new matrix, new quaternion, etc.

 The second choice is to use a classless object for the OCaml
 thing. This works with let rec vector (cv:c_vector) = object
 (self:'vector) ... and matrix (cm:c_matrix) = object (self:'matrix)
 ..., and mutual object creation can be performed by dropping the
 'new'.

 It is also possible to use modules. For example:

module type Vector = sig
    type t = \{ cv : c_vector \}
    val create : c_vector -> t
    val get_cv : t -> c_vector
    val length : t -> int
    val set    : t -> n:int -> f:float  -> t
    end
module type Matrix = sig
    type t = \{ cm: c_matrix \}
    val create : c_matrix -> t
    val get_cm : t -> c_matrix
    end
module rec Vector : Vector = struct
    type t = \{ cv : c_vector \}
     let create (cv_in:c_vector) = \{ cv = cv_in \}
     let get_cv m = m.cv
     let copy   m = Vector.create (v_clone (get_cv m))
     let coords m = v_coords m.cv
     let length m = v_length m.cv
     let set m ~n ~f   = v_set m.cv n f ; m
     let assign m v2  = v_assign m.cv v2.cv ; m
     let assign_m_v m m2 v2  = v_assign_m_v m.cv (Matrix.get_cm m2) v2.cv ; m
end
and Matrix : Matrix = struct
     type t = \{ cm: c_matrix \}
     let create (cm_in:c_matrix) = \{ cm = cm_in \}
     let get_cm m = m.cm
     let copy   m = Matrix.create (m_clone (get_cm m))
     let apply  m (v:Vector.t) = Vector.create (m_apply m (Vector.get_cv v))
end

  In this case the usage looks like:

           let v = Atcflib.Vector.create(Atcflib.v_create 2) in
           (Atcflib.Vector.set v ~n:0 ~f:1.0) |>
           Atcflib.Vector.set ~n:1 ~f:2.0 ;
           assert_equal_int "length" (Atcflib.Vector.length v) 2

  As opposed to an object variant of:

           let v = (new Atcflib.Vector(Atcflib.v_create 2)#set 0 1.0)#set 1 2.0 in
           assert_equal_int "length" v#length 2

  The module variant requires ~n and ~f if the pipe syntax is to be used; the object syntax
  with method invocation is cleaner.

  The downside of an object setup would be performance if objects are
  dynamically created (with new) constantly; this is an undesirable
  methodology, as with any OCaml structure around the C++ library
  requires use of the C++ objects, which have also to be allocated for
  every new OCaml thing.

  There is also a penalty for invoking methods on an object. To
  evaluate this, the test 'test_time.ml' uses the Atcflib timer module
  to time the invocation of methods, C calls, and module functions. On
  a 2015 MacBook Pro (2.7GHz Intel i7) an object method invocation
  takes 6ns more than a module function, which takes 0.5ns more than a
  direct C function invocation. So the penalty is about 6ns.

 *)

(**/**)

(*a Types - private *)
type c_vector
type c_matrix
type c_quaternion
type t_timer

(*a Atcflib OCaml wrapper C functions - private *)
(*b timer functions *)
external t_create   : unit -> t_timer   = "atcf_timer_create"
external t_init     : t_timer -> unit   = "atcf_timer_init"
external t_entry    : t_timer -> unit   = "atcf_timer_entry"
external t_exit     : t_timer -> unit   = "atcf_timer_exit"
external t_value    : t_timer -> int64  = "atcf_timer_value"
external t_value_us : t_timer -> float  = "atcf_timer_value_us"

(*b vector functions *)
external v_create  : int -> c_vector   = "atcf_vector_create"
external v_clone   : c_vector -> c_vector   = "atcf_vector_clone"
external v_modulus : c_vector -> float = "atcf_vector_modulus"
external v_modulus_squared : c_vector -> float = "atcf_vector_modulus_squared"
external v_assign  : c_vector -> c_vector -> unit = "atcf_vector_assign"
external v_assign_m_v  : c_vector -> c_matrix -> c_vector -> unit = "atcf_vector_assign_m_v"
external v_assign_q    : c_vector -> c_quaternion -> float * float = "atcf_vector_assign_q"
external v_apply_q     : c_vector -> c_quaternion -> unit = "atcf_vector_apply_q"
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
external q_assign_q     : c_quaternion -> c_quaternion -> unit   = "atcf_quaternion_assign_q"
external q_assign_lookat : c_quaternion -> c_vector -> c_vector -> unit   = "atcf_quaternion_assign_lookat"
external q_assign_of_rotation : c_quaternion -> c_vector -> float -> float -> unit   = "atcf_quaternion_assign_of_rotation"
external q_get_rijk  : c_quaternion -> float array  = "atcf_quaternion_rijk"
external q_modulus : c_quaternion -> float = "atcf_quaternion_modulus"
external q_modulus_squared : c_quaternion -> float = "atcf_quaternion_modulus_squared"
external q_normalize : c_quaternion -> unit  = "atcf_quaternion_normalize"
external q_reciprocal : c_quaternion -> unit  = "atcf_quaternion_reciprocal"
external q_conjugate : c_quaternion -> unit  = "atcf_quaternion_conjugate"
external q_scale         : c_quaternion -> float -> unit  = "atcf_quaternion_scale"
external q_premultiply   : c_quaternion -> c_quaternion -> unit  = "atcf_quaternion_premultiply"
external q_postmultiply   : c_quaternion -> c_quaternion -> unit  = "atcf_quaternion_postmultiply"
external q_add_scaled    : c_quaternion -> c_quaternion -> float -> unit  = "atcf_quaternion_add_scaled"

(*a Logging functions - private *)
let qlog format = Printf.ifprintf () format
let vlog format = Printf.printf format
let log = qlog
let verbose_log = true
(*let log = if verbose_log then vlog else qlog*)
(* This should work, except the type of ifprintf is incompatible with printf... *sigh*
let verbose_log = true
let log = if verbose_log then
     fun format -> (Printf.ifprintf () format)
     else
     fun format -> (Printf.printf format)
 *)
(**/**)

(*a Vector, matrix and quaternion classes - mutually recursive *)
(*b vector *)
 (** A vector is an object representing a set of N 'float' coordinates.
  *)
let rec vector (cv:c_vector) =
  object (self:'vector)
         val v = cv
         method get_cv      = v
         method copy       = vector(v_clone v)
         method coords     = v_coords v
         method length     = v_length v
         method set n f    = v_set v n f ; self
         method assign (v2:'vector)  = v_assign v v2#get_cv ; self
         method assign_m_v (m:'matrix) (v2:'vector)  = v_assign_m_v v m#get_cm v2#get_cv ; self
         method assign_q_as_rotation (q:'quaternion) = (v_assign_q v q#get_cq)
         method apply_q (q:'quaternion) = (v_apply_q v q#get_cq) ; self
         method scale f    = v_scale v f ; self
         method modulus    = v_modulus v
         method modulus_squared   = v_modulus_squared v
         method add (v2:'vector)    = v_add_scaled v v2#get_cv 1.0 ; self
         method add_scaled (v2:'vector) f = v_add_scaled v v2#get_cv f ; self
         method normalize     = v_normalize v ; self
         method dot_product (v2:'vector) = v_dot_product v v2#get_cv
         method cross_product3 (v2:'vector) = vector(v_cross_product v v2#get_cv)
         method angle_axis_to3 (v2:'vector) = let (va,c,s) = v_angle_axis_to v v2#get_cv in (vector(va),c,s)
         method repr = let f i c = (Printf.printf "%d:%f " i c) in Array.iteri f (v_coords v)  ; self
  end
and
(*b matrix *)
 matrix (cm:c_matrix) =
  object (self:'matrix)
         val m = cm
         method get_cm      = m
         method set r c f  = m_set m r c f ; self
         method copy       = matrix(m_clone m)
         method identity   = m_identity m ; self
         method nrows      = m_nrows m
         method ncols      = m_ncols m
         method row_vector n     = vector(m_row_vector m n)
         method col_vector n     = vector(m_col_vector m n)
         method scale f      = (m_scale m f) ; self
         method transpose    = (m_transpose m) ; self
         method add_scaled (m2:'matrix) f = (m_add_scaled m m2#get_cm f) ; self
         method apply (v:'vector) = vector(m_apply m v#get_cv)
         method assign_m_m (m1:'matrix) (m2:'matrix) = m_assign_m_m m m1#get_cm m2#get_cm ; self
         method lup_decompose = vector(m_lup_decompose m)
         method lup_get_l     = (m_lup_get_l m) ; self
         method lup_get_u     = (m_lup_get_u m) ; self
         method lup_invert    = (m_lup_invert m) ; self
         method lup_inverse   = matrix(m_lup_inverse m)
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
 (** This is supposed to be the comment for class quaternion **)
 quaternion (cq:c_quaternion) =
  object (self:'quaternion)
         val q = cq
         method get_cq       = q
         method get_rijk     = q_get_rijk q
         method assign (q1:'quaternion)  = q_assign_q q q1#get_cq ; self
         method assign_q_q (q1:'quaternion) (q2:'quaternion)  = (q_assign_q q q1#get_cq) ; (q_postmultiply q q2#get_cq) ; self
         method assign_lookat (at:'vector) (up:'vector) = (q_assign_lookat q at#get_cv up#get_cv) ; self
         method assign_of_rotation (axis:'vector) c s  = (q_assign_of_rotation q axis#get_cv c s) ; self
         method copy                                  = quaternion(q_clone q)
         method scale f                               = (q_scale q f) ; self
         method add_scaled (q2:'quaternion) f          = (q_add_scaled q q2#get_cq f) ; self
         method reciprocal                            = q_reciprocal q ; self
         method conjugate                             = q_conjugate q ; self
         method modulus                               = q_modulus q
         method modulus_squared                       = q_modulus_squared q
         method premultiply (q2:'quaternion)           = q_premultiply q q2#get_cq ; self
         method postmultiply (q2:'quaternion)          = q_postmultiply q q2#get_cq ; self
         method repr = let rijk=self#get_rijk in Printf.printf "%f,%f,%f,%f " rijk.(0) rijk.(1) rijk.(2) rijk.(3); self
  end

(*a Vector constructors *)
let mkvector n =
  vector (v_create n)
let mkvector2 c0 c1 =
  ((vector (v_create 2))#set 0 c0) #set 1 c1
let mkvector3 c0 c1 c2 =
  (((vector (v_create 3))#set 0 c0)#set 1 c1)#set 2 c2
let mkvector4 c0 c1 c2 c3 =
  ((((vector (v_create 4))#set 0 c0)#set 1 c1)#set 2 c2)#set 3 c3

let matrix_x_vector m v = (v#copy)#assign_m_v m v

(*a Matrix *)

let mkmatrix r c =
  matrix (m_create r c)
let matrix_x_matrix m1 m2 = (m1#copy)#assign_m_m m1 m2

(*a Quaternion *)
let mkquaternion =
  quaternion (q_create ())

let mkquaternion_rijk r i j k =
  quaternion (q_create_rijk r i j k)

(*a Timer module *)
module type Timer = sig
    type t ={ c : t_timer }
    val create   : unit -> t
    val init     : t -> unit
    val exit     : t -> unit
    val entry    : t -> unit
    val value_us : t -> float
    end
module Timer : Timer = struct
    type t = {c:t_timer}
    let create ()   = {c = t_create ()}
    let init t      = t_init t.c
    let exit t      = t_exit t.c
    let entry t     = t_entry t.c
    let value_us t  = t_value_us t.c
end
