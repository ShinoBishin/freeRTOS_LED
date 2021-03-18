#include <Arduino_FreeRTOS.h>
#define LED 3

void TaskBlink(void *pvParameters);
void TaskChecker(void *pvParameters);

void setup()
{
    Serial.begin(9600);

    xTaskCreate(TaskBlink, "Blink", 128, NULL, 2, NULL);

    xTaskCreate(TaskChecker, "Checker", 128, NULL, 1, NULL);
}

void loop()
{
}
unsigned char led_state;
int value = 0;
int led_pw = 0;
const int sensorMIN = 100;
const int sensorMAX = 950;

void TaskBlink(void *pvParameters)
{
    (void)pvParameters;

    pinMode(LED, OUTPUT);

    for (;;)
    {
        led_pw = (value - sensorMIN) * 255 / (sensorMAX - sensorMIN);
        if (led_pw < 0)
        {
            led_pw = 0;
        }
        if (led_pw > 255)
        {
            led_pw = 255;
        }
        analogWrite(LED, led_pw);
        led_state = HIGH;
        Serial.print("HIGH, ");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        analogWrite(LED, 0);
        led_state = LOW;
        Serial.print("LOW, ");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void TaskChecker(void *pvParameters)
{
    (void)pvParameters;
    value = analogRead(A0);
    // int value = analogRead(A0);
    int i = 0;
    for (;;)
    {

        value = analogRead(A0);

        Serial.println(value);
        delay(100);
        i++;
        if (i == 30)
        {
            if ((value < 700) && (led_state == HIGH))
            {
                Serial.println(", くらい");
                i = 0;
            }
            else if ((value > 900) && (led_state == HIGH))
            {
                Serial.println(", 明るすぎ！");
                i = 0;
            }
            else
            {
                i = 0;
            }
        }
        vTaskDelay(1);
    }
}
