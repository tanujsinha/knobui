#ifndef SDCARD_BSP_H
#define SDCARD_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

void _sdcard_init(void);

esp_err_t s_example_read_file(const char *path,uint8_t *pxbuf,uint32_t *outLen);
esp_err_t s_example_write_file(const char *path, char *data);

#ifdef __cplusplus
}
#endif

#endif