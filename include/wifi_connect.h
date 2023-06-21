#ifndef WIFI_CONNECT_H_
#define WIFI_CONNECT_H_

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

extern int wifi_connect_status;

void wifi_connect(void);

#endif