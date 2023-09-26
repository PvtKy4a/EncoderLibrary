/*
 * Encoder_Lib.h
 *
 * Created: 03.03.2022 13:58:57
 *  Author: PavlovVG
 */ 

#ifndef ENCODER_LIB_H_
#define ENCODER_LIB_H_

#include <stdint.h>

#define ENCODER_TURNED_LEFT		0x1
#define ENCODER_TURNED_RIGHT	0x2

#define ENCODER_BUTTON_PRESSED	1
#define ENCODER_BUTTON_RELEASED	0

typedef enum {
	WAITING_FOR_ENCODER_INITIAL_STATE,
	WAITING_FOR_ENCODER_ROTATION_STATE
} encoder_processing_state_t;

typedef enum {
	WAITING_FOR_ENCODER_BUTTON_PRESS_STATE,
	WAITING_FOR_ENCODER_BUTTON_DEBOUNCE_STATE,
	WAITING_FOR_ENCODER_BUTTON_HOLD_STATE,
	ENCODER_BUTTON_BUTTON_HOLD_STATE,
	ENCODER_HOLD_TURN
} encoder_button_processing_state_t;

typedef uint8_t (* read_encoder_callback_t)(void *);
typedef uint8_t (* read_encoder_button_callback_t)(void *);

typedef struct {
    void * context;
    read_encoder_callback_t read_encoder_callback;
    read_encoder_button_callback_t read_encoder_button_callback;
    uint8_t enc_timer_period_ms;
    uint16_t enc_btn_hold_timeout_ms;
	encoder_processing_state_t encoder_processing_state;
	encoder_button_processing_state_t encoder_button_processing_state;
	uint8_t enc_state;
	uint8_t enc_btn_debounce_ticks;
	uint8_t enc_btn_debounce_counter;
	uint16_t enc_btn_hold_ticks;
	uint16_t enc_btn_hold_counter;
} Encoder_t;

void init_encoder(Encoder_t * encoder,
        read_encoder_callback_t read_encoder_function,
		read_encoder_button_callback_t read_encoder_button_function,
        void * context,
		uint8_t enc_timer_period_ms,
		uint16_t enc_btn_hold_timeout_ms);

void encoder_processing(Encoder_t * encoder);
void encoder_button_processing(Encoder_t * encoder);

uint8_t encoder_is_right(Encoder_t * encoder);
uint8_t encoder_is_left(Encoder_t * encoder);
uint8_t encoder_is_hold_right(Encoder_t * encoder);
uint8_t encoder_is_hold_left(Encoder_t * encoder);
uint8_t encoder_button_is_click(Encoder_t * encoder);
uint8_t encoder_button_is_hold(Encoder_t * encoder);

#endif /* ENCODER_LIB_H_ */
