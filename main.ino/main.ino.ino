#include <xxxx_inferencing.h> // Replace xxxx with your Edge Impulse project name
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vector>

// Camera model selection
#define CAMERA_MODEL_AI_THINKER

// Camera pin definitions
#if defined(CAMERA_MODEL_AI_THINKER)
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
#endif

// Display configuration
#define I2C_SDA 15
#define I2C_SCL 14
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Constants
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3
const int DETECTION_DURATION = 15000; // 15 seconds
const float TIME_PER_OBJECT = 1.5;    // Seconds per object

// Global variables
TwoWire I2Cbus = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Cbus, OLED_RESET);
static bool is_initialised = false;
uint8_t *snapshot_buf;
std::vector<String> unique_objects;
unsigned long processing_start;
bool processing_active = true;
int frame_count = 0;

// Camera configuration
camera_config_t camera_config = {
  .pin_pwdn = PWDN_GPIO_NUM,
  .pin_reset = RESET_GPIO_NUM,
  .pin_xclk = XCLK_GPIO_NUM,
  .pin_sscb_sda = SIOD_GPIO_NUM,
  .pin_sscb_scl = SIOC_GPIO_NUM,

  .pin_d7 = Y9_GPIO_NUM,
  .pin_d6 = Y8_GPIO_NUM,
  .pin_d5 = Y7_GPIO_NUM,
  .pin_d4 = Y6_GPIO_NUM,
  .pin_d3 = Y5_GPIO_NUM,
  .pin_d2 = Y4_GPIO_NUM,
  .pin_d1 = Y3_GPIO_NUM,
  .pin_d0 = Y2_GPIO_NUM,
  .pin_vsync = VSYNC_GPIO_NUM,
  .pin_href = HREF_GPIO_NUM,
  .pin_pclk = PCLK_GPIO_NUM,

  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,

  .pixel_format = PIXFORMAT_JPEG,
  .frame_size = FRAMESIZE_QVGA,
  .jpeg_quality = 12,
  .fb_count = 1,
  .fb_location = CAMERA_FB_IN_PSRAM,
  .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

void setup() {
  Serial.begin(115200);
  I2Cbus.begin(I2C_SDA, I2C_SCL, 100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED init failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  while (!Serial);
  Serial.println("15-Second Object Detection");

  if (!ei_camera_init()) {
    Serial.println("Camera init failed");
    display.print("Camera init failed!");
    display.display();
    while (true);
  }

  display.setCursor(0, 0);
  display.print("Starting detection...");
  display.display();
  delay(2000);
  
  processing_start = millis();
  processing_active = true;
  unique_objects.clear();
  display.clearDisplay();
}

void loop() {
  if (processing_active) {
    process_frame();
    update_display();
    
    if (millis() - processing_start >= DETECTION_DURATION) {
      processing_active = false;
      show_final_results();
    }
  }
}

void process_frame() {
  snapshot_buf = (uint8_t *)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * 
                                 EI_CAMERA_RAW_FRAME_BUFFER_ROWS * 
                                 EI_CAMERA_FRAME_BYTE_SIZE);
  if (!snapshot_buf) return;

  if (!ei_camera_capture(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf)) {
    free(snapshot_buf);
    return;
  }

  ei::signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
  signal.get_data = &ei_camera_get_data;

  ei_impulse_result_t result = {0};
  EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
  free(snapshot_buf);

  if (err != EI_IMPULSE_OK) return;

  // Process detection results
  for (size_t ix = 0; ix < result.bounding_boxes_count; ix++) {
    auto bb = result.bounding_boxes[ix];
    if (bb.value < 0.5) continue;

    String label = String(bb.label);
    bool exists = false;
    
    for (auto& obj : unique_objects) {
      if (obj == label) {
        exists = true;
        break;
      }
    }
    
    if (!exists) {
      unique_objects.push_back(label);
    }
  }
  frame_count++;
}

void update_display() {
  display.clearDisplay();
  
  // Time remaining
  int remaining = (DETECTION_DURATION - (millis() - processing_start)) / 1000;
  remaining = remaining > 0 ? remaining : 0;
  
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("Processing: %02ds", remaining);
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  // Detection info
  display.setTextSize(2);
  display.setCursor(0, 15);
  display.printf("Objects: %d", unique_objects.size());
  
  // Frame counter
  display.setTextSize(1);
  display.setCursor(0, 55);
  display.printf("Frames: %d", frame_count);

  display.display();
}

void show_final_results() {
  ei_camera_deinit();
  float total_time = unique_objects.size() * TIME_PER_OBJECT;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Final Results");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.printf("Total: %d", unique_objects.size());
  
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.printf("Processing Time: %.1fs", total_time);
  
  display.setCursor(0, 55);
  display.printf("Frames Processed: %d", frame_count);

  display.display();
}

bool ei_camera_init(void) {
  if (is_initialised) return true;

  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, 0);
  }

  is_initialised = true;
  return true;
}

bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
  if (!is_initialised) return false;

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return false;

  bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, out_buf);
  esp_camera_fb_return(fb);

  if (!converted) return false;

  if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS) || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {
    ei::image::processing::crop_and_interpolate_rgb888(
      out_buf,
      EI_CAMERA_RAW_FRAME_BUFFER_COLS,
      EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
      out_buf,
      img_width,
      img_height);
  }

  return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
  size_t pixel_ix = offset * 3;
  size_t pixels_left = length;
  size_t out_ptr_ix = 0;

  while (pixels_left != 0) {
    out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix];
    out_ptr_ix++;
    pixel_ix += 3;
    pixels_left--;
  }
  return 0;
}

void ei_camera_deinit(void) {
  if (is_initialised) {
    esp_camera_deinit();
    is_initialised = false;
  }
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif