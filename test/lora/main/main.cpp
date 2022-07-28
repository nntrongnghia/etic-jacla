#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_sleep.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include "rak3172.h"

#include "LoRaWAN_Default.h"

static RAK3172_t _Device = {
    .Interface = (uart_port_t)CONFIG_RAK3172_UART,
    .Rx = (gpio_num_t)18,
    .Tx = (gpio_num_t)17,
    .Baudrate = (RAK3172_Baud_t)CONFIG_RAK3172_BAUD,
};

static const char* TAG 							= "LORA";

static void applicationTask(void* p_Parameter)
{
    bool Status;
    RAK3172_Error_t Error;

    Error = RAK3172_Init(&_Device);
    if(Error != RAK3172_OK)
    {
        ESP_LOGE(TAG, "Can not initialize RAK3172! Error: 0x%04X", Error);
    }

    ESP_LOGI(TAG, "Firmware: %s", _Device.Firmware.c_str());
    ESP_LOGI(TAG, "Serial number: %s", _Device.Serial.c_str());
    ESP_LOGI(TAG, "Current mode: %u", _Device.Mode);

    Error = RAK3172_Init_LoRaWAN(&_Device, 16, 3, RAK_JOIN_OTAA, DEVEUI, APPEUI, APPKEY, 'A', RAK_BAND_EU868, RAK_SUB_BAND_NONE);
    if(Error != RAK3172_OK)
    {
        ESP_LOGE(TAG, "Can not initialize RAK3172 LoRaWAN! Error: 0x%04X", Error);
    }

    Error = RAK3172_Joined(&_Device, &Status);
    if(Error != RAK3172_OK)
    {
        ESP_LOGE(TAG, "Error: 0x%04X", Error);
    }

    if(!Status)
    {
        ESP_LOGI(TAG, "Not joined. Rejoin...");

        Error = RAK3172_StartJoin(&_Device, 0, LORAWAN_JOIN_ATTEMPTS, true, LORAWAN_MAX_JOIN_INTERVAL_S, NULL);
        if(Error != RAK3172_OK)
        {
            ESP_LOGE(TAG, "Can not join network!");
        }
        else
        {
            ESP_LOGI(TAG, "Joined...");

            /*char Payload[] = {'H', 'e', 'l', 'l', 'o', ' ', 'J', 'a', 'c', 'l', 'a'};

            Error = RAK3172_LoRaWAN_Transmit(&_Device, 1, Payload, sizeof(Payload), LORAWAN_TX_TIMEOUT_S, true, NULL);
            if(Error == RAK3172_INVALID_RESPONSE)
            {
                ESP_LOGE(TAG, "Can not transmit message network!");
            }
            else
            {
                ESP_LOGI(TAG, "Message transmitted...");
            }*/

            /*std::string s0 ("Initial string");
            ESP_LOGI(TAG, "0");
            Error = RAK3172_LoRaWAN_Receive(&_Device, NULL, NULL, NULL, LORAWAN_RX_TIMEOUT_S);
             ESP_LOGI(TAG, "%d", Error);
            if(Error == RAK3172_INVALID_RESPONSE)
            {
                ESP_LOGI(TAG, "Invalid response");
            }
            else if (Error == RAK3172_INVALID_ARG)
            {
                ESP_LOGI(TAG, "Arguments");
            }
            else if(Error == RAK3172_OK)
            {
                ESP_LOGI(TAG, "Success !");
            }*/
            std::string payload;

            RAK3172_t* p_Device = &_Device;
            std::string *p_Payload = &payload;
            int* p_RSSI = NULL;
            int* p_SNR = NULL;
            uint32_t Timeout = 60;

            ESP_LOGI(TAG, "1");
            ESP_LOGI(TAG, "Initialize module in LoRaWAN mode...");

            uint32_t Now;

            if((p_Payload == NULL) || (Timeout <= 1))
            {
                ESP_LOGI(TAG, "Initialize module in LoRaWAN mode...");
            }
            ESP_LOGI(TAG, "2");

            Now = esp_timer_get_time() / 1000ULL;
            std::string* Line;

            while(true)
            {
                ESP_LOGI(TAG, "3");
                if(xQueueReceive(p_Device->Internal.Rx_Queue, &Line, 100 / portTICK_PERIOD_MS) == pdPASS)
                {
                    int Index;

                    ESP_LOGI(TAG, "Receive event: %s", Line->c_str());

                    // Get the RX metadata first.
                    if(Line->find("RX") != std::string::npos)
                    {
                        std::string Dummy;

                        // Get the RSSI value.
                        if(p_RSSI != NULL)
                        {
                            Index = Line->find(",");
                            Dummy = Line->substr(Index + 7, Index + Line->find(",", Index) + 1);
                            *p_RSSI = std::stoi(Dummy);
                        }

                        // Get the SNR value.
                        if(p_SNR != NULL)
                        {
                            Index = Line->find_last_of(",");
                            Dummy = Line->substr(Index + 6, Line->length() - 1);
                            *p_SNR = std::stoi(Dummy);
                        }
                    }

                    // Then get the data and leave the function.
                    if(Line->find("UNICAST") != std::string::npos)
                    {
                        //Line = p_Device->p_Interface->readStringUntil('\n');
                        ESP_LOGI(TAG, "    Payload: %s", Line->c_str());

                        // Clean up the payload string ("+EVT:Port:Payload")
                        //  - Remove the "+EVT" indicator
                        //  - Remove the port number
                        *p_Payload = Line->substr(Line->find_last_of(":") + 1, Line->length());
                        break;
                    }
                }

                //delete Line;

                ESP_LOGI(TAG, "4");
                if((Timeout > 0) && ((((esp_timer_get_time() / 1000ULL) - Now) / 1000ULL) >= Timeout))
                {
                    ESP_LOGE(TAG, "Receive timeout!");
                    break;
                }
                esp_sleep_enable_timer_wakeup(100 * 1000ULL);
                esp_light_sleep_start();
                vTaskDelay(10 / portTICK_RATE_MS);
                ESP_LOGI(TAG, "7");
            }
        } //else
    } // if(!status)
            
    while(true)
    {
        esp_task_wdt_reset();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "IDF: %s", esp_get_idf_version());
    xTaskCreatePinnedToCore(applicationTask, TAG, 100 * 1024, NULL, 6, NULL, 0);
}

         /*std::string p_Payload;
            int p_RSSI;
            int p_SNR;
                std::string recv;
                RAK3172_SendCommand(&_Device, "AT+RECV=?", &recv, NULL);

                int RSSI = (int8_t)std::stoi(recv);
                ESP_LOGI(TAG, "RECV : %d", RSSI);*/