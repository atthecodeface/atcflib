(** Copyright (C) 2017,  Gavin J Stark.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          quaternion_play.ml
 * @brief         Playing with quaternions
 *
 *)

open Atcflib
open Tsdl
open Tgl4
open Tsdl
open Tgl4
open Result
type ogl_result             = (unit, [ `Msg of string ]) Result.result


let mk = Quaternion.make_rijk

let str q = Quaternion.repr q

let add    q1 q2 = Quaternion.(add_scaled (copy q1) q2 1.0);;
let mult   q1 q2 = Quaternion.(postmultiply (copy q1) q2);;
let divide q1 q2 = Quaternion.(premultiply (reciprocal (copy q2)) q1);;

let zero = mk 0.0 0.0 0.0 0.0
let one = mk 1.0 0.0 0.0 0.0
let ptoner = mk 0.1 0.0 0.0 0.0
let ptonei = mk 0.0 0.1 0.0 0.0
let ptoneir = mk 0.1 0.1 0.0 0.0
let ptoneij = mk 0.0 0.1 0.1 0.0 
let ptoneijk = mk 0.0 0.1 0.1 0.1
let dh = 0.01
let dhr = mk dh 0. 0. 0.
let dhi = mk 0. dh 0. 0.
let dhj = mk 0. 0. dh 0.
let dhk = mk 0. 0. 0. dh

let q2 q = mult q q ;;
let q3 q = mult (mult q q) q ;;
let q4 q = mult (mult q q) (mult q q) ;;

let div qh f = let (q, dq) = qh in (q/f, dq/f)
let qh q dh  = (q, one)
let qh2 q dh = ((q2 q), (divide (add (mult q dh) (mult dh q)) dh))
let qh3 q dh =
    let qdh_dhq = (add (mult q dh) (mult dh q)) in 
    ((q3 q), (divide (add (mult q qdh_dhq) (mult dh (q2 q))) dh))

let qh4 q dh =
    let qdh_dhq = (add (mult q dh) (mult dh q)) in 
    ((q4 q), (divide (add (mult (q2 q) qdh_dhq) (mult qdh_dhq (q2 q))) dh))

;;

let addh qdh1 qdh2 =
     let (q1,dq1)=qdh1 in
     let (q2,dq2)=qdh2 in
     ((add q1 q2),(add dq1 dq2))

let divh qdh r =
     let (q,dq)=qdh in
     ( (Quaternion.scale q  (1. /. r)),
       (Quaternion.scale dq (1. /. r)) )

let e q dh = (addh (one,zero) (divh (addh (qh q dh) 
                      (divh (addh (qh2 q dh) 
                      (divh (addh (qh3 q dh) 
                                  (divh (qh4 q dh) 4.)
                           ) 3. )
                           ) 2. )
                           ) 1. )
             )
;;

let pqdh qdh =
   let (q,dq)=qdh in
   Printf.printf "q:%s\nq':%s\n" (str q) (str dq) ;
   ()
;;

(*
pqdh (e zero dhr) ;
pqdh (e zero dhi) ;
pqdh (e zero dhj) ;
pqdh (e zero dhk) ;

pqdh (e ptoner dhr) ;
pqdh (e ptoner dhi) ;
pqdh (e ptoner dhj) ;
pqdh (e ptoner dhk) ;

pqdh (e ptonei dhr) ;
pqdh (e ptonei dhi) ;
pqdh (e ptonei dhj) ;
pqdh (e ptonei dhk) ;

pqdh (e ptoneir dhr) ;
pqdh (e ptoneir dhi) ;
pqdh (e ptoneir dhj) ;
pqdh (e ptoneir dhk) ;

pqdh (e ptoneij dhr) ;
pqdh (e ptoneij dhi) ;
pqdh (e ptoneij dhj) ;
pqdh (e ptoneij dhk) ;

pqdh (e ptoneijk dhr) ;
pqdh (e ptoneijk dhi) ;
pqdh (e ptoneijk dhj) ;
pqdh (e ptoneijk dhk) ;

 *)

(* >>= is an infix function, as it starts with one of the infix-chars
          =<>@^|&+-*/$%
*)
let str = Printf.sprintf

let ( >>= ) x f = match x with Ok v -> f v | Error _ as e -> e

(* Helper functions. *)
open Bigarray

let ba_floats fs = Bigarray.(Array1.of_array float32 c_layout fs)
let ba_uint8s is = Bigarray.(Array1.of_array int8_unsigned c_layout is)

let bigarray_create k len = Bigarray.(Array1.create k c_layout len)

let get_int =
  let a = bigarray_create Bigarray.int32 1 in
  fun f -> f a; Int32.to_int a.{0}

let get_string len f =
  let a = bigarray_create Bigarray.char len in
  f a; Gl.string_of_bigarray a

(* Shaders *)

let glsl_version gl_version = match gl_version with
| 3,2 -> "150" | 3,3 -> "330"
| 4,0 -> "400" | 4,1 -> "410" | 4,2 -> "420" | 4,3 -> "430" | 4,4 -> "440"
| _ -> assert false

;;

let vertex_shader v = str "
  #version %s core
  in vec3 vertex;
  in vec3 color;
  uniform float spin;
  out vec4 v_color;
mat3 rotz(in float ang)
{
    mat3 m;
    m =  mat3( vec3(cos(ang),-sin(ang),0.0),
              vec3(sin(ang),cos(ang),0.0),
              vec3(0.0,0.0,1.0) );
    return m;
}
mat3 rotx(in float ang)
{
    mat3 m;
    m = mat3( vec3(1.0,0.0,0.0),
              vec3(0.0, cos(ang),-sin(ang)),
              vec3(0.0,sin(ang),cos(ang)) );
    return m;
}
mat3 roty(in float ang)
{
    mat3 m;
    m =  mat3( vec3(cos(ang),0.0,-sin(ang)),
                  vec3(0.0,1.0,0.0),
                  vec3(sin(ang),0.0,cos(ang)) );
    return m;
}
  void main()
  {
    mat3 m;
    vec3 v;
    float dz;
    float ang2;
    float ang=spin/300.0;
    float brightness = 0.5;
    ang2 = -2.0;
    v = vec3(vertex.x-0.5, vertex.z-0.5, 0.5-vertex.y);
    float light_z = spin/30.0;
    light_z = (light_z-floor(light_z));
    dz = vertex.z-light_z;
    brightness = brightness + 0.01 / (dz*dz);
    m = rotx(0.5) * roty(ang2+ang);
    v_color = vec4(color*brightness, 1.0);
    gl_Position = vec4(m*v, 1.0);
  }" v

let fragment_shader v = str "
  #version %s core
  in vec4 v_color;
  out vec4 color;
  void main() { color = v_color; }" v

(* OpenGL setup *)

let compile_shader src typ =
  let get_shader sid e = get_int (Gl.get_shaderiv sid e) in
  let sid = Gl.create_shader typ in
  Gl.shader_source sid src;
  Gl.compile_shader sid;
  if get_shader sid Gl.compile_status = Gl.true_ then Ok sid else
  let len = get_shader sid Gl.info_log_length in
  let log = get_string len (Gl.get_shader_info_log sid len None) in
  (Gl.delete_shader sid; Error (`Msg log))

let create_program glsl_v =
  compile_shader (vertex_shader glsl_v) Gl.vertex_shader >>= fun vid ->
  compile_shader (fragment_shader glsl_v) Gl.fragment_shader >>= fun fid ->
  let pid = Gl.create_program () in
  let get_program pid e = get_int (Gl.get_programiv pid e) in
  Gl.attach_shader pid vid; Gl.delete_shader vid;
  Gl.attach_shader pid fid; Gl.delete_shader fid;
  Gl.bind_attrib_location pid 0 "vertex";
  Gl.bind_attrib_location pid 1 "color";
  Gl.link_program pid;
  if get_program pid Gl.link_status = Gl.true_ then Ok pid else
  let len = get_program pid Gl.info_log_length in
  let log = get_string len (Gl.get_program_info_log pid len None) in
  (Gl.delete_program pid; Error (`Msg log))

let delete_program pid =
  Gl.delete_program pid; Ok ()

let draw pid objs win =
  Gl.clear_color 0. 0. 0. 1.;
  Gl.clear Gl.color_buffer_bit;
  Gl.use_program pid;
  let draw_obj o = o#draw in List.iter draw_obj objs;
  Gl.bind_vertex_array 0;
  Sdl.gl_swap_window win;
  Ok ()

let reshape win w h =
  Gl.viewport 0 0 w h

(* Window and OpenGL context *)

let pp_opengl_info ppf () =
  let pp = Format.fprintf in
  let pp_opt ppf = function None -> pp ppf "error" | Some s -> pp ppf "%s" s in
  pp ppf "@[<v>@,";
  pp ppf "Renderer @[<v>@[%a@]@," pp_opt (Gl.get_string Gl.renderer);
  pp ppf "@[OpenGL %a / GLSL %a@]@]@,"
    pp_opt (Gl.get_string Gl.version)
    pp_opt (Gl.get_string Gl.shading_language_version);
  pp ppf "@]"

let create_window ~gl:(maj, min) =
  let w_atts = Sdl.Window.(opengl + resizable) in
  let w_title = Printf.sprintf "OpenGL %d.%d (core profile)" maj min in
  let set a v = Sdl.gl_set_attribute a v in
  set Sdl.Gl.context_profile_mask Sdl.Gl.context_profile_core >>= fun () ->
  set Sdl.Gl.context_major_version maj                        >>= fun () ->
  set Sdl.Gl.context_minor_version min                        >>= fun () ->
  set Sdl.Gl.doublebuffer 1                                   >>= fun () ->
  Sdl.create_window ~w:640 ~h:480 w_title w_atts              >>= fun win ->
  Sdl.gl_create_context win                                   >>= fun ctx ->
  Sdl.gl_make_current win ctx                                 >>= fun () ->
  Sdl.log "%a" pp_opengl_info ();
  Ok (win, ctx)

let destroy_window win ctx =
  Sdl.gl_delete_context ctx;
  Sdl.destroy_window win;
  Ok ()

(* Event loop *)

let event_loop win draw =
  let e = Sdl.Event.create () in
  let key_scancode e = Sdl.Scancode.enum Sdl.Event.(get e keyboard_scancode) in
  let event e = Sdl.Event.(enum (get e typ)) in
  let window_event e = Sdl.Event.(window_event_enum (get e window_event_id)) in
  let rec loop () =
    (* Sdl.wait_event (Some e) >>= fun () -> *)
    let idle = Sdl.poll_event (Some e) in
        if idle then begin
        match event e with
        | `Quit -> Ok ()
        | `Key_down when key_scancode e = `Escape -> Ok ()
        | `Window_event ->
           begin match window_event e with
           | `Exposed | `Resized ->
              let w, h = Sdl.get_window_size win in
              reshape win w h;
              draw win;
              loop ()
           | _ -> loop ()
           end
        | _ -> loop ()
      end
    else
      begin
        draw win ;
        Sdl.delay (Int32.of_int 10);
        loop ()
      end
  in
  (draw win; loop ())

(* Main *)

let tri ~gl:(maj, min as gl) objs =
  Sdl.init Sdl.Init.video           >>= fun () ->
  create_window ~gl                 >>= fun (win, ctx) ->
  create_program (glsl_version gl)  >>= fun pid ->
  let draw_obj o = ignore (o#create_geometry pid) in List.iter draw_obj objs; Ok ()>>= fun () ->
  event_loop win (draw pid objs)     >>= fun () ->
  delete_program pid                >>= fun () ->
  let draw_obj o = ignore (o#delete_geometry) in List.iter draw_obj objs; Ok ()>>= fun () ->
  destroy_window win ctx            >>= fun () ->
  Sdl.quit ();
  Ok ()

module Objects = struct

let bigarray_create k len = Bigarray.(Array1.create k c_layout len)

let get_int =
  let a = bigarray_create Bigarray.int32 1 in
  fun f -> f a; Int32.to_int a.{0}

let set_int =
  let a = bigarray_create Bigarray.int32 1 in
  fun f i -> a.{0} <- Int32.of_int i; f a

(* Geometry *)

let create_buffer b =
  let id = get_int (Gl.gen_buffers 1) in
  let bytes = Gl.bigarray_byte_size b in
  Gl.bind_buffer Gl.array_buffer id;
  Gl.buffer_data Gl.array_buffer bytes (Some b) Gl.static_draw;
  id

let delete_buffer bid =
  set_int (Gl.delete_buffers 1) bid

type uint8_bigarray    = (int,   Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t
type float32_bigarray  = (float, Bigarray.float32_elt,       Bigarray.c_layout) Bigarray.Array1.t
type ogl_result        = (unit, [ `Msg of string ]) Result.result

class virtual opengl_object  =
    object (self)
      val mutable gid = -1
      val mutable bids = [-1; -1; -1]
      val mutable spin = 50.0
      val mutable spin_uid = -1
      val mutable num_triangles = 1
      method draw_internal d =
        let bind_attrib id loc dim typ =
          Gl.bind_buffer Gl.array_buffer id;
          Gl.enable_vertex_attrib_array loc;
          Gl.vertex_attrib_pointer loc dim typ false 0 (`Offset 0);
        in
        (*Sdl.log "draw";*)
        spin <- spin +. -0.1;
        Gl.uniform1f spin_uid spin;

        Gl.bind_vertex_array gid;
        Gl.bind_buffer Gl.element_array_buffer (List.nth bids 0); (* iid *)
        bind_attrib (List.nth bids 1) 0 3 Gl.float; (* vid *)
        bind_attrib (List.nth bids 2) 1 3 Gl.float; (* cid *)

        d () ;
        Gl.bind_buffer Gl.array_buffer 0;
        Gl.bind_buffer Gl.element_array_buffer 0;
        Gl.bind_vertex_array 0

      method draw =
        let d _ = 
           Gl.draw_elements Gl.triangles (num_triangles*3) Gl.unsigned_byte (`Offset 0)
        in self#draw_internal d

      method virtual create_geometry : int -> ogl_result
      method private create_geometry_from_indices (pid:int)
                                                  (indices:uint8_bigarray)
                                                  (vertices:float32_bigarray)
                                                  (colors:float32_bigarray)
                     : ogl_result  = 
        spin_uid <- Gl.get_uniform_location pid "spin";
        gid <- get_int (Gl.gen_vertex_arrays 1);
        let iid = create_buffer indices in
        let vid = create_buffer vertices in
        let cid = create_buffer colors in
        bids <- [iid;  vid;  cid] ;
        Ok ()
      method delete_geometry : ogl_result = 
        set_int (Gl.delete_vertex_arrays 1) gid;
        List.iter delete_buffer bids;
        Ok ()
    end

class axes =
    object (self)
      inherit opengl_object as super
      method create_geometry pid =
        let axis_vertices = ba_floats [| 0.; 0.; 0.;
                                         1.; 0.; 0.;
                                         0.; 1.; 0.;
                                         0.; 0.; 1.;|]
        in
        let axis_colors = ba_floats [|1.0; 1.0; 1.0;
                                      1.0; 0.0; 0.0;
                                      0.0; 1.0; 0.0;
                                      0.0; 0.0; 1.0;|]
        in
        let axis_indices = ba_uint8s [| 0; 1; 0; 2; 0; 3; |]
        in
      super#create_geometry_from_indices pid axis_indices axis_vertices axis_colors
      method draw =
        let d _ = 
           Gl.draw_elements Gl.lines 6 Gl.unsigned_byte (`Offset 0)
        in self#draw_internal d
    end

class pt_dir_marker pt dir =
    object (self)
      val x = pt.(0)
      val y = pt.(1)
      val z = pt.(2)
      val dx = dir.(0)
      val dy = dir.(1)
      val dz = dir.(2)
      inherit opengl_object as super
      method create_geometry pid =
        let axis_vertices = ba_floats [| x; y; z;
                                         x+.dx ; y+.dy; z;
                                         x+.dx ; y+.dy; z+.dz; |]
        in
        let axis_colors = ba_floats [| 0.; 0.; 1.;
                                       0.5; 0.; 1.;
                                       1.; 0.; 0.; |]
        in
        let axis_indices = ba_uint8s [| 0; 1; 1; 2; 0; 2; |]
        in
        super#create_geometry_from_indices pid axis_indices axis_vertices axis_colors
      method draw =
        let d _ = 
           Gl.draw_elements Gl.lines 6 Gl.unsigned_byte (`Offset 0)
        in self#draw_internal d
    end

end

let obj_of_rijk r i j k =
let q = mk r i j k in
let (g, gh) = e q dhr in
let rijk = Quaternion.get_rijk g in
new Objects.pt_dir_marker [|i;j;k|] [|rijk.(1)*.0.1;rijk.(2)*.0.1;rijk.(3)*.0.1;|]

let main () =
  let exec = Filename.basename Sys.executable_name in
  let usage = str "Usage: %s [OPTION]\n Tests Tgl4.\nOptions:" exec in
  let minor = ref 0 in
  let object_n = ref 0 in
  let options =
    [ "-object", Arg.Set_int object_n,
      " <x> use specify object number";
      "-minor", Arg.Set_int minor,
      " <x> use Use an OpenGL 4.x context (defaults to 4.0)"; ]
  in
  let anon _ = raise (Arg.Bad "no arguments are supported") in
  Arg.parse (Arg.align options) anon usage;
  let (is,js,ks) = (0.1,0.1,0.1) in
  let rec build_objs i j k l =
    if (i+.is)>1.01 then begin
      if (j+.js)>1.01 then begin
        if (k+.ks)>1.01 then l
        else build_objs 0. 0. (k+.ks) (l @ [obj_of_rijk 0. i  j k])
      end
      else build_objs 0. (j+.js) k (l @ [obj_of_rijk 0. i  j k])
    end
    else build_objs (i+.is) j k (l @ [obj_of_rijk 0. i  j k])
    in
  let objs = build_objs 0. 0. 0. [(new Objects.axes)] in
  match tri ~gl:(4, !minor) objs with
  | Ok () -> exit 0
  | Error (`Msg msg) -> Sdl.log "%s@." msg; exit 1

let () = main ()

