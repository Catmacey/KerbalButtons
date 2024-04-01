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
