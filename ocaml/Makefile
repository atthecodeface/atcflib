

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

OCAML_WRAPPER_SRCS := ocaml_timer.cpp ocaml_vector.cpp ocaml_matrix.cpp ocaml_quaternion.cpp
OCAML_WRAPPER_OBJS := $(patsubst %.cpp,build/%.o, ${OCAML_WRAPPER_SRCS} )
#$(foreach me,$3,$(eval $(call nffw.add_obj,$1,$2,$(me))))

all: camltime

.PHONY: atcflib
atcflib: atcflib.cmxa

build/%.o: %.cpp
	c++ -I ${OCAML_INCLUDE} -I ~/Git/atcflib -c $< -o $@

atcflib.cmxa: ${OCAML_WRAPPER_OBJS} ../libatcf.so atcflib.ml atcflib.mli
	# The next one creates atcflib.cmi, atcflib.cmx, atcflib.o
	$(OCAMLFIND) ocamlopt  -linkpkg -package core -c atcflib.mli -thread 
	@echo "Compile atcflib.ml"
	@$(OCAMLFIND) ocamlopt  -linkpkg -package core -c atcflib.ml -thread 
	# The next one creates atcflib.cmxa
	@echo "Build atcflib.cmxa"
	@$(OCAMLFIND) ocamlopt -a -o atcflib.cmxa atcflib.cmx ${OCAML_WRAPPER_OBJS} -cclib -L$(LIBDIR) -cclib -latcf -cclib -lc++

atfcml: atcflib.cmxa
	# Make toplevel ML
	ocamlmktop -custom -o atcfml atcflib.ml  ${OCAML_WRAPPER_OBJS} -cclib -L$(LIBDIR) -cclib -L. -cclib -latcf  -cclib -lc++

./test_atcf.native: atcflib.cmxa test_atcf.ml
	# The next one creates test_atcf.o, test_atcf.native, test_atcf.cmx, test_atcf.cmi
	$(OCAMLFIND) ocamlopt -linkpkg -package ounit,bigarray atcflib.cmxa test_atcf.ml -o test_atcf.native

./test_time.native: atcflib.cmxa test_time.ml
	@echo "Compile test_time.ml with atcflib.cmxa"
	@$(OCAMLFIND) ocamlopt -linkpkg -package ounit,bigarray atcflib.cmxa test_time.ml -o test_time.native

camltest: ./test_atcf.native
	# Run it...
	${LIBPATH} ./test_atcf.native -help
	${LIBPATH} ./test_atcf.native -list-test
	${LIBPATH} ./test_atcf.native 
#	$(OCAMLFIND) ocamlopt -linkpkg -package tgls,tgls.tgl4,tsdl camltest2.ml camltestlib.cmxa -o camltest2.native

camltime: ./test_time.native
	@echo "The time code no longer compares different methods of invocation as the atcflib is cleaned up"
	${LIBPATH} ./test_time.native 

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
	rm build/*
	rm -f *.native *.cmi *.cmx *.cmxa *.o doc/html/*

