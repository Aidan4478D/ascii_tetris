#ifndef BOARD_LOGIC_H
#define BOARD_LOGIC_H
#include <stdbool.h>

//constants
#define BOARD_X 24
#define BOARD_Y 10
#define NUM_LOOKUP 7
#define NUM_COMBINATIONS 4

//variables from board_logic.c that can be useful in other files
extern char board[BOARD_X][BOARD_Y];
extern char set_board[BOARD_X][BOARD_Y]; 
extern char *lookup_shapes[NUM_LOOKUP][NUM_COMBINATIONS];
extern int level; 
extern int lines_cleared; 

//structure for the x y coordinate info
struct xy_coord_s {
  
  int x;
  int y; 
}; 

//structure for row and column info
struct row_col_s {
  
  int row;
  int col;
};

//structure for piece info
struct current_piece_s {
  
  int lookup_index; 

  char *displayed_piece; 
  int orientation_state; 

  struct row_col_s row_col; 
  struct xy_coord_s coordinate; 

  bool has_settled; 
};

//initial and basic board functions
void initialize_board();
void update_piece(int x, int y, char shape);
void print_board();

//functions that keep track of score and win/loss
bool check_lost_game(); 
void check_clear_lines(void *score); 
void clear_line(int starting_pos);
void update_score(int num_cleared_lines, void *score); 

//functions that relate to controlling the shape
void create_shape(struct current_piece_s *my_piece, struct xy_coord_s cord, int *shape_row_col, int shape_lookup_index, int orientation); 
char *lookup_shape_decode(int index);
void move_shape(struct current_piece_s *my_piece, char direction);
void draw_shape(struct current_piece_s *my_piece); 
void rotate_shape(struct current_piece_s *my_piece); 

//shape checks
bool check_collision(struct current_piece_s *my_piece, int delta, bool left_right, bool moving_left, int bottom);
bool check_rotation(struct current_piece_s *my_piece, int direction);
#endif /* BOARD_LOGIC_H */
