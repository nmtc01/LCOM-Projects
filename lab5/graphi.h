#include "kb.h"

//Global variables


//Macros
#define	BIOS_VIDEO_SERV			0x10
#define RET_VBE_CTRL			0x4F00
#define RET_VBE_MODE			0x4F01
#define SET_VBE_MODE			0x4F02
#define SET_DEFAULT_TEXT_MODE	0x0003
#define VIDEO_MODE				0x105

// COLOURS

#define BLACK	0x00000000
#define RED		0xff000000
#define BLUE	0x0000ff00
#define GREEN	0x00ff0000

int set_graphics_mode(uint16_t mode);
int set_text_mode();
int ret_graphics_mode(uint16_t mode);
int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
int draw_matrix(uint8_t no_rectangles, uint32_t first, uint8_t step);
int draw_xpm(const char xpm[], uint16_t x, uint16_t y, uint16_t width, uint16_t height);
int erase_xpm(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
int escape_break();
int draw_left(const char xpm[], uint16_t* x, uint16_t y, uint16_t wd, uint16_t hg, uint16_t mov_per_frame);
int draw_right(const char xpm[], uint16_t* x, uint16_t y, uint16_t wd, uint16_t hg, uint16_t mov_per_frame);
int draw_up(const char xpm[], uint16_t x, uint16_t* y, uint16_t wd, uint16_t hg, uint16_t mov_per_frame);
int draw_down(const char xpm[], uint16_t x, uint16_t* y, uint16_t wd, uint16_t hg, uint16_t mov_per_frame);
int escape_or_move(const char sprite[], uint16_t x, uint16_t y, uint16_t wd, uint16_t hg, uint16_t xf, uint16_t xi, uint16_t yf, uint16_t yi, uint16_t nr_frames, uint16_t mov_per_frame, uint8_t fr_rate);
int escape_or_move_want(const char sprite[], const char shoot[], uint16_t x, uint16_t y, uint16_t wd, uint16_t hg, uint16_t wid, uint16_t heg, uint16_t mov_per_frame, uint8_t fr_rate);
