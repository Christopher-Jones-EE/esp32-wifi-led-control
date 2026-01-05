#include <string.h>
#define CONFIG_HTTPD_MAX_URI_HANDLERS 20
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define WIFI_SSID "ORBI92"
#define WIFI_PASS "dizzyapple952"

static const char *TAG = "ESP WIFI";
static bool status = 0;
int brightness = 255;
char value_str [12];
int percent = 50;
int speed = 3;
int mode = 0;
char speed_str [12];

static void ledc_config() {
ledc_timer_config_t timer_config = {
.timer_num = LEDC_TIMER_0,
.freq_hz = 5000,
.duty_resolution = LEDC_TIMER_8_BIT,
.speed_mode = LEDC_LOW_SPEED_MODE,
.clk_cfg = LEDC_AUTO_CLK
};

ledc_timer_config(&timer_config);


ledc_channel_config_t channel_config = {
    .timer_sel = LEDC_TIMER_0,
    .channel = LEDC_CHANNEL_0,
    .duty = 0,
    .hpoint = 0,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .gpio_num = GPIO_NUM_2
};

ledc_channel_config(&channel_config); }


static void event_handler (void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "CONNECTING TO WIFI....");
    }
     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "DISCONNECTED, RETRYING ...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "ESP received IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }

}

void wifi_init_sta () {

nvs_flash_init();
esp_netif_init();
ESP_ERROR_CHECK(esp_event_loop_create_default());
esp_netif_create_default_wifi_sta ();

wifi_init_config_t wifi_init = WIFI_INIT_CONFIG_DEFAULT ();
esp_wifi_init(&wifi_init);

wifi_config_t wifi_config = {
.sta = {
.ssid = WIFI_SSID,
.password = WIFI_PASS
}
};


esp_event_handler_instance_register (WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL, NULL);
esp_event_handler_instance_register (IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL, NULL);

esp_wifi_set_mode (WIFI_MODE_STA);
esp_wifi_set_config (WIFI_IF_STA, &wifi_config);
esp_wifi_start ();
ESP_LOGI (TAG, "ESP32 STA mode has been initiated");


}


esp_err_t led_on_handler(httpd_req_t *req) {
  status = 1;
  printf("on\n");
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  printf("mode: %d\n", mode);
  printf("%d", status);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    printf("Handler %s called\n", req->uri);

    return ESP_OK;
}

esp_err_t led_off_handler(httpd_req_t *req) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    printf("off\n");
    status = 0;
    printf("Handler %s called\n", req->uri);
    return ESP_OK;
}

esp_err_t brightness_handler(httpd_req_t *req) {
    char buffer[16];
    if(httpd_req_get_url_query_len(req) < sizeof(buffer)) {
         httpd_req_get_url_query_str(req, buffer, sizeof(buffer));}
        if(httpd_query_key_value(buffer, "value", value_str, sizeof(value_str)) == ESP_OK) {
            brightness = atoi(value_str);
            percent = brightness*100/255;
            snprintf(value_str, sizeof(value_str),"%d", percent);
        }
      
    if (status == 1) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);}
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    printf("Handler %s called\n", req->uri);
    return ESP_OK;
}



esp_err_t updateB_handler (httpd_req_t *req) {

if (status == 0) {
  httpd_resp_send(req, "0", HTTPD_RESP_USE_STRLEN);
}
else {httpd_resp_send(req, value_str, HTTPD_RESP_USE_STRLEN);}
printf("Handler %s called\n", req->uri);
return ESP_OK;
}

esp_err_t status_handler (httpd_req_t *req) {

httpd_resp_send(req, status ? "ON":"OFF", HTTPD_RESP_USE_STRLEN);
printf("Handler %s called\n", req->uri);
return ESP_OK;
}

esp_err_t fade_handler (httpd_req_t *req) {
mode = 1;
printf("fade mode\n");
printf("Handler %s called\n", req->uri);
httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
return ESP_OK;
}

esp_err_t blink_handler (httpd_req_t *req) {

  mode = 2;
  printf("blink mode\n");
  printf("Handler %s called\n", req->uri);
  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t speed_handler (httpd_req_t *req) {
  char buffer[16];
    if(httpd_req_get_url_query_len(req) < sizeof(buffer)) {
         httpd_req_get_url_query_str(req, buffer, sizeof(buffer));}
        if(httpd_query_key_value(buffer, "speed", speed_str, sizeof(speed_str)) == ESP_OK) {
            speed = atoi(speed_str);
        }
        printf("New speed: %d\n", speed);
        printf("Handler %s called\n", req->uri);

      httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);  
        
  return ESP_OK;
}

esp_err_t solid_handler (httpd_req_t *req) {
  mode = 0;
  printf("solid Mode\n");
  printf("Handler %s called\n", req->uri);
  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t index_handler (httpd_req_t *req) {
const char *first_page = 
"<!DOCTYPE html>"
"<html>"
"<head>"
  "<meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>"
  "<title>ESP32 Dashboard</title>"
  "<style>"
    "body {"
      "font-family: Arial;"
      "background: #7bd4edff;"
      "padding: 20px;"
      "text-align: center;}"

      ".card-container {"
      "display: flex;"
      "flex-direction: row;"
      "align-items: center;"
      "justify-content: center;"
      "gap: 30px;"
      "flex-wrap: wrap;}"

    ".card {"
      "background: rgba(189, 87, 189, 1);"
      "padding: 30px;"
      "padding-bottom: 20px;"
      "border: 10px;"
      "border-radius: 40px;"
      "width: 220px;"
      "margin-bottom: 20px;"
      "box-shadow: 0 2px 5px rgba(0,0,0,0.2);}"

    "button {"
      "width: 100%;"
      "padding: 10px;"
      "margin-top: 10px;"
      "box-shadow: 0px 4px 10px black"
      "border: 2px solid blue;"
      "border-radius: 30px;"
      "background: #007bff;"
      "color: black;"
      "font-size: 16px;"
      "cursor: pointer;}"

    "button:active {"
      "background: #0056b3;}"

    ".value {"
      "font-size: 20px;"
      "font-weight:bold;"
      "text-align: center;"
      "padding-top: 20 px;}"

      "h2 {"
      "font-weight: bold;}"

      "h3 {"
      "margin-bottom: 5px;"
      "color: #7bd4edff;}"

".slider {"
  "-webkit-appearance: none;"
  "appearance: none;"
  "width: 80%;"
  "height: 6px;"
  "background: #ddd;"
  "margin-bottom: 15px;"
  "margin-top: 5px;"
  "border: 5px solid black"
  "border-radius: 5px;}"

".slider::-webkit-slider-runnable-track {"
  "height: 6px;"
  "background: #984cafff;"
  "border-radius: 5px;}"

".slider::-moz-range-track {"
  "height: 6px;"
  "background: #984cafff;"
  "border-radius: 5px;}"

".slider::-webkit-slider-thumb {"
  "-webkit-appearance: none;"
 " appearance: none;"
  "width: 20px;"
  "height: 20px;"
  "background: #007bff;"
  "border-radius: 50%;"
  "cursor: pointer;"
  "margin-top: -7px;}" 

".slider::-moz-range-thumb {"
  "width: 20px;"
  "height: 20px;"
  "background: #007bff;"
  "border-radius: 50%;"
  "cursor: pointer;}"
   
  "</style>"
"</head>"
"<body>"
  "<h1>Dashboard</h1>"

  "<div class=\"card-container\">"
  "<!-- LED Control -->"
  "<div class=\"card\">"
    "<h2>LED Control</h2>"
    "<h3>Brightness</h3>"
    "<input class=\"slider\" type=\"range\" name=\"value\" min=\"0\" max=\"255\" value=\"128\" id=\"brightness\" oninput=\"updateBrightness(this.value)\">"
    "<button onclick=\"ledOn()\">LED ON</button>"
    "<button onclick=\"ledOff()\">LED OFF</button>"
  "</div>"

  "<!-- Sensor Section -->"
  "<div class=\"card\">"
    "<h2>LED</h2>"
    "<div id=\"Status\" class=\"value\">Status: </div>"
    "<div id=\"Brightness\" class=\"value\">Brightness: </div>"
    "<div id=\"Mode\" class=\"value\">Mode: Solid</div>"
    "<div id=\"Speed\" class=\"value\">Speed: 3</div>"
    "</div>"


  "<div class=\"card\">"
  "<h2>Mode</h2>"
  "<h3>Speed</h3>"
  "<input type=\"range\" name=\"speed\" id=\"speed\" class=\"slider\" min=\"1\" max=\"5\" oninput=\"updateSpeed(this.value)\">"
  "<button onclick=\"Solid()\">Solid</button>\n"
  "<button onclick=\"Blink()\">Blink</button>\n"
  "<button onclick=\"Fade()\">Fade</button>\n"
  "</div>"
  "</div>"

    "<script>"

    "let status = \"OFF\";"
    "let x = 50;"
    "let bright = 50;"
    "document.getElementById(\"Status\").innerText = \"Status: OFF\";\n"
    "document.getElementById(\"Brightness\").innerText = \"Brightness: 0%\";\n"

      "function Blink() {"
      "document.getElementById(\"Mode\").innerText=\"Mode: Blink\";"
      "fetch('/blink');}\n"

      "function Fade() {"
      "document.getElementById(\"Mode\").innerText=\"Mode: Fade\";"
      "fetch('/fade');}\n"

      "function Solid() {"
      "document.getElementById(\"Mode\").innerText=\"Mode: Solid\";"
      "fetch('/solid');}\n"

      "function updateSpeed(value){"
      "document.getElementById(\"Speed\").innerText=\"Speed: \" + value;"
      "fetch(`/speed?speed=${value}`);}"

      "function updateBrightness(value) {"
      "bright = value;\n"
      "if (status == \"ON\") {"
      "x = Math.round(value*100/255);\n"
      "document.getElementById(\"Brightness\").innerText = \"Brightness: \" + x + \"%\";}\n"
      "else {document.getElementById(\"Brightness\").innerText = \"Brightness: 0%\";}\n"
      "fetch(`/brightness?value=${value}`);}\n"

      "function ledOn() {"
      "fetch(`/brightness?value=${bright}`);\n"
        "fetch('/led/on');\n"
        "status = \"ON\";\n"
        "document.getElementById(\"Status\").innerText = \"Status: ON\";\n"
        "document.getElementById(\"Brightness\").innerText = \"Brightness: \" + Math.round(bright*100/255) + \"%\";\n}"

      "function ledOff() {"
      "fetch('/led/off');"
      "document.getElementById(\"Brightness\").innerText = \"Brightness: 0%\";\n"
      "status = \"OFF\";"
      "document.getElementById(\"Status\").innerText = \"Status: OFF\";\n}"

  "</script>"
"</body>"
"</html>";

printf("Handler %s called\n", req->uri);
httpd_resp_set_type(req, "text/html");
httpd_resp_send (req, first_page, HTTPD_RESP_USE_STRLEN);
printf("webpage was summoned\n");
return ESP_OK;
}

httpd_handle_t start_webserver () {
httpd_handle_t server = NULL;
httpd_config_t config = HTTPD_DEFAULT_CONFIG ();

config.recv_wait_timeout = 10;  // seconds
config.send_wait_timeout = 10; 

if(httpd_start(&server, &config) == ESP_OK) {

httpd_uri_t first_page_uri = {
.uri = "/",
.user_ctx = NULL,
.handler = index_handler,
.method = HTTP_GET
};

httpd_uri_t solid_uri = {
  .uri = "/solid",
  .handler = solid_handler,
  .method = HTTP_GET
};

httpd_uri_t speed_uri = {
  .uri = "/speed",
  .handler = speed_handler,
  .method = HTTP_GET
};

httpd_uri_t blink_uri = {
.method = HTTP_GET,
.user_ctx = NULL,
.handler = blink_handler,
.uri = "/blink"
};

httpd_uri_t fade_uri = {
.method = HTTP_GET,
.handler = fade_handler,
.uri = "/fade",
.user_ctx = NULL
};

httpd_uri_t uri_led_on = {
    .uri = "/led/on",
    .method = HTTP_GET,
    .handler = led_on_handler
};

httpd_uri_t uri_led_off = {
    .uri = "/led/off",
    .method = HTTP_GET,
    .handler = led_off_handler
};

httpd_uri_t uri_brightness = {
    .uri = "/brightness",
    .method = HTTP_GET,
    .handler = brightness_handler
};

httpd_register_uri_handler(server, &speed_uri);
httpd_register_uri_handler(server, &solid_uri);
httpd_register_uri_handler(server, &first_page_uri);
httpd_register_uri_handler(server, &uri_led_on);
httpd_register_uri_handler(server, &uri_led_off);
httpd_register_uri_handler(server, &uri_brightness);
httpd_register_uri_handler(server, &fade_uri);
httpd_register_uri_handler(server, &blink_uri);

ESP_LOGI(TAG, "ESP SERVER HAS STARTED");
return server;

}

ESP_LOGE(TAG, "ESP FAILED TO START SERVER");
return NULL;
}

void ledTask(void *pvParameters) {
while (1) {
  if (status == 1) {
switch (mode) {
  case 0:
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  break;
  case 1: 
  int delay=25;
  switch(speed) {
    case 1:delay = 35;
    break;
    case 2: delay = 28;
    break;
    case 3:delay = 22;
    break;
    case 4:delay = 16;
    break;
    case 5:delay = 10;
    break;
  }
    for(int i=brightness; i>0; i--) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(delay/portTICK_PERIOD_MS);}
    for(int i=0; i<brightness; i++) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(delay/portTICK_PERIOD_MS);}
  break;
  case 2: 
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay((1000/speed)/portTICK_PERIOD_MS);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay((1000/speed)/portTICK_PERIOD_MS);
  break;
}
  }
  else {ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);}
    vTaskDelay(10);
}
}


void app_main () {

gpio_set_direction(22, GPIO_MODE_OUTPUT);
ledc_config();
wifi_init_sta (); 
start_webserver();
xTaskCreate(ledTask, "ledTask", 2048, NULL, 5, NULL);
}



