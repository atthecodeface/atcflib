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

(*a Libraries *)
open Bigarray

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
external v_create_bigarray_slice  : (float, float64_elt, c_layout) Bigarray.Genarray.t -> l:int -> o:int -> s:int -> c_vector   = "atcf_vector_create_bigarray_slice"
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
external q_assign_lookat_graphics : c_quaternion -> c_vector -> c_vector -> unit   = "atcf_quaternion_assign_lookat_graphics"
external q_assign_lookat_aeronautic : c_quaternion -> c_vector -> c_vector -> unit   = "atcf_quaternion_assign_lookat_aeronautic"
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

(*a Timer module *)
module Timer : sig
    type timer ={ c : t_timer }
    val make     : unit -> timer
    val init     : timer -> unit
    val exit     : timer -> unit
    val entry    : timer -> unit
    val value_us : timer -> float
end = struct
    type timer = {c:t_timer}
    let make ()     = {c = t_create ()}
    let init t      = t_init t.c
    let exit t      = t_exit t.c
    let entry t     = t_entry t.c
    let value_us t  = t_value_us t.c
end

(* Module version *)
module rec Vector : sig
    type vector = { cv : c_vector ;
                    ba : ((float, float64_elt, c_layout) Bigarray.Genarray.t) option }
    val create  : c_vector -> vector
    val length  : vector -> int
    val copy    : vector -> vector
    val coords  : vector -> float array
    val length  : vector -> int
    val set     : vector -> n:int -> f:float  -> vector
    val assign  : vector -> vector -> vector
    val assign_m_v : vector -> Matrix.matrix -> vector -> vector
    val assign_q_as_rotation : vector -> Quaternion.quaternion -> float * float
    val apply_q              : vector -> Quaternion.quaternion -> vector
    val scale                : vector -> f:float  -> vector
    val modulus              : vector -> float
    val modulus_squared      : vector -> float
    val add                  : vector -> vector -> vector
    val add_scaled           : vector -> vector -> f:float -> vector
    val normalize            : vector -> vector
    val dot_product          : vector -> vector -> float
    val cross_product3       : vector -> vector -> vector
    val angle_axis_to3       : vector -> vector -> vector * float * float
    val make                 : int -> vector
    val make2                : float -> float -> vector
    val make3                : float -> float -> float -> vector
    val make4                : float -> float -> float -> float -> vector
    val make_slice_array     : (float, float64_elt, c_layout) Bigarray.Genarray.t -> int -> int -> int -> vector
    val repr                 : vector -> string
end = struct
    type vector = { cv : c_vector ;
                    ba : ((float, float64_elt, c_layout) Bigarray.Genarray.t) option }

     let create cv_in = { cv = cv_in; ba = None }
     let copy        v = Vector.create (v_clone v.cv)
     let coords      v        = v_coords v.cv
     let length      v        = v_length v.cv
     let set         v ~n ~f  = v_set v.cv n f ; v
     let assign      v v2     = v_assign v.cv v2.cv ; v
     let assign_m_v  v m2 v2  = v_assign_m_v v.cv m2.Matrix.cm v2.cv ; v
     let add_scaled  v v2 ~f  = v_add_scaled v.cv v2.cv f; v
     let add         v v2     = v_add_scaled v.cv v2.cv 1.0; v
     let normalize   v        = v_normalize v.cv ; v
     let modulus     v        = v_modulus v.cv
     let modulus_squared  v   = v_modulus_squared v.cv
     let scale       v ~f     = v_scale v.cv f ; v
     let dot_product v v2     = v_dot_product v.cv v2.cv
     let assign_m_v  v m v2   = v_assign_m_v v.cv m.Matrix.cm v2.cv ; v
     let assign_q_as_rotation v q = (v_assign_q v.cv q.Quaternion.cq)
     let apply_q     v q      = (v_apply_q v.cv q.Quaternion.cq) ; v
     let cross_product3  v v2 = Vector.create(v_cross_product v.cv v2.cv)
     let angle_axis_to3  v v2 = let (va,c,s) = v_angle_axis_to v.cv v2.cv in (Vector.create(va),c,s)
     let make        n            = { cv = v_create n ; ba = None }
     let make2       c0 c1        = let v = make 2 in set (set v 0 c0) 1 c1
     let make3       c0 c1 c2     = let v = make 3 in set (set (set v 0 c0) 1 c1) 2 c2
     let make4       c0 c1 c2 c3  = let v = make 4 in set (set (set (set v 0 c0) 1 c1) 2 c2) 3 c3
     let matrix_x_vector      m v = assign_m_v (copy v) m v
     let make_slice_array b n o s   = { cv = v_create_bigarray_slice b n o s ;
                                        ba = Some b}
     let repr        v         =
       let f c s = (Printf.sprintf "%f%s " c s) in
       Array.fold_right f (v_coords v.cv) ""
end
   and Matrix : sig
     type matrix = { cm: c_matrix }
    val create : c_matrix -> matrix
    val copy   : matrix -> matrix
    val apply  : matrix -> Vector.vector -> Vector.vector
    val set          : matrix -> int -> int -> float -> matrix
    val identity     : matrix -> matrix
    val nrows        : matrix -> int
    val ncols        : matrix -> int
    val row_vector   : matrix -> int -> Vector.vector
    val col_vector   : matrix -> int -> Vector.vector
    val scale        : matrix -> f:float -> matrix
    val transpose    : matrix -> matrix
    val add_scaled   : matrix -> matrix -> float -> matrix
    val apply        : matrix -> Vector.vector -> Vector.vector
    val assign_m_m   : matrix -> matrix -> matrix -> matrix
    val lup_decompose : matrix -> Vector.vector
    val lup_get_l     : matrix -> matrix
    val lup_get_u     : matrix -> matrix
    val lup_invert    : matrix -> matrix
    val lup_inverse   : matrix -> matrix
    val make          : int -> int -> matrix
    val matrix_x_matrix : matrix -> matrix -> matrix
    val repr          : matrix -> string
end = struct
     type matrix = { cm: c_matrix }
     let create (cm_in:c_matrix) = { cm = cm_in }
     let copy   m = Matrix.create (m_clone m.cm)
     let apply  m v = Vector.create (m_apply m v.Vector.cv)
     let set m r c f     = m_set m.cm r c f ; m
     let identity m      = m_identity m.cm ; m
     let nrows m         = m_nrows m.cm
     let ncols m         = m_ncols m.cm
     let row_vector m n  = Vector.create(m_row_vector m.cm n)
     let col_vector m n  = Vector.create(m_col_vector m.cm n)
     let scale m ~f      = (m_scale m.cm f) ; m
     let transpose m     = (m_transpose m.cm) ; m
     let add_scaled m m2 f = (m_add_scaled m.cm m2.cm f) ; m
     let apply m v         = Vector.create(m_apply m.cm v.Vector.cv)
     let assign_m_m m m1 m2 = m_assign_m_m m.cm m1.cm m2.cm ; m
     let lup_decompose m = Vector.create(m_lup_decompose m.cm)
     let lup_get_l m     = (m_lup_get_l m.cm)  ; m
     let lup_get_u m     = (m_lup_get_u m.cm)  ; m
     let lup_invert m    = (m_lup_invert m.cm) ; m
     let lup_inverse m   = create(m_lup_inverse m.cm)
     let make r c        = create (m_create r c)
     let matrix_x_matrix m1 m2 = assign_m_m (make (nrows m1) (ncols m2)) m1 m2
     let repr m = let f c s = (Printf.sprintf "%f%s " c s) in
                  let rec show_row r l s =
                         if r==l then s
                         else begin
                             show_row (r+1) l (Array.fold_right f (v_coords (m_row_vector m.cm r)) s)
                           end
                       in
                       show_row 0 (m_nrows m.cm) ""
end
and Quaternion : sig
    type quaternion = { cq: c_quaternion }
    val create : c_quaternion -> quaternion
    val copy   : quaternion -> quaternion
    val get_rijk           : quaternion -> float array
    val assign             : quaternion -> quaternion -> quaternion
    val assign_q_q         : quaternion -> quaternion -> quaternion -> quaternion
    val assign_lookat_graphics      : quaternion -> Vector.vector -> Vector.vector -> quaternion 
    val assign_lookat_aeronautic      : quaternion -> Vector.vector -> Vector.vector -> quaternion 
    val assign_of_rotation : quaternion -> Vector.vector -> float -> float -> quaternion
    val scale              : quaternion -> float -> quaternion
    val add_scaled        : quaternion -> quaternion -> float -> quaternion
    val reciprocal        : quaternion -> quaternion
    val conjugate         : quaternion -> quaternion
    val modulus           : quaternion -> float
    val modulus_squared   : quaternion -> float
    val premultiply       : quaternion -> quaternion -> quaternion
    val postmultiply      : quaternion -> quaternion -> quaternion
    val make              : unit -> quaternion
    val make_rijk         : float -> float -> float -> float -> quaternion
    val repr              : quaternion -> string
end = struct
     type quaternion = { cq: c_quaternion }
     let create (cq_in:c_quaternion) = { cq = cq_in }
     let copy   q = create (q_clone q.cq)
     let get_rijk q    = q_get_rijk q.cq
     let assign q q1   = q_assign_q q.cq q1.cq ; q
     let assign_q_q q q1 q2 = (q_assign_q q.cq q1.cq) ; (q_postmultiply q.cq q2.cq) ; q
     let assign_lookat_graphics q at up =  (q_assign_lookat_graphics q.cq at.Vector.cv up.Vector.cv) ; q
     let assign_lookat_aeronautic q at up =  (q_assign_lookat_aeronautic q.cq at.Vector.cv up.Vector.cv) ; q
     let assign_of_rotation q axis c s  = (q_assign_of_rotation q.cq axis.Vector.cv c s) ; q
     let scale q f                             = (q_scale q.cq f) ; q
     let add_scaled q q2 f          = (q_add_scaled q.cq q2.cq f) ; q
     let reciprocal q                           = q_reciprocal q.cq ; q
     let conjugate q                            = q_conjugate q.cq ; q
     let modulus q                              = q_modulus q.cq
     let modulus_squared q                      = q_modulus_squared q.cq
     let premultiply q q2      = q_premultiply q.cq q2.cq ; q
     let postmultiply q q2     = q_postmultiply q.cq q2.cq ; q
     let make _ = create (q_create ())
     let make_rijk r i j k = create (q_create_rijk r i j k)
     let repr q = let rijk=(q_get_rijk q.cq) in (Printf.sprintf "%f,%f,%f,%f " rijk.(0) rijk.(1) rijk.(2) rijk.(3))
end

