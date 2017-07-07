type c_vector
type c_matrix
type c_quaternion
type t_timer

external t_create : unit -> t_timer = "atcf_timer_create"
external t_init : t_timer -> unit = "atcf_timer_init"
external t_entry : t_timer -> unit = "atcf_timer_entry"
external t_exit : t_timer -> unit = "atcf_timer_exit"
external t_value : t_timer -> int64 = "atcf_timer_value"
external t_value_us : t_timer -> float = "atcf_timer_value_us"
external v_create : int -> c_vector = "atcf_vector_create"
external v_clone : c_vector -> c_vector = "atcf_vector_clone"
external v_modulus : c_vector -> float = "atcf_vector_modulus"
external v_modulus_squared : c_vector -> float
  = "atcf_vector_modulus_squared"
external v_assign : c_vector -> c_vector -> unit = "atcf_vector_assign"
external v_assign_m_v : c_vector -> c_matrix -> c_vector -> unit
  = "atcf_vector_assign_m_v"
external v_assign_q : c_vector -> c_quaternion -> float * float
  = "atcf_vector_assign_q"
external v_apply_q : c_vector -> c_quaternion -> unit = "atcf_vector_apply_q"
external v_normalize : c_vector -> unit = "atcf_vector_normalize"
external v_length : c_vector -> int = "atcf_vector_length"
external v_coords : c_vector -> float array = "atcf_vector_coords"
external v_set : c_vector -> int -> float -> unit = "atcf_vector_set"
external v_scale : c_vector -> float -> unit = "atcf_vector_scale"
external v_add_scaled : c_vector -> c_vector -> float -> unit
  = "atcf_vector_add_scaled"
external v_dot_product : c_vector -> c_vector -> float
  = "atcf_vector_dot_product"
external v_cross_product : c_vector -> c_vector -> c_vector
  = "atcf_vector_cross_product3"
external v_angle_axis_to : c_vector -> c_vector -> c_vector * float * float
  = "atcf_vector_angle_axis_to3"
external m_create : int -> int -> c_matrix = "atcf_matrix_create"
external m_clone : c_matrix -> c_matrix = "atcf_matrix_clone"
external m_nrows : c_matrix -> int = "atcf_matrix_nrows"
external m_ncols : c_matrix -> int = "atcf_matrix_ncols"
external m_row_vector : c_matrix -> int -> c_vector
  = "atcf_matrix_row_vector"
external m_col_vector : c_matrix -> int -> c_vector
  = "atcf_matrix_col_vector"
external m_apply : c_matrix -> c_vector -> c_vector = "atcf_matrix_apply"
external m_identity : c_matrix -> unit = "atcf_matrix_set_identity"
external m_assign_m_m : c_matrix -> c_matrix -> c_matrix -> unit
  = "atcf_matrix_assign_m_m"
external m_set : c_matrix -> int -> int -> float -> unit = "atcf_matrix_set"
external m_transpose : c_matrix -> unit = "atcf_matrix_transpose"
external m_scale : c_matrix -> float -> unit = "atcf_matrix_scale"
external m_add_scaled : c_matrix -> c_matrix -> float -> unit
  = "atcf_matrix_add_scaled"
external m_lup_decompose : c_matrix -> c_vector = "atcf_matrix_lup_decompose"
external m_lup_get_l : c_matrix -> unit = "atcf_matrix_lup_get_l"
external m_lup_get_u : c_matrix -> unit = "atcf_matrix_lup_get_u"
external m_lup_invert : c_matrix -> unit = "atcf_matrix_lup_invert"
external m_lup_inverse : c_matrix -> c_matrix = "atcf_matrix_lup_inverse"
external q_create : unit -> c_quaternion = "atcf_quaternion_create"
external q_create_rijk : float -> float -> float -> float -> c_quaternion
  = "atcf_quaternion_create_rijk"
external q_clone : c_quaternion -> c_quaternion = "atcf_quaternion_clone"
external q_assign_q : c_quaternion -> c_quaternion -> unit
  = "atcf_quaternion_assign_q"
external q_assign_lookat : c_quaternion -> c_vector -> c_vector -> unit
  = "atcf_quaternion_assign_lookat"
external q_assign_of_rotation :
  c_quaternion -> c_vector -> float -> float -> unit
  = "atcf_quaternion_assign_of_rotation"
external q_get_rijk : c_quaternion -> float array = "atcf_quaternion_rijk"
external q_modulus : c_quaternion -> float = "atcf_quaternion_modulus"
external q_modulus_squared : c_quaternion -> float
  = "atcf_quaternion_modulus_squared"
external q_normalize : c_quaternion -> unit = "atcf_quaternion_normalize"
external q_reciprocal : c_quaternion -> unit = "atcf_quaternion_reciprocal"
external q_conjugate : c_quaternion -> unit = "atcf_quaternion_conjugate"
external q_scale : c_quaternion -> float -> unit = "atcf_quaternion_scale"
external q_premultiply : c_quaternion -> c_quaternion -> unit
  = "atcf_quaternion_premultiply"
external q_postmultiply : c_quaternion -> c_quaternion -> unit
  = "atcf_quaternion_postmultiply"
external q_add_scaled : c_quaternion -> c_quaternion -> float -> unit
  = "atcf_quaternion_add_scaled"
val qlog : ('a, unit, 'b, unit) format4 -> 'a
val vlog : ('a, out_channel, unit) format -> 'a
val log : ('a, unit, 'b, unit) format4 -> 'a
val verbose_log : bool
val vector :
  c_vector ->
  (< add : 'a -> 'a; add_scaled : 'a -> float -> 'a;
     angle_axis_to3 : 'a -> 'a * float * float;
     apply_q : (< add_scaled : 'b -> float -> 'b; assign : 'b -> 'b;
                  assign_lookat : 'a -> 'a -> 'b;
                  assign_of_rotation : 'a -> float -> float -> 'b;
                  assign_q_q : 'b -> 'b -> 'b; conjugate : 'b; copy : 'b;
                  get_cq : c_quaternion; get_rijk : float array;
                  modulus : float; modulus_squared : float;
                  postmultiply : 'b -> 'b; premultiply : 'b -> 'b;
                  reciprocal : 'b; repr : 'b; scale : float -> 'b >
                as 'b) ->
               'a;
     assign : 'a -> 'a;
     assign_m_v : (< add_scaled : 'c -> float -> 'c; apply : 'a -> 'a;
                     assign_m_m : 'c -> 'c -> 'c; col_vector : int -> 'a;
                     copy : 'c; get_cm : c_matrix; identity : 'c;
                     lup_decompose : 'a; lup_get_l : 'c; lup_get_u : 'c;
                     lup_inverse : 'c; lup_invert : 'c; ncols : int;
                     nrows : int; repr : 'c; row_vector : int -> 'a;
                     scale : float -> 'c; set : int -> int -> float -> 'c;
                     transpose : 'c >
                   as 'c) ->
                  'a -> 'a;
     assign_q_as_rotation : 'b -> float * float; coords : float array;
     copy : 'a; cross_product3 : 'a -> 'a; dot_product : 'a -> float;
     get_cv : c_vector; length : int; modulus : float;
     modulus_squared : float; normalize : 'a; repr : 'a; scale : float -> 'a;
     set : int -> float -> 'a >
   as 'a)
val matrix :
  c_matrix ->
  (< add_scaled : 'a -> float -> 'a;
     apply : (< add : 'b -> 'b; add_scaled : 'b -> float -> 'b;
                angle_axis_to3 : 'b -> 'b * float * float;
                apply_q : (< add_scaled : 'c -> float -> 'c;
                             assign : 'c -> 'c;
                             assign_lookat : 'b -> 'b -> 'c;
                             assign_of_rotation : 'b -> float -> float -> 'c;
                             assign_q_q : 'c -> 'c -> 'c; conjugate : 'c;
                             copy : 'c; get_cq : c_quaternion;
                             get_rijk : float array; modulus : float;
                             modulus_squared : float;
                             postmultiply : 'c -> 'c; premultiply : 'c -> 'c;
                             reciprocal : 'c; repr : 'c;
                             scale : float -> 'c >
                           as 'c) ->
                          'b;
                assign : 'b -> 'b; assign_m_v : 'a -> 'b -> 'b;
                assign_q_as_rotation : 'c -> float * float;
                coords : float array; copy : 'b; cross_product3 : 'b -> 'b;
                dot_product : 'b -> float; get_cv : c_vector; length :
                int; modulus : float; modulus_squared : float;
                normalize : 'b; repr : 'b; scale : float -> 'b;
                set : int -> float -> 'b >
              as 'b) ->
             'b;
     assign_m_m : 'a -> 'a -> 'a; col_vector : int -> 'b; copy : 'a;
     get_cm : c_matrix; identity : 'a; lup_decompose : 'b; lup_get_l : 'a;
     lup_get_u : 'a; lup_inverse : 'a; lup_invert : 'a; ncols : int;
     nrows : int; repr : 'a; row_vector : int -> 'b; scale : float -> 'a;
     set : int -> int -> float -> 'a; transpose : 'a >
   as 'a)
val quaternion :
  c_quaternion ->
  (< add_scaled : 'a -> float -> 'a; assign : 'a -> 'a;
     assign_lookat : (< add : 'b -> 'b; add_scaled : 'b -> float -> 'b;
                        angle_axis_to3 : 'b -> 'b * float * float;
                        apply_q : 'a -> 'b; assign : 'b -> 'b;
                        assign_m_v : (< add_scaled : 'c -> float -> 'c;
                                        apply : 'b -> 'b;
                                        assign_m_m : 'c -> 'c -> 'c;
                                        col_vector : int -> 'b; copy : 'c;
                                        get_cm : c_matrix; identity : 'c;
                                        lup_decompose : 'b; lup_get_l : 'c;
                                        lup_get_u : 'c; lup_inverse : 'c;
                                        lup_invert : 'c; ncols : int;
                                        nrows : int; repr : 'c;
                                        row_vector : int -> 'b;
                                        scale : float -> 'c;
                                        set : int -> int -> float -> 'c;
                                        transpose : 'c >
                                      as 'c) ->
                                     'b -> 'b;
                        assign_q_as_rotation : 'a -> float * float;
                        coords : float array; copy : 'b;
                        cross_product3 : 'b -> 'b; dot_product : 'b -> float;
                        get_cv : c_vector; length : int; modulus : float;
                        modulus_squared : float; normalize : 'b; repr : 'b;
                        scale : float -> 'b; set : int -> float -> 'b >
                      as 'b) ->
                     'b -> 'a;
     assign_of_rotation : 'b -> float -> float -> 'a;
     assign_q_q : 'a -> 'a -> 'a; conjugate : 'a; copy : 'a;
     get_cq : c_quaternion; get_rijk : float array; modulus : float;
     modulus_squared : float; postmultiply : 'a -> 'a;
     premultiply : 'a -> 'a; reciprocal : 'a; repr : 'a;
     scale : float -> 'a >
   as 'a)
val mkvector :
  int ->
  (< add : 'a -> 'a; add_scaled : 'a -> float -> 'a;
     angle_axis_to3 : 'a -> 'a * float * float;
     apply_q : (< add_scaled : 'b -> float -> 'b; assign : 'b -> 'b;
                  assign_lookat : 'a -> 'a -> 'b;
                  assign_of_rotation : 'a -> float -> float -> 'b;
                  assign_q_q : 'b -> 'b -> 'b; conjugate : 'b; copy : 'b;
                  get_cq : c_quaternion; get_rijk : float array;
                  modulus : float; modulus_squared : float;
                  postmultiply : 'b -> 'b; premultiply : 'b -> 'b;
                  reciprocal : 'b; repr : 'b; scale : float -> 'b >
                as 'b) ->
               'a;
     assign : 'a -> 'a;
     assign_m_v : (< add_scaled : 'c -> float -> 'c; apply : 'a -> 'a;
                     assign_m_m : 'c -> 'c -> 'c; col_vector : int -> 'a;
                     copy : 'c; get_cm : c_matrix; identity : 'c;
                     lup_decompose : 'a; lup_get_l : 'c; lup_get_u : 'c;
                     lup_inverse : 'c; lup_invert : 'c; ncols : int;
                     nrows : int; repr : 'c; row_vector : int -> 'a;
                     scale : float -> 'c; set : int -> int -> float -> 'c;
                     transpose : 'c >
                   as 'c) ->
                  'a -> 'a;
     assign_q_as_rotation : 'b -> float * float; coords : float array;
     copy : 'a; cross_product3 : 'a -> 'a; dot_product : 'a -> float;
     get_cv : c_vector; length : int; modulus : float;
     modulus_squared : float; normalize : 'a; repr : 'a; scale : float -> 'a;
     set : int -> float -> 'a >
   as 'a)
val mkvector2 :
  float ->
  float ->
  (< add : 'a -> 'a; add_scaled : 'a -> float -> 'a;
     angle_axis_to3 : 'a -> 'a * float * float;
     apply_q : (< add_scaled : 'b -> float -> 'b; assign : 'b -> 'b;
                  assign_lookat : 'a -> 'a -> 'b;
                  assign_of_rotation : 'a -> float -> float -> 'b;
                  assign_q_q : 'b -> 'b -> 'b; conjugate : 'b; copy : 'b;
                  get_cq : c_quaternion; get_rijk : float array;
                  modulus : float; modulus_squared : float;
                  postmultiply : 'b -> 'b; premultiply : 'b -> 'b;
                  reciprocal : 'b; repr : 'b; scale : float -> 'b >
                as 'b) ->
               'a;
     assign : 'a -> 'a;
     assign_m_v : (< add_scaled : 'c -> float -> 'c; apply : 'a -> 'a;
                     assign_m_m : 'c -> 'c -> 'c; col_vector : int -> 'a;
                     copy : 'c; get_cm : c_matrix; identity : 'c;
                     lup_decompose : 'a; lup_get_l : 'c; lup_get_u : 'c;
                     lup_inverse : 'c; lup_invert : 'c; ncols : int;
                     nrows : int; repr : 'c; row_vector : int -> 'a;
                     scale : float -> 'c; set : int -> int -> float -> 'c;
                     transpose : 'c >
                   as 'c) ->
                  'a -> 'a;
     assign_q_as_rotation : 'b -> float * float; coords : float array;
     copy : 'a; cross_product3 : 'a -> 'a; dot_product : 'a -> float;
     get_cv : c_vector; length : int; modulus : float;
     modulus_squared : float; normalize : 'a; repr : 'a; scale : float -> 'a;
     set : int -> float -> 'a >
   as 'a)
val mkvector3 :
  float ->
  float ->
  float ->
  (< add : 'a -> 'a; add_scaled : 'a -> float -> 'a;
     angle_axis_to3 : 'a -> 'a * float * float;
     apply_q : (< add_scaled : 'b -> float -> 'b; assign : 'b -> 'b;
                  assign_lookat : 'a -> 'a -> 'b;
                  assign_of_rotation : 'a -> float -> float -> 'b;
                  assign_q_q : 'b -> 'b -> 'b; conjugate : 'b; copy : 'b;
                  get_cq : c_quaternion; get_rijk : float array;
                  modulus : float; modulus_squared : float;
                  postmultiply : 'b -> 'b; premultiply : 'b -> 'b;
                  reciprocal : 'b; repr : 'b; scale : float -> 'b >
                as 'b) ->
               'a;
     assign : 'a -> 'a;
     assign_m_v : (< add_scaled : 'c -> float -> 'c; apply : 'a -> 'a;
                     assign_m_m : 'c -> 'c -> 'c; col_vector : int -> 'a;
                     copy : 'c; get_cm : c_matrix; identity : 'c;
                     lup_decompose : 'a; lup_get_l : 'c; lup_get_u : 'c;
                     lup_inverse : 'c; lup_invert : 'c; ncols : int;
                     nrows : int; repr : 'c; row_vector : int -> 'a;
                     scale : float -> 'c; set : int -> int -> float -> 'c;
                     transpose : 'c >
                   as 'c) ->
                  'a -> 'a;
     assign_q_as_rotation : 'b -> float * float; coords : float array;
     copy : 'a; cross_product3 : 'a -> 'a; dot_product : 'a -> float;
     get_cv : c_vector; length : int; modulus : float;
     modulus_squared : float; normalize : 'a; repr : 'a; scale : float -> 'a;
     set : int -> float -> 'a >
   as 'a)
val mkvector4 :
  float ->
  float ->
  float ->
  float ->
  (< add : 'a -> 'a; add_scaled : 'a -> float -> 'a;
     angle_axis_to3 : 'a -> 'a * float * float;
     apply_q : (< add_scaled : 'b -> float -> 'b; assign : 'b -> 'b;
                  assign_lookat : 'a -> 'a -> 'b;
                  assign_of_rotation : 'a -> float -> float -> 'b;
                  assign_q_q : 'b -> 'b -> 'b; conjugate : 'b; copy : 'b;
                  get_cq : c_quaternion; get_rijk : float array;
                  modulus : float; modulus_squared : float;
                  postmultiply : 'b -> 'b; premultiply : 'b -> 'b;
                  reciprocal : 'b; repr : 'b; scale : float -> 'b >
                as 'b) ->
               'a;
     assign : 'a -> 'a;
     assign_m_v : (< add_scaled : 'c -> float -> 'c; apply : 'a -> 'a;
                     assign_m_m : 'c -> 'c -> 'c; col_vector : int -> 'a;
                     copy : 'c; get_cm : c_matrix; identity : 'c;
                     lup_decompose : 'a; lup_get_l : 'c; lup_get_u : 'c;
                     lup_inverse : 'c; lup_invert : 'c; ncols : int;
                     nrows : int; repr : 'c; row_vector : int -> 'a;
                     scale : float -> 'c; set : int -> int -> float -> 'c;
                     transpose : 'c >
                   as 'c) ->
                  'a -> 'a;
     assign_q_as_rotation : 'b -> float * float; coords : float array;
     copy : 'a; cross_product3 : 'a -> 'a; dot_product : 'a -> float;
     get_cv : c_vector; length : int; modulus : float;
     modulus_squared : float; normalize : 'a; repr : 'a; scale : float -> 'a;
     set : int -> float -> 'a >
   as 'a)
val matrix_x_vector :
  'a -> (< copy : < assign_m_v : 'a -> 'b -> 'c; .. >; .. > as 'b) -> 'c
val mkmatrix :
  int ->
  int ->
  (< add_scaled : 'a -> float -> 'a;
     apply : (< add : 'b -> 'b; add_scaled : 'b -> float -> 'b;
                angle_axis_to3 : 'b -> 'b * float * float;
                apply_q : (< add_scaled : 'c -> float -> 'c;
                             assign : 'c -> 'c;
                             assign_lookat : 'b -> 'b -> 'c;
                             assign_of_rotation : 'b -> float -> float -> 'c;
                             assign_q_q : 'c -> 'c -> 'c; conjugate : 'c;
                             copy : 'c; get_cq : c_quaternion;
                             get_rijk : float array; modulus : float;
                             modulus_squared : float;
                             postmultiply : 'c -> 'c; premultiply : 'c -> 'c;
                             reciprocal : 'c; repr : 'c;
                             scale : float -> 'c >
                           as 'c) ->
                          'b;
                assign : 'b -> 'b; assign_m_v : 'a -> 'b -> 'b;
                assign_q_as_rotation : 'c -> float * float;
                coords : float array; copy : 'b; cross_product3 : 'b -> 'b;
                dot_product : 'b -> float; get_cv : c_vector; length :
                int; modulus : float; modulus_squared : float;
                normalize : 'b; repr : 'b; scale : float -> 'b;
                set : int -> float -> 'b >
              as 'b) ->
             'b;
     assign_m_m : 'a -> 'a -> 'a; col_vector : int -> 'b; copy : 'a;
     get_cm : c_matrix; identity : 'a; lup_decompose : 'b; lup_get_l : 'a;
     lup_get_u : 'a; lup_inverse : 'a; lup_invert : 'a; ncols : int;
     nrows : int; repr : 'a; row_vector : int -> 'b; scale : float -> 'a;
     set : int -> int -> float -> 'a; transpose : 'a >
   as 'a)
val matrix_x_matrix :
  (< copy : < assign_m_m : 'a -> 'b -> 'c; .. >; .. > as 'a) -> 'b -> 'c

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
    val repr : t -> string
  end

val mkquaternion : Quaternion.t
val mkquaternion_rijk : float -> float -> float -> float -> Quaternion.t
