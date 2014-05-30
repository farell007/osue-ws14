/**
 * @file gamelogic.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief The implementation of the game logic of a 2048 game
 * @date 29.04.2014
 */

#include "gamelogic.h"

/* === PROTOTYPES === */

/**
 * @brief move the tile of the field to the specified direction if possible
 * @param field the field to get updated
 * @param x the x coordinate of the tile to move
 * @param y the y coordinate of the tile to move
 * @param dir the direction to move
 * @param has_moved if a tile moves it gets true
 */
static void move(struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X], 
	int x, int y, struct direction dir, bool *has_moved);

/**
 * @brief this function returns true if the coordinates are in the field
 * @param x the x coordinate of the tile
 * @param y the y coordinate of the tile
 * @return true if the coordinates are in the field
 */
static bool is_in_field(int x, int y);

/**
 * @brief adds a new tile on a free tile on the field
 * @param field the field that need a new random tile
 * @return ST_LOST if no fields are left to add a new tile or ST_ON if everything went well
 */
static unsigned int new_number_field(struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X]);

/* === CONSTANTS === */

/**
 * @brief direction to move a tile up
 */
 const static struct direction up 		= {0,-1};
/**
 * @brief direction to move a tile down
 */
 const static struct direction down 	= {0,1};
/**
 * @brief direction to move a tile left
 */
 const static struct direction left 	= {-1,0};
/**
 * @brief direction to move a tile right
 */
 const static struct direction right 	= {1,0};

/* === IMPLEMENTATIONS === */

static unsigned int new_number_field(struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X])
{
    int number_zero_fields = 0;
    for(int y = 0; y < FIELD_SIZE_Y; ++y){
        for(int x = 0; x < FIELD_SIZE_X; ++x){
            if(field[y][x].val == 0){
                ++number_zero_fields;
            }
        }
    }
    if(number_zero_fields == 0){
    	//Error
    	return ST_LOST;
    	DEBUG("NO FIELDS LEFT!\n");
    } else{
	    int r = rand() % number_zero_fields + 1;
	    int power = 2;
	    if((rand() % 4) < 3){
	        power = 1;
	    }
	    for(int y = 0; y < FIELD_SIZE_Y; ++y){
	        for(int x = 0; x < FIELD_SIZE_X; ++x){
	            if(field[y][x].val == 0){
	                if(--r==0){
	                    field[y][x].val = power;
	                }
	            }
	        }
	    }
	}

	return ST_ON;
}

static bool is_in_field(int x, int y)
{
	return (x >= 0 && x < FIELD_SIZE_X 
		&& y >= 0 && y < FIELD_SIZE_Y);
}

static void move(struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X], 
	int x, int y, struct direction dir, bool *has_moved)
{
	int start_x = x;
	int start_y = y;
	if(field[y][x].val == 0) {
		return;
	}

	//get next available tile
	do{
		x += dir.x;
		y += dir.y;
	} while(is_in_field(x,y)
		&& field[y][x].val == 0);
	x -= dir.x;
	y -= dir.y;
	

	if(x != start_x || y != start_y)
	{
		//move tile to next available tile
		DEBUG("MOVE FIELD (%d,%d) TO (%d,%d)\n",start_x,start_y,x,y);
		field[y][x].val = field[start_y][start_x].val;
		field[y][x].has_merged = field[start_y][start_x].has_merged;
		field[start_y][start_x].val = 0;
		field[start_y][start_x].has_merged = false;
		*has_moved = true;
		if(is_in_field(start_x-dir.x,start_y-dir.y)){
			move(field,start_x-dir.x,start_y-dir.y,dir,has_moved);
		}
	}
	//merge
	if(is_in_field(x+dir.x,y+dir.y)
	 && field[y][x].val == field[y+dir.y][x+dir.x].val
	 && field[y+dir.y][x+dir.x].has_merged == false
	 && field[y][x].has_merged == false)
	{
		DEBUG("MERGE FIELD (%d,%d) TO (%d,%d)\n",x,y,y+dir.y,x+dir.x);
		field[y][x].val = 0;
		field[y+dir.y][x+dir.x].val++;
		field[y+dir.y][x+dir.x].has_merged = true;
		*has_moved = true;
		if(is_in_field(x-dir.x,y-dir.y)){
			move(field,x-dir.x,y-dir.y,dir,has_moved);
		}
	}
}

void new_game(struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X])
{
	for(int y = 0; y < FIELD_SIZE_Y; ++y){
        for(int x = 0; x < FIELD_SIZE_X; ++x){
            field[y][x].val = 0;
            field[y][x].has_merged = false;
        }
    }

    new_number_field(field);
}

unsigned int move_numbers_field(
	struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X], 
	unsigned int command, 
	unsigned int power_of_two)
{
	struct direction dir;
	switch(command){
		case CMD_LEFT:	
			DEBUG("MOVE LEFT!\n");
			dir = left;
			break;
		case CMD_RIGHT:	
			DEBUG("MOVE RIGHT!\n");
			dir = right;
			break;
		case CMD_UP:	
			DEBUG("MOVE UP!\n");
			dir = up;
			break;
		case CMD_DOWN:	
			DEBUG("MOVE DOWN!\n");
			dir = down;
			break;		
		case CMD_DELETE:
		 	return ST_DELETE;
		case CMD_DISCONNECT:	
		 	return ST_HALT;	
	}

	bool has_moved = false;

	for(int y = 0; y < FIELD_SIZE_Y; ++y){
        for(int x = 0; x < FIELD_SIZE_X; ++x){
            move(field,x,y,dir,&has_moved);
        }
    }

    if(has_moved == false){
    	DEBUG("NO SUCH GAME!\n");
    	return ST_NOSUCHGAME;
    }


    for(int y = 0; y < FIELD_SIZE_Y; ++y){
        for(int x = 0; x < FIELD_SIZE_X; ++x){
        	//reset merges
        	field[y][x].has_merged = false;
        	//check for win
            if(field[y][x].val == power_of_two){
            	return ST_WON;
            }
        }
    }

    return new_number_field(field);
}