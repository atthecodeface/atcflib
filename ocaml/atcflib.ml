type c_vector
external v_create  : int -> c_vector   = "atcf_vector_create"
external v_clone   : c_vector -> c_vector   = "atcf_vector_clone"
external v_destroy : c_vector -> unit  = "atcf_vector_destroy"
external v_modulus : c_vector -> float = "atcf_vector_modulus"
external v_modulus_squared : c_vector -> float = "atcf_vector_modulus_squared"
external v_assign  : c_vector -> c_vector -> unit = "atcf_vector_assign"
external v_normalize : c_vector -> unit = "atcf_vector_normalize"
external v_length  : c_vector -> int = "atcf_vector_length"
external v_coords  : c_vector -> float array  = "atcf_vector_coords"
external v_set     : c_vector -> int -> float -> unit  = "atcf_vector_set"
external v_scale   : c_vector -> float -> unit  = "atcf_vector_scale"
external v_add_scaled    : c_vector -> c_vector -> float -> unit  = "atcf_vector_add_scaled"
external v_dot_product   : c_vector -> c_vector -> float  = "atcf_vector_dot_product"
external v_cross_product : c_vector -> c_vector -> c_vector  = "atcf_vector_cross_product3"
external v_angle_axis_to : c_vector -> c_vector -> (c_vector * float * float)  = "atcf_vector_angle_axis_to3"

let log = Printf.printf

(*class type __vector_type*)

class __vector (cv:c_vector) =
  object (self)
         val v = cv
         initializer
           Gc.finalise (fun self -> self#destroy ()) self
         method destroy () =
           log "destroying vector %d\n" (Oo.id self) ;
           v_destroy v
         method get_cv      = v
         method copy       = new __vector(v_clone v)
         method coords     = v_coords v
         method length     = v_length v
         method set n f    = v_set v n f ; self
         method assign (v2:__vector)  = v_assign v v2#get_cv ; self
         method scale f    = v_scale v f ; self
         method modulus    = v_modulus v
         method modulus_squared   = v_modulus_squared v
         method add (v2:__vector)    = v_add_scaled v v2#get_cv 1.0 ; self
         method add_scaled (v2:__vector) f = v_add_scaled v v2#get_cv f ; self
         method normalize     = v_normalize v ; self
         method dot_product (v2:__vector) = v_dot_product v v2#get_cv
         method cross_product3 (v2:__vector) = new __vector(v_cross_product v v2#get_cv)
         method angle_axis_to3 (v2:__vector) = let (va,c,s) = v_angle_axis_to v v2#get_cv in (new __vector(va),c,s)
         method repr = let f i c = (Printf.printf "%d:%f " i c) in Array.iteri f (v_coords v)  ; self
  end ;;

let vector n =
  new __vector (v_create n)
let vector2 c0 c1 =
  ((new __vector (v_create 2))#set 0 c0)#set 1 c1
let vector3 c0 c1 c2 =
  (((new __vector (v_create 3))#set 0 c0)#set 1 c1)#set 2 c2
let vector4 c0 c1 c2 c3 =
  ((((new __vector (v_create 4))#set 0 c0)#set 1 c1)#set 2 c2)#set 3 c3
