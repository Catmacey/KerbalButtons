/**
 * Very simple Kerbal Space Program Control Panel
 * 4 Illuminated buttons
 * Author: Matt Casey matt@catmacey.com
 * https://github.com/Catmacey/KerbalButtons
 * Connects to KSPSerialIO https://github.com/zitron-git/KSPSerialIO
 **/

// Comment this out for live build
// #define DEBUGBUILD
// Comment this out to not bother sending full debug data
// #define DEBUGLEVEL_FULL

#include <FastLED.h>
#include "TypeDefs.h"
#include "HardwareConf.h"
#include "KSPSerial.h"

#define FASTLED_NUM_LEDS 1
#define FASTLED_DATA_PIN 16
CRGB ws2812_led[FASTLED_NUM_LEDS];

/**
 * Setup of which serial port we use to talk to KSPSerialIO
 * Either USBSerial in live mode or Serial2 (HW Serial) in dev mode
 **/
void setup_ksp_serial_port(){
	#ifdef DEBUGBUILD
		// Dev build port setup
		// Use one of the HW serial ports
		#define SERIALKSP Serial2
		// Pins need configuring
		SERIALKSP.setTX(PIN_SERIAL2_TX);
		SERIALKSP.setRX(PIN_SERIAL2_RX);
	#else
		// Live build
		// Use the USB Serial
		#define SERIALKSP Serial
	#endif
	// KSPSerialIO expects this baud rate
	SERIALKSP.begin(38400);
}

// Buffer to store bit output (big enough to store your largest value + 1)
char pBinFill_Buffer[17];

// Approx ms between each control process
#define CONTROLREFRESHMS 75
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
// VesselData_t VData;

char sprintbuff[100];

// Declare prototypes with default value (seems to be the way in Arduino)
void LEDSAllOn(uint8_t delaytime = 20);
void LEDSAllOff(uint8_t delaytime = 20);

void setup(){

	FastLED.addLeds<NEOPIXEL, FASTLED_DATA_PIN>(ws2812_led, FASTLED_NUM_LEDS);

	// Debug output always on Serial1 (HW Serial 0)
	Serial1.setTX(PIN_SERIAL1_TX);
	Serial1.begin(115200);

	setup_ksp_serial_port();

	Serial1.println("Boot");
	#ifdef DEBUGBUILD
	Serial1.println("Debug Build : KSP on Serial2");
	#else
	Serial1.println("Live Build : KSP on USBSerial");
	#endif

	
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

boolean debug_control_data = false;
boolean data_to_send = false;
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
			// Toggle a debug pin
			digitalWrite(PIN_RECEIVE_TICK, !digitalRead(PIN_RECEIVE_TICK));
			// Update LEDs
			UpdateIndicators();
			UpdateCPacketDataFromVessel();
			if(
				(VData.ActionGroups != old_VData_ActionGroups)
				|| (VData.NavballSASMode != old_VData_NavballSASMode)
			){
				// Log changes to button states
				DebugVesselData();
				old_VData_ActionGroups = VData.ActionGroups;
				old_VData_NavballSASMode = VData.NavballSASMode;
			}
			// Flag send it as false now we have received some data
			data_to_send = false;
			if(debug_control_data){
				DebugControlData(false);
				// Don't debug again until needed
				debug_control_data = false;
			}
			break;
	}


	uint32_t now = millis();
	controlTime = now - controlTimeOld;
	if(controlTime > CONTROLREFRESHMS){
		
		digitalWrite(PIN_CONTROL_TICK, !digitalRead(PIN_CONTROL_TICK));

		InputState_t state = processInputs();

		// Only update control data if input has changed
		if(state.pressed.allbits){

			// Populate the Control Packet with out new values
			WriteControlData(state);
			DebugControlData(true);

			// Flag that we want to send a packet
			data_to_send = true;

		}
	
		// If we have more contiously changing data such as throttle we might want to send every time
		if(data_to_send){
			// Send it as many times as we can until we recieve new data
			// Indicate that we want to debug next time we receive data
			debug_control_data = true;
			KSPSendControlData();
		}

		controlTimeOld = now;
	}
	delay(3);
	UpdateStatusLED();
}