#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- THÔNG TIN WIFI QUÁN CAFE ---
const char* ssid = "Nam";
const char* password = "12345678";

// --- IP TĨNH (Đã sửa theo ipconfig của ông) ---
IPAddress local_IP(10, 114, 192, 200); 
IPAddress gateway(10, 114, 192, 178); 
IPAddress subnet(255, 255, 255, 0);

// Chân cắm chuẩn ESP32-CAM AI-Thinker
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

WebServer server(80);
WebSocketsServer webSocket(81);

void handle_stream() {
  WiFiClient client = server.client();
  client.write("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n");
  while (true) {
    if (!client.connected()) break;
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) break;
    client.write("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: ");
    client.print(fb->len);
    client.write("\r\n\r\n");
    client.write((char *)fb->buf, fb->len);
    client.write("\r\n", 2);
    esp_camera_fb_return(fb);
  }
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    Serial.printf("Lệnh từ Web: %s\n", payload);
    // Ở đây ông có thể dùng Serial.print để gửi sang STM32
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Tắt Brownout detector
  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM; config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM; config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM; config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM; config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; // Độ phân giải thấp cho mượt
  config.jpeg_quality = 12; config.fb_count = 2;

  esp_camera_init(&config);

  // Cấu hình IP tĩnh
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  Serial.println("\nWiFi OK! Sẵn sàng điều khiển.");
  Serial.print("Stream Video: http://"); Serial.print(WiFi.localIP()); Serial.println("/stream");

  server.on("/stream", handle_stream);
  server.begin();
  
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}