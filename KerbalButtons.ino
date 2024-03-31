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
char pBinFill_Buffer[17];

// Approx freq that process input
#define CONTROLREFRESH 100
uint32_t controlTime, controlTimeOld;

uint8_t all_leds[] = {
	LED_ALLTOP,
	LED_LIGHTS,
	LED_GEAR,
	LED_RCS,
	LED_SAS,
	255
};

// This is here for debug
VesselData_t VData;

char sprintbuff[100];

// Declare prototypes with default value (seems to be the way in Arduino)
void LEDSAllOn(uint8_t delaytime = 20);
void LEDSAllOff(uint8_t delaytime = 20);


void setup(){
	// USB serial
	// Serial.begin(38400);
	// DEV: Using Serial 2 to communicate with KSP

	FastLED.addLeds<NEOPIXEL, FASTLED_DATA_PIN>(ws2812_led, FASTLED_NUM_LEDS);

	// HW Serial 0 on pin 12 for debug output
	Serial1.setTX(12);
	Serial1.begin(115200);
	Serial1.println("Boot");

	// Serial 2 is used to talk to KSP
	Serial2.setTX(4);
	Serial2.setRX(5);
	Serial2.begin(38400);
	
	InitTxPackets();

	pinMode(PIN_LIGHTS, INPUT_PULLUP);
	pinMode(PIN_GEAR,   INPUT_PULLUP);
	pinMode(PIN_RCS,    INPUT_PULLUP);
	pinMode(PIN_SAS,    INPUT_PULLUP);
	
	pinMode(LED_ALLTOP,   OUTPUT);
	pinMode(LED_LIGHTS,   OUTPUT);
	pinMode(LED_GEAR,     OUTPUT);
	pinMode(LED_RCS,      OUTPUT);
	pinMode(LED_SAS,      OUTPUT);
	pinMode(PIN_LOOP_TICK,    OUTPUT);
	pinMode(PIN_CONTROL_TICK, OUTPUT);
	pinMode(PIN_RECEIVE_TICK, OUTPUT);

	Serial1.println("LED");
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

boolean donesend = false;
boolean sendit = false;
uint16_t old_VData_ActionGroups = 0;
uint16_t old_VData_NavballSASMode = 0;

void loop(){
	
	// TODO: Make a better job of running this at a specific rate
	digitalWrite(PIN_LOOP_TICK, !digitalRead(PIN_LOOP_TICK));

	ClearState();
	GatherInput();


	switch(KSPCheckForUpdate()){
		case -1:
			// No new data
			break;
		case 0:
			// Handshake
			break;
		case 1:
			// New data
			digitalWrite(PIN_RECEIVE_TICK, !digitalRead(PIN_RECEIVE_TICK));
			// Update LEDs
			UpdateIndicators();
			UpdateMainControlsState();
			if(
				(VData.ActionGroups != old_VData_ActionGroups)
				|| (VData.NavballSASMode != old_VData_NavballSASMode)
			){
				DebugVesselData();
				old_VData_ActionGroups = VData.ActionGroups;
				old_VData_NavballSASMode = VData.NavballSASMode;
			}
			// Flag send it as false now we have received some data
			sendit = false;
			if(donesend){
				DebugMainControls();
				Serial1.print(" <-- As received");
				donesend = false;
			}
			break;
	}


	uint32_t now = millis();
	controlTime = now - controlTimeOld;
	if(controlTime > CONTROLREFRESH){
		
		digitalWrite(PIN_CONTROL_TICK, !digitalRead(PIN_CONTROL_TICK));

		InputState_t state = processInputs();

		// DebugVesselData();
		// DebugMainControls();
		// Only send data if input has changed
		if(state.pressed.allbits){
			DebugButtonStates();

			// Populate the Control Packet with out new values
			WriteControlData(state);
			DebugMainControls();
			Serial1.print(" --> To send");

			// Flag that we want to send a packet
			sendit = true;

		}
	
		if(sendit){
			// Send it as many times as we can until we recieve new data
			donesend = true;
			KSPSendControlData();
		}

		controlTimeOld = now;
	}
	delay(3);
	UpdateStatusLED();
}