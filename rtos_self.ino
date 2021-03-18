#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#define LED_PIN 3

TaskHandle_t TaskBlink_Handler;
TaskHandle_t TaskCds_Handler;
TaskHandle_t TaskSerial_Handler;

QueueHandle_t InterTaskQueue;
// SemaphoreHandle_t SerialSemaphore;

void TaskBlink(void *pvParameters);
void TaskCds(void *pvParameters);
// void TaskSerial(void *pvParameters);

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        ;
    }

    InterTaskQueue = xQueueCreate(1, sizeof(int));
    if (InterTaskQueue != NULL)
    {
        xTaskCreate(TaskBlink, "Blink", 128, NULL, 2, &TaskBlink_Handler);
        xTaskCreate(TaskCds, "cds", 128, NULL, 1, &TaskCds_Handler);
        // xTaskCreate(TaskSerial, "Serial", 128, NULL, 0, &TaskSerial_Handler);

        vTaskStartScheduler();
    }
}

void loop()
{
}

unsigned char led_state;

void TaskBlink(void *pvParameters)
{
    (void)pvParameters;
    pinMode(LED_PIN, OUTPUT);
    for (;;)
    {
        int pw;
        if (xQueueReceive(InterTaskQueue, &pw, portMAX_DELAY) == pdPASS)
        {
            analogWrite(LED_PIN, pw);
            led_state = HIGH;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            analogWrite(LED_PIN, 0);
            led_state = LOW;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

void TaskCds(void *pvParameters)
{
    int cds;
    int pw_val = 125;
    (void)pvParameters;
    for (;;)
    {
        cds = analogRead(A0);
        Serial.println(cds);
        xQueueSend(InterTaskQueue, &pw_val, portMAX_DELAY);
        if ((cds < 700) && (led_state == HIGH))
        {
            Serial.println("Dark!");
            pw_val = 255;
            xQueueSend(InterTaskQueue, &pw_val, portMAX_DELAY);
            Serial.println("出力あげました");
        }
        else if ((cds > 900) && (led_state == HIGH))
        {
            Serial.println("Britely!");
            pw_val = 30;
            xQueueSend(InterTaskQueue, &pw_val, portMAX_DELAY);
            Serial.println("出力落としました");
        }
        vTaskDelay(1);
    }
}