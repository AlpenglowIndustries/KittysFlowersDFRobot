/*
  This code can be a good test code for your strip
  function: leds.setPixelColor(i,y) can be used to let number i of your led turn on with color of y
  and you can draw your idea easily with this function but never forget function: led.show()
*/
#include <Adafruit_NeoPixel.h>

#define RGBPIN     9
#define VMPIN      5     //Vibration motor pin
#define LED_COUNT  7 // the amount of the leds of your strip
// Create an instance of the Adafruit_NeoPixel class called "leds".
// That'll be what we refer to from here on...
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, RGBPIN, NEO_GRB + NEO_KHZ800);

String rssi = "";
String recData = "";
int rssi_int = 0;

static void InitVibrateMotor() {
  pinMode(VMPIN, OUTPUT);
}

static void StartToVibrate() {
  digitalWrite(VMPIN, HIGH);
}

static void StopToVibrate() {
  digitalWrite(VMPIN, LOW);
}



static void FlashingColor(uint8_t r,uint8_t g,uint8_t b){
  static uint32_t last_display_time = 0
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds.setPixelColor(i, r, g, b);
    delay(10);
  }
  leds.show();
  delay(50);
  clearLEDs();
  leds.show();
}


static void WaterLamp() {
  static uint32_t last_display_time = 0;
  static uint8_t  current_led = 0;
  if (millis() - last_display_time > 50) {  // 50 ms delay between frames
    last_display_time = millis();
    rainbow(current_led);
    current_led = (current_led + 1) % LED_COUNT;
  }
}


static void setMotherMode(){
  Serial.print("+++");
  delay(1000);
  Serial.println("AT+SETTING=DEFCENTRAL");
  delay(50);
  Serial.println("AT+EXIT");
  delay(50);
  
}

static void CalculationRSSI() {
  while (Serial.available()) {
    rssi += char(Serial.read());
    delay(2);
  }
  Serial.flush();
  delay(10);
  if (rssi.length() == 6) {
    rssi_int = rssi.toInt();
  }
  rssi = "";
}


static bool CheckRec(String data){
  if(!recData.compareTo(data)){
    return true;
  }
   return false;
}

static void WaitData(uint32_t timeout){
  uint32_t nowTemptime = millis();
  while(!Serial.available()){
    if((millis() - nowTemptime) > timeout){
      break;
    }
  }

}
static void ReceiveData(){
  recData = "";
  while(Serial.available()){
    recData += (char)Serial.read();
    delay(2);
  }
}

static void getRSSI(){
  ReceiveData();
  if(recData.length() > 0){//there are some data
    return;
  }
  Serial.print("+++");
  WaitData(2000);
  ReceiveData();
  if(CheckRec("Enter AT Mode\r\n")){
    Serial.println("AT+RSSI=?");
    WaitData(2000);
    CalculationRSSI();
    Serial.println("AT+EXIT");
    WaitData(200);
    ReceiveData();
  }else{
    Serial.println("AT+EXIT");
    delay(100);
    Serial.println("AT+EXIT");
    WaitData(200);
    ReceiveData();
  }
  recData = "";
}



void setup()
{
  Serial.begin(115200);
  delay(1000);
  setMotherMode();
  InitVibrateMotor();
  leds.begin();  // Call this to start up the LED strip.
  clearLEDs();   // This function, defined below, turns all LEDs off...
  leds.show();   // ...but the LEDs don't actually update until you call this.
  for (int i = 0 ; i < 7; i++) {
    WaterLamp();
  }
}

uint32_t nowtime = 0;
uint32_t lasttime = 0;
uint32_t vibetime = 0;
void loop()
{
  if (abs(rssi_int) != 0) {
    nowtime = millis();
    if (nowtime - lasttime > 5000) { //Check signal every 5 seconds
      lasttime = nowtime;
      getRSSI();
    }
  } else {
    getRSSI();
  }

  if (abs(rssi_int) > 0 && abs(rssi_int) <= 60) { //Close
//    StopToVibrate();
//    WaterLamp();
    StartToVibrate();
    FlashingColor(255,0,0);
  } else if (abs(rssi_int) > 60) {//More than two meters, shaking, mom red flashes quickly
//    StartToVibrate();
//    FlashingColor(255,0,0);
    StopToVibrate();
    WaterLamp();
  } else if (abs(rssi_int) == 0) { //Not connected yet or disconnected
  }

}


// Sets all LEDs to off, but DOES NOT update the display;
// call leds.show() to actually turn them off after this.
void clearLEDs()
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds.setPixelColor(i, 0);
  }
}

// Prints a rainbow on the ENTIRE LED strip.
//  The rainbow begins at a specified position.
// ROY G BIV!
void rainbow(byte startPosition)
{
  // Need to scale our rainbow. We want a variety of colors, even if there
  // are just 10 or so pixels.
  int rainbowScale = 192 / LED_COUNT;

  // Next we setup each pixel with the right color
  for (int i = 0; i < LED_COUNT; i++)
  {
    // There are 192 total colors we can get out of the rainbowOrder function.
    // It'll return a color between red->orange->green->...->violet for 0-191.
    leds.setPixelColor(i, rainbowOrder((rainbowScale * (i + startPosition)) % 192));
  }
  // Finally, actually turn the LEDs on:
  leds.show();
}

// Input a value 0 to 191 to get a color value.
// The colors are a transition red->yellow->green->aqua->blue->fuchsia->red...
//  Adapted from Wheel function in the Adafruit_NeoPixel library example sketch
uint32_t rainbowOrder(byte position)
{
  // 6 total zones of color change:
  if (position < 31)  // Red -> Yellow (Red = FF, blue = 0, green goes 00-FF)
  {
    return leds.Color(0xFF, position * 8, 0);
  }
  else if (position < 63)  // Yellow -> Green (Green = FF, blue = 0, red goes FF->00)
  {
    position -= 31;
    return leds.Color(0xFF - position * 8, 0xFF, 0);
  }
  else if (position < 95)  // Green->Aqua (Green = FF, red = 0, blue goes 00->FF)
  {
    position -= 63;
    return leds.Color(0, 0xFF, position * 8);
  }
  else if (position < 127)  // Aqua->Blue (Blue = FF, red = 0, green goes FF->00)
  {
    position -= 95;
    return leds.Color(0, 0xFF - position * 8, 0xFF);
  }
  else if (position < 159)  // Blue->Fuchsia (Blue = FF, green = 0, red goes 00->FF)
  {
    position -= 127;
    return leds.Color(position * 8, 0, 0xFF);
  }
  else  //160 <position< 191   Fuchsia->Red (Red = FF, green = 0, blue goes FF->00)
  {
    position -= 159;
    return leds.Color(0xFF, 0x00, 0xFF - position * 8);
  }
}
