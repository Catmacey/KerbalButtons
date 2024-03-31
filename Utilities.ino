
/**
 * Update our indicator LEDs based on new Vessel data
 **/
void UpdateIndicators(){
	// Illuminate the top section of all the buttons
	digitalWrite(LED_ALLTOP, HIGH);
	// Illuminate the relevant button
	digitalWrite(LED_GEAR,   KSPGetControlState(AGGear));
	digitalWrite(LED_RCS,    KSPGetControlState(AGRCS));
	digitalWrite(LED_LIGHTS, KSPGetControlState(AGLight));
	digitalWrite(LED_SAS,    KSPGetControlState(AGSAS));
}


/**
 * Update the CPacket.MainControls data to make the incomming Vessel Data
 * Call this every time you receive new data
 **/
void UpdateMainControlsState(){
	KSPSetMainControls(RCS, KSPGetControlState(AGRCS));
	KSPSetMainControls(GEAR, KSPGetControlState(AGGear));
	KSPSetMainControls(LIGHTS, KSPGetControlState(AGLight));
	KSPSetMainControls(SAS, KSPGetControlState(SAS));
}


/**
 * Update the ControlData with change to state
 * Our buttons are all momentary action. So we
 * need to toggle the current state of whatever
 * button was pressed.
 **/
void WriteControlData(InputState_t state){
	if(state.pressed.rcs){
		KSPToggleMainControls(RCS);
	}
	if(state.pressed.gear){
		KSPToggleMainControls(GEAR);
	}
	if(state.pressed.lights){
		KSPToggleMainControls(LIGHTS);
	}
	if(state.pressed.sas){
		
		// KSPToggleMainControls(SAS);
		// We need to maintain the SAS state
		uint8_t SASMode = KSPGetSASMode();
		if(SASMode > 0){
			Serial1.print("\nClr SAS: Current SASMode:");
			Serial1.print(SASMode);
			KSPSetSASMode(SMOFF); //setting SAS mode
			KSPSetMainControls(SAS, false);
		}else{
			Serial1.print("\nClr SAS: Current SASMode:");
			Serial1.print(SASMode);
			// If we are enabling SAS we need to set the SAS mode to default
			KSPSetSASMode(SMSAS); //setting SAS mode
			KSPSetMainControls(SAS, true);
		}
	}
}


/**
 * Turn off all the LEDs
 * @delaytime optional ms delay between LEDs
 **/
void LEDSAllOn(uint8_t delaytime){
	uint8_t idx = 0;
	while(all_leds[idx] < 255){
		digitalWrite(all_leds[idx++], HIGH);
		if(delaytime){
			delay(delaytime);
		}
	}
}


/**
 * Turn on all the LEDs
 * @delaytime optional ms delay between LEDs
 **/
void LEDSAllOff(uint8_t delaytime){
	uint8_t idx = 0;
	while(all_leds[idx] < 255){
		digitalWrite(all_leds[idx++], LOW);
		if(delaytime){
			delay(delaytime);
		}
	}
}



/**
 * Fills a buffer with a text representation of a binary number
 * @x the value to render
 * @so the buffer to fill with the output
 * @fillChar the character to use to right-fill the output
 * eg. number 5 will render as "101"
 * NOTE:
 * You need to declare a buffer to store the output
 * eg. char pBinFill_Buffer[kDisplayWidth+1];
 */
char* pBinFill(long int x, char *so, char fillChar, uint8_t len){
	// fill in array from right to left
	char s[len+1];
	int i = len;
	s[i--] = 0x00;  // terminate string
	do{
		// fill in array from right to left
		s[i--] = (x & 1) ? '1' : fillChar;
		x >>= 1;  // shift right 1 bit
	} while (x > 0);
	while (i >= 0) s[i--] = fillChar;  // fill with fillChar 
	sprintf(so, "%s", s);
	return so;
}


/**
 * Use the built in WS2812B to show Connected state
 * plus RX and TX activity
 **/
void UpdateStatusLED(){
	ws2812_led[0] = CRGB(
		(IOState.Connected?0:0),
		(IOState.DataSent?50:0),
		(IOState.DataReceived?50:0)
	);

	FastLED.show();

}


#define INPUT_DEBOUNCE_BUFLEN 8 // How many input samples we use to debounce
uint8_t inputstep = 0; // Current debounce buffer index

InputState_t _state;  // Computed state
Input_t _now;
Input_t _then;
// Array to store input buffer samples for debouncing
Input_t g_inputbuffer[INPUT_DEBOUNCE_BUFLEN];


/**
 * Gather input for later debounce
 * Run at around 200+Hz
 **/
void GatherInput(){
	// Fill up the buffer with input events
	
	// MAYB: Read all these pins in one go somehow
	g_inputbuffer[inputstep].sas    = !digitalRead(PIN_SAS);
	g_inputbuffer[inputstep].rcs    = !digitalRead(PIN_RCS);
	g_inputbuffer[inputstep].gear   = !digitalRead(PIN_GEAR);
	g_inputbuffer[inputstep].lights = !digitalRead(PIN_LIGHTS);

	inputstep++;
	if(inputstep == INPUT_DEBOUNCE_BUFLEN) inputstep = 0;
}


/**
 * Read input buttons and decide if things have changed
 * return 1 to indicate change (data worth sending)
 * Run this at around 20Hz
 **/
InputState_t processInputs(){
	uint8_t idx = 0;

	// Store the last input run in _then for comparison
	_then.allbits = _now.allbits;
	
	// Clear current states
	_state.pressed.allbits = 0;
	// _state.repeat.allbits = 0;
	_state.released.allbits = 0;
	
	// Default the _now input to pressed
	_now.allbits = 0xff;
	// AND the input buffer into _now to debounce button presses
	// We end up with a struct containing the debounced state of the buttons
	for(idx=0; idx < INPUT_DEBOUNCE_BUFLEN; idx++){
		_now.allbits = (_now.allbits & g_inputbuffer[idx].allbits);
	}

	// Calculate the changed and released states using our previous data
	_state.changed.allbits = (_then.allbits ^ _now.allbits);
	_state.released.allbits = (_then.allbits & _state.changed.allbits);

	// Pressed is true if the button wasn't pressed before
	_state.pressed.allbits = (~_then.allbits & _now.allbits);

	// Pressed is the current state
	_state.held.allbits = _now.allbits;

	return _state;
}


/**
 * Prints out the state of CPacket.MainControls
 **/
void DebugMainControls(){
	sprintf(
		sprintbuff,
		"\nCPacket.MainControls %s",
		pBinFill(CPacket.MainControls, pBinFill_Buffer, '\xB7', 8)
	);
	Serial1.print(sprintbuff);
}


/**
 * Prints out the state of VesselData controls
 **/
void DebugVesselData(){
	sprintf(
		sprintbuff,
		"\nVesselData: SAS(%d):%d RCS:%d GEAR:%d LUZ:%d BRK:%d",
		KSPGetSASMode(),
		KSPGetControlState(AGSAS),
		KSPGetControlState(AGRCS),
		KSPGetControlState(AGGear),
		KSPGetControlState(AGLight),
		KSPGetControlState(AGBrakes)
	);
	Serial1.print(sprintbuff);
}


/**
 * Prints out the current buttons states
 **/
void DebugButtonStates(){
	sprintf(sprintbuff, 
		"\nBtn Pressed:%s",
		pBinFill(_state.pressed.allbits,  pBinFill_Buffer, '\xB7', 4)
	);
	Serial1.print(sprintbuff);
	
	sprintf(sprintbuff, 
		" Released:%s",
		pBinFill(_state.released.allbits, pBinFill_Buffer, '\xB7', 4)
	);
	Serial1.print(sprintbuff);

	sprintf(sprintbuff, 
		" Held:%s",
		pBinFill(_state.held.allbits, pBinFill_Buffer, '\xB7', 4)
	);
	Serial1.print(sprintbuff);
	
	sprintf(sprintbuff, 
		" Changed:%s",
		pBinFill(_state.changed.allbits,  pBinFill_Buffer, '\xB7', 4)
	);
	Serial1.print(sprintbuff);
}