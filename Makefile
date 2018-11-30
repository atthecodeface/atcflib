OS := $(shell uname)

VERBOSE_FLAGS = 
LINK      = g++
LINKFLAGS = -g -lGL -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
CPPFLAGS  = -g -Wall -I/usr/include/SDL2 $(VERBOSE_FLAGS)

ifeq ($(OS),Darwin)
GLM = ../glm
LINK      = c++
LINKFLAGS = -g -iframework /Library/Frameworks -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework OpenGL -lpng16  -ljpeg -L/usr/local/lib 
CPPFLAGS  = -std=c++11 -DGLM_FORCE_RADIANS -DGL_GLEXT_PROTOTYPES -g -Wall -I$(GLM) -iframework /Library/Frameworks -I/Library/Frameworks/SDL2.framework/Headers -I/Library/Frameworks/SDL2_image.framework/Headers -I/Library/Frameworks/SDL2_ttf.framework/Headers -I/usr/local/include $(VERBOSE_FLAGS)
endif

ATCF_INCLUDE := include

C_LIBRARIES := libatcf

include Makefile_c
include Makefile_python
include Makefile_apps
include Makefile_ocaml
