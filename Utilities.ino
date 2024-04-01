
/**
 * Update our indicator LEDs based on new Vessel data
 **/
void UpdateIndicators(){
	// Illuminate the top section of all the buttons
	digitalWrite(LED_ALLTOP, HIGH);
	// Illuminate the relevant button
	digitalWrite(LED_GEAR,   KSPGetActionGroups(AGGear));
	digitalWrite(LED_RCS,    KSPGetActionGroups(AGRCS));
	digitalWrite(LED_LIGHTS, KSPGetActionGroups(AGLight));
	digitalWrite(LED_SAS,    KSPGetActionGroups(AGSAS));
}


/**
 * Update the CPacket.NavballSASMode data to match the incomming Vessel Data
 * Call this every time you receive new data
 **/
void UpdateNavballSASModeState(){
	KSPSetSASMode(KSPGetSASMode());
	KSPSetNavballMode(KSPGetNavballMode());
}

/**
 * Update the CPacket.MainControls data to match the incomming Vessel Data
 * Call this every time you receive new data
 **/
void UpdateMainControlsState(){
	KSPSetMainControls(RCS, KSPGetActionGroups(AGRCS));
	KSPSetMainControls(GEAR, KSPGetActionGroups(AGGear));
	KSPSetMainControls(LIGHTS, KSPGetActionGroups(AGLight));
	KSPSetMainControls(SAS, KSPGetActionGroups(AGSAS));
}

/**
 * Update the CPacket data to match the incomming Vessel Data
 * Call this every time you receive new data
 **/
void UpdateCPacketDataFromVessel(){
	// CPacket.NavballSASMode
	KSPSetSASMode(KSPGetSASMode());
	KSPSetNavballMode(KSPGetNavballMode());
	// CPacket.MainControls
	KSPSetMainControls(RCS, KSPGetActionGroups(AGRCS));
	KSPSetMainControls(GEAR, KSPGetActionGroups(AGGear));
	KSPSetMainControls(LIGHTS, KSPGetActionGroups(AGLight));
	KSPSetMainControls(SAS, KSPGetActionGroups(AGSAS));
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
		if(KSPGetActionGroups(AGSAS)){
			// Turn SAS off
			KSPSetSASMode(SMOFF);
			KSPSetMainControls(SAS, false);
		}else{
			// Turn SAS on
			KSPSetMainControls(SAS, true);
			KSPSetSASMode(SMSAS);
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



/**
 * Prints out the state of CPacket
 * @output is this data we are sending [true] or that we received [false]
 **/
void DebugControlData(boolean output){
	#ifndef DEBUGLEVEL_FULL
		// Skip this if not in correct level
		return;
	#endif
	Serial1.print("\nCPacket:");
	// Show navball
	sprintf(
		sprintbuff,
		" NavballSASMode %s",
		pBinFill(CPacket.NavballSASMode, pBinFill_Buffer, '\xB7', 8)
	);
	Serial1.print(sprintbuff);
	
	// Show Control Groups
	sprintf(
		sprintbuff,
		" ControlGroup %s",
		pBinFill(CPacket.ControlGroup, pBinFill_Buffer, '\xB7', 16)
	);
	Serial1.print(sprintbuff);

	sprintf(
		sprintbuff,
		" MainControls %s",
		pBinFill(CPacket.MainControls, pBinFill_Buffer, '\xB7', 8)
	);
	Serial1.print(sprintbuff);

	if(output){
		Serial1.print(" --> To send");
	}else{
		Serial1.print(" <-- Received");
	}

}


/**
 * Prints out the state of VesselData controls
 **/
void DebugVesselData(){
	#ifndef DEBUGLEVEL_FULL
		// Skip this if not in correct level
		return;
	#endif
	Serial1.print("\nVessel: ");
	// Show navball
	sprintf(
		sprintbuff,
		" NavballSASMode %s",
		pBinFill(VData.NavballSASMode, pBinFill_Buffer, '\xB7', 8)
	);
	Serial1.print(sprintbuff);
	// Show action group
	sprintf(
		sprintbuff,
		" ActionGroups %s",
		pBinFill(VData.ActionGroups, pBinFill_Buffer, '\xB7', 16)
	);
	Serial1.print(sprintbuff);

}


/**
 * Prints out the current buttons states
 **/
void DebugButtonStates(){
	#ifndef DEBUGLEVEL_FULL
		// Skip this if not in correct level
		return;
	#endif
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