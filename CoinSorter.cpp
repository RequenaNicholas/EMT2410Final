//============================================================================
// Name        : CoinSorter.cpp
// Author      : Nicholas Requena
// Version     : 0.3
/* Description :    An Infrared Sensor (IR) sensor coin detection system.
                    Displays the total value of money passed through the
                    machine on an 16x2 I2C LCD screen, which uses 
                    SDI and SCL pins for its connectors. Runs on  */
//============================================================================

#include "mbed.h"
#include "TextLCD.h"

#define COOLDOWN     0
#define READY        1
#define PN_DETECTED  2
#define NI_DETECTED  3
#define DI_DETECTED  4
#define QT_DETECTED  5
#define DL_DETECTED  6

// Physical components: LEDs, Infrared Sensors,
DigitalOut  led(LED1);
DigitalIn   PinPN(D4);
// Replace pin# with the pin that each sensor is connected to (i.e. PinPN(D4)).
DigitalIn   PinNI(pin#);
DigitalIn   PinDI(pin#);
DigitalIn   PinQT(pin#);
DigitalIn   PinDL(pin#);


// Threads: Essentially Timers
Thread thread2;

// LCD Setup
I2C i2c_lcd(PB_9, PB_8); // SDA and SCL pins. LCD will not be detected if they are not connected here.
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD16x2); // I2C bus, PCF8574 Slaveaddress, LCD Type

// Function Declarations (Definitions are at the end of the page)
void timerThread();
void timers(void);
void startLCD();
void readSensor();
void displayAmountSerial();
void displayAmountLCD(float moneyNumber);
void detectPenny();
void detectNickel();
void detectDime();
void detectQuarter();
void detectDollar();
float convertUSD(float moneyNumber);


// Timers
unsigned long coin_tmr = 0; //increases every ~100ms
unsigned long bad_tmr = 0; //counter that increases based on the mbed board's clock speed

// LCD and Serial Monitor Init.
Serial pc(USBTX, USBRX); // Initialize Transmitting and Recieving

// IR Sensor values
int pin_state_pn = 0;
int pin_state_ni = 0;
int pin_state_di = 0;
int pin_state_qt = 0;
int pin_state_dl = 0;

// Coin Counters
int coin_count = 0;
int money_worth = 0;
int machine_state = COOLDOWN;

int main() {
    // Start timers & LCD
    thread2.start(timerThread);
    startLCD();

    while (true) {
        
        timers();
        readSensor();

        switch (machine_state) {  
            case COOLDOWN:
                if (coin_tmr > 3)    machine_state = READY;
                else                 machine_state = COOLDOWN;
                break;
            case READY:
                if (pin_state_pn == 0) machine_state = PN_DETECTED;
                if (pin_state_ni == 0) machine_state = NI_DETECTED;
                if (pin_state_di == 0) machine_state = DI_DETECTED;
                if (pin_state_qt == 0) machine_state = QT_DETECTED;
                if (pin_state_dl == 0) machine_state = DL_DETECTED;
                else                   machine_state = READY;
                break;
            case PN_DETECTED:
                detectPenny();
                coin_tmr = 0;
                machine_state = COOLDOWN;
                break;
            case NI_DETECTED:
                detectNickel();
                coin_tmr = 0;
                machine_state = COOLDOWN;
                break;
            case DI_DETECTED:
                detectDime();
                coin_tmr = 0;
                machine_state = COOLDOWN;
                break;
            case QT_DETECTED:
                detectQuarter();
                coin_tmr = 0;
                machine_state = COOLDOWN;
                break;
            case DL_DETECTED:
                detectDollar();
                coin_tmr = 0;
                machine_state = COOLDOWN;
                break;                                                
        }                
    }
}



void timerThread() {
    // us. long millis increases every 1ms, replaces millis() function from arduino
    while (true) {
        bad_tmr++;
        ThisThread::sleep_for(1);
    }
}

void displayAmountSerial()
{
// Function to display coin info via. Serial Monitor
// Print the value of coin_count--the # of coins that has been detected
// Print the value of money_worth--the value of the coins added up
  pc.printf("Coin Count:   %i\n", coin_count);
  pc.printf("Total Money: $%f\n", convertUSD(money_worth));
  pc.printf(" \n");
}

void displayAmountLCD(float moneyNumber)
{   // Diplays float amount in dollars on LCD
    lcd.cls();
    lcd.printf("Total Amount is: \n");
    lcd.locate(0,1);
    lcd.printf("$%f", convertUSD(moneyNumber));      
}

void timers() {
// Create: "unsigned long timer_name = 0" at start of program
// Put 'timer_name++' under "if(one_ms_timer > 99)"
// Declare timers() function at beginning of loop to run every 100ms
  static unsigned long ms_runtime = 0;
  static unsigned long one_ms_timer = 0;

  if (bad_tmr > ms_runtime)
  {
    ms_runtime++;
    one_ms_timer++;
  }

  else if (ms_runtime > bad_tmr)
  {
    ms_runtime = bad_tmr;
    one_ms_timer++;
  }

  if(one_ms_timer > 15)
  { //runs every fuck if i know
    //put new timers here
    coin_tmr++;
    one_ms_timer = 0;
  }
}

float convertUSD(float moneyNumber)
{
// Take the moneyNumber--the total cents detected
// divide that by 100 to convert to USD dollars
// return the dollar count
  float cents = moneyNumber;
  float dollar = cents/100;
  
  return dollar;
}

void startLCD() {
    // Clears LCD, Turns on backlight, Tests a line of text
    lcd.cls();
    lcd.setBacklight(TextLCD::LightOn);
    lcd.printf("Init. Success");
}

void readSensor () {
    // Reads value across all IR Sensors
    pin_state_pn = PinPN.read();
    pin_state_ni = PinNI.read();
    pin_state_di = PinDI.read();
    pin_state_qt = PinQT.read();
    pin_state_dl = PinDL.read();

}

void detectPenny () {
    pc.printf("Penny has been detected\n");
    coin_count++;
    money_worth = money_worth + 1;
    displayAmountSerial();
    displayAmountLCD(money_worth);
}

void detectNickel () {
    pc.printf("Nickel has been detected\n");
    coin_count++;
    money_worth = money_worth + 5;
    displayAmountSerial();
    displayAmountLCD(money_worth);
}

void detectDime () {
    pc.printf("Dime has been detected\n");
    coin_count++;
    money_worth = money_worth + 10;
    displayAmountSerial();
    displayAmountLCD(money_worth);
}

void detectQuarter () {
    pc.printf("Quarter has been detected\n");
    coin_count++;
    money_worth = money_worth + 25;
    displayAmountSerial();
    displayAmountLCD(money_worth);
}

void detectDollar () {
    pc.printf("Dollar has been detected\n");
    coin_count++;
    money_worth = money_worth + 100;
    displayAmountSerial();
    displayAmountLCD(money_worth);
}