
 
//****************** Libraries Needed ***************//
 
 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
 
//************************* User Defined Variables ********************************************************//
 
int R1 = 470;     //Do not Replace R1 with a resistor lower than 300 ohms
int Ra = 25;      //Resistance of powering Pins
int ECPin = A0;
int ECGround = A1;
int ECPower = A4;

//************** Thermistor Setup*************//

// which analog pin to connect
#define THERMISTORPIN A2         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    
 
int samples[NUMSAMPLES];
 
 
//*********** Converting to ppm constant [US = 0.5]*************//

float PPMconversion=0.5;

 
 
//*************Compensating for temperature ************************************//
//The value below will change depending on what chemical solution we are measuring
//[google "Temperature compensation EC" for more info] 2.14 is standard for NaCl
float TemperatureCoef = 2.14;
 
 
 
 
//********************** Cell Constant For EC Measurements *********************//
//Standard value: 2.9
//My calculated value for plug with holes: 1.82
//calibration code value: 2.87

float K = 3.1;
 
 
 
//************ Temp Probe Related *********************************************//
#define ONE_WIRE_BUS 2          // Data wire For Temp Probe is plugged into pin 2 on the Mayfly

 
 
 
 
//***************************** END Of Recomended User Inputs *****************************************************************//
 
 
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
 
LiquidCrystal lcd(19, 18, 9, 8, 7, 6); //initialize library with numbers of pins for LCD screen

 
float Temperature=10;
float EC=0;
float EC25 =0;
float ECnew = 0;
int ppm =0;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;
 
 
 
 
//*********************************Setup - runs Once and sets pins etc ******************************************************//
void setup()
{
  Serial.begin(9600);
  analogReference(EXTERNAL);
  pinMode(ECPin,INPUT);
  pinMode(ECPower,OUTPUT);//Setting pin for sourcing current
  pinMode(ECGround,OUTPUT);//setting pin for sinking current
  digitalWrite(ECGround,LOW);//We can leave the ground connected permanantly
 
  delay(100);// gives sensor time to settle
  sensors.begin();
  delay(100);
  //** Adding Digital Pin Resistance to [25 ohm] to the static Resistor *********//
  R1=(R1+Ra);// Taking into acount Powering Pin Resitance

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("reading...");
  
  Serial.println("Make sure Probe and Temp Sensor are in Solution and solution is well mixed");
  Serial.println("");
  Serial.println("Measurments at 5 Second intervals [Dont read Ec more than once every 5 seconds]:");
 
 
};
//******************************************* End of Setup **********************************************************************//
 
 
 
 
//************************************* Main Loop - Runs Forever ***************************************************************//

void loop()
{
 
 
 
 
GetEC();          // dont call this more that 1/5 hhz [once every five seconds] or you will polarize the water
PrintReadings();  
 
 
delay(5000);
 
 
}
//************************************** End Of Main Loop **********************************************************************//
 
 
 
 
//************ This Loop Is called From Main Loop************************//
void GetEC(){
 
 
//*********Reading Temperature Of Solution *******************//

uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
 // Serial.print("Average analog reading "); 
 // Serial.println(average);
 
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  //Serial.print("Thermistor resistance "); 
  //Serial.println(average);
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 
  Serial.print("Temperature "); 
  Serial.print(steinhart);
  Serial.println(" *C");

  Temperature = steinhart;
  //delay(1000);
 
 
//************Estimates Resistance of Liquid ****************//
digitalWrite(ECPower,HIGH);
raw= analogRead(ECPin);
raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
digitalWrite(ECPower,LOW);
 
 
 
 
//***************** Converts to EC **************************//
Vdrop= (Vin*raw)/1024.0;
Rc=(Vdrop*R1)/(Vin-Vdrop);
Rc=Rc-Ra; //acounting for Digital Pin Resitance
EC = 1000/(Rc*K);
 
 
//*************Compensating For Temperaure********************//
EC25  =  EC/ (1+ TemperatureCoef*(Temperature-25.0));
ppm=(EC25)*(PPMconversion*1000);


 //********************Manual EC Calibration************************//
 ECnew = ((EC25) * (EC25) * (18.344)) + ((8.5029) * (EC25))
 
;}
//************************** End OF EC Function ***************************//
 
 
 
 
//***This Loop Is called From Main Loop- Prints to serial and LCD ***//
void PrintReadings(){
Serial.print("Rc: ");
Serial.print(Rc);
Serial.print(" EC: ");
Serial.print(ECnew);
Serial.print(" mS/cm  ");
Serial.print(ppm);
Serial.print(" ppm  ");
Serial.print(Temperature);
Serial.println(" *C ");

//print to LCD screen
lcd.setCursor (0 ,0);                 //Set cursor to start of first line
lcd.print("                ");        //Blank previous text there
lcd.setCursor (0 ,0);                 //Reset cursor to start of line
lcd.print(ECnew);                     //Print EC
lcd.print(" mS/cm ");

lcd.setCursor (0 ,1);                 //Set cursor to start of second line
lcd.print("                ");        //Blank previous text there
lcd.setCursor (0 ,1);                 //Reset cursor to start of line
lcd.print(Temperature);               //Print temperature
lcd.print(" *C");


};
