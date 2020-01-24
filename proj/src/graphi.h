/** @defgroup graphic graphic
 * @{
 *
 */

#include <lcom/lcf.h>
#include <math.h>
#include <string.h>

#include "enemy.h"
#include "player.h"
#include "shot.h"

#include "kb.h"
#include "mouse.h"
#include "rt.h"
#include "time.h"

#include "xpms.h"

// Global variables
extern xpm_image_t bola;
extern xpm_image_t one;
extern xpm_image_t two;
extern xpm_image_t three;
extern xpm_image_t four;
extern xpm_image_t five;
extern xpm_image_t six;
extern xpm_image_t seven;
extern xpm_image_t eight;
extern xpm_image_t nine;
extern bool able;

// Macros
#define BIOS_VIDEO_SERV 0x10
#define RET_VBE_CTRL 0x4F00
#define RET_VBE_MODE 0x4F01
#define SET_VBE_MODE 0x4F02
#define SET_DEFAULT_TEXT_MODE 0x0003
#define VIDEO_MODE 0x14B
#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))

// COLOURS
#define BLACK 0x00000000
#define WHITE 0xFFFFFFFF
#define BLUE 0xFF000000
#define GREEN 0x00FF0000
#define RED 0x0000FF00
#define BACKGROUND 0x258796
#define IGNORE_BYTE 0x80

/**
 * @brief Enumerated type for identifying the mouse states
 */
enum mouse_states {
  zero,         /*!< initial state, nothing happens */
  left_pressed, /*!< left button pressed */
  left_released /*!< left button released */
};

/**
 * @brief Auxiliary function used by function void *(vg_init) to help setting up
 * the pretended graphics mode
 *
 * @param mode	Pretended graphics mode
 */
int ret_graphics_mode(uint16_t mode);

/**
 * @brief Draws a xpm image on the screen on the specified position, and with
 *the specified width and height
 *
 * @param xpm	XPM image to be drawn
 *		 x		Position on the x axis of the left-up corner of
 *the xpm y		Position on the y axis of the left-up corner of the xpm
 *width XPMs width height	XPMs height
 */
int draw_xpm(xpm_image_t xpm[],
             uint16_t x,
             uint16_t y,
             uint16_t width,
             uint16_t height);

/**
 * @brief Uses memcpy() to copy (h_res * v_res * byte_per_pixel) characters from
 * second_buffer to video_mem, ensuring the data updating
 */
void update_buffer();

/**
 * @brief Function that handles the movement of an enemy
 *
 * @param e	Enemy to be moved
 */
void move_enemy(enemy* e);

/**
 * @brief Function that handles the movement of a shot
 *
 * @param s	Shot to be moved
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int move_shot(shot* s);

/**
 * @brief Function that handles the movement of the player
 */
void move_player();

/**
 * @brief Function that handles all the drawing of the xpms and make sure the
 *buffer is updated
 *
 * @param fundo	Background to be drawn
 *		 big	Bool variable indicating whether the missile used is the
 *large or the small one barra  Board to be drawn
 */
int draw_buffer(xpm_image_t fundo[], bool big, xpm_image_t barra[]);

/**
 * @brief Draws the background and ensures its movement
 *
 * @param xpm	Background to be drawn
 */
void draw_buffer_background(xpm_image_t xpm[]);

/**
 * @brief Verifies collisions between enemies and missiles
 *
 * @param big			Bool variable indicating whether the missile
 * used is the large or the small one
 */
void verifyCollision(bool* big);

/**
 * @brief Draws the selected button between the three: "jogar", "intrucoes" and
 *"sair"
 *
 * @param tecla			XPM image of the button to display
 *		 nr				Integer indicating which button
 *is being displayed (between 0 and 2)
 *		 pause_on		Boolean variable indicating wheter the
 *game is paused or not
 */
void draw_tecla(xpm_image_t tecla[], int nr, bool pause_on);

/**
 * @brief Decrements players characters lives if it isnt dead
 *
 * @param explosion		XPM image of the players explosion
 *
 * @return Return true if players character is dead, false if it is alive
 */
bool remove_life(xpm_image_t explosion[]);

/**
 * @brief Draws main menu
 *
 * @param sprite			XPM image of the main_menu
 */
void draw_menu(xpm_image_t main_menu[]);

/**
 * @brief Main function that handles the gameplay
 *
 * @param sprite			XPM image of the players character
 *        sprite_mouth	XPM image of the players character with mouth open
 *        shoot			XPM image of the missile
 *        big_shoot		XPM image of the larger missile
 *        ghost			XPM image of the enemy
 *        exploded		XPM image of the players explosion
 *        x				Initial position on the x axis of the
 * player y				Initial position on the y axis of the
 * player mov_per_frame	Players character speed fr_rate		Frame rate
 *
 * @return Return 0 upon success and non-zero otherwise
 */
int escape_or_move(xpm_image_t sprite[],
                   xpm_image_t sprite_mouth[],
                   xpm_image_t shoot[],
                   xpm_image_t big_shoot[],
                   xpm_image_t ghost[],
                   xpm_image_t exploded[],
                   uint16_t x,
                   uint16_t y,
                   uint16_t mov_per_frame,
                   uint8_t fr_rate);

/**
 * @brief Function that prints the score on the screen
 *
 * @param number		Array of chars with the numbers to be printed
 *		 x			Position on the x axis of the left-up
 *corner of the xpm y			Position on the y axis of the left-up
 *corner of the xpm max_number	Number of digits of the score
 */
void print_number(char number[], uint16_t x, uint16_t y, unsigned max_number);

/**
 * @brief Function that prints the date on the screen
 *
 * @param seconds	Seconds
 *		 minutes	Minutes
 *		 hours		Hours
 *		 days		Day
 *		 months		Month
 *		 years		Year
 *		 x			Position on the x axis
 *		 y			Position on the y axis
 */
void print_date(int seconds,
                int minutes,
                int hours,
                int days,
                int months,
                int years,
                uint16_t x,
                uint16_t y);

/**
 * @brief Function that keeps in an array the actual score by splitting the
 *score into digits
 *
 * @param digits		Array of chars where the digits are saved
 *		 number		Number which ensures the right digit is saved on
 *the right position of the array n_digits	Number of digits of the score
 */
void number_to_array(char digits[], unsigned number, unsigned n_digits);

/**
 * @brief Function that handles the path of an enemy and makes it move according
 * to a sine function
 *
 * @param e	Enemy to be moved
 */
int sin_pattern(enemy* e);

/**
 * @brief Function that handles the path of an enemy and makes it move in a
 * straight line with a certain slope
 *
 * @param e	Enemy to be moved
 */
int cross_pattern(enemy* e);

/**
 * @brief Function that handles generation of enemies and their paths
 *
 * @param medium Medium level of enemy generation
 *		 harder Hard level of enemy generation
 */
void enemies_handler(bool medium, bool harder);

/**
 * @brief Updates the file that stores highscores
 *
 * @param new_score the new score achieved
 */
void update_highscore(uint32_t new_score);

/**
 * @brief Save game to a file
 * @param medium difficulty
 * @param harder difficulty
 * @param seconds seconds of starting date
 * @param minutes minutes of starting date
 * @param hours hour of starting date
 * @param days day of starting date
 * @param months month of starting date
 * @param years year of starting date
 */
void save_game(bool medium,
               bool harder,
               int seconds,
               int minutes,
               int hours,
               int days,
               int months,
               int years);

/**
 * @brief Load game to a file
 * @param medium difficulty
 * @param harder difficulty
 * @param seconds seconds of starting date
 * @param minutes minutes of starting date
 * @param hours hour of starting date
 * @param days day of starting date
 * @param months month of starting date
 * @param years year of starting date
 * @return Return false if failed true otherwise
 */
bool load_game(bool* medium,
               bool* harder,
               int* seconds,
               int* minutes,
               int* hours,
               int* days,
               int* months,
               int* years);

/**@}*/
