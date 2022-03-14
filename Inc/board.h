#include "string.h"
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_ts.h"
#include "math.h"
#include "buffer.h"
#include "main.h"

/*
 *  DEFINITIONS
 */
#define MAX_ALLOWED_X (770)
#define MAX_ALLOWED_Y (450)
#define BORDER_SIZE (30)
#define AIR_HOCKEY_PAD_RADIUS (40)
#define AIR_HOCKEY_PAD_BUFFER_SIZE (2)
#define AIR_HOCKEY_BALL_RADIUS (30)
#define AIR_HOCKEY_BALL_BUFFER_SIZE (4)
#define MAIN_COLOR_DARK ((uint32_t) 0xffbd93f9)
#define MAIN_COLOR_VIOLET ((uint32_t) 0xff282a36)
#define R_0 ((uint8_t) 189)
#define G_0 ((uint8_t) 147)
#define B_0 ((uint8_t) 249)
#define R_1 ((uint8_t) 173)
#define G_1 ((uint8_t) 255)
#define B_1 ((uint8_t) 47)
#define A_u ((uint8_t) 255)


/*
* ENUMS
*/

typedef enum {
   SINGLEPLAYER = 0,
   MULTIPLAYER = 1    
} GameMode;

typedef enum {
    NOT_RUNNING = 0,
    RUNNING = 1,
    PAUSED = 2,
} GameStatus;


/*
 * DEFINITIONS OF STRUCTURES
 */
typedef struct Game_ {
    uint8_t player_one_score, player_two_score; // left player is player one
    uint16_t hockey_ball_coordinate_x, hockey_ball_coordinate_y;
    int8_t ball_speed_x, ball_speed_y;
    uint8_t current_color;
    GameStatus game_status;
    GameMode game_mode;
} Game;

typedef struct Color_ {
    uint8_t r, g, b, a; 
} Color;

typedef struct Mssg_ {
    uint8_t contents[BUFF_SIZE];
    uint8_t size;
} Mssg;

typedef struct SystemStatus_ {
    uint8_t is_ethernet_connected;
    Color background_color;
    Color elements_color;
}SystemStatus;

typedef struct Menu_{
    uint8_t menu_header_text[20];
    uint8_t first_option_text[20];
    uint8_t second_option_text[20];
    uint8_t third_option_text[20];
} Menu;

/*
 * EXPORTED VARIABLES
 */
extern uint8_t lcd_status;
extern uint32_t ts_status;
extern TS_StateTypeDef TS_State;
extern Game game;


/*
 * EXPORTED FUNCTIONS
 */

void init_display(void);

void draw_board_borders(void);

void draw_game_field(uint8_t draw_borders);

void clear_game_field(void);

void handle_bottom_corner_event(Game *game, Menu *menu, Buffer *air_left_air_hockey_pad,
		Buffer *air_right_air_hockey_pad,
		Buffer *air_hoceky_ball);

void draw_main_menu(void);

void draw_generic_menu(Menu *menu);

void draw_winner(uint8_t player);

void draw_player_score(Game *game);

void send_current_score_to_PC(uint8_t *score, uint8_t score_size);

void draw_a_partial_circle(uint16_t coordinate_x, uint16_t coordinate_y,
		uint16_t radius, uint16_t start, uint16_t end, uint32_t color);

void draw_air_hockey_pad(BufferedElement *hockey_pad);

void clear_air_hockey_pad_from_buffer(Buffer *hockey_pad_buffer);

void clear_air_hockey_pad(BufferedElement *position);

void draw_hockey_ball(BufferedElement *hockey_ball);

void clear_hockey_ball_from_buffer(Buffer *hockey_ball_buffer);

void clear_hockey_ball(BufferedElement *position);

uint8_t is_menu_button_pressed(TS_StateTypeDef touchscreen_state,
		int touch_index, int startx, int starty, uint8_t strlen, int strwid);

uint8_t is_touch_in_game_field(TS_StateTypeDef touchscreen_state,
		int touch_index);

uint8_t is_touch_in_bottom_menubar(TS_StateTypeDef touchscreen_state,
    int touch_index);

uint8_t is_touch_in_upper_menubar(TS_StateTypeDef touchscreen_state,
    int touch_index);

uint8_t is_ball_in_a_goal(Game *game);

void display_winner_and_reset_game_to_main_menu(Game *game);

 void send_winner_to_PC(uint8_t player);

void process_pause_menu_click_event(TS_StateTypeDef touchscreen_state, Menu *menu, Game *game);

void process_main_menu_click_event(TS_StateTypeDef touchscreen_state, Menu *menu, Game *game);

void switch_board_colors(Game *game);

void process_single_in_game_tap(TS_StateTypeDef touchscreen_state);

void process_double_in_game_tap(TS_StateTypeDef touchscreen_state);

void process_display_event(TS_StateTypeDef touchscreen_state, Game *game);



/*
* GAME FUNCTIONS
*/

void start_game(void);

void reset_game(void);

void pause_game(void);

void move_ball(void);

void handle_collisions(Game *game);

/*
* MISC FUNCTIONS
*/

void send_message_to_PC(uint8_t *message, uint8_t message_length);

uint8_t get_random_number(uint32_t *random_number);

uint32_t convert_color_to_rgba_int_value(Color *color);


// todo 
// --> add ability to change colors as u wish and send score
