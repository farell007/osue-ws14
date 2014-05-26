/**
 * @file commands.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief TODO
 * @date 26.04.2014
 */

#ifndef dp_commands_h /*prevent multible inclusion*/ 
#define dp_commands_h

/* === CLIENT COMMANDS === */
 #define CMD_LEFT		(0)
 #define CMD_RIGHT		(1)
 #define CMD_UP			(2)
 #define CMD_DOWN		(3)
 #define CMD_DELETE		(4)
 #define CMD_DISCONNECT	(5)
 #define CMD_UNSET		(6)

/* === SERVER STATUS CODES === */

 #define ST_WON			(0)
 #define ST_LOST		(1)
 #define ST_NOSUCHGAME	(2)
 #define ST_ON 			(3)

#endif /*ifndef dp_commands_h*/
