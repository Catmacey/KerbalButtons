uint8_t rx_len;
uint8_t * address;
uint8_t buffer[256]; //address for temporary storage and parsing buffer
uint8_t structSize;
uint8_t rx_array_inx;  //index for RX parsing buffer
uint8_t calc_CS;	   //calculated Chacksum

KSPSerialIOState_t IOState = {false, false, false};

//macro
#define details(name) (uint8_t*)&name,sizeof(name)

//This shit contains stuff borrowed from EasyTransfer lib
boolean KSPBoardReceiveData() {
	if ((rx_len == 0)&&(Serial.available()>3)){
		while(Serial.read()!= 0xBE) {
			if (Serial.available() == 0){
				// digitalWrite(LED_RX, LOW);

				return false;
			}
		}
		if(Serial.read() == 0xEF){
			// digitalWrite(LED_RX, HIGH);
			IOState.DataReceived = true;
			rx_len = Serial.read();
			id = Serial.read(); 
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
			// digitalWrite(LED_RX, LOW);
			return false;
		}
	}

	if(rx_len != 0){
		while(Serial.available() && rx_array_inx <= rx_len){
			buffer[rx_array_inx] = Serial.read();
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
				// digitalWrite(LED_RX, LOW);
				return true;
			}
			else{
				//failed checksum, need to clear this out anyway
				rx_len = 0;
				rx_array_inx = 1;
				// digitalWrite(LED_RX, LOW);
				return false;
			}
		}
	}
	// digitalWrite(LED_RX, LOW);
	return false;
}

void KSPBoardSendData(uint8_t * address, uint8_t len){
	// digitalWrite(LED_TX,HIGH); 

	IOState.DataSent = true;

	uint8_t CS = len;

	Serial.write(0xBE);
	Serial.write(0xEF);
	Serial.write(len);

	for(int i = 0; i<len; i++){
		CS^=*(address+i);
		Serial.write(*(address+i));
	}
	
	Serial.write(CS);
	
	// digitalWrite(LED_TX,LOW); 
}

void InitTxPackets(){
	HPacket.id = 0;
	CPacket.id = 101;
}


/**
 * Receives data from KSPIO
 **/
int ReceiveSerialData() {
	int returnValue = -1;
	now = millis();

	if(KSPBoardReceiveData()){
		deadtimeOld = now;
		returnValue = id;
		switch(id) {
			case 0: //Handshake packet
				SendHandshake();
				Serial1.print("\nSent Handshake\n");
				break;
			case 1:
				// Indicators();
				// This was Vessel Data
				break;
		}

		//We got some data, turn the green led on
		// digitalWrite(LED_CONNECTED, HIGH);
		IOState.Connected = true;
		// Connected = true;
	}else{
		//if no message received for a while, go idle
		deadtime = now - deadtimeOld; 
		if(deadtime > IDLETIMER){
			deadtimeOld = now;
			// Connected = false;
			Serial1.print("\nNot connected!\n");
			IOState.Connected = false;
			// digitalWrite(LED_CONNECTED, LOW);
		}
	}
	return returnValue;
}

/**
 * Sends a Serial Handshake to KSPIO
 **/
void SendHandshake(){
	HPacket.id = 0;
	HPacket.M1 = 3;
	HPacket.M2 = 1;
	HPacket.M3 = 4;
	KSPBoardSendData(details(HPacket));
}

/**
 * Clears the status flags so they can be populated again next loop
 **/
void ClearState(){
	IOState = {false, false, false};
}