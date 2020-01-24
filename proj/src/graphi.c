// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <machine/int86.h>  // /usr/src/include/arch/i386
#include <stdint.h>
#include <stdio.h>
#include "graphi.h"

// Global variables
vbe_mode_info_t* info_ptr;
mmap_t mamap;
static char* video_mem;         // frame-buffer VM address
static unsigned int vram_base;  // VRAM's physical addresss
static unsigned int vram_size;  // VRAM's size
static char* second_buffer;

static unsigned h_res;           // Horizontal resolution in pixels
static unsigned v_res;           // Vertical resolution in pixels
static unsigned bits_per_pixel;  // Number of VRAM bits per pixel
static unsigned byte_per_pixel;  // Number of bytes per pixel

static player* p;                    // player object
static enemy* e_array[20] = {NULL};  // array of enemies
static shot* s_array[20] = {NULL};   // array of shots

enum mouse_states states = 0;

xpm_image_t* p_sprite;
xpm_image_t* s_sprite;
xpm_image_t* e_sprite;
xpm_image_t* explosion_sprite;
xpm_image_t bola;
xpm_image_t one;
xpm_image_t two;
xpm_image_t three;
xpm_image_t four;
xpm_image_t five;
xpm_image_t six;
xpm_image_t seven;
xpm_image_t eight;
xpm_image_t nine;

bool able = true;
static uint16_t player_speed;
static bool up = 0;
static bool dn = 0;
static bool rt = 0;
static bool lf = 0;
static bool shooting = 0;
static bool dead = false;
static unsigned nr_shots = 0;
static unsigned lives = 3;
static unsigned time_pressed = 0;
static bool first_click = false;
static bool ignore_next = false;
static char loaded = 0;

static uint32_t scores[3] = {0, 0, 0};
static uint32_t score = 0000000;
static unsigned back_frame = 0;

void*(vg_init)(uint16_t mode) {
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

  vram_base = info.PhysBasePtr;
  vram_size = h_res * v_res * bits_per_pixel;

  struct minix_mem_range mr;
  mr.mr_base = (phys_bytes)vram_base;
  mr.mr_limit = mr.mr_base + vram_size;

  int r;  // error
  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    return NULL;

  video_mem = vm_map_phys(SELF, (void*)mr.mr_base, vram_size);

  /* Map memory */
  if (video_mem == MAP_FAILED)
    return NULL;

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

int draw_xpm_noclip(xpm_image_t xpm[],
                    int x,
                    int y,
                    uint16_t width,
                    uint16_t height) {
  if ((y + height) < 0) {
    return 1;
  }
  if ((x + width) < 0) {
    return 1;
  }

  int xi = x;
  int yi = y;
  unsigned skip_xi = 0;
  unsigned skip_yi = 0;

  unsigned xf = x + width;
  unsigned yf = y + height;
  unsigned skip_xf = 0;

  unsigned line = y;
  unsigned i = 0;
  unsigned n_bytes_line = width * byte_per_pixel;

  if (y < 0) {
    yi = 0;
    line = 0;
    skip_yi = abs(y);
  }
  if (x < 0) {
    xi = 0;
    n_bytes_line = (width + x) * byte_per_pixel;
    skip_xi = abs(x);
  }

  if (yf >= v_res) {
    yf = v_res;
  }
  if (xf >= h_res) {
    xf = h_res;
    n_bytes_line = (xf - x) * byte_per_pixel;
    skip_xf = (x + width - xf);
  }

  unsigned x0 = (xi + yi * h_res) * byte_per_pixel;

  i += (skip_yi * width) * byte_per_pixel;
  while (line < yf) {
    i += (skip_xi)*byte_per_pixel;
    for (unsigned j = 0; j < n_bytes_line; j++) {
      if (xpm->bytes[i] != IGNORE_BYTE) {
        second_buffer[x0 + j] = xpm->bytes[i];
      }
      i++;
    }
    line++;
    x0 = (line * h_res + xi) * byte_per_pixel;
    i += skip_xf * byte_per_pixel;
  }

  return 0;
}

void number_to_array(char digits[], unsigned number, unsigned n_digits) {
  unsigned tens = 10;
  unsigned tens_prv = 1;
  for (unsigned i = 0; i < n_digits; i++) {
    digits[i] = (number % tens) / tens_prv;
    tens *= 10;
    tens_prv *= 10;
  }
}

void print_number(char number[], uint16_t x, uint16_t y, unsigned n_digits) {
  for (unsigned i = n_digits - 1; i < n_digits; i--) {
    switch (number[i]) {
      case 0:
        draw_xpm_noclip(&bola, x, y, 10, 18);
        break;
      case 1:
        draw_xpm_noclip(&one, x, y, 10, 18);
        break;
      case 2:
        draw_xpm_noclip(&two, x, y, 10, 18);
        break;
      case 3:
        draw_xpm_noclip(&three, x, y, 10, 18);
        break;
      case 4:
        draw_xpm_noclip(&four, x, y, 10, 18);
        break;
      case 5:
        draw_xpm_noclip(&five, x, y, 10, 18);
        break;
      case 6:
        draw_xpm_noclip(&six, x, y, 10, 18);
        break;
      case 7:
        draw_xpm_noclip(&seven, x, y, 10, 18);
        break;
      case 8:
        draw_xpm_noclip(&eight, x, y, 10, 18);
        break;
      case 9:
        draw_xpm_noclip(&nine, x, y, 10, 18);
        break;
      default:
        return;
        break;
    }
    x += 12;
  }
}

void print_date(int seconds,
                int minutes,
                int hours,
                int days,
                int months,
                int years,
                uint16_t x,
                uint16_t y) {
  char date[2];

  number_to_array(date, seconds, 2);
  print_number(date, x + 60, y + 20, 2);

  number_to_array(date, minutes, 2);
  print_number(date, x + 30, y + 20, 2);

  number_to_array(date, hours, 2);
  print_number(date, x, y + 20, 2);

  number_to_array(date, days, 2);
  print_number(date, x, y, 2);

  number_to_array(date, months, 2);
  print_number(date, x + 30, y, 2);

  number_to_array(date, 2000 + years, 4);
  print_number(date, x + 60, y, 4);
}

int draw_xpm(xpm_image_t xpm[],
             uint16_t x,
             uint16_t y,
             uint16_t width,
             uint16_t height) {
  if ((y + height) > v_res)
    return 1;
  if ((x + width) > h_res)
    return 1;

  unsigned yf = y + height;
  unsigned line = y;
  unsigned i = 0;
  unsigned n_bytes_line = width * byte_per_pixel;
  unsigned x0 = (y * h_res + x) * byte_per_pixel;

  while (line < yf) {
    for (unsigned j = 0; j < n_bytes_line; j++) {
      if (xpm->bytes[i] != IGNORE_BYTE) {
        second_buffer[x0 + j] = xpm->bytes[i];
      }
      i++;
    }
    line++;
    x0 = (line * h_res + x) * byte_per_pixel;
  }

  return 0;
}

void update_buffer() {
  memcpy(video_mem, second_buffer, h_res * v_res * byte_per_pixel);
}

int sin_pattern(enemy* e) {
  return (int)(e->og_y + 100 - 100 * sin(e->x / 50.0));
}

int cross_pattern(enemy* e) {
  // return (int)(e->og_y + e->x * 0.5);
  if (e->y >= 2 * v_res) {
    e->speedY = abs(e->speedY);
  } else if (e->y + e->height >= v_res) {
    e->speedY = -abs(e->speedY);
  }
  return e->y + e->speedY;
}

void move_enemy(enemy* e) {
  int x, y;
  if (e->active) {
    x = e->x + e->speedX;
    switch (e->path) {
      case 0:
        y = e->y;
        break;
      case 1:
        y = sin_pattern(e);
        break;
      case 2:
        y = cross_pattern(e);
        break;
      default:
        y = e->y + e->speedY;
        break;
    }

    if (x >= (int)h_res || x + e->width <= 0) {
      delete_enemy(e);
    } else if (((e->x >= p->x && e->x <= p->x + p->width) &&
                (e->y >= p->y && e->y <= p->y + p->height)) ||
               ((e->x + e->width >= p->x &&
                 e->x + e->width <= p->x + p->width) &&
                (e->y + e->height >= p->y &&
                 e->y + e->height <= p->y + p->height)) ||
               ((e->x + e->width >= p->x &&
                 e->x + e->width <= p->x + p->width) &&
                (e->y >= p->y && e->y <= p->y + p->height)) ||
               ((e->x >= p->x && e->x <= p->x + p->width) &&
                (e->y + e->height >= p->y &&
                 e->y + e->height <= p->y + p->height))) {
      if (!dead)
        delete_enemy(e);
      else {
        e->x = x;
        e->y = y;
      }
      remove_life(explosion_sprite);
    } else {
      e->x = x;
      e->y = y;
    }
    draw_xpm_noclip(e_sprite, e->x, e->y, e->width, e->height);
  }
}

int move_shot(shot* s) {
  unsigned x, y;
  if (s->active) {
    x = s->x + s->speedX;
    y = s->y + s->speedY;
    if (x > (2 * h_res)) {
      if (delete_shot(s))
        return 1;
    }
    if (x + s->width >= h_res) {
      if (delete_shot(s))
        return 1;
    } else
      s->x = x;

    if (y > (2 * v_res)) {
      if (delete_shot(s))
        return 1;
    }
    if (y > v_res) {
      if (delete_shot(s))
        return 1;
    } else
      s->y = y;

    draw_xpm_noclip(s_sprite, s->x, s->y, s->width, s->height);
  }
  return 0;
}

void move_player() {
  unsigned x, y;
  x = p->x;
  y = p->y;

  if (up)
    y -= player_speed;
  if (dn)
    y += player_speed;
  if (rt)
    x += player_speed;
  if (lf)
    x -= player_speed;

  if (x > (h_res * 2))
    p->x = 0;
  else if ((x + p->width) >= h_res)
    p->x = h_res - p->width;
  else
    p->x = x;

  if (y > (2 * v_res))
    p->y = 0;
  else if ((y + p->height) > v_res - 100)
    p->y = v_res - p->height - 100;
  else
    p->y = y;

  draw_xpm(p_sprite, p->x, p->y, p->width, p->height);
}

void draw_buffer_background(xpm_image_t xpm[]) {
  back_frame %= h_res;
  unsigned x0;
  unsigned byte_line = h_res * byte_per_pixel;
  for (unsigned line = 0; line < v_res; line++) {
    x0 = (back_frame + line * h_res) * byte_per_pixel;
    byte_line = (1 + line) * h_res * byte_per_pixel;
    for (unsigned i = line * h_res * byte_per_pixel; i < byte_line; i++) {
      second_buffer[i] = xpm->bytes[x0];
      x0++;
      if (x0 >= byte_line) {
        x0 -= h_res * byte_per_pixel;
      }
    }
  }
}

int draw_buffer(xpm_image_t fundo[], bool big, xpm_image_t barra[]) {
  draw_buffer_background(fundo);
  unsigned i, array_elem;  // Number of entities on screen
  enemy* e;
  shot* s;
  char digits[9];

  // PLAYER
  move_player();

  // ENEMY
  array_elem = NELEMS(e_array);  // Number of enemies on screen
  i = 0;
  while (i < array_elem) {
    e = e_array[i];
    if (e != NULL && e->active) {
      move_enemy(e);
    }
    i++;
  }

  // SHOT
  array_elem = NELEMS(s_array);  // Number of shots on screen
  i = 0;
  while (i < array_elem) {
    s = s_array[i];
    if (s != NULL && s->active) {
      if (move_shot(s))
        return 1;
    }
    i++;
  }

  if (shooting && s_array[nr_shots] != NULL) {
    s = s_array[nr_shots];
    if (big) {
      s->x = p->x + 50;
      s->y = p->y + 10;
    } else {
      s->x = p->x + 50;
      s->y = p->y + 20;
    }
    if (up)
      s->y -= player_speed;
    if (dn)
      s->y += player_speed;
    if (rt)
      s->x += player_speed;
    if (lf)
      s->x -= player_speed;
  }

  draw_xpm(barra, 129, (v_res - 78), 894, 78);
  number_to_array(digits, score / 10, 7);
  print_number(digits, 800, (v_res - 30), 7);
  return 0;
}

void verifyCollision(bool* big) {
  for (unsigned i = 0; i < 20; i++) {
    if (s_array[i] != NULL && s_array[i]->speedX > 0 && s_array[i]->active)
      for (unsigned j = 0; j < 20; j++) {
        if (e_array[j] != NULL && e_array[j]->active)
          if (((s_array[i]->x >= e_array[j]->x &&
                s_array[i]->x <= e_array[j]->x + e_array[j]->width) &&
               (s_array[i]->y >= e_array[j]->y &&
                s_array[i]->y <= e_array[j]->y + e_array[j]->height)) ||
              ((s_array[i]->x + s_array[i]->width >= e_array[j]->x &&
                s_array[i]->x + s_array[i]->width <=
                    e_array[j]->x + e_array[j]->width) &&
               (s_array[i]->y + s_array[i]->height >= e_array[j]->y &&
                s_array[i]->y + s_array[i]->height <=
                    e_array[j]->y + e_array[j]->height)) ||
              ((s_array[i]->x + s_array[i]->width >= e_array[j]->x &&
                s_array[i]->x + s_array[i]->width <=
                    e_array[j]->x + e_array[j]->width) &&
               (s_array[i]->y >= e_array[j]->y &&
                s_array[i]->y <= e_array[j]->y + e_array[j]->height)) ||
              ((s_array[i]->x >= e_array[j]->x &&
                s_array[i]->x <= e_array[j]->x + e_array[j]->width) &&
               (s_array[i]->y + s_array[i]->height >= e_array[j]->y &&
                s_array[i]->y + s_array[i]->height <=
                    e_array[j]->y + e_array[j]->height))) {
            e_array[j]->health -= s_array[i]->damage;
            if (e_array[j]->health <= 0) {
              delete_enemy(e_array[j]);
            }
            score += 30;
            if (!(*big)) {
              delete_shot(s_array[i]);
            }
          }
      }
  }
}

void draw_menu(xpm_image_t main_menu[]) {
  unsigned j = h_res * v_res * byte_per_pixel;
  for (unsigned i = 0; i < j; i++) {
    second_buffer[i] = main_menu->bytes[i];
    i++;
    second_buffer[i] = main_menu->bytes[i];
    i++;
    second_buffer[i] = main_menu->bytes[i];
  }
}

void draw_tecla(xpm_image_t tecla[], int nr, bool pause_on) {
  switch (nr) {
    case 0:
      if (pause_on)
        draw_xpm(tecla, 420, 365, tecla->width, tecla->height);
      else
        draw_xpm(tecla, 127, 393, tecla->width, tecla->height);
      break;
    case 1:
      draw_xpm(tecla, 127, 486, tecla->width, tecla->height);
      break;
    case 2:
      if (pause_on)
        draw_xpm(tecla, 422, 457, tecla->width, tecla->height);
      else
        draw_xpm(tecla, 127, 579, tecla->width, tecla->height);
      break;
  }
}

bool remove_life(xpm_image_t explosion[]) {
  if (dead)
    return false;
  if (lives == 0)
    return true;
  dead = true;
  lives--;
  p_sprite = explosion;
  able = false;
  if (shooting) {
    ignore_next = true;
    delete_shot(s_array[nr_shots]);
  }

  return false;
}

void enemies_handler(bool medium, bool harder) {
  srand(time(NULL));
  uint8_t pattern;
  uint8_t nr_enemies;
  uint8_t nr_lives;
  uint8_t place = rand() % 300;
  pattern = rand() % 3;
  nr_enemies = 8 + rand() % 3;
  nr_lives = 1 + rand() % 3;
  unsigned j = nr_enemies;
  unsigned k = j + 1;

  switch (pattern) {
    case 0:
      for (unsigned i = 0; i < nr_enemies; i++)
        if ((e_array[i] = new_enemy(h_res - 8, 0 + 90 * i, nr_lives,
                                    e_sprite->width, e_sprite->height, 0)) ==
            NULL)
          return;
      break;
    case 1:
      for (unsigned i = 0; i < nr_enemies; i++)
        if ((e_array[i] = new_enemy(h_res - 8, 0 + 90 * i, nr_lives,
                                    e_sprite->width, e_sprite->height, 1)) ==
            NULL)
          return;
      break;
    case 2:
      for (unsigned i = 0; i < nr_enemies; i++)
        if ((e_array[i] = new_enemy(h_res - 8, 0 + 90 * i, nr_lives,
                                    e_sprite->width, e_sprite->height, 2)) ==
            NULL)
          return;
      break;
  }

  if (medium) {
    srand(1);
    unsigned i = 0;
    pattern = rand() % 3;
    nr_lives = 1 + rand() % 3;

    switch (pattern) {
      case 0:
        for (; j < 15; j++) {
          if ((e_array[j] = new_enemy(h_res - 200, 0 + 90 * i, nr_lives,
                                      e_sprite->width, e_sprite->height, 0)) ==
              NULL)
            return;
          i++;
        }
        break;
      case 1:
        for (; j < 15; j++) {
          if ((e_array[j] = new_enemy(h_res - 200, 0 + 90 * i, nr_lives,
                                      e_sprite->width, e_sprite->height, 1)) ==
              NULL)
            return;
          i++;
        }
        break;
      case 2:
        for (; j < 15; j++) {
          if ((e_array[j] = new_enemy(h_res - 200, 0 + 90 * i, nr_lives,
                                      e_sprite->width, e_sprite->height, 2)) ==
              NULL)
            return;
          i++;
        }
        break;
    }
  }

  if (harder) {
    srand(2);
    unsigned i = 0;
    pattern = rand() % 3;
    nr_lives = 3;

    switch (pattern) {
      case 0:
        for (; k < 20; k++) {
          if ((e_array[k] = new_enemy(h_res - 400, place + 90 * i, nr_lives,
                                      e_sprite->width, e_sprite->height, 0)) ==
              NULL)
            return;
          i++;
        }
        break;
      case 1:
        for (; k < 20; k++) {
          if ((e_array[k] = new_enemy(h_res - 400, place + 90 * i, nr_lives,
                                      e_sprite->width, e_sprite->height, 1)) ==
              NULL)
            return;
          i++;
        }
        break;
      case 2:
        for (; k < 20; k++) {
          if ((e_array[k] = new_enemy(h_res - 400, place + 90 * i, nr_lives,
                                      e_sprite->width, e_sprite->height, 2)) ==
              NULL)
            return;
          i++;
        }
        break;
    }
  }
}

int escape_or_move(xpm_image_t sprite[],
                   xpm_image_t sprite_mouth[],
                   xpm_image_t shoot[],
                   xpm_image_t big_shoot[],
                   xpm_image_t ghost[],
                   xpm_image_t exploded[],
                   uint16_t x,
                   uint16_t y,
                   uint16_t mov_per_frame,
                   uint8_t fr_rate) {
  uint8_t bit_no_kb = 1;
  uint8_t bit_no_tm = 2;
  uint8_t bit_no_mouse = 3;
  uint8_t bit_no_rtc = 8;

  uint8_t option = 0;
  uint8_t time_dead = 0;

  char s[7];

  bool space = false;
  bool big = false;
  bool menu_on = true;
  bool guia_on = false;
  bool game_over_on = false;
  bool pause_on = false;
  bool medium = false;
  bool harder = false;
  bool updated = false;

  uint16_t last_rtc = 0;
  bool backup_rtc = false;

  player_speed = mov_per_frame;

  xpm_image_t jogar_pressed;
  xpm_image_t instru_pressed;
  xpm_image_t sair_pressed;
  xpm_image_t guia_show;
  xpm_image_t perdeu;
  xpm_image_t tres_vidas;
  xpm_image_t duas_vidas;
  xpm_image_t uma_vida;
  xpm_image_t main_menu;
  xpm_image_t fundo2;
  xpm_image_t barra;
  xpm_image_t pause;

  int seconds;
  int minutes;
  int hours;
  int days;
  int months;
  int years;

  int s_seconds;
  int s_minutes;
  int s_hours;
  int s_days;
  int s_months;
  int s_years;

  if (xpm_load(jogar, 2, &jogar_pressed) == NULL)
    return 1;
  if (xpm_load(instru, 2, &instru_pressed) == NULL)
    return 1;
  if (xpm_load(sair, 2, &sair_pressed) == NULL)
    return 1;
  if (xpm_load(guia, 2, &guia_show) == NULL)
    return 1;
  if (xpm_load(game_over, 2, &perdeu) == NULL)
    return 1;
  if (xpm_load(three_lives, 2, &tres_vidas) == NULL)
    return 1;
  if (xpm_load(two_lives, 2, &duas_vidas) == NULL)
    return 1;
  if (xpm_load(one_life, 2, &uma_vida) == NULL)
    return 1;
  if (xpm_load(menu, 2, &main_menu) == NULL)
    return 1;
  if (xpm_load(fundo, 2, &fundo2) == NULL)
    return 1;
  if (xpm_load(barra_xpm, 2, &barra) == NULL)
    return 1;
  if (xpm_load(pause_menu, 2, &pause) == NULL)
    return 1;
  if (xpm_load(zero_xpm, 2, &bola) == NULL)
    return 1;
  if (xpm_load(one_xpm, 2, &one) == NULL)
    return 1;
  if (xpm_load(two_xpm, 2, &two) == NULL)
    return 1;
  if (xpm_load(three_xpm, 2, &three) == NULL)
    return 1;
  if (xpm_load(four_xpm, 2, &four) == NULL)
    return 1;
  if (xpm_load(five_xpm, 2, &five) == NULL)
    return 1;
  if (xpm_load(six_xpm, 2, &six) == NULL)
    return 1;
  if (xpm_load(seven_xpm, 2, &seven) == NULL)
    return 1;
  if (xpm_load(eight_xpm, 2, &eight) == NULL)
    return 1;
  if (xpm_load(nine_xpm, 2, &nine) == NULL)
    return 1;

  unsigned i = 0;

  p_sprite = sprite;
  s_sprite = shoot;
  e_sprite = ghost;
  explosion_sprite = exploded;

  while ((second_buffer = malloc(vram_size)) == NULL) {
    i++;
    if (i >= 3) {
      return 1;
    }
  }

  if (mouse_enable_data_report())  // Enables mouse data report
    return 1;

  if (kb_subscribe_int(&bit_no_kb))  // subscribes keyboard interrupt
    return 1;                        // failure

  if (timer_subscribe_int(&bit_no_tm))  // subscribes keyboard interrupt
    return 1;

  if (mouse_subscribe_int(&bit_no_mouse))
    return 1;

  if (rtc_subscribe_int(&bit_no_rtc))
    return 1;

  if (set_rtc_int())
    return 1;

  timer_set_frequency(0, fr_rate);

  int ipc_status, r, irq_set_kb = BIT(bit_no_kb), irq_set_tm = BIT(bit_no_tm),
                     irq_set_mouse = BIT(bit_no_mouse),
                     irq_set_rtc = BIT(bit_no_rtc);
  message msg;

  while (data != ESC_BREAK) {  // while escape key is not pressed

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set_kb) {
            kbc_ih();
            if (data != 0) {
              if (data == TWO_LONG) {
                kbd_data_handler();
                continue;
              }
              kbd_data_handler();
              switch (data) {
                case UP_MAKE:
                  if (menu_on && !pause_on) {
                    if (option == 0)
                      break;
                    else
                      option--;
                  } else if (menu_on && pause_on) {
                    if (option == 0)
                      break;
                    else
                      option -= 2;
                  }
                  up = 1;
                  break;

                case RIGHT_MAKE:
                  rt = 1;
                  break;

                case DOWN_MAKE:
                  if (menu_on && !pause_on) {
                    if (option == 2)
                      break;
                    else
                      option++;
                  } else if (menu_on && pause_on) {
                    if (option == 2)
                      break;
                    else
                      option += 2;
                  }
                  dn = 1;
                  break;

                case LEFT_MAKE:
                  lf = 1;
                  break;

                case UP_BREAK:
                  up = 0;
                  break;

                case RIGHT_BREAK:
                  rt = 0;
                  break;

                case DOWN_BREAK:
                  dn = 0;
                  break;

                case LEFT_BREAK:
                  lf = 0;
                  break;

                case ENTER_MAKE:
                  if (menu_on && !pause_on) {
                    switch (option) {
                      case 0:
                        update_highscore(0);
                        updated = false;
                        p_sprite = sprite;
                        if (!load_game(&medium, &harder, &s_seconds, &s_minutes,
                                       &s_hours, &s_days, &s_months,
                                       &s_years)) {
                          s_seconds = get_seconds();
                          s_minutes = get_minutes();
                          s_hours = get_hours();
                          s_days = get_day();
                          s_months = get_month();
                          s_years = get_year();
                          medium = false;
                          harder = false;
                          p = newPlayer(x, y, sprite->height, sprite->width);
                        }
                        printf("%d\n", loaded);
                        menu_on = false;
                        guia_on = false;
                        break;
                      case 1:
                        menu_on = true;
                        guia_on = true;
                        break;
                      case 2:
                        data = ESC_BREAK;
                        score = 0;
                        for (unsigned i = 0; i < NELEMS(e_array); i++) {
                          delete_enemy(e_array[i]);
                        }
                        for (unsigned i = 0; i < NELEMS(s_array); i++) {
                          delete_shot(s_array[i]);
                        }
                        deletePlayer(p);
                        break;
                      default:
                        break;
                    }
                  } else if (menu_on && pause_on) {
                    switch (option) {
                      case 0:
                        menu_on = false;
                        pause_on = false;
                        break;
                      case 1:
                        menu_on = true;
                        guia_on = true;
                        break;
                      case 2:
                        if (pause_on) {
                          p_sprite = sprite;
                          save_game(medium, harder, s_seconds, s_minutes,
                                    s_hours, s_days, s_months, s_years);
                          score = 0;
                          for (unsigned i = 0; i < NELEMS(e_array); i++) {
                            delete_enemy(e_array[i]);
                          }
                          for (unsigned i = 0; i < NELEMS(s_array); i++) {
                            delete_shot(s_array[i]);
                          }
                          lives = 3;
                          pause_on = false;
                        } else
                          data = ESC_BREAK;
                        break;
                      default:
                        break;
                    }
                  }
                  break;

                case ESC_BREAK:
                  if (pause_on) {
                    save_game(medium, harder, s_seconds, s_minutes, s_hours,
                              s_days, s_months, s_years);
                    pause_on = false;
                    dead = false;
                    able = true;
                    p_sprite = sprite;
                    data = 0;
                    break;
                  } else if (game_over_on) {
                    game_over_on = false;
                    dead = false;
                    able = true;
                    p_sprite = sprite;
                    data = 0;
                    break;
                  } else if (!menu_on) {
                    data = 0;
                    pause_on = true;
                    menu_on = true;
                  }
                  break;
              }
            }
          }

          if (msg.m_notify.interrupts & irq_set_tm) {
            switch (data) {
              case SPACE_MAKE:
                if (!able)
                  break;
                if (time_pressed == 0) {
                  s_sprite = shoot;
                  big = false;
                }
                shooting = 1;
                if (!first_click)
                  space = true;
                first_click = true;
                break;

              case SPACE_BREAK:
                if (!ignore_next) {
                  if (first_click) {
                    nr_shots++;
                    if (nr_shots == 20)
                      nr_shots = 0;
                    shooting = 0;
                    time_pressed = 0;
                  }
                  if (states != left_pressed) {
                    if (nr_shots == 0) {
                      if (s_array[19] != NULL) {
                        s_array[19]->speedX = 3 * mov_per_frame;
                      }
                    } else {
                      if (s_array[nr_shots - 1] != NULL) {
                        s_array[nr_shots - 1]->speedX = 3 * mov_per_frame;
                      }
                    }
                    if (!dead)
                      p_sprite = sprite;
                  }
                  first_click = false;
                }
                ignore_next = false;
                break;
            }

            update_buffer();

            if (menu_on) {  // MENU
              if (game_over_on) {
                draw_menu(&perdeu);
                if (!updated) {
                  updated = true;
                  update_highscore(score);
                  score = 0;
                  for (unsigned i = 0; i < NELEMS(e_array); i++) {
                    delete_enemy(e_array[i]);
                  }
                  for (unsigned i = 0; i < NELEMS(s_array); i++) {
                    delete_shot(s_array[i]);
                  }
                }

                number_to_array(s, scores[0] / 10, 7);
                print_number(s, 1000, 10, 7);

                number_to_array(s, scores[1] / 10, 7);
                print_number(s, 1000, 40, 7);

                number_to_array(s, scores[2] / 10, 7);
                print_number(s, 1000, 70, 7);
              } else if (guia_on) {
                draw_menu(&guia_show);
              } else if (pause_on) {
                draw_menu(&pause);
              } else {
                draw_menu(&main_menu);
              }
              switch (option) {
                case 0:
                  guia_on = false;
                  if (!game_over_on)
                    draw_tecla(&jogar_pressed, option, pause_on);
                  break;
                case 1:
                  if (!guia_on)
                    draw_tecla(&instru_pressed, option, pause_on);
                  game_over_on = false;
                  break;
                case 2:
                  guia_on = false;
                  draw_tecla(&sair_pressed, option, pause_on);
                  game_over_on = false;
                  break;
              }

              if (menu_on && (game_over_on || pause_on)) {
                print_date(seconds, minutes, hours, days, months, years, 10,
                           10);
                print_date(s_seconds, s_minutes, s_hours, s_days, s_months,
                           s_years, 10, 80);
              } else {
                print_date(seconds, minutes, hours, days, months, years, 10,
                           10);
              }
            } else {  // JOGO
              back_frame++;

              if (score < 9999999) {
                score++;
              } else {
                score = 9999999;
              }

              if (score > 2000) {
                medium = true;
              }
              if (score > 4000) {
                harder = true;
              }

              draw_buffer(&fundo2, big, &barra);

              switch (lives) {
                case 3:
                  draw_xpm(&tres_vidas, 990, 780, tres_vidas.width,
                           tres_vidas.height);
                  break;
                case 2:
                  draw_xpm(&duas_vidas, 990, 780, duas_vidas.width,
                           duas_vidas.height);
                  break;
                case 1:
                  draw_xpm(&uma_vida, 990, 780, uma_vida.width,
                           uma_vida.height);
                  break;
                default:
                  break;
              }

              verifyCollision(&big);

              if (shooting)
                time_pressed++;
              if (dead)
                time_dead++;

              if (lives == 0) {
                draw_menu(&perdeu);
                time_dead = 100;
                menu_on = true;
                game_over_on = true;
                lives = 3;
                p->x = 0;
                p->y = 0;
              }

              if (time_dead == 100) {
                p_sprite = sprite;
                time_dead = 0;
                dead = false;
                able = true;
              }

              if (counter % 250 == 0) {
                enemies_handler(medium, harder);
              }

              if (space) {
                p_sprite = sprite_mouth;
                if ((s_array[nr_shots] =
                         new_shot(p->x + 50, p->y + 20, 1, shoot->height,
                                  shoot->width)) == NULL)
                  return 1;
                space = false;
              }

              if (time_pressed == 50) {
                big = true;
                able = false;
                s_sprite = big_shoot;
                s_array[nr_shots]->height = big_shoot->height;
                s_array[nr_shots]->width = big_shoot->width;
                s_array[nr_shots]->x = p->x + 50;
                s_array[nr_shots]->y = p->y + 10;
              }
            }

            if (last_rtc >= 60 && !backup_rtc) {
              printf("\nBACKUPRTC\n");
              backup_rtc = true;
              unset_rtc_int();
            } else if(!backup_rtc) {
              last_rtc++;
            }

            if (backup_rtc && !(counter % fr_rate)) {
              seconds = get_seconds();
              minutes = get_minutes();
              hours = get_hours();
              days = get_day();
              months = get_month();
              years = get_year();
            }

            counter++;
          }

          if (msg.m_notify.interrupts & irq_set_mouse) {
            mouse_ih();
            if (mouse_data_handler())  // After recieving a byte of a packet
                                       // processes it
              continue;
            if (Nbyte == 3) {
              p->x = p->x + pp.delta_x;
              p->y = p->y - pp.delta_y;

              switch (states) {
                case 0:
                  if (pp.rb || !able)
                    break;
                  if (pp.lb) {
                    shooting = 1;
                    space = true;
                    if (time_pressed == 0) {
                      s_sprite = shoot;
                      big = false;
                    }
                    states = left_pressed;
                  }
                  break;

                case 1:
                  if (!pp.lb && !ignore_next) {
                    shooting = 0;
                    nr_shots++;
                    if (nr_shots == 20)
                      nr_shots = 0;
                    states = left_released;
                    time_pressed = 0;
                    p_sprite = sprite;
                  }
                  ignore_next = false;
                  break;

                case 2:
                  if (pp.rb || !able)
                    break;
                  if (pp.lb) {
                    shooting = 1;
                    space = true;
                    if (time_pressed == 0) {
                      s_sprite = shoot;
                      big = false;
                    }
                    states = left_pressed;
                  }
                  break;
              }

              if (nr_shots == 0) {
                if (s_array[19] != NULL && s_array[19]->active) {
                  s_array[19]->speedX = 3 * mov_per_frame;
                }
              } else {
                if (s_array[nr_shots - 1] != NULL &&
                    s_array[nr_shots - 1]->active)
                  s_array[nr_shots - 1]->speedX = 3 * mov_per_frame;
              }

              Nbyte = 0;
            }
          }

          if (msg.m_notify.interrupts & irq_set_rtc) {
            seconds = get_seconds();
            minutes = get_minutes();
            hours = get_hours();
            days = get_day();
            months = get_month();
            years = get_year();
            last_rtc = 0;
            clear_reg_c();
          }

          break;
        default:
          break;
      }
    }
  }
  if (unset_rtc_int())
    return 1;
  if (rtc_unsubscribe_int())
    return 1;

  if (mouse_unsubscribe_int())
    return 1;

  if (timer_unsubscribe_int())
    return 1;

  if (kb_unsubscribe_int())  // unsubscribes keyboard interrupt
    return 1;                // failure

  if (mouse_disable_data_report())  // Set mouse data report to default
    return 1;

  return 0;
}

void update_highscore(uint32_t new_score) {
  FILE* scores_file = fopen("highscores1.txt", "r");
  if (scores_file != NULL) {
    if (!feof(scores_file)) {
      fscanf(scores_file, "%d %d %d", &scores[0], &scores[1], &scores[2]);
    }
  }

  if (new_score > scores[0]) {
    scores[2] = scores[1];
    scores[1] = scores[0];
    scores[0] = new_score;
  } else if (new_score > scores[1]) {
    scores[2] = scores[1];
    scores[1] = new_score;
  } else if (new_score > scores[2]) {
    scores[2] = new_score;
  }

  scores_file = fopen("highscores1.txt", "w");
  fprintf(scores_file, "%d %d %d", scores[0], scores[1], scores[2]);

  fclose(scores_file);
}

void save_game(bool medium,
               bool harder,
               int seconds,
               int minutes,
               int hours,
               int days,
               int months,
               int years) {
  FILE* save_file = fopen("save.txt", "w");
  enemy* e;

  // SAVE ENEMIES
  for (unsigned i = 0; i < 20; i++) {
    e = e_array[i];
    if (e != NULL) {
      fprintf(save_file, "e %u %u %u %u %u %u %d %d %u %c %d\n", e->x, e->y,
              e->og_x, e->og_y, e->height, e->width, e->speedX, e->speedY,
              e->health, e->path, e->active);
    } else {
      fprintf(save_file, "ee\n");
    }
  }
  // SAVE PLAYER
  fprintf(save_file, "p %u %u %u %u\n", p->x, p->y, p->height, p->width);

  // SAVE LIVES SCORE DIFFICULTY COUNTER
  fprintf(save_file, "l %u %u %d %d %u\n", lives, score, medium, harder,
          counter);

  // SAVE START DATE
  fprintf(save_file, "d %u %u %u %u %u %u", seconds, minutes, hours, days,
          months, years);

  fclose(save_file);
}

bool load_game(bool* medium,
               bool* harder,
               int* seconds,
               int* minutes,
               int* hours,
               int* days,
               int* months,
               int* years) {
  FILE* save_file = fopen("save.txt", "r");
  if (save_file != NULL) {
    char line[40];
    unsigned i = 0;

    unsigned u1;
    unsigned u2;
    unsigned u3;
    unsigned u4;
    unsigned u5;
    unsigned u6;
    unsigned u7;
    int i1;
    int i2;
    int i3;
    char c1;

    while (!feof(save_file)) {
      fgets(line, sizeof(line), save_file);
      if (line[0] == 'e' && line[1] != 'e') {
        sscanf(line, "e %u %u %u %u %u %u %d %d %u %c %d", &u1, &u2, &u3, &u4,
               &u5, &u6, &i1, &i2, &u7, &c1, &i3);
        e_array[i] = load_enemy(u1, u2, u3, u4, u5, u6, i1, i2, u7, c1, i3);
        i++;
      } else if (line[0] == 'p') {
        deletePlayer(p);
        sscanf(line, "p %u %u %u %u\n", &u1, &u2, &u3, &u4);
        p = newPlayer(u1, u2, u3, u4);
        loaded++;
      } else if (line[0] == 'l') {
        sscanf(line, "l %u %u %d %d %u\n", &lives, &score, &i1, &i2, &counter);
        *medium = i1;
        *harder = i2;
        loaded++;
      } else if (line[0] == 'd') {
        sscanf(line, "d %u %u %u %u %u %u", seconds, minutes, hours, days,
               months, years);
        loaded++;
      }
    }

    if (loaded < 3)
      return false;

    return true;
  }
  fclose(save_file);
  return false;
}
