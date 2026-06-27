#ifndef HID_APP_H_
#define HID_APP_H_
#include <hid.h>
#include <tusb.h>

typedef enum
{
  JOYSTICK_BUTTON_X  = (1 << 0),
  JOYSTICK_BUTTON_A  = (1 << 1),
  JOYSTICK_BUTTON_B  = (1 << 2),
  JOYSTICK_BUTTON_Y  = (1 << 3),
} hid_joystick_button_bm_t;

typedef enum
{
  X_LEFT   = 0,
  X_CENTER = 511,
  X_RIGHT  = 1023,
} hid_joystick_X_t;

typedef enum
{
  Y_UP     = 0,
  Y_CENTER = 511,
  Y_DOWN   = 1023,
} hid_joystick_Y_t;

typedef struct TU_ATTR_PACKED
{
    uint32_t Buttons;   
    uint8_t  Hat_switch : 4;   
    uint16_t X : 10; //0-1023
    uint16_t Y : 10; 
    uint16_t Rx : 10; 
    uint16_t Ry : 10; 
    uint16_t Slider_L : 10; 
    uint16_t Slider_R : 10; 
} hid_joystick_report_t;

typedef union {
    uint8_t byte;
    struct
      {
        uint8_t button_x : 1;
        uint8_t button_a : 1; 
        uint8_t button_b : 1; 
        uint8_t button_y : 1; 
        uint8_t x_left   : 1; 
        uint8_t x_right  : 1; 
        uint8_t y_up     : 1; 
        uint8_t y_down   : 1; 
      } bit;
} joystick_out_t;

#endif /* HID_APP_H_ */