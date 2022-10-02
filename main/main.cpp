// Base includes
#include <cstdio>

// Added includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "leds/ledControle.h"
#include "vehicle/vehicleData.h"
#include "can/canFrames.h"

extern "C"{
    void app_main();
}

extern void app_main()
{
    // Create two Queues, one for leds, and one for display
    MessageBufferHandle_t message_buffer_led;
    MessageBufferHandle_t message_buffer_display;

    message_buffer_led = xMessageBufferCreate(2 * sizeof(struct Vehicle_Data));
    message_buffer_display = xMessageBufferCreate(2 * sizeof(struct Vehicle_Data));

    MessageBufferHandle_t message_buffers[2] = {message_buffer_led, message_buffer_display};

    if ((message_buffer_led != nullptr)  & (message_buffer_display != nullptr)) {
        // CAN init
        initCAN();

        // Create tasks
        xTaskCreatePinnedToCore(
                &updateCAN,
                "UpdateCAN",
                2048,
                (void *) message_buffers,
                2,
                nullptr,
                0
        );

        xTaskCreatePinnedToCore( // Led task
                &updateLEDs,
                "UpdateLEDs",
                2048,
                (void *) message_buffer_led,
                2,
                nullptr,
                0
        );


        // TODO: Create Display task
    }
}