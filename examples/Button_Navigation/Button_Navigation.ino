#include <Arduino.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <menu.h>
#include <menuIO/SSD1306AsciiOut.h>
#include <AceButton.h>
#include <menuIO/serialIn.h>

using namespace ace_button;
using namespace Menu;

const int buttonUpPin = 22;
const int buttonLeftPin = 26;
const int buttonEnterPin = 27;
const int buttonRightPin = 28;
const int buttonDownPin = 29;

AceButton buttonUp(buttonUpPin);
AceButton buttonLeft(buttonLeftPin);
AceButton buttonEnter(buttonEnterPin);
AceButton buttonRight(buttonRightPin);
AceButton buttonDown(buttonDownPin);

SSD1306AsciiWire oled;

#include "config.h"

int ledCtrl = HIGH;  //Default LED State of LED at D11 is LOW

result doAlert(eventMask e, prompt &item);

result showEvent(eventMask e, navNode& nav, prompt& item) {
  return proceed;
}

result action1(eventMask e) {
  return proceed;
}

result action2(eventMask e, navNode& nav, prompt &item) {
  //Serial.print(e);
  //Serial.println(" action2 executed, quiting menu");
  return quit;
}

result ledOn() {
  ledCtrl = HIGH;
  analogWrite(LED_PIN, 255);
  return proceed;
}

result ledOff() {
  ledCtrl = LOW;
  analogWrite(LED_PIN, 0);
  return proceed;
}

result internalLedOn() {
  digitalWrite(LED_BUILTIN, HIGH);
  return proceed;
}

result internalLedOff() {
  analogWrite(LED_BUILTIN, LOW);
  return proceed;
}

int brightnessValue = 15;   //Default LED brightness value
result adjustBrightness() {
  if (ledCtrl == HIGH) {
    int pwm = int(2.55 * float(brightnessValue));
    analogWrite(LED_PIN, pwm);
  }
  return proceed;
}

TOGGLE(ledCtrl, setLed, "Led: ", doNothing, noEvent, noStyle //,doExit,enterEvent,noStyle
       , VALUE("On", HIGH, ledOn, enterEvent)//ledOn function is called
       , VALUE("Off", LOW, ledOff, enterEvent)//ledOff function is called
      );

int selTest = 0;
SELECT(selTest, selMenu, "Select", doNothing, noEvent, noStyle
       , VALUE("Zero", 0, doNothing, noEvent)
       , VALUE("One", 1, doNothing, noEvent)
       , VALUE("Two", 2, doNothing, noEvent)
      );

int chooseTest = -1;
CHOOSE(chooseTest, chooseMenu, "Choose", doNothing, noEvent, noStyle
       , VALUE("First", 1, doNothing, noEvent)
       , VALUE("Second", 2, doNothing, noEvent)
       , VALUE("Third", 3, doNothing, noEvent)
       , VALUE("Last", -1, doNothing, noEvent)
      );

//customizing a prompt look!
//by extending the prompt class
class altPrompt: public prompt {
  public:
    altPrompt(constMEM promptShadow& p): prompt(p) {}
    Used printTo(navRoot &root, bool sel, menuOut& out, idx_t idx, idx_t len, idx_t) override {
      return out.printRaw(F("special prompt!"), len);
    }
};

MENU(subMenu, "Sub-Menu", showEvent, anyEvent, noStyle
     , OP("Sub1", showEvent, anyEvent)
     , OP("Sub2", showEvent, anyEvent)
     , OP("Sub3", showEvent, anyEvent)
     , altOP(altPrompt, "", showEvent, anyEvent)
     , EXIT("<Back")
    );

MENU(mainMenu, "Main menu", doNothing, noEvent, wrapStyle
     , OP("Op1", action1, anyEvent)
     , OP("Op2", action2, enterEvent)
     /* FIELD Parameters :

        Action Name(function name), Action Heading, Action Heading Unit,
        range_lowest, range_highest, range_increment_step,
        range_decrement_step
     */
     , FIELD(brightnessValue, "Brightness", "%", 0, 100, 5, 5, adjustBrightness, enterEvent, wrapStyle)
     , SUBMENU(subMenu)
     , SUBMENU(setLed)
     , OP("LED On", internalLedOn, enterEvent) // will turn on built-in LED
     , OP("LED Off", internalLedOff, enterEvent)// will turn off built-in LED
     , SUBMENU(selMenu)
     , SUBMENU(chooseMenu)
     , OP("Alert test", doAlert, enterEvent)
     , EXIT("<Back")
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

result alert(menuOut& o, idleEvent e) {
  if (e == idling) {
    o.setCursor(0, 0);
    o.print("alert test");
    o.setCursor(0, 1);
    o.print("press [select]");
    o.setCursor(0, 2);
    o.print("to continue...");
  }
  return proceed;
}

result doAlert(eventMask e, prompt &item) {
  nav.idleOn(alert);
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

void buttonHandler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println(button->getPin());
      switch (button->getPin()) {
        case buttonUpPin:
          nav.doNav(navCmd(downCmd)); // replace with the appropriate menu navigation command
          break;
        case buttonDownPin:
          nav.doNav(navCmd(upCmd)); // replace with the appropriate menu navigation command
          break;
        case buttonEnterPin:
          nav.doNav(navCmd(enterCmd)); // replace with the appropriate menu navigation command
          break;
        case buttonLeftPin:
          nav.doNav(navCmd(escCmd)); // replace with the appropriate menu navigation command
          break;
      }
      break;
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonEnterPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  buttonUp.setEventHandler(buttonHandler);
  buttonLeft.setEventHandler(buttonHandler);
  buttonEnter.setEventHandler(buttonHandler);
  buttonRight.setEventHandler(buttonHandler);
  buttonDown.setEventHandler(buttonHandler);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin();

  oled.begin(&Adafruit128x64, OLED_I2C_ADDRESS); //check config
  oled.setFont(menuFont);
  oled.clear();
  nav.idleTask = idle; //point a function to be used when menu is suspended
}

void loop() {
  buttonDown.check();
  buttonUp.check();
  buttonEnter.check();
  buttonLeft.check();
  nav.poll();
}
