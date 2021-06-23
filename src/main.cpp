#include <Arduino.h>
#include <TM1637.h>
#include <NewPing.h>
#include <EEPROM.h>

#define TRIGGER_PIN 16                              // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 16                                 // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 130                            // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

// Instantiation and pins configurations
#define CLK_PIN 14
#define DIO_PIN 12
TM1637 tm1637(DIO_PIN, CLK_PIN);

#define BUTTON_1_PIN 4
#define BUTTON_2_PIN 5
#define BUTTON_3_PIN 2
#define BUTTON_4_PIN 0

#define H_BRIDGE_RPWM 13
#define H_BRIDGE_LPWM 15

#define MOTOR_SPEED 127

void goUp()
{
  Serial.println("Going Up");
  analogWrite(H_BRIDGE_RPWM, 0);
  analogWrite(H_BRIDGE_LPWM, MOTOR_SPEED);
}

void goDown()
{
  Serial.println("Going Down");
  analogWrite(H_BRIDGE_RPWM, MOTOR_SPEED);
  analogWrite(H_BRIDGE_LPWM, 0);
}

void stop()
{
  Serial.println("Stopping");
  analogWrite(H_BRIDGE_LPWM, 0);
  analogWrite(H_BRIDGE_RPWM, 0);
}

/**
 * Get value of the EEPROM at a certain address
 */
int getHeightValue(int index)
{
  return EEPROM.read(index);
}

/**
 * Save the value to the EEPROM
 */
void saveHeightValue(int index, int height)
{
  Serial.print("Writing ");
  Serial.print(height);
  Serial.print(" to index ");
  Serial.println(index);

  EEPROM.write(index, height);
  EEPROM.commit();
}

int lastDisplayValue = 0;
/**
 * Clear the display and display
 */
void display(int display)
{
  //if the value is diffrent then clean the screen
  if (lastDisplayValue != display)
  {
    tm1637.clearScreen();
  }
  tm1637.display(display);
  lastDisplayValue = display;
}

void display(String display)
{
  // tm1637.clearScreen();
  tm1637.display(display);
}

/**
 * Get the current value of the ultrasonic sensor
 */
int getCurrentHeight()
{
  return sonar.ping_cm();
  ;
}

void goToHeight(int height)
{
  while (getCurrentHeight() < height)
  {
    goUp();
    display(getCurrentHeight());
    delay(250);
  }
  while (getCurrentHeight() > height)
  {
    goDown();
    display(getCurrentHeight());
    delay(250);
  }
}

void setup()
{
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  EEPROM.begin(32);
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_4_PIN, INPUT_PULLUP);

  pinMode(H_BRIDGE_RPWM, OUTPUT);
  pinMode(H_BRIDGE_LPWM, OUTPUT);

  tm1637.init();
  tm1637.setBrightness(255);
  display(8888);
  tm1637.colonOn();
  delay(1500);
  tm1637.colonOff();
}

bool setHeight = false;

void loop()
{
  stop();
  display(getCurrentHeight());

  //if we are not in the setting height mode
  if (!setHeight)
  {
    tm1637.colonOff();
    if (digitalRead(BUTTON_1_PIN) == LOW)
    {
      goToHeight(getHeightValue(0));
    }
    if (digitalRead(BUTTON_2_PIN) == LOW)
    {
      goToHeight(getHeightValue(1));
    }
  }

  // go into change height "menu"
  if (digitalRead(BUTTON_3_PIN) == LOW && digitalRead(BUTTON_4_PIN) == LOW)
  {
    setHeight = true;
  }

  //if we are in the set height mode
  if (setHeight)
  {
    Serial.println("Setting Value");
    tm1637.colonOn();
    if (digitalRead(BUTTON_1_PIN) == LOW)
    {
      saveHeightValue(0, getCurrentHeight());
      setHeight = false;
    }
    if (digitalRead(BUTTON_2_PIN) == LOW)
    {
      saveHeightValue(1, getCurrentHeight());
      setHeight = false;
    }
  }

  if (digitalRead(BUTTON_3_PIN) == LOW)
  {
    // set a limite so we don't break the desk
    // if (getCurrentHeight() < 117)
    // {
    goUp();
    // }
  }
  if (digitalRead(BUTTON_4_PIN) == LOW)
  {
    // set a limite so we don't break the desk
    // if (getCurrentHeight() > 71)
    // {
    goDown();
    // }
  }

  Serial.print("Ping: ");
  Serial.print(getCurrentHeight()); // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");
  Serial.println("buttons");
  Serial.println(digitalRead(BUTTON_1_PIN));
  Serial.println(digitalRead(BUTTON_2_PIN));
  Serial.println(digitalRead(BUTTON_3_PIN));
  Serial.println(digitalRead(BUTTON_4_PIN));
  Serial.println("eeprom");
  Serial.println(getHeightValue(0));
  Serial.println(getHeightValue(1));
  Serial.println();
  delay(250);
}
