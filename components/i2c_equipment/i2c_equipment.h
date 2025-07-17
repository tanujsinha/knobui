#ifndef I2C_EQUIPMENT_H
#define I2C_EQUIPMENT_H


#ifdef __cplusplus
extern "C" {
#endif



void i2c_drv2605_setup(void);
void i2c_drv2605_loop_task(void *arg);
void i2c_drv2605_haptic_feedback(uint8_t effect);
void i2c_drv2605_haptic_feedback_strong(uint8_t intensity);

#ifdef __cplusplus
}
#endif

#endif 
