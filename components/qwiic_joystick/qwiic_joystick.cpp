#include "math.h"

#include "qwiic_joystick.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace qwiic_joystick {

static const char *const TAG = "qwiic_joystick";


void QwiicJoystick::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Qwiic Joystick...");
  
  uint8_t version[2];
  
  if( this->read_bytes(0x01, version, 2) )
    ESP_LOGCONFIG(TAG, "- Version %d.%d", version[0], version[1]);
  else
    ESP_LOGCONFIG(TAG, "- Error: could not read version number!");

  uint8_t buf[4];
  this->read_bytes(0x03, buf, 4);
  
  uint16_t x = ((buf[0] << 8) | buf[1]) >> 6;
  uint16_t y = ((buf[2] << 8) | buf[3]) >> 6;

  this->old_x_ = x;
  this->old_y_ = y;

  this->center_x_ = x;
  this->center_y_ = y;

  ESP_LOGCONFIG(TAG, "- Center point (%d, %d).", x, y);
}

void QwiicJoystick::update() {
  ESP_LOGCONFIG(TAG, "Updating Qwiic Joystick...");

  uint8_t buf[6];
  this->read_bytes(0x03, buf, 6);
  
  uint16_t x = ((buf[0] << 8) | buf[1]) >> 6;
  uint16_t y = ((buf[2] << 8) | buf[3]) >> 6;
  
  float x_f = x;
  float y_f = y;
  
  float x_c = x - this->center_x_;
  float y_c = y - this->center_y_;
  
  if( this->button_sensor_ ) {
    if( buf[5] )
        this->button_sensor_->publish_state(true);
    if( buf[5] || buf[4] != this->old_button_pressed_ )
        this->button_sensor_->publish_state(! static_cast<bool>(buf[4]));
    this->write_byte(0x08, 0x00);
  }  
  
  for(uint8_t i = 0; i < 6; i++) {
    ESP_LOGCONFIG(TAG, "buf[%d] is 0x%X (%d)", i, buf[i], buf[i]);
  }
  ESP_LOGCONFIG(TAG, "x is 0x%X (%d)", x, x);
  ESP_LOGCONFIG(TAG, "y is 0x%X (%d)", y, y);
  ESP_LOGCONFIG(TAG, "x_f is %d", x_f);
  ESP_LOGCONFIG(TAG, "y_f is %d", y_f);
  ESP_LOGCONFIG(TAG, "x_c is %d", x_c);
  ESP_LOGCONFIG(TAG, "y_c is %d", y_c);
  
  this->old_button_pressed_ = buf[4];

  if( x == this->old_x_ && y == this->old_y_ )
    return;

  if( this->x_axis_sensor_ != nullptr)
    this->x_axis_sensor_->publish_state(x_f);
  
  if( this->x_axis_centered_sensor_ != nullptr)
    this->x_axis_sensor_->publish_state(x_c);
  
  
  if( this->y_axis_sensor_ != nullptr)
    this->y_axis_sensor_->publish_state(y_f);
  
  if( this->y_axis_centered_sensor_ != nullptr)
    this->y_axis_sensor_->publish_state(y_c);
  
  /*
  if( this->radius_squared_sensor_ != nullptr) {
    this->radius_squared_sensor_->publish_state(x_f*x_f+y_f*y_f);
  }
  
  if( this->theta_sensor_ != nullptr)
    this->theta_sensor_->publish_state(atan2(y_c, x_c)*180.0/3.14159);
  */
  this->old_x_ = x;
  this->old_y_ = y;
}

float QwiicJoystick::get_setup_priority() const {
  return setup_priority::IO;
}



} // namespace qwiic_joystick
} // namespace esphome

