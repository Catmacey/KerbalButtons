uint8_t * address;
uint8_t buffer[256]; //address for temporary storage and parsing buffer
uint8_t structSize;
uint8_t calc_CS;	   //calculated Chacksum
// Packet id
uint8_t id;
//index for RX parsing buffer
uint8_t rx_array_inx;
uint8_t rx_len;

uint32_t deadtime, deadtimeOld;

KSPSerialIOState_t IOState = {false, false, false};

// These should be here
HandShakePacket_t HPacket;
// VesselData_t VData;
ControlPacket_t CPacket;


// Macro to provide address and len of datapacket
#define DETAILS(name) (uint8_t*)&name,sizeof(name)

//This shit contains stuff borrowed from EasyTransfer lib
boolean KSPBoardReceiveData() {

	
	if((rx_len == 0)&&(Serial2.available()>3)){
		
		// Serial1.println("here");
		
		// We look for the preamble "BEEF"
		while(Serial2.read()!= 0xBE) {
			if(Serial2.available() == 0){
				return false;
			}
		}


		if(Serial2.read() == 0xEF){
			IOState.DataReceived = true;
			rx_len = Serial2.read();
			id = Serial2.read(); 
			rx_array_inx = 1;

			switch(id) {
				case 0:
					structSize = sizeof(HPacket);   
					address = (uint8_t*)&HPacket;     
					break;
				case 1:
					structSize = sizeof(VData);   
					address = (uint8_t*)&VData;     
					break;
			}
		}

		//make sure the binary structs on both Arduinos are the same size.
		if(rx_len != structSize){
			rx_len = 0;
			return false;
		}
	}

	if(rx_len != 0){
		while(Serial2.available() && rx_array_inx <= rx_len){
			buffer[rx_array_inx] = Serial2.read();
			rx_array_inx++;
		}
		buffer[0] = id;

		if(rx_len == (rx_array_inx-1)){
			// Seem to have got whole message
			// Last uint8_t is CS
			calc_CS = rx_len;
			for (int i = 0; i<rx_len; i++){
				calc_CS^=buffer[i];
			} 
			if(calc_CS == buffer[rx_array_inx-1]){//CS good
				memcpy(address,buffer,structSize);
				rx_len = 0;
				rx_array_inx = 1;
				return true;
			}
			else{
				//failed checksum, need to clear this out anyway
				rx_len = 0;
				rx_array_inx = 1;
				return false;
			}
		}
	}
	return false;
}


/**
 * Send a data packet to KSPSerial
 * address and len can be provided by the DETAIL() macro
 **/
void KSPBoardSendData(uint8_t * address, uint8_t len){
	IOState.DataSent = false;
	uint8_t CS = len;

	Serial2.write(0xBE);
	Serial2.write(0xEF);
	Serial2.write(len);

	for(int i = 0; i<len; i++){
		CS^=*(address+i);
		Serial2.write(*(address+i));
	}
	
	Serial2.write(CS);
	
	IOState.DataSent = true;
}

void InitTxPackets(){
	HPacket.id = 0;
	CPacket.id = 101;
}


/**
 * Check Serial buffer for new data from KSPIO
 * Handles inital handshaking and maintains IOState.connected
 * return:
 *  -1 no connection
 *   0 Handshake received
 *   1 Data received
 **/
int KSPCheckForUpdate() {
	int returnValue = -1;
	uint32_t now = millis();

	if(KSPBoardReceiveData()){
		deadtimeOld = now;
		returnValue = id;
		switch(id) {
			case 0: //Handshake packet
				SendHandshake();
				break;
			case 1:
				// Indicators();
				// This was Vessel Data
				break;
		}

		IOState.Connected = true;
	}else{
		// If no message received for a while, go idle
		deadtime = now - deadtimeOld; 
		if(deadtime > IDLETIMER){
			deadtimeOld = now;
			IOState.Connected = false;
		}
	}
	return returnValue;
}


/**
 * Sends a Control Packet to KSPIO
 **/
void KSPSendControlData(){
	// TODO: Replace macro with inline code
	KSPBoardSendData(DETAILS(CPacket));
}


/**
 * Sends a Serial Handshake to KSPIO
 **/
void SendHandshake(){
	HPacket.id = 0;
	HPacket.M1 = 3;
	HPacket.M2 = 1;
	HPacket.M3 = 4;
	// TODO: Replace macro with inline code
	KSPBoardSendData(DETAILS(HPacket));
}

/**
 * Clears the status flags so they can be populated again next loop
 **/
void ClearState(){
	IOState = {false, false, false};
}

/**
 * Returns a State value from VesselData.ActionGroups
 * This is where RCS, SAS, GEAR as well as Action group state is stored
 * @agbit Action group ENUM value eg. AGSAS or AGCustom04
 **/
boolean KSPGetControlState(uint8_t agbit){
	return ((VData.ActionGroups >> agbit) & 1) == 1;
}


/**
 * Returns the Current SAS mode
 **/
uint8_t KSPGetSASMode(){
	// Navballmode and SASmode are stored in the same byte
	// We want just the low nibble
	return VData.NavballSASMode & 0b00001111;
}


/**
 * Returns the current Navball mode.
 **/
uint8_t KSPGetNavballMode(){
	// Navballmode and SASmode are stored in the same byte
	// We want just the high nibble
	return VData.NavballSASMode >> 4;
}


/**
 * Sets a new SAS mode
 * @mode SAS mode enum value eg. SMPrograde
 **/
void KSPSetSASMode(uint8_t mode){
	// Navballmode and SASmode are stored in the same byte
	// We only want to change the low nibble
	CPacket.NavballSASMode &= 0b11110000;
	CPacket.NavballSASMode += mode;
}

/**
 * Sets a new Navball mode
 * @mode Navball mode enum eg. NAVBallORBIT
 **/
void KSPSetNavballMode(uint8_t mode){
	// Navballmode and SASmode are stored in the same byte
	// We only want to change the high nibble
	CPacket.NavballSASMode &= 0b00001111;
	CPacket.NavballSASMode += (mode << 4);
}


/**
 * Sets a control group bit
 * I think this is setting Action Groups?
 * @groupbit the group number 1 - 10
 * @state the new state
 **/
void KSPSetActionGroup(uint8_t groupbit, boolean state){
	if(state){
		CPacket.ControlGroup |= (1 << groupbit);       // forces nth bit of x to be 1.  all other bits left alone.
	}else{
		CPacket.ControlGroup &= ~(1 << groupbit);      // forces nth bit of x to be 0.  all other bits left alone.
	}
}


/**
 * Sets the state of one of the ControlPacket.MainContols bits
 * This is how we send button presses back to KSPIO
 * @controlbit Main Controls ENUM value eg. SAS
 * @state the new value
 **/
void KSPSetMainControls(uint8_t controlbit, boolean state){
	if(state){
		CPacket.MainControls |= (1 << controlbit);  // forces nth bit of x to be 1. all other bits left alone.
	}else{
		CPacket.MainControls &= ~(1 << controlbit); // forces nth bit of x to be 0. all other bits left alone.
	}
}

/**
 * Toggles the state one of the ControlPacket.MainContols bits
 * @controlbit Main Controls ENUM value eg. SAS
 **/
void KSPToggleMainControls(uint8_t controlbit){
	CPacket.MainControls ^= (1 << controlbit);
}


/**
 * Returns the state of the requested main control bit
 * I DONT THINK THIS IS NEEDED.
 * Why would you read the control packet it is used to send data
 **/
boolean KSPGetMainControls(uint8_t controlbit){
	// Mask out all but the bit we want
	return CPacket.MainControls & (1 << controlbit);
}