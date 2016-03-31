/*--------------------------------------------------------------------------------------

  Snake Display Module for Ardiuno Due and Adafruit SSD1306 128x64 LCD screen.


  By Luke Mathes, 31 March 2016

  Uses Adafruit graphics and SSD1306 drivers.
      https://github.com/adafruit/Adafruit_SSD1306
  --------------------------------------------------------------------------------------

  The display is pixel controlled with the game board area divided into spaces. The game board
  is surrounded by a one pixel thick rectangle with the player score to the right of the game
  board.

  The game board is divided into repeating squares 2 pixels high and wide. Each square has one
  pixel worth of padding between it and the next square on all sides creating a repeating 9x9
  pattern as below.

    O O X
    O O X
    X X I

  The O symbols above signify the main space of the square, the X symbols are the connecting
  pixels which can visually connect two spaces, and the I symbol is an unused pixel. Each space
  must have these padding X and I symbols on all sides, so an additional row and column of these
  are placed on the top and left of the game board.

  The rectangle surrounding the game board is spaced one pixel from top, left and bottom with
  a larger space on the right to accomodate the player score. The rectangle also has a one pixel
  thick buffer between it and the start of the game board.

  The player score to the right of the game board is represented by a two digit number and printed
  at size 1 (each character 5 pixels wide by 7 high).
    
  --------------------------------------------------------------------------------------

  When each space is drawn by calling display_draw_new(), the direction in which the snake is
  travelling can also be passed. The module will then draw the two pixels between the current
  space and the previous space to connect spaces and show the path of the snake.

  When each space is erased, the surrounding connecting pixels which may have been drawn will
  also need to be cleared. This is done by drawing a 4x4 black filled rectangle over the space
  and it's surrounding pixels clearing all connecting pixels.
  
  --------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


/*--------------------------------------------------------------------------------------
  Defines
  --------------------------------------------------------------------------------------*/
// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


/*--------------------------------------------------------------------------------------
  display_init()
    LCD display initialisation for Adafruit SSD1306 128x64 LCD screen.
    Called from Arduino setup() function.
  --------------------------------------------------------------------------------------*/
void display_init(void)
{
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  display.clearDisplay();
}


/*--------------------------------------------------------------------------------------
  display_draw_new()
    Descriptions:
      Draws snake to LCD screen at the given location. Takes the updated x and y coordinates as well as a direction.
      Fills in the four pixels at the corresponding location as well as the pixels on the opposite side of the direction
      given to link the previous location to the current location.
      Can also print a square at any given coordinate for starting positions or fruit by passing the NONE direction.
    Inputs:
      X and Y coordinates to be printed.
      Direction which the snake travelled to get to the current position.
    Outputs:
      Prints snake to LCD screen.
  --------------------------------------------------------------------------------------*/
void display_draw_new(struct coordinates player)
{
  // Check if passed coordinates are out of bounds
  if (( X_COORD_LIMIT < player.x_coord ) || ( Y_COORD_LIMIT < player.y_coord ))
  {
    //Coordinates are out of bounds, return to main function
#ifdef DEBUG
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("ERROR: COORD OOB");
    display.display();
    while (1);
#endif
    return;
  }

  // Set pixel location to corresponding coordinate location
  uint8_t x_location = coord_to_pixel(player.x_coord);
  uint8_t y_location = coord_to_pixel(player.y_coord);
  // x and y location is top left corner of square, print rectangle at location
  display.drawRect(x_location, y_location, 2, 2, WHITE);
  // Print connecting pixels at location determined by direction moved
  switch (player.player_direction)
  {
    case LEFT:
      // Draw pixels to right of current location
      display.drawPixel(x_location + 2, y_location, WHITE);
      display.drawPixel(x_location + 2, y_location + 1, WHITE);
      break;
    case RIGHT:
      // Draw pixels to left of current location
      display.drawPixel(x_location - 1, y_location, WHITE);
      display.drawPixel(x_location - 1, y_location + 1, WHITE);
      break;
    case UP:
      // Draw pixels below current location
      display.drawPixel(x_location, y_location + 2, WHITE);
      display.drawPixel(x_location + 1, y_location + 2, WHITE);
      break;
    case DOWN:
      // Draw pixels above current location
      display.drawPixel(x_location, y_location - 1, WHITE);
      display.drawPixel(x_location + 1, y_location - 1, WHITE);
      break;
    case NONE:
    default:
      // Used for fruits to be collected and initial starting positions which do not require padding
      break;
  }
  display.display();
}

#define SCORE_LOCATION_X 115
#define SCORE_LOCATION_Y 5
#define HEX_ASCII_OFFSET 0x30
#define SCORE_WIDTH 11
#define SCORE_HEIGHT 7
/*--------------------------------------------------------------------------------------
  display_print_score()
    Descriptions:
      Prints the current player score on the right side of the screen. The old score must first be cleared
      by printing a solid black rectangle at the specified location. The number is then divided into the tens
      and singles portions and printed by adding an ASCII offset to the number.
    Inputs:
      Integer score to be printed.
    Outputs:
      Prints passed score to LCD screen at defined location.
  --------------------------------------------------------------------------------------*/
void display_print_score(int8_t score)
{
  //If greater than 99 or below zero, display 99 and 0 respectively.
  if (99 < score)
  {
    score = 99;
  }
  if (0 > score)
  {
    score = 0;
  }
  // Clear previous score
  display.fillRect(SCORE_LOCATION_X, SCORE_LOCATION_Y, SCORE_WIDTH, SCORE_HEIGHT, BLACK);
  // Format text size and location
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(SCORE_LOCATION_X, SCORE_LOCATION_Y);
  // Print first digit
  display.write(score / 10 + HEX_ASCII_OFFSET);
  // Print second digit
  display.write(score % 10 + HEX_ASCII_OFFSET);
  display.display();
}


/*--------------------------------------------------------------------------------------
  display_clear_space()
    Descriptions:
      Clears the passed board location. Called by cleanup function to remove old spaces.
      Prints a black filled rectangle over the central pixels, the outer connecting pixels, and the
      corner padding pixels.
    Inputs:
      X and Y coordinates of space to be cleared.
    Outputs:
      Changes pixels on LCD screen at given coordinate location to black.
  --------------------------------------------------------------------------------------*/
void display_draw_board(int8_t x_arg, int8_t y_arg)
{
  uint8_t x_coord = coord_to_pixel(x_arg);
  uint8_t y_coord = coord_to_pixel(y_arg);
  // Coordinate-1 is used to set starting location for rectangle in top left padding pixel
  display.drawRect(x_coord - 1, y_coord - 1, 4, 4, BLACK);
  display.display();
}


// Define where the rectanglular game board should be
#define BOARD_BOUND_LEFT 1
#define BOARD_BOUND_TOP 1
#define BOARD_BOUND_RIGHT 113
#define BOARD_BOUND_BOTTOM 62


/*--------------------------------------------------------------------------------------
  display_draw_board()
    Clears the display and prints a blank game board.
  --------------------------------------------------------------------------------------*/
void display_draw_board(void)
{
  display.clearDisplay();
  display.drawRect(BOARD_BOUND_LEFT, BOARD_BOUND_TOP, BOARD_BOUND_RIGHT, BOARD_BOUND_BOTTOM, WHITE);
  display.display();
}

/*--------------------------------------------------------------------------------------
  coord_to_pixel()
    Takes a coordinate integer and returns the corresponding location of the top-left pixel
    Pixels have four spaces padding on the top and left of the screen and are 3 pixels apart.
  --------------------------------------------------------------------------------------*/
uint8_t coord_to_pixel(int8_t coord)
{
  return ( coord * 3 ) + 4;
}
