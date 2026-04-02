#include "esp_camera.h"
#include <WiFi.h>
#include <esp_http_server.h> 
#include <ArduinoJson.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "index.h"           

const char* ssid = "Nam Son";
const char* password = "061224010606";

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

httpd_handle_t control_httpd = NULL; 
httpd_handle_t stream_httpd = NULL;  

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  char part_buf[64];

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  
  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK) return res;

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      res = ESP_FAIL;
    } else {
      size_t hlen = snprintf(part_buf, 64, _STREAM_PART, fb->len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
      if(res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
      if(res == ESP_OK) res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      esp_camera_fb_return(fb);
    }
    if(res != ESP_OK) break; 
    
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  return res;
}

esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, index_html, strlen(index_html));
}

esp_err_t ws_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) return ESP_OK; 

  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  
  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
  if (ret != ESP_OK) return ret;

  if (ws_pkt.len) {
    uint8_t *buf = (uint8_t*)calloc(1, ws_pkt.len + 1); 
    if (buf) {
      ws_pkt.payload = buf;
      ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
      if (ret == ESP_OK) {
        buf[ws_pkt.len] = 0;
        StaticJsonDocument<200> doc;
        if (deserializeJson(doc, (char*)buf) == DeserializationError::Ok) {
          int L = doc["L"]; int R = doc["R"]; int A = doc["A"]; int P = doc["P"];
          Serial.printf("Trai:%d | Phai:%d | Phanh:%d | Bom:%d\n", L, R, A, P);
        }
      }
      free(buf);
    }
  }
  return ret;
}

void startServers() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  config.server_port = 80;
  config.ctrl_port = 32768; 
  if (httpd_start(&control_httpd, &config) == ESP_OK) {
    httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL };
    httpd_uri_t ws_uri = { .uri = "/ws", .method = HTTP_GET, .handler = ws_handler, .user_ctx = NULL, .is_websocket = true };
    httpd_register_uri_handler(control_httpd, &index_uri);
    httpd_register_uri_handler(control_httpd, &ws_uri);
  }

  config.server_port = 81;
  config.ctrl_port = 32769; 
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL };
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM; config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM; config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM; config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM; config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; 
  config.pixel_format = PIXFORMAT_JPEG;
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA; 
    config.jpeg_quality = 12; 
    config.fb_count = 2; 
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }
  if (esp_camera_init(&config) != ESP_OK) { Serial.println("Loi Camera!"); return; }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  Serial.println("\nIP cua Robot: " + WiFi.localIP().toString());
  startServers(); 
  } 

void loop() {
  delay(10); 
}