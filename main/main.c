#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "radar_sensor.h"

static const char *TAG = "RADAR_WATCH";

void app_main(void)
{
    radar_sensor_t radar_sensor;

    // Initialize radar sensor on UART1 with RX pin 14, TX pin 13
    esp_err_t ret = radar_sensor_init(&radar_sensor, UART_NUM_1, GPIO_NUM_14, GPIO_NUM_13);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize radar sensor: %s", esp_err_to_name(ret));
        return;
    }

    // Begin communication at 256000 baud
    ret = radar_sensor_begin(&radar_sensor, 256000);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to begin radar sensor communication: %s", esp_err_to_name(ret));
        radar_sensor_deinit(&radar_sensor);
        return;
    }

    ESP_LOGI(TAG, "Radar sensor initialized successfully");

    gpio_reset_pin(GPIO_NUM_16);
    gpio_set_direction(GPIO_NUM_16, GPIO_MODE_OUTPUT);

    while (1)
    {
        // Update radar sensor
        if (radar_sensor_update(&radar_sensor))
        {
            radar_target_t target = radar_sensor_get_target(&radar_sensor);

            if (target.detected)
            {
                ESP_LOGI(TAG, "Target detected - X: %.2f mm, Y: %.2f mm, Speed: %.2f cm/s, Distance: %.2f mm, Angle: %.2f°",
                         target.x, target.y, target.speed, target.distance, target.angle);

                gpio_set_level(GPIO_NUM_16, 1);
            }
            else
            {
                ESP_LOGI(TAG, "No target detected");
                gpio_set_level(GPIO_NUM_16, 0);
            }
        }

        // Small delay to prevent overwhelming the system
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Cleanup (this won't be reached in this example)
    radar_sensor_deinit(&radar_sensor);
}