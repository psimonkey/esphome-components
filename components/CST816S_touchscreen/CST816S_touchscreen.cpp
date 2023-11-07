#include "esphome/core/log.h"
#include "CST816S_touchscreen.h"
#include "CST816S.h"
#define I2C_SDA 4
#define I2C_SCL 5
#define TP_RST 1
#define TP_INT 0

namespace esphome {
namespace cst816s_touchscreen {

unsigned long previousMillis = 0UL;
unsigned long interval = 100UL;

uint8_t rgbColorWheelValues[3] = {0, 0, 0};

static const char *TAG = "cst816s_touchscreen.text_sensor";

CST816S touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT);	// sda, scl, rst, irq

void CST816STouchScreen::setup() {
    ESP_LOGI("touchscreen", "Starting touchscreen");
    touch.begin();
}

void CST816STouchScreen::dump_config() { 
    ESP_LOGCONFIG(TAG, "CST816STouchScreen");
}

uint8_t CST816STouchScreen::getX() { 
    return touch.data.x;
}

uint8_t CST816STouchScreen::getY() { 
    return touch.data.y;
}

uint8_t CST816STouchScreen::getEvent() { 
    return touch.data.event;
}

void CST816STouchScreen::updateColorWheelValues(uint8_t x, uint8_t y)
{
    // Code is derived from the example at: https://www.cnx-software.com/2023/10/22/review-matouch-esp32-s3-rotary-ips-display-2-1-inch-round-touchscreen-display/

    // Shift origin to center of the screen
    int dx = x - 120;
    int dy = 120 - y;

    // angle
    float angle = atan2(dy, dx);
    
    // distance
    float dist = sqrt((dx*dx) + (dy*dy));
    float v = dist/120.0;
    
    // Hue
    float t = angle;
    float t_step = 120.0 * DEG_TO_RAD;
    float r = 255 * ((cos(t         )/2.0) + 0.5);
    float g = 255 * ((cos(t - t_step)/2.0) + 0.5);
    float b = 255 * ((cos(t + t_step)/2.0) + 0.5);
    
    // Adjust brightness        // Let's not adjust the brightness
    // r = v * r;
    // g = v * g;
    // b = v * b;
    
    // Limit the min/max
    if(r > 255.0) { r = 255.0; }
    if(g > 255.0) { g = 255.0; }
    if(b > 255.0) { b = 255.0; }
    if(r <   0.0) { r =   0.0; }
    if(g <   0.0) { g =   0.0; }
    if(b <   0.0) { b =   0.0; }
    
    // Update the values
    rgbColorWheelValues[0] = (uint8_t)r; 
    rgbColorWheelValues[1] = (uint8_t)g;
    rgbColorWheelValues[2] = (uint8_t)b;

    // ESP_LOGI("touchscreen", "%d, %d , %d, %d, %f, %f", touch.data.x, touch.data.y, dx, dy, angle, angle*RAD_TO_DEG);

}

void CST816STouchScreen::loop() {
    unsigned long currentMillis = millis();

    if (touch.available()) {
        if(touch.data.event != 2){                          //ignore when the event is 2 because it can send too much data - Event (0 = Down, 1 = Up, 2 = Contact)
            char buf[20];
            sprintf(buf, "%s", touch.gesture());
            // ESP_LOGI("touchscreen", "Gesture: %s, %d, %d , %d", touch.gesture(), touch.data.x, touch.data.y, touch.data.event);
            this->publish_state(buf);
        }
    }
}

}  // namespace cst816s_touchscreen
}  // namespace esphome