/*
 * Encoder_Lib.c
 *
 * Created: 03.03.2022 13:58:48
 *  Author: PavlovVG
 */

#include "Encoder_Lib.h"

#define ENC_INITIAL_STATE       0x3
#define ENC_ERROR_STATE         0x0
#define ENC_RIGHT_STATE         0x1
#define ENC_LEFT_STATE          0x2
#define ENC_HOLD_RIGHT_STATE    0x4
#define ENC_HOLD_LEFT_STATE     0x8
#define ENC_BTN_CLICK_STATE     0x10
#define ENC_BTN_HOLD_STATE      0x20

#define ENC_HOLD_OFFSET 0x2

#define ENC_BTN_DEBOUNCE_TIME_MS 50

void init_encoder(Encoder_t *encoder,
				  read_encoder_callback_t read_encoder_function,
				  read_encoder_button_callback_t read_encoder_button_function,
				  void *context,
				  uint8_t enc_timer_period_ms,
				  uint16_t enc_btn_hold_timeout_ms)
{
	encoder->read_encoder_callback = read_encoder_function;
	encoder->read_encoder_button_callback = read_encoder_button_function;
	encoder->context = context;
	encoder->enc_timer_period_ms = enc_timer_period_ms;
	encoder->enc_btn_hold_timeout_ms = enc_btn_hold_timeout_ms;
	encoder->encoder_processing_state = WAITING_FOR_ENCODER_INITIAL_STATE;
	encoder->encoder_button_processing_state = WAITING_FOR_ENCODER_BUTTON_PRESS_STATE;
	encoder->enc_state = 0;
	encoder->enc_btn_debounce_ticks = ENC_BTN_DEBOUNCE_TIME_MS / encoder->enc_timer_period_ms;
	encoder->enc_btn_debounce_counter = encoder->enc_btn_debounce_ticks;
	encoder->enc_btn_hold_ticks = encoder->enc_btn_hold_timeout_ms / encoder->enc_timer_period_ms;
	encoder->enc_btn_hold_counter = encoder->enc_btn_hold_ticks;
}

void encoder_processing(Encoder_t *encoder)
{
	uint8_t encoder_state = encoder->read_encoder_callback(encoder->context);
	if (encoder->encoder_processing_state == WAITING_FOR_ENCODER_INITIAL_STATE && encoder_state == ENC_INITIAL_STATE)
	{
		encoder->encoder_processing_state = WAITING_FOR_ENCODER_ROTATION_STATE;
		return;
	}
	if (encoder->encoder_processing_state == WAITING_FOR_ENCODER_INITIAL_STATE || encoder_state == ENC_INITIAL_STATE)
	{
		return;
	}
	if (encoder_state == ENC_ERROR_STATE)
	{
		encoder->encoder_processing_state = WAITING_FOR_ENCODER_INITIAL_STATE;
		return;
	}
	if (encoder->read_encoder_button_callback(encoder->context) == ENCODER_BUTTON_PRESSED)
	{
		encoder->enc_state |= (encoder_state << ENC_HOLD_OFFSET);
		encoder->encoder_button_processing_state = ENCODER_HOLD_TURN;
		encoder->encoder_processing_state = WAITING_FOR_ENCODER_INITIAL_STATE;
		return;
	}
	encoder->enc_state |= encoder_state;
	encoder->encoder_processing_state = WAITING_FOR_ENCODER_INITIAL_STATE;
}

void encoder_button_debounce(Encoder_t *encoder, uint8_t encoder_button_state)
{
	if (encoder->encoder_button_processing_state != WAITING_FOR_ENCODER_BUTTON_DEBOUNCE_STATE)
	{
		return;
	}
	if (--encoder->enc_btn_debounce_counter != 0)
	{
		return;
	}
	if (encoder_button_state == ENCODER_BUTTON_RELEASED)
	{
		encoder->enc_btn_debounce_counter = encoder->enc_btn_debounce_ticks;
		encoder->encoder_button_processing_state = WAITING_FOR_ENCODER_BUTTON_PRESS_STATE;
		return;
	}
	encoder->enc_btn_debounce_counter = encoder->enc_btn_debounce_ticks;
	encoder->encoder_button_processing_state = WAITING_FOR_ENCODER_BUTTON_HOLD_STATE;
}

void encoder_button_hold(Encoder_t *encoder, uint8_t encoder_button_state)
{
	if (encoder->encoder_button_processing_state != WAITING_FOR_ENCODER_BUTTON_HOLD_STATE)
	{
		return;
	}
	if (encoder_button_state == ENCODER_BUTTON_RELEASED)
	{
		encoder->enc_state |= ENC_BTN_CLICK_STATE;
		encoder->enc_btn_hold_counter = encoder->enc_btn_hold_ticks;
		encoder->encoder_button_processing_state = WAITING_FOR_ENCODER_BUTTON_PRESS_STATE;
	}
	if (--encoder->enc_btn_hold_counter == 0)
	{
		encoder->enc_state |= ENC_BTN_HOLD_STATE;
		encoder->enc_btn_hold_counter = encoder->enc_btn_hold_ticks;
		encoder->encoder_button_processing_state = ENCODER_BUTTON_BUTTON_HOLD_STATE;
	}
}

void encoder_button_processing(Encoder_t *encoder)
{
	uint8_t encoder_button_state = encoder->read_encoder_button_callback(encoder->context);
	if (encoder->encoder_button_processing_state == WAITING_FOR_ENCODER_BUTTON_PRESS_STATE && encoder_button_state == ENCODER_BUTTON_PRESSED)
	{
		encoder->encoder_button_processing_state = WAITING_FOR_ENCODER_BUTTON_DEBOUNCE_STATE;
	}
	if (encoder->encoder_button_processing_state == WAITING_FOR_ENCODER_BUTTON_PRESS_STATE)
	{
		return;
	}
	encoder_button_debounce(encoder, encoder_button_state);
	encoder_button_hold(encoder, encoder_button_state);
	if (encoder_button_state == ENCODER_BUTTON_RELEASED && encoder->encoder_button_processing_state == ENCODER_HOLD_TURN)
	{
		encoder->encoder_button_processing_state = WAITING_FOR_ENCODER_BUTTON_PRESS_STATE;
		return;
	}
	if (encoder_button_state == ENCODER_BUTTON_RELEASED && encoder->encoder_button_processing_state == ENCODER_BUTTON_BUTTON_HOLD_STATE)
	{
		encoder->encoder_button_processing_state = WAITING_FOR_ENCODER_BUTTON_PRESS_STATE;
		return;
	}
}

uint8_t encoder_is_right(Encoder_t *encoder)
{
	if (encoder->enc_state & ENC_RIGHT_STATE)
	{
		encoder->enc_state &= ~ENC_RIGHT_STATE;
		return 1;
	}
	return 0;
}

uint8_t encoder_is_left(Encoder_t *encoder)
{
	if (encoder->enc_state & ENC_LEFT_STATE)
	{
		encoder->enc_state &= ~ENC_LEFT_STATE;
		return 1;
	}
	return 0;
}

uint8_t encoder_is_hold_right(Encoder_t *encoder)
{
	if (encoder->enc_state & ENC_HOLD_RIGHT_STATE)
	{
		encoder->enc_state &= ~ENC_HOLD_RIGHT_STATE;
		return 1;
	}
	return 0;
}

uint8_t encoder_is_hold_left(Encoder_t *encoder)
{
	if (encoder->enc_state & ENC_HOLD_LEFT_STATE)
	{
		encoder->enc_state &= ~ENC_HOLD_LEFT_STATE;
		return 1;
	}
	return 0;
}

uint8_t encoder_button_is_click(Encoder_t *encoder)
{
	if (encoder->enc_state & ENC_BTN_CLICK_STATE)
	{
		encoder->enc_state &= ~ENC_BTN_CLICK_STATE;
		return 1;
	}
	return 0;
}

uint8_t encoder_button_is_hold(Encoder_t *encoder)
{
	if (encoder->enc_state & ENC_BTN_HOLD_STATE)
	{
		encoder->enc_state &= ~ENC_BTN_HOLD_STATE;
		return 1;
	}
	return 0;
}
