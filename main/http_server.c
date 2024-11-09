#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_http_server.h"
#include "html.h"

#define WIFI_SSID "ESP32_TimeCounter"
#define WIFI_PASS "12345678"
#define WPA2
static const char *TAG = "MAIN";

// Start time (in milliseconds) since the ESP32 started
static uint32_t start_time = 0;
char response[512];

// HTTP request handler function
esp_err_t time_get_handler(httpd_req_t *req)
{
    // Calculate elapsed time in seconds
    int current_time = (esp_log_timestamp() - start_time) / 1000;

    // Generate HTML response
    snprintf(response, sizeof(response), html_oneline_page, current_time);

    // Send response to client
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

esp_err_t favicon_get_handler(httpd_req_t *req)
{
    // Respond with a blank icon or a placeholder
    httpd_resp_send(req, "", 0);
    return ESP_OK;
}

// Initialize and start the HTTP server
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    // Start the HTTP server
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handler for the root path
        httpd_uri_t time_counter_uri = {
            .uri = "/", .method = HTTP_GET, .handler = time_get_handler, .user_ctx = NULL};
        httpd_uri_t favicon_uri = {.uri = "/favicon.ico",
                                   .method = HTTP_GET,
                                   .handler = favicon_get_handler,
                                   .user_ctx = NULL};
        httpd_register_uri_handler(server, &time_counter_uri);
        httpd_register_uri_handler(server, &favicon_uri);
        ESP_LOGI(TAG, "Web server started");
    } else {
        ESP_LOGI(TAG, "Failed to start web server");
    }
    return server;
}

// Stop the HTTP server
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}

// Wi-Fi event handler
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                               void *event_data)
{
    if (event_id == WIFI_EVENT_AP_START) {
        // Start time counting when AP starts
        start_time = esp_log_timestamp();
        ESP_LOGI(TAG, "Wi-Fi AP started. Time counting started.");
        start_webserver();
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {.ssid = WIFI_SSID,
               .ssid_len = strlen(WIFI_SSID),
               .channel = 1,
               .password = WIFI_PASS,
               .max_connection = 4,
#ifdef WPA2
               .authmode = WIFI_AUTH_WPA_WPA2_PSK},
#elif WPA3
               .authmode = WIFI_AUTH_WPA3_PSK},
#endif
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
}
