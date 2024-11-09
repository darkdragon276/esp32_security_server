/* Simple HTTP + SSL Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_eth.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include "esp_tls.h"
#include "html.h"
#include "sdkconfig.h"
#include <esp_https_server.h>

#define WIFI_SSID "ESP32_TimeCounter"
#define WIFI_PASS "12345678"
static const char *TAG = "MAIN";

// Start time (in milliseconds) since the ESP32 started
static uint32_t start_time = 0;
char response[512];

// HTTPS request handler function
esp_err_t time_get_handler(httpd_req_t *req)
{
    // Calculate elapsed time in seconds
    int current_time = (esp_log_timestamp() - start_time) / 1000;

    // Generate HTML response
    snprintf(response, sizeof(response), html_oneline_page, current_time);

    // Send response to client
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t favicon_get_handler(httpd_req_t *req)
{
    // Respond with a blank icon or a placeholder
    httpd_resp_send(req, "", 0);
    return ESP_OK;
}

httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = time_get_handler};

httpd_uri_t favicon_uri = {
    .uri = "/favicon.ico", .method = HTTP_GET, .handler = favicon_get_handler, .user_ctx = NULL};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server");

    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();

    extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
    extern const unsigned char servercert_end[] asm("_binary_servercert_pem_end");
    conf.servercert = servercert_start;
    conf.servercert_len = servercert_end - servercert_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[] asm("_binary_prvtkey_pem_end");
    conf.prvtkey_pem = prvtkey_pem_start;
    conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    esp_err_t ret = httpd_ssl_start(&server, &conf);
    if (ESP_OK != ret) {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &favicon_uri);
    return server;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_ssl_stop(server);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                               void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server) {
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop https server");
        }
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
               .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();
}
