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
 * You need to define your max bit length
 * eg. #define kDisplayWidth 16
 * You also need to declare a buffer to store the output
 * eg. char pBinFill_Buffer[kDisplayWidth+1];
 */
char* pBinFill(long int x, char *so, char fillChar){
	// fill in array from right to left
	char s[kDisplayWidth+1];
	int i = kDisplayWidth;
	s[i--] = 0x00;  // terminate string
	do{
		// fill in array from right to left
		s[i--] = (x & 1) ? '1' : '0';
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
void showStatusLED(){
	ws2812_led[0] = CRGB(
		(IOState.Connected?50:0),
		(IOState.DataSent?50:0),
		(IOState.DataReceived?50:0)
	);

	FastLED.show();

}




/**
 * Gather input for later debounce
 * Run at around 200+Hz
 **/
void GatherInput(){
	// Fill up the buffer with input events
	
	// MAYB: Read all these pins in one go somehow
	g_inputbuffer[inputstep].sas    = digitalRead(PIN_SAS);
	g_inputbuffer[inputstep].rcs    = digitalRead(PIN_RCS);
	g_inputbuffer[inputstep].gear   = digitalRead(PIN_GEAR);
	g_inputbuffer[inputstep].lights = digitalRead(PIN_LIGHTS);

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
	_then.complete = _now.complete;
	
	// Clear current states
	_state.pressed.complete = 0;
	// _state.repeat.complete = 0;
	_state.released.complete = 0;
	
	// Default the _now input to all pressed
	_now.complete = 0xff;
	// AND the input buffer into _now to debounce button presses
	// We end up with a struct containing the debounced state of the buttons
	for(idx=0; idx < INPUT_DEBOUNCE_BUFLEN; idx++){
		_now.complete = (_now.complete & g_inputbuffer[idx].complete);
	}
	
	// Calculate the changed and released states using our previous data

	_state.changed.complete = (_then.complete ^ _now.complete);
	_state.released.complete = (_then.complete & _state.changed.complete);

	// Pressed is true if the button wasn't pressed before
	_state.pressed.complete = (~_then.complete & _now.complete);



	// NOTE: held and repeat are not needed
	/*
	// We use a buffer of previous states to determine if a button has been held down
	// After a certain number of held down events we can emit a repeat
	// Store current input to determine id individual buttons have been held
	_buffer[_repeat_buff_idx].complete = _now.complete;
	if(_repeat_buff_idx < INPUT_REPEAT_LONG-1){
		_repeat_buff_idx++;
	}else{
		_repeat_buff_idx = 0;
	}
	
	// Now determine if buttons have been held
	// Held has to start all 1s so we can AND out those that are not pressed 
	_state.held.complete = 0xff;
	for(idx=0; idx < INPUT_REPEAT_LONG; idx++){
		_state.held.complete = (_state.held.complete & _buffer[idx].complete);
	}

	// Update changed in case of repeat
	// _state.changed.complete |= _state.held.complete;
	*/

	return _state;
}