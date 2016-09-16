
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

OBJS = main.o key_value.o texture.o shader.o filter.o

prog: $(OBJS)
	$(LINK) $(OBJS) $(LINKFLAGS) -o prog

blah2: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:intensity_from_rgb(0,2)&-DSELECTED_VALUE=r' \
						--filter='glsl:intensity_from_rgb(1,8)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(3,4)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(4)'  \
	                    --filter='glsl:gauss(8,9)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(9,10)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_b.png(10)'  \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_copy_shader(2,6)'  --filter='save:test_corr.png(6)' \
	                    --filter='glsl:sum_sq_pixel_diff(6,2,11)' --filter='save:test_ada.png(11)'  --filter='find:a(11)'  \
	                    --filter='glsl:sum_sq_pixel_diff(6,8,11)' --filter='save:test_bda.png(11)'  --filter='find:a(11)' 

blah2b: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:intensity_from_rgb(0,4)&-DSELECTED_VALUE=r' \
						--filter='glsl:intensity_from_rgb(1,10)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_copy_shader(4,6)'  --filter='save:test_corr.png(6)' \
	                    --filter='glsl:sum_sq_pixel_diff(6,4,11)' --filter='save:test_ada.png(11)'  --filter='find:a(11)'  \
	                    --filter='glsl:sum_sq_pixel_diff(6,10,11)' --filter='save:test_bda.png(11)' --filter='find:a(11)' 

blah2c: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:intensity_from_rgb(0,2)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:windowed_equalization(2,4)&-DNUM_OFFSETS=81&-DOFFSETS=offsets_2d_81' \
						--filter='glsl:intensity_from_rgb(1,8)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:windowed_equalization(8,10)&-DNUM_OFFSETS=81&-DOFFSETS=offsets_2d_81' \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_a.png(4)'  \
						--filter='save:test_b.png(10)'  \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_copy_shader(4,6)'  --filter='save:test_corr.png(6)' \
	                    --filter='glsl:sum_sq_pixel_diff(6,4,11)' --filter='save:test_ada.png(11)'  --filter='find:a(11)'  \
	                    --filter='glsl:sum_sq_pixel_diff(6,10,11)' --filter='save:test_bda.png(11)' --filter='find:a(11)' 

blah3: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:intensity_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:intensity_from_rgb(1,8)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(3,4)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(2)'  \
	                    --filter='glsl:gauss(8,9)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(9,10)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_b.png(8)'  \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_copy_circle(2,6)'  --filter='save:test_corr.png(6)'

blah4: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:intensity_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:intensity_from_rgb(1,8)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(3,4)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(2)'  \
	                    --filter='glsl:gauss(8,9)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(9,10)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_b.png(8)'  \
	                    --filter='glsl:harris(4,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)'  --filter='save:test_corr.png(6)' \
						--filter='glsl:sq_dft_diff(6,2,7)'  --filter='save:test_ada.png(7)' \
						--filter='find:a(7)' \
						--filter='glsl:sq_dft_diff(6,10,7)'  --filter='save:test_bda.png(7)' \
						--filter='find:a(7)'

blah5: prog
	./prog -n 12 -i images/IMG_1854.JPG -i images/IMG_1855.JPG \
						--filter='glsl:intensity_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:intensity_from_rgb(1,8)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
	                    --filter='glsl:harris(2,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(8)'  \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=8'  --filter='save:test_corr_8.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,7)&-DCORR_CIRCLE_RADIUS=8'           --filter='save:test_bda_8.png(7)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=6'  --filter='save:test_corr_6.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,3)&-DCORR_CIRCLE_RADIUS=6'           --filter='save:test_bda_6.png(3)' \
						--filter='glsl:alu_buffers(3,7,4)&-DOP=src_b*src_a'  --filter='save:test_mult_2.png(4)' \
						--filter='find:a(3)' \
						--filter='find:a(4)'

blah6: prog
	./prog -n 12 -i images/IMG_1900.JPG -i images/IMG_1901.JPG \
						--filter='glsl:intensity_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='glsl:intensity_from_rgb(1,8)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0' \
						--filter='save:test_a.png(2)'  \
						--filter='save:test_b.png(8)'  \
	                    --filter='glsl:harris(2,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=8'  --filter='save:test_corr_8.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,7)&-DCORR_CIRCLE_RADIUS=8'           --filter='save:test_bda_8.png(7)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=4'  --filter='save:test_corr_4.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,3)&-DCORR_CIRCLE_RADIUS=4'           --filter='save:test_bda_4.png(3)' \
						--filter='glsl:alu_buffers(3,7,4)&-DOP=src_b*src_a'  --filter='save:test_mult_2.png(4)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=6'  --filter='save:test_corr_6.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,7)&-DCORR_CIRCLE_RADIUS=6'           --filter='save:test_bda_6.png(7)' \
						--filter='glsl:alu_buffers(4,7,3)&-DOP=src_b*src_a'  --filter='save:test_mult_3.png(3)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=7'  --filter='save:test_corr_7.png(6)' \
						--filter='glsl:sq_dft_diff(6,8,3)&-DCORR_CIRCLE_RADIUS=7'           --filter='save:test_bda_7.png(3)' \
						--filter='glsl:alu_buffers(3,7,4)&-DOP=src_b*src_a'  --filter='save:test_mult_4.png(4)' \
						--filter='find:a(4)'



test: gauss harris windowed_equalization sobel

alu_constant: prog
	./prog -n 3 \
		--filter='glsl:alu_buffers(1,1,0)&-DOP=0.5'  --filter='save:test_constant_50.png(0)' \
		--filter='glsl:alu_buffers(0,0,1)&-DOP=0.8'  --filter='save:test_constant_80.png(0)' \
		--filter='glsl:alu_buffers(1,0,2)&-DOP=src_a*src_b'  --filter='save:test_constant_40.png(2)'

alu_mult: prog
	./prog -n 3 \
		--filter='glsl:alu_buffers(1,1,0)&-DOP=(x*y)'       --filter='save:test_xy.png(0)' \
		--filter='glsl:alu_buffers(0,0,1)&-DOP=((1-x)*y)'   --filter='save:test_xy2.png(1)' \
		--filter='glsl:alu_buffers(1,0,2)&-DOP=(1-4*src_a*src_b)' --filter='save:test_xymult.png(2)'

windowed_equalization: prog
	./prog -n 4 -i images/IMG_1854.JPG --filter='glsl:intensity_from_rgb(0,1)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:windowed_equalization(1,3)&-DNUM_OFFSETS=81&-DOFFSETS=offsets_2d_81' \
						--filter='save:test_a.png(3)'

sobel: prog
	./prog -n 4 -i images/IMG_1854.JPG --filter='glsl:intensity_from_rgb(0,1)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:convolve_2d(1,3)&-DNUM_WEIGHTS=9&-DOFFSET_WEIGHTS=sobel_weights' \
						--filter='save:test_a.png(3)'


gauss: prog
	./prog -n 4 -i images/IMG_1854.JPG --filter='glsl:intensity_from_rgb(0,1)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:gauss(1,2)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(3)'

harris: prog
	./prog -n 4 -i images/IMG_1854.JPG --filter='glsl:intensity_from_rgb(0,1)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:harris(1,2)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_a.png(2)'

fft: prog
	./prog -n 12 -i images/IMG_1900.JPG \
						--filter='glsl:intensity_from_rgb(0,2)&-DSELECTED_VALUE=r' \
	                    --filter='glsl:gauss(2,3)&-DX_NOT_Y=false&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
	                    --filter='glsl:gauss(3,2)&-DX_NOT_Y=true&-DNUM_WEIGHTS=9&-DWEIGHTS=gauss_offset_weights9' \
						--filter='save:test_a.png(2)'  \
	                    --filter='glsl:harris(2,5)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25' \
						--filter='save:test_h.png(5)' \
						--filter='find:a(5)' \
						--filter='corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=8'  --filter='save:test_corr_8.png(6)' \
						--filter='find:a(6)'

clean:
	rm *.o prog

