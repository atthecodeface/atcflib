(** Libraries  *)
open Bigarray

(** Basic types  *)
type c_vector
type c_matrix
type c_quaternion
type t_timer
type c_bunzip
type bz_uint8_array = (int, int8_unsigned_elt, c_layout) Bigarray.Array1.t
type t_ba_doubles = (float, float64_elt, c_layout) Bigarray.Array1.t
type t_ba_floats  = (float, float32_elt, c_layout) Bigarray.Array1.t

(** The Timer module exposes the atcflib timers, which in turn come
 from sl_timer These are usually used in C modules to time the
 execution of modules very precisely, using the x86 RDTSC instruction,
 which reads an instruction-level timestamp counter. They may be used
 from within an OCaml program to time code sequences accurately.

 This means that the precision, on an x86 implementation, is to the
 instruction execution time.


 The usage is to create a timer:

  [let tim = Timer.create () in]

 and then use it:

  [Timer.init tim ;]

  [Timer.entry tim ;]

  [for i=0 to n do f () done ;]

  [Timer.exit tim ;]

  [Printf.printf "Time taken call in ns:%f\n" ((Timer.value_us tim) /. 10000.)]

  Timers can be reinitialized at any point; then the count the time
  between 'entry' and 'exit' calls. The accumulated time is read with
  'value_us' - read the time taken in usecs. Currently it assumes a
  2.4Ghz clock...

 *)
module Timer :
  sig
    (**/**)
    type timer = { c : t_timer; }
    (**/**)
    (** Timer.make returns a fresh timer structure that can be used
    for accumulating timing of entry and exit from sections of
    code. The structure is initialized ready for first use. *)
    val make : unit -> timer
    (** Timer.init initializes a timer structure, clearing accumulated
    time values. It may be used to reset a timer structure *)
    val init : timer-> unit
    (** Timer.entry is used to mark the entry point to a portion of
    code. This causes the 'last entry' time to be recorded; it is
    usually be paired with a later Timer.exit. The Timer.exit can be
    optionally not taken, though; if Timer.entry is called more than
    once before a Timer.exit, then only the last Timer.entry will have
    an effect. *)
    val entry : timer-> unit
    (** Timer.exit is used to mark the exit of a portion of timed
    code. This causes the current time to be read and subtracted from
    the 'last entry' most recently recorded by a Timer.entry, and the
    resulting interval added to the accumulated time held in the
    Timer. *)
    val exit : timer-> unit
    (** Timer.value_us returns the accumulated time recorded across
    the (many) entry/exits, in microseconds. This depends currently on
    the x86 CPU having a 2.4GHz clock; scale appropriately... *)
    val value_us : timer-> float
  end

(** The Vector module provides effectively one dimensional float
arrays of any length, with vector operations such as the dot product
(inner product). These are provided by the atcflib in C, and interact
with the Quaternion and Matrix modules for use in graphics,
particularly.

The C++ library provides the c_vector class and appropriate methods;
this library module mirrors that, with one c_vector class instance for
each Vector.t module type instance. For performance, these should
not be constantly allocated and released, hence they should be
sensibly managed.

For performance, the wrapper does not range-check accesses to vectors;
this means that a vector should always be of the correct length for
the operation being requested of it. A wrapping module could be
written that would implement these checks, but this module is aimed at
high performance first.  *)
(*type t_vector = { cv : c_vector ; ba : t_ba_doubles option }*)
module rec
    Vector :
      sig
        (** The vector type is an instance of the C++ c_vector class
        *)
        type t

        val make             : int -> t
        (** Vector.make n creates a new vector of length n; the length
        of the vector is then fixed forever. *)

        val make2            : float -> float -> t
        (** Vector.make2 x y is a convenience function to create a
        vector of length 2 with value (x, y)
        *)

        val make3            : float -> float -> float -> t
        (** Vector.make3 x y z is a convenience function to create a
        vector of length 3 with value (x, y, z) *)

        val make4            : float -> float -> float -> float -> t
        (** Vector.make4 w x y z is a convenience function to create a
        vector of length 4 with value (w, x, y, z) *)

        val of_bigarray  : ?length:int -> ?offset:int -> ?stride:int -> t_ba_doubles -> t
        (** Vector.of_bigarray length offset stride ba makes a new vector using a
        slice of the big array *)

        val copy             : t -> t
        (** Vector.copy creates a copy of a vector, using a new
        c_vector for the new vector. Create a new big array *)

        val length           : t -> int
        (** Vector.length returns the length of the vector *)

        val coords           : t -> float array
        (** Vector.coords returns a float array with the coordinates of the vector *)

        val get              : t -> int -> float
        (** Vector.get v n gets the nth coordinate of the vector. *)

        val set              : int -> float -> t -> t
        (** Vector.set n f v sets the nth coordinate of the vector to be f. It returns v *)

        val scale            : float -> t -> t
        (** Vector.scale f v scales all the coordinates of the vector by f, and returns v *)

        val modulus          : t -> float
        (** Vector.modulus v returns the modulus (length) of the
        vector - i.e. the square root of the inner product of the
        vector with itself sqrt(x*x + y*y + z*z + ...) *)

        val modulus_squared  : t -> float
        (** Vector.modulus_squared v returns the square of the modulus (length) of the
        vector - i.e. inner product of the vector with itself (x*x + y*y + z*z + ...) *)

        val add              : t -> t -> t
        (** Vector.add v2 v adds vector v2 to vector v, requiring both
        to be of the same length. After updating v, it returns it. *)

        val add_scaled       : float -> t -> t -> t
        (** Vector.add_scaled v2 adds v vector v2 scaled by f to vector v,
        requiring both vectors to be of the same length. After updating v, it returns it. *)

        val normalize        : t -> t
        (** Vector.normalize v scales vector v by the reciprocal of
        its modulus - i.e. afterwards it should be a unit vector. If
        the modulus of the vector is less than a value epsilon (approx
        1E-20) then the vector is zeroed. It returns v*)

        val assign           : t -> t -> t
        (** Vector.assign v2 v sets v to have the same coordinates as
        v2 (effectively v.coords := v2.coords); it requires that v
        have the same length as v2 (which is not checked). It returns v. *)

        (* val assign_m_v       : Matrix.t -> t -> t -> t *)
        (** Vector.assign_m_v m v2 v sets v to be Matrix m * v2; it
        requires that v have the same length as M.nrows, and M.ncols
        is the length of v2 (neither of which is checked). It
        returns v. *)

        (* val assign_q_as_rotation : t -> Quaternion.t -> float * float *)
        (** Vector.assign_q_as_rotation v q assumes q is a unit
        quaternion, and sets v to be the axis of rotation that q
        stresents (in three dimensions), and it returns a tuple of
        (cosine, sine) of the angle of rotation. It requires v to have
        length 3, which is not checked. *)

        (* val apply_q          : Quaternion.t -> t -> t *)
        (** Vector.apply_q v q applies q to the vector v, and sets the
        first 3 coordinates of v appropriately. This requires v to be
        of length 3 or more.

        Applying a quaternion q is to perform q * V * q', where q' is
        the conjugate of q, and where V is the quaternion whose vector
        component (ijk) is the three coordinates of the vector v.

        The purpose of this is to allow a unit quaternion to be
        treated as a three-dimensional rotation; the axis of rotation
        is the vector component of the quaternion (ijk), the cosine of
        half of the angle of rotation is the real part, and ijk is
        scaled by the sine of half of the angle of rotation. *)

        val dot_product      : t -> t -> float
        (** Vector.dot_product v v2 calculates the inner product, or
        dot product, of two vectors. This is calculated by adding up
        the corresponding coordinates of each vector multiplied
        together (i.e. v.x*v2.x + v.y*v2.y + ...). It requires the two
        vectors to be of the same length. *)

        val assign_cross_product3   : t -> t -> t -> t
        (** Vector.assign_cross_product3 v1 v2 v sets v to be
        the 3-dimensional vector product (outer product) of
        two three dimensional vectors v1 x v2. *)

        val angle_axis_to3   : t -> t -> t -> t * float * float
        (** Vector.angle_axis_to3 v1 v2 v sets v to be the 3-dimensional
        vector is the cross product (outer product) of two three dimensional
        vectors, and it calculates the (cosine, sine) of the angle of rotation
        around the axis vector. It returns (axis, cosine, sine) as a
        tuple. *)

        val str             : t -> string
        (** Vector.str v produces a textual representation of the
        vector, for use in debugging, for example *)
      end
(** The Matrix module provides two dimensional float arrays of
           any number of rows an columns and provides some simple
           operations thereon. These are provided by the atcflib in C,
           and interact with the Quaternion and Matrix modules for use
           in graphics, particularly.

           The C++ library provides the c_matrix class and appropriate
           methods; this library module mirrors that, with one
           c_matrix class instance for each Matrix.matrix module type
           instance. For performance, these should not be constantly
           allocated and released, hence they should be sensibly
           managed.

           For performance, the wrapper does not range-check accesses
           to matrices; this means that a matrix should always be of
           the correct size for the operation being requested of
           it. A wrapping module could be written that would implement
           these checks, but this module is aimed at high performance
           first.  *)
     and
       Matrix :
         sig
           (** The matrix type is an instance of the C++ c_vector class
            *)
           type t

           val of_bigarray  : ncols:int -> nrows:int -> ?offset:int -> ?col_stride:int-> ?row_stride:int -> t_ba_doubles -> t
           (** Matrix.of_bigarray makes a new matrix using a
            slice of the big array *)

           val copy : t -> t
           (** Matrix.copy m creates a copy of matrix m and returns it *)

           val set : int -> int -> float -> t -> t
           (** Matrix.set c r f m sets matrix m element row r column c
           to be f. It returns the matrix *)

           val identity : t -> t
           (** Matrix.identity m sets matrix m to be zero, except where
           for elements whose row number equals the column number,
           where it sets the element to 1. For a square matrix this
           means it sets the matrix to be the identity matrix I. It
           returns the matrix m. *)

           val nrows : t -> int
           (** Matrix.nrows returns the number of rows in the matrix. *)

           val ncols : t -> int
           (** Matrix.ncols returns the number of columns in the matrix. *)

           val row_vector : t -> int -> Vector.t -> Vector.t
           (** Matrix.row_vector m n fills a Vector.t
           corresponding to the n'th row of the matrix.
           It returns the vector *)

           val col_vector : t -> int -> Vector.t -> Vector.t
           (** Matrix.col_vector m n fills a Vector.t
           corresponding to the n'th column of the matrix.
           It returns the vector *)

           val scale : float -> t -> t
           (** Matrix.scale f m scales (multiplies) all the elemnts of
           the matrix by f. It returns the matrix. *)

           val transpose : t -> t
           (** Matrix.transpose m transposes matrix m in-place; the
           updated matrix will be of size NxM if it has originally
           been MxN.   It returns the matrix. *)

           val add_scaled : float -> t -> t -> t
           (** Matrix.add_scaled m2 f m adds m2 scaled by f to matrix
           m, and sets m to the result; this permits add and
           subtract. It returns the matrix m. *)

           val apply : t -> Vector.t -> Vector.t -> Vector.t
           (** Matrix.apply m v1 v multiplies Vector v1 by the matrix m
           (which must have the same length as m has columns) and
           puts the result in v (of length
           m.ncols). It returns v *)

           val assign : t -> t -> t
           (** Matrix.assign m1 m assigns m to be the value of m1. It returns m. *)

           val assign_m_m : t -> t -> t -> t
           (** Matrix.assign_m_m m1 m2 m assigns m to be the result of
           the product of m1 and m2 (in that order). It returns m. *)

           (* val assign_from_q : Quaternion.t  -> t -> t *)
           (** Matrix.assign_from_q m q assumes that q is a
           unit quaternion and it makes m be the identity matrix with
           the top level 3x3 being a rotation that matches the
           quaternion. *)

           val lup_decompose : t -> Vector.t -> Vector.t
           (** Matrix.lup_decompose m performs a lower-upper-pivot
           decomposition (partial pivoting) of a square N-dimensional
           matrix m; that is, it finds L, U and P such that M = L * U
           * P.

           L is a lower triangular matrix (all values above the main
           diagonal of matrix are zeros) with a main diagonal of 1; U
           is an upper triangular matrix (all values below the main
           diagonal of matrix are zeros) with a main diagonal which
           may have other values than one. P is a permutation matrix
           that has a single 1 in each column and row, and hence is a
           permutation mapping rows of the matrix.

           The matrix m is returned, having had its values below the main
           diagonal to be L, and the values along and above the main
           diagonal to be U. The length-N vector (v) representation of P
           is returned from the method; row i of L.U maps to row v[i] of 
           M.

           Since M = L*U*P, M' = P' * U' * L', where ' indicates
           inverse. Note that the inverse of a triangular matrix can
           be calculated relatively simply. *)
           val lup_get_l : t -> t
           (** Matrix.lup_get_l assigns an LU decomposition matrix to
           be its lower triangular matrix, by zeroing the values above
           the diagonal, and setting the diagonal to all ones. It returns the matrix. *)
           val lup_get_u : t -> t
           (** Matrix.lup_get_u assigns an LU decomposition matrix to
           be its upper triangular matrix, by zeroing the values below
           the diagonal. It returns the matrix. *)
           val lup_invert : t -> t
           (** Matrix.lup_invert performs an LUP decomposition on the
           matrix, using this to determine the inverse; the matrix is
           then set to this inverse. It returns the matrix. *)

           (* val lup_inverse : t -> t*)
           (** Matrix.lup_inverse performs an LUP decomposition on the
           matrix, using this to determine the inverse; a new matrix
           is then created and set to this inverse. It returns this
           new matrix. *)
           val make          : int -> int -> t
           (** Matrix.make r c creates a new matrix with r rows anc c
           columns. The size of the matrix may change in the future, if
           it is assigned to a different size of matrix, for example. *)
           val matrix_x_matrix : t -> t -> t
           (** Matrix.matrix_x_matrix m1 m2 multiplies matrix m1 and
           m2 together (in that order, and returns a new matrix with
           the product. *)
           val str : t -> string
         (** Matrix.str m produces a textual representation of the
        matrix, for use in debugging, for example *)
         end
(** The Quaternion module provides quaternions and some simple
           operations thereon. These are provided by the atcflib in C,
           and interact with the Quaternion and Vector modules for use
           in graphics, particularly.

           The C++ library provides the c_quaternion class.

    The roll-pitch-yaw convention for aeronautics is to consider being
    in an aeroplane; roll is to rotate around the nose-to-tail axis of
    the aeroplane, the 'Z' axis; pitch is to rotate about the axis
    through the wings, the 'Y' axis; yaw is to rotate around the axis
    through the pilot's spine, the 'X' axis; hence X is up, Z is
    forward, Y is to the right.

 *)
     and Quaternion :
           sig
            (** The quaternion type is an instance of the C++ c_vector class
            *)
             type t = { cq : c_quaternion; }

             val create : c_quaternion -> t
             (** Quaternion.create is a privatem method to create a quaternion from an instances of the C classs. *)

             val copy : t -> t
             (** Quaternion.copy qdm creates a copy of quaternion q and returns it *)

             val get_rijk : t -> float array
             (** Quaternion.get_rijk q returns a float array of size 4
             containing the scalar (r) and the vector component
             (i,j,k) of the quaternion (in that order). *)

             val assign : t -> t -> t
             (** Quaternion.assign q q1 assigns quaternion q to have the same value as q1, returning q. *)

             val assign_q_q : t -> t -> t -> t
             (** Quaternion.assign q q1 q2 assigns quaternion q to be the product of q1 * q2. It returns q. *)

             val assign_lookat_aeronautic : Vector.t -> Vector.t -> t -> t
             (** Quaternion.assign_lookat_aeronautic q at up assigns quaternion q
             to be a unit quaternion that (when applied) would make
             the vector 'at' map to the positive 3-dimensional unit Z vector
             (0,0,1), with the vector 'up' mapping to be parallel to
             the unit X axis (1,0,0); this matches the aeronautical convention. It returns q. *)

             val assign_lookat_graphics : Vector.t -> Vector.t -> t -> t
             (** Quaternion.assign_lookat_graphics q at up assigns quaternion q
             to be a unit quaternion that (when applied) would make
             the vector 'at' map to the negative 3-dimensional unit Z vector
             (0,0,-1), with the vector 'up' mapping to be parallel to
             the unit Y axis (0,1,0); this matches the normal view of
             a 3-dimensional graphical view with positive X to the
             right, postitive Y up the screen, and positive Z out of
             the screen. It returns q. *)

             val assign_of_rotation : Vector.t -> float -> float -> t -> t
             (** Quaternion.assign_of_rotation q axis cosine sine
             assigns q to be the unit quaternion that corresponds to a
             rotation around the axis by an angle whose cosine and
             sine are provided. It returns q. *)

             val scale : float -> t -> t
             (** Quaternion.scale q f scales (multiplies) q by f. It returns q. *)

             val add_scaled : float -> t -> t -> t
             (** Quaternion.add_scaled q1 q f adds q1 scaled by f to q. It returns q. *)

             val reciprocal : t -> t
             (** Quaternion.conjugate q updates quaternion q to be its
             reciprocal (it negates the vector component, and divides
             the whole by the modulus). It returns q. *)

             val conjugate : t -> t
             (** Quaternion.conjugate q updates quaternion q to be its
             conjugate (it negates the vector component); it returns
             q. *)

             val modulus : t -> float
             (** Quaternion.modulus q returns the modulus of the
             quaternion q - sqrt(r*r + i*i + j*j + k*k). *)

             val modulus_squared : t -> float
             (** Quaternion.modulus_squared q returns the modulus of
             the quaternion q squared - r*r + i*i + j*j + k*k. *)

             val premultiply : t -> t -> t
             (** Quaternion.premultiply q1 q updates q to be the
             product q * q1, returning q. *)

             val postmultiply : t -> t -> t
             (** Quaternion.postmultiply q1 q updates q to be the
             product q1 * q, returning q. *)

             val make              : unit -> t
             (** Quaternion.make creates a new Quaternion.t
             with a value of zero. *)

             val make_rijk         : float -> float -> float -> float -> t
             (** Quaternion.make_rijk creates a new
             Quaternion.t with scalar part r and vector part
             (i,j,k). *)

             val str : t -> string
             (** Quaternion.str m produces a textual representation of the
            quaternion, for use in debugging, for example *)
           end
(** Bunzip module **)
module Bunzip :
  sig
    type  ('a) block_type = | Error of string
                            | End_marker
                            | Ok of 'a
    module Indexentry :
      sig
        type t = {
          mutable no_rle_start : int64;
          mutable rle_start : int64;
          mutable bz_start_bit : int64;
          mutable bz_num_bits : int32;
          mutable no_rle_length : int32;
          mutable rle_length : int32;
          mutable block_crc : int32;
          mutable decomp_data : int32;
          mutable user_data : int32 * int32 * int32 * int32 * int32;
        }
        val create : 'a -> t
        val bz_bit_pos : t -> int64 -> int64 -> unit
        val no_rle : t -> t -> int -> unit
        val index_entry : c_bunzip -> t -> t * int64
        val build_index_r :
           (int64 -> int block_type) ->
          c_bunzip -> int64 -> (int -> t -> 'b) -> int -> t -> int -> t list
        val write : t -> out_channel -> unit
        val str : t -> string
      end
    module Index :
      sig
        type t = { entries : Indexentry.t list; }
        val verbose_progress : int -> Indexentry.t -> unit
        val quiet_progress : 'a -> 'b -> unit
        val build_index : (int64 -> int block_type) -> c_bunzip -> bool -> t
        val show : (string -> unit) -> t -> unit
        val write : out_channel -> t -> unit
        val read : string -> 'b -> t
      end
    type t = { fd : Unix.file_descr; ba : bz_uint8_array; bz : c_bunzip;            mutable index : Index.t option;}
    val open_bunzip : string -> t option
    val create_index : t -> bool -> Index.t
    val read_index : t -> string -> 'a -> Index.t 
     exception Invalid_index of string
    val read_data_no_rle : ?verbose:bool -> t -> bz_uint8_array -> int64 -> (int,string) result
    val unrle : string -> string
  end
