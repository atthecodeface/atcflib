(** Atcflib wrapper  *)
type c_vector
type c_matrix
type c_quaternion
type t_timer

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
each Vector.vector module type instance. For performance, these should
not be constantly allocated and released, hence they should be
sensibly managed.

For performance, the wrapper does not range-check accesses to vectors;
this means that a vector should always be of the correct length for
the operation being requested of it. A wrapping module could be
written that would implement these checks, but this module is aimed at
high performance first.  *)
module rec
    Vector :
      sig
        (** The vector type is an instance of the C++ c_vector class
        *)
        type vector = { cv : c_vector; (** *) }  (** *)

        val create : c_vector -> vector
        (** Vector.create is a private method to create a
        Vector.vector from a c_vector *)

        val make             : int -> vector
        (** Vector.make n creates a new vector of length n; the length
        of the vector is then fixed forever. *)

        val make2            : float -> float -> vector
        (** Vector.make2 x y is a convenience function to create a
        vector of length 2 with value (x, y)
        *)

        val make3            : float -> float -> float -> vector
        (** Vector.make3 x y z is a convenience function to create a
        vector of length 3 with value (x, y, z) *)

        val make4            : float -> float -> float -> float -> vector
        (** Vector.make4 w x y z is a convenience function to create a
        vector of length 4 with value (w, x, y, z) *)

        val copy             : vector -> vector
        (** Vector.copy creates a copy of a vector, using a new
        c_vector for the new vector *)

        val length           : vector -> int
        (** Vector.length returns the length of the vector *)

        val coords           : vector -> float array
        (** Vector.coords returns a float array with the coordinates of the vector *)

        val set              : vector -> n:int -> f:float -> vector
        (** Vector.set v n f sets the nth coordinate of the vector to be f. It returns v *)

        val scale            : vector -> f:float -> vector
        (** Vector.scale v f scales all the coordinates of the vector by f, and returns v *)

        val modulus          : vector -> float
        (** Vector.modulus v returns the modulus (length) of the
        vector - i.e. the square root of the inner product of the
        vector with itself sqrt(x*x + y*y + z*z + ...) *)

        val modulus_squared  : vector -> float
        (** Vector.modulus_squared v returns the square of the modulus (length) of the
        vector - i.e. inner product of the vector with itself (x*x + y*y + z*z + ...) *)

        val add              : vector -> vector -> vector
        (** Vector.add v v2 adds vector v2 to vector v, requiring both
        to be of the same length. After updating v, it returns it. *)

        val add_scaled       : vector -> vector -> f:float -> vector
        (** Vector.add_scaled v v2 adds vector v2 scaled by f to vector v,
        requiring both vectors to be of the same length. After updating v, it returns it. *)

        val normalize        : vector -> vector
        (** Vector.normalize v scales vector v by the reciprocal of
        its modulus - i.e. afterwards it should be a unit vector. If
        the modulus of the vector is less than a value epsilon (approx
        1E-20) then the vector is zeroed. It returns v*)

        val assign           : vector -> vector -> vector
        (** Vector.assign v v2 sets v to have the same coordinates as
        v2 (effectively v.coords := v2.coords); it requires that v
        have the same length as v2 (which is not checked). It returns v. *)

        val assign_m_v       : vector -> Matrix.matrix -> vector -> vector
        (** Vector.assign v m v2 sets v to be Matrix m * v2; it
        requires that v have the same length as M.nrows, and M.ncols
        is the length of v2 (neither of which is not checked). It
        returns v. *)

        val assign_q_as_rotation : vector -> Quaternion.t -> float * float
        (** Vector.assign_q_as_rotation v q assumes q is a unit
        quaternion, and sets v to be the axis of rotation that q
        represents (in three dimensions), and it returns a tuple of
        (cosine, sine) of the angle of rotation. It requires v to have
        length 3, which is not checked. *)

        val apply_q          : vector -> Quaternion.t -> vector
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

        val dot_product      : vector -> vector -> float
        (** Vector.dot_product v v2 calculates the inner product, or
        dot product, of two vectors. This is calculated by adding up
        the corresponding coordinates of each vector multiplied
        together (i.e. v.x*v2.x + v.y*v2.y + ...). It requires the two
        vectors to be of the same length. *)

        val cross_product3   : vector -> vector -> vector
        (** Vector.cross_product3 v v2 creates a {e new} Vector.vector
        that is the 3-dimensional vector product (outer product) of
        two three dimensional vectors. *)

        val angle_axis_to3   : vector -> vector -> vector * float * float
        (** Vector.angle_axis_to3 v v2 creates a {e new} axis
        Vector.vector that is the 3-dimensional vector is the cross
        product (outer product) of two three dimensional vectors, and
        it calculates the (cosine, sine) of the angle of rotation
        around the axis vector. It returns (axis, cosine, sine) as a
        tuple. *)

        val repr             : vector -> string
        (** Vector.repr v produces a textual representation of the
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
           type matrix = { cm : c_matrix; }
           val create : c_matrix -> matrix
           val copy : matrix -> matrix
           val set : matrix -> int -> int -> float -> matrix
           val identity : matrix -> matrix
           val nrows : matrix -> int
           val ncols : matrix -> int
           val row_vector : matrix -> int -> Vector.vector
           val col_vector : matrix -> int -> Vector.vector
           val scale : matrix -> f:float -> matrix
           val transpose : matrix -> matrix
           val add_scaled : matrix -> matrix -> float -> matrix
           val apply : matrix -> Vector.vector -> Vector.vector
           val assign_m_m : matrix -> matrix -> matrix -> matrix
           val lup_decompose : matrix -> Vector.vector
           val lup_get_l : matrix -> matrix
           val lup_get_u : matrix -> matrix
           val lup_invert : matrix -> matrix
           val lup_inverse : matrix -> matrix
           val make          : int -> int -> matrix
           val matrix_x_matrix : matrix -> matrix -> matrix
           val repr : matrix -> string
         end
   and Quaternion :
         sig
           type t = { cq : c_quaternion; }
           val create : c_quaternion -> t
           val get_cq : t -> c_quaternion
           val copy : t -> t
           val get_rijk : t -> float array
           val assign : t -> t -> t
           val assign_q_q : t -> t -> t -> t
           val assign_lookat : t -> Vector.vector -> Vector.vector -> t
           val assign_of_rotation : t -> Vector.vector -> float -> float -> t
           val scale : t -> float -> t
           val add_scaled : t -> t -> float -> t
           val reciprocal : t -> t
           val conjugate : t -> t
           val modulus : t -> float
           val modulus_squared : t -> float
           val premultiply : t -> t -> t
           val postmultiply : t -> t -> t
           val make              : unit -> t
           val make_rijk         : float -> float -> float -> float -> t
           val repr : t -> string
         end
