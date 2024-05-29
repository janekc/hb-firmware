#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <menu.h>
#include <menuIO/SSD1306AsciiOut.h>
#include <menuIO/serialIO.h>
#include <plugin/SDMenu.h>
#include <AceButton.h>
#include <menuIO/serialIn.h>
#include "config.h"

using namespace ace_button;
using namespace Menu;

AceButton buttonUp(BUTTON_UP_PIN);
AceButton buttonLeft(BUTTON_LEFT_PIN);
AceButton buttonEnter(BUTTON_ENTER_PIN);
AceButton buttonRight(BUTTON_RIGHT_PIN);
AceButton buttonDown(BUTTON_DOWN_PIN);

SSD1306AsciiWire oled;

// default led state
int ledZeroState = LOW;
int ledOneState = LOW;

constexpr int menuFPS=25;
unsigned long nextPool=0;

//function to handle file select
// declared here and implemented bellow because we need
// to give it as event handler for `filePickMenu`
// and we also need to refer to `filePickMenu` inside the function
result filePick(eventMask event, navNode& nav, prompt &item);

// SDMenu filePickMenu("SD Card","/",filePick,enterEvent);
//caching 32 file entries
CachedSDMenu<32> filePickMenu("SD Card","/",filePick,enterEvent);

//customizing a prompt look!
//by extending the prompt class
class altPrompt: public prompt {
  public:
    altPrompt(constMEM promptShadow& p): prompt(p) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t) override {
      return out.printRaw(F("special prompt!"), len);
    }
};

result ledZeroOn() {
  ledZeroState = HIGH;
  analogWrite(LED_ONE_PIN, 255);
  return proceed;
}

result ledZeroOff() {
  ledZeroState = LOW;
  analogWrite(LED_ONE_PIN, 0);
  return proceed;
}

result ledOneOn() {
  ledOneState = HIGH;
  digitalWrite(LED_TWO_PIN, HIGH);
  return proceed;
}

result ledOneOff() {
  ledOneState = LOW;
  analogWrite(LED_TWO_PIN, LOW);
  return proceed;
}

TOGGLE(ledZeroState, setLed0, "Led 0: ", doNothing, noEvent, noStyle //,doExit,enterEvent,noStyle
       , VALUE("On", HIGH, ledZeroOn, enterEvent)//ledZeroOn function is called
       , VALUE("Off", LOW, ledZeroOff, enterEvent)//ledZeroOff function is called
      );

TOGGLE(ledOneState, setLed1, "Led 1: ", doNothing, noEvent, noStyle //,doExit,enterEvent,noStyle
       , VALUE("On", HIGH, ledOneOn, enterEvent)//ledZeroOn function is called
       , VALUE("Off", LOW, ledOneOff, enterEvent)//ledZeroOff function is called
      );

// Define the settings submenu
MENU(settingsMenu, "Settings", doNothing, noEvent, noStyle
     , EXIT("<Back")
     , OP("Settings 1", doSettings1, enterEvent)
     , OP("Settings 2", doSettings2, enterEvent)
     , OP("Settings 3", doSettings3, enterEvent)
     , OP("Settings 4", doSettings4, enterEvent)
     , SUBMENU(setLed0)
     , SUBMENU(setLed1)
    );

// Modify the main menu to include the new items
MENU(mainMenu, "HackBat 1.0", doNothing, noEvent, wrapStyle
     , SUBMENU(filePickMenu)
     , OP("BadUSB", doBadUSB, enterEvent)
     , OP("NFC", doNFC, enterEvent)
     , OP("SubGHz", doSubGHz, enterEvent)
     , OP("WiFi", doNothing, enterEvent)
     , SUBMENU(settingsMenu)
    );

//define at least one panel for menu output
const panel panels[] MEMMODE = {{0, 0, 128 / fontW, 64 / fontH}};
navNode* nodes[sizeof(panels) / sizeof(panel)]; //navNodes to store navigation status
panelsList pList(panels, nodes, 1); //a list of panels and nodes
idx_t tops[MAX_DEPTH] = {0, 0}; //store cursor positions for each level

#ifdef LARGE_FONT
SSD1306AsciiOut outOLED(&oled, tops, pList, 8, 2); //oled output device menu driver

#else
SSD1306AsciiOut outOLED(&oled, tops, pList, 5, 1); //oled output device menu driver
#endif

menuOut* constMEM outputs[]  MEMMODE  = {&outOLED}; //list of output devices
outputsList out(outputs, 1); //outputs list

serialIn serial(Serial);
NAVROOT(nav,mainMenu,MAX_DEPTH,serial,out);

// Define the actions for the new menu items
result doArchive(eventMask e, prompt &item) {
  // Add code to handle the "archive" action
  nav.idleOn(alert);
  return proceed;
}

result doBadUSB(eventMask e, prompt &item) {
  // Add code to handle the "badusb" action
  nav.idleOn(alert);
  return proceed;
}

result doNFC(eventMask e, prompt &item) {
  // Add code to handle the "nfc" action
  nav.idleOn(alert);
  return proceed;
}

result doSubGHz(eventMask e, prompt &item) {
  // Add code to handle the "subghz" action
  // For example, you can call a function that runs the code in CC1101_Receive_Interrupt.ino
  nav.idleOn(alert);
  return proceed;
}

result doSettings1(eventMask e, prompt &item) {
  // Add code to handle the first "settings" action
  nav.idleOn(alert);
  return proceed;
}

result doSettings2(eventMask e, prompt &item) {
  // Add code to handle the second "settings" action
  nav.idleOn(alert);
  return proceed;
}

result doSettings3(eventMask e, prompt &item) {
  // Add code to handle the third "settings" action
  nav.idleOn(alert);
  return proceed;
}

result doSettings4(eventMask e, prompt &item) {
  // Add code to handle the fourth "settings" action
  return proceed;
}

result alert(menuOut& o, idleEvent e) {
  if (e == idling) {
    o.setCursor(0, 0);
    o.print("Not implementes yet!");
    o.setCursor(0, 1);
    o.print("press [select]");
    o.setCursor(0, 2);
    o.print("to continue...");
  }
  return proceed;
}

//when menu is suspended
result idle(menuOut &o, idleEvent e) {
  o.clear();
  switch (e) {
    case idleStart: o.println("suspending menu!"); break;
    case idling: o.println("suspended..."); break;
    case idleEnd: o.println("resuming menu."); break;
  }
  return proceed;
}

//implementing the handler here after filePick is defined...
result filePick(eventMask event, navNode& nav, prompt &item) {
  // switch(event) {//for now events are filtered only for enter, so we dont need this checking
  //   case enterCmd:
      if (nav.root->navFocus==(navTarget*)&filePickMenu) {
        Serial.println();
        Serial.print("selected file:");
        Serial.println(filePickMenu.selectedFile);
        Serial.print("from folder:");
        Serial.println(filePickMenu.selectedFolder);
      }
  //     break;
  // }
  return proceed;
}

void buttonHandler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println(button->getPin());
      switch (button->getPin()) {
        case BUTTON_UP_PIN:
          nav.doNav(navCmd(downCmd));
          break;
        case BUTTON_DOWN_PIN:
          nav.doNav(navCmd(upCmd));
          break;
        case BUTTON_ENTER_PIN:
          nav.doNav(navCmd(enterCmd));
          break;
        case BUTTON_LEFT_PIN:
          nav.doNav(navCmd(escCmd));
          break;
      }
    break;
  }
}

void setupSerial() {
  Serial.begin(9600);
  while (!Serial);
}

void setupSDCard() {
  Serial.print("Initializing SD card...");
  SPI1.setRX(SD_MISO);
  SPI1.setTX(SD_MOSI);
  SPI1.setSCK(SD_SCK);
  if(!SD.begin(SD_CS, SPI1)) {
    Serial.println(F("ERROR: In SPI Configuration"));
    while(1);
  }
  filePickMenu.begin(); //need this after sd begin
  Serial.println("initialization done.");
}

void setupNav() {
  nav.useAccel=false;
  nav.idleTask = idle; //point a function to be used when menu is suspended
}

void setupLEDs() {
  pinMode(LED_ONE_PIN, OUTPUT);
  pinMode(LED_TWO_PIN, OUTPUT);
}

void setupButtons() {
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ENTER_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  buttonUp.setEventHandler(buttonHandler);
  buttonLeft.setEventHandler(buttonHandler);
  buttonEnter.setEventHandler(buttonHandler);
  buttonRight.setEventHandler(buttonHandler);
  buttonDown.setEventHandler(buttonHandler);
}

void setupOLED() {
  Wire.begin();
  oled.begin(&Adafruit128x64, OLED_I2C_ADDRESS); //check config
  oled.setFont(menuFont);
  oled.clear();
}

void setup() {
  setupSerial();
  setupSDCard();
  setupNav();
  setupLEDs();
  setupButtons();
  setupOLED();
}

void loop() {
  unsigned long now=millis();
  if(now>=nextPool) {
    buttonDown.check();
    buttonUp.check();
    buttonEnter.check();
    buttonLeft.check();
    nav.poll();
    nextPool=now+1000/menuFPS;
  }
}
