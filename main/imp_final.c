// Uncomment one of the lines to select the implementation
#define WITH_RTOS
// #define BARE_METAL

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

// Pin configuration
#define LED1_PIN GPIO_NUM_2 // Green LED
#define LED2_PIN GPIO_NUM_4 // Blue LED
#define BUTTON_PIN GPIO_NUM_25

// Debounce delay
#define DEBOUNCE_TIME_MS 200
volatile uint32_t last_interrupt_time = 0;

// Flag to control blinking
volatile int can_blink = 1;

#ifdef BARE_METAL
// ====================
// Bare-metal implementation
// ====================
void app_main() {
    int current_button_state;

    // Configure the pins
    gpio_reset_pin(LED1_PIN);
    gpio_set_direction(LED1_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    printf("Bare-metal mode started.\n");

    while (1) {
        // Read the current button state
        current_button_state = gpio_get_level(BUTTON_PIN);

        // If the button is pressed, turn on the LED, otherwise turn it off
        if (current_button_state == 0) {
            gpio_set_level(LED1_PIN, 1); // Turn on the LED
        } else {
            gpio_set_level(LED1_PIN, 0); // Turn off the LED
        }

        // Debounce with a small delay
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#endif

#ifdef WITH_RTOS
// ====================
// RTOS implementation
// ====================

// Semaphore for synchronization
SemaphoreHandle_t buttonSemaphore;

// Button interrupt handler
static void IRAM_ATTR button_isr_handler(void *arg) {
    uint32_t current_time = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;

    // Check if enough time has passed since the last interrupt
    if ((current_time - last_interrupt_time) > DEBOUNCE_TIME_MS) {
        last_interrupt_time = current_time; // Update the last interrupt time

        // Reset the blinking flag
        can_blink = 0;

        // Release the semaphore
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(buttonSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// Task for controlling the Green LED
void led1_task(void *pvParameter) {
    int isLampOn = 0; // State of the Green LED

    printf("RTOS task for Green Led started.\n");

    while (1) {
        // Wait for the semaphore
        if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY)) {
            printf("Detected higher-priority task: handling button press.\n");

            // Toggle the first LED state
            isLampOn = !isLampOn;
            gpio_set_level(LED1_PIN, isLampOn);
            printf("Button pressed, Green is now: %s\n", isLampOn ? "ON" : "OFF");

            // Reset the blinking flag and delay
            can_blink = 0;
        }
    }
}

// Task for blinking the Blue LED
void led2_task(void *pvParameter) {
    while (1) {
        // If blinking is not allowed, wait for a delay
        if (!can_blink) {
            vTaskDelay(pdMS_TO_TICKS(2000)); // Delay before starting blinking
            can_blink = 1; // Enable blinking
            printf("No higher-priority tasks, switching to blinking Blue.\n");
        }

        // Toggle the second LED state
        gpio_set_level(LED2_PIN, 1); // Turn on the second LED
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500 ms
        gpio_set_level(LED2_PIN, 0); // Turn off the second LED
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500 ms
    }
}

void app_main() {
    // Configure the Green LED pin
    gpio_reset_pin(LED1_PIN);
    gpio_set_direction(LED1_PIN, GPIO_MODE_OUTPUT);

    // Configure the Blue LED pin
    gpio_reset_pin(LED2_PIN);
    gpio_set_direction(LED2_PIN, GPIO_MODE_OUTPUT);

    // Configure the button pin
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    // Create a semaphore
    buttonSemaphore = xSemaphoreCreateBinary();
    if (buttonSemaphore == NULL) {
        printf("Failed to create semaphore.\n");
        return;
    }

    // Set up the interrupt handler for the button
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    if (gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL) != ESP_OK) {
        printf("Failed to set ISR handler.\n");
        return;
    }

    // Configure the GPIO for the interrupt
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);

    // Create the task for controlling the first LED
    if (xTaskCreate(led1_task, "LED1 Task", 2048, NULL, 10, NULL) != pdPASS) {
        printf("Failed to create LED1 Task.\n");
    }

    // Create the task for blinking the second LED
    if (xTaskCreate(led2_task, "LED2 Task", 2048, NULL, 5, NULL) != pdPASS) {
        printf("Failed to create LED2 Task.\n");
    }
}
#endif
