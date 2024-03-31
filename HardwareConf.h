// NOTE: Pins are for Waveshare RP2040-Zero
// RP2040 GPIO number is same as the Arduino PIN number
// We also have a WS2812B on PIN16
/*        
				Pin#              Pin#
            ___(_____)___
      5v 1 |   *USB C*   | 23 GPIO0
     GND 2 |             | 22 GPIO1
    3.3v 3 |             | 21 GPIO2
  GPIO29 4 |   RP2040    | 20 GPIO3
  GPIO28 5 |    Zero     | 19 GPIO4
  GPIO27 6 |             | 18 GPIO5
  GPIO26 7 |             | 17 GPIO6
  GPIO15 8 |             | 16 GPIO7
  GPIO14 9 |__|_|_|_|_|__| 15 GPIO8
              1 1 1 1 1
              0 1 2 3 4

            Pin10 = GPIO13
            Pin11 = GPIO12
            Pin12 = GPIO11
            Pin13 = GPIO10
            Pin14 = GPIO9
*/

// Button inputs
#define PIN_LIGHTS    0
#define PIN_GEAR      1
#define PIN_RCS       2
#define PIN_SAS       3

// LED outputs
#define LED_CONNECTED 4 // Top led panel of all buttons. Used to indicate connection
#define LED_LIGHTS    5
#define LED_GEAR      6
#define LED_RCS       7
#define LED_SAS       8

