
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "board_logic.h"
#include <ncurses.h>

//declares the two boards we need, the current board and set board
int level = 1, lines_cleared = 0; 
char board[BOARD_X][BOARD_Y];
char set_board[BOARD_X][BOARD_Y]; 
                                     
//shapes the correspond to an orientation of a shape
//i.e. orientation state 1 = the first index of the shape, 2 is the second index, etc. 
char *lookup_shapes[NUM_LOOKUP][NUM_COMBINATIONS] = {{"BBBB", "BBBB", "BBBB", "BBBB"},          //bar shape
                                                     {"SSSS", "SSSS", "SSSS", "SSSS"},          //square shape
                                                     {"L..LLL", ".L.LLL", "LLL..L", "LLL.L."},  //top left start L
                                                     {"..LLLL", "L.L.LL", "LLLL..", "LL.L.L"},  //top right start L
                                                     {"Z.ZZ.Z", ".ZZZZ.", "Z.ZZ.Z", ".ZZZZ."},  //zigzag top left
                                                     {".ZZZZ.", "ZZ..ZZ", ".ZZZZ.", "ZZ..ZZ"},  //zigzag top right
                                                     {"TTT.T.", ".TTT.T", ".T.TTT", "T.TTT."}}; //t shape

//function that initilizes the board with a bunch of .'s
void initialize_board() {

  for(int i = 0; i < BOARD_X; i++) {

    for(int j = 0; j < BOARD_Y; j++) {
      
      board[i][j] = '.'; 
    }
  }
}


//function that updates the game score at the top of the window
void update_score(int num_cleared_lines, void *score) {

  //scores that correlate to the real amount of points that you get in the real tetris game 
  //per amount of lines cleared
  int scores[5] = {0, 40, 100, 300, 1200};

  //the score is added to the main score variable which is passed as a refrence from the main
  //and mltiplied by the level you're on
  *((int*)score) += (scores[num_cleared_lines] * level); 
}


//function that clears a specific line and adds to the score accordingly
void clear_line(int starting_pos) {

  for(int j = starting_pos; j--; ) {
    
    for(int z = 0; z < BOARD_Y; z++) {
    
      board[j+1][z] = board[j][z];
    }
  }
}


//function that checks if there's lines to clear
//does so by checking if a line doesn't have any .'s in it and if so it's filled with characters
void check_clear_lines(void *score) {

  int clear_count = 0, local_lines_cleared = 0;
  
  for(int i = BOARD_X - 1; i >= 0; i--) {

    for(int j = BOARD_Y - 1; j >= 0; j--) {

      if(board[i][j] != '.') {

        clear_count++; 
      }
    }

    //if the row is all .'s
    if(clear_count == 10) {
        
      local_lines_cleared++; 
      clear_line(i);
      i++; //decrements the counter as there might be a line to clear that has just been replaced
    }
    clear_count = 0;
  }
  
  lines_cleared += local_lines_cleared; 

  if(lines_cleared / 10 > 0) {

    level++;
    lines_cleared %= 10; 
  }
  update_score(local_lines_cleared, score); 
}

//function that manually places a piece on the board; good for testing
void update_piece(int x, int y, char shape) {

  if(board[x][y] == '.') {
    
    board[x][y] = shape;
  }
  else {
    
    printw("cannot place a character here!");
  }
}



//function that returns true if we can keep playing and false if you lost
bool check_lost_game() {

  for(int i = 0; i < BOARD_Y - 1; i++) {

    //you lost if a character has reached the top row
    if(board[0][i] != '.') {

      return false; 
    }
  }
  return true; 
}



//function that prints the current board
//move is an ncurse function that moves the cursor to print to a specific location
void print_board() {

  move(4, 0); 
  printw("----------------------\n"); 
  for(int i = 0; i < BOARD_X; i++) {

    move(i + 5, 0); 
    for(int j = 0; j < BOARD_Y; j++) {

      printw(" %c", board[i][j]); 
    }
    printw(" |\n");
  }
  printw("----------------------\n"); 
  refresh();
}


//updates the current board with a shape to be drawn
void draw_shape(struct current_piece_s *my_piece) {

  for(int i = 0; i < my_piece->row_col.row; i++) {

    for(int j = 0; j < my_piece->row_col.col; j++) {

      //if the piece has a . in it replace it with the character that's on the set board
      if(my_piece->displayed_piece[i * my_piece->row_col.col + j] == '.') {
      
        board[my_piece->coordinate.x + i][my_piece->coordinate.y + j] = set_board[my_piece->coordinate.x + i][my_piece->coordinate.y + j];
      }
      //otherwise replace the piece on the current board with the corresponding character i.e. B for bar, T for t shape, etc.
      else {

        board[my_piece->coordinate.x + i][my_piece->coordinate.y + j] = my_piece->displayed_piece[i * my_piece->row_col.col + j]; 
      }
    }
  }
}



//function that creates the actual shape by initializing all of its values in the structure
void create_shape(struct current_piece_s *my_piece, struct xy_coord_s cord, int *shape_row_col, int shape_lookup_index, int orientation) {

  //this part is a little wacky due to the fact that the orientation is randomly generated for every shape
  //but basically if the orientation is even then flip the rows and columns and if it's odd keep them the same
  if(orientation % 2 == 0) {

    my_piece->row_col.row = shape_row_col[1];
    my_piece->row_col.col = shape_row_col[0];
  }
  else {

    my_piece->row_col.row = shape_row_col[0];
    my_piece->row_col.col = shape_row_col[1];
  }

  //initilizes information about the shape's lookup index and orientation state
  my_piece->lookup_index = shape_lookup_index; 
  my_piece->orientation_state = orientation; 

  //initializes the shape's x and y position on the board
  my_piece->coordinate.x = cord.x;
  my_piece->coordinate.y = cord.y; 
  
  //initializes whether or not the piece has settled yet
  my_piece->has_settled = false; 

  //will set the displayed piece depending on the shape's orientation state and lookup index
  my_piece->displayed_piece = malloc((my_piece->row_col.row * my_piece->row_col.col) * sizeof(char));
  char *shape = lookup_shapes[my_piece->lookup_index][my_piece->orientation_state];//lookup_shape_decode(shape_lookup_index); 
  strcpy(my_piece->displayed_piece, shape); 

  //draws the shape onto the board
  draw_shape(my_piece); 
}



//function that rotates the shape
void rotate_shape(struct current_piece_s *my_piece) {

  //performs modulus to wrap based on direction rotated
  int state = my_piece->orientation_state;
  state = (state < 0) ? 3 : my_piece->orientation_state % 4; 
  my_piece->orientation_state = state; 

  //swapping rows and columns
  int temp = my_piece->row_col.row; 
  my_piece->row_col.row = my_piece->row_col.col; 
  my_piece->row_col.col = temp; 

  //copying new orientation state to displayed piece
  strcpy(my_piece->displayed_piece, lookup_shapes[my_piece->lookup_index][my_piece->orientation_state]); 
}



//function that checks both left right collision and vertical collision
//will return false if there is a collision and true if there is no collision and the piece is safe to move down
bool check_collision(struct current_piece_s *my_piece, int delta, bool left_right, bool moving_left, int bottom) {

  //if it's left right collision check whether or not it's gone past the column bounds
  if(left_right) {
    
    if(delta <= 0 || delta >= BOARD_Y) {

      return false;
    }
  }

  //if it's vertical collision check if it's gone past the bottom of the board
  else {
  
    if(delta >= bottom) {
      
      return false;
    }
  }
  
  
  //creates a 2D array to store the shape in as the single string got me a little confused working with
  char cur_piece[my_piece->row_col.row][my_piece->row_col.col]; 

  //sets the 2D array equal to the piece's displayed piece
  for(int i = 0; i < my_piece->row_col.row; i++) {

    for(int j = 0; j < my_piece->row_col.col; j++) {

      cur_piece[i][j] = my_piece->displayed_piece[i * my_piece->row_col.col + j];
    }
  }

  
  //checks whether or not a piece has collided with another piece on the set board
  for(int i = 0; i < my_piece->row_col.row; i++) {

    for(int j = 0; j < my_piece->row_col.col; j++) {

      if(cur_piece[i][j] != '.') {

        if(set_board[i + my_piece->coordinate.x][j + my_piece->coordinate.y] != '.') {
          
          return false;
        }
      }
    }
  }

  //checks if you're trying to collide with a piece to the left or right
  if(left_right) {
    
    //left side collision
    if(moving_left) {

      for(int i = 0; i < my_piece->row_col.row; i++) {

        int left_col = my_piece->coordinate.y;
        char c_at_point = board[my_piece->coordinate.x + i][left_col];

        if(c_at_point != '.') {

          //if the character you're colliding with is not a . you have hit a solid object
          if(board[my_piece->coordinate.x + i][left_col - 1] != '.') {

            return false;
          }
        }
      }
    }
    //moving right
    else {

      for(int i = 0; i < my_piece->row_col.row; i++) {

      int right_col = my_piece->coordinate.y + my_piece->row_col.col - 1;
      char c_at_point = board[my_piece->coordinate.x + i][right_col];

      if(c_at_point != '.') {

        //same logic as previously just checking the right bound instead of left 
        if(board[my_piece->coordinate.x + i][right_col + 1] != '.') {

            return false;
          }
        }
      }
    }
  }

  //if it's gotten this far without having collided, it mused be safe to move in its direction
  return true; 
}


//function that checks whether or not a piece can rotate based off of its bounds
//will return true if it can rotate and false if it can't rotate
bool check_rotation(struct current_piece_s *my_piece, int direction) {

  //instead of rotating the actual piece and finding out that it can't actually rotate, I created
  //a new instance of the piece and copied over the necessary information to find out whether it
  //can rotate or not.
  //
  //so if this new piece can rotate, as it has the same information as the other piece, then it must
  //mean that the other piece can also rotate

  struct current_piece_s copy_piece;

  memcpy(&copy_piece, my_piece, sizeof(my_piece)); 
  
  copy_piece.coordinate.x = my_piece->coordinate.x;
  copy_piece.coordinate.y = my_piece->coordinate.y;
  copy_piece.orientation_state = my_piece->orientation_state; 
  copy_piece.row_col.row = my_piece->row_col.row;
  copy_piece.row_col.col = my_piece->row_col.col; 

  //rotates the piece based off the direction
  copy_piece.orientation_state += direction; 
  
  //performs the modulus and rotation
  int state = copy_piece.orientation_state;
  state = (state < 0) ? 3 : copy_piece.orientation_state % 4;
  copy_piece.orientation_state = state;
  rotate_shape(&copy_piece);

  //finds the left bound of the object aka where it is after rotating
  int lb = copy_piece.coordinate.y + copy_piece.row_col.col; 
  
  //if that left bound is greater than the number of columns then the original piece cannot rotate
  if(lb > BOARD_Y) {

    return false;
  }
  else {

    return true; 
  }
}






//function that controls the movement of the shape itself based off of the user input
void move_shape(struct current_piece_s *my_piece, char direction) {

  //declaring and initilizing variable to see if a peice is supposed to rotate
  bool has_rotated = false;

  //left, right, and vertical bounds for the shape depending on it's size and position
  int rb = my_piece->coordinate.y; 
  int lb = my_piece->coordinate.y + my_piece->row_col.col; 
  int vb = my_piece->coordinate.x + my_piece->row_col.row; 

  //moving up which isn't allowed!
  if(direction == 'w') {
    
    //my_piece->coordinate.x--;
  }
  //moving down
  if(direction == 's') {
    
    my_piece->coordinate.x++;

    //checking collision off of falling down without hard dropping
    if(check_collision(my_piece, vb++, false, false, BOARD_X) == false) {

      my_piece->has_settled = true; 
      return; 
    }
  }
  //will move to the left if it's legal
  if(direction == 'a' && check_collision(my_piece, rb--, true, true, 0)) {

    my_piece->coordinate.y--;
  }
  //will mnove to the right if it's legal
  if(direction == 'd' && check_collision(my_piece, lb++, true, false, 0)) {

    my_piece->coordinate.y++; 
  }

  //the following check_rotation functions are known to cause seg faults half of the time which
  //I still do not understand why it will work some complies and not others. For now though, it is 
  //a known bug which when commented out will allow pieces to rotate through the border but for now
  //its better to let the pieces rotate and play the game trying to avoid the bug

  //will rotate the shape left if it's legal
  if(direction == 'q') { //&& check_rotation(my_piece, -1)) {
    
    my_piece->orientation_state--;
    rotate_shape(my_piece); 
  }
  //will rotate the shape right if it's legal
  if(direction == 'e') { //&& check_rotation(my_piece, 1)) {

    my_piece->orientation_state++;
    rotate_shape(my_piece); 
  }
  
  //honestly a very sketchy way of hard dropping a piece but it works
  if(direction == ' ') {

    //moves the piece down until it collides with something
    while(check_collision(my_piece, vb++, false, false, BOARD_X + 1) == true) {
      
      my_piece->coordinate.x++; 
    }
    
    //it will go past one spot so we have to correct for that by going back one
    memcpy(board, set_board, sizeof(char) * BOARD_X * BOARD_Y);
    my_piece->coordinate.x--;
    draw_shape(my_piece);
    print_board();

    my_piece->has_settled = true;
    return; 
  }


  //gets rid of the previous piece on the board by copying the set board to the new board
  memcpy(board, set_board, sizeof(char) * BOARD_X * BOARD_Y);
  
  //draws the shape on the new board and prints that board
  draw_shape(my_piece);
  print_board();
}

      


/***********************************/
