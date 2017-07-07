type c_vector
type c_matrix
type c_quaternion
type t_timer

module type Timer =
  sig
    type t = { c : t_timer; }
    val create : unit -> t
    val init : t -> unit
    val exit : t -> unit
    val entry : t -> unit
    val value_us : t -> float
  end
module Timer : Timer
module rec Vector :
  sig
    type t = { cv : c_vector; }
    val create : c_vector -> t
    val get_cv : t -> c_vector
    val copy : t -> t
    val coords : t -> float array
    val length : t -> int
    val set : t -> n:int -> f:float -> t
    val assign : t -> t -> t
    val assign_m_v : t -> Matrix.t -> t -> t
    val assign_q_as_rotation : t -> Quaternion.t -> float * float
    val apply_q : t -> Quaternion.t -> t
    val scale : t -> f:float -> t
    val modulus : t -> float
    val modulus_squared : t -> float
    val add : t -> t -> t
    val add_scaled : t -> t -> f:float -> t
    val normalize : t -> t
    val dot_product : t -> t -> float
    val cross_product3 : t -> t -> t
    val angle_axis_to3 : t -> t -> t * float * float
    val make                 : int -> t
    val make2                : float -> float -> t
    val make3                : float -> float -> float -> t
    val make4                : float -> float -> float -> float -> t
    val repr : t -> string
  end
and Matrix :
  sig
    type t = { cm : c_matrix; }
    val create : c_matrix -> t
    val get_cm : t -> c_matrix
    val copy : t -> t
    val set : t -> int -> int -> float -> t
    val identity : t -> t
    val nrows : t -> int
    val ncols : t -> int
    val row_vector : t -> int -> Vector.t
    val col_vector : t -> int -> Vector.t
    val scale : t -> f:float -> t
    val transpose : t -> t
    val add_scaled : t -> t -> float -> t
    val apply : t -> Vector.t -> Vector.t
    val assign_m_m : t -> t -> t -> t
    val lup_decompose : t -> Vector.t
    val lup_get_l : t -> t
    val lup_get_u : t -> t
    val lup_invert : t -> t
    val lup_inverse : t -> t
    val make          : int -> int -> t
    val matrix_x_matrix : t -> t -> t
    val repr : t -> string
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
    val assign_lookat : t -> Vector.t -> Vector.t -> t
    val assign_of_rotation : t -> Vector.t -> float -> float -> t
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
