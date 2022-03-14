#include "board.h"

/*
 * VARIABLES
 */

uint8_t lcd_status = LCD_OK;
uint32_t ts_status = TS_OK;
TS_StateTypeDef TS_State = { 0 };
Buffer hockey_pad_buffer_left_player = { 0 };
Buffer hockey_pad_buffer_right_player = { 0 };
Buffer hockey_ball_buffer = { 0 };
Game game = { 0 };

Menu main_menu = { .menu_header_text = "MAIN MENU", .first_option_text =
		"singleplayer", .second_option_text = "multiplayer",
		.third_option_text = "random color", };

Menu pause_menu = { .menu_header_text = "PAUSED", .first_option_text =
		"continue game", .second_option_text = "reset game",
		.third_option_text = "main menu", };

uint32_t MAIN_COLOR = MAIN_COLOR_DARK;
uint32_t SECONDARY_COLOR = MAIN_COLOR_VIOLET;

/*** DISPLAY DRAWING FUNCTIONS ***/

/*
 * this function will initialize display
 */
void init_display(void) {
	BSP_LCD_Init();

	BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
	BSP_LCD_Clear(SECONDARY_COLOR);

	ts_status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
	while (ts_status != TS_OK)
		;

	ts_status = BSP_TS_ITConfig();
	while (ts_status != TS_OK)
		;
}

/*
 * this function draws game borders
 * */
void draw_board_borders(void) {
	BSP_LCD_Clear(SECONDARY_COLOR);
	//  left borders
	BSP_LCD_FillRect(0, 0, BORDER_SIZE, 180);
	BSP_LCD_FillRect(0, 300, BORDER_SIZE, 180);
	// upper borders
	BSP_LCD_FillRect(0, 0, 300, BORDER_SIZE);
	BSP_LCD_FillRect(500, 0, 300, BORDER_SIZE);
	// right borders
	BSP_LCD_FillRect(MAX_ALLOWED_X, 0, BORDER_SIZE, 180);
	BSP_LCD_FillRect(MAX_ALLOWED_X, 300, BORDER_SIZE, 180);
	// bottom borders
	BSP_LCD_FillRect(0, MAX_ALLOWED_Y, 300, BORDER_SIZE);
	BSP_LCD_FillRect(500, MAX_ALLOWED_Y, 300, BORDER_SIZE);
}

/*
 * (0.0)                            800
 *   |- - - - - - - - - - - - - -           - - - - - - - - - - - - - - - |
 *   |----------------------------        ------------------------------| |<------------- 30
 *   | |                                                                | |    1
 *   | |                                                                | |    8
 *   | |                                                                | |    0
 *   | |                                                                | |<------------- 180
 *                               (400.240)                                       4
 *                                   *                                           8
 *                                                                               0
 *   | |                                                                | |<------------- 300
 *   | |                                                                | |
 *   | |                                                                | |    1
 *   | |                                                                | |    8
 *   | |--------------------------       -------------------------------| |<---0-------- 450
 *   |_ _ _ _ _ _ _ _ _ _ _ _ _ _         _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ |
 *              300             ^         ^                    300
 *                              |         |
 *                             300       500
 *
 * this function will draw main game field, if parameter draw_borders is set to 1, the borders will bi redrawn also
 **/
void draw_game_field(uint8_t draw_borders) {
	BSP_LCD_SetTextColor(MAIN_COLOR);
	if (draw_borders)
		draw_board_borders();

	for (uint8_t i = 0; i < 4; i++) {
		BSP_LCD_DrawCircle(400, 240, 90 - i); // center circle
		BSP_LCD_DrawCircle(400, 240, i + 1);
		draw_a_partial_circle(0, 240, 90 - i, 0, 180, MAIN_COLOR); // left goal
		draw_a_partial_circle(800, 240, 90 - i, 180, 360, MAIN_COLOR); // right goal
	}
}

/*
 * this function will clear game field
 */
void clear_game_field(void) {
	BSP_LCD_Clear(SECONDARY_COLOR);
}

/*
 * Bottom corner click event handler
 **/
void handle_bottom_corner_event(Game *game, Menu *menu,
		Buffer *air_left_air_hockey_pad, Buffer *air_right_air_hockey_pad,
		Buffer *air_hoceky_ball) {

	// game is now paused
    pause_game();

	// first clear pads from screen
	while (air_left_air_hockey_pad->num_of_elements > 0)
		clear_air_hockey_pad(air_left_air_hockey_pad->front);
	while (air_right_air_hockey_pad->num_of_elements > 0)
		clear_air_hockey_pad(air_right_air_hockey_pad->front);

	clear_game_field();
	draw_generic_menu(menu);
}

/*
 * this function will draw main menu
 **/
void draw_main_menu(void) {
	clear_game_field();
	draw_generic_menu(&main_menu);
}

/*
 * this function will draw generic menu
 **/
void draw_generic_menu(Menu *menu) {
	// Draw menu header
	BSP_LCD_SetFont(&Font8);
	BSP_LCD_SetTextColor(MAIN_COLOR);
	BSP_LCD_SetBackColor(SECONDARY_COLOR);
	BSP_LCD_DisplayStringAt(200, 50, menu->menu_header_text, LEFT_MODE);

	// display first option
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetTextColor(MAIN_COLOR);
	BSP_LCD_SetBackColor(SECONDARY_COLOR);
	BSP_LCD_DisplayStringAt(400, 150, menu->first_option_text, LEFT_MODE);

	// display second option
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetTextColor(MAIN_COLOR);
	BSP_LCD_SetBackColor(SECONDARY_COLOR);
	BSP_LCD_DisplayStringAt(200, 250, menu->second_option_text, LEFT_MODE);

	// display third option
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetTextColor(MAIN_COLOR);
	BSP_LCD_SetBackColor(SECONDARY_COLOR);
	BSP_LCD_DisplayStringAt(400, 350, menu->third_option_text, LEFT_MODE);
}

/*
 * this function draws winning player to a screen,
 * if parameter player is set to:
 *               - 0 -> left player will be drawn
 *               - 1 -> right player will be drawn
 **/
void draw_winner(uint8_t player) {
	clear_game_field();
	uint8_t text[5] = "WIN!";
	BSP_LCD_SetFont(&Font8);
	BSP_LCD_SetTextColor(MAIN_COLOR);
	BSP_LCD_SetBackColor(SECONDARY_COLOR);

	if (player == 0)
		BSP_LCD_DisplayStringAt(100, 240, text, LEFT_MODE);
	else
		BSP_LCD_DisplayStringAt(600, 240, text, LEFT_MODE);
}

/*
 * this function will display player scores in top border
 **/
void draw_player_score(Game *game) {
	uint8_t score[4] = "1:1";
	score[0] = game->player_one_score + '0';
	score[2] = game->player_two_score + '0';
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetTextColor(MAIN_COLOR);
	BSP_LCD_SetBackColor(SECONDARY_COLOR);
	BSP_LCD_DisplayStringAt(375, 10, score, LEFT_MODE);
    send_current_score_to_PC(score, (uint8_t) sizeof(score));
}

/*
* this function recieves current score and passes it to task_driven_mq, which then sends data via UART to PC
**/
void send_current_score_to_PC(uint8_t *score, uint8_t score_size) {
    // build a message
	Mssg mssg; 
    uint8_t temp_string[8] = "SCORE: ";
	memcpy(mssg.contents, temp_string, sizeof(temp_string));
    memcpy(&mssg.contents[7], score, score_size);
    mssg.size = sizeof(temp_string) + score_size;
    // send message to other task
	osMessageQueuePut(task_driven_mq, &mssg, 0U, 0U);
}

/*
 * this function draws a circle, with provided angle span [alpha, beta]
 * */
void draw_a_partial_circle(uint16_t coordinate_x, uint16_t coordinate_y,
		uint16_t radius, uint16_t start, uint16_t end, uint32_t color) {
	uint32_t calculated_x;
	uint32_t calculated_y;

	for (float angle = start; angle < end; angle += 0.5) {
		calculated_x = (uint32_t) radius * cos(angle) + coordinate_x;
		calculated_y = (uint32_t) radius * sin(angle) + coordinate_y;
		if (calculated_x < BSP_LCD_GetXSize() && calculated_x > 0
				&& calculated_y < BSP_LCD_GetYSize() && calculated_y > 0) {
			BSP_LCD_DrawPixel(calculated_x, calculated_y + 1, color);
			BSP_LCD_DrawPixel(calculated_x, calculated_y - 1, color);
			BSP_LCD_DrawPixel(calculated_x + 1, calculated_y, color);
			BSP_LCD_DrawPixel(calculated_x - 1, calculated_y, color);
			BSP_LCD_DrawPixel(calculated_x + 1, calculated_y - 1, color);
			BSP_LCD_DrawPixel(calculated_x - 1, calculated_y + 1, color);
		}
	}
}

/*
 * this function accepts and draws new hockey pad to a screen, returns a pointer to element collected from buffer to be cleared from display
 * */
void draw_air_hockey_pad(BufferedElement *hockey_pad) {
	// if element is NULL, then do nothing
	if (hockey_pad == NULL)
		return;

	// draw that element
	BSP_LCD_SetTextColor(MAIN_COLOR);
	// outer circle
	for (uint8_t i = 0; i < 5; i++)
		BSP_LCD_DrawCircle(hockey_pad->coordinate_x, hockey_pad->coordinate_y,
		AIR_HOCKEY_PAD_RADIUS - i);
	// inner circle
	for (uint8_t i = 1; i <= 5; i++)
		BSP_LCD_DrawCircle(hockey_pad->coordinate_x, hockey_pad->coordinate_y,
				i);
}

/*
 * this function clears hockey pad from screen, if no tap is detected
 * */
void clear_air_hockey_pad_from_buffer(Buffer *hockey_pad_buffer) {
	if (hockey_pad_buffer->num_of_elements > 0)
		clear_air_hockey_pad(
				collect_buffered_element_from_buffer(hockey_pad_buffer));
}

/*
 * this function clears hockey pad from screen and frees pointer
 * */
void clear_air_hockey_pad(BufferedElement *position) {
	// outer circle
	BSP_LCD_SetTextColor(SECONDARY_COLOR);
	for (uint8_t i = 0; i < 5; i++)
		BSP_LCD_DrawCircle(position->coordinate_x, position->coordinate_y,
		AIR_HOCKEY_PAD_RADIUS - i);
	// inner circle
	for (uint8_t i = 1; i <= 5; i++)
		BSP_LCD_DrawCircle(position->coordinate_x, position->coordinate_y, i);

	// free that element
	vPortFree(position);
}

/*
 * this function accepts and draws new hockey ball to a screen
 * */
void draw_hockey_ball(BufferedElement *hockey_ball) {
	// if buffer is empty, then do nothing
	if (hockey_ball == NULL)
		return;

	BSP_LCD_SetTextColor(MAIN_COLOR);
	for (uint8_t i = 1; i <= AIR_HOCKEY_BALL_RADIUS; i++)
		BSP_LCD_DrawCircle(hockey_ball->coordinate_x, hockey_ball->coordinate_y,
				i);
}

/*
 * this function collects hockey ball from buffer and calls clear_hockey_ball function, iff buffer is full
 * */
void clear_hockey_ball_from_buffer(Buffer *hockey_ball_buffer) {
	if (hockey_ball_buffer->num_of_elements < AIR_HOCKEY_PAD_BUFFER_SIZE)
		return;
	BufferedElement *hockey_ball_to_be_cleared =
			collect_buffered_element_from_buffer(hockey_ball_buffer);
	if (hockey_ball_to_be_cleared != NULL)
		clear_hockey_ball(hockey_ball_to_be_cleared);
}

/*
 * this function clears hockey ball from screen and frees pointer
 * */
void clear_hockey_ball(BufferedElement *position) {
	BSP_LCD_SetTextColor(SECONDARY_COLOR);

	for (uint8_t i = 1; i <= AIR_HOCKEY_BALL_RADIUS; i++)
		BSP_LCD_DrawCircle(position->coordinate_x, position->coordinate_y, i);

	vPortFree(position);
}

/*
 * this function returns 1 if menu button is pressed
 **/
uint8_t is_menu_button_pressed(TS_StateTypeDef touchscreen_state,
		int touch_index, int startx, int starty, uint8_t strlen, int strwid) {
	if (touchscreen_state.touchX[touch_index] > startx
			&& touchscreen_state.touchX[touch_index] < startx + strlen
			&& touchscreen_state.touchY[touch_index] > starty
			&& touchscreen_state.touchY[touch_index] < starty + strwid)
		return 1;
	return 0;
}

/*
 * this function returns 1 if detected touch is in game field, 0 if it is not
 * */
uint8_t is_touch_in_game_field(TS_StateTypeDef touchscreen_state,
		int touch_index) {
	return (TS_State.touchY[touch_index] > BORDER_SIZE + AIR_HOCKEY_PAD_RADIUS
			&& TS_State.touchY[touch_index]
					< MAX_ALLOWED_Y - AIR_HOCKEY_PAD_RADIUS
			&& TS_State.touchX[touch_index]
					> BORDER_SIZE + AIR_HOCKEY_PAD_RADIUS
			&& TS_State.touchX[touch_index]
					< MAX_ALLOWED_X - AIR_HOCKEY_PAD_RADIUS) ? 1 : 0;
}

/*
 * this function returns 1 if detected touch is in bottom menu
 **/
uint8_t is_touch_in_bottom_menubar(TS_StateTypeDef touchscreen_state,
		int touch_index) {
	return (TS_State.touchX[touch_index] >= 300
			&& TS_State.touchX[touch_index] <= 500
			&& TS_State.touchY[touch_index] > BSP_LCD_GetYSize() - BORDER_SIZE
			&& TS_State.touchY[touch_index] < BSP_LCD_GetYSize()) ? 1 : 0;
}

/*
 * this function returns 1 if detected touch is in upper menu
 **/
uint8_t is_touch_in_upper_menubar(TS_StateTypeDef touchscreen_state,
		int touch_index) {
	return (TS_State.touchX[touch_index] >= 300
			&& TS_State.touchX[touch_index] <= 500
			&& TS_State.touchY[touch_index] > 0
			&& TS_State.touchY[touch_index] < BORDER_SIZE) ? 1 : 0;
}

/*
 * this function returns:
 *                       - 0 -> if ball is not in any goal
 *                       - 1 -> if ball is in left goal
 *                       - 2 -> if ball is in right goal
 **/
uint8_t is_ball_in_a_goal(Game *game) {
	uint8_t return_value = 0;
	// left goal ball hit
	if (game->hockey_ball_coordinate_x == 100
			&& game->hockey_ball_coordinate_y > 180
			&& game->hockey_ball_coordinate_y < 300)
		return_value = 1;
	else if (game->hockey_ball_coordinate_x == 700
			&& game->hockey_ball_coordinate_y > 180
			&& game->hockey_ball_coordinate_y < 300)
		return_value = 2;
	return return_value;
}

/*
 * this function displays winner, stops game and returns game back to main menu
 **/
void display_winner_and_reset_game_to_main_menu(Game *game) {
	// firstly stop the game
	game->game_status = NOT_RUNNING;
	// then display a winner and delay..
	draw_winner(game->player_one_score == 7 ? 1 : 0);
	// send one big dick through message queue
    osDelay(300);
    send_winner_to_PC(game->player_one_score == 7 ? 1 : 0); 
    osDelay(1700);
	// finally display main menu
	draw_main_menu();
}

/*
* this function recieves winner and passes it to task_driven_mq, which then sends data via UART to PC
*     if parameter 0 is passed, then left player won, else if 
*     parameter 1 is passed to this function then right player won
**/
void send_winner_to_PC(uint8_t player) {
    uint8_t temp_left[17] = "LEFT PLAYER WON!";
    uint8_t temp_right[18] = "RIGHT PLAYER WON!";

    // build a message 
    Mssg mssg; 
    mssg.size = player == 0 ? 17 : 18; 
    memcpy(&mssg.contents, player == 0 ? temp_left : temp_right, mssg.size);
    // send message to other task
    osMessageQueuePut(task_driven_mq, &mssg, 0U, 0U);
}

/*
 * this function handles pause menu events
 **/
void process_pause_menu_click_event(TS_StateTypeDef touchscreen_state,
		Menu *menu, Game *game) {
	// check if option one is clicked  x400 y150
	if (is_menu_button_pressed(touchscreen_state, 0, 400, 140,
			strlen((char*) menu->first_option_text) * 14, 30)) {
		clear_game_field();
		draw_game_field(1);
		game->game_status = RUNNING;
        
        uint8_t message_for_pc[13] = "GAME RESUMED";
        send_message_to_PC(message_for_pc, 13);
	}
	// check if option two is clicked  x200 y250
	else if (is_menu_button_pressed(touchscreen_state, 0, 200, 240,
			strlen((char*) menu->first_option_text) * 14, 30)) {
		clear_game_field();
		draw_game_field(1);
		reset_game();
        uint8_t message_for_pc[13] = "GAME RESETED";
        send_message_to_PC(message_for_pc, 13);
	}
	// check if option one is clicked  x400 y350
	else if (is_menu_button_pressed(touchscreen_state, 0, 390, 340,
			strlen((char*) menu->first_option_text) * 14, 30)) {
		game->game_status = NOT_RUNNING;
		draw_main_menu();
        uint8_t message_for_pc[13] = "GAME STOPPED";
        send_message_to_PC(message_for_pc, 13);
	}
}

/*
 * this function handles pause menu events
 **/
void process_main_menu_click_event(TS_StateTypeDef touchscreen_state,
		Menu *menu, Game *game) {
	// check if option one is clicked  x400 y150
	if (is_menu_button_pressed(touchscreen_state, 0, 400, 140,
			strlen((char*) menu->first_option_text) * 14, 30)) {
		return;
	}
	// check if option two is clicked  x200 y250
	else if (is_menu_button_pressed(touchscreen_state, 0, 200, 240,
			strlen((char*) menu->first_option_text) * 14, 30)) {
		draw_game_field(1);
		start_game();
		draw_player_score(game);
	}
	// check if option one is clicked  x400 y350
	else if (is_menu_button_pressed(touchscreen_state, 0, 390, 340,
			strlen((char*) menu->first_option_text) * 14, 30)) {
        switch_board_colors(game);
	}
}

/*
* this function swtiches colors of board
**/
void switch_board_colors(Game *game) {
    uint32_t rand_col;
    uint8_t r = get_random_number(&rand_col);
    uint8_t g = get_random_number(&rand_col);
    uint8_t b = get_random_number(&rand_col);
    Color c = {.r = r, .g = g, .b = b, .a = A_u};
    MAIN_COLOR = convert_color_to_rgba_int_value(&c);
    
    draw_main_menu();
    uint8_t switch_game_color_message[16];
    sprintf((char *)switch_game_color_message, "COLOR: 0x%x%x%x", r, g, b);
    send_message_to_PC(switch_game_color_message, 16);
}

/*
 * this function processes single tap on game field
 * */
void process_single_in_game_tap(TS_StateTypeDef touchscreen_state) {
	BufferedElement *element_to_be_cleared = NULL;
	BufferedElement *element_to_be_drawn = NULL;
	if (TS_State.touchX[0] < 400) { // left player
		// firstly clear collect element from buffer
		if (hockey_pad_buffer_left_player.num_of_elements
				== AIR_HOCKEY_PAD_BUFFER_SIZE)
			element_to_be_cleared = collect_buffered_element_from_buffer(
					&hockey_pad_buffer_left_player);

		// add a new element into buffer
		add_element_into_buffer(TS_State.touchX[0], TS_State.touchY[0],
				&hockey_pad_buffer_left_player);
		// set that element to be drawn
		element_to_be_drawn = hockey_pad_buffer_left_player.rear;
	} else { // right player
		// firstly clear collect element from buffer
		if (hockey_pad_buffer_right_player.num_of_elements
				== AIR_HOCKEY_PAD_BUFFER_SIZE)
			element_to_be_cleared = collect_buffered_element_from_buffer(
					&hockey_pad_buffer_right_player);

		// add a new element into buffer
		add_element_into_buffer(TS_State.touchX[0], TS_State.touchY[0],
				&hockey_pad_buffer_right_player);
		// set that element to be drawn
		element_to_be_drawn = hockey_pad_buffer_right_player.rear;
	}

	// firstly clear pad from screen
	if (element_to_be_cleared != NULL)
		clear_air_hockey_pad(element_to_be_cleared);

	// then check if ball is also ready to be clear from buffer, iff it is, clear it
	clear_hockey_ball_from_buffer(&hockey_ball_buffer);

	move_ball();

	// ...then draw a new pad and ball
	draw_air_hockey_pad(element_to_be_drawn);
	draw_hockey_ball(hockey_ball_buffer.front);
	// redraw game field
	draw_game_field(0);
}

/*
 * this function processes double tap on game field
 * */
void process_double_in_game_tap(TS_StateTypeDef touchscreen_state) {
	BufferedElement *element_to_be_cleared_left = NULL;
	BufferedElement *element_to_be_cleared_right = NULL;
	BufferedElement *element_to_be_drawn_left = NULL;
	BufferedElement *element_to_be_drawn_right = NULL;

	// firstly check if elements are to be cleared
	// firstly clear collect element from buffer
	if (hockey_pad_buffer_right_player.num_of_elements
			== AIR_HOCKEY_PAD_BUFFER_SIZE)
		element_to_be_cleared_right = collect_buffered_element_from_buffer(
				&hockey_pad_buffer_right_player);

	if (hockey_pad_buffer_left_player.num_of_elements
			== AIR_HOCKEY_PAD_BUFFER_SIZE)
		element_to_be_cleared_left = collect_buffered_element_from_buffer(
				&hockey_pad_buffer_left_player);

	if (TS_State.touchX[0] <= 400 && TS_State.touchX[1] >= 400) { // left player is player on index 0

		// add these two pads into buffer
		add_element_into_buffer(TS_State.touchX[0], TS_State.touchY[0],
				&hockey_pad_buffer_left_player);
		add_element_into_buffer(TS_State.touchX[1], TS_State.touchY[1],
				&hockey_pad_buffer_right_player);
		// set that elements to be drawn
		element_to_be_drawn_left = hockey_pad_buffer_left_player.rear;
		element_to_be_drawn_right = hockey_pad_buffer_right_player.rear;
	} else if (TS_State.touchX[1] <= 400 && TS_State.touchX[0] >= 400) { // left player is player on index 1
		// add these two pads into buffer
		add_element_into_buffer(TS_State.touchX[1], TS_State.touchY[1],
				&hockey_pad_buffer_left_player);
		add_element_into_buffer(TS_State.touchX[0], TS_State.touchY[0],
				&hockey_pad_buffer_right_player);
		// set that elements to be drawn
		element_to_be_drawn_left = hockey_pad_buffer_left_player.rear;
		element_to_be_drawn_right = hockey_pad_buffer_right_player.rear;
	}

	// firstly clear pads from screen
	if (element_to_be_cleared_left != NULL)
		clear_air_hockey_pad(element_to_be_cleared_left);
	if (element_to_be_cleared_right != NULL)
		clear_air_hockey_pad(element_to_be_cleared_right);
	// then check if ball is also ready to be clear from buffer, iff it is, clear it
	clear_hockey_ball_from_buffer(&hockey_ball_buffer);

	move_ball();

	// ...then draw new ones
	draw_air_hockey_pad(element_to_be_drawn_left);
	draw_air_hockey_pad(element_to_be_drawn_right);
	draw_hockey_ball(hockey_ball_buffer.front);

	// redraw game field
	draw_game_field(0);
}
/*
 * this function processes user input
 * */
void process_display_event(TS_StateTypeDef touchscreen_state, Game *game) {

	// this switch is necessary to check if ball is hit or not
	switch (game->game_status) {
	case RUNNING:
		;
		uint8_t status = is_ball_in_a_goal(game);
		if (status > 0) {
			// for now only increase score
			if (status == 1)
				game->player_one_score++;
			else if (status == 2)
				game->player_two_score++;
			draw_player_score(game);
			if (game->player_one_score == 7 || game->player_two_score == 7)
				display_winner_and_reset_game_to_main_menu(game);
			game->hockey_ball_coordinate_x = 400;
			game->hockey_ball_coordinate_y = 240;
			game->ball_speed_x = -game->ball_speed_x;
		}
		break;
	case PAUSED:
		break;
	case NOT_RUNNING:
		break;
	}

	// single touch detected
	if (TS_State.touchDetected == 1) {

		switch (game->game_status) {
		case RUNNING:
			if (is_touch_in_game_field(touchscreen_state, 0))
				process_single_in_game_tap(touchscreen_state);
			else if (is_touch_in_bottom_menubar(touchscreen_state, 0))
				handle_bottom_corner_event(game, &pause_menu,
						&hockey_pad_buffer_left_player,
						&hockey_pad_buffer_right_player, &hockey_ball_buffer);
			break;
		case PAUSED:
			if (is_touch_in_game_field(touchscreen_state, 0))
				process_pause_menu_click_event(touchscreen_state, &pause_menu,
						game);
			break;
		case NOT_RUNNING:
			if (is_touch_in_game_field(touchscreen_state, 0))
				process_main_menu_click_event(touchscreen_state, &pause_menu,
						game);
			break;
		}
	}
	// double touch detected
	else if (TS_State.touchDetected == 2 && game->game_mode == MULTIPLAYER) {
		switch (game->game_status) {
		case RUNNING:
			if (is_touch_in_game_field(touchscreen_state, 0)
					&& is_touch_in_game_field(touchscreen_state, 1))
				process_double_in_game_tap(touchscreen_state);
			break;
		case PAUSED:
			break;
		case NOT_RUNNING:
			break;
		}
		// no tap is detected
	} else {
		switch (game->game_status) {
		case RUNNING:
			clear_air_hockey_pad_from_buffer(&hockey_pad_buffer_left_player);
			clear_air_hockey_pad_from_buffer(&hockey_pad_buffer_right_player);
			// then check if ball is also ready to be clear from buffer, iff it is, clear it
			clear_hockey_ball_from_buffer(&hockey_ball_buffer);

			move_ball();
			draw_hockey_ball(hockey_ball_buffer.front);

			// redraw game field
			draw_game_field(0);
			break;
		case PAUSED:
			break;
		case NOT_RUNNING:
			break;
		}
	}
}

/*
 * MOVE BALL FUNCTIONS
 */

/*
 * function that initializes game with default values
 * */
void start_game(void) {
	game.player_one_score = 0;
	game.player_two_score = 0;
	game.ball_speed_x = 20;
	game.ball_speed_y = 20;
	game.hockey_ball_coordinate_x = 400;
	game.hockey_ball_coordinate_y = 240;
	game.game_status = RUNNING;
	game.game_mode = MULTIPLAYER;
    game.current_color = 0;
    uint8_t pause_message[13] = "GAME STARTED";
    send_message_to_PC(pause_message, 13);
}

/*
* this function resets game
**/
void reset_game(void) {
	game.player_one_score = 0;
	game.player_two_score = 0;
	game.ball_speed_x = 20;
	game.ball_speed_y = 20;
	game.hockey_ball_coordinate_x = 400;
	game.hockey_ball_coordinate_y = 240;
	game.game_status = RUNNING;
	game.game_mode = MULTIPLAYER;
}

/*
* function that pauses game
**/
void pause_game(void) {
	game.game_status = PAUSED;
    uint8_t pause_message[12] = "GAME PAUSED";
    send_message_to_PC(pause_message, 12);
}

/*
 * function that moves ball one position forwards
 * */
void move_ball(void) {
	// if game is not running do nothing
	if (game.game_status == PAUSED || game.game_status == NOT_RUNNING)
		return;
	// firstly check for collisions
	handle_collisions(&game);
	// increment/decrement coordinates
	game.hockey_ball_coordinate_x += game.ball_speed_x;
	game.hockey_ball_coordinate_y += game.ball_speed_y;
	// add that new frame to buffer
	add_element_into_buffer(game.hockey_ball_coordinate_x,
			game.hockey_ball_coordinate_y, &hockey_ball_buffer);
}

/*
 * function that handles collisions
 * */
void handle_collisions(Game *game) {

	BufferedElement *left_hockey_pad =
			(BufferedElement*) hockey_pad_buffer_left_player.rear;
	BufferedElement *right_hockey_pad =
			(BufferedElement*) hockey_pad_buffer_right_player.rear;
    uint8_t border_hit_mssg[12] = "border hit!";
    uint8_t collision_detected[13] = "ball bounce!";
	// if we hit game border on x coordinate or , invert x
	if (game->hockey_ball_coordinate_x + game->ball_speed_x - BORDER_SIZE
			<= BORDER_SIZE
			|| game->hockey_ball_coordinate_x + game->ball_speed_x
					>= MAX_ALLOWED_X - BORDER_SIZE) {
		    game->ball_speed_x = -game->ball_speed_x;
            send_message_to_PC(border_hit_mssg, 12);     
    }

	// if we hit game border on y coordinate or , invert y
	if (game->hockey_ball_coordinate_y + game->ball_speed_y - BORDER_SIZE
			<= BORDER_SIZE
			|| game->hockey_ball_coordinate_y + game->ball_speed_y
					>= MAX_ALLOWED_Y - BORDER_SIZE) {
    		game->ball_speed_y = -game->ball_speed_y;
            send_message_to_PC(border_hit_mssg, 12);     
        }

	// check for collisions with pads
	// https://stackoverflow.com/a/1736741
	if (left_hockey_pad != NULL) {
		if (pow(left_hockey_pad->coordinate_x - game->hockey_ball_coordinate_x,
				2)
				+ pow(
						left_hockey_pad->coordinate_y
								- game->hockey_ball_coordinate_y, 2)
				<= pow(AIR_HOCKEY_PAD_RADIUS + AIR_HOCKEY_BALL_RADIUS, 2)) {
			game->ball_speed_x = -game->ball_speed_x;
			game->ball_speed_y = -game->ball_speed_y;
            send_message_to_PC(collision_detected, 13);
			return;
		}
	}

	// check for collisions with pads
	if (right_hockey_pad != NULL) {
		if (pow(right_hockey_pad->coordinate_x - game->hockey_ball_coordinate_x,
				2)
				+ pow(
						right_hockey_pad->coordinate_y
								- game->hockey_ball_coordinate_y, 2)
				<= pow(AIR_HOCKEY_PAD_RADIUS + AIR_HOCKEY_BALL_RADIUS, 2)) {
			game->ball_speed_x = -game->ball_speed_x;
			game->ball_speed_y = -game->ball_speed_y;
            send_message_to_PC(collision_detected, 13);
            return;
		}
	}
}

/*
 * MISC FUNCTIONS
 */

/*
* this function sends generic message to PC
**/
void send_message_to_PC(uint8_t *message, uint8_t message_length) {
    // build a message 
    Mssg mssg;
    mssg.size = message_length;
    memcpy(&mssg.contents, message, mssg.size);
    // send message to other task
    osMessageQueuePut(task_driven_mq, &mssg, 0U, 0U);
}


/*
* this function generates random number using RNG, it accepts pointer to integer,
* and when function finishes, random number in uint8_t format is returned
**/
uint8_t get_random_number(uint32_t *random_number) {
    __HAL_RNG_ENABLE(&rng);
    HAL_RNG_GenerateRandomNumber(&rng, random_number);
    *random_number = *random_number % 255; // just mod it:)
    __HAL_RNG_DISABLE(&rng);
    return (uint8_t) *random_number;
}


/*
* this function that converts provided Color structure into rgba int format
**/
uint32_t convert_color_to_rgba_int_value(Color *color) {
	uint32_t rgba = color->a << 24 | color->r << 16 | color->g << 8 | color->b;
	return rgba;
}
