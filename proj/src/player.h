/** @defgroup player player
 * @{
 *
 */
#include <lcom/lcf.h>

/**
 * @brief Struct type for the features of a player
 */
typedef struct {
  unsigned x;      /*!< position on the x axis */
  unsigned y;      /*!< position on the y axis */
  unsigned height; /*!< heigth */
  unsigned width;  /*!< width */

  int speedX; /*!< speed in the x axis */
  int speedY; /*!< speed in the y axis */
} player;

/**
 * @brief Creates a new player
 * @param x		 Position on the x axis where the shot is created
 * @param y		 Position on the y axis where the shot is created
 * @param hg		 Players height
 * @param wd		 Players width
 * @return Return struct type player
 */
player* newPlayer(unsigned x, unsigned y, unsigned hg, unsigned wd);

/**
 * @brief Deletes a previous created player
 * @param p		 Previous created player
 */
void deletePlayer(player* p);

/**@}*/
