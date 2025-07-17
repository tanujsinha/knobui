#ifndef USER_ENCODER_H
#define USER_ENCODER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

extern EventGroupHandle_t knob_even_;

#ifdef __cplusplus
extern "C" {
#endif

void user_encoder_init(void);

#ifdef __cplusplus
}
#endif

#endif
