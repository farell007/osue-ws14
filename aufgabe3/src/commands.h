/**
 * @file commands.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief Commands for the 2048 game
 * @date 26.04.2014
 */

#ifndef dp_commands_h /*prevent multible inclusion*/ 
#define dp_commands_h

 #include <stdint.h>
 #include <stdbool.h>

/* === CLIENT COMMANDS === */
/**
 * @def CMD_LEFT
 * @brief Command to move the field left
 */
#define CMD_LEFT		(0)
/**
 * @def CMD_RIGHT
 * @brief Command to move the field right
 */
#define CMD_RIGHT		(1)
/**
 * @def CMD_UP
 * @brief Command to move the field up
 */
#define CMD_UP			(2)
/**
 * @def CMD_DOWN
 * @brief Command to move the field down
 */
#define CMD_DOWN		(3)
/**
 * @def CMD_DELETE
 * @brief Command to delete the game
 */
#define CMD_DELETE		(4)
/**
 * @def CMD_DISCONNECT
 * @brief Command to disconnect the game
 */
#define CMD_DISCONNECT	(5)
/**
 * @def CMD_UNSET
 * @brief Command that means that the game has not received a comman yet
 */
#define CMD_UNSET		(6)

/* === SERVER STATUS CODES === */

 /**
  * @def ST_WON
  * @brief Status that the game is won
  */
 #define ST_WON			(0)
  /**
   * @def ST_LOST
   * @brief Status that the game is lost
   */
 #define ST_LOST		(1)
  /**
   * @def ST_NOSUCHGAME
   * @brief Status that there were no moves done with the last command
   */
 #define ST_NOSUCHGAME	(2)
  /**
   * @def ST_ON
   * @brief Status that the game is on
   */
 #define ST_ON 			(3)
  /**
   * @def ST_DELETE
   * @brief Status that the game has been deleted
   */
 #define ST_DELETE 		(4)
   /**
    * @def ST_HALT
    * @brief Status that the game has been halted
    */
 #define ST_HALT 		(5)

/* === GAME LOGIC CONSTANTS  === */
    
/**
 * @def FIELD_SIZE_X
 * @brief The size of the field in x axis
 */
#define FIELD_SIZE_X			(4)
/**
  * @def FIELD_SIZE_Y
  * @brief The size of the field in y axis
  */
#define FIELD_SIZE_Y			(4)

/**
 * @brief the tile of a field
 */
struct tile{
	/**
	 * @brief the value of the tile
	 */
	uint8_t val;
	/**
	 * @brief indicates if the tile has been merged in this round
	 */
	bool has_merged;
};

#endif /*ifndef dp_commands_h*/
