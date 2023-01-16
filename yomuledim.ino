#include <NeoSWSerial.h>
#include <Adafruit_NeoPixel.h>


// Buttons pin definitions:
#define buttonPin1 4 // the number of the "play random song" button pin (stomach)
#define buttonPin2 5  // the number of the "move one library backward" button pin (button2)
#define buttonPin3 6  // the number of the "move one library forward" button pin (button3)

//Leds pin definitions:
static const int LED_COUNT = 2;  // Number of LEDs in the strip
static const int LED_PIN = 7;  // Pin where the LED is connected
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


//Mp3 player pin definition
#define ARDUINO_RX 3 //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 2 //connect to RX of the module
NeoSWSerial myMP3(ARDUINO_RX, ARDUINO_TX);


//Variables for reading the pushbutton current and previous state
int buttonState_1 = 0;  
int buttonState_2 = 0;  
int buttonState_3 = 0;  

int previousButton_1_State = HIGH;  // for checking the state of a pushButton
int previousButton_2_State = HIGH;  // for checking the state of a pushButton
int previousButton_3_State = HIGH;  // for checking the state of a pushButton



static int8_t Send_buf[6] = {0} ;
/************Command byte**************************/
/*basic commands*/
#define CMD_PLAY  0X01
#define CMD_PAUSE 0X02
#define CMD_NEXT_SONG 0X03
#define CMD_PREV_SONG 0X04
#define CMD_VOLUME_UP   0X05
#define CMD_VOLUME_DOWN 0X06
#define CMD_FORWARD 0X0A // >>
#define CMD_REWIND  0X0B // <<
#define CMD_STOP 0X0E
#define CMD_STOP_INJECT 0X0F//stop interruptting with a song, just stop the interlude

/*5 bytes commands*/
#define CMD_SEL_DEV 0X35
  #define DEV_TF 0X01
#define CMD_IC_MODE 0X35
  #define CMD_SLEEP   0X03
  #define CMD_WAKE_UP 0X02
  #define CMD_RESET   0X05

/*6 bytes commands*/  
#define CMD_PLAY_W_INDEX   0X41
#define CMD_PLAY_FILE_NAME 0X42
#define CMD_INJECT_W_INDEX 0X43

/*Special commands*/
#define CMD_SET_VOLUME 0X31
#define CMD_PLAY_W_VOL 0X31
#define CMD_SET_PLAY_MODE 0X33
  #define ALL_CYCLE 0X00
  #define SINGLE_CYCLE 0X01
#define CMD_PLAY_COMBINE 0X45//can play combination up to 15 songs

uint8_t volume = 0X10;
uint8_t songId;

//red array
const int8_t yesh_anashim = 0X03;
const int8_t san_fransisco = 0X04;

//green array
const int8_t saa_leat = 0X05;
const int8_t od_ihiye = 0X06;
const int8_t od_nipagesh = 0X07;

//blue array
const int8_t atur_mizheh = 0X01;
const int8_t yesh_bi_ahava = 0X02;

//Cyan array
const int8_t oof_gozal = 0X08;


//{red_library, green_library, blue_library, yellow_library, cyan_library, magneta_library, white_library};
/*
  IMPORTATN!!!
  When adding/removing songs/library from 'libraries' 2D array make sure:
 - numOfLibraries and biggestLibraryLength values are up to date and correct
 - librariesLengths array is updated with the correct lengths of each library in 'libraries', respectively to the position of each library. e.g: if the libraries[0] library has the length of 3,
   librariesLengths[0] should be also 3 and will look like this: librariesLengths[numOfLibraries] = {3,x,y, ...}
 - colors array consist a respective color to your new library (if added one)
*/
const int numOfLibraries = 4;
const int biggestLibraryLength = 4;
const int8_t libraries[numOfLibraries][biggestLibraryLength] = {{yesh_anashim, san_fransisco}, {saa_leat, od_ihiye, od_nipagesh}, {atur_mizheh, yesh_bi_ahava}, {oof_gozal}};
const int8_t librariesLengths[numOfLibraries] = {2,3,2,1}; //only way to get the length of an 'internal' array in a 2d array, with different lengths. UPDATE IT ACCORDING TO 'libraries' UPON ANY CHANGE!!!


// Define an array of colors
uint32_t colors[] = {
  strip.Color(255, 0, 0),  // Red
  strip.Color(0, 255, 0),  // Green
  strip.Color(0, 0, 255),  // Blue
  strip.Color(0, 255, 255),  // Cyan
  strip.Color(255, 255, 0),  // Yellow
  strip.Color(255, 0, 255),  // Magenta
  strip.Color(255, 255, 255)  // White
};

int libraryLength;
int currentLibraryIndex = 0; //set red library as default

int8_t randomInt = -1;

void sendCommand(int8_t command, int16_t dat );

void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);

  // Initialize all led pixels to 'off'
  strip.begin();
  strip.show();  

  // mp3 chip initialization
  myMP3.begin(9600);
	delay(500);//Wait chip initialization is complete
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card  
	delay(500);//wait for 200ms
  // mp3 chip initialization
  
  Serial.begin(9600);
  randomSeed(analogRead(A0)); 
}

void loop() {
  // read the state of the pushbutton value:
  buttonState_1 = digitalRead(buttonPin1);
  buttonState_2 = digitalRead(buttonPin2);
  buttonState_3 = digitalRead(buttonPin3);

  // Button 1 case: play a random song out ouf the current library
  if ((buttonState_1 != previousButton_1_State) && buttonState_1 == LOW){
    playRandomSong(currentLibraryIndex);
    delay(50);
  }
  
  // Button 2 case: move one library 'backward' and play a random song out of it
  else if ((buttonState_2 != previousButton_2_State) && buttonState_2 == LOW){
    if (currentLibraryIndex == 0){
      currentLibraryIndex = numOfLibraries-1;      
    }
    else{
      currentLibraryIndex--;
    }
    Serial.print("currentLibraryIndex ");
    Serial.println(currentLibraryIndex);
    playRandomSong(currentLibraryIndex);
    delay(50);
  }

  // Button 3 case: move one library 'forward' and play a random song out of it
  else if ((buttonState_3 != previousButton_3_State) && buttonState_3 == LOW){
    if (currentLibraryIndex == numOfLibraries-1){
      currentLibraryIndex = 0;      
    }
    else{
      currentLibraryIndex++;
    }
    Serial.print("currentLibraryIndex ");
    Serial.println(currentLibraryIndex);
    playRandomSong(currentLibraryIndex);
    delay(50);
  }
  strip.setPixelColor(0, colors[currentLibraryIndex]);  // Set the first LED (eye) to the current color in the array
  strip.setPixelColor(1, colors[currentLibraryIndex]);  // Set the second LED (eye) to the current color in the array
  strip.show();
  previousButton_1_State = buttonState_1;
  previousButton_2_State = buttonState_2;
  previousButton_3_State = buttonState_3;  
  delay(100);
}

int generateRandomInt(int catagory_length) {
  Serial.print("catagory_length: ");
  Serial.println(catagory_length);
  if (catagory_length == 1){ 
    return 0;
  }
  int randomValue = random(0, catagory_length);
  while (randomValue == randomInt){ //making sure to return a different value each time
    randomValue = random(0, catagory_length);
    
  }
  Serial.print("pre: ");
  Serial.println(randomInt);
  Serial.print("new: ");
  Serial.println(randomValue);
  return randomValue;
}

void playRandomSong(int8_t libraryIndex)
{
  libraryLength = librariesLengths[libraryIndex];
  randomInt = generateRandomInt(libraryLength);  
  songId = libraries[libraryIndex][randomInt];
  Serial.print("songId ");
  Serial.println(songId);
  uint16_t value = (volume << 8) | songId;
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
  if((command == CMD_PLAY_W_VOL)||(command == CMD_SET_PLAY_MODE)||(command == CMD_PLAY_COMBINE))
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