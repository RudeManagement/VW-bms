  /*
  Copyright (c) 2019 Simp ECO Engineering
  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:
  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

/////////////////////////////////////////////////////////////////////////////////////////////////

░██████╗██╗███╗░░░███╗██████╗░██████╗░███╗░░░███╗░██████╗  
██╔════╝██║████╗░████║██╔══██╗██╔══██╗████╗░████║██╔════╝  
╚█████╗░██║██╔████╔██║██████╔╝██████╦╝██╔████╔██║╚█████╗░  
░╚═══██╗██║██║╚██╔╝██║██╔═══╝░██╔══██╗██║╚██╔╝██║░╚═══██╗  
██████╔╝██║██║░╚═╝░██║██║░░░░░██████╦╝██║░╚═╝░██║██████╔╝  
╚═════╝░╚═╝╚═╝░░░░░╚═╝╚═╝░░░░░╚═════╝░╚═╝░░░░░╚═╝╚═════╝░  

░██████╗██████╗░░█████╗░░█████╗░███████╗  ██████╗░░█████╗░██╗░░░░░██╗░░░░░░██████╗
██╔════╝██╔══██╗██╔══██╗██╔══██╗██╔════╝  ██╔══██╗██╔══██╗██║░░░░░██║░░░░░██╔════╝
╚█████╗░██████╔╝███████║██║░░╚═╝█████╗░░  ██████╦╝███████║██║░░░░░██║░░░░░╚█████╗░
░╚═══██╗██╔═══╝░██╔══██║██║░░██╗██╔══╝░░  ██╔══██╗██╔══██║██║░░░░░██║░░░░░░╚═══██╗
██████╔╝██║░░░░░██║░░██║╚█████╔╝███████╗  ██████╦╝██║░░██║███████╗███████╗██████╔╝
╚═════╝░╚═╝░░░░░╚═╝░░╚═╝░╚════╝░╚══════╝  ╚═════╝░╚═╝░░╚═╝╚══════╝╚══════╝╚═════╝░

███████╗██████╗░██╗████████╗██╗░█████╗░███╗░░██╗
██╔════╝██╔══██╗██║╚══██╔══╝██║██╔══██╗████╗░██║
█████╗░░██║░░██║██║░░░██║░░░██║██║░░██║██╔██╗██║
██╔══╝░░██║░░██║██║░░░██║░░░██║██║░░██║██║╚████║
███████╗██████╔╝██║░░░██║░░░██║╚█████╔╝██║░╚███║
╚══════╝╚═════╝░╚═╝░░░╚═╝░░░╚═╝░╚════╝░╚═╝░░╚══╝


This version of SimpBMS has been modified as the Space Balls edition utilising the Teensey 3.6, alowingfor upto 4 Canbus' 
  2 Native(Flexcans) + 2 MCP2515/SPI Cans
*/

#include "BMSModuleManager.h"
#include <Arduino.h>
#include "config.h"
#include "SerialConsole.h"
#include "Logger.h"
#include <ADC.h> //https://github.com/pedvide/ADC
#include <EEPROM.h>
#include <SPI.h>
#include "BMSUtil.h"
#include "BMSCan.h"
#include <Filters.h>//https://github.com/JonHub/Filters

#define CPU_REBOOT (_reboot_Teensyduino_());


BMSModuleManager bms;
SerialConsole console;
EEPROMSettings settings;


/////Version Identifier/////////
int firmver = 220225;

//Curent filter//
float filterFrequency = 5.0 ;
FilterOnePole lowpassFilter( LOWPASS, filterFrequency );


//Simple BMS V2 wiring//
//const int ACUR2 = A0; // current 1
//const int ACUR1 = A1; // current 2
const int IN1 = 17; // input 1 - high active
const int IN2 = 16; // input 2- high active
const int IN3 = 18; // input 1 - high active
const int IN4 = 19; // input 2- high active
const int OUT1 = 11;// output 1 - high active
const int OUT2 = 12;// output 1 - high active
//const int OUT3 = 20;// output 1 - high active, using for 2nd SPI CAN
const int OUT4 = 21;// output 1 - high active //repurpose for fan
const int OUT5 = 22;// output 1 - high active
const int OUT6 = 23;// output 1 - high active
const int OUT7 = 5;// output 1 - high active
const int OUT8 = 6;// output 1 - high active
const int led = 13;
const int BMBfault = 11;

byte bmsstatus = 0;
//bms status values
#define Boot 0
#define Ready 1
#define Drive 2
#define Charge 3
#define Precharge 4
#define RapidCharge 5
#define Error 6
//

//Current sensor values
#define Undefined 0
#define Analoguedual 1
#define Canbus 2
#define Analoguesing 3
#define TeslaSPICS 36
//

// Can current sensor values
#define LemCAB300 1
#define IsaScale 3
#define VictronLynx 4
#define LemCAB500 2
#define CurCanMax 4 // max value
#define TeslaP100D 5

//Charger Types
#define NoCharger 0
#define BrusaNLG5 1
#define ChevyVolt 2
#define Eltek 3
#define Elcon 4
#define Victron 5
#define Coda 6
#define Outlander 8
//
int outlander_charger_reported_voltage = 0;
int outlander_charger_reported_current = 0;
int outlander_charger_reported_temp1 = 0;
int outlander_charger_reported_temp2 = 0;
byte outlander_charger_reported_status = 0;
byte evse_duty = 0;
bool secondPackFound = false;

int Discharge;
int ErrorReason = 0;

//variables for output control
int pulltime = 1000;
int contctrl, contstat = 0; //1 = out 5 high 2 = out 6 high 3 = both high
unsigned long conttimer1, conttimer2, conttimer3, Pretimer, Pretimer1, overtriptimer, undertriptimer, mainconttimer = 0;
uint16_t pwmfreq = 15000;//pwm frequency

int pwmcurmax = 200;//Max current to be shown with pwm
int pwmcurmid = 50;//Mid point for pwm dutycycle based on current
int16_t pwmcurmin = 0;//DONOT fill in, calculated later based on other values


//variables for VE driect bus comms
char* myStrings[] = {"V", "14674", "I", "0", "CE", "-1", "SOC", "800", "TTG", "-1", "Alarm", "OFF", "Relay", "OFF", "AR", "0", "BMV", "600S", "FW", "212", "H1", "-3", "H2", "-3", "H3", "0", "H4", "0", "H5", "0", "H6", "-7", "H7", "13180", "H8", "14774", "H9", "137", "H10", "0", "H11", "0", "H12", "0"};

//variables for VE can
uint16_t chargevoltage = 49100; //max charge voltage in mv
int chargecurrent;
uint16_t disvoltage = 42000; // max discharge voltage in mv
int discurrent;
uint16_t SOH = 100; // SOH place holder

unsigned char alarm[4], warning[4] = {0, 0, 0, 0};
unsigned char mes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char bmsname[8] = {'S', 'I', 'M', 'P', ' ', 'B', 'M', 'S'};
unsigned char bmsmanu[8] = {'S', 'I', 'M', 'P', ' ', 'E', 'C', 'O'};
long unsigned int rxId;
unsigned char len = 0;
byte rxBuf[8];
char msgString[128];                        // Array to store serial string
uint32_t inbox;
int32_t CANmilliamps;
signed long voltage1, voltage2, voltage3 = 0; //mV only with ISAscale sensor
double amphours, kilowatthours, kilowatts; //only with ISAscale sensor
//struct can_frame canMsg;
//MCP2515 CAN1(10); //set CS pin for can controlelr

//variables for current calulation
int value;
float currentact, RawCur;
float ampsecond;
unsigned long lasttime;
unsigned long inverterLastRec;
byte inverterStatus;
bool inverterInDrive = false;
bool rapidCharging = false;
unsigned long looptime, looptime1, looptime2, UnderTime, cleartime, chargertimer = 0; //ms
int currentsense = 14;
int sensor = 1;

//running average
const int RunningAverageCount = 16;
float RunningAverageBuffer[RunningAverageCount];
int NextRunningAverage;

//Variables for SOC calc
int SOC = 100; //State of Charge
int SOCset = 0;
int SOCreset = 0;
int SOCtest = 0;
int SOCmem = 0;

///charger variables
int maxac1 = 16; //Shore power 16A per charger
int maxac2 = 10; //Generator Charging
int chargerid1 = 0x618; //bulk chargers
int chargerid2 = 0x638; //finishing charger
float chargerendbulk = 0; //V before Charge Voltage to turn off the bulk charger/s
float chargerend = 0; //V before Charge Voltage to turn off the finishing charger/s
int chargertoggle = 0;
int ncharger = 1; // number of chargers

//variables
int outputstate = 0;
int incomingByte = 0;
int storagemode = 0;
int x = 0;
int balancecells;
int cellspresent = 0;
int Charged = 0;
int chargeOverride = 0;


//VW BMS CAN variables////////////
int controlid = 0x0BA;
int moduleidstart = 0x1CC;

//Serial Expansion Variables///
int SerialID = 0; //ID assigned over serialbus
int SerialSlaves = 0; //number of slaves present


//Debugging modes//////////////////
int debug = 1;
int gaugedebug = 0;
int inputcheck = 0; //read digital inputs
int outputcheck = 0; //check outputs
int candebug = 0; //view can frames
int debugCur = 0;
int CSVdebug = 0;
int menuload = 0;
int debugdigits = 2; //amount of digits behind decimal for voltage reading

ADC *adc = new ADC(); // adc object

void getcurrent();
static void receivedFiltered (const CANMessage & inMsg) {
    if (candebug ==1) {
          Serial.print(millis());
          if ((inMsg.id & 0x80000000) == 0x80000000)    // Determine if ID is standard (11 bits) or extended (29 bits)
            sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (inMsg.id & 0x1FFFFFFF), inMsg.len);
          else
            sprintf(msgString, ",0x%.3lX,false,%1d", inMsg.id, inMsg.len);

          Serial.print(msgString);

          Serial.println(" Filtered Can ");
    }

    if (inMsg.id == 0x389) {
      outlander_charger_reported_voltage = inMsg.data[0] * 2;
      outlander_charger_reported_current = inMsg.data[2];
      outlander_charger_reported_temp1 = inMsg.data[3] - 40;
      outlander_charger_reported_temp2 = inMsg.data[4] - 40;

    } else if (inMsg.id == 0x38A) {
       outlander_charger_reported_status = inMsg.data[4];
       evse_duty = inMsg.data[3];
    } else if (inMsg.id == 0x527) {
        long ampseconds = inMsg.data[2] + (inMsg.data[3] << 8) + (inMsg.data[4] << 16) + (inMsg.data[5] << 24);
        amphours = ampseconds/3600.0f;
    } else if(inMsg.id == 0x521) {
        CANmilliamps = inMsg.data[2] + (inMsg.data[3] << 8) + (inMsg.data[4] << 16) + (inMsg.data[5] << 24);
        RawCur = CANmilliamps; 
        getcurrent();
    } else if(inMsg.id == 0x522) {
        voltage1 = inMsg.data[2] + (inMsg.data[3] << 8) + (inMsg.data[4] << 16) + (inMsg.data[5] << 24);
    } else if(inMsg.id == 0x523) {
         voltage2 = inMsg.data[2] + (inMsg.data[3] << 8) + (inMsg.data[4] << 16) + (inMsg.data[5] << 24);
    } else if(inMsg.id == 0x526) {
        long watt = inMsg.data[2] + (inMsg.data[3] << 8) + (inMsg.data[4] << 16) + (inMsg.data[5] << 24);
        kilowatts = watt/1000.0f;
    } else if(inMsg.id == 0x527) {
        long ampseconds = inMsg.data[2] + (inMsg.data[3] << 8) + (inMsg.data[4] << 16) + (inMsg.data[5] << 24);
        amphours = ampseconds/3600.0f;
    } else if(inMsg.id == 0x528) {
        long wh = inMsg.data[2] + (inMsg.data[3] << 8) + (inMsg.data[4] << 16) + (inMsg.data[5] << 24);
        kilowatthours = wh/1000.0f;
    } else if (inMsg.id == 0x02) {
      inverterLastRec = millis();
      inverterStatus = inMsg.data[0];
    }
    //canio
    else if (inMsg.id == 0x01) {
      inverterInDrive = inMsg.data[1] & 0x80 == 0x80;
    }
    //chademo
    else if (inMsg.id == 0x354 && inMsg.data[0] == 0x01) {
      rapidCharging = true;
    }
  
}

bool chargeEnabled() {
  return digitalRead(IN3) == HIGH || chargeOverride == 1;
}

bool inverterControlledContactorsStatus() {
  // if inverter in RUN mode
  if (inverterStatus == 0x01) {
    return true;
  }

  return false;
}

void loadSettings()
{
  Logger::console("Resetting to factory defaults");
  settings.version = EEPROM_VERSION;
  settings.checksum = 2;
  settings.canSpeed = 500000;
  settings.batteryID = 0x01; //in the future should be 0xFF to force it to ask for an address
  settings.OverVSetpoint = 4.2f;
  settings.UnderVSetpoint = 3.0f;
  settings.ChargeVsetpoint = 4.1f;
  settings.ChargeHys = 0.2f; // voltage drop required for charger to kick back on
  settings.WarnOff = 0.1f; //voltage offset to raise a warning
  settings.DischVsetpoint = 3.2f;
  settings.CellGap = 0.2f; //max delta between high and low cell
  settings.OverTSetpoint = 65.0f;
  settings.UnderTSetpoint = -10.0f;
  settings.ChargeTSetpoint = 0.0f;
  settings.DisTSetpoint = 40.0f;
  settings.WarnToff = 5.0f; //temp offset before raising warning
  settings.IgnoreTemp = 0; // 0 - use both sensors, 1 or 2 only use that sensor
  settings.IgnoreVolt = 0.5;//
  settings.balanceVoltage = 3.9f;
  settings.balanceHyst = 0.04f;
  settings.logLevel = 2;
  settings.CAP = 100; //battery size in Ah
  settings.Pstrings = 1; // strings in parallel used to divide voltage of pack
  settings.Scells = 12;//Cells in series
  settings.StoreVsetpoint = 3.8; // V storage mode charge max
  settings.discurrentmax = 300; // max discharge current in 0.1A
  settings.DisTaper = 0.3f; //V offset to bring in discharge taper to Zero Amps at settings.DischVsetpoint
  settings.chargecurrentmax = 300; //max charge current in 0.1A
  settings.chargecurrentend = 50; //end charge current in 0.1A
  settings.socvolt[0] = 3100; //Voltage and SOC curve for voltage based SOC calc
  settings.socvolt[1] = 10; //Voltage and SOC curve for voltage based SOC calc
  settings.socvolt[2] = 4100; //Voltage and SOC curve for voltage based SOC calc
  settings.socvolt[3] = 90; //Voltage and SOC curve for voltage based SOC calc
  settings.invertcur = 0; //Invert current sensor direction
  settings.cursens = 2;
  settings.chargerCanIndex = DEFAULT_CAN_INTERFACE_INDEX; //default to can0
  settings.veCanIndex = DEFAULT_CAN_INTERFACE_INDEX; //default to can0
  settings.secondBatteryCanIndex = DEFAULT_CAN_INTERFACE_INDEX; //default to can0, effectivly no second pack
  settings.curcan = LemCAB300;
  settings.voltsoc = 0; //SOC purely voltage based
  settings.Pretime = 5000; //ms of precharge time
  settings.conthold = 50; //holding duty cycle for contactor 0-255
  settings.Precurrent = 1000; //ma before closing main contator
  settings.convhigh = 58; // mV/A current sensor high range channel
  settings.convlow = 643; // mV/A current sensor low range channel
  settings.changecur = 20000;//mA change overpoint
  settings.offset1 = 1750; //mV mid point of channel 1
  settings.offset2 = 1750;//mV mid point of channel 2
  settings.gaugelow = 50; //empty fuel gauge pwm
  settings.gaugehigh = 255; //full fuel gauge pwm
  settings.ESSmode = 0; //activate ESS mode
  settings.ncur = 1; //number of multiples to use for current measurement
  settings.chargertype = 2; // 1 - Brusa NLG5xx 2 - Volt charger 0 -No Charger
  settings.chargerspd = 100; //ms per message
  settings.UnderDur = 5000; //ms of allowed undervoltage before throwing open stopping discharge.
  settings.CurDead = 5;// mV of dead band on current sensor
  settings.ChargerDirect = 1; //1 - charger is always connected to HV battery // 0 - Charger is behind the contactors
  settings.DeltaVolt = 0.5; //V of allowable difference between measurements
  settings.tripcont = 1; //in ESSmode 1 - Main contactor function, 0 - Trip function
  settings.triptime = 500;//mS of delay before counting over or undervoltage
}

BMSCan bmscan;
BMS_CAN_MESSAGE msg;
BMS_CAN_MESSAGE inMsg;

uint32_t lastUpdate;


void setup()
{
  delay(4000);  //just for easy debugging. It takes a few seconds for USB to come up properly on most OS's
//  pinMode(ACUR1, INPUT);
//  pinMode(ACUR2, INPUT);
  pinMode(IN1, INPUT);
  pinMode(IN2, INPUT);
  pinMode(IN3, INPUT);
  pinMode(IN4, INPUT);
  pinMode(OUT1, OUTPUT); // drive contactor
  digitalWrite(OUT1, LOW);
  pinMode(OUT2, OUTPUT); // precharge
  digitalWrite(OUT2, LOW);
//  pinMode(OUT3, OUTPUT); // charge relay
//  digitalWrite(OUT3, LOW);
  pinMode(OUT4, OUTPUT); // fan relay
  digitalWrite(OUT4, LOW);
  pinMode(OUT5, OUTPUT); // pwm driver output
  digitalWrite(OUT5, LOW);
  pinMode(OUT6, OUTPUT); // pwm driver output
  digitalWrite(OUT6, LOW);
  pinMode(OUT7, OUTPUT); // pwm driver output
  digitalWrite(OUT7, LOW);
  pinMode(OUT8, OUTPUT); // pwm driver output
  digitalWrite(OUT8, LOW);
  pinMode(led, OUTPUT);

  analogWriteFrequency(OUT5, pwmfreq);
  analogWriteFrequency(OUT6, pwmfreq);
  analogWriteFrequency(OUT7, pwmfreq);
  analogWriteFrequency(OUT8, pwmfreq);

  /// SCK on 14, CS 15, MOSI - 7, MISO - 8
   Serial.print ("Using pin #") ;
  Serial.print (MCP2515_SI) ;
  Serial.print (" for MOSI: ") ;
  Serial.println (SPI.pinIsMOSI (MCP2515_SI) ? "yes" : "NO!!!") ;
  Serial.print ("Using pin #") ;
  Serial.print (MCP2515_SO) ;
  Serial.print (" for MISO: ") ;
  Serial.println (SPI.pinIsMISO (MCP2515_SO) ? "yes" : "NO!!!") ;
  Serial.print ("Using pin #") ;
  Serial.print (MCP2515_SCK) ;
  Serial.print (" for SCK: ") ;
  Serial.println (SPI.pinIsSCK (MCP2515_SCK) ? "yes" : "NO!!!") ;
 
  SPI.setMOSI (MCP2515_SI) ;
  SPI.setMISO (MCP2515_SO) ;
  SPI.setSCK (MCP2515_SCK) ;
  SPI.begin () ;
  
/* #ifdef __MK66FX1M0__
  SPI1.setMOSI (MCP2515_SI_2) ;
  SPI1.setMISO (MCP2515_SO_2) ;
  SPI1.setSCK (MCP2515_SCK_2) ;
  SPI1.begin () ;
  #endif
*/

  SERIALCONSOLE.begin(115200);
  SERIALCONSOLE.println("Starting up!");
  SERIALCONSOLE.println("SimpBMS V2 VW");

  Serial2.begin(115200);

  // Display reason the Teensy was last reset
  Serial.println();
  Serial.println("Reason for last Reset: ");

  if (RCM_SRS1 & RCM_SRS1_SACKERR)   Serial.println("Stop Mode Acknowledge Error Reset");
  if (RCM_SRS1 & RCM_SRS1_MDM_AP)    Serial.println("MDM-AP Reset");
  if (RCM_SRS1 & RCM_SRS1_SW)        Serial.println("Software Reset");                   // reboot with SCB_AIRCR = 0x05FA0004
  if (RCM_SRS1 & RCM_SRS1_LOCKUP)    Serial.println("Core Lockup Event Reset");
  if (RCM_SRS0 & RCM_SRS0_POR)       Serial.println("Power-on Reset");                   // removed / applied power
  if (RCM_SRS0 & RCM_SRS0_PIN)       Serial.println("External Pin Reset");               // Reboot with software download
  if (RCM_SRS0 & RCM_SRS0_WDOG)      Serial.println("Watchdog(COP) Reset");              // WDT timed out
  if (RCM_SRS0 & RCM_SRS0_LOC)       Serial.println("Loss of External Clock Reset");
  if (RCM_SRS0 & RCM_SRS0_LOL)       Serial.println("Loss of Lock in PLL Reset");
  if (RCM_SRS0 & RCM_SRS0_LVD)       Serial.println("Low-voltage Detect Reset");
  Serial.println();
  ///////////////////


  // enable WDT
  noInterrupts();                                         // don't allow interrupts while setting up WDOG
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;                         // unlock access to WDOG registers
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
  delayMicroseconds(1);                                   // Need to wait a bit..

  WDOG_TOVALH = 0x1000;
  WDOG_TOVALL = 0x0000;
  WDOG_PRESC  = 0;
  WDOG_STCTRLH |= WDOG_STCTRLH_ALLOWUPDATE |
                  WDOG_STCTRLH_WDOGEN | WDOG_STCTRLH_WAITEN |
                  WDOG_STCTRLH_STOPEN | WDOG_STCTRLH_CLKSRC;
  interrupts();
  /////////////////
 // SERIALBMS.begin(115200);
  //SERIALBMS.begin(612500); //Tesla serial bus
  //VE.begin(19200); //Victron VE direct bus
#if defined (__arm__) && defined (__SAM3X8E__)
  serialSpecialInit(USART0, 612500); //required for Due based boards as the stock core files don't support 612500 baud.
#endif

  SERIALCONSOLE.println("Started serial interface to BMS.");

  EEPROM.get(0, settings);
  if (settings.version != EEPROM_VERSION)
  {
    Serial.println();
    loadSettings();
  }

  bmscan.begin(500000, DEFAULT_CAN_INTERFACE_INDEX);
  bmscan.can2 = new ACAN2515 (MCP2515_CS, SPI, MCP2515_INT) ;
  ACAN2515Settings cansettings(8 * 1000 * 1000, 500000);
  const ACAN2515Mask rxm0 = standard2515Mask(0x7FF, 0, 0) ; // For filter #0 and #1
  const ACAN2515Mask rxm1 = standard2515Mask(0x7F0, 0, 0) ; // For filter #2 to #
  const ACAN2515AcceptanceFilter filters [] = {
    {standard2515Filter(0x02, 0, 0), receivedFiltered},
    {standard2515Filter(0x02, 0, 0), receivedFiltered},
    {standard2515Filter(0x520, 0, 0), receivedFiltered},
    {standard2515Filter(0x380, 0, 0), receivedFiltered},
    {standard2515Filter(0x354, 0, 0), receivedFiltered}
  };
  const uint16_t errorCode = bmscan.can2->begin (cansettings, [] { bmscan.can2->isr () ; }, rxm0, rxm1, filters, 5) ;
  bmscan.begin(500000, settings.chargerCanIndex);
  
//  bmscan.begin(500000, settings.veCanIndex);
//  bmscan.begin(500000, settings.secondBatteryCanIndex);

  Logger::setLoglevel(Logger::Off); //Debug = 0, Info = 1, Warn = 2, Error = 3, Off = 4

  lastUpdate = 0;
  digitalWrite(led, HIGH);
  bms.setPstrings(settings.Pstrings);
  bms.setSensors(settings.IgnoreTemp, settings.IgnoreVolt, settings.DeltaVolt);
  bms.setBalanceHyst(settings.balanceHyst);
   //SOC recovery//

  SOC = (EEPROM.read(1000));
  if (settings.voltsoc == 1)
  {
    SOCmem = 0;
  }
  else
  {
    if (SOC > 100)
    {
      SOCmem = 0;
    }
    else if (SOC > 1)
    {
      //SOCmem = 1;
    }
  }

  ////Calculate fixed numbers
  pwmcurmin = (pwmcurmid / 50 * pwmcurmax * -1);
  ////
  //if (settings.Serialexp == 1)
  //{
  //  delay(300);//wait for all other boards to boot
  //Serialslaveinit();
  // }


  ///precharge timer kickers
  Pretimer = millis();
  Pretimer1  = millis();

  cleartime = millis();
}

float readTeslaSPI() {
  int buff[18];
  SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
  digitalWrite(TeslaSPICS, LOW);
  // send the device the register you want to read:
  buff[0] = SPI.transfer(0x41);
  for (int I = 1; I < 7; I++)
  {
    buff[I] = SPI.transfer(0x00);
  }
  buff[8] = SPI.transfer(0x9A);
  digitalWrite(TeslaSPICS, HIGH);
  SPI.endTransaction();
  int32_t current = ((int32_t(buff[6]*256+buff[5])*256+buff[4])*256+buff[3]) ;
  SERIALCONSOLE.print("Tesla Shunt: ");
  SERIALCONSOLE.println(current);
  return 1.0f;
}

void loop()
{

  while (bmscan.available(DEFAULT_CAN_INTERFACE_INDEX))
  {
    canread(DEFAULT_CAN_INTERFACE_INDEX, 0);
  }

  bmscan.can2->dispatchReceivedMessage () ;


  //read secondBatteryCan if different from deafult. (If same as default, no secondard pack installed)
//  if (settings.secondBatteryCanIndex != DEFAULT_CAN_INTERFACE_INDEX) {
//    canread(settings.secondBatteryCanIndex, 32);
//  }
  
//  //read chargerCan if different to DEFAULT and different to secondary
//  if (settings.chargerCanIndex != DEFAULT_CAN_INTERFACE_INDEX 
//    && settings.chargerCanIndex != settings.secondBatteryCanIndex) {
//    canread(settings.chargerCanIndex, 0);
//  }
//
//  //read veCan if different to DEFAULT and charger, and secondary
//  if (settings.veCanIndex != DEFAULT_CAN_INTERFACE_INDEX 
//    && settings.veCanIndex != settings.chargerCanIndex 
//    && settings.veCanIndex != settings.secondBatteryCanIndex) {
//    canread(settings.veCanIndex, 0);
//  }


  
  if (SERIALCONSOLE.available() > 0)
  {
    menu();
  }

  if (outputcheck != 1)
  {
    contcon();
      switch (bmsstatus)
      {
        case (Boot):
          Discharge = 0;
          digitalWrite(OUT4, LOW);
          //digitalWrite(OUT3, LOW);//turn off charger
          digitalWrite(OUT2, LOW);
          digitalWrite(OUT1, LOW);//turn off discharge
          contctrl = 0;
          bmsstatus = Ready;
          break;

        case (Ready):
          Discharge = 0;
          digitalWrite(OUT4, LOW);
          //digitalWrite(OUT3, LOW);//turn off charger
          digitalWrite(OUT2, LOW);
          digitalWrite(OUT1, LOW);//turn off discharge
          contctrl = 0; //turn off out 5 and 6
          if (bms.getHighCellVolt() > settings.balanceVoltage && bms.getHighCellVolt() > bms.getLowCellVolt() + settings.balanceHyst)
          {
            //bms.balanceCells();
            balancecells = 1;
//          //balancecells = 0; //disabled for now
          }
          else
          {
            balancecells = 0;
          }
          if (chargeEnabled() && (bms.getHighCellVolt() < (settings.ChargeVsetpoint - settings.ChargeHys))) //detect AC present for charging and check not balancing
          {
            if (inverterControlledContactorsStatus())
            {
              bmsstatus = Charge;
            }
            else
            {
              bmsstatus = Precharge;
            }
          }
          if (inverterInDrive) {
            if (inverterControlledContactorsStatus())
            {
              bmsstatus = Drive;
            }
            else
            {
              bmsstatus = Precharge;
            } 
          }
          if (rapidCharging) {
            if (inverterControlledContactorsStatus())
            {
              bmsstatus = RapidCharge;
            }
            else
            {
              bmsstatus = Precharge;
            }
          }
//          if (digitalRead(IN1) == HIGH) //detect Key ON
//          {
//            bmsstatus = Precharge;
//            Pretimer = millis();
//          }

          break;

        case (Precharge):
          Discharge = 0;
          if (!rapidCharging && inverterControlledContactorsStatus() && chargeEnabled()) {
             bmsstatus = Charge;
          }
          if (!rapidCharging &&inverterControlledContactorsStatus() && inverterInDrive) {
             bmsstatus = Drive;
          }
//          if (rapidCharging &&inverterControlledContactorsStatus()) {
//             bmsstatus = RapidCharge;
//          }
          break;


        case (Drive):
          Discharge = 1;
          if (!inverterInDrive)//Key OFF
          {
            bmsstatus = Ready;
          }
          if (chargeEnabled() && (bms.getHighCellVolt() < (settings.ChargeVsetpoint - settings.ChargeHys))) //detect AC present for charging and check not balancing
          {
            bmsstatus = Charge;
          }

          break;

        case (Charge):
          Discharge = 0;
          digitalWrite(OUT4, HIGH);//enable fan
          if (bms.getHighCellVolt() > settings.balanceVoltage)
          {
            //bms.balanceCells();
            balancecells = 1;
            //balancecells = 0; //disabled for now

          }
          else
          {
            balancecells = 0;
          }
          if (bms.getHighCellVolt() > settings.ChargeVsetpoint)
          {
            if (bms.getAvgCellVolt() > (settings.ChargeVsetpoint - settings.ChargeHys))
            {
              SOCcharged(2);
            }
            else
            {
              resetISACounters();
            }
            //digitalWrite(OUT3, LOW);//turn off charger
            bmsstatus = Ready;
          }
          if (rapidCharging) {
            bmsstatus = RapidCharge;
          }
          if (!chargeEnabled() || !inverterControlledContactorsStatus())//detect AC not present for charging or inverter not closed the contactors
          {
            //send a 0 amp request to outlander
            chargecurrent = 0;
            chargercomms();
            bmsstatus = Ready;
          }
          break;

        case (Error):
          Discharge = 0;
          digitalWrite(OUT4, LOW);
          //digitalWrite(OUT3, LOW);//turn off charger
          digitalWrite(OUT2, LOW);
          digitalWrite(OUT1, LOW);//turn off discharge
          contctrl = 0; //turn off out 5 and 6
          /*
                    if (digitalRead(IN3) == HIGH) //detect AC present for charging
                    {
                      bmsstatus = Charge;
                    }
          */
          if (digitalRead(IN1) == LOW)//Key OFF
          {
            //if (cellspresent == bms.seriescells()) //detect a fault in cells detected
            //{
            if (bms.getLowCellVolt() >= settings.UnderVSetpoint && bms.getHighCellVolt() >= settings.OverVSetpoint)
            {
              bmsstatus = Ready;
            }
            //}
          }

          break;
      }
    
    if ( settings.cursens == Analoguedual || settings.cursens == Analoguesing)
    {
      getcurrent();
    }
    if (settings.cursens == TeslaP100D) 
    {
      RawCur = readTeslaSPI();
      //getcurrent();
    }
  }

  if(millis() - inverterLastRec > 200 ) {
    //Serial.println("Inverter TIMEOUT");
    //inverterStatus = 0;
  }

  if (millis() - looptime > 500)
  {
    looptime = millis();
    bms.getAllVoltTemp();
    if (SOCset != 0 && balancecells == 1)
    {
        bms.balanceCells(bmscan, 0, 0, DEFAULT_CAN_INTERFACE_INDEX);//1 is debug
        if (settings.secondBatteryCanIndex != DEFAULT_CAN_INTERFACE_INDEX) {
          bms.balanceCells(bmscan, 0, 32, settings.secondBatteryCanIndex);//1 is debug

        }
    }
    if (bms.getLowCellVolt() < settings.UnderVSetpoint || bms.getHighCellVolt() < settings.UnderVSetpoint)
    {
      if (UnderTime > millis()) //check is last time not undervoltage is longer thatn UnderDur ago
      {
        bmsstatus = Error;
        ErrorReason = ErrorReason | 0x02;
      }
    }
    else
    {
      UnderTime = millis() + settings.UnderDur;
      ErrorReason = ErrorReason & ~0x02;
    }
    
    if (inputcheck != 0)
    {
      inputdebug();
    }

    if (outputcheck != 0)
    {
      outputdebug();
    }
    else
    {
      gaugeupdate();
    }

    updateSOC();
    currentlimit();
    VEcan();

    sendcommand();
    
    if (cellspresent == 0 && SOCset == 1)
    {
      cellspresent = bms.seriescells();
      bms.setSensors(settings.IgnoreTemp, settings.IgnoreVolt, settings.DeltaVolt);
    }
    else
    {
      if (cellspresent != bms.seriescells() || cellspresent != (settings.Scells * settings.Pstrings)) //detect a fault in cells detected
      {
        if (debug != 0)
        {
          SERIALCONSOLE.println("  ");
          SERIALCONSOLE.print("   !!! Series Cells Fault !!!");
          SERIALCONSOLE.println("  ");
        }
        bmsstatus = Error;
        ErrorReason = ErrorReason | 0x04;
      }
      else
      {
        ErrorReason = ErrorReason & ~0x04;
      }

      //Allow the Error to be reset once, to allow time for the can bridge to boot)
      if (!secondPackFound && bms.seriescells() ==  (settings.Scells * settings.Pstrings)) {
        cellspresent = bms.seriescells();
        secondPackFound = true;
      }
      

    }
    alarmupdate();
    if (CSVdebug != 1)
    {
      dashupdate();
    }

    resetwdog();
  }

  if (millis() - looptime2 > 5000)
  {
    looptime2 = millis();

    if (debug != 0)
    {
      printbmsstat();
      bms.printPackDetails(debugdigits);
    }
    if (CSVdebug != 0)
    {
      bms.printAllCSV(millis(), currentact, SOC);
    }
  }
  
  if (millis() - cleartime > 20000)
  {
    if (bms.checkcomms())
    {
      //no missing modules
      /*
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print(" ALL OK NO MODULE MISSING :) ");
        SERIALCONSOLE.println("  ");
      */
      ErrorReason = ErrorReason & ~0x08;
      if (bmsstatus == Error && ErrorReason == 0)
      {
        bmsstatus = Boot;
      }
    }
    else
    {
      //missing module
      if (debug != 0)
      {
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("   !!! MODULE MISSING !!!");
        SERIALCONSOLE.println("  ");
      }
      bmsstatus = Error;
      ErrorReason = ErrorReason | 0x08;
    }
    //bms.clearmodules(); // Not functional
    cleartime = millis();
  }
  
  if (millis() - looptime1 > settings.chargerspd)
  {
    looptime1 = millis();
    if (settings.ESSmode == 1)
    {
      chargercomms();
    }
    else
    {
      if (bmsstatus == Charge)
      {
        chargercomms();
      }
    }
  }
}

void alarmupdate()
{
  alarm[0] = 0x00;
  if (settings.OverVSetpoint < bms.getHighCellVolt())
  {
    alarm[0] = 0x04;
  }
  if (bms.getLowCellVolt() < settings.UnderVSetpoint)
  {
    alarm[0] |= 0x10;
  }
  if (bms.getHighTemperature() > settings.OverTSetpoint)
  {
    alarm[0] |= 0x40;
  }
  alarm[1] = 0;
  if (bms.getLowTemperature() < settings.UnderTSetpoint)
  {
    alarm[1] = 0x01;
  }
  alarm[3] = 0;
  if ((bms.getHighCellVolt() - bms.getLowCellVolt()) > settings.CellGap)
  {
    alarm[3] = 0x01;
  }

  ///warnings///
  warning[0] = 0;

  if (bms.getHighCellVolt() > (settings.OverVSetpoint - settings.WarnOff))
  {
    warning[0] = 0x04;
  }
  if (bms.getLowCellVolt() < (settings.UnderVSetpoint + settings.WarnOff))
  {
    warning[0] |= 0x10;
  }

  if (bms.getHighTemperature() > (settings.OverTSetpoint - settings.WarnToff))
  {
    warning[0] |= 0x40;
  }
  warning[1] = 0;
  if (bms.getLowTemperature() < (settings.UnderTSetpoint + settings.WarnToff))
  {
    warning[1] = 0x01;
  }
}

void gaugeupdate()
{
  if (gaugedebug == 1)
  {
    SOCtest = SOCtest + 10;
    if (SOCtest > 1000)
    {
      SOCtest = 0;
    }
    analogWrite(OUT8, map(SOCtest * 0.1, 0, 100, settings.gaugelow, settings.gaugehigh));

    SERIALCONSOLE.println("  ");
    SERIALCONSOLE.print("SOC : ");
    SERIALCONSOLE.print(SOCtest * 0.1);
    SERIALCONSOLE.print("  fuel pwm : ");
    SERIALCONSOLE.print(map(SOCtest * 0.1, 0, 100, settings.gaugelow, settings.gaugehigh));
    SERIALCONSOLE.println("  ");
  }
  if (gaugedebug == 2) 
  {
    SOCtest = 0;
    analogWrite(OUT8, map(SOCtest * 0.1, 0, 100, settings.gaugelow, settings.gaugehigh));
  }
  if (gaugedebug == 3)
  {
    SOCtest = 1000;
    analogWrite(OUT8, map(SOCtest * 0.1, 0, 100, settings.gaugelow, settings.gaugehigh));
  }
  if (gaugedebug == 0)
  {
    analogWrite(OUT8, map(SOC, 0, 100, settings.gaugelow, settings.gaugehigh));
  }
}

void printbmsstat()
{
  SERIALCONSOLE.println();
  SERIALCONSOLE.println();
  SERIALCONSOLE.println();
  SERIALCONSOLE.print("BMS Status : ");
  if (settings.ESSmode == 1)
  {
    SERIALCONSOLE.print("ESS Mode ");

    if (bms.getLowCellVolt() < settings.UnderVSetpoint)
    {
      SERIALCONSOLE.print(": UnderVoltage ");
    }
    if (bms.getHighCellVolt() > settings.OverVSetpoint)
    {
      SERIALCONSOLE.print(": OverVoltage ");
    }
    if ((bms.getHighCellVolt() - bms.getLowCellVolt()) > settings.CellGap)
    {
      SERIALCONSOLE.print(": Cell Imbalance ");
    }
    if (bms.getAvgTemperature() > settings.OverTSetpoint)
    {
      SERIALCONSOLE.print(": Over Temp ");
    }
    if (bms.getAvgTemperature() < settings.UnderTSetpoint)
    {
      SERIALCONSOLE.print(": Under Temp ");
    }
    if (storagemode == 1)
    {
      if (bms.getLowCellVolt() > settings.StoreVsetpoint)
      {
        SERIALCONSOLE.print(": OverVoltage Storage ");
        SERIALCONSOLE.print(": UNhappy:");
      }
      else
      {
        SERIALCONSOLE.print(": Happy ");
      }
    }
    else
    {
      if (bms.getLowCellVolt() > settings.UnderVSetpoint && bms.getHighCellVolt() < settings.OverVSetpoint)
      {

        if ( bmsstatus == Error)
        {
          SERIALCONSOLE.print(": UNhappy:");
        }
        else
        {
          SERIALCONSOLE.print(": Happy ");
        }
      }
    }
    SERIALCONSOLE.print("ErrSt: ");
    SERIALCONSOLE.print(ErrorReason);
  }
  else
  {
    SERIALCONSOLE.print(bmsstatus);
    switch (bmsstatus)
    {
      case (Boot):
        SERIALCONSOLE.print(" Boot ");
        break;

      case (Ready):
        SERIALCONSOLE.print(" Ready ");
        break;

      case (Precharge):
        SERIALCONSOLE.print(" Precharge ");
        break;

      case (Drive):
        SERIALCONSOLE.print(" Drive ");
        break;

      case (Charge):
        SERIALCONSOLE.print(" Charge ");
        break;

      case (Error):
        SERIALCONSOLE.print(" Error ");
        break;
    }
  }
  SERIALCONSOLE.print("  ");
  if (chargeEnabled())
  {
    SERIALCONSOLE.print("| AC Present |");
  }
  if (digitalRead(IN1) == HIGH)
  {
    SERIALCONSOLE.print("| Key ON |");
  }
  if (balancecells == 1)
  {
    SERIALCONSOLE.print("|Balancing Active");
  }
  SERIALCONSOLE.print("  ");
  SERIALCONSOLE.print(cellspresent);
  SERIALCONSOLE.println();
  SERIALCONSOLE.print("Out:");
  SERIALCONSOLE.print(digitalRead(OUT1));
  SERIALCONSOLE.print(digitalRead(OUT2));
//  SERIALCONSOLE.print(digitalRead(OUT3));
  //SERIALCONSOLE.print(digitalRead(OUT4));
  SERIALCONSOLE.print(" Cont:");
  if ((contstat & 1) == 1)
  {
    SERIALCONSOLE.print("1");
  }
  else
  {
    SERIALCONSOLE.print("0");
  }
  if ((contstat & 2) == 2)
  {
    SERIALCONSOLE.print("1");
  }
  else
  {
    SERIALCONSOLE.print("0");
  }
  if ((contstat & 4) == 4)
  {
    SERIALCONSOLE.print("1");
  }
  else
  {
    SERIALCONSOLE.print("0");
  }
  if ((contstat & 8) == 8)
  {
    SERIALCONSOLE.print("1");
  }
  else
  {
    SERIALCONSOLE.print("0");
  }
  SERIALCONSOLE.print(" In:");
  SERIALCONSOLE.print(digitalRead(IN1));
  SERIALCONSOLE.print(digitalRead(IN2));
  SERIALCONSOLE.print(digitalRead(IN3));
  SERIALCONSOLE.print(digitalRead(IN4));
  
  SERIALCONSOLE.println();
  SERIALCONSOLE.print("- Can0 TX buffer number: ");
  SERIALCONSOLE.print(ACAN::can0.transmitBufferCount ());
  SERIALCONSOLE.print("/");
  SERIALCONSOLE.print(ACAN::can0.transmitBufferSize ());
  SERIALCONSOLE.print("-  RX buffer number: ");
  SERIALCONSOLE.print(ACAN::can0.receiveBufferCount ());
  SERIALCONSOLE.print("/");
  SERIALCONSOLE.print(ACAN::can0.receiveBufferSize ());
  SERIALCONSOLE.print("- Peak buffer number: ");
  SERIALCONSOLE.print(ACAN::can0.transmitBufferPeakCount ());
    SERIALCONSOLE.println();
  SERIALCONSOLE.print("- Can1 TX buffer number: ");
  SERIALCONSOLE.print(ACAN::can1.transmitBufferCount ());
  SERIALCONSOLE.print("/");
  SERIALCONSOLE.print(ACAN::can1.transmitBufferSize ());
  SERIALCONSOLE.print("-  RX buffer number: ");
  SERIALCONSOLE.print(ACAN::can1.receiveBufferCount ());
  SERIALCONSOLE.print("/");
  SERIALCONSOLE.print(ACAN::can1.receiveBufferSize ());
  SERIALCONSOLE.print("- Peak buffer number: ");
  SERIALCONSOLE.print(ACAN::can1.transmitBufferPeakCount ());

  if (bmsstatus == Charge && settings.chargertype == Outlander) {
    SERIALCONSOLE.println();
    SERIALCONSOLE.print("Outlander Charger - Reported Voltage: ");
    SERIALCONSOLE.print(outlander_charger_reported_voltage);
    SERIALCONSOLE.print("V Reported Current: ");
    SERIALCONSOLE.print(outlander_charger_reported_current / 10);
    SERIALCONSOLE.print("A Reported Temp1: ");
    SERIALCONSOLE.print(outlander_charger_reported_temp1);
    SERIALCONSOLE.print("C Reported Temp2: ");
    SERIALCONSOLE.print(outlander_charger_reported_temp2);
    SERIALCONSOLE.print("C status: ");
    if (outlander_charger_reported_status == 0) {
          SERIALCONSOLE.print("Not Charging");
      } else if (outlander_charger_reported_status == 0x04) {
          SERIALCONSOLE.print("Wait for Mains");
      } else if (outlander_charger_reported_status == 0x08) {
          SERIALCONSOLE.print("Ready/Charging");
      }
    SERIALCONSOLE.println();
  }
}


void getcurrent()
{
  if ( settings.cursens == Analoguedual || settings.cursens == Analoguesing)
  {
    if ( settings.cursens == Analoguedual)
    {
      if (currentact < settings.changecur && currentact > (settings.changecur * -1))
      {
        sensor = 1;
//        adc->startContinuous(ACUR1);
      }
      else
      {
        sensor = 2;
//        adc->adc0->startContinuous(ACUR2);
      }
    }
    else
    {
      sensor = 1;
//      adc->adc0->startContinuous(ACUR1);
    }
    if (sensor == 1)
    {
      if (debugCur != 0)
      {
        SERIALCONSOLE.println();
        if ( settings.cursens == Analoguedual)
        {
          SERIALCONSOLE.print("Low Range: ");
        }
        else
        {
          SERIALCONSOLE.print("Single In: ");
        }
        SERIALCONSOLE.print("Value ADC0: ");
      }
      value = (uint16_t)adc->adc0->analogReadContinuous(); // the unsigned is necessary for 16 bits, otherwise values larger than 3.3/2 V are negative!
      if (debugCur != 0)
      {
        SERIALCONSOLE.print(value * 3300 / adc->adc0->getMaxValue()); //- settings.offset1)
        SERIALCONSOLE.print(" ");
        SERIALCONSOLE.print(settings.offset1);
      }
      RawCur = int16_t((value * 3300 / adc->adc0->getMaxValue()) - settings.offset1) / (settings.convlow * 0.00001);

      if (abs((int16_t(value * 3300 / adc->adc0->getMaxValue()) - settings.offset1)) <  settings.CurDead)
      {
        RawCur = 0;
      }
      if (debugCur != 0)
      {
        SERIALCONSOLE.print("  ");
        SERIALCONSOLE.print(int16_t(value * 3300 / adc->adc0->getMaxValue()) - settings.offset1);
        SERIALCONSOLE.print("  ");
        SERIALCONSOLE.print(RawCur);
        SERIALCONSOLE.print(" mA");
        SERIALCONSOLE.print("  ");
      }
    }
    else
    {
      if (debugCur != 0)
      {
        SERIALCONSOLE.println();
        SERIALCONSOLE.print("High Range: ");
        SERIALCONSOLE.print("Value ADC0: ");
      }
      value = (uint16_t)adc->adc0->analogReadContinuous(); // the unsigned is necessary for 16 bits, otherwise values larger than 3.3/2 V are negative!
      if (debugCur != 0)
      {
        SERIALCONSOLE.print(value * 3300 / adc->adc0->getMaxValue() );//- settings.offset2)
        SERIALCONSOLE.print("  ");
        SERIALCONSOLE.print(settings.offset2);
      }
      RawCur = int16_t((value * 3300 / adc->adc0->getMaxValue()) - settings.offset2) / (settings.convhigh * 0.00001);
      if (value < 100 || value > (adc->adc0->getMaxValue() - 100))
      {
        RawCur = 0;
      }
      if (debugCur != 0)
      {
        SERIALCONSOLE.print("  ");
        SERIALCONSOLE.print((float(value * 3300 / adc->adc0->getMaxValue()) - settings.offset2));
        SERIALCONSOLE.print("  ");
        SERIALCONSOLE.print(RawCur);
        SERIALCONSOLE.print("mA");
        SERIALCONSOLE.print("  ");
      }
    }
  }

  if (settings.invertcur == 1)
  {
    RawCur = RawCur * -1;
  }

  lowpassFilter.input(RawCur);

  if (debugCur != 0)
  {
    SERIALCONSOLE.print(lowpassFilter.output());
    SERIALCONSOLE.print(" | ");
    SERIALCONSOLE.print(settings.changecur);
    SERIALCONSOLE.print(" | ");
  }

  currentact = lowpassFilter.output();

  if (debugCur != 0)
  {
    SERIALCONSOLE.print(currentact);
    SERIALCONSOLE.print("mA  ");
  }

  if ( settings.cursens == Analoguedual)
  {
    if (sensor == 1)
    {
      if (currentact > 500 || currentact < -500 )
      {
        ampsecond = ampsecond + ((currentact * (millis() - lasttime) / 1000) / 1000);
        lasttime = millis();
      }
      else
      {
        lasttime = millis();
      }
    }
    if (sensor == 2)
    {
      if (currentact > settings.changecur || currentact < (settings.changecur * -1) )
      {
        ampsecond = ampsecond + ((currentact * (millis() - lasttime) / 1000) / 1000);
        lasttime = millis();
      }
      else
      {
        lasttime = millis();
      }
    }
  }
  else
  {
    if (currentact > 500 || currentact < -500 )
    {
      ampsecond = ampsecond + ((currentact * (millis() - lasttime) / 1000) / 1000);
      lasttime = millis();
    }
    else
    {
      lasttime = millis();
    }
  }
  currentact = settings.ncur * currentact;
  RawCur = 0;
}


void updateSOC()
{
  if (SOCreset == 1)
  {
    SOC = map(uint16_t(bms.getLowCellVolt() * 1000), settings.socvolt[0], settings.socvolt[2], settings.socvolt[1], settings.socvolt[3]);
    ampsecond = (SOC * settings.CAP * settings.Pstrings * 10) / 0.27777777777778 ;
    SOCreset = 0;
  }

  if (SOCset == 0)
  {
    if (millis() > 8000)
    {
      if (SOCmem == 0)
      {
        SOC = map(uint16_t(bms.getLowCellVolt() * 1000), settings.socvolt[0], settings.socvolt[2], settings.socvolt[1], settings.socvolt[3]);
        ampsecond = (SOC * settings.CAP * settings.Pstrings * 10) / 0.27777777777778 ;
        if (debug != 0)
        {
          SERIALCONSOLE.println("  ");
          SERIALCONSOLE.println("//////////////////////////////////////// SOC SET ////////////////////////////////////////");
        }
      }
      SOCset = 1;
    }
  }
  if (settings.voltsoc == 1 || settings.cursens == 0)
  {
    SOC = map(uint16_t(bms.getLowCellVolt() * 1000), settings.socvolt[0], settings.socvolt[2], settings.socvolt[1], settings.socvolt[3]);

    ampsecond = (SOC * settings.CAP * settings.Pstrings * 10) / 0.27777777777778 ;
  }
  SOC = ((ampsecond * 0.27777777777778) / (settings.CAP * settings.Pstrings * 1000)) * 100;
  if (SOC >= 100)
  {
    ampsecond = (settings.CAP * settings.Pstrings * 1000) / 0.27777777777778 ; //reset to full, dependant on given capacity. Need to improve with auto correction for capcity.
    SOC = 100;
  }


  if (SOC < 0)
  {
    SOC = 0; //reset SOC this way the can messages remain in range for other devices. Ampseconds will keep counting.
  }

  if (debug != 0)
  {
    if (settings.cursens == Analoguedual)
    {
      if (sensor == 1)
      {
        SERIALCONSOLE.print("Low Range ");
      }
      else
      {
        SERIALCONSOLE.print("High Range");
      }
    }
    if (settings.cursens == Analoguesing)
    {
      SERIALCONSOLE.print("Analogue Single ");
    }
    if (settings.cursens == Canbus)
    {
      SERIALCONSOLE.print("CANbus ");
    }
    SERIALCONSOLE.print("  ");
    SERIALCONSOLE.print(currentact);
    SERIALCONSOLE.print("mA");
    SERIALCONSOLE.print("  ");
    SERIALCONSOLE.print(SOC);
    SERIALCONSOLE.print("% SOC ");
    SERIALCONSOLE.print(ampsecond * 0.27777777777778, 2);
    SERIALCONSOLE.print ("mAh");
  }
}

void SOCcharged(int y)
{
  if (y == 1)
  {
    SOC = 95;
    ampsecond = (settings.CAP * settings.Pstrings * 1000) / 0.27777777777778 ; //reset to full, dependant on given capacity. Need to improve with auto correction for capcity.
  }
  if (y == 2)
  {
    SOC = 100;
    ampsecond = (settings.CAP * settings.Pstrings * 1000) / 0.27777777777778 ; //reset to full, dependant on given capacity. Need to improve with auto correction for capcity.
  }
}

void contcon()
{
  if (contctrl != contstat) //check for contactor request change
  {
    if ((contctrl & 1) == 0)
    {
      analogWrite(OUT5, 0);
      contstat = contstat & 254;
    }
    if ((contctrl & 2) == 0)
    {
      analogWrite(OUT6, 0);
      contstat = contstat & 253;
    }
    if ((contctrl & 4) == 0)
    {
      analogWrite(OUT7, 0);
      contstat = contstat & 251;
    }


    if ((contctrl & 1) == 1)
    {
      if ((contstat & 1) != 1)
      {
        if (conttimer1 == 0)
        {
          analogWrite(OUT5, 255);
          conttimer1 = millis() + pulltime ;
        }
        if (conttimer1 < millis())
        {
          analogWrite(OUT5, settings.conthold);
          contstat = contstat | 1;
          conttimer1 = 0;
        }
      }
    }

    if ((contctrl & 2) == 2)
    {
      if ((contstat & 2) != 2)
      {
        if (conttimer2 == 0)
        {
          Serial.println();
          Serial.println("pull in OUT6");
          analogWrite(OUT6, 255);
          conttimer2 = millis() + pulltime ;
        }
        if (conttimer2 < millis())
        {
          analogWrite(OUT6, settings.conthold);
          contstat = contstat | 2;
          conttimer2 = 0;
        }
      }
    }
    if ((contctrl & 4) == 4)
    {
      if ((contstat & 4) != 4)
      {
        if (conttimer3 == 0)
        {
          Serial.println();
          Serial.println("pull in OUT7");
          analogWrite(OUT7, 255);
          conttimer3 = millis() + pulltime ;
        }
        if (conttimer3 < millis())
        {
          analogWrite(OUT7, settings.conthold);
          contstat = contstat | 4;
          conttimer3 = 0;
        }
      }
    }
    /*
       SERIALCONSOLE.print(conttimer);
       SERIALCONSOLE.print("  ");
       SERIALCONSOLE.print(contctrl);
       SERIALCONSOLE.print("  ");
       SERIALCONSOLE.print(contstat);
       SERIALCONSOLE.println("  ");
    */

  }
  if (contctrl == 0)
  {
    analogWrite(OUT5, 0);
    analogWrite(OUT6, 0);
  }
}

void calcur()
{
//  adc->adc0->startContinuous(ACUR1);
  sensor = 1;
  x = 0;
  SERIALCONSOLE.print(" Calibrating Current Offset ::::: ");
  while (x < 20)
  {
    settings.offset1 = settings.offset1 + ((uint16_t)adc->adc0->analogReadContinuous() * 3300 / adc->adc0->getMaxValue());
    SERIALCONSOLE.print(".");
    delay(100);
    x++;
  }
  settings.offset1 = settings.offset1 / 21;
  SERIALCONSOLE.print(settings.offset1);
  SERIALCONSOLE.print(" current offset 1 calibrated ");
  SERIALCONSOLE.println("  ");
  x = 0;
//  adc->adc0->startContinuous(ACUR2);
  sensor = 2;
  SERIALCONSOLE.print(" Calibrating Current Offset ::::: ");
  while (x < 20)
  {
    settings.offset2 = settings.offset2 + ((uint16_t)adc->adc0->analogReadContinuous() * 3300 / adc->adc0->getMaxValue());
    SERIALCONSOLE.print(".");
    delay(100);
    x++;
  }
  settings.offset2 = settings.offset2 / 21;
  SERIALCONSOLE.print(settings.offset2);
  SERIALCONSOLE.print(" current offset 2 calibrated ");
  SERIALCONSOLE.println("  ");
}

void VEcan() //communication with Victron system over CAN
{
  msg.id  = 0x351;
  msg.len = 8;
  if (storagemode == 0)
  {
    msg.buf[0] = lowByte(uint16_t((settings.ChargeVsetpoint * settings.Scells ) * 10));
    msg.buf[1] = highByte(uint16_t((settings.ChargeVsetpoint * settings.Scells ) * 10));
  }
  else
  {
    msg.buf[0] = lowByte(uint16_t((settings.StoreVsetpoint * settings.Scells ) * 10));
    msg.buf[1] = highByte(uint16_t((settings.StoreVsetpoint * settings.Scells ) * 10));
  }   
  msg.buf[2] = lowByte(chargecurrent);
  msg.buf[3] = highByte(chargecurrent);
  msg.buf[4] = lowByte(discurrent );
  msg.buf[5] = highByte(discurrent);
  msg.buf[6] = lowByte(uint16_t((settings.DischVsetpoint * settings.Scells) * 10));
  msg.buf[7] = highByte(uint16_t((settings.DischVsetpoint * settings.Scells) * 10));

  bmscan.write(msg, settings.veCanIndex);

    msg.id  = 0x355;
    msg.len = 8;
    msg.buf[0] = lowByte(SOC);
    msg.buf[1] = highByte(SOC);
    msg.buf[2] = lowByte(SOH);
    msg.buf[3] = highByte(SOH);
    msg.buf[4] = lowByte(SOC * 10);
    msg.buf[5] = highByte(SOC * 10);
    msg.buf[6] = 0;
    msg.buf[7] = 0;
   bmscan.write(msg, settings.veCanIndex);

  //Send Charge if in Precharge for VCU
  if (bmsstatus == Precharge) {
     msg.buf[6] = lowByte(Charge);
     msg.buf[7] = highByte(Charge);
  } else {
     msg.buf[6] = lowByte(bmsstatus);
     msg.buf[7] = highByte(bmsstatus);
  }

  bmscan.write(msg, settings.veCanIndex);

  msg.id  = 0x356;
  msg.len = 8;
  msg.buf[0] = lowByte(uint16_t(bms.getAvgPackVoltage() * 100));
  msg.buf[1] = highByte(uint16_t(bms.getAvgPackVoltage() * 100));
  msg.buf[2] = lowByte(long(currentact / 100));
  msg.buf[3] = highByte(long(currentact / 100));
  msg.buf[4] = lowByte(int16_t(bms.getAvgTemperature() * 10));
  msg.buf[5] = highByte(int16_t(bms.getAvgTemperature() * 10));
  msg.buf[6] = lowByte(uint16_t(bms.getAvgCellVolt() * 1000));
  msg.buf[7] = highByte(uint16_t(bms.getAvgCellVolt() * 1000));
  bmscan.write(msg, settings.veCanIndex);

  delay(2);
  msg.id  = 0x35A;
  msg.len = 8;
  msg.buf[0] = alarm[0];//High temp  Low Voltage | High Voltage
  msg.buf[1] = alarm[1]; // High Discharge Current | Low Temperature
  msg.buf[2] = alarm[2]; //Internal Failure | High Charge current
  msg.buf[3] = alarm[3];// Cell Imbalance
  msg.buf[4] = warning[0];//High temp  Low Voltage | High Voltage
  msg.buf[5] = warning[1];// High Discharge Current | Low Temperature
  msg.buf[6] = warning[2];//Internal Failure | High Charge current
  msg.buf[7] = warning[3];// Cell Imbalance
  bmscan.write(msg, settings.veCanIndex);

  msg.id  = 0x35E;
  msg.len = 8;
  msg.buf[0] = bmsname[0];
  msg.buf[1] = bmsname[1];
  msg.buf[2] = bmsname[2];
  msg.buf[3] = bmsname[3];
  msg.buf[4] = bmsname[4];
  msg.buf[5] = bmsname[5];
  msg.buf[6] = bmsname[6];
  msg.buf[7] = bmsname[7];
  bmscan.write(msg, settings.veCanIndex);

  delay(2);
  msg.id  = 0x370;
  msg.len = 8;
  msg.buf[0] = bmsmanu[0];
  msg.buf[1] = bmsmanu[1];
  msg.buf[2] = bmsmanu[2];
  msg.buf[3] = bmsmanu[3];
  msg.buf[4] = bmsmanu[4];
  msg.buf[5] = bmsmanu[5];
  msg.buf[6] = bmsmanu[6];
  msg.buf[7] = bmsmanu[7];
  bmscan.write(msg, settings.veCanIndex);

  delay(2);
  msg.id  = 0x373;
  msg.len = 8;
  msg.buf[0] = lowByte(uint16_t(bms.getLowCellVolt() * 1000));
  msg.buf[1] = highByte(uint16_t(bms.getLowCellVolt() * 1000));
  msg.buf[2] = lowByte(uint16_t(bms.getHighCellVolt() * 1000));
  msg.buf[3] = highByte(uint16_t(bms.getHighCellVolt() * 1000));
  msg.buf[4] = lowByte(uint16_t(bms.getLowTemperature() + 273.15));
  msg.buf[5] = highByte(uint16_t(bms.getLowTemperature() + 273.15));
  msg.buf[6] = lowByte(uint16_t(bms.getHighTemperature() + 273.15));
  msg.buf[7] = highByte(uint16_t(bms.getHighTemperature() + 273.15));
  bmscan.write(msg, settings.veCanIndex);

  delay(2);
  msg.id  = 0x379; //Installed capacity
  msg.len = 2;
  msg.buf[0] = lowByte(uint16_t(settings.Pstrings * settings.CAP));
  msg.buf[1] = highByte(uint16_t(settings.Pstrings * settings.CAP));
  /*
      delay(2);
    msg.id  = 0x378; //Installed capacity
    msg.len = 2;
    //energy in 100wh/unit
    msg.buf[0] =
    msg.buf[1] =
    msg.buf[2] =
    msg.buf[3] =
    //energy out 100wh/unit
    msg.buf[4] =
    msg.buf[5] =
    msg.buf[6] =
    msg.buf[7] =
  */
  delay(2);
  msg.id  = 0x372;
  msg.len = 8;
  msg.buf[0] = lowByte(bms.getNumModules());
  msg.buf[1] = highByte(bms.getNumModules());
  msg.buf[2] = 0x00;
  msg.buf[3] = 0x00;
  msg.buf[4] = 0x00;
  msg.buf[5] = 0x00;
  msg.buf[6] = 0x00;
  msg.buf[7] = 0x00;
  bmscan.write(msg, settings.veCanIndex);

}

// Settings menu
void menu()
{

  incomingByte = Serial.read(); // read the incoming byte:
  if (menuload == 4)
  {
    switch (incomingByte)
    {

      case '1':
        menuload = 1;
        candebug = !candebug;
        incomingByte = 'd';
        break;

      case '2':
        menuload = 1;
        debugCur = !debugCur;
        incomingByte = 'd';
        break;

      case '3':
        menuload = 1;
        outputcheck = !outputcheck;
        if (outputcheck == 0)
        {
          contctrl = 0;
          digitalWrite(OUT1, LOW);
          digitalWrite(OUT2, LOW);
          //digitalWrite(OUT3, LOW);
          digitalWrite(OUT4, LOW);
        }
        incomingByte = 'd';
        break;

      case '4':
        menuload = 1;
        inputcheck = !inputcheck;
        incomingByte = 'd';
        break;

      case '5':
        menuload = 1;
        settings.ESSmode = !settings.ESSmode;
        incomingByte = 'd';
        break;

      case '6':
        menuload = 1;
        cellspresent = bms.seriescells();
        incomingByte = 'd';
        break;

      case '7':
        menuload = 1;
        gaugedebug = !gaugedebug;
        incomingByte = 'd';
        break;

      case '8':
        menuload = 1;
        CSVdebug = !CSVdebug;
        incomingByte = 'd';
        break;

      case '9':
        menuload = 1;
        if (Serial.available() > 0)
        {
          debugdigits = Serial.parseInt();
        }
        if (debugdigits > 4)
        {
          debugdigits = 2;
        }
        incomingByte = 'd';
        break;

      case 113: //q for quite menu

        menuload = 0;
        incomingByte = 115;
        break;

      default:
        // if nothing else matches, do the default
        // default is optional
        break;
    }
  }

  if (menuload == 2)
  {
    switch (incomingByte)
    {


      case 99: //c for calibrate zero offset

        calcur();
        break;

      case '1':
        menuload = 1;
        settings.invertcur = !settings.invertcur;
        incomingByte = 'c';
        break;

      case '2':
        menuload = 1;
        settings.voltsoc = !settings.voltsoc;
        incomingByte = 'c';
        break;

      case '3':
        menuload = 1;
        if (Serial.available() > 0)
        {
          settings.ncur = Serial.parseInt();
        }
        menuload = 1;
        incomingByte = 'c';
        break;

      case '4':
        menuload = 1;
        if (Serial.available() > 0)
        {
          settings.convlow = Serial.parseInt();
        }
        incomingByte = 'c';
        break;

      case '5':
        menuload = 1;
        if (Serial.available() > 0)
        {
          settings.convhigh = Serial.parseInt();
        }
        incomingByte = 'c';
        break;

      case '6':
        menuload = 1;
        if (Serial.available() > 0)
        {
          settings.CurDead = Serial.parseInt();
        }
        incomingByte = 'c';
        break;

      case '8':
        menuload = 1;
        if (Serial.available() > 0)
        {
          settings.changecur = Serial.parseInt();
        }
        menuload = 1;
        incomingByte = 'c';
        break;

      case 113: //q for quite menu

        menuload = 0;
        incomingByte = 115;
        break;

      case 115: //s for switch sensor
        settings.cursens ++;
        if (settings.cursens > 4)
        {
          settings.cursens = 0;
        }
        /*
          if (settings.cursens == Analoguedual)
          {
            settings.cursens = Canbus;
            SERIALCONSOLE.println("  ");
            SERIALCONSOLE.print(" CANbus Current Sensor ");
            SERIALCONSOLE.println("  ");
          }
          else
          {
            settings.cursens = Analoguedual;
            SERIALCONSOLE.println("  ");
            SERIALCONSOLE.print(" Analogue Current Sensor ");
            SERIALCONSOLE.println("  ");
          }
        */
        menuload = 1;
        incomingByte = 'c';
        break;

      case '7': //s for switch sensor
        settings.curcan++;
        if (settings.curcan > CurCanMax) {
          settings.curcan = 1;
        }
        menuload = 1;
        incomingByte = 'c';
        break;

      default:
        // if nothing else matches, do the default
        // default is optional
        break;
    }
  }

  if (menuload == 8)
  {
    switch (incomingByte)
    {
      case '1': //e dispaly settings
        if (Serial.available() > 0)
        {
          settings.IgnoreTemp = Serial.parseInt();
        }
        if (settings.IgnoreTemp > 3)
        {
          settings.IgnoreTemp = 0;
        }
        bms.setSensors(settings.IgnoreTemp, settings.IgnoreVolt, settings.DeltaVolt);
        menuload = 1;
        incomingByte = 'i';
        break;

      case '2':
        if (Serial.available() > 0)
        {
          settings.IgnoreVolt = Serial.parseInt();
          settings.IgnoreVolt = settings.IgnoreVolt * 0.001;
          bms.setSensors(settings.IgnoreTemp, settings.IgnoreVolt, settings.DeltaVolt);
          // Serial.println(settings.IgnoreVolt);
          menuload = 1;
          incomingByte = 'i';
        }
        break;

      case 113: //q to go back to main menu

        menuload = 0;
        incomingByte = 115;
        break;
    }
  }



  if (menuload == 7)
  {
    switch (incomingByte)
    {
      case '1':
        if (Serial.available() > 0)
        {
          settings.WarnOff = Serial.parseInt();
          settings.WarnOff = settings.WarnOff * 0.001;
          menuload = 1;
          incomingByte = 'a';
        }
        break;

      case '2':
        if (Serial.available() > 0)
        {
          settings.CellGap = Serial.parseInt();
          settings.CellGap = settings.CellGap * 0.001;
          menuload = 1;
          incomingByte = 'a';
        }
        break;

      case '3':
        if (Serial.available() > 0)
        {
          settings.WarnToff = Serial.parseInt();
          menuload = 1;
          incomingByte = 'a';
        }
        break;

      case '4':
        if (Serial.available() > 0)
        {
          settings.triptime = Serial.parseInt();
          menuload = 1;
          incomingByte = 'a';
        }
        break;

      case 113: //q to go back to main menu
        menuload = 0;
        incomingByte = 115;
        break;
    }
  }

  if (menuload == 6) //Charging settings
  {
    switch (incomingByte)
    {

      case 113: //q to go back to main menu

        menuload = 0;
        incomingByte = 115;
        break;

      case '1':
        if (Serial.available() > 0)
        {
          settings.ChargeVsetpoint = Serial.parseInt();
          settings.ChargeVsetpoint = settings.ChargeVsetpoint / 1000;
          menuload = 1;
          incomingByte = 'e';
        }
        break;


      case '2':
        if (Serial.available() > 0)
        {
          settings.ChargeHys = Serial.parseInt();
          settings.ChargeHys = settings.ChargeHys / 1000;
          menuload = 1;
          incomingByte = 'e';
        }
        break;


      case '4':
        if (Serial.available() > 0)
        {
          settings.chargecurrentend = Serial.parseInt() * 10;
          menuload = 1;
          incomingByte = 'e';
        }
        break;


      case '3':
        if (Serial.available() > 0)
        {
          settings.chargecurrentmax = Serial.parseInt() * 10;
          menuload = 1;
          incomingByte = 'e';
        }
        break;

      case '5': //1 Over Voltage Setpoint
        settings.chargertype = settings.chargertype + 1;
        if (settings.chargertype > 8)
        {
          settings.chargertype = 0;
        }
        menuload = 1;
        incomingByte = 'e';
        break;

      case '6':
        if (Serial.available() > 0)
        {
          settings.chargerspd = Serial.parseInt();
          menuload = 1;
          incomingByte = 'e';
        }
        break;

      case '7':
        if ( settings.ChargerDirect == 1)
        {
          settings.ChargerDirect = 0;
        }
        else
        {
          settings.ChargerDirect = 1;
        }
        menuload = 1;
        incomingByte = 'e';
        break;

      case '9':
        if (Serial.available() > 0)
        {
          settings.ChargeTSetpoint = Serial.parseInt();
          if (settings.ChargeTSetpoint < settings.UnderTSetpoint)
          {
            settings.ChargeTSetpoint = settings.UnderTSetpoint;
          }
          menuload = 1;
          incomingByte = 'e';
        }
        break;
       case 'c':
        if (Serial.available() > 0)
        {
           #ifdef __MK66FX1M0__
          settings.chargerCanIndex++;
          #else
          settings.chargerCanIndex = settings.chargerCanIndex + 2;
          #endif
          if (settings.chargerCanIndex > 3) {
            settings.chargerCanIndex = 0;
          } else if (settings.chargerCanIndex < 0) {
            settings.chargerCanIndex = 0;
          }
          bmscan.begin(500000, settings.chargerCanIndex);
          menuload = 1;
          incomingByte = 'e';
        }
        break;
       case 'v':
        if (Serial.available() > 0)
        {
          #ifdef __MK66FX1M0__
          settings.veCanIndex++;
          #else
          settings.veCanIndex = settings.veCanIndex + 2;
          #endif
          if (settings.veCanIndex > 3) {
            settings.veCanIndex = 0;
          }
          bmscan.begin(500000, settings.veCanIndex);
          menuload = 1;
          incomingByte = 'e';
        }
        break;
        case 'o':
        if (Serial.available() > 0)
        {
          if (chargeOverride == 0) {
            chargeOverride = 1;
          } else {
            chargeOverride = 0;
          }
          menuload = 1;
          incomingByte = 'e';
        }
        break;
      case '0':
        if (Serial.available() > 0)
        {
          settings.chargecurrentcold = Serial.parseInt() * 10;
          if (settings.chargecurrentcold > settings.chargecurrentmax)
          {
            settings.chargecurrentcold = settings.chargecurrentmax;
          }
          menuload = 1;
          incomingByte = 'e';
        }
        break;

    }
  }
  if (menuload == 5)
  {
    switch (incomingByte)
    {
      case '1':
        if (Serial.available() > 0)
        {
          settings.Pretime = Serial.parseInt();
          menuload = 1;
          incomingByte = 'k';
        }
        break;

      case '2':
        if (Serial.available() > 0)
        {
          settings.Precurrent = Serial.parseInt();
          menuload = 1;
          incomingByte = 'k';
        }
        break;

      case '3':
        if (Serial.available() > 0)
        {
          settings.conthold = Serial.parseInt();
          menuload = 1;
          incomingByte = 'k';
        }
        break;

      case '4':
        if (Serial.available() > 0)
        {
          settings.gaugelow = Serial.parseInt();
          gaugedebug = 2;
          gaugeupdate();
          menuload = 1;
          incomingByte = 'k';
        }
        break;

      case '5':
        if (Serial.available() > 0)
        {
          settings.gaugehigh = Serial.parseInt();
          gaugedebug = 3;
          gaugeupdate();
          menuload = 1;
          incomingByte = 'k';
        }
        break;

      case '6':
        settings.tripcont = !settings.tripcont;
        if (settings.tripcont > 1)
        {
          settings.tripcont = 0;
        }
        menuload = 1;
        incomingByte = 'k';
        break;

      case 113: //q to go back to main menu
        gaugedebug = 0;
        menuload = 0;
        incomingByte = 115;
        break;
    }
  }

  if (menuload == 3)
  {
    switch (incomingByte)
    {
      case 113: //q to go back to main menu

        menuload = 0;
        incomingByte = 115;
        break;

      case 'f': //f factory settings
        loadSettings();
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.println(" Coded Settings Loaded ");
        SERIALCONSOLE.println("  ");
        menuload = 1;
        incomingByte = 'b';
        break;

      case 114: //r for reset
        SOCset = 0;
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print(" mAh Reset ");
        SERIALCONSOLE.println("  ");
        menuload = 1;
        incomingByte = 'b';
        break;




      case '1': //1 Over Voltage Setpoint
        if (Serial.available() > 0)
        {
          settings.OverVSetpoint = Serial.parseInt();
          settings.OverVSetpoint = settings.OverVSetpoint / 1000;
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case 'g':
        if (Serial.available() > 0)
        {
          settings.StoreVsetpoint = Serial.parseInt();
          settings.StoreVsetpoint = settings.StoreVsetpoint / 1000;
          menuload = 1;
          incomingByte = 'b';
        }

      case 'h':
        if (Serial.available() > 0)
        {
          settings.DisTaper = Serial.parseInt();
          settings.DisTaper = settings.DisTaper / 1000;
          menuload = 1;
          incomingByte = 'b';
        }

      case 'b':
        if (Serial.available() > 0)
        {
          settings.socvolt[0] = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;


      case 'c':
        if (Serial.available() > 0)
        {
          settings.socvolt[1] = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case 'd':
        if (Serial.available() > 0)
        {
          settings.socvolt[2] = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case 'e':
        if (Serial.available() > 0)
        {
          settings.socvolt[3] = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case 'k': //Discharge Voltage hysteresis
        if (Serial.available() > 0)
        {
          settings.DischHys = Serial.parseInt();
          settings.DischHys  = settings.DischHys  / 1000;
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case 'j':
        if (Serial.available() > 0)
        {
          settings.DisTSetpoint = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;
     case 'l': //secondary battery pack interface
        if (Serial.available() > 0)
        {
          settings.secondBatteryCanIndex++;
          #ifndef __MK66FX1M0__
          //teensy 3.2 doens't have the 2nd interface
          if (settings.secondBatteryCanIndex == 1) {
            settings.secondBatteryCanIndex++;
          }
          #endif
          if (settings.secondBatteryCanIndex > 3) {
            settings.secondBatteryCanIndex = 0;
          }
          bmscan.begin(500000, settings.secondBatteryCanIndex);
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case '9': //Discharge Voltage Setpoint
        if (Serial.available() > 0)
        {
          settings.DischVsetpoint = Serial.parseInt();
          settings.DischVsetpoint = settings.DischVsetpoint / 1000;
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case '0': //c Pstrings
        if (Serial.available() > 0)
        {
          settings.Pstrings = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
          bms.setPstrings(settings.Pstrings);
        }
        break;

      case 'a': //
        if (Serial.available() > 0)
        {
          settings.Scells  = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case '2': //2 Under Voltage Setpoint
        if (Serial.available() > 0)
        {
          settings.UnderVSetpoint = Serial.parseInt();
          settings.UnderVSetpoint =  settings.UnderVSetpoint / 1000;
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case '3': //3 Over Temperature Setpoint
        if (Serial.available() > 0)
        {
          settings.OverTSetpoint = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case '4': //4 Udner Temperature Setpoint
        if (Serial.available() > 0)
        {
          settings.UnderTSetpoint = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case '5': //5 Balance Voltage Setpoint
        if (Serial.available() > 0)
        {
          settings.balanceVoltage = Serial.parseInt();
          settings.balanceVoltage = settings.balanceVoltage / 1000;
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case '6': //6 Balance Voltage Hystersis
        if (Serial.available() > 0)
        {
          settings.balanceHyst = Serial.parseInt();
          settings.balanceHyst =  settings.balanceHyst / 1000;
          menuload = 1;
          bms.setBalanceHyst(settings.balanceHyst);
          incomingByte = 'b';
        }
        break;

      case '7'://7 Battery Capacity inAh
        if (Serial.available() > 0)
        {
          settings.CAP = Serial.parseInt();
          menuload = 1;
          incomingByte = 'b';
        }
        break;

      case '8':// discurrent in A
        if (Serial.available() > 0)
        {
          settings.discurrentmax = Serial.parseInt() * 10;
          menuload = 1;
          incomingByte = 'b';
        }
        break;

    }
  }

  if (menuload == 1)
  {
    switch (incomingByte)
    {
      case 'R'://restart
        CPU_REBOOT ;
        break;

      case 'i': //Ignore Value Settings
        while (Serial.available()) {
          Serial.read();
        }
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println("Ignore Value Settings");
        SERIALCONSOLE.print("1 - Temp Sensor Setting:");
        SERIALCONSOLE.println(settings.IgnoreTemp);
        SERIALCONSOLE.print("2 - Voltage Under Which To Ignore Cells:");
        SERIALCONSOLE.print(settings.IgnoreVolt * 1000, 0);
        SERIALCONSOLE.println("mV");
        SERIALCONSOLE.println("q - Go back to menu");
        menuload = 8;
        break;

      case 'e': //Charging settings
        while (Serial.available()) {
          Serial.read();
        }
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println("Charging Settings");
        SERIALCONSOLE.print("1 - Cell Charge Voltage Limit Setpoint: ");
        SERIALCONSOLE.print(settings.ChargeVsetpoint * 1000, 0);
        SERIALCONSOLE.println("mV");
        SERIALCONSOLE.print("2 - Charge Hystersis: ");
        SERIALCONSOLE.print(settings.ChargeHys * 1000, 0 );
        SERIALCONSOLE.println("mV");
        if (settings.chargertype > 0)
        {
          SERIALCONSOLE.print("3 - Pack Max Charge Current: ");
          SERIALCONSOLE.print(settings.chargecurrentmax * 0.1);
          SERIALCONSOLE.println("A");
          SERIALCONSOLE.print("4 - Pack End of Charge Current: ");
          SERIALCONSOLE.print(settings.chargecurrentend * 0.1);
          SERIALCONSOLE.println("A");
        }
        SERIALCONSOLE.print("5- Charger Type: ");
        switch (settings.chargertype)
        {
          case 0:
            SERIALCONSOLE.print("Relay Control");
            break;
          case 1:
            SERIALCONSOLE.print("Brusa NLG5xx");
            break;
          case 2:
            SERIALCONSOLE.print("Volt Charger");
            break;
          case 3:
            SERIALCONSOLE.print("Eltek Charger");
            break;
          case 4:
            SERIALCONSOLE.print("Elcon Charger");
            break;
          case 5:
            SERIALCONSOLE.print("Victron/SMA");
            break;
          case 6:
            SERIALCONSOLE.print("Coda");
            break;
            break;
          case 7:
            SERIALCONSOLE.print("Eltek PC Charger");
            break;
          case 8:
            SERIALCONSOLE.print("Mitsubish Outlander Charger");
            break;
        }
        SERIALCONSOLE.println();
        if (settings.chargertype > 0)
        {
          SERIALCONSOLE.print("6 - Charger Can Msg Spd: ");
          SERIALCONSOLE.print(settings.chargerspd);
          SERIALCONSOLE.println("mS");
          SERIALCONSOLE.println();
        }
        /*
          SERIALCONSOLE.print("7- Can Speed:");
          SERIALCONSOLE.print(settings.canSpeed/1000);
          SERIALCONSOLE.println("kbps");
        */
        SERIALCONSOLE.print("7 - Charger HV Connection: ");
        switch (settings.ChargerDirect)
        {
          case 0:
            SERIALCONSOLE.print(" Behind Contactors");
            break;
          case 1:
            SERIALCONSOLE.print("Direct To Battery HV");
            break;
        }
        SERIALCONSOLE.println();

        SERIALCONSOLE.print("9 - Charge Current derate Low: ");
        SERIALCONSOLE.print(settings.ChargeTSetpoint);
        SERIALCONSOLE.println(" C");
        SERIALCONSOLE.print("c - Charger Can Interface Index: ");
        switch (settings.chargerCanIndex)
        {
          case 0:
            SERIALCONSOLE.print("Can0");
            break;
          case 1:
            SERIALCONSOLE.print("Can1");
            break;
          case 2:
            SERIALCONSOLE.print("SPI");
            break;
          case 3:
            SERIALCONSOLE.print("SPI1");
            break;
        }
        SERIALCONSOLE.println();

        SERIALCONSOLE.print("v - Status (VE CAN) Can Interface Index: ");
        switch (settings.veCanIndex)
        {
          case 0:
            SERIALCONSOLE.print("Can0");
            break;
          case 1:
            SERIALCONSOLE.print("Can1");
            break;
          case 2:
            SERIALCONSOLE.print("SPI");
            break;
          case 3:
            SERIALCONSOLE.print("SPI1");
            break;
        }
        SERIALCONSOLE.println();

        SERIALCONSOLE.print("0 - Pack Cold Charge Current: ");
        SERIALCONSOLE.print(settings.chargecurrentcold * 0.1);
        SERIALCONSOLE.println("A");

        SERIALCONSOLE.print("o - Override AC present: ");
        if (chargeOverride == true) {
          SERIALCONSOLE.print("ON");
        } else {
          SERIALCONSOLE.print("OFF");
        }
        SERIALCONSOLE.println();
        SERIALCONSOLE.println("q - Go back to menu");
        menuload = 6;
        break;
        
        SERIALCONSOLE.println();
        
      case 'a': //Alarm and Warning settings
        while (Serial.available()) {
          Serial.read();
        }
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println("Alarm and Warning Settings Menu");
        SERIALCONSOLE.print("1 - Voltage Warning Offset: ");
        SERIALCONSOLE.print(settings.WarnOff * 1000, 0);
        SERIALCONSOLE.println("mV");
        SERIALCONSOLE.print("2 - Cell Voltage Difference Alarm: ");
        SERIALCONSOLE.print(settings.CellGap * 1000, 0);
        SERIALCONSOLE.println("mV");
        SERIALCONSOLE.print("3 - Temp Warning Offset: ");
        SERIALCONSOLE.print(settings.WarnToff);
        SERIALCONSOLE.println(" C");
        /*
          SERIALCONSOLE.print("4 - Temp Warning Offset: ");
          SERIALCONSOLE.print(settings.UnderDur);
          SERIALCONSOLE.println(" mS");
        */
        SERIALCONSOLE.print("4 - Over and Under Voltage Delay: ");
        SERIALCONSOLE.print(settings.triptime);
        SERIALCONSOLE.println(" mS");

        menuload = 7;
        break;

      case 'k': //contactor settings
        while (Serial.available()) {
          Serial.read();
        }
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println("Contactor and Gauge Settings Menu");
        SERIALCONSOLE.print("1 - PreCharge Timer: ");
        SERIALCONSOLE.print(settings.Pretime);
        SERIALCONSOLE.println("mS");
        SERIALCONSOLE.print("2 - PreCharge Finish Current: ");
        SERIALCONSOLE.print(settings.Precurrent);
        SERIALCONSOLE.println(" mA");
        SERIALCONSOLE.print("3 - PWM contactor Hold 0-255 :");
        SERIALCONSOLE.println(settings.conthold);
        SERIALCONSOLE.print("4 - PWM for Gauge Low 0-255 :");
        SERIALCONSOLE.println(settings.gaugelow);
        SERIALCONSOLE.print("5 - PWM for Gauge High 0-255 :");
        SERIALCONSOLE.println(settings.gaugehigh);

        if (settings.ESSmode == 1)
        {
          SERIALCONSOLE.print("6 - ESS Main Contactor or Trip :");
          if (settings.tripcont == 0)
          {
            SERIALCONSOLE.println( "Trip Shunt");
          }
          else
          {
            SERIALCONSOLE.println( "Main Contactor and Precharge");
          }
        }

        menuload = 5;
        break;

      case 113: //q to go back to main menu
        EEPROM.put(0, settings); //save all change to eeprom
        menuload = 0;
        debug = 1;
        break;
      case 'd': //d for debug settings
        while (Serial.available()) {
          Serial.read();
        }
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println("Debug Settings Menu");
        SERIALCONSOLE.println("Toggle on/off");
        SERIALCONSOLE.print("1 - Can Debug :");
        SERIALCONSOLE.println(candebug);
        SERIALCONSOLE.print("2 - Current Debug :");
        SERIALCONSOLE.println(debugCur);
        SERIALCONSOLE.print("3 - Output Check :");
        SERIALCONSOLE.println(outputcheck);
        SERIALCONSOLE.print("4 - Input Check :");
        SERIALCONSOLE.println(inputcheck);
        SERIALCONSOLE.print("5 - ESS mode :");
        SERIALCONSOLE.println(settings.ESSmode);
        SERIALCONSOLE.print("6 - Cells Present Reset :");
        SERIALCONSOLE.println(cellspresent);
        SERIALCONSOLE.print("7 - Gauge Debug :");
        SERIALCONSOLE.println(gaugedebug);
        SERIALCONSOLE.print("8 - CSV Output :");
        SERIALCONSOLE.println(CSVdebug);
        SERIALCONSOLE.print("9 - Decimal Places to Show :");
        SERIALCONSOLE.println(debugdigits);
        SERIALCONSOLE.println("q - Go back to menu");
        menuload = 4;
        break;

      case 99: //c for calibrate zero offset
        while (Serial.available()) {
          Serial.read();
        }
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println("Current Sensor Calibration Menu");
        SERIALCONSOLE.println("c - To calibrate sensor offset");
        SERIALCONSOLE.print("s - Current Sensor Type : ");
        switch (settings.cursens)
        {
          case Analoguedual:
            SERIALCONSOLE.println(" Analogue Dual Current Sensor ");
            break;
          case Analoguesing:
            SERIALCONSOLE.println(" Analogue Single Current Sensor ");
            break;
          case Canbus:
            SERIALCONSOLE.println(" Canbus Current Sensor ");
            break;
          case TeslaP100D:
            SERIALCONSOLE.println("  TESLA P100D SPI Current Sensor ");
            break;
          default:
            SERIALCONSOLE.println("Undefined");
            break;
        }
        SERIALCONSOLE.print("1 - invert current :");
        SERIALCONSOLE.println(settings.invertcur);
        SERIALCONSOLE.print("2 - Pure Voltage based SOC :");
        SERIALCONSOLE.println(settings.voltsoc);
        SERIALCONSOLE.print("3 - Current Multiplication :");
        SERIALCONSOLE.println(settings.ncur);
        if (settings.cursens == Analoguesing || settings.cursens == Analoguedual)
        {
          SERIALCONSOLE.print("4 - Analogue Low Range Conv:");
          SERIALCONSOLE.print(settings.convlow * 0.1, 1);
          SERIALCONSOLE.println(" mV/A");
        }
        if ( settings.cursens == Analoguedual)
        {
          SERIALCONSOLE.print("5 - Analogue High Range Conv:");
          SERIALCONSOLE.print(settings.convhigh * 0.1, 1);
          SERIALCONSOLE.println(" mV/A");
        }
        if (settings.cursens == Analoguesing || settings.cursens == Analoguedual)
        {
          SERIALCONSOLE.print("6 - Current Sensor Deadband:");
          SERIALCONSOLE.print(settings.CurDead);
          SERIALCONSOLE.println(" mV");
        }
        if ( settings.cursens == Analoguedual)
        {

          SERIALCONSOLE.print("8 - Current Channel ChangeOver:");
          SERIALCONSOLE.print(settings.changecur * 0.001);
          SERIALCONSOLE.println(" A");
        }
        if ( settings.cursens == Canbus)
        {
          SERIALCONSOLE.print("7 -Can Current Sensor :");
          if (settings.curcan == LemCAB300)
          {
            SERIALCONSOLE.println(" LEM CAB300/500 series ");
          }
          else  if (settings.curcan == LemCAB500)
          {
            SERIALCONSOLE.println(" LEM CAB500 Special ");
          }
          else if (settings.curcan == IsaScale)
          {
            SERIALCONSOLE.println(" IsaScale IVT-S ");
          }
          else if (settings.curcan == VictronLynx)
          {
            SERIALCONSOLE.println(" Victron Lynx VE.CAN Shunt");
          }
        }
        SERIALCONSOLE.println("q - Go back to menu");
        menuload = 2;
        break;

      case 98: //c for calibrate zero offset
        while (Serial.available())
        {
          Serial.read();
        }
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.println("Battery Settings Menu");
        SERIALCONSOLE.println("r - Reset AH counter");
        SERIALCONSOLE.println("f - Reset to Coded Settings");
        SERIALCONSOLE.println("q - Go back to menu");
        SERIALCONSOLE.println();
        SERIALCONSOLE.println();
        SERIALCONSOLE.print("1 - Cell Over Voltage Setpoint: ");
        SERIALCONSOLE.print(settings.OverVSetpoint * 1000, 0);
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("2 - Cell Under Voltage Setpoint: ");
        SERIALCONSOLE.print(settings.UnderVSetpoint * 1000, 0);
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("3 - Over Temperature Setpoint: ");
        SERIALCONSOLE.print(settings.OverTSetpoint);
        SERIALCONSOLE.print("C");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("4 - Under Temperature Setpoint: ");
        SERIALCONSOLE.print(settings.UnderTSetpoint);
        SERIALCONSOLE.print("C");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("5 - Cell Balance Voltage Setpoint: ");
        SERIALCONSOLE.print(settings.balanceVoltage * 1000, 0);
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("6 - Balance Voltage Hystersis: ");
        SERIALCONSOLE.print(settings.balanceHyst * 1000, 0);
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("7 - Ah Battery Capacity: ");
        SERIALCONSOLE.print(settings.CAP);
        SERIALCONSOLE.print("Ah");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("8 - Pack Max Discharge: ");
        SERIALCONSOLE.print(settings.discurrentmax * 0.1);
        SERIALCONSOLE.print("A");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("9 - Cell Discharge Voltage Limit Setpoint: ");
        SERIALCONSOLE.print(settings.DischVsetpoint * 1000, 0);
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("0 - Slave strings in parallel: ");
        SERIALCONSOLE.print(settings.Pstrings);
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("a - Cells in Series per String: ");
        SERIALCONSOLE.print(settings.Scells );
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("b - setpoint 1: ");
        SERIALCONSOLE.print(settings.socvolt[0] );
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("c - SOC setpoint 1:");
        SERIALCONSOLE.print(settings.socvolt[1] );
        SERIALCONSOLE.print("%");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("d - setpoint 2: ");
        SERIALCONSOLE.print(settings.socvolt[2] );
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("e - SOC setpoint 2: ");
        SERIALCONSOLE.print(settings.socvolt[3] );
        SERIALCONSOLE.print("%");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("g - Storage Setpoint: ");
        SERIALCONSOLE.print(settings.StoreVsetpoint * 1000, 0 );
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("h - Discharge Current Taper Offset: ");
        SERIALCONSOLE.print(settings.DisTaper * 1000, 0 );
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("j - Discharge Current Temperature Derate : ");
        SERIALCONSOLE.print(settings.DisTSetpoint);
        SERIALCONSOLE.print("C");
        SERIALCONSOLE.println("  ");
        SERIALCONSOLE.print("k - Cell Discharge Voltage Hysteresis: ");
        SERIALCONSOLE.print(settings.DischHys * 1000, 0);
        SERIALCONSOLE.print("mV");
        SERIALCONSOLE.println();
        SERIALCONSOLE.print("l - Secondary Battery Pack Can Interface: ");
        switch (settings.secondBatteryCanIndex)
        {
          case 0:
            SERIALCONSOLE.print("No seconary Battery Pack");
            break;
          case 1:
            SERIALCONSOLE.print("Can1");
            break;
          case 2:
            SERIALCONSOLE.print("SPI");
            break;
          case 3:
            SERIALCONSOLE.print("SPI1");
            break;
        }
        SERIALCONSOLE.println();
        menuload = 3;
        break;

      default:
        // if nothing else matches, do the default
        // default is optional
        break;
    }
  }

  if (incomingByte == 115 & menuload == 0)
  {
    SERIALCONSOLE.println();
    SERIALCONSOLE.println("MENU");
    SERIALCONSOLE.println("Debugging Paused");
    SERIALCONSOLE.print("Firmware Version : ");
    SERIALCONSOLE.println(firmver);
    SERIALCONSOLE.println("b - Battery Settings");
    SERIALCONSOLE.println("a - Alarm and Warning Settings");
    SERIALCONSOLE.println("e - Charging Settings");
    SERIALCONSOLE.println("c - Current Sensor Calibration");
    SERIALCONSOLE.println("k - Contactor and Gauge Settings");
    SERIALCONSOLE.println("i - Ignore Value Settings");
    SERIALCONSOLE.println("d - Debug Settings");
    SERIALCONSOLE.println("q - exit menu");
    debug = 0;
    menuload = 1;
  }
}

//ID offset is only applied to battery module frames
void canread(int canInterfaceOffset, int idOffset)
{
  bmscan.read(inMsg, canInterfaceOffset);


  // Read data: len = data length, buf = data byte(s)
//  if ( settings.cursens == Canbus)
//  {
//
//    if (settings.curcan == 1)
//    {
//      switch (inMsg.id)
//      {
//        case 0x3c1:
//          CAB500();
//          break;
//
//        case 0x3c2:
//          CAB300();
//          break;
//
//        default:
//          break;
//      }
//    }
//    if (settings.curcan == 2)
//    {
//      switch (inMsg.id)
//      {
//        case 0x3c1:
//          CAB500();
//          break;
//
//        case 0x3c2:
//          CAB500();
//          break;
//
//        default:
//          break;
//      }
//    }
//    if (settings.curcan == 3)
//    {
//
//      //Wont match in the switch statement below for some reason
//      if (inMsg.id == 0x527) {
//        long ampseconds = inMsg.buf[2] + (inMsg.buf[3] << 8) + (inMsg.buf[4] << 16) + (inMsg.buf[5] << 24);
//        amphours = ampseconds/3600.0f;
//      }
//      switch (inMsg.id)
//      {
//        case 0x521: //
//          CANmilliamps = inMsg.buf[2] + (inMsg.buf[3] << 8) + (inMsg.buf[4] << 16) + (inMsg.buf[5] << 24);
//          RawCur = CANmilliamps; 
//          getcurrent();
//          break;
//        case 0x522: //
//          voltage1 = inMsg.buf[2] + (inMsg.buf[3] << 8) + (inMsg.buf[4] << 16) + (inMsg.buf[5] << 24);
//          break;
//        case 0x523: //
//          voltage2 = inMsg.buf[2] + (inMsg.buf[3] << 8) + (inMsg.buf[4] << 16) + (inMsg.buf[5] << 24);
//          break;
//        case 0x526: 
//          long watt = inMsg.buf[2] + (inMsg.buf[3] << 8) + (inMsg.buf[4] << 16) + (inMsg.buf[5] << 24);
//          kilowatts = watt/1000.0f;
//          break;
//        case 0x527: 
//          long ampseconds = inMsg.buf[2] + (inMsg.buf[3] << 8) + (inMsg.buf[4] << 16) + (inMsg.buf[5] << 24);
//          Serial.println("-----------------------");
//          amphours = ampseconds/3600.0f;
//          break;
//        case 0x528: 
//          long wh = inMsg.buf[2] + (inMsg.buf[3] << 8) + (inMsg.buf[4] << 16) + (inMsg.buf[5] << 24);
//          kilowatthours = wh/1000.0f;
//          break;
//      }
//
//    }
//    if (settings.curcan == 4)
//    {
//      if (pgnFromCANId(inMsg.id) == 0x1F214 && inMsg.buf[0] == 0) // Check PGN and only use the first packet of each sequence
//      {
//        handleVictronLynx();
//      }
//    }
//  }


  if (inMsg.id < 0x300 && inMsg.id > 0x20)//do VW BMS magic if ids are ones identified to be modules
  {
    inMsg.id = inMsg.id + idOffset;
    if (candebug == 1)
    {
      bms.decodecan(inMsg, 1); //do VW BMS if ids are ones identified to be modules
    }
    else
    {
      bms.decodecan(inMsg, 0); //do VW BMS if ids are ones identified to be modules
    }
  }

//  if ((inMsg.id & 0x1FFFFFFF) < 0x1A5554F0 && (inMsg.id & 0x1FFFFFFF) > 0x1A555400)   // Determine if ID is Temperature CAN-ID
  if ((inMsg.id >= 0x1A555401 && inMsg.id <= 0x1A555408) || (inMsg.id >= 0x1A555421 && inMsg.id <= 0x1A555428))

  {

    inMsg.id = inMsg.id + idOffset/4; // the temps only require offsetting id by 8 (1/4 of 32) i.e. 1 can id per slave. 
    if (candebug == 1)
    {
      bms.decodetemp(inMsg, 1);

    }
    else
    {
      bms.decodetemp(inMsg, 0);
    }

  }

//moved to filtered
//  if (settings.chargerCanIndex == canInterfaceOffset && settings.chargertype == Outlander) {
//    if (inMsg.id == 0x389) {
//      outlander_charger_reported_voltage = inMsg.buf[0] * 2;
//      outlander_charger_reported_current = inMsg.buf[2];
//      outlander_charger_reported_temp1 = inMsg.buf[3] - 40;
//      outlander_charger_reported_temp2 = inMsg.buf[4] - 40;
//
//    } else if (inMsg.id == 0x38A) {
//       outlander_charger_reported_status = inMsg.buf[4];
//       evse_duty = inMsg.buf[3];
//    }
//  }

//  if (settings.veCanIndex == canInterfaceOffset) {
//    //from inverter
//    if (inMsg.id == 0x02) {
//      inverterLastRec = millis();
//      inverterStatus = inMsg.buf[0];
//    }
//    //canio
//    if (inMsg.id == 0x01) {
//      inverterInDrive = inMsg.buf[1] & 0x80 == 0x80;
//    }
//    //chademo
//    if (inMsg.id == 0x354 && inMsg.buf[0] == 0x01) {
//      rapidCharging = true;
//    }
//  }

  if (candebug == 1)
  {
    Serial.print(millis());
    if ((inMsg.id & 0x80000000) == 0x80000000)    // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (inMsg.id & 0x1FFFFFFF), inMsg.len);
    else
      sprintf(msgString, ",0x%.3lX,false,%1d", inMsg.id, inMsg.len);

    Serial.print(msgString);

    if ((inMsg.id & 0x40000000) == 0x40000000) {  // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for (byte i = 0; i < inMsg.len; i++) {
        sprintf(msgString, ", 0x%.2X", inMsg.buf[i]);
        Serial.print(msgString);
      }
    }
    Serial.print(" Can Interface: ");
    Serial.print(canInterfaceOffset);
    Serial.println();
  }
}

void CAB300()
{
  for (int i = 0; i < 4; i++)
  {
    inbox = (inbox << 8) | inMsg.buf[i];
  }
  CANmilliamps = inbox;
  if (CANmilliamps > 0x80000000)
  {
    CANmilliamps -= 0x80000000;
  }
  else
  {
    CANmilliamps = (0x80000000 - CANmilliamps) * -1;
  }
  if (settings.cursens == Canbus)
  {
    RawCur = CANmilliamps;
    getcurrent();
  }
  if (candebug == 1)
  {
    Serial.println();
    Serial.print(CANmilliamps);
    Serial.print("mA ");
  }
}

void CAB500()
{
  inbox = 0;
  for (int i = 1; i < 4; i++)
  {
    inbox = (inbox << 8) | inMsg.buf[i];
  }
  CANmilliamps = inbox;
  if (candebug == 1)
  {
    Serial.println();
    Serial.print(CANmilliamps, HEX);
  }
  if (CANmilliamps > 0x800000)
  {
    CANmilliamps -= 0x800000;
  }
  else
  {
    CANmilliamps = (0x800000 - CANmilliamps) * -1;
  }
  if ( settings.cursens == Canbus)
  {
    RawCur = CANmilliamps;
    getcurrent();
  }
  if (candebug == 1)
  {
    Serial.println();
    Serial.print(CANmilliamps);
    Serial.print("mA ");
  }
}

void handleVictronLynx()
{
  if (inMsg.buf[4] == 0xff && inMsg.buf[3] == 0xff) return;
  int16_t current = (int)inMsg.buf[4] << 8; // in 0.1A increments
  current |= inMsg.buf[3];
  CANmilliamps = current * 100;
  if (settings.cursens == Canbus)
  {
    RawCur = CANmilliamps;
    getcurrent();
  }
  if (candebug == 1)
  {
    Serial.println();
    Serial.print(CANmilliamps);
    Serial.print("mA ");
  }
}


void currentlimit()
{
  if (bmsstatus == Error)
  {
    discurrent = 0;
    chargecurrent = 0;
  }
  /*
    settings.PulseCh = 600; //Peak Charge current in 0.1A
    settings.PulseChDur = 5000; //Ms of discharge pulse derating
    settings.PulseDi = 600; //Peak Charge current in 0.1A
    settings.PulseDiDur = 5000; //Ms of discharge pulse derating
  */
  else
  {

    ///Start at no derating///
    discurrent = settings.discurrentmax;
    int maxchargingcurrent;
    if (bmsstatus == RapidCharge) {
       maxchargingcurrent = chargecurrent = settings.rapidchargecurrentmax;
    } else {
       maxchargingcurrent = chargecurrent = settings.chargecurrentmax;
    }


    ///////All hard limits to into zeros
    if (bms.getLowTemperature() < settings.UnderTSetpoint)
    {
      //discurrent = 0; Request Daniel
      chargecurrent = settings.chargecurrentcold;
    }
    if (bms.getHighTemperature() > settings.OverTSetpoint)
    {
      discurrent = 0;
      chargecurrent = 0;
    }
    if (bms.getHighCellVolt() > settings.OverVSetpoint)
    {
      chargecurrent = 0;
    }
    if (bms.getHighCellVolt() > settings.OverVSetpoint)
    {
      chargecurrent = 0;
    }
    if (bms.getLowCellVolt() < settings.UnderVSetpoint || bms.getLowCellVolt() < settings.DischVsetpoint)
    {
      discurrent = 0;
    }


    //Modifying discharge current///

    if (discurrent > 0)
    {
      //Temperature based///

      if (bms.getHighTemperature() > settings.DisTSetpoint)
      {
        discurrent = discurrent - map(bms.getHighTemperature(), settings.DisTSetpoint, settings.OverTSetpoint, 0, settings.discurrentmax);
      }
      //Voltagee based///
      if (bms.getLowCellVolt() < (settings.DischVsetpoint + settings.DisTaper))
      {
        discurrent = discurrent - map(bms.getLowCellVolt(), settings.DischVsetpoint, (settings.DischVsetpoint + settings.DisTaper), settings.discurrentmax, 0);
      }

    }

    //Modifying Charge current///
    if (chargecurrent > settings.chargecurrentcold)
    {
      //Temperature based///
      if (bms.getLowTemperature() < settings.ChargeTSetpoint)
      {
        chargecurrent = chargecurrent - map(bms.getLowTemperature(), settings.UnderTSetpoint, settings.ChargeTSetpoint, (maxchargingcurrent - settings.chargecurrentcold), 0);
      }
      //Voltagee based///
      if (storagemode == 1)
      {
        if (bms.getHighCellVolt() > (settings.StoreVsetpoint - settings.ChargeHys))
        {
          chargecurrent = chargecurrent - map(bms.getHighCellVolt(), (settings.StoreVsetpoint - settings.ChargeHys), settings.StoreVsetpoint, settings.chargecurrentend, maxchargingcurrent);
        }
      }
      else
      {
        if (bms.getHighCellVolt() > (settings.ChargeVsetpoint - settings.ChargeHys))
        {
          chargecurrent = chargecurrent - map(bms.getHighCellVolt(), (settings.ChargeVsetpoint - settings.ChargeHys), settings.ChargeVsetpoint, 0, (maxchargingcurrent - settings.chargecurrentend));
        }
      }
    }

  }

  //extra safety check
  if (settings.chargertype == Outlander) {
    uint16_t fullVoltage = uint16_t(settings.ChargeVsetpoint * settings.Scells * 10);
    if (outlander_charger_reported_voltage > fullVoltage) {
//      chargecurrent = 0;
    }
    
  }
  
  ///No negative currents///

  if (discurrent < 0)
  {
    discurrent = 0;
  }
  if (chargecurrent < 0)
  {
    chargecurrent = 0;
  }
}



void inputdebug()
{
  Serial.println();
  Serial.print("Input: ");
  if (digitalRead(IN1))
  {
    Serial.print("1 ON  ");
  }
  else
  {
    Serial.print("1 OFF ");
  }
  if (digitalRead(IN3))
  {
    Serial.print("2 ON  ");
  }
  else
  {
    Serial.print("2 OFF ");
  }
  if (digitalRead(IN3))
  {
    Serial.print("3 ON  ");
  }
  else
  {
    Serial.print("3 OFF ");
  }
  if (digitalRead(IN4))
  {
    Serial.print("4 ON  ");
  }
  else
  {
    Serial.print("4 OFF ");
  }
  Serial.println();
}

void resetISACounters() {
  msg.id  = 0x411;
  msg.len = 8;
  msg.buf[0] = 0x3F;
  msg.buf[1] = 0x00;
  msg.buf[2] = 0x00;
  msg.buf[3] = 0x00;
  msg.buf[4] = 0x00;
  msg.buf[5] = 0x00;
  msg.buf[6] = 0x00;
  msg.buf[7] = 0x00;
  bmscan.write(msg, settings.veCanIndex);
}

void outputdebug()
{
  if (outputstate < 5)
  {
    digitalWrite(OUT1, HIGH);
    digitalWrite(OUT2, HIGH);
//    digitalWrite(OUT3, HIGH);
    digitalWrite(OUT4, HIGH);
    analogWrite(OUT5, 255);
    analogWrite(OUT6, 255);
    analogWrite(OUT7, 255);
    analogWrite(OUT8, 255);
    outputstate ++;
  }
  else
  {
    digitalWrite(OUT1, LOW);
    digitalWrite(OUT2, LOW);
//    digitalWrite(OUT3, LOW);
    digitalWrite(OUT4, LOW);
    analogWrite(OUT5, 0);
    analogWrite(OUT6, 0);
    analogWrite(OUT7, 0);
    analogWrite(OUT8, 0);
    outputstate ++;
  }
  if (outputstate > 10)
  {
    outputstate = 0;
  }
}

void sendcommand()
{
  msg.id  = controlid;
  msg.len = 8;
  msg.buf[0] = 0x00;
  msg.buf[1] = 0x00;
  msg.buf[2] = 0x00;
  msg.buf[3] = 0x00;
  msg.buf[4] = 0x00;
  msg.buf[5] = 0x00;
  msg.buf[6] = 0x00;
  msg.buf[7] = 0x00;
  bmscan.write(msg, DEFAULT_CAN_INTERFACE_INDEX);
  if (settings.secondBatteryCanIndex != DEFAULT_CAN_INTERFACE_INDEX) {
      bmscan.write(msg, settings.secondBatteryCanIndex);
  }
  delay(1);
  msg.id  = controlid;
  msg.len = 8;
  msg.buf[0] = 0x45;
  msg.buf[1] = 0x01;
  msg.buf[2] = 0x28;
  msg.buf[3] = 0x00;
  msg.buf[4] = 0x00;
  msg.buf[5] = 0x00;
  msg.buf[6] = 0x00;
  msg.buf[7] = 0x30;
  bmscan.write(msg, DEFAULT_CAN_INTERFACE_INDEX);
  if (settings.secondBatteryCanIndex != DEFAULT_CAN_INTERFACE_INDEX) {
      bmscan.write(msg, settings.secondBatteryCanIndex);
  }
}

void resetwdog()
{
  noInterrupts();                                     //   No - reset WDT
  WDOG_REFRESH = 0xA602;
  WDOG_REFRESH = 0xB480;
  interrupts();
}

void pwmcomms()
{
  int p = 0;
  p = map((currentact * 0.001), pwmcurmin, pwmcurmax, 50 , 255);
  analogWrite(OUT7, p);
  /*
    Serial.println();
      Serial.print(p*100/255);
      Serial.print(" OUT8 ");
  */
  if (bms.getLowCellVolt() < settings.UnderVSetpoint)
  {
    analogWrite(OUT8, 224); //12V to 10V converter 1.5V
  }
  else
  {
    p = map(SOC, 0, 100, 220, 50);
    analogWrite(OUT8, p); //2V to 10V converter 1.5-10V
  }
  /*
      Serial.println();
      Serial.print(p*100/255);
      Serial.print(" OUT7 ");
  */
}

void dashupdate()
{
  Serial2.write("stat.txt=");
  Serial2.write(0x22);
  if (settings.ESSmode == 1)
  {
    switch (bmsstatus)
    {
      case (Boot):
        Serial2.print("Active");
        break;
      case (Error):
        Serial2.print("Error");
        break;
    }
  }
  else
  {
    switch (bmsstatus)
    {
      case (Boot):
        Serial2.print("Boot");
        break;

      case (Ready):
        Serial2.print("Ready");
        break;

      case (Precharge):
        Serial2.print("Precharge");
        break;

      case (Drive):
        Serial2.print("Drive");
        break;

      case (Charge):
        Serial2.print("Charge");
        break;

      case (Error):
        Serial2.print("Error");
        break;
    }
  }
  Serial2.println();
  Serial2.print("soc.val=");
/*  if (SOCoverride != -1) {
    Serial2.print(SOCoverride);
  } else {
    Serial2.print(SOC);
  }*/
  Serial2.println();
  Serial2.print("soc1.val=");
  Serial2.print(SOC);
  Serial2.println();
  Serial2.print("current.val=");
  Serial2.print(CANmilliamps);
  Serial2.println();
  Serial2.print("temp.val=");
  Serial2.print(bms.getAvgTemperature(), 0);
  Serial2.println();
  Serial2.print("templow.val=");
  Serial2.print(bms.getLowTemperature(), 0);
  Serial2.println();
  Serial2.print("temphigh.val=");
  Serial2.print(bms.getHighTemperature(), 0);
  Serial2.println();
  Serial2.print("volt.val=");
  Serial2.print(bms.getPackVoltage() * 10, 0);
  Serial2.println();
  Serial2.print("lowcell.val=");
  Serial2.print(bms.getLowCellVolt() * 1000, 0);
  Serial2.println();
  Serial2.print("highcell.val=");
  Serial2.print(bms.getHighCellVolt() * 1000, 0);
  Serial2.println();
  Serial2.print("firm.val=");
  Serial2.print(firmver);
  Serial2.println();
  Serial2.print("ac.val=");
  Serial2.print(chargeEnabled());
  Serial2.println();
  Serial2.print("celldelta.val=");
  Serial2.print((bms.getHighCellVolt() - bms.getLowCellVolt()) * 1000, 0);
  Serial2.println();
  Serial2.print("chargevsetpoint.val=");
  Serial2.print(settings.ChargeVsetpoint * 1000);
  Serial2.println();
  Serial2.print("chargecurrentmax.val=");
  Serial2.print(settings.chargecurrentmax / 10);
  Serial2.println();
  Serial2.print("inverterstatus.val=");
  Serial2.print(inverterStatus);
  Serial2.println();
 /* Serial2.print("socOverride.val=");
  Serial2.print(SOCoverride);*/
  Serial2.println();

  if (settings.curcan == IsaScale) {
    Serial2.print("kilowatts.val=");
    Serial2.print(kilowatts);
    Serial2.println();

    Serial2.print("kilowatthours.val=");
    Serial2.print(kilowatthours);
    Serial2.println();
  
    Serial2.print("amphours.val=");
    Serial2.print(amphours);
    Serial2.println();

  }
  if(bmsstatus == Charge) {
    Serial2.print("requestedchargecurrent.val=");
    Serial2.print(chargecurrent);
    Serial2.println();

    if(settings.chargertype == Outlander) {
      Serial2.print("chargercurrent.val=");
      Serial2.print(outlander_charger_reported_current);
      Serial2.println();


      Serial2.print("chargervolts.val=");
      Serial2.print(outlander_charger_reported_voltage);
      Serial2.println();


      Serial2.print("chargertemp.val=");
      Serial2.print(outlander_charger_reported_temp2);
      Serial2.println();

      Serial2.print("evse_duty.val=");
      Serial2.print(evse_duty);
      Serial2.println();

      Serial2.print("balancing.val=");
      Serial2.print(balancecells);
      Serial2.println();

      Serial2.print("capacity.val=");
      Serial2.print(settings.CAP);
      Serial2.println();

      Serial2.print("chargerstatus.val=");
      if (outlander_charger_reported_status == 0) {
          Serial2.print("Not Charging");
      } else if (outlander_charger_reported_status == 0x04) {
          Serial2.print("Wait for Mains");
      } else if (outlander_charger_reported_status == 0x08) {
          Serial2.print("Ready/Charging");
      }
      Serial2.println();
 
    }
  }

  
  /*
    Serial2.print("cellbal.val=");
    Serial2.print(bms.getBalancing());
    Serial2.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
    Serial2.write(0xff);
    Serial2.write(0xff);
  */
  if (Serial2.available() > 0) {
    //needs improvement but works for now
    char inByte = Serial2.read();
    if (inByte == 'a') {
      chargeOverride = 1;
    } else if (inByte == 'o') {
      chargeOverride = 0;
    } else if (inByte == 'v') {
      if (Serial2.available() > 0)
        {
          settings.ChargeVsetpoint = Serial2.parseInt();
          settings.ChargeVsetpoint = settings.ChargeVsetpoint / 1000;
        }
    } else if (inByte == 'c') {
      if (Serial2.available() > 0)
        {
          settings.chargecurrentmax = Serial2.parseInt();
          settings.chargecurrentmax = settings.chargecurrentmax * 10;
        }
    } else if (inByte == 's') {
      EEPROM.put(0, settings); //save all change to eeprom
    } else if (inByte == 'q') {
       // SOCoverride = Serial2.parseInt();
    } else if (inByte == 'r') {//reset ISA shunt
      resetISACounters();
    } else if (inByte == 'b') {
      if (Serial2.available() > 0)
        {
          settings.CAP= Serial2.parseInt();
        }
    }
    
  }

}

void chargercomms()
{
  if (settings.chargertype == Elcon)
  {
    msg.id  =  0x1806E5F4; //broadcast to all Elteks
    msg.len = 8;
    msg.flags.extended = 1;
    msg.buf[0] = highByte(uint16_t(settings.ChargeVsetpoint * settings.Scells * 10));
    msg.buf[1] = lowByte(uint16_t(settings.ChargeVsetpoint * settings.Scells * 10));
    msg.buf[2] = highByte(chargecurrent / ncharger);
    msg.buf[3] = lowByte(chargecurrent / ncharger);
    msg.buf[4] = 0x00;
    msg.buf[5] = 0x00;
    msg.buf[6] = 0x00;
    msg.buf[7] = 0x00;

    bmscan.write(msg, settings.chargerCanIndex);
    msg.flags.extended = 0;
  }

  if (settings.chargertype == Eltek)
  {
    msg.id  = 0x2FF; //broadcast to all Elteks
    msg.len = 7;
    msg.buf[0] = 0x01;
    msg.buf[1] = lowByte(1000);
    msg.buf[2] = highByte(1000);
    msg.buf[3] = lowByte(uint16_t(settings.ChargeVsetpoint * settings.Scells * 10));
    msg.buf[4] = highByte(uint16_t(settings.ChargeVsetpoint * settings.Scells * 10));
    msg.buf[5] = lowByte(chargecurrent / ncharger);
    msg.buf[6] = highByte(chargecurrent / ncharger);

    bmscan.write(msg, settings.chargerCanIndex);
  }
  if (settings.chargertype == BrusaNLG5)
  {
    msg.id  = chargerid1;
    msg.len = 7;
    msg.buf[0] = 0x80;
    /*
      if (chargertoggle == 0)
      {
      msg.buf[0] = 0x80;
      chargertoggle++;
      }
      else
      {
      msg.buf[0] = 0xC0;
      chargertoggle = 0;
      }
    */
    if (digitalRead(IN2) == LOW)//Gen OFF
    {
      msg.buf[1] = highByte(maxac1 * 10);
      msg.buf[2] = lowByte(maxac1 * 10);
    }
    else
    {
      msg.buf[1] = highByte(maxac2 * 10);
      msg.buf[2] = lowByte(maxac2 * 10);
    }
    msg.buf[5] = highByte(chargecurrent / ncharger);
    msg.buf[6] = lowByte(chargecurrent / ncharger);
    msg.buf[3] = highByte(uint16_t(((settings.ChargeVsetpoint * settings.Scells ) - chargerendbulk) * 10));
    msg.buf[4] = lowByte(uint16_t(((settings.ChargeVsetpoint * settings.Scells ) - chargerendbulk)  * 10));
    bmscan.write(msg, settings.chargerCanIndex);

    delay(2);

    msg.id  = chargerid2;
    msg.len = 7;
    msg.buf[0] = 0x80;
    if (digitalRead(IN2) == LOW)//Gen OFF
    {
      msg.buf[1] = highByte(maxac1 * 10);
      msg.buf[2] = lowByte(maxac1 * 10);
    }
    else
    {
      msg.buf[1] = highByte(maxac2 * 10);
      msg.buf[2] = lowByte(maxac2 * 10);
    }
    msg.buf[3] = highByte(uint16_t(((settings.ChargeVsetpoint * settings.Scells ) - chargerend) * 10));
    msg.buf[4] = lowByte(uint16_t(((settings.ChargeVsetpoint * settings.Scells ) - chargerend) * 10));
    msg.buf[5] = highByte(chargecurrent / ncharger);
    msg.buf[6] = lowByte(chargecurrent / ncharger);
    bmscan.write(msg, settings.chargerCanIndex);

  }
  if (settings.chargertype == ChevyVolt)
  {
    msg.id  = 0x30E;
    msg.len = 1;
    msg.buf[0] = 0x02; //only HV charging , 0x03 hv and 12V charging
    bmscan.write(msg, settings.chargerCanIndex);


    msg.id  = 0x304;
    msg.len = 4;
    msg.buf[0] = 0x40; //fixed
    if ((chargecurrent * 2) > 255)
    {
      msg.buf[1] = 255;
    }
    else
    {
      msg.buf[1] = (chargecurrent * 2);
    }
    if ((settings.ChargeVsetpoint * settings.Scells ) > 200)
    {
      msg.buf[2] = highByte(uint16_t((settings.ChargeVsetpoint * settings.Scells ) * 2));
      msg.buf[3] = lowByte(uint16_t((settings.ChargeVsetpoint * settings.Scells ) * 2));
    }
    else
    {
      msg.buf[2] = highByte( 400);
      msg.buf[3] = lowByte( 400);
    }
    bmscan.write(msg, settings.chargerCanIndex);

  }

  if (settings.chargertype == Coda)
  {
    msg.id  = 0x050;
    msg.len = 8;
    msg.buf[0] = 0x00;
    msg.buf[1] = 0xDC;
    if ((settings.ChargeVsetpoint * settings.Scells ) > 200)
    {
      msg.buf[2] = highByte(uint16_t((settings.ChargeVsetpoint * settings.Scells ) * 10));
      msg.buf[3] = lowByte(uint16_t((settings.ChargeVsetpoint * settings.Scells ) * 10));
    }
    else
    {
      msg.buf[2] = highByte( 400);
      msg.buf[3] = lowByte( 400);
    }
    msg.buf[4] = 0x00;
    if ((settings.ChargeVsetpoint * settings.Scells)*chargecurrent < 3300)
    {
      msg.buf[5] = highByte(uint16_t(((settings.ChargeVsetpoint * settings.Scells) * chargecurrent) / 240));
      msg.buf[6] = highByte(uint16_t(((settings.ChargeVsetpoint * settings.Scells) * chargecurrent) / 240));
    }
    else //15 A AC limit
    {
      msg.buf[5] = 0x00;
      msg.buf[6] = 0x96;
    }
    msg.buf[7] = 0x01; //HV charging
    bmscan.write(msg, settings.chargerCanIndex);

  }

  if (settings.chargertype == Outlander)
  {

    msg.id = 0x285;
    msg.len = 8;
    msg.buf[0] = 0x0;
    msg.buf[1] = 0x0;
    msg.buf[2] = 0xb6;
    msg.buf[3] = 0x0;
    msg.buf[4] = 0x0;
    msg.buf[5] = 0x0;
    msg.buf[6] = 0x0;
    bmscan.write(msg, settings.chargerCanIndex);

    
    msg.id  = 0x286;
    msg.len = 8;
    msg.buf[0] = highByte(uint16_t(settings.ChargeVsetpoint * settings.Scells * 10));//volage
    msg.buf[1] = lowByte(uint16_t(settings.ChargeVsetpoint * settings.Scells * 10));
    msg.buf[2] = lowByte(chargecurrent / ncharger);
    msg.buf[3] = 0x0;
    msg.buf[4] = 0x0;
    msg.buf[5] = 0x0;
    msg.buf[6] = 0x0;
    bmscan.write(msg, settings.chargerCanIndex);
  }
}

int pgnFromCANId(int canId)
{
  if ((canId & 0x10000000) == 0x10000000)
  {
    return (canId & 0x03FFFF00) >> 8;
  }
  else
  {
    return canId; // not sure if this is really right?
  }
}

////////END///////////
