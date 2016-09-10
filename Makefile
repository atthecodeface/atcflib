
all: prog


OS := $(shell uname)

LINK      = g++
LINKFLAGS = -g -lGL -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
CPPFLAGS  = -g -Wall -I/usr/include/SDL2

ifeq ($(OS),Darwin)
GLM = ../glm
LINK      = c++
LINKFLAGS = -g -iframework /Library/Frameworks -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework OpenGL
CPPFLAGS  = -g -Wall -I$(GLM) -iframework /Library/Frameworks -I/Library/Frameworks/SDL2.framework/Headers -I/Library/Frameworks/SDL2_image.framework/Headers -I/Library/Frameworks/SDL2_ttf.framework/Headers
endif

OBJS = main.o

prog: $(OBJS)
	$(LINK) $(OBJS) $(LINKFLAGS) -o prog

blah: prog
	./prog -n 3 -i images/IMG_1664.JPG --filter='glsl:intensity_from_rgb(0,1)' --filter='glsl:gauss_x9(1,2)' --filter='glsl:gauss_y9(2,1)' --filter='glsl:harris(1,2)' --filter='save:test.png(2)' --filter='find:a(2)' --filter='corr:correlation_copy_shader(0,3)' --filter='save:test2.png(3)' 

clean:
	rm *.o prog

