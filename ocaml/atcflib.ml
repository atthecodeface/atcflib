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
type c_bunzip
type bz_uint8_array = (int, int8_unsigned_elt, c_layout) Bigarray.Genarray.t

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
(*a and Matrix module *)
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
(*a and Quaternion module *)
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

(*a Bunzip module *)
module Bunzip = struct
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
    let ( >>= ) x f =
      match x with
        Ok v         -> f v
      | Error _ as e -> e
    let chk_error m x = if x=0 then Ok x else Error m
    let get_bzip_block_data bz ba start_bit =
      chk_error "data" (bz_block_data bz ba start_bit 100000L) >>= fun _ ->
      chk_error "hdr"  (bz_block_read_header bz)              >>= fun _ ->
      chk_error "huf"  (bz_block_huffman_decode bz)           >>= fun _ ->
      Ok ((bz_block_start_bit bz),(bz_block_end_bit bz),(bz_block_no_rle_size bz))

    let index_entry bz prev =
        let i = create () in
        bz_bit_pos i (bz_block_start_bit bz) (bz_block_end_bit bz) ;
        no_rle i prev (bz_block_no_rle_size bz) ;
        (i,(bz_block_end_bit bz))

    let rec build_index_r bz ba start_bit verbose n prev max_n =
      if (n>max_n) then [] else begin
      verbose n prev ;
      let r = get_bzip_block_data bz ba start_bit in
      match r with
        Error e -> Printf.printf "Error %s\n" e ; []
      | Ok d    -> let (i,e) = index_entry bz prev in (i::(build_index_r bz ba e verbose (n+1) i max_n))
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
    let read f =
        let b = Bytes.create 64 in
        if ((input f b 0 64)<64) then
          None
        else
          Some { no_rle_start  = read_int64 b 0 ;
            rle_start     = read_int64 b 8 ;
            bz_start_bit  = read_int64 b 16 ;
            bz_num_bits   = read_int32 b 24 ;
            no_rle_length = read_int32 b 28 ;
            rle_length    = read_int32 b 32 ;
            block_crc     = read_int32 b 36 ;
            decomp_data   = read_int32 b 40 ;
            user_data     = (0l, 0l, 0l, 0l, 0l);
          }
    let str i =
      Printf.sprintf "%20Ld +%8ld : %20Ld +%8ld" i.bz_start_bit i.bz_num_bits i.no_rle_start i.no_rle_length
end
module Index = struct
    type t = {
    block_size : int ;
    entries    : Indexentry.t list ;
    }
    let verbose_progress n i = Printf.printf "%d:Bz at bit %Ld : %ld : %Ld\r%!" n i.Indexentry.bz_start_bit i.Indexentry.bz_num_bits i.Indexentry.no_rle_start
    let quiet_progress n i = ()
    let build_index bz ba verbose = { 
        block_size = (bz_block_size bz) ;
        entries = 
          let progress_fn verbose = if verbose then verbose_progress else quiet_progress in
          Indexentry.build_index_r bz ba 32L (progress_fn verbose) 0 (Indexentry.create ()) 100000
      }
    let show f i = 
       f (Printf.sprintf "Block size %d" i.block_size) ;
       let df n ie = f (Printf.sprintf "%8d: %s" n (Indexentry.str ie)) in
       List.iteri df i.entries
    let write f i =
       let wf n ie = Indexentry.write ie f in
       List.iteri wf i.entries
    let read filename verbose =
      let f = open_in_bin "8926ff5477452ba9aea697f796e7d3570195576f.csv.bz2.index" in
      let rec read_entries f entries =
        match (Indexentry.read f) with
        Some e -> read_entries f (entries@[e])
        | None   -> entries
      in
      { block_size=9 ;
        entries = (read_entries f []) ;
      }
end
    type t = {
      fd : Unix.file_descr ;
      ba : bz_uint8_array ;
      bz : c_bunzip;
      }
    let open_bunzip filename =
        let open_read filename = Unix.openfile filename [Unix.O_RDONLY ;] 0 in
        let fd = open_read filename in
        let ba = Bigarray.Genarray.map_file fd (*pos:(int64 0)*) Bigarray.Int8_unsigned c_layout false [|-1;|] in
        let bz = bz_create () in
        let ba0 = Bigarray.Genarray.get ba [|0;|] in
        let ba1 = Bigarray.Genarray.get ba [|1;|] in
        let ba2 = Bigarray.Genarray.get ba [|2;|] in
        let ba3 = Bigarray.Genarray.get ba [|3;|] in
        if ((ba0==0x42) &&
            (ba1==0x5a) &&
            (ba2==0x68) &&
            true
           ) then begin
              bz_set_size bz (ba3-48) ;
              Some { fd ; bz; ba }
    end else begin
    None
    end
    let create_index bz verbose = Index.build_index bz.bz bz.ba verbose

end
