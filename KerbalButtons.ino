/**
 * Kerbal Control Panel
 * 4 Illuminated buttons
 * Connects to Kerbal Space Program 1 via KSPSerialIO
 * https://github.com/zitron-git/KSPSerialIO
 **/

#include <FastLED.h>
#include "TypeDefs.h"
#include "HardwareConf.h"
#include "KSPSerial.h"

#define FASTLED_NUM_LEDS 1
#define FASTLED_DATA_PIN 16
CRGB ws2812_led[FASTLED_NUM_LEDS];


// Bits to dispaly and buffer to store bit output
#define kDisplayWidth 16
char pBinFill_Buffer[kDisplayWidth+1];



unsigned long deadtime, deadtimeOld, controlTime, controlTimeOld, now;

uint8_t all_leds[] = {
	LED_CONNECTED,
	LED_LIGHTS,
	LED_GEAR,
	LED_RCS,
	LED_SAS,
	255
};

// KSPSerialIOState_t IOState = {false, false, false};

char sprintbuff[100];
// uint8_t ctr = 0;
String txtMsg = ""; 

// Declare prototypes with default value (seems to be the way in Arduino)
void LEDSAllOn(uint8_t delaytime = 20);
void LEDSAllOff(uint8_t delaytime = 20);

HandShakePacket_t HPacket;
VesselData_t VData;
ControlPacket_t CPacket;



#define INPUT_DEBOUNCE_BUFLEN 8 // How many input samples we use to debounce
uint8_t inputstep = 0; // Current debounce buffer index

InputState_t _state;  // Computed state
Input_t _now;
Input_t _then;
// Array to store input buffer samples for debouncing
Input_t g_inputbuffer[INPUT_DEBOUNCE_BUFLEN];

void setup(){
  // USB serial
	Serial.begin(38400);

	FastLED.addLeds<NEOPIXEL, FASTLED_DATA_PIN>(ws2812_led, FASTLED_NUM_LEDS);

	// HW Serial 0 on pin 12 for debug output
	Serial1.setTX(12);
	Serial1.begin(38400);

	Serial1.println("Boot");
	Serial1.print("LED");
	
	InitTxPackets();

	pinMode(PIN_LIGHTS, INPUT_PULLUP);
	pinMode(PIN_GEAR,   INPUT_PULLUP);
	pinMode(PIN_RCS,    INPUT_PULLUP);
	pinMode(PIN_SAS,    INPUT_PULLUP);
	
	pinMode(LED_CONNECTED, OUTPUT);
	pinMode(LED_LIGHTS,    OUTPUT);
	pinMode(LED_GEAR,      OUTPUT);
	pinMode(LED_RCS,       OUTPUT);
	pinMode(LED_SAS,       OUTPUT);

	Serial1.println("OK");

	uint8_t idx = 6;
	while(idx-- > 0){
		LEDSAllOn(25);
		delay(50);
		LEDSAllOff(25);
		delay(50);
		Serial1.print(idx);
		Serial1.print(" ");
	}

	// LEDSAllOff();
	Serial1.println("Run");
}

void loop(){
	
	ClearState();
	

	switch(KSPCheckForUpdate()){
		case -1:
			// Nothing
			// Serial1.println("\nNot connected!");
			break;
		case 0:
			// Handshake
			Serial1.println("\nSent Handshake");
			break;
		case 1:
			// New data
			// Serial1.println("\nReceived new data");

			// Update LEDs
			UpdateIndicators();
			break;
	}

	GatherInput();

	now = millis();
	controlTime = now - controlTimeOld;
	if(controlTime > CONTROLREFRESH){
		
		Serial1.print(".");

		InputState_t state = processInputs();

		if(state.pressed.complete){
			sprintf(sprintbuff,   "\nPressed %s", pBinFill(state.pressed.complete, pBinFill_Buffer, '_'));
			Serial1.print(sprintbuff);

			// Only send data if something has changed
			//sprintf(sprintbuff,   "\nM %s", pBinFill(CPacket.MainControls, pBinFill_Buffer, '_'));
			//Serial1.print(sprintbuff);
			
			// Populate the Control Packet with out new values
			WriteControlData(state);
			// Send the Packet
			KSPSendControlData();
			//KSPBoardSendData(details(CPacket));

			//sprintf(sprintbuff,   "\nM %s", pBinFill(CPacket.MainControls, pBinFill_Buffer, '_'));
			// Serial1.print(sprintbuff);

		}
	
		controlTimeOld = now;
	}

	UpdateStatusLED();

	// No point running the loop faster than the game framerate
	delay(10);
}