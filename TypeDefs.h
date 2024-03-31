/**
 * Project specific typedefs
 **/

// This represents the inputs
typedef union {
	uint8_t allbits;
	struct {
		unsigned gear: 1;
		unsigned rcs: 1;
		unsigned sas: 1;
		unsigned lights: 1;
		unsigned brakes: 1;
	};
} Input_t;

// This represents the overall state once debounced
typedef struct {
	Input_t pressed;  // Just pressed
	Input_t held;     // Currently pressed
	Input_t released; // Just Released
	Input_t changed;  // Change in state
} InputState_t;
