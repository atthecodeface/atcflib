

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

camltest:
	cp ../libatcf.so .
	c++ -I ${OCAML_INCLUDE} -I ~/Git/atcflib -c ocaml_vector.cpp
	c++ -I ${OCAML_INCLUDE} -I ~/Git/atcflib -c ocaml_matrix.cpp
	c++ -I ${OCAML_INCLUDE} -I ~/Git/atcflib -c ocaml_quaternion.cpp
	# The next one creates atcflib.cmi, atcflib.cmx, atcflib.o
	$(OCAMLFIND) ocamlopt -c atcflib.ml 
	# The next one creates atcflib.a
	$(OCAMLFIND) ocamlopt -a -o atcflib.cmxa atcflib.cmx ocaml_vector.o ocaml_matrix.o ocaml_quaternion.o -cclib -latcf -cclib -L$(LIBDIR) -cclib -lc++
	# The next one creates test_atcf.o, test_atcf.native, test_atcf.cmx, test_atcf.cmi
	$(OCAMLFIND) ocamlopt -linkpkg -package ounit atcflib.cmxa test_atcf.ml -o test_atcf.native
	# Make toplevel ML
	ocamlmktop -custom -o atcfml atcflib.ml ocaml_vector.o ocaml_matrix.o ocaml_quaternion.o -cclib -L. -cclib -latcf  -cclib -lc++
	# Run it...
	LD_LIBRARY_PATH=.:/Users/gavinprivate/Git/atcflib ./test_atcf.native -help
	LD_LIBRARY_PATH=.:/Users/gavinprivate/Git/atcflib ./test_atcf.native -list-test
	LD_LIBRARY_PATH=.:/Users/gavinprivate/Git/atcflib ./test_atcf.native 
#	$(OCAMLFIND) ocamlopt -linkpkg -package tgls,tgls.tgl4,tsdl camltest2.ml camltestlib.cmxa -o camltest2.native

.PHONY: doc
doc:
	ocamldoc -html -d doc/html atcflib.ml

clean:
	mkdir -p doc/html
	rm -f *.native *.cmi *.cmx *.cmxa *.o doc/html/*

