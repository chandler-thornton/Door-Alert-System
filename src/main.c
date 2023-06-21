/* GENERAL */
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"

#define LOG_LEVEL_LOCAL     ESP_LOG_VERBOSE        
#define LOG_TAG             "MAIN"

/* GPIO */
#define DOOR_SENSOR_PIN     GPIO_NUM_19
#define LED_PIN             GPIO_NUM_13

/* TWILIO */
static const char *TAG = "HTTP_CLIENT";

//Initialize info for message
char account_sid[] = "PLACEHOLDER";
char auth_token[] = "PLACEHOLDER";
char recipient_num[] = "PLACEHOLDER";
char sender_num[] = "PLACEHOLDER";

char message_open[] = "Bedroom: Door opened";
char message_close[] = "Bedroom: Door closed";


void twilio_send_sms_open(void *pvParameters)
{

    char twilio_url[200];
    snprintf(twilio_url,
             sizeof(twilio_url),
             "%s%s%s",
             "https://api.twilio.com/2010-04-01/Accounts/",
             account_sid,
             "/Messages");

    char post_data[200];

    snprintf(post_data,
             sizeof(post_data),
             "%s%s%s%s%s%s",
             "To=",
             recipient_num,
             "&From=",
             sender_num,
             "&Body=",
             message_open);

    esp_http_client_config_t config = {
        .url = twilio_url,
        .method = HTTP_METHOD_POST,
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,  //Specify transport type
        .crt_bundle_attach = esp_crt_bundle_attach, //Attach the certificate bundle 
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_username(client, account_sid);
    esp_http_client_set_password(client, auth_token);

    ESP_LOGI(TAG, "post = %s", post_data);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 201)
        {
            ESP_LOGI(TAG, "Message sent Successfully");
        }
        else
        {
            ESP_LOGI(TAG, "Message sent Failed");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Message sent Failed");
    }
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void twilio_send_sms_close(void *pvParameters)
{

    char twilio_url[200];
    snprintf(twilio_url,
             sizeof(twilio_url),
             "%s%s%s",
             "https://api.twilio.com/2010-04-01/Accounts/",
             account_sid,
             "/Messages");

    char post_data[200];

    snprintf(post_data,
             sizeof(post_data),
             "%s%s%s%s%s%s",
             "To=",
             recipient_num,
             "&From=",
             sender_num,
             "&Body=",
             message_close);

    esp_http_client_config_t config = {
        .url = twilio_url,
        .method = HTTP_METHOD_POST,
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,  //Specify transport type
        .crt_bundle_attach = esp_crt_bundle_attach, //Attach the certificate bundle 
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_username(client, account_sid);
    esp_http_client_set_password(client, auth_token);

    ESP_LOGI(TAG, "post = %s", post_data);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 201)
        {
            ESP_LOGI(TAG, "Message sent Successfully");
        }
        else
        {
            ESP_LOGI(TAG, "Message sent Failed");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Message sent Failed");
    }
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void app_main(void)
{
    /* INITIALIZE NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    /* INITIALIZE GPIOs */
    int PREVIOUS_DOOR_PIN_LEVEL = gpio_get_level(DOOR_SENSOR_PIN);
    int CURRENT_DOOR_PIN_LEVEL;

    esp_rom_gpio_pad_select_gpio(DOOR_SENSOR_PIN);                   
    gpio_set_direction(DOOR_SENSOR_PIN, GPIO_MODE_INPUT);        
    gpio_set_pull_mode(DOOR_SENSOR_PIN, GPIO_PULLUP_ONLY);     

    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    /* MAIN LOOP */
    while(true)    
    {
        CURRENT_DOOR_PIN_LEVEL = gpio_get_level(DOOR_SENSOR_PIN);
        if(CURRENT_DOOR_PIN_LEVEL == 1 && PREVIOUS_DOOR_PIN_LEVEL == 0){
            xTaskCreate(&twilio_send_sms_open, "twilio_send_sms_open", 8192, NULL, 6, NULL);
        }    
        else if(CURRENT_DOOR_PIN_LEVEL == 0 && PREVIOUS_DOOR_PIN_LEVEL == 1){
            xTaskCreate(&twilio_send_sms_close, "twilio_send_sms_close", 8192, NULL, 6, NULL);
        }
        else if(CURRENT_DOOR_PIN_LEVEL == PREVIOUS_DOOR_PIN_LEVEL){
            if(CURRENT_DOOR_PIN_LEVEL == 1){
                gpio_set_level(LED_PIN, 1);
            }
            else{
                gpio_set_level(LED_PIN, 0);
            }
        }
        else{
            ESP_LOGI(LOG_TAG, "ERROR");
        }

        PREVIOUS_DOOR_PIN_LEVEL = CURRENT_DOOR_PIN_LEVEL;
    }
}
