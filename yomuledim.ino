#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include <avr/sleep.h>
#include "recordings.h"


// Buttons pin definitions:
#define buttonPin1 4 // the number of the "play random song" button pin (stomach)
#define buttonPin2 2  // the number of the "move one library backward" button pin (button2)
#define buttonPin3 3  // the number of the "move one library forward" button pin (button3)

//Leds pin definitions:
static const int LED_COUNT = 2;  // Number of LEDs in the strip
static const int LED_PIN = 9;  // Pin where the LED is connected
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


//Mp3 player pin definition
#define ARDUINO_RX 6 //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 5 //connect to RX of the module
SoftwareSerial myMP3(ARDUINO_RX, ARDUINO_TX);


//Variables for reading the pushbutton current and previous state
int stomach_buttonState = 0;  
int leftHand_buttonState = 0;  
int rightHand_buttonState = 0;  

int stomach_previousButton_State = HIGH;  // for checking the state of a pushButton
int leftHand_previousButton_State = HIGH;  // for checking the state of a pushButton
int rightHand_previousButton_State = HIGH;  // for checking the state of a pushButton

bool pressed_both_buttons = false;
bool stomach_button = false;
bool left_hand_button = false;
bool right_hand_button = false;



static int8_t Send_buf[6] = {0} ;
/************Command byte**************************/
/*5 bytes commands*/
#define CMD_SEL_DEV 0X35
#define DEV_TF 0X01

/*6 bytes commands*/  
#define CMD_PLAY_W_INDEX   0X41
#define CMD_PLAY_FILE_NAME 0X42

#define CMD_PLAY_W_VOL 0X31


// Define an array of                     
uint32_t colors[] = { //add/remove color according to libraries changes
  strip.Color(255, 0, 0),  // Red 
  strip.Color(0, 255, 0),  // Green
  strip.Color(0, 0, 255),  // Blue
  strip.Color(0, 255, 255),  // Cyan
  strip.Color(255, 255, 0),  // Yellow
  strip.Color(255, 0, 255),  // Magenta
  strip.Color(255, 255, 255),  // White
  strip.Color(255, 165, 0),  // Orange
  strip.Color(255, 192, 203)  // Pink
};

//uint8_t volume = 0X20;
uint8_t volume = 0X20;
uint8_t songId;

int mainDelay = 100; //Delay time between each loop of the main func (in milli seconds)

int libraryLength;
int currentLibraryIndex = 0; //set red library as default
int previousLibraryIndex = 0;

int brightness = 255;
int dimming_time = 5; //how much time (sec) will it take to dim the led completly off
unsigned long startTime;
const long interval = 5000;

int8_t randomInt = -1;

int timeWindow = 70;

void sendCommand(int8_t command, int16_t dat );

void setup() {
  //turn off builtin led to save power
  pinMode(LED_BUILTIN,OUTPUT);//We use the led on pin 13 to indecate when Arduino is A sleep
  digitalWrite(LED_BUILTIN,LOW);//turning LED off

  // initialize the pushbutton pin as an input:
  startTime = millis();
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);

  // Initialize all led pixels to 'off'
  strip.begin();
  strip.setPixelColor(0, colors[currentLibraryIndex]);  // Set the first LED (eye) to the current color in the array
  strip.setPixelColor(1, colors[currentLibraryIndex]);  // Set the second LED (eye) to the current color in the array
  strip.show();  

  // mp3 chip initialization
  myMP3.begin(9600);
	delay(500);//Wait chip initialization is complete
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card  
	delay(500);//wait for 200ms
  
  Serial.begin(9600);
  randomSeed(analogRead(A0)); 
}

void loop() {
  // read the state of the pushbutton value:
  stomach_buttonState = digitalRead(buttonPin1);
  leftHand_buttonState = digitalRead(buttonPin2);
  rightHand_buttonState = digitalRead(buttonPin3);
  
  pressed_both_buttons = false;
  stomach_button = (stomach_buttonState != stomach_previousButton_State) && stomach_buttonState == LOW;  
  left_hand_button = (leftHand_buttonState != leftHand_previousButton_State) && leftHand_buttonState == LOW;  
  right_hand_button = (rightHand_buttonState != rightHand_previousButton_State) && rightHand_buttonState == LOW;  


  if ((left_hand_button) || (right_hand_button)){
    Serial.println("Pressed one of the buttons");    
    if (left_hand_button){
      //wait for button 3 for 50 ms
      pressed_both_buttons = wait_for_button_press(buttonPin3, timeWindow);
      }
    else if (right_hand_button){
      //wait for button 2 for 50 ms (pin= ,time window)
      pressed_both_buttons = wait_for_button_press(buttonPin2, timeWindow);
    }
  }

  if (pressed_both_buttons){
    rainbow(10);      
  }
  // Button 1 case: play a random song out ouf the current library
  else if (stomach_button){
    clicked_stomach();
  }
  else if (left_hand_button){
    clicked_left_hand();  // Left hand case: move one library 'backward' and play a random song out of it
  }
  else if (right_hand_button){
    clicked_right_hand(); // Right hand case: move one library 'forward' and play a random song out of it
  }

  if (currentLibraryIndex != previousLibraryIndex){ //if changed library
    turn_on_leds();
    previousLibraryIndex = currentLibraryIndex;
  }
  else if (millis() - startTime >= interval) { // 5 seconds have passed
    if (strip.getBrightness() != 0) {    
      dimLeds();
      mainDelay = (dimming_time*1000)/127; // (dimmimng_time*1000)/(255/brightness_interval)
    }
    else if (millis() - startTime >= 60*1000) { // Go to sleep after 30 seconds from the last activity
      Going_To_Sleep();
      mainDelay = 100; 
    }
  }
  
  stomach_previousButton_State = stomach_buttonState;
  leftHand_previousButton_State = leftHand_buttonState;
  rightHand_previousButton_State = rightHand_buttonState;

  delay(mainDelay);
}

//********************************************************************//
//********************************************************************//
//********************************************************************//
//********************************************************************//

void clicked_left_hand(){
  Serial.println("Pressed left hand button");
  startTime = millis();
  mainDelay = 100;
  if (currentLibraryIndex == 0){
    currentLibraryIndex = numOfLibraries-1;      
  }
  else{
    currentLibraryIndex--;
  }
  Serial.print("currentLibraryIndex ");
  Serial.println(currentLibraryIndex);
  playRandomSong(currentLibraryIndex);
  delay(10);
}

void clicked_right_hand(){
  Serial.println("Pressed right hand button");
  startTime = millis();
  mainDelay = 100;
  if (currentLibraryIndex == numOfLibraries-1){
    currentLibraryIndex = 0;      
  }
  else{
    currentLibraryIndex++;
  }
  Serial.print("currentLibraryIndex ");
  Serial.println(currentLibraryIndex);
  playRandomSong(currentLibraryIndex);
  delay(10);
}

void clicked_stomach(){
  Serial.println("Pressed stomach button");
  startTime = millis();
  mainDelay = 100;
  turn_on_leds();
  playRandomSong(currentLibraryIndex);
  delay(10);
}



bool wait_for_button_press(int buttonPin, int timeWindow){    
    int buttonState = 1;
    int previousButton_State = HIGH;  // for checking the state of a pushButton
    unsigned long endTime;
    bool buttonPressed = false;

    endTime = millis() + timeWindow;
    while ((endTime > millis()) and (!buttonPressed)){
      buttonState = digitalRead(buttonPin);
      buttonPressed = (buttonState != previousButton_State) && buttonState == LOW;
      previousButton_State = buttonState;
    }
  return buttonPressed;    
  }

void dimLeds(){
  brightness = brightness - 2; //brightness_interval
    if (brightness < 0){
      brightness = 0;
    }
    strip.setBrightness(brightness);
    strip.show();
}

void turn_on_leds(){
    brightness = 255;
    strip.setBrightness(brightness);
    strip.setPixelColor(0, colors[currentLibraryIndex]);  // Set the first LED (eye) to the current color in the array
    strip.setPixelColor(1, colors[currentLibraryIndex]);  // Set the second LED (eye) to the current color in the array
    strip.show();
}

int generateRandomInt(int catagory_length) {
  if (catagory_length == 1){ 
    return 0;
  }
  int randomValue = random(0, catagory_length);
  while (randomValue == randomInt){ //making sure to return a different value each time
    randomValue = random(0, catagory_length);
    
  }
  return randomValue;
}

void playRandomSong(int8_t libraryIndex)
{
  libraryLength = librariesLengths[libraryIndex];
  randomInt = generateRandomInt(libraryLength);  
  songId = libraries[libraryIndex][randomInt];
  uint16_t value = (volume << 8) | songId;
  Serial.println(songId);
  playWithVolume(value);
}

void playWithVolume(int16_t dat)
{
  mp3_6bytes(CMD_PLAY_W_VOL, dat);
}

void mp3Basic(int8_t command)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x02; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = 0xef; //
  sendBytes(4);
}
void mp3_5bytes(int8_t command, uint8_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x03; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = dat; //
  Send_buf[4] = 0xef; //
  sendBytes(5);
}
void mp3_6bytes(int8_t command, int16_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x04; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = (int8_t)(dat >> 8);//datah
  Send_buf[4] = (int8_t)(dat); //datal
  Send_buf[5] = 0xef; //
  sendBytes(6);
}
void sendBytes(uint8_t nbytes)
{
  for(uint8_t i=0; i < nbytes; i++)//
  {
    myMP3.write(Send_buf[i]) ;
  }
}

void sendCommand(int8_t command, int16_t dat = 0)
{
  delay(20);
  if((command == CMD_PLAY_W_VOL))
  	return;
  else if(command < 0x10) 
  {
	mp3Basic(command);
  }
  else if(command < 0x40)
  { 
	mp3_5bytes(command, dat);
  }
  else if(command < 0x50)
  { 
	mp3_6bytes(command, dat);
  }
  else return;
}


//############## special leds effects ##############//
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void rainbow(uint8_t wait) {
  brightness = 255;
  strip.setBrightness(brightness);
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}


//############## sleep mode functions ##############//
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.

void Going_To_Sleep(){
    sleep_enable();//Enabling sleep mode
    attachInterrupt(0, wakeUp, LOW);//attaching a interrupt to pin d2
    attachInterrupt(1, wakeUp, LOW);//attaching a interrupt to pin d2
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);//Setting the sleep mode, in our case full sleep
    sleep_cpu();//activating sleep mode
    Serial.println("just woke up!");//next line of code executed after the interrupt 
  }

void wakeUp(){
  Serial.println("Interrrupt Fired");//Print message to serial monitor
  sleep_disable();//Disable sleep mode
  detachInterrupt(0); //Removes the interrupt from pin 2;
  detachInterrupt(1); //Removes the interrupt from pin 2;
}
