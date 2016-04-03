/*--------------------------------------------------------------------------------------------

  Snake Input Module for Ardiuno Due and Adafruit SSD1306 128x64 LCD screen.


  By Luke Mathes, 2 April 2016

  --------------------------------------------------------------------------------------------

  This module interfaces the player input to the main Snake game. Player input is done through
  four directional input buttons that move the snake in the corresponding cardinal direction.

  The module takes the direction the player is currently facing and the time to delay between
  snake movements as inputs and returns the direction the player has inputted. This module
  provides the main delay between each movement of the snake.
    
  --------------------------------------------------------------------------------------------

  Debouncing is done through a shift and add method. Whenever the detected input is active the
  corresponding register is shifted left one and a 1 added to it effectively placing a 1 on
  the right end. Whenever an input is detected as not active, the register is returned to
  zero.

  The reason for this method is that it is incredibly easy to detect a rising edge while
  debouncing an input. If the function required five consecutive active reads to detect the
  input, the register would be 0b00011111 at the earliest possible time and an output can be
  given from this read.

  The difference between this method and using an integer to count is that an integer can
  overflow and would give a second input if the button was held down for sufficiently long.
  The shift and add method will read 0b11111111 when the button is held down and would not
  return an additional input unless the button was released.
  
  ------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------
  Defines
  ------------------------------------------------------------------------------------------*/
// Debouncing times measured in milliseconds.
// Interval and total time for debouncing can both be changed, but total_time should be a
// multiple of interval. For debounce mask purposes, total_time should be longer than
// interval but not over 16 times larger.
#define DEBOUNCING_INTERVAL 5
#define DEBOUNCING_TOTAL_TIME 20

// Button locations can be redefined here
#define BUTTON_LEFT 31
#define BUTTON_RIGHT 33
#define BUTTON_UP 35
#define BUTTON_DOWN 37


// If buttons are active-low, define BUTTON_INVERT here
#define BUTTON_INVERT
// Sets active state of buttons depending on configuration.
#ifdef BUTTON_INVERT
#define BUTTON_PRESSED LOW
#else
#define BUTTON_PRESSED HIGH
#endif

// Used to check if DEBOUNCE_MASK will be zero or 0xFFFF and will not pick up a rising edge.
#if(( 0 == DEBOUNCING_TOTAL_TIME/DEBOUNCING_INTERVAL ) || ( 16 <= DEBOUNCING_TOTAL_TIME/DEBOUNCING_INTERVAL ))
#error("Debounce Mask is either too large or too small!");
#endif

/*--------------------------------------------------------------------------------------------
  Variables
  ------------------------------------------------------------------------------------------*/
  // Stored as variable to prevent calculation being completed on every call.
  const uint16_t DEBOUNCE_MASK = (1 << (DEBOUNCING_TOTAL_TIME/DEBOUNCING_INTERVAL)) - 1;
  


/*--------------------------------------------------------------------------------------------
  input_init()
    Sets defined button pin locations to input and turns off pull-up resistors if applicable.
    Can be called from the start of Snake game function or during setup phase of
    microcontroller.
  ------------------------------------------------------------------------------------------*/
void input_init(void)
{
  // Set all input pins to input and digital low to guarantee pullup resistor is off.
  pinMode( BUTTON_LEFT, INPUT );
  pinMode( BUTTON_RIGHT, INPUT );
  pinMode( BUTTON_UP, INPUT );
  pinMode( BUTTON_DOWN, INPUT );
  digitalWrite( BUTTON_LEFT, LOW );
  digitalWrite( BUTTON_RIGHT, LOW );
  digitalWrite( BUTTON_UP, LOW );
  digitalWrite( BUTTON_DOWN, LOW );
}




/*--------------------------------------------------------------------------------------------
  get_direction_input()
    Descriptions:
      Recieves user input through a debouncer and checks if it is a valid direction for snake
      to travel before returning the direction to the main loop. This function also performs
      the main delay between each move of the snake.
    Inputs:
      Previous direction snake was travelling.
      Total wait time.
    Outputs:
      Newly inputted direction.
      Program delays the given amount of time.
  ------------------------------------------------------------------------------------------*/
void get_direction_input ( struct coordinates * player_data, int16_t input_wait )
{
  // Variable new_input keeps track of the newly inputted direction while the player_data
  // variable keeps track of the old direction.
  // Old direction is retained to guarantee that multiple valid inputs in one cycle will not
  // allow snake to go backwards.
  snake_direction new_input = player_data->player_direction;
  // Temp value used to store debounced values before passing to caller function
  snake_direction debounced_input = NONE;

  // Total delay between snake movements is variable, but time between each debouncing check
  // must be 5ms (DEBOUNCING_INTERVAL).
  // With each iteration taking 5ms, the loop is run total_delay/5ms times.
  for ( int16_t i = 0; (input_wait / DEBOUNCING_INTERVAL) > i; i++)
  {
    delay(DEBOUNCING_INTERVAL);
    debounced_input = receive_debounced_input();
    // Must check if button has been pressed and is valid direction relative to old input
    if ((NONE != debounced_input) && 
        is_not_opposite_direction( player_data->player_direction, debounced_input))
    {
      new_input = debounced_input;
      display_print_direction(new_input);
    }
  }
  // Due to integer rounding, a small delay of the remainder is done after the loop.
  delay(input_wait % DEBOUNCING_INTERVAL);
  // If delay is <5ms debouncing will not occur and game will be unplayable, but with the
  // snake moving once every 5ms the game is already unplayable for humans.

  // Set returned player_data direction variable to enterred variable
  player_data->player_direction = new_input;
}

/*--------------------------------------------------------------------------------------------
  receive_debounced_input()
    Descriptions:
      Iterates through each button and checks if it is debounced by using
      input_check_debounced(). If a debounced rising edge is detected, function returns that
      direction. Otherwise, it returns NONE.
      This function also keeps the static debounce variables so each variable can be passed to
      input_check_debounced() without additional checks.
    Inputs:
      User input through buttons
    Outputs:
      Returns NONE when no debounced button detected.
      Returns a direction when a debounced rising edge is detected.
  ------------------------------------------------------------------------------------------*/
enum snake_direction receive_debounced_input(void)
{
  static uint16_t debounce_left = 0;
  static uint16_t debounce_right = 0;
  static uint16_t debounce_up = 0;
  static uint16_t debounce_down = 0;

  // While an order of preference is present here, due to the rising edge detection of the
  // buttons and the latching nature once an input is received, it is rarely relevant.
  if (input_check_debounced(&debounce_left, BUTTON_LEFT))
  {
    return LEFT;
  }
  if (input_check_debounced(&debounce_right, BUTTON_RIGHT))
  {
    return RIGHT;
  }
  if (input_check_debounced(&debounce_up, BUTTON_UP))
  {
    return UP;
  }
  if (input_check_debounced(&debounce_down, BUTTON_DOWN))
  {
    return DOWN;
  }
  // Return NONE for most cases where an input should not be changed.
  return NONE;
}

/*--------------------------------------------------------------------------------------------
  input_check_debounced()
    Description:
      Checks the given input pin and debounces the input. If the pin is high, it shifts and
      increments the associated given integer and then checks it against the debounce mask.
      If the pin is low, the integer is set to zero.
      The shift and increment method is used as it easily identifies consecutive high inputs
      and will not overflow to zero as simply incrementing an integer would.
      The DEBOUNCE_MASK value has a number of low bits in the most significant bits followed
      by all high bits for the remainder. This picks up a rising edge where the input has
      transitioned from low to high.
      The number of high bits is controlled by DEBOUNCING_TOTAL_TIME and DEBOUNCING_INTERVAL.
    Inputs:
      Location of variable and Pin to be debounce tested.
    Outputs:
      Boolean indicating whether the given button for the given input is a rising edge and
      has been debounced.
  ------------------------------------------------------------------------------------------*/
bool input_check_debounced(uint16_t * debounce_value, const int8_t pin)
{
  if (BUTTON_PRESSED == digitalRead(pin))
  {
    // Shift and increment integer to add a 1 to the right side of the buffer.
    *debounce_value <<= 1;
    *debounce_value += 1;
    if (DEBOUNCE_MASK == *debounce_value)
    {
      return true;
    }
  }
  else
  {
    // Set to zero is a low is detected to guarantee a series of high inputs will be preceded
    // by low bits. For example 0b001010111111 would not be detected, but if each low bit
    // clears the value then this will never occur
    *debounce_value = 0;
  }
  return false;
}


/*--------------------------------------------------------------------------------------------
  is_opposite_direction()
    Checks if the two given directions are in oppositiong to each other.
    Used for determining if a debounced value is in opposition to the direction the snake
    was just travelling and is therefore invalid.
  ------------------------------------------------------------------------------------------*/
bool is_not_opposite_direction( enum snake_direction old_direction, enum snake_direction new_direction)
{
  switch (old_direction)
  {
    case LEFT:
      if (RIGHT == new_direction)
      {
        return false;
      }
      break;
    case RIGHT:
      if (LEFT == new_direction)
      {
        return false;
      }
      break;
    case UP:
      if (DOWN == new_direction)
      {
        return false;
      }
      break;
    case DOWN:
      if (UP == new_direction)
      {
        return false;
      }
      break;
  }
  return true;
}
