#include "WiFi.h"
#define SCAN_COUNT_SLEEP 5
// Uncomment this option if using PNP transistor control LCD power
//#define PNP_PWR_TRANSISTOR

#if defined(PNP_PWR_TRANSISTOR)
#define LCD_PWR_PIN 2 // D2
#else
#define LCD_PWR_PIN 4 // D2
#define LED_PWR_PIN 2 // D4
#endif

#include <TFT_eSPI.h> // Graphics and font library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  
TFT_eSprite spr = TFT_eSprite(&tft);

#define TFT_MOSI 12
#define TFT_CLK 13
#define TFT_RST 2
#define TFT_MISO 3

#define TFT_DC 14
#define TFT_CS 0
// Use hardware SPI and the above for CS/DC
#define TFT_MOSI 12
#define TFT_CLK 13
#define TFT_RST 2
#define TFT_MISO 3
// If using the breakout, change pins as desired


// Graph constant

#define WIDTH 240
#define HEIGHT 135


#define GRAPH_BASELINE (HEIGHT - 18)
#define GRAPH_HEIGHT (HEIGHT - 52)
#define CHANNEL_WIDTH (WIDTH / 15)

// RSSI RANGE
#define RSSI_CEILING -40
#define RSSI_FLOOR -100
#define NEAR_CHANNEL_RSSI_ALLOW -70

// define color



// Channel color mapping from channel 1 to 14
uint16_t channel_color[] = {
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_MAGENTA,
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_MAGENTA,
  TFT_RED, TFT_ORANGE
};

uint8_t scan_count = 0;

void setup() {
#if defined(PNP_PWR_TRANSISTOR)
  pinMode(LCD_PWR_PIN, OUTPUT);   // sets the pin as output
  digitalWrite(LCD_PWR_PIN, LOW); // PNP transistor on
#else
  pinMode(LCD_PWR_PIN, OUTPUT);   // sets the pin as output
  pinMode(LED_PWR_PIN, OUTPUT);   // sets the pin as output
  digitalWrite(LCD_PWR_PIN, HIGH); // power on
  digitalWrite(LED_PWR_PIN, HIGH); // power on
#endif
  
  tft.begin();
  tft.setRotation(1);
  
// Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // rest for WiFi routine?
  delay(100);
  spr.createSprite(WIDTH, HEIGHT);
}

void loop() {
  uint8_t ap_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int32_t max_rssi[] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();

  // clear old graph
  spr.fillRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);
  spr.setTextSize(0);
  
  if (n == 0) {
    spr.setTextColor(TFT_BLACK);
    spr.setCursor(0, 0);
    spr.println("no networks found");
  } else {
    // plot found WiFi info
    for (int i = 0; i < n; i++) {
      int32_t channel = WiFi.channel(i);
      int32_t rssi = WiFi.RSSI(i);
      uint16_t color = channel_color[channel - 1];
      int height = constrain(map(rssi, RSSI_FLOOR, RSSI_CEILING, 1, GRAPH_HEIGHT), 1, GRAPH_HEIGHT);

      // channel stat
      ap_count[channel - 1]++;
      if (rssi > max_rssi[channel - 1]) {
        max_rssi[channel - 1] = rssi;
      }

      spr.drawLine(channel * CHANNEL_WIDTH, GRAPH_BASELINE - height, (channel - 1) * CHANNEL_WIDTH, GRAPH_BASELINE + 1, color);
      spr.drawLine(channel * CHANNEL_WIDTH, GRAPH_BASELINE - height, (channel + 1) * CHANNEL_WIDTH, GRAPH_BASELINE + 1, color);
      
      // Print SSID, signal strengh and if not encrypted
      spr.setTextColor(color);
      spr.setCursor((channel - 1) * CHANNEL_WIDTH, GRAPH_BASELINE - 10 - height);
      spr.print(WiFi.SSID(i));
      spr.print('(');
      spr.print(rssi);
      spr.print(')');
      //if (WiFi.encryptionType(i) == ENC_TYPE_NONE) {
      //  tft.print('*');
      //}

      // rest for WiFi routine?
      delay(10);
    }
  }

  // print WiFi stat
  spr.pushSprite(0, 0);
  spr.setTextColor(TFT_WHITE);
  spr.setCursor(0, 0);
  spr.print(n);
  spr.print(" networks found, suggested channels: ");
  spr.println();
  bool listed_first_channel = false;
  for (int i = 1; i <= 11; i++) { // channels 12-14 may not available
    if ((i == 1) || (max_rssi[i - 2] < NEAR_CHANNEL_RSSI_ALLOW)) { // check previous channel signal strengh
      if ((i == sizeof(channel_color)) || (max_rssi[i] < NEAR_CHANNEL_RSSI_ALLOW)) { // check next channel signal strengh
        if (ap_count[i - 1] == 0) { // check no AP exists in same channel
          if (!listed_first_channel) {
            listed_first_channel = true;
          } else {
            spr.print(" ");
          }
          spr.print(i);
        }
      }
    }
  }

  // draw graph base axle
  spr.drawFastHLine(0, GRAPH_BASELINE, WIDTH, TFT_WHITE);
  for (int i = 1; i <= 14; i++) {
    spr.setTextColor(channel_color[i - 1]);
    spr.setCursor((i * CHANNEL_WIDTH) - ((i < 10)?3:6), GRAPH_BASELINE + 2);
    spr.print(i);
    if (ap_count[i - 1] > 0) {
      spr.setCursor((i * CHANNEL_WIDTH) - ((ap_count[i - 1] < 10)?9:12), GRAPH_BASELINE + 11);
      spr.print('[');
      spr.print(ap_count[i - 1]);
      spr.print(']');
      
      
      
      
    }
  }

  spr.pushSprite(0, 0);
  delay(5000); // Wait a bit before scanning again

}
