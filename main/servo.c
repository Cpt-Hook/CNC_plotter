//
// Created by standa on 2/10/19.
//

#include <driver/ledc.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include "servo.h"
#include "utils.h"

#define DEFAULT_uS_LOW 400
#define DEFAULT_uS_HIGH 2400
#define PWM_FREQ 50    //Hz
#define SERVO_PIN 32

#define MAX_RESOLUTION (1ULL<<13)   // 13 bit timer
#define PERIOD_uS (1000000.0/PWM_FREQ)
#define MIN_DUTY_CYCLE (float)((DEFAULT_uS_LOW / PERIOD_uS) * MAX_RESOLUTION)
#define MAX_DUTY_CYCLE (float)((DEFAULT_uS_HIGH / PERIOD_uS) * MAX_RESOLUTION)

#define SERVO_DOWN_PERCENT 80
#define SERVO_UP_PERCENT 30

#define SERVO_DELAY_MS 400

static uint32_t get_duty_cycle(int);
static const TickType_t servo_delay = SERVO_DELAY_MS / portTICK_PERIOD_MS; // 500ms delay
static const char *TAG = "SERVO";
static int servo_pos;

int servo_paper_pos = SERVO_DOWN;

void servo_setup(void) {
    servo_attach();
    vTaskDelay(servo_delay);
    servo_detach();
}

void move_servo(int percent) {
    percent = CONSTRAIN(percent, 0, 100);
    if(percent == servo_pos){
        return;
    }

    uint32_t duty_cycle = get_duty_cycle(percent);
    ESP_LOGI(TAG, "Moving servo to %d%%", percent);
    servo_pos = percent;

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

    vTaskDelay(servo_delay);
}

void servo_attach(void) {
    ESP_LOGI(TAG, "Attaching servo");
    ledc_timer_config_t ledc_timer = {
            .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
            .freq_hz = PWM_FREQ,                      // frequency of PWM signal
            .speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
            .timer_num = LEDC_TIMER_0            // timer index
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
            .channel    = LEDC_CHANNEL_0,
            .duty       = get_duty_cycle(SERVO_UP),
            .gpio_num   = SERVO_PIN,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .timer_sel  = LEDC_TIMER_0
    };

    ledc_channel_config(&ledc_channel);
    servo_pos = SERVO_UP;
}

void servo_detach(void) {
    ESP_LOGI(TAG, "Detaching servo");
    ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
}


static uint32_t get_duty_cycle(int percent) {
    percent = CONSTRAIN(percent, 0, 100);
    percent = map(percent, 0, 100, SERVO_DOWN_PERCENT, SERVO_UP_PERCENT);
    return (uint32_t) mapf(percent, 0, 100, MIN_DUTY_CYCLE, MAX_DUTY_CYCLE);
}
