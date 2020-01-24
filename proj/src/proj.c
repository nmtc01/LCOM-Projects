// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include "graphi.h"

// Any header files included below this line should have been created by you

int main(int argc, char* argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int argc, char* argv[]) {
  if (argc == 0) {
    argv[0] = 0;
  }
  FILE* save_file = fopen("save.txt", "w");
  fclose(save_file);
  vg_init(VIDEO_MODE);
  xpm_image_t dragon2;
  xpm_image_t shoot2;
  xpm_image_t dragon_mouth;
  xpm_image_t enemy;
  xpm_image_t big_shoot;
  xpm_image_t exploded;

  if (xpm_load(dragon, 2, &dragon2) == NULL)
    return 1;

  if (xpm_load(shoot, 2, &shoot2) == NULL)
    return 1;

  if (xpm_load(big_shot, 2, &big_shoot) == NULL)
    return 1;

  if (xpm_load(mouth_open, 2, &dragon_mouth) == NULL)
    return 1;

  if (xpm_load(phantom, 2, &enemy) == NULL)
    return 1;

  if (xpm_load(explosion, 2, &exploded) == NULL)
    return 1;

  escape_or_move(&dragon2, &dragon_mouth, &shoot2, &big_shoot, &enemy,
                 &exploded, 26, 28, 10, 30);
  vg_exit();
  return 0;

  // iniciar o modo grafico
  // importar as sprites para as structs
  // chamar a funçao no graphic
  // terminar
}
