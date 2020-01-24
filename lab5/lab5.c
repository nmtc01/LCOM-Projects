// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>
#include <machine/int86.h> // /usr/src/include/arch/i386
#include <lcom/pixmap.h> // defines  pic1, pic2, etc 

// Any header files included below this line should have been created by you
#include "graphi.h"


int main(int argc, char *argv[]) {
	// sets the language of LCF messages (can be either EN-US or PT-PT)
	lcf_set_language("EN-US");

	// enables to log function invocations that are being "wrapped" by LCF
	// [comment this out if you don't want/need it]
	lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

	// enables to save the output of printf function calls on a file
	// [comment this out if you don't want/need it]
	lcf_log_output("/home/lcom/labs/lab5/output.txt");

	// handles control over to LCF
	// [LCF handles command line arguments and invokes the right function]
	if (lcf_start(argc, argv))
		return 1;

	// LCF clean up tasks	
	// [must be the last statement before return]
	lcf_cleanup();

	return 0;
}

int (video_test_init)(uint16_t mode, uint8_t delay) {

	if (vg_init(mode) == NULL) {
		vg_exit();
		return 1;
	}

	sleep(delay);

	if (vg_exit())
		return 1;

	return 0;
}

int (video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {	
	if (vg_init(mode) == NULL) {
		vg_exit();
		return 1;
	}

	draw_rectangle(x,y,width,height,color);
	  
	if (escape_break())
		return 1;

	if (vg_exit())
		return 1;
	
	// printf("\nvideo meme    %x ", video_mem);


	return 0;
}

int (video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
	if (vg_init(mode) == NULL) {
		vg_exit();
		return 1;
	}
	if(no_rectangles == 0)
		return 1;

	draw_matrix(no_rectangles, first, step);

	sleep(5);
	if (vg_exit())
		return 1;

	return 0;
}

int (video_test_xpm)(const char *xpm[], uint16_t x, uint16_t y) {
	if (vg_init(VIDEO_MODE) == NULL) {
		vg_exit();
		return 1;
	}

	int wd, hg;
	char *sprite = read_xpm(xpm, &wd, &hg);

	if (draw_xpm(sprite, x, y, (uint16_t)wd, (uint16_t)hg))
		return 1;

	if (escape_break())
		return 1;

	if (vg_exit())
		return 1;

	return 0;
}  

int (video_test_move)(const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate) {
	if (vg_init(VIDEO_MODE) == NULL) {
		vg_exit();
		return 1;
	}

	int wd, hg;
	char *sprite = read_xpm(xpm, &wd, &hg);

	int wid, heg;
	char *shoot = read_xpm(pic2, &wid, &heg);

	if (draw_xpm(sprite, xi, yi, (uint16_t)wd, (uint16_t)hg))
		return 1;
 
	uint16_t nr_frames = 0;
	uint16_t x = xi;
	uint16_t y = yi;
	
	if (yf == yi)
		nr_frames = (xf - xi) / speed;
	else if (xf == xi)
		nr_frames = (yf - yi) / speed;
	/*
	if (escape_or_move(sprite, x, y, wd, hg, xf, xi, yf, yi, nr_frames, speed, fr_rate))
		return 1;*/

	if (escape_or_move_want(sprite, shoot, x, y, wd, hg, wid, heg, speed, fr_rate))
		return 1;

	if (vg_exit())
		return 1;

	return 0;
}



