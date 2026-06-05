/**
 * @file  sd_storage.c
 * @brief SD card — raw flow sample logging (CSV format).
 *
 * File format: /sdcard/sess_<id>.csv
 *   timestamp_ms,flow_lpm
 *   1000,0.0
 *   1020,12.5
 *   ...
 */

#include "storage.h"
#include "befine_config.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "SD_STORE";
static FILE *s_csv_file = NULL;
static bool  s_sd_ready = false;

esp_err_t sd_init(void)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs   = BEFINE_SD_CS_GPIO;
    slot_config.host_id   = host.slot;

    /* Init SPI bus */
    spi_bus_config_t bus_cfg = {
        .mosi_io_num   = BEFINE_SD_MOSI_GPIO,
        .miso_io_num   = BEFINE_SD_MISO_GPIO,
        .sclk_io_num   = BEFINE_SD_CLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files              = 5,
        .allocation_unit_size   = 16 * 1024,
    };
    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdspi_mount(
        BEFINE_SD_MOUNT_POINT, &host, &slot_config, &mount_cfg, &card);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SD card not found or mount failed: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    s_sd_ready = true;
    ESP_LOGI(TAG, "SD card mounted — %llu MB",
             (unsigned long long)card->csd.capacity * card->csd.sector_size / (1024 * 1024));
    return ESP_OK;
}

esp_err_t sd_open_session(uint64_t session_id)
{
    if (!s_sd_ready) return ESP_ERR_INVALID_STATE;
    if (s_csv_file) fclose(s_csv_file);

    char path[64];
    snprintf(path, sizeof(path),
             BEFINE_SD_MOUNT_POINT "/sess_%llu.csv",
             (unsigned long long)session_id);

    s_csv_file = fopen(path, "w");
    if (!s_csv_file) {
        ESP_LOGE(TAG, "Cannot open %s", path);
        return ESP_ERR_NOT_FOUND;
    }
    fprintf(s_csv_file, "timestamp_ms,flow_lpm\n");
    ESP_LOGI(TAG, "CSV logging started: %s", path);
    return ESP_OK;
}

esp_err_t sd_write_sample(uint32_t ts_ms, float flow_lpm)
{
    if (!s_csv_file) return ESP_ERR_INVALID_STATE;
    fprintf(s_csv_file, "%lu,%.2f\n", (unsigned long)ts_ms, flow_lpm);
    return ESP_OK;
}

esp_err_t sd_close_session(void)
{
    if (s_csv_file) {
        fclose(s_csv_file);
        s_csv_file = NULL;
        ESP_LOGI(TAG, "CSV session file closed");
    }
    return ESP_OK;
}
