// การทดลองที่ 1: อ่านค่า Potentiometer
// ตัวอย่างพื้นฐาน ADC ของ ESP32 โดยใช้ ESP-IDF

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

// กำหนด pin ที่ใช้
#define POTENTIOMETER_CHANNEL ADC1_CHANNEL_6  // GPIO34 (ADC1_CH6)
#define DEFAULT_VREF    1100        // ค่าอ้างอิง Vref (mV)
#define NO_OF_SAMPLES   64          // จำนวนครั้งในการ oversampling

static const char *TAG = "ADC_POT";
static esp_adc_cal_characteristics_t *adc_chars;

static void check_efuse(void)
{
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        ESP_LOGI(TAG, "eFuse Two Point: รองรับ");
    } else {
        ESP_LOGI(TAG, "eFuse Two Point: ไม่รองรับ");
    }

    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        ESP_LOGI(TAG, "eFuse Vref: รองรับ");
    } else {
        ESP_LOGI(TAG, "eFuse Vref: ไม่รองรับ");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGI(TAG, "ใช้การปรับเทียบแบบ Two Point Value");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGI(TAG, "ใช้การปรับเทียบแบบ eFuse Vref");
    } else {
        ESP_LOGI(TAG, "ใช้การปรับเทียบแบบ Default Vref");
    }
}

void app_main(void)
{
    check_efuse();

    // กำหนดค่า ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POTENTIOMETER_CHANNEL, ADC_ATTEN_DB_11);

    // ปรับเทียบ ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    ESP_LOGI(TAG, "ทดสอบ ADC Potentiometer ของ ESP32");
    ESP_LOGI(TAG, "Pin: GPIO34 (ADC1_CH6)");
    ESP_LOGI(TAG, "ช่วง: 0-3.3V");
    ESP_LOGI(TAG, "ความละเอียด: 12-bit (0-4095)");
    ESP_LOGI(TAG, "Attenuation: 11dB");
    ESP_LOGI(TAG, "-------------------------");

    while (1) {
        uint32_t adc_reading = 0;

        // อ่านค่าเฉลี่ย
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading += adc1_get_raw(POTENTIOMETER_CHANNEL);
        }
        adc_reading /= NO_OF_SAMPLES;

        // แปลงค่าเป็นแรงดัน
        uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        float voltage = voltage_mv / 1000.0f;

        // คำนวณเป็นเปอร์เซ็นต์ (0–100%)
        float percentage = (adc_reading / 4095.0f) * 100.0f;

        // แสดงผล
        ESP_LOGI(TAG, "ค่า ADC: %" PRIu32 " | แรงดัน: %.2fV | เปอร์เซ็นต์: %.1f%%",
                 adc_reading, voltage, percentage);

        vTaskDelay(pdMS_TO_TICKS(500));  // 500 ms
    }
}
