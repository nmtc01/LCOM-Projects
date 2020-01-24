// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>
#include <machine/int86.h> // /usr/src/include/arch/i386

#include "graphi.h"

vbe_mode_info_t *info_ptr;
mmap_t mamap;
static char *video_mem;         	// frame-buffer VM address
static unsigned int vram_base;  	// VRAM's physical addresss
static unsigned int vram_size;		// VRAM's size

static unsigned h_res;	        	// Horizontal resolution in pixels
static unsigned v_res;	        	// Vertical resolution in pixels
static unsigned bits_per_pixel;	 	// Number of VRAM bits per pixel
static unsigned byte_per_pixel;		// Number of bytes per pixel

static unsigned MemoryModel;
static unsigned BlueMaskSize;
static unsigned GreenMaskSize;
static unsigned RedMaskSize;
static unsigned BluePosition;
static unsigned GreenPosition;
static unsigned RedPosition;

void *(vg_init)(uint16_t mode) {
	lm_init(false);

	if (ret_graphics_mode(mode))
		return NULL;

	vbe_mode_info_t info = *info_ptr;

	h_res = info.XResolution;
	v_res = info.YResolution;
	bits_per_pixel = info.BitsPerPixel;

	if (bits_per_pixel % 8 != 0)
		byte_per_pixel = bits_per_pixel / 8 + 1;
	else
		byte_per_pixel = bits_per_pixel / 8;

	BlueMaskSize = info.BlueMaskSize;
	GreenMaskSize = info.GreenMaskSize;
	RedMaskSize = info.RedMaskSize;
	BluePosition = info.BlueFieldPosition;
	GreenPosition = info.GreenFieldPosition;
	RedPosition = info.RedFieldPosition;
	MemoryModel = info.MemoryModel;

	vram_base = info.PhysBasePtr;
	vram_size = h_res * v_res * bits_per_pixel;

	struct minix_mem_range mr;
	mr.mr_base = (phys_bytes)vram_base;
	mr.mr_limit = mr.mr_base + vram_size;

	int r;															// error
	if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
		return NULL;

	video_mem = vm_map_phys(SELF, (void*)mr.mr_base, vram_size);

	/* Map memory */
	if (video_mem == MAP_FAILED)
		return NULL;
	printf("\nVideo meme     %x", video_mem);
	printf("\nh_res          %u", h_res);
	printf("\nv_res          %u", v_res);
	printf("\nbyte_per_pixel %u", byte_per_pixel);

	struct reg86u reg86;
	memset(&reg86, 0, sizeof(reg86));

	reg86.u.b.intno = BIOS_VIDEO_SERV;
	reg86.u.w.ax = SET_VBE_MODE;
	reg86.u.w.bx = mode | (1 << 14);
	if (sys_int86(&reg86) != OK) {
		return NULL;
	}

	lm_free(&mamap);

	return video_mem;
}

int set_graphics_mode(uint16_t mode) {
	struct reg86u reg86;
	memset(&reg86, 0, sizeof(reg86));

	reg86.u.b.intno = BIOS_VIDEO_SERV;
	reg86.u.w.ax = SET_VBE_MODE;
	reg86.u.w.bx = mode | (1 << 14);
	if (sys_int86(&reg86) != OK)
		return 1;

	return 0;
}

int set_text_mode() {
	struct reg86u reg86;
	memset(&reg86, 0, sizeof(reg86));

	reg86.u.b.intno = BIOS_VIDEO_SERV;
	reg86.u.w.ax = SET_DEFAULT_TEXT_MODE;
	if (sys_int86(&reg86) != OK)
		return 1;

	return 0;
}

int ret_graphics_mode(uint16_t mode) {
	unsigned i = 0;
	while ((info_ptr = lm_alloc(sizeof(vbe_mode_info_t), &mamap)) == NULL) {
		i++;
		if (i >= 3) {
			return 1;
		}
	}

	struct reg86u reg86;
	memset(&reg86, 0, sizeof(reg86));

	reg86.u.b.intno = BIOS_VIDEO_SERV;
	reg86.u.w.ax = RET_VBE_MODE;
	reg86.u.w.cx = mode;
	reg86.u.w.es = PB2BASE(mamap.phys);
	reg86.u.w.di = PB2OFF(mamap.phys);

	if (sys_int86(&reg86) != OK)
		return 1;



	return 0;
}

int draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
	uint32_t c = color;			//Divide color into 4 bytes
	uint8_t c1 = (uint8_t)c;
	c = c >> 8;
	uint8_t c2 = (uint8_t)c;
	c = c >> 8;
	uint8_t c3 = (uint8_t)c;
	c = c >> 8;
	uint8_t c4 = (uint8_t)c;

	char* byte = video_mem + y * h_res * byte_per_pixel + x * byte_per_pixel;

	for (uint16_t i = 0; i < len; i++) {
		if (byte_per_pixel == 1) {
			*byte = c1;
			byte++;
		}
		if (byte_per_pixel == 2) {
			*byte = c2;
			byte++;
			*byte = c1;
			byte++;
		}
		if (byte_per_pixel == 3) {
			*byte = c3;
			byte++;
			*byte = c2;
			byte++;
			*byte = c1;
			byte++;
		}
		if (byte_per_pixel == 4) {
			*byte = c4;
			byte++;
			*byte = c3;
			byte++;
			*byte = c2;
			byte++;
			*byte = c1;
			byte++;
		}
	}
	return 0;
}

int draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
	if ((y + height) > v_res)
		return 1;
	if ((x + width) > h_res)
		return 1;

	uint16_t i = 0;
	uint16_t line = y;

	while (i < height) {
		if (draw_hline(x, line, width, color))
			return 1;
		i++;
		line++;
	}
	return 0;
}

int draw_matrix(uint8_t no_rectangles, uint32_t first, uint8_t step) {
	unsigned h_rect = h_res / no_rectangles;
	unsigned v_rect = v_res / no_rectangles;
	uint32_t colour = 0;
	uint16_t col = 0, line = 0;
	unsigned nh_rect = 0, nv_rect = 0;


	if (MemoryModel == 0x06 || MemoryModel == 0x07) {
		// Direct colour
		// R(row, col) = (R(first) + col * step) % (1 << RedScreeMask)
		// G(row, col) = (G(first) + row * step) % (1 << GreenScreeMask)
		// B(row, col) = (B(first) + (col + row) * step) % (1 << BlueScreeMask)		

		/* while(nh_rect < no_rectangles){
			while(nv_rect < no_rectangles){

				colour = (R(first) + col * step) % (1 << RedMaskSize);

				if(draw_rectangle(col, line, h_rect, v_rect, colour))
					return 1;

				line += v_rect;
				nv_rect++;
			}
			nh_rect++;
			nv_rect = 0;
			line = 0;
			col += h_rect;
		} */

		return 0;
	}
	else {
		// Indexed
		// index(row, col) = (first + (row * no_rectangles + col) * step) % (1 << BitsPerPixel)
		while (nh_rect < no_rectangles) {
			while (nv_rect < no_rectangles) {

				colour = (first + (((line * no_rectangles) + col) * step)) % 0x3f;

				if (draw_rectangle(col, line, h_rect, v_rect, colour))
					return 1;

				line += v_rect;
				nv_rect++;
			}
			nh_rect++;
			nv_rect = 0;
			line = 0;
			col += h_rect;
		}
		return 0;
	}
	return 1;
}

int draw_xpm(const char xpm[], uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	if ((y + height) > v_res)
		return 1;
	if ((x + width) > h_res)
		return 1;

	uint16_t line = y;
	uint16_t i = 0;

	while (i < height * width)
	{
		for (unsigned int j = 0; j < width; j++)
		{
			char* byte;
			byte = video_mem + line * h_res * byte_per_pixel + x + j * byte_per_pixel;
			*byte = xpm[i];
			i++;
			byte++;
		}
		line++;
	}

	return 0;
}

int erase_xpm(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	if ((y + height) > v_res)
		return 1;
	if ((x + width) > h_res)
		return 1;

	uint16_t line = y;
	uint16_t i = 0;

	while (i < height * width)
	{
		for (unsigned int j = 0; j < width; j++)
		{
			char* byte;
			byte = video_mem + line * h_res * byte_per_pixel + x + j * byte_per_pixel;
			*byte = BLACK;
			i++;
			byte++;
		}
		line++;
	}

	return 0;
}

int escape_break()
{
	uint8_t bit_no = 1;

	if (kb_subscribe_int(&bit_no))   //subscribes keyboard interrupt
		return 1; //failure

	int ipc_status, r, irq_set = BIT(bit_no); //irq_set is an integer with the bit specified by the variable bit_no at 1
	message msg;

	while (data != ESC_BREAK) {  //while escape key is not pressed
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set) {
					kbc_ih();
					if (data != 0)
						kbd_data_handler();
				}
				break;
			default:
				break;
			}
		}
	}

	if (kb_unsubscribe_int())  //unsubscribes keyboard interrupt
		return 1; //failure

	return 0;
}

int draw_left(const char sprite[], uint16_t* x, uint16_t y, uint16_t wd, uint16_t hg, uint16_t mov_per_frame)
{
	if (erase_xpm(*x, y, wd, hg))
		return 1;
	if (draw_xpm(sprite, *x - mov_per_frame, y, (uint16_t)wd, (uint16_t)hg))
		return 1;
	*x -= mov_per_frame;
	return 0;
}

int draw_right(const char sprite[], uint16_t* x, uint16_t y, uint16_t wd, uint16_t hg, uint16_t mov_per_frame)
{
	if (erase_xpm(*x, y, wd, hg))
		return 1;
	if (draw_xpm(sprite, *x + mov_per_frame, y, (uint16_t)wd, (uint16_t)hg))
		return 1;
	*x += mov_per_frame;
	return 0;
}

int draw_up(const char sprite[], uint16_t x, uint16_t* y, uint16_t wd, uint16_t hg, uint16_t mov_per_frame)
{
	if (erase_xpm(x, *y, wd, hg))
		return 1;
	if (draw_xpm(sprite, x, *y - mov_per_frame, (uint16_t)wd, (uint16_t)hg))
		return 1;
	*y -= mov_per_frame;
	return 0;
}

int draw_down(const char sprite[], uint16_t x, uint16_t* y, uint16_t wd, uint16_t hg, uint16_t mov_per_frame)
{
	if (erase_xpm(x, *y, wd, hg))
		return 1;
	if (draw_xpm(sprite, x, *y + mov_per_frame, (uint16_t)wd, (uint16_t)hg))
		return 1;
	*y += mov_per_frame;
	return 0;
}

int escape_or_move(const char sprite[], uint16_t x, uint16_t y, uint16_t wd, uint16_t hg, uint16_t xf, uint16_t xi, uint16_t yf, uint16_t yi, uint16_t nr_frames, uint16_t mov_per_frame, uint8_t fr_rate)
{
	uint8_t bit_no_kb = 1;
	uint8_t bit_no_tm = 3;

	if (kb_subscribe_int(&bit_no_kb))   //subscribes keyboard interrupt
		return 1; //failure

	if (timer_subscribe_int(&bit_no_tm))   //subscribes keyboard interrupt
		return 1;

	timer_set_frequency(0, fr_rate);

	int ipc_status, r, irq_set_kb = BIT(bit_no_kb), irq_set_tm = BIT(bit_no_tm);
	message msg;

	while (data != ESC_BREAK) {  //while escape key is not pressed
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set_kb) {
					kbc_ih();
					if (data != 0)
						kbd_data_handler();
				}
				if (msg.m_notify.interrupts & irq_set_tm) {
					if (nr_frames)
					{
						if (yf == yi) {
							if (erase_xpm(x, y, wd, hg))
								return 1;
							if (draw_xpm(sprite, x + mov_per_frame, y, (uint16_t)wd, (uint16_t)hg))
								return 1;
							x += mov_per_frame;
							nr_frames--;
						}
						else if (xf == xi) {
							if (erase_xpm(x, y, wd, hg))
								return 1;
							if (draw_xpm(sprite, x, y + mov_per_frame, (uint16_t)wd, (uint16_t)hg))
								return 1;
							y += mov_per_frame;
							nr_frames--;
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if (timer_unsubscribe_int())
		return 1;

	if (kb_unsubscribe_int())  //unsubscribes keyboard interrupt
		return 1; //failure

	return 0;
}

int escape_or_move_want(const char sprite[], const char shoot[], uint16_t x, uint16_t y, uint16_t wd, uint16_t hg, uint16_t wid, uint16_t heg, uint16_t mov_per_frame, uint8_t fr_rate)
{
	uint8_t bit_no_kb = 1;
	uint8_t bit_no_tm = 2;
	bool move = false;
	bool first_shot = true;
	uint16_t x0 = x;
	uint16_t y0 = y;

	if (kb_subscribe_int(&bit_no_kb))   //subscribes keyboard interrupt
		return 1; //failure

	if (timer_subscribe_int(&bit_no_tm))   //subscribes keyboard interrupt
		return 1;

	timer_set_frequency(0, fr_rate);

	int ipc_status, r, irq_set_kb = BIT(bit_no_kb), irq_set_tm = BIT(bit_no_tm);
	message msg;

	while (data != ESC_BREAK) {  //while escape key is not pressed
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set_kb) {
					kbc_ih();
					if (data != 0)
					{
						if (data == TWO_LONG)
						{
							move = true;
							kbd_data_handler();
							continue;
						}
						kbd_data_handler();
					}
				}
				if ((msg.m_notify.interrupts & irq_set_tm) && move) {
					switch (data) {
					case UP_MAKE:
					{
						if (erase_xpm(x, y, wd, hg))
							return 1;
						if (draw_xpm(sprite, x, y - mov_per_frame, (uint16_t)wd, (uint16_t)hg))
							return 1;
						y -= mov_per_frame;
						y0 = y;
						break;
					}

					case LEFT_MAKE:
					{
						if (erase_xpm(x, y, wd, hg))
							return 1;
						if (draw_xpm(sprite, x - mov_per_frame, y, (uint16_t)wd, (uint16_t)hg))
							return 1;
						x -= mov_per_frame;
						x0 = x;
						break;
					}

					case DOWN_MAKE:
					{
						if (erase_xpm(x, y, wd, hg))
							return 1;
						if (draw_xpm(sprite, x, y + mov_per_frame, (uint16_t)wd, (uint16_t)hg))
							return 1;
						y += mov_per_frame;
						y0 = y;
						break;
					}

					case RIGHT_MAKE:
					{
						if(erase_xpm(x, y, wd, hg))
							return 1;
						if (draw_xpm(sprite, x + mov_per_frame, y, (uint16_t)wd, (uint16_t)hg))
							return 1;
						x += mov_per_frame;
						x0 = x;
						break;
					}
					}

					if (msg.m_notify.interrupts & irq_set_tm)
					{
						if (data == SPACE_MAKE)
						{
							first_shot = true;
							x0 = x;
						}
						if (data == SPACE_BREAK)
						{
							if (erase_xpm(x0, y0, wd, hg))
								return 1;
							if (draw_xpm(shoot, x0 + x0 + mov_per_frame, y0, (uint16_t)wid, (uint16_t)heg))
								return 1;
							if (draw_xpm(sprite, x, y, (uint16_t)wid, (uint16_t)heg))
								return 1;
							if (x0 < 200 && first_shot)
								x0 += mov_per_frame;
							else {
								x0 = x;
								first_shot = false;
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if (timer_unsubscribe_int())
		return 1;

	if (kb_unsubscribe_int())  //unsubscribes keyboard interrupt
		return 1; //failure

	return 0;
}

