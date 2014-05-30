/**
 * @file gamelogic.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief The header file of the game logic of a 2048 game
 * @date 29.04.2014
 */

#ifndef dp_gamelogic_h /*prevent multible inclusion*/ 
#define dp_gamelogic_h

#include "commands.h"
#include <stdlib.h>
#include <stdio.h>

/* === MACROS === */

/**
 * @def DEBUG(...)
 * @brief Prints formatted debug message to stderr
 */
#ifdef ENDEBUG /*Flag to set from compiler for debugging*/
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__);} while(0)
#else
#define DEBUG(...)
#endif

/* === DEFS === */

/**
 * @brief a struct that encapsulates the direction of a move
 */
 struct direction{
 	/**
 	 * @brief x {-1,0,1}
 	 */
 	int x;
 	/**
 	 * @brief y {-1,0,1}
 	 */
 	int y;
 };

/* === PROTOTYPES === */

/**
 * @brief move the field by the given command
 * @param field the field to get updated
 * @param command the command that updates the field
 * @param power_of_two the challange to win
 * @return the new game status
 */
unsigned int move_numbers_field(
	struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X], 
	unsigned int command, 
	unsigned int power_of_two);

/**
 * @brief creats a new field
 * @param field the field that gets reset and one new random tile
 */
void new_game(struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X]);

#endif /*ifndef dp_gamelogic_h*/
