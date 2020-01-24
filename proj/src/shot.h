/** @defgroup shot shot
 * @{
 *
 */

#include <lcom/lcf.h>

/**
 * @brief Struct type for the features of a shot
 */
typedef struct {
  unsigned x;      /*!< position on the x axis */
  unsigned y;      /*!< position on the y axis */
  unsigned height; /*!< heigth */
  unsigned width;  /*!< width */

  int speedX;      /*!< speed in the x axis */
  int speedY;      /*!< speed in the y axis */
  bool active;     /*!< indicating whethter the shot is active or not */
  unsigned damage; /*!< damage caused by the shot */
} shot;

/**
 * @brief Creates a new shot
 * @param x		 Position on the x axis where the shot is created
 * @param y		 Position on the y axis where the shot is created
 * @param damage  Damage caused by the shot
 * @param hg		 Shots height
 * @param wd		 Shots width
 * @return Return struct type shot
 */
shot* new_shot(unsigned x,
               unsigned y,
               unsigned damage,
               unsigned hg,
               unsigned wd);

/**
 * @brief Deletes a previous created shot
 * @param s		 Previous created shot
 * @return Return 0 upon success and 1 otherwise
 */
int delete_shot(shot* s);

/**@}*/
