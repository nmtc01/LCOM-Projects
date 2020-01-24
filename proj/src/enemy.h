/** @defgroup enemy enemy
 * @{
 *
 */
#include <lcom/lcf.h>

/**
 * @brief Struct type for the features of an enemy
 */
typedef struct {
  unsigned x;    /*!< position on the x axis */
  unsigned y;    /*!< position on the y axis */
  unsigned og_x; /*!< initial position on the x axis */
  unsigned og_y; /*!< initial position on the y axis */

  unsigned height; /*!< heigth */
  unsigned width;  /*!< width */

  int speedX;      /*!< speed in the x axis */
  int speedY;      /*!< speed in the y axis */
  unsigned health; /*!< enemys health */

  char path; /*!< path of the enemy */

  bool active; /*!< indicating whethter the enemy is active or not */
} enemy;

/**
 * @brief Creates a new enemy
 * @param x		 Position on the x axis where the shot is created
 * @param y		 Position on the y axis where the shot is created
 * @param health  Health
 * @param wde	 Enemys width
 * @param hge	 Enemys height
 * @param path	 Enemys path
 * @return Return struct type enemy
 */
enemy* new_enemy(unsigned x,
                 unsigned y,
                 unsigned health,
                 unsigned wde,
                 unsigned hge,
                 char path);

/**
 * @brief Deletes a previous created enemy
 * @param e		 Previous created enemy
 */
void delete_enemy(enemy* e);

/**
 * @brief Load enemy
 * @param x x position of the enemy
 * @param y y position of the enemy
 * @param og_x starting x position
 * @param og_y starting y position
 * @param height enemy height
 * @param width enemy width
 * @param speedX enemy speed in x
 * @param speedY enemy speed in y
 * @param health enemy health
 * @param pathenemy path
 * @param active enemy active
 */
enemy* load_enemy(unsigned x,
                  unsigned y,
                  unsigned og_x,
                  unsigned og_y,
                  unsigned height,
                  unsigned width,
                  int speedX,
                  int speedY,
                  unsigned health,
                  char path,
                  bool active);

/**@}*/
