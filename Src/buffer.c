#include "buffer.h"

/*
 * function that allocates new node to be placed into buffer
 * */
BufferedElement* allocate_buffered_element(uint16_t coordinate_x,
		uint16_t coordinate_y) {
	BufferedElement *buffered_element = (BufferedElement*) pvPortMalloc(
			sizeof(BufferedElement));
	buffered_element->coordinate_x = coordinate_x;
	buffered_element->coordinate_y = coordinate_y;
	return buffered_element;
}

/*
 * function that stores given coordinates into a fifo buffer
 * */
void add_element_into_buffer(uint16_t coordinate_x, uint16_t coordinate_y,
		Buffer *buffer) {
	BufferedElement *element = allocate_buffered_element(coordinate_x,
			coordinate_y);

	// a buffer is empty
	if (buffer->rear == NULL)
		buffer->rear = buffer->front = element;

	// add element on the end of buffer
	buffer->rear->next = (struct BufferedElement*) element;
	buffer->rear = element;

	// increment counter of current elements in a buffer
	buffer->num_of_elements += 1;
}

/*
 * function that collects last element from buffer, warning pointer must be freed after!
 * */
BufferedElement* collect_buffered_element_from_buffer(Buffer *buffer) {
	if (buffer->front == NULL)
		return NULL;

	BufferedElement *collected_element = (BufferedElement*) buffer->front;

	// move front one buffered element ahead
	buffer->front = (BufferedElement*) buffer->front->next;
	buffer->num_of_elements -= 1;

	// if we collected last element, rewire pointers to point to NULL
	if (buffer->num_of_elements == 0)
		buffer->front = buffer->rear = NULL;

	return collected_element;
}

/*
 * function that collects last element from buffer and frees it
 * */
void dequeue_buffer(Buffer *buffer) {
	if (buffer->num_of_elements > 0)
		vPortFree(collect_buffered_element_from_buffer(buffer));
}
