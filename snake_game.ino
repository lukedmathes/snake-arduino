/*--------------------------------------------------------------------------------------------

  Snake Game Module for Ardiuno Due and Adafruit SSD1306 128x64 LCD screen.


  By Luke Mathes, 1 April 2016

  --------------------------------------------------------------------------------------------

  This module runs a basic Snake game. The user controls the snake with four button inputs
  corresponding to the cardinal directions.

  Rules:

  The player must move the Snake around the game area and obtain the fruit without running
  into the boundaries or itself. The more fruit the snake gets, the longer its tail becomes
  and the more difficult it is to navigate without colliding with itself.

  The player can move the snake in the four cardinal directions; left, right, up and down. The
  snake cannot go backwards. The player and fruit are started at random locations every game.
    
  --------------------------------------------------------------------------------------------

  The snake_game function is called from the main funciton and will persist until the player
  loses. Display functions are handled by the snake_display module and button presses are
  handled by the snake_input module.

  The game makes use of a two-dimensional array called game_board which serves to show where
  the snake has been. While this method takes up reasonable chunk of memory, it allows the
  game to handle situations up to the Snake covering the entire screen.
  
  Each integer in the array indicates how many more moves the snake should make before the
  corresponding space should be turned off. By setting each space the player moves over to the
  player score plus a constant initial length, this gives the illusion that the snake is
  slithering along.

  The main snake loop requires a delay to allow user input, this delay is provided by the
  get_direction_input function from the snake_input module, although the delay is an argument
  and can be controlled from this module to adjust difficulty.
  
  ------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------
  Defines
  ------------------------------------------------------------------------------------------*/
#define INITIAL_LENGTH 4



/*--------------------------------------------------------------------------------------------
  snake_game()
    Description:
      Snake game function. Called from main() or Arduino loop() and not exited until game is
      completed. Function initialises local variables, draws board and starting positions, and
      then goes into main Snake Loop. Main Snake Loop checks if player position is valid,
      checks if player has moved to a fruit, clears old locations, gets user input, and then
      moves the snake head in the inputted direction.
      Function takes a wait between snake movements as an argument so difficulty can be
      adjusted from caller function.
    Inputs:
      Debounced user input through get_direction_input.
      Wait between each movement of the snake measured in milliseconds.
    Outputs:
      Prints snake, fruit and score to LCD screen through display handler functions.
  ------------------------------------------------------------------------------------------*/
void snake_game(uint16_t input_wait)
{
  // Initialise variables
  int8_t game_board[X_COORD_LIMIT][Y_COORD_LIMIT] = {0};
  int8_t score = 0;
  coordinates player;
  player.x_coord = random( 5, X_COORD_LIMIT - 5 );
  player.y_coord = random( 5, Y_COORD_LIMIT - 5 );
  player.player_direction = RIGHT;
  coordinates fruit;

  // Default for this application is 500ms
  //uint16_t input_wait = 500;

  // Generate new fruit location
  generate_fruit_location(&fruit, &player, game_board);

  //Draw game board, player location, and fruit location
  display_draw_board();
  display_draw_new(player);
  display_draw_new(fruit);
  display_print_score(score);
  display_print_direction(player.player_direction);


  // Main Snake Loop
  // On every iteration, check if player is within game board bounds and has not run into
  // themselves
  while ( location_valid( &player, game_board))
  {
    // Fruit is checked before setting current location on board to prevent issues with score
    // and game board values
    if (player_at_fruit( &player, &fruit ))
    {
      // When player is at fruit location, update score and generate new fruit
      score++;
      display_print_score(score);
      generate_fruit_location(&fruit, &player, game_board);
      // As score is incremented, all spaces must also be incremented by one. This is
      // effectively done by skipping the decrementing of non-zero spaces in board_clear_old()
      // during this loop.
    }
    else
    {
      // Clear old spaces by decrementing all non-zero counters in game_board array
      board_clear_old(game_board);
    }
    // Add current location to board with updated score added to initial length of snake
    game_board[player.x_coord][player.y_coord] = score + INITIAL_LENGTH;

    get_direction_input( &player, input_wait );
    move_player(&player);
  }
  display_print_score(88);
  delay(500);
}

/*--------------------------------------------------------------------------------------------
  generate_fruit_location()
    Description:
      Randomly generates a new X and Y coordinate for the fruit to be collected. Called
      whenever the player obtains the fruit.
      Function will loop until a valid position that the snake does not currently occupy is
      generated.
    Inputs:
      Player location so fruit is not placed on player location.
      Game board array so fruit is not placed on location snake currently occupies.
    Outputs:
      X and Y coordinates for fruit.
      Prints to LCD display new fruit at generated location.
  ------------------------------------------------------------------------------------------*/
void generate_fruit_location(struct coordinates * new_fruit, const struct coordinates * player_location, int8_t game_board[X_COORD_LIMIT][Y_COORD_LIMIT])
{
  do
  {
    // Generate new fruit location based on random value
    new_fruit->x_coord = random(X_COORD_LIMIT);
    new_fruit->y_coord = random(Y_COORD_LIMIT);
    // Loop if location is already occupied by snake
    // Both the game_board location and the current player location must be checked as the
    // current player location has not yet been stored in game board array
  } while ((0 != game_board[new_fruit->x_coord][new_fruit->y_coord]) &&
           ((new_fruit->x_coord != player_location->x_coord) &&
             (new_fruit->y_coord != player_location->y_coord)));
  display_draw_new(*new_fruit);
}

/*--------------------------------------------------------------------------------------------
  move_player()
    Moves the player's X and Y coordinates in the direction that the snake is facing, then
    prints the new location to the screen.
  ------------------------------------------------------------------------------------------*/
void move_player( struct coordinates * player )
{
  switch (player->player_direction)
  {
    case LEFT:
      (player->x_coord)--;
      break;
    case RIGHT:
      (player->x_coord)++;
      break;
    case UP:
      (player->y_coord)--;
      break;
    case DOWN:
      (player->y_coord)++;
      break;
    case NONE:
    default:
      break;
  }
  display_draw_new(*player);
}

/*--------------------------------------------------------------------------------------------
  board_clear_old()
    Iterates through every integer in the game_board array and decrements it if it is greater
    than zero.If the decremented integer is now zero or below, clear the display at that
    location.
    Used for clearing the end of the snake as it moves forward and maintaining a constant
    snake leangth.
  ------------------------------------------------------------------------------------------*/
void board_clear_old(int8_t game_board[X_COORD_LIMIT][Y_COORD_LIMIT])
{
  // Iterate through all X coordinates
  for (int8_t i_x = 0; X_COORD_LIMIT > i_x; i_x++)
  {
    // Iterare through all Y coordinates
    for (int8_t i_y = 0; Y_COORD_LIMIT > i_y; i_y++)
    {
      // If greater than zero, snake has passed of space and time spent there is decremented
      if (0 < game_board[i_x][i_y])
      {
        game_board[i_x][i_y]--;
        // If time is now zero, clear that space
        if (0 >= game_board[i_x][i_y])
        {
          // Awful 4 layer deep nesting
          display_clear_space( i_x, i_y );
        }
      }
    }
  }
}

/*--------------------------------------------------------------------------------------------
  player_at_fruit()
    Checks if player location is equal to fruit location and returns a boolean.
    Could be inlined at main snake loop, mostly given own function for abstraction purposes.
  ------------------------------------------------------------------------------------------*/
bool player_at_fruit(const struct coordinates * fruit, const struct coordinates * player)
{
  return (( player->x_coord == fruit->x_coord ) && ( player->y_coord == fruit->y_coord ));
}

/*--------------------------------------------------------------------------------------------
  location_valid()
    Checks if player location is within the confines of the game board and player is not
    moving over space already occupied.
    Occupied space is indicated by game_board coordinate being greater than zero.
  ------------------------------------------------------------------------------------------*/
bool location_valid( const struct coordinates * player_data, int8_t game_board[X_COORD_LIMIT][Y_COORD_LIMIT])
{
  // Player has gone beyond the limits of the game board
  // Limits for x are 0 inclusive and X_COORD_LIMIT exclusive, limits for y are 0 inclusive
  // and Y_COORD_LIMIT exclusive
  if (( 0 > player_data->x_coord ) || ( 0 > player_data->y_coord ) ||
      ( X_COORD_LIMIT <= player_data->x_coord ) || ( Y_COORD_LIMIT <= player_data->y_coord ))
  {
    return false;
  }
  // Game board space already occupied by previous section of snake
  if ( 0 < game_board[player_data->x_coord][player_data->y_coord] )
  {
    return false;
  }
  // If location is valid
  return true;
}


