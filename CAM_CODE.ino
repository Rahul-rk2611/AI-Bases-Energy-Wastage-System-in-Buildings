#include <WiFi.h>
#include <HTTPClient.h>

#include <ai_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"

#include "esp_camera.h"
#include "esp_http_server.h"
#include "img_converters.h"

// ==========================
// WIFI DETAILS
// ==========================

const char* ssid = "OP12";
const char* password = "12345678900";

// ESP32 SERVER IP
String serverName = "http://10.42.156.196";

// ==========================
// CAMERA MODEL
// ==========================

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#endif

// ==========================
// CAMERA SETTINGS
// ==========================

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3

static bool debug_nn = false;
static bool is_initialised = false;
uint8_t *snapshot_buf;

// ==========================
// CAMERA CONFIG
// ==========================

static camera_config_t camera_config = {
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

// ==========================
// FUNCTION DECLARATIONS
// ==========================

bool ei_camera_init(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr);

void startCameraServer();

// ==========================
// CAMERA STREAM FUNCTION
// ==========================

static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;

    res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");

    if(res != ESP_OK)
    {
        return res;
    }

    while(true)
    {
        fb = esp_camera_fb_get();

        if (!fb)
        {
            continue;
        }

        char part_buf[64];

        size_t hlen = snprintf(
            part_buf,
            64,
            "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
            fb->len
        );

        res = httpd_resp_send_chunk(req, part_buf, hlen);

        if(res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        }

        if(res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, "\r\n", 2);
        }

        esp_camera_fb_return(fb);

        if(res != ESP_OK)
        {
            break;
        }
    }

    return res;
}

// ==========================
// START CAMERA SERVER
// ==========================

void startCameraServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL
    };

    httpd_handle_t stream_httpd = NULL;

    if (httpd_start(&stream_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}

// ==========================
// SETUP
// ==========================

void setup()
{
    Serial.begin(115200);

    // CONNECT WIFI
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi Connected!");
    Serial.print("ESP32-CAM IP: ");
    Serial.println(WiFi.localIP());

    // CAMERA INIT
    if (ei_camera_init() == false)
    {
        Serial.println("Camera init failed");
    }
    else
    {
        Serial.println("Camera initialized");
    }

    // START STREAM SERVER
    startCameraServer();

    Serial.println("Camera Stream Started");

    Serial.println("Open Stream:");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/stream");

    delay(2000);
}

// ==========================
// LOOP
// ==========================

void loop()
{
    snapshot_buf = (uint8_t*)malloc(
        EI_CAMERA_RAW_FRAME_BUFFER_COLS *
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS *
        EI_CAMERA_FRAME_BYTE_SIZE
    );

    if(snapshot_buf == nullptr)
    {
        Serial.println("Snapshot buffer failed");
        return;
    }

    ei::signal_t signal;

    signal.total_length =
        EI_CLASSIFIER_INPUT_WIDTH *
        EI_CLASSIFIER_INPUT_HEIGHT;

    signal.get_data = &ei_camera_get_data;

    if (ei_camera_capture(
            EI_CLASSIFIER_INPUT_WIDTH,
            EI_CLASSIFIER_INPUT_HEIGHT,
            snapshot_buf) == false)
    {
        Serial.println("Capture failed");
        free(snapshot_buf);
        return;
    }

    // ==========================
    // RUN AI MODEL
    // ==========================

    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR err =
        run_classifier(&signal, &result, debug_nn);

    if (err != EI_IMPULSE_OK)
    {
        Serial.println("Inference failed");
        free(snapshot_buf);
        return;
    }

    // ==========================
    // PRINT RESULTS
    // ==========================

    Serial.println("Predictions:");

    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
    {
        Serial.print(ei_classifier_inferencing_categories[i]);
        Serial.print(": ");
        Serial.println(result.classification[i].value);
    }

    // ==========================
    // HUMAN DETECTION
    // ==========================

    float humanScore = result.classification[0].value;

    if (humanScore > 0.50)
    {
        Serial.println("HUMAN DETECTED");

        HTTPClient http;

        http.begin(serverName + "/human");

        int httpResponseCode = http.GET();

        Serial.print("HTTP Response: ");
        Serial.println(httpResponseCode);

        http.end();
    }
    else
    {
        Serial.println("NO HUMAN");

        HTTPClient http;

        http.begin(serverName + "/nohuman");

        int httpResponseCode = http.GET();

        Serial.print("HTTP Response: ");
        Serial.println(httpResponseCode);

        http.end();
    }

    free(snapshot_buf);

    delay(2000);
}

// ==========================
// CAMERA INIT
// ==========================

bool ei_camera_init(void)
{
    if (is_initialised) return true;

    esp_err_t err = esp_camera_init(&camera_config);

    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    is_initialised = true;

    return true;
}

// ==========================
// CAMERA CAPTURE
// ==========================

bool ei_camera_capture(
    uint32_t img_width,
    uint32_t img_height,
    uint8_t *out_buf)
{
    if (!is_initialised)
    {
        Serial.println("Camera not initialized");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        Serial.println("Camera capture failed");
        return false;
    }

    bool converted = fmt2rgb888(
        fb->buf,
        fb->len,
        PIXFORMAT_JPEG,
        snapshot_buf
    );

    esp_camera_fb_return(fb);

    if(!converted)
    {
        Serial.println("Conversion failed");
        return false;
    }

    if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS) ||
        (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS))
    {
        ei::image::processing::crop_and_interpolate_rgb888(
            out_buf,
            EI_CAMERA_RAW_FRAME_BUFFER_COLS,
            EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
            out_buf,
            img_width,
            img_height
        );
    }

    return true;
}

// ==========================
// GET IMAGE DATA
// ==========================

static int ei_camera_get_data(
    size_t offset,
    size_t length,
    float *out_ptr)
{
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0)
    {
        out_ptr[out_ptr_ix] =
            (snapshot_buf[pixel_ix + 2] << 16) +
            (snapshot_buf[pixel_ix + 1] << 8) +
            snapshot_buf[pixel_ix];

        out_ptr_ix++;
        pixel_ix += 3;
        pixels_left--;
    }

    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif