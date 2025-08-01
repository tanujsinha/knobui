#include "cst816.h"
#include "esp_err.h"
#include "lcd_config.h"

#define TEST_I2C_PORT I2C_NUM_0

uint8_t I2C_writr_buff(uint8_t addr,uint8_t reg,uint8_t *buf,uint8_t len)
{
  uint8_t ret;
  uint8_t *pbuf = (uint8_t*)malloc(len+1);
  pbuf[0] = reg;
  for(uint8_t i = 0; i<len; i++)
  {
    pbuf[i+1] = buf[i];
  }
  ret = i2c_master_write_to_device(TEST_I2C_PORT,addr,pbuf,len+1,1000);
  free(pbuf);
  pbuf = NULL;
  return ret;
}
uint8_t I2C_read_buff(uint8_t addr,uint8_t reg,uint8_t *buf,uint8_t len)
{
  uint8_t ret;
  ret = i2c_master_write_read_device(TEST_I2C_PORT,addr,&reg,1,buf,len,1000);
  return ret;
}
uint8_t I2C_master_write_read_device(uint8_t addr,uint8_t *writeBuf,uint8_t writeLen,uint8_t *readBuf,uint8_t readLen)
{
  uint8_t ret;
  ret = i2c_master_write_read_device(TEST_I2C_PORT,addr,writeBuf,writeLen,readBuf,readLen,1000);
  return ret;
}
void Touch_Init(void)
{
  i2c_config_t conf = 
  {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = EXAMPLE_PIN_NUM_TOUCH_SDA,         // Configure the GPIO of the SDA
    .scl_io_num = EXAMPLE_PIN_NUM_TOUCH_SCL,         // Configure GPIO for SCL
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {.clk_speed = 300 * 1000,},  // Select a frequency for the project
    .clk_flags = 0,          // Optionally, use the I2C SCLK SRC FLAG * flag to select the I2C source clock
  };
  ESP_ERROR_CHECK(i2c_param_config(TEST_I2C_PORT, &conf));
  ESP_ERROR_CHECK(i2c_driver_install(TEST_I2C_PORT, conf.mode,0,0,0));

  uint8_t data = 0x00;
  I2C_writr_buff(EXAMPLE_TOUCH_ADDR,0x00,&data,1); //Switch to normal mode
}
uint8_t getTouch(uint16_t *x,uint16_t *y)
{
  uint8_t GetNum = 0;
  uint8_t data[7] = {0};
  I2C_read_buff(EXAMPLE_TOUCH_ADDR,0x00,data,7);
  GetNum = data[2];
  if(GetNum)
  {
    *x = ((uint16_t)(data[3] & 0x0f)<<8) + (uint16_t)data[4];
    *y = ((uint16_t)(data[5] & 0x0f)<<8) + (uint16_t)data[6];
    return 1;
  }
  return 0;
}


