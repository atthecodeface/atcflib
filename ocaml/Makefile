

OPAMROOT = ~/.opam/system/bin/
OPAMLIB = ~/.opam/system/lib
BREWROOT = ~/Git/brew/
OCAMLROOT = $(BREWROOT)/bin/
#OCAMLOCAML can come from ocamlc -where
OCAMLOCAML = $(BREWROOT)/lib/ocaml

SETPATH := PATH=$(OCAMLROOT):$(OPAMROOT):$(PATH) 
OCAMLFIND  = $(SETPATH) $(OPAMROOT)/ocamlfind
OCAMLBUILD = $(SETPATH) $(OPAMROOT)/ocamlbuild
OCAMLC     = $(SETPATH) $(OCAMLROOT)/ocamlc

OCAML_INCLUDE = $(OCAMLOCAML)

LIBDIR := /Users/gavinprivate/Git/atcflib
LIBPATH := DYLD_LIBRARY_PATH=.:/Users/gavinprivate/Git/atcflib

OCAML_WRAPPER_SRCS := ocaml_timer.cpp ocaml_vector.cpp ocaml_matrix.cpp ocaml_quaternion.cpp ocaml_bunzip.cpp
OCAML_WRAPPER_OBJS := $(patsubst %.cpp,build/%.o, ${OCAML_WRAPPER_SRCS} )
#$(foreach me,$3,$(eval $(call nffw.add_obj,$1,$2,$(me))))

all: camltime

.PHONY: atcflib
atcflib: atcflib.cmxa

build/%.o: %.cpp
	c++ -I ${OCAML_INCLUDE} -I ~/Git/atcflib -c $< -o $@

.PHONY:show_mli
show_mli:
	ocamlc -i atcflib.ml

ocaml_atcflib.a: ${OCAML_WRAPPER_OBJS} ../libatcf.so
	ld -undefined dynamic_lookup -dylib -o ocaml_atcflib.a  ${OCAML_WRAPPER_OBJS} -L$(LIBDIR) -latcf -lc++
	@echo "Build atcflib.cmxa"

atcflib.cmxa: ocaml_atcflib.a ../libatcf.so atcflib.ml atcflib.mli
	# The next one creates atcflib.cmi, atcflib.cmx, atcflib.o
	$(OCAMLFIND) ocamlopt  -linkpkg -package core,re -c atcflib.mli -thread 
	@echo "Compile atcflib.ml"
	@$(OCAMLFIND) ocamlopt  -linkpkg -package core,re -c atcflib.ml -thread 
	# The next one creates atcflib.cmxa
	@echo "Build atcflib.cmxa"
	#$(OCAMLFIND) ocamlopt -a -o atcflib.cmxa atcflib.cmx ${OCAML_WRAPPER_OBJS} -cclib -L$(LIBDIR) -cclib -latcf -cclib -lc++
	@$(OCAMLFIND) ocamlopt -a -o atcflib.cmxa atcflib.cmx ocaml_atcflib.a

atcfml: atcflib.cmxa
	# Make toplevel ML
	$(OCAMLFIND) ocamlmktop -custom -linkpkg -package ounit,re,result,bigarray,tgls,tgls.tgl4,tsdl -o atcfml atcflib.ml  ${OCAML_WRAPPER_OBJS} -cclib -L$(LIBDIR) -cclib -L. -cclib -latcf  -cclib -lc++

./test_math.native: atcflib.cmxa test_math.ml
	# The next one creates test_math.o, test_math.native, test_math.cmx, test_math.cmi
	$(OCAMLFIND) ocamlopt -linkpkg -package ounit,re,bigarray atcflib.cmxa test_math.ml -o test_math.native

./test_time.native: atcflib.cmxa test_time.ml
	@echo "Compile test_time.ml with atcflib.cmxa"
	@$(OCAMLFIND) ocamlopt -linkpkg -package ounit,re,bigarray atcflib.cmxa test_time.ml -o test_time.native

./test_compression.native: atcflib.cmxa test_compression.ml
	@echo "Compile test_compression.ml with atcflib.cmxa"
	@$(OCAMLFIND) ocamlopt -linkpkg -package ounit,re,bigarray atcflib.cmxa test_compression.ml -o test_compression.native

.PHONY: bunzip_index
bunzip_index: ./bunzip_index.native

./bunzip_index.native: atcflib.cmxa bunzip_index.ml
	@echo "Compile bunzip_index.ml with atcflib.cmxa"
	@$(OCAMLFIND) ocamlopt -linkpkg -package bigarray,re atcflib.cmxa bunzip_index.ml -o bunzip_index.native

camltest: ./test_math.native
	# Run it...
	${LIBPATH} ./test_math.native -help
	${LIBPATH} ./test_math.native -list-test
	${LIBPATH} ./test_math.native 
#	$(OCAMLFIND) ocamlopt -linkpkg -package tgls,tgls.tgl4,tsdl camltest2.ml camltestlib.cmxa -o camltest2.native

camltime: ./test_time.native
	@echo "The time code no longer compares different methods of invocation as the atcflib is cleaned up"
	${LIBPATH} ./test_time.native 

camlcomp: ./test_compression.native
	${LIBPATH} ./test_compression.native 

quaternion_play_int: quaternion_play.ml atcfml
	DYLD_LIBRARY_PATH=`pwd`/.. ./atcfml -I ~/Git/brew/lib/ocaml -I ~/.opam/system/lib/tgls -I ~/.opam/system/lib/tsdl/ -I ~/Git/brew/lib/ocaml -I ~/.opam/system/lib/result ./quaternion_play.ml

quaternion_play: quaternion_play.ml 
	$(OCAMLFIND) ocamlopt -linkpkg -package ounit,bigarray,re,tgls,tgls.tgl4,tsdl atcflib.cmxa quaternion_play.ml -o quaternion_play.native
	${LIBPATH} ./quaternion_play.native

.PHONY: release_test
release_test:
	(cd ..; make clean; make)
	make clean
	make camltime camltest doc

.PHONY: doc
doc:
	mkdir -p doc/html
	ocamldoc -html -d doc/html atcflib.mli

clean:
	mkdir -p doc/html
	mkdir -p build
	rm -f build/*
	rm -f *.native *.cmi *.cmx *.cmxa *.o doc/html/*

