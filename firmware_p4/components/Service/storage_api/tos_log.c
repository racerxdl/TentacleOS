#include "tos_log.h"
#include "tos_config.h"
#include "tos_storage_paths.h"
#include "storage_init.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static const char *TAG = "tos_log";

#define LOG_FILE      TOS_PATH_LOGS "/system.log"
#define LOG_FILE_BAK  TOS_PATH_LOGS "/system.log.bak"
#define LOG_MAX_SIZE  (2 * 1024 * 1024)  // 2MB

static tos_log_level_t s_min_level = TOS_LOG_INFO;

static const char *level_str[] = {"E", "W", "I", "D"};

static tos_log_level_t parse_level(const char *str) {
    if (!str) return TOS_LOG_INFO;
    if (strcmp(str, "debug") == 0) return TOS_LOG_DEBUG;
    if (strcmp(str, "error") == 0) return TOS_LOG_ERROR;
    if (strcmp(str, "warn")  == 0) return TOS_LOG_WARN;
    return TOS_LOG_INFO;
}

void tos_log_init(void) {
    s_min_level = parse_level(g_config_system.log_level);
    ESP_LOGI(TAG, "Log system initialized (level: %s)", g_config_system.log_level);
}

static void rotate_if_needed(void) {
    struct stat st;
    if (stat(LOG_FILE, &st) != 0) return;
    if (st.st_size < LOG_MAX_SIZE) return;

    remove(LOG_FILE_BAK);
    rename(LOG_FILE, LOG_FILE_BAK);
    ESP_LOGI(TAG, "Log rotated: system.log -> system.log.bak");
}

void tos_log_write(tos_log_level_t level, const char *tag, const char *fmt, ...) {
    if (level > s_min_level) return;
    if (!storage_is_mounted()) return;

    rotate_if_needed();

    FILE *f = fopen(LOG_FILE, "a");
    if (!f) return;

    // Timestamp (uptime in seconds)
    uint32_t uptime = (uint32_t)(esp_log_timestamp() / 1000);
    fprintf(f, "[%6lu] [%s] [%s] ", (unsigned long)uptime, level_str[level], tag ? tag : "?");

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);

    fprintf(f, "\n");
    fclose(f);
}
