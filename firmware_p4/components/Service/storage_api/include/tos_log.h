#ifndef TOS_LOG_H
#define TOS_LOG_H

#include "esp_err.h"

typedef enum {
    TOS_LOG_ERROR,
    TOS_LOG_WARN,
    TOS_LOG_INFO,
    TOS_LOG_DEBUG,
} tos_log_level_t;

void tos_log_init(void);

void tos_log_write(tos_log_level_t level, const char *tag, const char *fmt, ...);

#define TOS_LOGE(tag, fmt, ...) tos_log_write(TOS_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define TOS_LOGW(tag, fmt, ...) tos_log_write(TOS_LOG_WARN,  tag, fmt, ##__VA_ARGS__)
#define TOS_LOGI(tag, fmt, ...) tos_log_write(TOS_LOG_INFO,  tag, fmt, ##__VA_ARGS__)
#define TOS_LOGD(tag, fmt, ...) tos_log_write(TOS_LOG_DEBUG, tag, fmt, ##__VA_ARGS__)

#endif // TOS_LOG_H
