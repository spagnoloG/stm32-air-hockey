#include "stm32f769i_discovery.h"
#include "cmsis_os.h"

typedef struct BufferedElement_ {
	uint16_t coordinate_x, coordinate_y;
	struct BufferedElement *next;
} BufferedElement;

typedef struct Buffer_ {
	uint16_t num_of_elements;
	BufferedElement *front, *rear;
} Buffer;

void add_element_into_buffer(uint16_t coordinate_x, uint16_t coordinate_y,
		Buffer *buffer);

BufferedElement* collect_buffered_element_from_buffer(Buffer *buffer); // WARNING: pointer must be freed!

void dequeue_buffer(Buffer *buffer);

extern Buffer hockey_pad_buffer_left_player;
extern Buffer hockey_pad_buffer_right_player;
extern Buffer hockey_ball_buffer;
