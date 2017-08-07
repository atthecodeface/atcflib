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
open Re
open Result

(*a Types - private *)
type c_vector
type c_matrix
type c_quaternion
type t_timer
type c_bunzip
type bz_uint8_array = (int, int8_unsigned_elt, c_layout) Bigarray.Array1.t

(*a Atcflib OCaml wrapper C functions - private *)
(*b timer functions *)
external t_create   : unit -> t_timer   = "atcf_timer_create"
external t_init     : t_timer -> unit   = "atcf_timer_init"
external t_entry    : t_timer -> unit   = "atcf_timer_entry"
external t_exit     : t_timer -> unit   = "atcf_timer_exit"
external t_value    : t_timer -> int64  = "atcf_timer_value"
external t_value_us : t_timer -> float  = "atcf_timer_value_us"

(*b compression functions *)
external bz_create    : unit -> c_bunzip   = "atcf_bunzip_create"
external bz_destroy   : c_bunzip -> unit   = "atcf_bunzip_destroy"
external bz_set_size  : c_bunzip -> int -> unit   = "atcf_bunzip_set_size"
external bz_block_data         : c_bunzip -> int -> int -> unit   = "atcf_bunzip_block_data"
external bz_block_read_header  : c_bunzip -> unit                 = "atcf_bunzip_block_read_header"

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
external v_get     : c_vector -> int -> float = "atcf_vector_get"
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
external m_assign_from_q : c_matrix -> c_quaternion -> unit  = "atcf_matrix_assign_from_q"
external m_set           : c_matrix -> int -> int -> float -> unit  = "atcf_matrix_set"
external m_transpose     : c_matrix -> unit  = "atcf_matrix_transpose_data"
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

(*b bunzip functions *)
external bz_create    : unit -> c_bunzip   = "atcf_bunzip_create"
external bz_destroy   : c_bunzip -> unit   = "atcf_bunzip_destroy"
external bz_set_size  : c_bunzip -> int -> unit   = "atcf_bunzip_set_size"
external bz_block_data           : c_bunzip -> bz_uint8_array -> int64 -> int64 -> int   = "atcf_bunzip_block_data"
external bz_block_read_header    : c_bunzip -> int                 = "atcf_bunzip_block_read_header"
external bz_block_huffman_decode : c_bunzip -> int                 = "atcf_bunzip_block_huffman_decode"
external bz_block_size           : c_bunzip -> int                 = "atcf_bunzip_block_size"
external bz_block_start_bit      : c_bunzip -> int64               = "atcf_bunzip_block_start_bit"
external bz_block_end_bit        : c_bunzip -> int64               = "atcf_bunzip_block_end_bit"
external bz_block_magic          : c_bunzip -> int64               = "atcf_bunzip_block_magic"
external bz_block_mtf            : c_bunzip -> int                 = "atcf_bunzip_block_mtf"
external bz_block_bwt_order      : c_bunzip -> int                 = "atcf_bunzip_block_bwt_order"
external bz_block_no_rle_size    : c_bunzip -> int                 = "atcf_bunzip_block_no_rle_size"
external bz_block_no_rle_decompress : c_bunzip -> bz_uint8_array -> int = "atcf_bunzip_block_no_rle_decompress"
(* external bz_block_decompress     : c_bunzip -> int                 = "atcf_bunzip_block_decompress" *)

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

(*a Vector module version *)
module rec Vector : sig
    type t = { cv : c_vector ;
                    ba : ((float, float64_elt, c_layout) Bigarray.Genarray.t) option }
    val create  : c_vector -> t
    val length  : t -> int
    val copy    : t -> t
    val coords  : t -> float array
    val length  : t -> int
    val get     : t -> int -> float
    val set     : int -> float  -> t -> t
    val assign  : t -> t -> t
    val assign_m_v : Matrix.t -> t -> t -> t
    val assign_q_as_rotation : t -> Quaternion.t -> float * float
    val apply_q              : Quaternion.t -> t -> t
    val scale                : float  -> t -> t
    val modulus              : t -> float
    val modulus_squared      : t -> float
    val add                  : t -> t -> t
    val add_scaled           : float -> t -> t -> t
    val normalize            : t -> t
    val dot_product          : t -> t -> float
    val cross_product3       : t -> t -> t
    val angle_axis_to3       : t -> t -> t * float * float
    val make                 : int -> t
    val make2                : float -> float -> t
    val make3                : float -> float -> float -> t
    val make4                : float -> float -> float -> float -> t
    val make_slice_array     : (float, float64_elt, c_layout) Bigarray.Genarray.t -> int -> int -> int -> t
    val str                 : t -> string
end = struct
    type t = { cv : c_vector ;
                    ba : ((float, float64_elt, c_layout) Bigarray.Genarray.t) option }

     let create cv_in = { cv = cv_in; ba = None }
     let copy        v = Vector.create (v_clone v.cv)
     let coords      v        = v_coords v.cv
     let length      v        = v_length v.cv
     let get         v n      = v_get v.cv n
     let set         n f v    = v_set v.cv n f ; v
     let assign      v2 v     = v_assign v.cv v2.cv ; v
     let assign_m_v  m2 v2 v  = v_assign_m_v v.cv m2.Matrix.cm v2.cv ; v
     let add_scaled  f v2 v   = v_add_scaled v.cv v2.cv f; v
     let add         v2 v     = v_add_scaled v.cv v2.cv 1.0; v
     let normalize   v        = v_normalize v.cv ; v
     let modulus     v        = v_modulus v.cv
     let modulus_squared  v   = v_modulus_squared v.cv
     let scale       f v     = v_scale v.cv f ; v
     let dot_product v v2     = v_dot_product v.cv v2.cv
     let assign_m_v  m v2 v   = v_assign_m_v v.cv m.Matrix.cm v2.cv ; v
     let assign_q_as_rotation v q = (v_assign_q v.cv q.Quaternion.cq)
     let apply_q     q v      = (v_apply_q v.cv q.Quaternion.cq) ; v
     let cross_product3  v v2 = Vector.create(v_cross_product v.cv v2.cv)
     let angle_axis_to3  v v2 = let (va,c,s) = v_angle_axis_to v.cv v2.cv in (Vector.create(va),c,s)
     let make        n            = { cv = v_create n ; ba = None }
     let make2       c0 c1        = make 2 |> set 0 c0 |> set 1 c1
     let make3       c0 c1 c2     = make 3 |> set 0 c0 |> set 1 c1 |> set 2 c2
     let make4       c0 c1 c2 c3  = make 4 |> set 0 c0 |> set 1 c1 |> set 2 c2 |> set 3 c3
     let matrix_x_vector      m v = assign_m_v m v (copy v)
     let make_slice_array b n o s   = { cv = v_create_bigarray_slice b n o s ;
                                        ba = Some b}
     let str        v         =
       let f c s = (Printf.sprintf "%f %s" c s) in
       Array.fold_right f (v_coords v.cv) ""
end
(*a and Matrix module *)
   and Matrix : sig
     type t = { cm: c_matrix }
    val create : c_matrix -> t
    val copy   : t -> t
    val apply  : t -> Vector.t -> Vector.t
    val set          : int -> int -> float -> t -> t
    val identity     : t -> t
    val nrows        : t -> int
    val ncols        : t -> int
    val row_vector   : t -> int -> Vector.t
    val col_vector   : t -> int -> Vector.t
    val scale        : float -> t -> t
    val transpose    : t -> t
    val add_scaled   : float -> t -> t -> t
    val apply        : t -> Vector.t -> Vector.t
    val assign_m_m   : t -> t -> t -> t
    val assign_from_q : Quaternion.t  -> t -> t
    val lup_decompose : t -> Vector.t
    val lup_get_l     : t -> t
    val lup_get_u     : t -> t
    val lup_invert    : t -> t
    val lup_inverse   : t -> t
    val make          : int -> int -> t
    val matrix_x_matrix : t -> t -> t
    val str          : t -> string
end = struct
     type t = { cm: c_matrix }
     let create (cm_in:c_matrix) = { cm = cm_in }
     let copy   m = Matrix.create (m_clone m.cm)
     let apply  m v = Vector.create (m_apply m v.Vector.cv)
     let set r c f m     = m_set m.cm r c f ; m
     let identity m      = m_identity m.cm ; m
     let nrows m         = m_nrows m.cm
     let ncols m         = m_ncols m.cm
     let row_vector m n  = Vector.create(m_row_vector m.cm n)
     let col_vector m n  = Vector.create(m_col_vector m.cm n)
     let scale f m      = (m_scale m.cm f) ; m
     let transpose m     = (m_transpose m.cm) ; m
     let add_scaled f m2 m = (m_add_scaled m.cm m2.cm f) ; m
     let apply m v         = Vector.create(m_apply m.cm v.Vector.cv)
     let assign_m_m m1 m2 m = m_assign_m_m m.cm m1.cm m2.cm ; m
     let assign_from_q q m = m_assign_from_q m.cm q.Quaternion.cq ; m
     let lup_decompose m = Vector.create(m_lup_decompose m.cm)
     let lup_get_l m     = (m_lup_get_l m.cm)  ; m
     let lup_get_u m     = (m_lup_get_u m.cm)  ; m
     let lup_invert m    = (m_lup_invert m.cm) ; m
     let lup_inverse m   = create(m_lup_inverse m.cm)
     let make r c        = create (m_create r c)
     let matrix_x_matrix m1 m2 = assign_m_m (make (nrows m1) (ncols m2)) m1 m2
     let str m = let rec show_row r l s =
                   if r==l then s
                   else
                     let f acc c = (Printf.sprintf "%s %f " acc c) in
                     show_row (r+1) l (Array.fold_left f s (v_coords (m_row_vector m.cm r)))
                   in
                     show_row 0 (m_nrows m.cm) ""
end
(*a and Quaternion module *)
and Quaternion : sig
    type t = { cq: c_quaternion }
    val create : c_quaternion -> t
    val copy   : t -> t
    val get_rijk           : t -> float array
    val assign             : t -> t -> t
    val assign_q_q         : t -> t -> t -> t
    val assign_lookat_graphics      : Vector.t -> Vector.t -> t -> t 
    val assign_lookat_aeronautic    : Vector.t -> Vector.t -> t -> t 
    val assign_of_rotation : Vector.t -> float -> float -> t -> t
    val scale              : float -> t -> t
    val add_scaled         : float -> t -> t -> t
    val reciprocal         : t -> t
    val conjugate          : t -> t
    val modulus            : t -> float
    val modulus_squared    : t -> float
    val premultiply        : t -> t -> t
    val postmultiply       : t -> t -> t
    val make               : unit -> t
    val make_rijk          : float -> float -> float -> float -> t
    val str               : t -> string
end = struct
     type t = { cq: c_quaternion }
     let create (cq_in:c_quaternion) = { cq = cq_in }
     let copy   q = create (q_clone q.cq)
     let get_rijk q    = q_get_rijk q.cq
     let assign q1 q   = q_assign_q q.cq q1.cq ; q
     let assign_q_q q1 q2 q = (q_assign_q q.cq q1.cq) ; (q_postmultiply q.cq q2.cq) ; q
     let assign_lookat_graphics at up q =  (q_assign_lookat_graphics q.cq at.Vector.cv up.Vector.cv) ; q
     let assign_lookat_aeronautic at up q =  (q_assign_lookat_aeronautic q.cq at.Vector.cv up.Vector.cv) ; q
     let assign_of_rotation axis c s  q = (q_assign_of_rotation q.cq axis.Vector.cv c s) ; q
     let scale f q                             = (q_scale q.cq f) ; q
     let add_scaled f q2 q            = (q_add_scaled q.cq q2.cq f) ; q
     let reciprocal q                           = q_reciprocal q.cq ; q
     let conjugate q                            = q_conjugate q.cq ; q
     let modulus q                              = q_modulus q.cq
     let modulus_squared q                      = q_modulus_squared q.cq
     let premultiply q2 q      = q_premultiply q.cq q2.cq ; q
     let postmultiply q2 q     = q_postmultiply q.cq q2.cq ; q
     let make _ = create (q_create ())
     let make_rijk r i j k = create (q_create_rijk r i j k)
     let str q = let rijk=(q_get_rijk q.cq) in (Printf.sprintf "%f,%f,%f,%f " rijk.(0) rijk.(1) rijk.(2) rijk.(3))
end

(*a Bunzip module *)
module Bunzip = struct
  type  ('a) block_type = | Error of string
                    | End_marker
                    | Ok of 'a
  (*b Indexentry submodule *)
  module Indexentry = struct
    type t = {
        mutable no_rle_start   : int64 ;
        mutable rle_start      : int64 ;
        mutable bz_start_bit   : int64 ;
        mutable bz_num_bits    : int32 ;
        mutable no_rle_length  : int32 ;
        mutable rle_length     : int32 ;
        mutable block_crc      : int32 ;
        mutable decomp_data    : int32 ;
        mutable user_data      : (int32 * int32 * int32 * int32 * int32)
      }
    let create _ = { no_rle_start = 0L ;
                     rle_start = 0L ;
                     bz_start_bit = 0L ;
                     bz_num_bits = 0l ;
                     no_rle_length = 0l ;
                     rle_length = 0l ;
                     block_crc = 0l ;
                     decomp_data = 0l ;
                     user_data = (0l, 0l, 0l, 0l, 0l);
                   }
    let bz_bit_pos i bz_sb bz_eb = 
      i.bz_start_bit <- bz_sb; 
      i.bz_num_bits  <- Int64.to_int32 (Int64.sub bz_eb bz_sb) ;
      ()
    let no_rle i pi l = 
      i.no_rle_start <- Int64.add pi.no_rle_start (Int64.of_int32 pi.no_rle_length) ;
      i.no_rle_length <- Int32.of_int l ;
      ()

    let index_entry bz prev =
      let i = create () in
      bz_bit_pos i (bz_block_start_bit bz) (bz_block_end_bit bz) ;
      no_rle i prev (bz_block_no_rle_size bz) ;
      (i,(bz_block_end_bit bz))

    (*f build_index_r - build the index entries starting at
    'start_bit' given last index_entry 'prev' (to give the start of
    this entry, if it has exists)

      For debug, there is a maximum number of blocks that can be put
      in the index - if that is exceeded then the index entries from
      here on out are empty

      Otherwise try to read the header of a BZIP block starting at
      start_bit and do its Huffman decode - that is sufficient to give
      its non-RLE length

      Possibly the block is an end marker - in which case the index
      entries from here on out are empty.

      If a really serious error occurred (such as a Huffman decode
      error) then perhaps an exception should be raised, but should we
      abandond all the hard work so far? For now print out a
      warning...

      If all is fine then build a new index_entry i and find its
      file-ending-bit e, and then build the index from e onwards with
      this as the previous.

     *)
    let rec build_index_r (bz_ba_huffman_decode:int64->int block_type) bz start_bit verbose n prev max_n =
      if (n>max_n) then []
      else begin
          verbose n prev ;
          match (bz_ba_huffman_decode start_bit) with
            Error e -> Printf.printf "Error %s\n" e ; []
          | End_marker -> []
          | Ok _    -> let (i,e) = index_entry bz prev in (i::(build_index_r bz_ba_huffman_decode bz e verbose (n+1) i max_n))
        end

    let rec write_int_n i max_i f n =
      let b i = (Int64.to_int (Int64.shift_right n i)) land 255 in
      output_byte f (b i) ;
      if (i<max_i-8) then write_int_n (8+i) max_i f n 
    let write_int64 f n = write_int_n 0 64 f n
    let write_int32 f n = write_int_n 0 32 f (Int64.of_int32 n) 
    let write i f = 
      let ua,ub,uc,ud,ue = i.user_data in
      write_int64 f i.no_rle_start ;
      write_int64 f i.rle_start  ;
      write_int64 f i.bz_start_bit  ;
      write_int32 f i.bz_num_bits  ;
      write_int32 f i.no_rle_length  ;
      write_int32 f i.rle_length  ;
      write_int32 f i.block_crc  ;
      write_int32 f i.decomp_data ;
      write_int32 f ua ;
      write_int32 f ub ;
      write_int32 f uc ;
      write_int32 f ud ;
      write_int32 f ue 
    let rec read_int_n i max_i b n v =
      let vb = Int64.of_int(Char.code(Bytes.get b n)) in
      let nv = Int64.logor v (Int64.shift_left vb i) in
      if (i>=max_i-8) then nv else read_int_n (8+i) max_i b (n+1) nv
    let read_int64 b n = read_int_n 0 64 b n 0L
    let read_int32 b n = Int64.to_int32(read_int_n 0 32 b n 0L)
    let read file bytes =
      try begin
          really_input file bytes 0 64 ;
          Some { no_rle_start  = read_int64 bytes 0 ;
                 rle_start     = read_int64 bytes 8 ;
                 bz_start_bit  = read_int64 bytes 16 ;
                 bz_num_bits   = read_int32 bytes 24 ;
                 no_rle_length = read_int32 bytes 28 ;
                 rle_length    = read_int32 bytes 32 ;
                 block_crc     = read_int32 bytes 36 ;
                 decomp_data   = read_int32 bytes 40 ;
                 user_data     = (0l, 0l, 0l, 0l, 0l);
               }
        end 
      with
        End_of_file -> None
    let str i =
      Printf.sprintf "%20Ld +%8ld : %20Ld +%8ld" i.bz_start_bit i.bz_num_bits i.no_rle_start i.no_rle_length
  end
  (*b Index submodule *)
  module Index = struct
    type t = {
        entries    : Indexentry.t list ;
      }

    (*f verbose_progress is used for verbose output while building the index *)
    let verbose_progress n i = Printf.printf "%d:Bz at bit %Ld : %ld : %Ld\r%!" n i.Indexentry.bz_start_bit i.Indexentry.bz_num_bits i.Indexentry.no_rle_start

    (*f quiet_progress is used for non-verbose output while building the index *)
    let quiet_progress n i = ()

    (*f building_index *)
    let build_index (bz_ba_huffman_decode:int64->int block_type)  bz verbose = { 
        entries = 
          let progress_fn verbose = if verbose then verbose_progress else quiet_progress in
          Indexentry.build_index_r bz_ba_huffman_decode bz 32L (progress_fn verbose) 0 (Indexentry.create ()) 100000
      }
    let show f i = 
      let df n ie = f (Printf.sprintf "%8d: %s" n (Indexentry.str ie)) in
      List.iteri df i.entries
    let write f i =
      let wf n ie = Indexentry.write ie f in
      List.iteri wf i.entries
    let read filename verbose =
      let b = Bytes.create 64 in
      try begin
          let f = open_in_bin filename in
          let rec read_entries entries =
            match (Indexentry.read f b) with
              Some e -> read_entries (entries@[e])
            | None   -> entries
          in
          { entries = (read_entries []) ;
          }
        end
      with Not_found as e -> Printf.printf "Index file %s not found\n" filename; raise e
    let rec block_containing entries start n last = 
      match entries with
        [] -> last
      | hd :: tl -> if (hd.Indexentry.no_rle_start<=start) then
                      (block_containing tl start (n+1) (n,entries))
                    else
                      last
    let blocks_containing index (start:int64) (length:int) =
       let last = (Int64.add start (Int64.of_int (length-1))) in
       let rec first_n l n r = match l with
           [] -> r
         | hd::tl -> if (n<=0) then r else first_n tl (n-1) (r@[hd]) in
       let (first_block,ie0) = block_containing index.entries start 0 (0,index.entries) in
       let (last_block,ie1)  = block_containing ie0           last  first_block (first_block,ie0) in
       let first_index = (List.hd ie0) in
       let first_index_end = Int64.add first_index.Indexentry.no_rle_start (Int64.of_int32 first_index.Indexentry.no_rle_length) in
       if ((Int64.compare first_index_end start)<0) then []
       else
         first_n ie0 (1+last_block-first_block) []
  end
  (*b Structure type for module *)
  type t = {
      fd : Unix.file_descr ;
      ba : bz_uint8_array ;
      bz : c_bunzip ;
      mutable index : Index.t option ;
    }

  (*f open_bunzip *)
  let open_bunzip filename =
    let open_read filename =
      try begin
          Unix.openfile filename [Unix.O_RDONLY ;] 0
        end
      with Not_found as e -> Printf.printf "Bzip file %s not found\n" filename; raise e
    in
    let fd = open_read filename in
    let ba = Bigarray.Array1.map_file fd (*pos:(int64 0)*) Bigarray.Int8_unsigned c_layout false (-1) in
    let bz = bz_create () in
    let ba0 = Bigarray.Array1.get ba 0 in
    let ba1 = Bigarray.Array1.get ba 1 in
    let ba2 = Bigarray.Array1.get ba 2 in
    let ba3 = Bigarray.Array1.get ba 3 in
    if ((ba0==0x42) &&
          (ba1==0x5a) &&
            (ba2==0x68) &&
              true
       ) then begin
        bz_set_size bz (ba3-48) ;
        Some { fd ; bz; ba; index=None }
      end else begin
        None
      end
  (*f block_read_header *)
    exception Bad_bzip_file of string
    let chk_error (m:string) x = if x!=0 then raise (Bad_bzip_file m) else Ok x
    let block_read_header bz start_bit =
      if ((bz_block_data bz.bz bz.ba start_bit 0L)!=0) then raise (Bad_bzip_file "reading block data")
      else
        let hdr_type = bz_block_read_header bz.bz in
        if (hdr_type<0) then raise (Bad_bzip_file "reading block header")
        else if (hdr_type>0) then End_marker
        else Ok 0
    let block_huffman_decode bz start_bit =
      match (block_read_header bz start_bit) with
        Error _ as e -> e
      | End_marker -> End_marker
      | Ok _ -> if ((bz_block_huffman_decode bz.bz)!=0) then Error "Bad Huffman data"
        else Ok 0
  (*f create_index *) 
  let create_index bz verbose = 
    let bz_ba_huffman_decode start_bit = block_huffman_decode bz start_bit in
    let index = (Index.build_index bz_ba_huffman_decode bz.bz verbose) in
    bz.index <- Some index ;
    index
  (*f read_index *) 
  let read_index bz filename verbose = 
    let index = (Index.read filename verbose) in
    bz.index <- Some index ;
    index

  (*f block_decompress_no_rle *)
  let ( >>= ) (x:int block_type) f =
    match x with
      Ok v              -> f v
    | End_marker   as e -> e
    | Error _      as e -> e

  let block_decompress_no_rle bz start_bit =
    (*    Printf.printf "Block decompress %Ld\n" start_bit ;*)
    block_huffman_decode bz start_bit >>= fun _ -> 
    chk_error "mtf" (bz_block_mtf bz.bz) >>= fun _ ->
    chk_error "bwt" (bz_block_bwt_order bz.bz) >>= fun _ ->
    let length = (bz_block_no_rle_size bz.bz) in
    (*Printf.printf "No RLE size %d\n" length ;*)
    let data = Bigarray.Array1.create Bigarray.int8_unsigned c_layout length in
    chk_error "decomp" (bz_block_no_rle_decompress bz.bz data ) >>= fun _ ->
    (Ok data)

  (*f read_data_no_rle *) 
  exception Invalid_index of string
  let rec decompress_and_copy_data bz ba blks dstart rofs length verbose : (int,string) result =
    if verbose then Printf.printf "decompress_and_copy_data: Num blks %d\n%!" (List.length blks) ;
    match blks with
      [] -> Result.Ok rofs
    | hd::tl ->
       match (block_decompress_no_rle bz hd.Indexentry.bz_start_bit) with
       Error s -> Result.Error s
       | End_marker -> Result.Ok rofs
       | Ok data ->
          let ds = Int64.to_int (Int64.sub dstart hd.Indexentry.no_rle_start) in
          let dl = (Int32.to_int hd.Indexentry.no_rle_length) - ds in
          let l = min dl length in
          let l_after = length - l in
          (*Printf.printf "Blit %d %d %d %d %d %!\n" ds rofs dl (Bigarray.Array1.dim data) (Bigarray.Array1.dim ba);*)
          let dstart_after = Int64.add dstart (Int64.of_int l) in
          let portion_of_data = (Bigarray.Array1.sub data ds l) in
          let portion_of_dest = (Bigarray.Array1.sub ba   rofs l) in
          Bigarray.Array1.blit portion_of_data portion_of_dest ;
          if l_after<=0 then
            Result.Ok (rofs+l)
          else
            decompress_and_copy_data bz ba tl dstart_after (rofs+l) l_after verbose

  (*f read_data_no_rle - read data without the final RLE, from BZIP at start *)
  let read_data_no_rle ?verbose:(verbose=false) bz ba start = 
    let length = (Bigarray.Array1.dim ba) in
    if verbose then Printf.printf "Read_Data_No_Rle with start %Ld length %d\n%!" start length ;
    match bz.index with
    None -> raise (Invalid_index "invalid index")
    | Some i ->
       let blks = (Index.blocks_containing i start length) in
       if verbose then Printf.printf "Num blks %d\n%!" (List.length blks) ;
       decompress_and_copy_data bz ba blks start 0 length verbose

  let rec unrle_rec (s:string) (i:int) (l:int) (c:char) (r:int) (result:string) =
    if (i=l) then result else
    if ((r=4) && (s.[i]='\x00')) then (unrle_rec s (i+1) l '\x00' 0 result) else
    if (r=4) then (unrle_rec s (i+1) l '\x00' 0 (result ^ String.make (int_of_char s.[i]) c)) else
    if (s.[i]=c) then (unrle_rec s (i+1) l c (r+1) (result ^ (String.make 1 c))) else
    (unrle_rec s (i+1) l (s.[i]) 1 (result ^ (String.make 1 (s.[i]))))
  let rec unrle_unchanged_rec (s:string) (i:int) (l:int) (c:char) (r:int) =
    if (i=l) then s else
    if (r=4) then unrle_rec s i l c r (String.sub s 0 i) else
    if (s.[i]=c) then unrle_unchanged_rec s (i+1) l c (r+1) else
    (unrle_unchanged_rec s (i+1) l (s.[i]) 1)
  let unrle s = unrle_unchanged_rec s 0 (String.length s) '\x00' 0
end
