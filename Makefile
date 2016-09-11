
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

gauss1: prog
	./prog -n 12 -i images/IMG_1664.JPG --filter='glsl:intensity_from_rgb(0,1)'     \
	                    --filter='glsl:gauss_x9(1,2)' --filter='glsl:gauss_y9(2,3)' \
	                    --filter='glsl:harris(3,4)'  \
	                    --filter='save:test_a.png(4)'


blah: prog
	./prog -n 3 -i images/IMG_1854.JPG --filter='glsl:intensity_from_rgb(0,1)' --filter='glsl:gauss_x9(1,2)' --filter='glsl:gauss_y9(2,1)' --filter='glsl:harris(1,2)' --filter='save:test.png(2)' --filter='find:a(2)' --filter='corr:correlation_copy_shader(0,3)' --filter='save:test2.png(3)' 

blah2: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG --filter='glsl:intensity_from_rgb(0,2)' --filter='glsl:intensity_from_rgb(1,8)' \
	                    --filter='glsl:gauss_x9(2,3)' --filter='glsl:gauss_y9(3,4)'  --filter='save:test_a.png(4)'  \
	                    --filter='glsl:gauss_x9(8,9)' --filter='glsl:gauss_y9(9,10)' --filter='save:test_b.png(10)'  \
	                    --filter='glsl:harris(4,5)'   --filter='save:test_h.png(5)'  --filter='find:a(5)' --filter='corr:correlation_copy_shader(2,6)'  --filter='save:test_corr.png(6)' \
	                    --filter='glsl:sum_sq_pixel_diff(6,2,11)' --filter='save:test_bdb.png(11)'  --filter='find:a(5)'  \
	                    --filter='glsl:sum_sq_pixel_diff(6,8,11)' --filter='save:test_bda.png(11)'  --filter='find:a(5)' 

blah2b: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG --filter='glsl:intensity_from_rgb(0,2)&-DNUM_WEIGHTS=3&fred=1&joe=2' --filter='glsl:intensity_from_rgb(1,8)' \
						--filter='glsl:intensity_from_rgb(0,4)' --filter='glsl:intensity_from_rgb(1,10)' \
	                    --filter='glsl:harris(4,5)'   --filter='save:test_h.png(5)'  --filter='find:a(5)' --filter='corr:correlation_copy_shader(2,6)'  --filter='save:test_corr.png(6)' \
	                    --filter='glsl:sum_sq_pixel_diff(6,2,11)' --filter='save:test_bdb.png(11)'  --filter='find:a(5)'  \
	                    --filter='glsl:sum_sq_pixel_diff(6,8,11)' --filter='save:test_bda.png(11)'  --filter='find:a(5)' 

blah3: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG --filter='glsl:intensity_from_rgb(0,2)' --filter='glsl:intensity_from_rgb(1,8)' \
	                    --filter='glsl:gauss_x9(2,3)' --filter='glsl:gauss_y9(3,4)'  --filter='save:test_a.png(4)'  \
	                    --filter='glsl:gauss_x9(8,9)' --filter='glsl:gauss_y9(9,10)' --filter='save:test_b.png(10)'  \
	                    --filter='glsl:harris(4,5)'   --filter='save:test_h.png(5)'  --filter='find:a(5)' --filter='corr:correlation_copy_circle(2,6)'  --filter='save:test_corr.png(6)'

test: gauss harris

gauss: prog
	./prog -n 4 -i images/IMG_1854.JPG --filter='glsl:intensity_from_rgb(0,1)' \
	                    --filter='glsl:gauss(1,2)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(3)'

harris: prog
	./prog -n 4 -i images/IMG_1854.JPG --filter='glsl:intensity_from_rgb(0,1)' \
	                    --filter='glsl:harris(1,2)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_a.png(2)'

clean:
	rm *.o prog

