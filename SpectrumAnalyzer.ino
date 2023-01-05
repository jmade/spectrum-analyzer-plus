

int PIXEL_LUT[256] = { 255, 254, 253, 252, 251, 250, 249, 248, 240, 241, 242, 243, 244, 245, 246, 247, 239, 238, 237, 236, 235, 234, 233, 232, 224, 225, 226, 227, 228, 229, 230, 231, 223, 222, 221, 220, 219, 218, 217, 216, 208, 209, 210, 211, 212, 213, 214, 215, 207, 206, 205, 204, 203, 202, 201, 200, 192, 193, 194, 195, 196, 197, 198, 199, 191, 190, 189, 188, 187, 186, 185, 184, 176, 177, 178, 179, 180, 181, 182, 183, 175, 174, 173, 172, 171, 170, 169, 168, 160, 161, 162, 163, 164, 165, 166, 167, 159, 158, 157, 156, 155, 154, 153, 152, 144, 145, 146, 147, 148, 149, 150, 151, 143, 142, 141, 140, 139, 138, 137, 136, 128, 129, 130, 131, 132, 133, 134, 135, 127, 126, 125, 124, 123, 122, 121, 120, 112, 113, 114, 115, 116, 117, 118, 119, 111, 110, 109, 108, 107, 106, 105, 104, 96, 97, 98, 99, 100, 101, 102, 103, 95, 94, 93, 92, 91, 90, 89, 88, 80, 81, 82, 83, 84, 85, 86, 87, 79, 78, 77, 76, 75, 74, 73, 72, 64, 65, 66, 67, 68, 69, 70, 71, 63, 62, 61, 60, 59, 58, 57, 56, 48, 49, 50, 51, 52, 53, 54, 55, 47, 46, 45, 44, 43, 42, 41, 40, 32, 33, 34, 35, 36, 37, 38, 39, 31, 30, 29, 28, 27, 26, 25, 24, 16, 17, 18, 19, 20, 21, 22, 23, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7 };

/* Audio */
#define SAMPLE_SIZE 1024   // Buffer size of read samples
#define SAMPLE_RATE 44100  // Audio Sample Rate
#define BAND_SIZE 32       // powers of 2 up to 64, defaults to 8
#define BAND_HEIGHT 8

#include <AudioInI2S.h>
#include <AudioAnalysis.h>

AudioAnalysis audioInfo;
AudioInI2S mic(A2, A3, A1, -1);  // defaults to RIGHT channel.
int32_t samples[SAMPLE_SIZE];    // I2S sample data is stored here

// audio analysis setup
void audio_setup() {
  mic.begin(SAMPLE_SIZE, SAMPLE_RATE);  // Starts the I2S DMA port.

  audioInfo.setNoiseFloor(10);        // sets the noise floor
  audioInfo.normalize(true, 0, 255);  // normalize all values to range provided.

  audioInfo.autoLevel(AudioAnalysis::ACCELERATE_FALLOFF, 1, 255, 1000);  // set auto level falloff rate
  audioInfo.bandPeakFalloff(AudioAnalysis::EXPONENTIAL_FALLOFF, 0.05);   // set the band peak fall off rate
  audioInfo.vuPeakFalloff(AudioAnalysis::ACCELERATE_FALLOFF, 0.05);      // set the volume unit peak fall off rate
}


#include <ArduinoJson.h>


// Settings
bool settingsCalled = false;

// Peak Variables
bool showCustomPeakColor = false;
//uint16_t peakColor = 0;
bool showPeaks = true;
unsigned long peakHold = 200;
unsigned long peakFall = 50;
// Fire Speed
unsigned long fireFPS = 20;
// Specturm
bool peaksOnly = false;

// wave
bool showSoundWaveColor = false;
bool dynamicBrightness = true;
// Packet Parssing... (unfinished)
bool parseRawPackets = false;

bool sendPixelACK = false;

// Time/Clock
#define DEFAULT_TIME_FMT "%A, %B %d %Y %H:%M:%S"
String customTimeFMT = DEFAULT_TIME_FMT;
bool showCustomTime = false;
bool showMeridian = true;
bool blinkTime = true;
int blinkRate = 1000;




// MessageBoard
bool scrollText = false;
int scroll_ms = 40;
int scroll_start_offset = -31;
int static_offset = 0;

int textInputMode = 0;
// 0 message board; 1 time format


void resetSettings() {
  // AV
  peaksOnly = false;
  showCustomPeakColor = false;
  showPeaks = true;
  peakHold = 200;
  peakFall = 50;
  // Sound Wave
  showSoundWaveColor = false;
  dynamicBrightness = true;
  // Fire Speed
  fireFPS = 20;
  // Clock
  customTimeFMT = DEFAULT_TIME_FMT;
  showCustomTime = false;
  showMeridian = true;
  blinkTime = true;
  blinkRate = 1000;
  /// MessageBoard
  scrollText = false;
  scroll_ms = 40;
  scroll_start_offset = -31;
  static_offset = 0;
  // Misc
  textInputMode = 0;
  sendPixelACK = false;
}




/// Scrolling Text 
#define FONT_5x8_FIXED_MEDIUM
#include <StripDisplay.h>
XBMFont * fontP = &fixedMedium_5x8;
// LEDs
//#include "FastLED.h"
#define NUM_LEDS 256
#define LED_PIN RX
#define MAX_BRIGHTNESS 40
#define FRAME_RATE 120
//
CRGB leds[NUM_LEDS];
unsigned long nextFrame = 0;
unsigned long tick = 0;
//
void fast_led_setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}
int current_render_mode = 0;
String renderMode = "Off";
/*****/

/// Custom Colors
CRGB peak = CRGB(128,128,128);
int soundWaveHue = 170;




// message board globals
StripDisplay strip(LED_PIN, BAND_SIZE, BAND_HEIGHT, WRAP_COLUMNS, ORIGIN_BOTTOM_RIGHT, leds);
CRGB fg(64,64,64);
CRGB bg(0,0,0);
int update_ms = 0; // time managemet globals
int current_scroll_offset = -31;


void scrolling_text_setup() {
	strip.setup(fontP);
	strip.setFgColor(fg);
	strip.setBgColor(bg);
}

// setup new message text
void updateMessageText(String text) {
	current_scroll_offset = scroll_start_offset;
	strip.setText(text);
}

// display and scroll message
void displayMessageText() {
	if (current_scroll_offset > strip.getTextWidth())
		current_scroll_offset = scroll_start_offset;
	strip.displayText(current_scroll_offset);
	current_scroll_offset++;
}



void renderScrollingText() {
  if (scrollText) {
	  int current_ms = millis(); // mesure current time
	  // scroll and display message at a fixed time rate
	  if (update_ms <= current_ms) {
		  displayMessageText();
		  update_ms = current_ms + scroll_ms;
  	} else {
		  FastLED.delay(update_ms - current_ms);
  	}
  } else {
    strip.displayText(static_offset);
    FastLED.delay(100);
  }
}











/// WIFI
// Networking
#include <WiFi.h>
#include <WiFiUdp.h>
char ssid[] = "";  // your network SSID (name)
char pass[] = "";        // your network password (use for WPA, or use as key for WEP)

const char *udpAddress = "192.168.1.107";
const int udpPort = 2601;

IPAddress listenerIP = { 192, 168, 1, 107 };
uint16_t listenerPort = 2601;

IPAddress lastRemoteIP = { 192, 168, 1, 107 };
uint16_t lastRemotePort = 2601;

IPAddress localIP = { 192, 168, 1, 138 };
IPAddress subnetIP = { 255, 255, 255, 0 };
IPAddress gatewayIP = { 192, 168, 1, 1 };


bool connected = false;
#define PACKET_BUFFER_SIZE 64
#define MESSAGE_SIZE 64
char incomingPacket[PACKET_BUFFER_SIZE];  // buffer for incoming packets
WiFiUDP udp;


/// ESPTime

#include <ESP32Time.h>
#define GMT_OFFSET  3600 * 5 // In seconds
ESP32Time rtc(-GMT_OFFSET);  // offset in seconds GMT+1

void renderClock();





void sendPacket(String msg);
void sendListener(String msg);
void sendPacketLastRemote(String msg);
void handleMessage(String msg);
void setup_wifi();
void listenForPacket();
void printWifiStatus();


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


void fadeOut(uint8_t amount);


void processPixelPacket(String msg) {
  int r,g,b,pos,index = 0;
  for (int i = 0; i < 4; i++) {
    int found = msg.indexOf(",", index);
    switch (i)
    { 
      case 0:
        pos = msg.substring(index,found).toInt();
        break;
      case 1:
        r = msg.substring(index,found).toInt();
        break;
      case 2:
        g = msg.substring(index,found).toInt();
        break;
      case 3:
        b = msg.substring(index,found).toInt();
        break;
    }
    index = (found + 1);
  }
  leds[pos] = CRGB(r, g, b);
  FastLED.show();
}


/*
void processSequencedPixelPacket(String msg) {
  int seq,pos,r,g,b,index = 0;
  for (int i = 0; i < 5; i++) {
    int found = msg.indexOf(",", index);
    switch (i)
    { 
      case 0:
        seq = msg.substring(index,found).toInt();
        break;
      case 1:
        pos = msg.substring(index,found).toInt();
        break;
      case 2:
        r = msg.substring(index,found).toInt();
        break;
      case 3:
        g = msg.substring(index,found).toInt();
        break;
      case 4:
        b = msg.substring(index,found).toInt();
        break;
    }
    index = (found + 1);
  }

  leds[pos] = CRGB(r, g, b);
  FastLED.show();
}
*/



void processColorAssignmentPacket(String msg) {
  int r,g,b = 0;
  int assignId = 0;
  int index = 0;
  for (int i = 0; i < 4; i++) {
    int found = msg.indexOf(",", index);
    switch (i)
    { 
      case 0:
        assignId = msg.substring(index,found).toInt();
        break;
      case 1:
        r = msg.substring(index,found).toInt();
        break;
      case 2:
        g = msg.substring(index,found).toInt();
        break;
      case 3:
        b = msg.substring(index,found).toInt();
        break;
    }
    index = (found + 1);
  }

  CRGB color = CRGB(r, g, b);

  switch (assignId) {
    case 0:
      strip.setFgColor(color);
      break;
    case 1:
      strip.setBgColor(color);
      break;
    case 2:
      peak = color;
      break;
    case 3:
      break;
    case 4:
      break;
    default:
      break;
  }

}



void standardPacketParse() {
  int packetSize = udp.parsePacket();
  if (packetSize) {

    // receive incoming UDP packets
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());

    lastRemoteIP = udp.remoteIP();
    lastRemotePort = udp.remotePort();

    int len = udp.read(incomingPacket, PACKET_BUFFER_SIZE);
    if (len > 0) {
      incomingPacket[len - 1] = 0;
    }

    handleMessage(String(incomingPacket));

    if (sendPixelACK) { 
      sendPacketLastRemote("ACK");
    }

    udp.flush();
  }
}


void rawPacketParse() {
  int packetSize = udp.parsePacket();

  uint16_t header;
  uint16_t red;
  uint16_t green;
  uint16_t blue;

  if (packetSize > 7) {
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    udp.read((unsigned char*)&header, 2);
    udp.read((unsigned char*)&red, 2);
    udp.read((unsigned char*)&green, 2);
    udp.read((unsigned char*)&blue, 2);
    
    Serial.print("Header Byte: ");
    Serial.println(header);
    Serial.print("Parsed: (");
    Serial.print(red);
    Serial.print(",");
    Serial.print(green);
    Serial.print(",");
    Serial.print(blue);
    Serial.println(")");
  } else if (packetSize > 1) {
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
  }

  if (packetSize == 1) {
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    parseRawPackets = false;
    Serial.println("Back to normal packet parsing!");
  }

  udp.flush();

}



void listenForPacket() {
  if (parseRawPackets) {
    rawPacketParse();
  } else {
    standardPacketParse();
  }
}


void handleMessage(String msg) {
  //Serial.println("Message: '" + msg + "'");

  // Text
  if (msg.startsWith("@")) {
    if (textInputMode == 0) {
      updateMessageText(msg.substring(1, msg.length()));
    }

    if (textInputMode == 1) {
      customTimeFMT = msg.substring(1, msg.length());
    }
    
    return;
  }

  // Mode
  if (msg.startsWith("!")) {
    int newRenderMode = msg.substring(1, msg.length()).toInt();
    if (newRenderMode > 99 ) {
      blackOutStrip();
    }

    if (newRenderMode == 11) {
      textInputMode = 1;
      sendListener("T");
      blackOutStrip();
      scrollText = false;
      strip.setFgColor(CRGB(32,32,32));
      strip.setBgColor(CRGB(0,0,0));
    } else {
      textInputMode = 0;
    }

    current_render_mode = newRenderMode;
    return;
  }


  if (msg.startsWith("#")) {
    processPixelPacket(msg.substring(1, msg.length()));
    return;
  }

  // Color Assigning
  if (msg.startsWith("%")) {
    processColorAssignmentPacket(msg.substring(1, msg.length()));
    return;
  }

  // Commands
  if (msg.startsWith("+")) {
    decode_commands(msg.substring(1, msg.length()));
    return;
  }

  if (msg.startsWith("$")) {
    decode_commands(msg.substring(1, msg.length()));
    return;
  }

  // Settings Query
  if (msg.startsWith("?")) {
    if (settingsCalled == false) {
      settingsCalled = true;
      fillDoc();
    }
    return;
  }
}



void decode_commands(String msg) {

  char msgBuff[MESSAGE_SIZE];
  msg.toCharArray(msgBuff, MESSAGE_SIZE);
  char *strings[MESSAGE_SIZE];
  char *ptr = NULL;

  byte index = 0;
  ptr = strtok(msgBuff, "&");
  while (ptr != NULL) {
    strings[index] = ptr;
    index++;
    ptr = strtok(NULL, "&");
  }

  //Serial.println("(" + String(index) + ") Decoded Command(s)");
  for (int n = 0; n < index; n++) {
    String message = String(strings[n]);
    process_comand(message);
  }
}


void process_comand(String cmd) {
  int mid = cmd.indexOf(":");
  int command = cmd.substring(0, mid).toInt();
  int value = cmd.substring(mid+1, cmd.length()).toInt();
  Serial.println("Command: "+String(command)+" Value: "+String(value));
  switch (command) {
    case 0:  // Peaks Enabled
      showPeaks = (value == 1);
      break;
    case 1:  // Peak hold time
      peakHold = value;
      break;
    case 2:  // Peak Fall time
      peakFall = value;
      break;
    case 3:  // Show Peak line only
      peaksOnly = (value == 1);
      break;
    case 4:
      dynamicBrightness = (value == 1);
      break;
    case 5:
      soundWaveHue = value;
      break;
    case 6:
      sendPixelACK = (value == 1);
      break;
    case 7:
      showCustomPeakColor = (value == 1);
      break;
    case 8:
      parseRawPackets = (value == 1);
      break;
    case 9: // clear display
      if (value == 0) {
        blackOutStrip();
      } else {
        fadeOut(value);
      }
      break;
    case 10: // Fire
      if (value > 0) {
        fireFPS = value;
      } else {
        fireFPS = 1;
      }
      break;
    case 20:
      scroll_ms = value;
      break;
    case 21:
      scroll_start_offset = value;
      break;
    case 22: // Should Scroll
      scrollText = (value == 1);
      break;
    case 25:
      static_offset = value;
      break;
    case 30:
      rtc.offset = value;
      break;
    case 31:
      rtc.setTime(value);
      break;
    case 32:
      blinkTime = (value == 1);
      break;
    case 33:
      blinkRate = value;
      break;
    case 34:
      showCustomTime = (value == 1);
      break;
    case 35:
      showMeridian = (value == 1);
      break;
    case 50:
      textInputMode = value;
      break;
    case 51:
      showSoundWaveColor = (value == 1);
      break;
    case 100:
      resetSettings();
      break;
    default:
      break;
  }
}






void sendPacket(String msg) {
  if (connected) {
    udp.beginPacket(udpAddress, udpPort);
    udp.printf("%s", msg.c_str());
    udp.endPacket();
  }
}

void sendPacketLastRemote(String msg) {
  if (connected) {
    udp.beginPacket(lastRemoteIP, lastRemotePort);
    udp.printf("%s", msg.c_str());
    udp.endPacket();
  }
}


void sendListener(String msg) {
  if (connected) {
    udp.beginPacket(listenerIP, listenerPort);
    udp.printf("%s", msg.c_str());
    udp.endPacket();
  }
}




void setup_wifi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  
  WiFi.config(localIP,gatewayIP,subnetIP);

  String hn = "PixelBoard";
  WiFi.setHostname(hn.c_str());

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to WiFi");
  printWifiStatus();

  udp.begin(WiFi.localIP(), 8888);
  connected = true;
}



void setup() {
  Serial.begin(115200);
  setup_wifi();
  audio_setup();
  fast_led_setup();
  scrolling_text_setup();
  sendListener("T");
}

void loop() {
  if (connected) {
    listenForPacket();
  }
  av_loop();
}




void av_loop() {
  if (current_render_mode > 100) {
    av_render(current_render_mode);
  } else {
    if (nextFrame > millis()) {
      return;
    }
    // enforce a predictable frame rate
    nextFrame = millis() + (1000 / FRAME_RATE);
    tick++;

    mic.read(samples);  // Stores the current I2S port buffer into samples.
    audioInfo.computeFFT(samples, SAMPLE_SIZE, SAMPLE_RATE);
    audioInfo.computeFrequencies(BAND_SIZE);

    av_render(current_render_mode);
  }
}






void av_render(int mode) {
  switch (mode) {
    case 0:  // OFF
      renderMode = F("Off");
      fadeOut(16);
      break;
    case 1:
      renderMode = F("Rainbow");
      renderBasicTest(true);  // Rainbow
      break;
    case 2:
      renderMode = F("Classic AV");
      renderBasicTest(false);  // Normal
      break;
    case 3:
      renderMode = F("Sound Wave");
      renderWave();
      break;
    case 10:
      renderMode = F("Beat Rainbow");
      renderBeatRainbow();
      break;
    case 11:
      renderMode = F("Clock");
      renderClock();
      break;
    case 91:
      renderMode = F("Serial");
      renderSerial();
      break;
    case 100:
      renderMode = F("Fire");
      renderMatrixFire();
      break;
    case 101:
      renderMode = F("Pixel Paint");
      renderPixelPaint();
      break;
    case 102:
      renderMode = F("Message Board");
      renderScrollingText();
      break;
    default:
      break;
  }
}


String standardTimeFMT(bool toggle) {
  String fmt = toggle ? "%I:%M" : "%I %M";
  if (showMeridian) {
    fmt += "%p";
  }
  return fmt;
}

void renderClock() {
  static bool toggle = false;

  if (showCustomTime) {
    strip.setText(rtc.getTime(customTimeFMT));
  } else {
    if (blinkTime) { 
      strip.setText(rtc.getTime(standardTimeFMT(toggle)));
    } else {
      strip.setText(rtc.getTime(standardTimeFMT(true)));
    }
  }
  
  if (scrollText) {
	  int current_ms = millis(); // mesure current time
	  // scroll and display message at a fixed time rate
	  if (update_ms <= current_ms) {
		  displayMessageText();
		  update_ms = current_ms + scroll_ms;
  	} else {
		  FastLED.delay(update_ms - current_ms);
  	}
  } else {
    strip.displayText(static_offset);
    if (blinkTime) {
      FastLED.delay(blinkRate);
    } else {
      FastLED.delay(500);
    }
  }

  toggle = !toggle;
}






// Peaks

int peaks[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned long peakDecays[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned long peakBottom[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
bool peakState[32] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };

void processPeak(uint8_t newPeak, int band);
/*****/

// 4 and 5 is mid line
//   3,4, 5,6
// 2,3,4  5,6,7
// 0,1,2,3,4,5,6,7


int offsets[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };



// uint32_t Wheel(byte WheelPos) {
//   WheelPos = 255 - WheelPos;
//   if (WheelPos < 85) {
//     return sspixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
//   }
//   if (WheelPos < 170) {
//     WheelPos -= 85;
//     return sspixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
//   }
//   WheelPos -= 170;
//   return sspixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
// }


void renderWave() {

  float *peaks = audioInfo.getPeaks();

  int pixel_index = 0;
  // Go through each band.
  for (int b = 0; b < BAND_SIZE; b++) {

    
    int hue = b * (200 / BAND_SIZE);

    int band_peak = (int)map((int)peaks[b], 0, 255, 0, BAND_HEIGHT);
    int offset = (BAND_HEIGHT - band_peak)/2;
    int high_offset = BAND_HEIGHT - offset;
    int band_bright = (int)peaks[b];

    int pixelColor = hue;
    if (showSoundWaveColor) {
      pixelColor = soundWaveHue;
    }

    int brightness = 255;
    if (dynamicBrightness) {
      brightness = band_bright;
    }

    for (int i = 0; i < BAND_HEIGHT; i++) {
      if (band_peak == 0) {
        leds[PIXEL_LUT[pixel_index]] = CRGB(0, 0, 0);
      } else {
        if (band_peak == 1) {
          if (i == 4) {
            leds[PIXEL_LUT[pixel_index]] = CHSV(pixelColor, 255, brightness);
          } else {
            leds[PIXEL_LUT[pixel_index]] = CRGB(0, 0, 0);
          }
        } else {
          if (i >= offset && i <= high_offset) {
            leds[PIXEL_LUT[pixel_index]] = CHSV(pixelColor, 255, brightness);
          } else {
            leds[PIXEL_LUT[pixel_index]] = CRGB(0, 0, 0);
          }
        }
      }
      pixel_index += 1;  // step through the pixels...
    }
  }

  FastLED.show();
}


void renderBasicTest(bool spectrum) {
  float *peaks = audioInfo.getPeaks();

  int pixel_index = 0;
  for (int b = 0; b < BAND_SIZE; b++) {

    int hue = b * (200 / BAND_SIZE);
    int band_threshold = BAND_HEIGHT - ((BAND_HEIGHT * 0.2) + 1.0);

    int band_peak = (int)map((int)peaks[b], 0, 255, 0, BAND_HEIGHT);

    for (int i = 0; i < BAND_HEIGHT; i++) {
      if (peaksOnly == false) {
        if (i < (band_peak)) {
          if (spectrum) {
            leds[PIXEL_LUT[pixel_index]] = CHSV(hue, 255, 255);
          } else {
            if (i == BAND_HEIGHT - 1) {
              leds[PIXEL_LUT[pixel_index]] = CRGB(255, 0, 0);
            } else if (i < band_threshold) {
              leds[PIXEL_LUT[pixel_index]] = CRGB(0, 255, 0);
            } else {
              leds[PIXEL_LUT[pixel_index]] = CRGB(255, 254, 0);
            }
          }
        } else {
          leds[PIXEL_LUT[pixel_index]] = CRGB(0, 0, 0);
        }
      } else {
        leds[PIXEL_LUT[pixel_index]] = CRGB(0, 0, 0);
      }
      pixel_index += 1;  // step through the pixels...
    }

    if (showPeaks) {
      processPeak(band_peak, b);
    }
  }

  FastLED.show();
}



void processPeak(uint8_t newPeak, int band) {

  int currentPeak = peaks[band];
  bool decaying = peakState[band];
  unsigned long previousDecay = peakDecays[band];

  // Compute Peak State
  if (newPeak >= currentPeak) {
    peaks[band] = newPeak;
    peakDecays[band] = millis();
    peakState[band] = false;
  } else if (!decaying && (millis() - previousDecay > peakHold)) {
    peakState[band] = true;
    peakDecays[band] += peakHold - peakFall;
  } else if (decaying && (millis() - previousDecay > peakFall)) {
    if (currentPeak > 0) {
      peaks[band] -= 1;
      peakDecays[band] += peakFall;
    }
  }

  int pixelPos = (band * BAND_HEIGHT);
  // Paint Peak
  if (peaks[band] > 0) {
    int peakPos = peaks[band] + pixelPos;
    if (showCustomPeakColor == false) {
      leds[PIXEL_LUT[peakPos]] = CRGB(50, 50, 50);
    } else {
      leds[PIXEL_LUT[peakPos]] = peak;
    }
    peakBottom[band] = 0;
  } else {
    if (currentPeak == 1 && peaks[band] == 0) {
      peakBottom[band] = millis();
      if (showCustomPeakColor == false) {
        leds[PIXEL_LUT[pixelPos]] = CRGB(50, 50, 50);
      } else {
        leds[PIXEL_LUT[pixelPos]] = peak;
      }
    } else if (peaks[band] == 0) {
      if (millis() - peakBottom[band] > peakFall) {
        leds[PIXEL_LUT[pixelPos]] = CRGB(0, 0, 0);
      }
    }
  }
}








void renderBeatRainbow() {
  float *bands = audioInfo.getBands();
  float *peaks = audioInfo.getPeaks();
  int peakBandIndex = audioInfo.getBandMaxIndex();
  static int beatCount = 0;

  bool beatDetected = false;
  bool clapsDetected = false;
  // beat detection
  if (peaks[1] == bands[1])  // new peak for bass must be a beat
  {
    beatCount++;
    beatDetected = true;
  }
  if (peakBandIndex >= BAND_SIZE / 2) {
    clapsDetected = true;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = blend(leds[i], CRGB(0, 0, 0), 100);  // fade to black over time

    // bass/beat = rainbow
    if (beatDetected) {
      if (random(0, 10 - ((float)peaks[1] / (float)255 * 10.0)) == 0) {
        leds[i] = CHSV((beatCount * 10) % 255, 255, 255);
      }
    }

    // claps/highs = white twinkles
    if (clapsDetected) {
      if (random(0, 40 - ((float)peakBandIndex / (float)BAND_SIZE * 10.0)) == 0) {
        leds[i] = CRGB(255, 255, 255);
      }
    }
  }
  FastLED.show();
}









/// Utility ///


/// fadeTowardColor


// Fade helpers

// Helper function that blends one uint8_t toward another by a given amount
void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount)
{
  if( cur == target) return;
  
  if( cur < target ) {
    uint8_t delta = target - cur;
    delta = scale8_video( delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video( delta, amount);
    cur -= delta;
  }
}


void fadeOut(uint8_t amount) {
  CRGB target = CRGB(0, 0, 0);
  for (int i = 0; i < NUM_LEDS; i++) {
    CRGB cur = leds[i];
    nblendU8TowardU8( cur.red,   target.red,   amount);
    nblendU8TowardU8( cur.green, target.green, amount);
    nblendU8TowardU8( cur.blue,  target.blue,  amount);
    leds[i] = cur;
  }
  FastLED.show();
}


void blackOutStrip() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}




void renderSerial() {
  float *peaks = audioInfo.getPeaks();
  for (int i = 0; i < BAND_SIZE; i++) {
    Serial.printf("%d\t", int(peaks[i]));
  }
  Serial.println();
}



void fillDoc() {
  
  StaticJsonDocument<786> doc;
  JsonObject root = doc.to<JsonObject>();

  root[F("renderMode")] = renderMode;
  root[F("fireFPS")] = fireFPS;
  root[F("sendPixelACK")] = sendPixelACK;
  root[F("textInputMode")] = textInputMode;

  JsonObject peakData = root.createNestedObject("peaks");  
  peakData[F("showPeaks")] = showPeaks;
  peakData[F("peaksOnly")] = peaksOnly;
  peakData[F("peakHold")] = peakHold;
  peakData[F("peakFall")] = peakFall;
  peakData[F("showCustomPeakColor")] = showCustomPeakColor;

  JsonObject peakColorData = peakData.createNestedObject("color");
  peakColorData[F("r")] = peak.red;
  peakColorData[F("g")] = peak.green;
  peakColorData[F("b")] = peak.blue;

  JsonObject soundWaveData = root.createNestedObject("soundWave");
  soundWaveData[F("dynamicBrightness")] = dynamicBrightness;
  soundWaveData[F("soundWaveHue")] = soundWaveHue;
  soundWaveData[F("showSoundWaveColor")] = showSoundWaveColor;

  JsonObject messageBoardObj = root.createNestedObject("messageBoard");
  messageBoardObj[F("scrollSpeed")] = scroll_ms;
  messageBoardObj[F("scrollOffset")] = scroll_start_offset;
  messageBoardObj[F("staticOffset")] = static_offset;
  messageBoardObj[F("scrollEnabled")] = scrollText;

  
  JsonObject clockObj = root.createNestedObject("clock");
  clockObj[F("blinkTime")] = blinkTime;
  clockObj[F("showCustomTime")] = showCustomTime;
  clockObj[F("blinkRate")] = blinkRate;
  clockObj[F("showMeridian")] = showMeridian;
  

  if (connected) {
    udp.beginPacket(lastRemoteIP, lastRemotePort);
    serializeJson(doc, udp);
    udp.println();
    udp.endPacket();
    settingsCalled = false;
  }

  Serial.println(doc.memoryUsage()); // 35
  doc.garbageCollect();
  Serial.println(doc.memoryUsage()); // 21

}


String deviceReport() {
  return String("{") +
  String(" \"renderMode\" :") + String(renderMode) + String(",")
  + String(" \"fireFPS\" :") + String(fireFPS) + String(",")
  + String("}");
}



/// Matrix Fire
#define VERSION 20275

/* MATRIX CONFIGURATION -- PLEASE SEE THE README (GITHUB LINK ABOVE) */

#define MAT_W 32 /* Size (columns) of entire matrix */
#define MAT_H 8  /* and rows */

#define MAT_COL_MAJOR /* define if matrix is column-major (that is pixel 1 is in the same column as pixel 0) */
#define MAT_TOP       /* define if matrix 0,0 is in top row of display; undef if bottom */
#undef MAT_LEFT       /* define if matrix 0,0 is on left edge of display; undef if right */
#define MAT_ZIGZAG    /* define if matrix zig-zags ---> <--- ---> <---; undef if scanning ---> ---> ---> */

/* MULTI-PANEL CONFIGURATION -- Do not change unless you connect multiple panels -- See README.md */
/* WARNING -- THIS IS CURRENTLY UNTESTED -- DO NOT ENABLE UNLESS YOU FEEL LIKE BEING MY CRASH TEST MANNEQUIN */
#undef MULTIPANEL   /* define to enable multi-panel support */
#define PANELS_W 1  /* Number of panels wide */
#define PANELS_H 1  /* Number of panels tall */
#undef PANEL_TOP    /* define if first panel is upper-left */
#undef PANEL_ZIGZAG /* define if panels zig-zag */
/* --- DO NOT CHANGE THESE LINES --- */
#ifndef MULTIPANEL
#define PANELS_H 1
#define PANELS_W 1
#undef PANEL_TOP
#undef PANEL_ZIGZAG
#endif


/* SECONDARY CONFIGURATION */
/* Display size; can be smaller than matrix size, and if so, you can move the origin.
 * This allows you to have a small fire display on a large matrix sharing the display
 * with other stuff. See README at Github. */
const uint16_t rows = MAT_H * PANELS_H;
const uint16_t cols = MAT_W * PANELS_W;
const uint16_t xorg = 0;
const uint16_t yorg = 0;

/* Flare constants */
const uint8_t flarerows = 2;    /* number of rows (from bottom) allowed to flare */
const uint8_t maxflare = 8;     /* max number of simultaneous flares */
const uint8_t flarechance = 50; /* chance (%) of a new flare (if there's room) */
const uint8_t flaredecay = 14;  /* decay rate of flare radiation; 14 is good */

/* This is the map of colors from coolest (black) to hottest. Want blue flames? Go for it! */
const uint32_t colors[] = {
  0x000000,
  0x100000,
  0x300000,
  0x600000,
  0x800000,
  0xA00000,
  0xC02000,
  0xC04000,
  0xC06000,
  0xC08000,
  0x807080
};
const uint8_t NCOLORS = (sizeof(colors) / sizeof(colors[0]));

uint8_t pix[rows][cols];
//CRGB matrix[MAT_H * PANELS_H * MAT_W * PANELS_W];
uint8_t nflare = 0;
uint32_t flare[maxflare];

/** pos - convert col/row to pixel position index. This takes into account
 *  the serpentine display, and mirroring the display so that 0,0 is the
 *  bottom left corner and (MAT_W-1,MAT_H-1) is upper right. You may need
 *  to jockey this around if your display is different.
 */
#ifndef MAT_LEFT
#define __MAT_RIGHT
#endif
#ifndef MAT_TOP
#define __MAT_BOTTOM
#endif
#if defined(MAT_COL_MAJOR)
const uint8_t phy_h = MAT_W;
const uint8_t phy_w = MAT_H;
#else
const uint8_t phy_h = MAT_H;
const uint8_t phy_w = MAT_W;
#endif
#if defined(MULTIPANEL)
uint16_t _pos(uint16_t col, uint16_t row) {
#else
uint16_t pos(uint16_t col, uint16_t row) {
#endif
#if defined(MAT_COL_MAJOR)
  uint16_t phy_x = xorg + (uint16_t)row;
  uint16_t phy_y = yorg + (uint16_t)col;
#else
  uint16_t phy_x = xorg + (uint16_t)col;
  uint16_t phy_y = yorg + (uint16_t)row;
#endif
#if defined(MAT_LEFT) && defined(MAT_ZIGZAG)
  if ((phy_y & 1) == 1) {
    phy_x = phy_w - phy_x - 1;
  }
#elif defined(__MAT_RIGHT) && defined(MAT_ZIGZAG)
  if ((phy_y & 1) == 0) {
    phy_x = phy_w - phy_x - 1;
  }
#elif defined(__MAT_RIGHT)
phy_x = phy_w - phy_x - 1;
#endif
#if defined(MAT_TOP) and defined(MAT_COL_MAJOR)
  phy_x = phy_w - phy_x - 1;
#elif defined(MAT_TOP)
  phy_y = phy_h - phy_y - 1;
#endif
  return phy_x + phy_y * phy_w;
}

#if defined(MULTIPANEL)
uint16_t pos(uint16_t col, uint16_t row) {
#if defined(PANEL_TOP)
  uint16_t panel_y = PANELS_H - (row / MAT_H) - 1;
#else
  uint16_t panel_y = row / MAT_H;
#endif
  uint16_t panel_x = col / MAT_W;
#if defined(PANEL_ZIGZAG)
  if ((panel_y & 1) == 1) {
    panel_x = PANELS_W - panel_x - 1;
  }
#endif
  uint16_t pindex = panel_x + panel_y * PANELS_W;
  return MAT_W * MAT_H * pindex + _pos(col % MAT_W, row % MAT_H);
}
#endif

uint32_t isqrt(uint32_t n) {
  if (n < 2) return n;
  uint32_t smallCandidate = isqrt(n >> 2) << 1;
  uint32_t largeCandidate = smallCandidate + 1;
  return (largeCandidate * largeCandidate > n) ? smallCandidate : largeCandidate;
}

// Set pixels to intensity around flare
void glow(int x, int y, int z) {
  int b = z * 10 / flaredecay + 1;
  for (int i = (y - b); i < (y + b); ++i) {
    for (int j = (x - b); j < (x + b); ++j) {
      if (i >= 0 && j >= 0 && i < rows && j < cols) {
        int d = (flaredecay * isqrt((x - j) * (x - j) + (y - i) * (y - i)) + 5) / 10;
        uint8_t n = 0;
        if (z > d) n = z - d;
        if (n > pix[i][j]) {  // can only get brighter
          pix[i][j] = n;
        }
      }
    }
  }
}

void newflare() {
  if (nflare < maxflare && random(1, 101) <= flarechance) {
    int x = random(0, cols);
    int y = random(0, flarerows);
    int z = NCOLORS - 1;
    flare[nflare++] = (z << 16) | (y << 8) | (x & 0xff);
    glow(x, y, z);
  }
}

/** make_fire() animates the fire display. It should be called from the
 *  loop periodically (at least as often as is required to maintain the
 *  configured refresh rate). Better to call it too often than not enough.
 *  It will not refresh faster than the configured rate. But if you don't
 *  call it frequently enough, the refresh rate may be lower than
 *  configured.
 */
unsigned long t = 0; /* keep time */
void make_fire() {
  uint16_t i, j;
  if (t > millis()) return;
  t = millis() + (1000 / fireFPS);

  // First, move all existing heat points up the display and fade
  for (i = rows - 1; i > 0; --i) {
    for (j = 0; j < cols; ++j) {
      uint8_t n = 0;
      if (pix[i - 1][j] > 0)
        n = pix[i - 1][j] - 1;
      pix[i][j] = n;
    }
  }

  // Heat the bottom row
  for (j = 0; j < cols; ++j) {
    i = pix[0][j];
    if (i > 0) {
      pix[0][j] = random(NCOLORS - 6, NCOLORS - 2);
    }
  }

  // flare
  for (i = 0; i < nflare; ++i) {
    int x = flare[i] & 0xff;
    int y = (flare[i] >> 8) & 0xff;
    int z = (flare[i] >> 16) & 0xff;
    glow(x, y, z);
    if (z > 1) {
      flare[i] = (flare[i] & 0xffff) | ((z - 1) << 16);
    } else {
      // This flare is out
      for (int j = i + 1; j < nflare; ++j) {
        flare[j - 1] = flare[j];
      }
      --nflare;
    }
  }
  newflare();

  // Set and draw
  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      leds[pos(j, i)] = colors[pix[i][j]];
    }
  }
  FastLED.show();
}



void renderMatrixFire() {
  make_fire();
}

///// End Matrix Fire



void renderPixelPaint() {

}





