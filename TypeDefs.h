/**
 * Project specific typedefs
 **/

// This represents the inputs
typedef union {
	uint8_t complete;
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
	Input_t pressed;  // Just pressed (ie was not pressed last time)
	Input_t released; // 
	Input_t changed;
} InputState_t;
