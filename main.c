#include <stdio.h>
#include "board_logic.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <signal.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#define ROW_COL 2           //one for row, one for column
#define STDIN 0             //needed for the timer
#define TIMER 1000000       //time in microseconds for how long to wait between each iteration  
#define NUM_TO_GENERATE 5   //number of pieces to generate
#define MAX_PIECE_SIZE 4    //largest piece size


/*
 * 2D integer array of a shape's number of rows and number of columns that correlates to its 
 * respective lookup shape
 *
 * i.e. the bar shape_row_col index matches with the lookup_shape index for the bar 
 * 
 */

int shape_row_col[NUM_LOOKUP][ROW_COL] = {{4, 1}, //bar
                                          {2, 2}, //square
                                          {3, 2}, //left start L
                                          {3, 2}, //right start L
                                          {2, 3}, //zig top left 
                                          {2, 3}, //zig bottom left
                                          {3, 2}}; //t shape


/* 
 * structure that will store five game pieces in an array
 * 
 * used when showing upcoming pices
 *
 */ 
struct game_pieces_s {

  struct current_piece_s piece_arr[NUM_TO_GENERATE]; 
};




/*
 * function that creates and returns a new instance of a shape
 *
 * used to efficiently create shapes
 *
 */ 
struct current_piece_s generate_piece() {

  int rand_start = rand() % 3 + 3; //random starting y position (column)
  struct xy_coord_s starting_position = {1, rand_start};

  struct current_piece_s my_piece; 
  int rand_shape = rand() % NUM_LOOKUP; //random num 0-6
  int rand_orientation = rand() % NUM_COMBINATIONS; //random num 0-3

  create_shape(&my_piece, starting_position, shape_row_col[rand_shape], rand_shape, rand_orientation); 
  return my_piece; 
}

/*
 * function that generates the five starting pieces for the game
 */

void create_starting_pieces(struct game_pieces_s *game_pieces) {

  //NUM_TO_GENERATE is set to 5 but can generate more if needed
  for(int i = 0; i < NUM_TO_GENERATE; i++) {
    
    struct current_piece_s temp_piece; 
    temp_piece = generate_piece(); 

    //puts the new pieces into the array of pieces
    game_pieces->piece_arr[i] = temp_piece; 
  }
}




/*
 * function that prints the next piece given the index of that piece and a starting position to 
 * print from
 */
void print_next(struct game_pieces_s *game_pieces, int index, int starting_x) {
 
  struct current_piece_s next_piece = game_pieces->piece_arr[index]; 
  
  //puts the piece to display into a new 2D arry
  //when I did this it worked and when I took it out it didn't so I'm going to leave it in for now
  char to_display[MAX_PIECE_SIZE][MAX_PIECE_SIZE]; 
  

  //initilizes the 2D array that stores the piece to print
  move(starting_x, 25);
  for(int i = 0; i < MAX_PIECE_SIZE; i++) {

    move(starting_x + i, 25);
    for(int j = 0; j < MAX_PIECE_SIZE; j++) {

      to_display[i][j] = ' '; 
      printw(" %c", to_display[i][j]); 
    }
  }

  //fills the 2D array with the piece shape and when there isn't piece it turns into a space
  for(int i = 0; i < next_piece.row_col.row; i++) {

    for(int j = 0; j < next_piece.row_col.col; j++) {

      if(next_piece.displayed_piece[i * next_piece.row_col.col + j] == '.') {

        to_display[i][j] = ' '; 
      }
      else {

        to_display[i][j] = next_piece.displayed_piece[i * next_piece.row_col.col + j];
      } 
    }
  }

  //prints the 2D array storing the piece shape in its designatd spot 
  move(starting_x, 25);
  for(int i = 0; i < next_piece.row_col.row; i++) {

    move(starting_x + i, 25);
    for(int j = 0; j < next_piece.row_col.col; j++) {

      printw(" %c", to_display[i][j]); 
    }
  }

  //refreshes/updates the window
  refresh(); 
}


/*
 * function that prints the four next pieces primarily using the print_next function
 */
void print_next_pieces(struct game_pieces_s *game_pieces, int starting_x) {

  move(starting_x - 2, 24);
  printw("Upcoming piece is:");
  
  //I found that increments of five give a comfortable distance between the shapes
  print_next(game_pieces, 1, starting_x); 
  print_next(game_pieces, 2, starting_x + 5); 
  print_next(game_pieces, 3, starting_x + 10);
  print_next(game_pieces, 4, starting_x + 15); 
}



/*
 * function that prints the instructions of how to play at the bottom of the screen
 */
void print_instructions() {

  move(32, 0); 
  printw("q = Rotate Left     e = Rotate Right\na = Move Left       d = Move Right\ns = Move Down       space = Hard Drop"); 
  refresh(); 
}



/*
 * function that will go to the next piece by shifting everything in the array to the right by 1
 * the newly generated piece is added at the end of the main
 *
 */ 
void next_piece(struct game_pieces_s *game_pieces) {

  for(int i = 0; i < NUM_TO_GENERATE; i++) {

    game_pieces->piece_arr[i] = game_pieces->piece_arr[i + 1]; 
  }
}



/*
 * main function that utilizes necessary functions to operate the game
 */ 
int main() { 
  
  //score keeps track of score
  //keep playing controls if the game is over or not
  int score = 0; 
  bool first_run = true;
  char c;

  //ncurses functions that deal with displaying the board, fluid key input, and timers
  srand(time(0));
  initscr();                      //initilizes the screen
  noecho();                       //gets rid of key input if it's pressed
  struct timeval before, after; 
  gettimeofday(&before, NULL); 
  nodelay(stdscr, TRUE); 


  //creates our array of pieces and initilizes it with 5 pieces
  struct game_pieces_s game_pieces;
  struct current_piece_s my_piece; 
  create_starting_pieces(&game_pieces);

  //initilizes the board and the set board as well as prints instructions
  initialize_board();
  memcpy(set_board, board, sizeof(char) * BOARD_X * BOARD_Y);
  print_instructions(); 


  //plays the game till you've lost aka a character has reached the top of the board
  while(check_lost_game()) {
    
    //prints the score at the top of the screen
    move(1, 0); 
    printw("your current score is %d\nyour current level is %d\nnumber of lines cleared is %d", score, level, lines_cleared);
    
    //prints the board off of the first run so the screen isn't empty at first
    if(first_run) {

      print_board(); 
      first_run = false; 
    }

    //creates a new piece to put at the end of the piece array
    //I don't know what I need it here specifically but if it's anywhere else it breaks
    struct current_piece_s ending_piece;
    ending_piece = generate_piece(); 
    
    my_piece = game_pieces.piece_arr[0];
    
    //will run logic to keep the piece moving down until it has settled
    while(!my_piece.has_settled) {

      //prints the next pieces on the side of the board
      //I know it spazzes out because it's in this while loop but anywhere else it only prints once
      //for a fraction of a second and never again till the next run 
      print_next_pieces(&game_pieces, 10); 
      
      //moves the shape depending on the character pressed
      if((c = getch()) != ERR) {
 
        move_shape(&my_piece, c);
      }
      
      gettimeofday(&after, NULL); 
      
      //if the time has surpassed one second, it will move the piece down regardless
      //as the levels increase, the time to drop decreases
      if(((double)(after.tv_sec - before.tv_sec)*1000000 + (double)(after.tv_usec - before.tv_usec)) > TIMER / level) {

        before = after;
        move_shape(&my_piece, 's');
      }
    }
    

    //shifts the next pieces in the piece array
    next_piece(&game_pieces);  
    game_pieces.piece_arr[NUM_TO_GENERATE - 1] = ending_piece;

    //checks if any lines were cleared and adds it to the score
    check_clear_lines(&score);

    //copys the current board to the set board as a new piece has been set
    memcpy(set_board, board, sizeof(char) * BOARD_X * BOARD_Y);
    refresh();
  }
  
  //the only way to exit the loop is if you lost so if you lost it prints a message and your score
  printw("You lost! Your final score is %d!", score);
  refresh(); 
  return 0;
}
















