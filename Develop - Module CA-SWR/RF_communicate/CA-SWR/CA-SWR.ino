#include <EEPROM.h>
#include <RF24.h>
#include <SPI.h>

/*
	Product code: 2a0a6b88-769e-4a63-ac5d-1392a7199e88
	RF channel (1 button): 83878226022001
	Button topic: be47fa93-15df-44b6-bdba-c821a117cd41
*/


/* In this product - the address (channel) to communicate is define as SWR (convert to decimal) 
+ 'date make device' + 'product no.' ; In this case, SWR is known as '83 87 82' and add with the date 
making device for example today is Feb 17th and this is the first product in that day; then the address 
for this SWR is: const byte address[15] = "83878217022001"	( 83 87 82 | 17 02 20 | 01 )					*/

RF24 radio(9, 10);   //nRF24L01 (CE,CSN) connections PIN
const byte address[15] = "83878226022001";    //Changeable

const int button = 5;
const int control = 2;

int deviceState;
int buttonState;             
int lastButtonState = LOW;   

long lastDebounceTime = 0;  
long debounceDelay = 50;    

boolean state_Device_sendtoHC[1];
boolean state_Device_controlfromHC[1];

void setup() {
  SPI.begin();
  Serial.begin(9600);
  Serial.println("\nCA-SWR say hello to your home <3 ! ");
  pinMode(button, INPUT);
  pinMode(control, OUTPUT);
  
  digitalWrite(control, deviceState);
  checkDeviceState();
  radio.begin();
  radio.setRetries(15, 15);
  radio.setPALevel(RF24_PA_MAX);
}

void loop() 
{
  radio.openReadingPipe(1, address);
  radio.startListening();
  
  if (radio.available())
  {
   memset(&state_Device_controlfromHC, ' ', sizeof(state_Device_controlfromHC));
	 radio.read(&state_Device_controlfromHC, sizeof(state_Device_controlfromHC));
   digitalWrite(control, state_Device_controlfromHC[0]);
   deviceState = state_Device_controlfromHC[0];
  }
  
  int check_Button = digitalRead(button);
  
  if(check_Button != lastButtonState) {
	  lastDebounceTime = millis();
  }
  if((millis() - lastDebounceTime) > debounceDelay) {
	  if (check_Button != buttonState) {
		  buttonState = check_Button;
		  if (buttonState = HIGH) {
			  deviceState = !deviceState;
			  }
	  }
  }
  
  radio.stopListening();
  digitalWrite(control, deviceState);
  state_Device_sendtoHC[0] = deviceState;
  EEPROM.update(0, deviceState);
  
  radio.openWritingPipe(address);
  radio.write(&state_Device_sendtoHC[0], sizeof(state_Device_sendtoHC));
  
  lastButtonState = check_Button;
}

void checkDeviceState() {
   deviceState = EEPROM.read(0);
   if(deviceState == 1) {
    digitalWrite(control, HIGH);
   } 
   if(deviceState == 0) {
    digitalWrite(control, LOW);
   }
}
