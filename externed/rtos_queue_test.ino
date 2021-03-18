// セマフォとキューの扱い方を理解する

#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

TaskHandle_t TaskBlink_Handler;
TaskHandle_t TaskButton_Handler;
TaskHandle_t TaskSerial_Handler;

QueueHandle_t InterTaskQueue;
SemaphoreHandle_t SerialSemaphore;

void TaskBlink(void *pvParameters);
void TaskButton(void *pvParameters);
void TaskSerial(void *pvParameters);

void setup()
{
    Serial.begin(9600);

    while (!Serial)
    {
        ;
    }
    if (SerialSemaphore == NULL)
    {
        SerialSemaphore = xSemaphoreCreateMutex();
        if ((SerialSemaphore) != NULL)
            xSemaphoreGive((SerialSemaphore));
    }

    InterTaskQueue = xQueueCreate(1, sizeof(int));
    if (InterTaskQueue != NULL)
    {
        xTaskCreate(TaskBlink, "Blink", 128, NULL, 2, &TaskBlink_Handler);
        xTaskCreate(TaskButton, "Button", 128, NULL, 1, &TaskButton_Handler);
        xTaskCreate(TaskSerial, "Serial", 128, NULL, 0, &TaskSerial_Handler);

        vTaskStartScheduler();
    }
}

void loop()
{
    // empty
}

void TaskSerial(void *pvParameters)
{
    (void)pvParameters;
    for (;;)
    {
        while (Serial.available() > 0)
        {
            String str = Serial.readString();
            if (str[0] == 's')
            {
                vTaskSuspend(TaskBlink_Handler);
                Serial.println("Suspend!");
            }
            else if (str[0] == 'r')
            {
                vTaskResume(TaskBlink_Handler);
                Serial.println("Resume!");
            }
            else
            {
                int val = str.toInt();
                xQueueSend(InterTaskQueue, &val, portMAX_DELAY); //valをblinkに送る
            }
            vTaskDelay(1);
        }
    }
}

void TaskButton(void *pvParameters)
{
    (void)pvParameters;
    const int DIN_PIN = 7;
    int val;

    pinMode(DIN_PIN, INPUT_PULLUP);
    for (;;)
    {
        val = digitalRead(DIN_PIN);
        if (val == LOW)
        {
            if (xSemaphoreTake(SerialSemaphore, (TickType_t)5) == pdTRUE)
            {
                Serial.println("Button Pushed");
                delay(1500);
                xSemaphoreGive(SerialSemaphore);
            }
        }
        vTaskDelay(1);
    }
}

void TaskBlink(void *pvParameters)
{
    const int LED_PIN = 3;
    (void)pvParameters;
    pinMode(LED_PIN, OUTPUT);

    for (;;)
    {
        int pw;
        if (xQueueReceive(InterTaskQueue, &pw, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < 10; i++)
            {
                if (xSemaphoreTake(SerialSemaphore, (TickType_t)5) == pdTRUE)
                {
                    char buf[10];
                    sprintf(buf, "N=%d", i + 1);
                    Serial.println(buf);
                    xSemaphoreGive(SerialSemaphore);
                }
                analogWrite(LED_PIN, pw);
                vTaskDelay(500 / portTICK_PERIOD_MS);
                analogWrite(LED_PIN, 0);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        }
    }
}