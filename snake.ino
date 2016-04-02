#define DEBUG

/*--------------------------------------------------------------------------------------
  Includes
  --------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------
  Defines
  --------------------------------------------------------------------------------------*/

// Limits for x and y coordinates
#define X_COORD_LIMIT 36
#define Y_COORD_LIMIT 19


/*--------------------------------------------------------------------------------------
  Variables
  --------------------------------------------------------------------------------------*/
// Cardinal directions used to move
enum snake_direction{ NONE, LEFT, RIGHT, UP, DOWN };
struct coordinates
{
  int8_t x_coord;
  int8_t y_coord;
  snake_direction player_direction;
};


/*--------------------------------------------------------------------------------------
  setup()
  Called by the Arduino framework once, before the main loop begins
  --------------------------------------------------------------------------------------*/
void setup() {
  display_init();
  input_init();

  // Initialise random seed
  pinMode( A1, INPUT);
  digitalWrite( A1, LOW );
  randomSeed( analogRead(A1) );
  digitalWrite( A1, HIGH );
  pinMode( A1, OUTPUT);
}


/*--------------------------------------------------------------------------------------
  loop()
  Arduino main loop
  --------------------------------------------------------------------------------------*/
void loop() {

  
  snake_game(500);
 

}
