#define BLYNK_TEMPLATE_ID "TMPL3OlHJW69Z"
#define BLYNK_TEMPLATE_NAME "SENSOR"
#define BLYNK_AUTH_TOKEN "DeJpe5aQLJhAkeJlVVDn7mYIVs5buE2Q"
#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(32);
DallasTemperature sensors(&oneWire);
float calibration_value = 21.34;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;

// *

#define VREF 3300  //VREF (mv)
#define ADC_RES 4096 //ADC Resolution

//Single-point calibration Mode=0
//Two-point calibration Mode=1
#define TWO_POINT_CALIBRATION 0

//Single point calibration needs to be filled CAL1_V and CAL1_T
#define CAL1_V (126) //mv
#define CAL1_T (25)   //℃
//Two-point calibration needs to be filled CAL2_V and CAL2_T
//CAL1 High temperature point, CAL2 Low temperature point
//#define CAL2_V (1300) //mv
//#define CAL2_T (15)   //℃

const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};

uint16_t ADC_Raw;
uint16_t ADC_Voltage;
uint16_t DO;

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c)
{
#if TWO_POINT_CALIBRATION == 0
  uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#else
  uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#endif
}
//**

char auth[] = "DeJpe5aQLJhAkeJlVVDn7mYIVs5buE2Q";
char ssid[] = "Galaxy";
char pass[] = "7800708022";


//Control an Motor
    
BLYNK_WRITE(V3)
  {
    int value = param.asInt();
    if(value==1)
    {
      digitalWrite(12,HIGH);
    }
    if(value==0)
    {
      digitalWrite(12,LOW);
    }
  }



void setup() 
  {
    Serial.begin(115200);
    pinMode(12,OUTPUT);
    Blynk.begin(auth, ssid, pass);
    lcd.init();      
    lcd.backlight();
    sensors.begin();
    sensors.setResolution(11);
  
  }
void loop() 
  {
    Blynk.run();  
  

  //pH sensor part
  
    for(int i=0;i<10;i++) 
    { 
    buffer_arr[i]=analogRead(35);
    delay(30);
    }
    for(int i=0;i<9;i++)
    {
    for(int j=i+1;j<10;j++)
    {
    if(buffer_arr[i]>buffer_arr[j])
    {
    temp=buffer_arr[i];
    buffer_arr[i]=buffer_arr[j];
    buffer_arr[j]=temp;
    }
    }
    }
    avgval=0;
    for(int i=2;i<8;i++)
    avgval+=buffer_arr[i];
    float volt=(float)avgval*3.3/4095/6;
    float ph_act = -5.70 * volt + calibration_value;
  
  
  
  
   //Temperature Sensor Part 
   
   sensors.requestTemperatures();
   uint8_t t = sensors.getTempCByIndex(0);
  
  
  
  //Dissolved Oxygen sensor part
  
   ADC_Raw= analogRead(34);
   ADC_Voltage=uint32_t(VREF)*ADC_Raw/ADC_RES;
   float dov = readDO(ADC_Voltage, t)/1000;
   
  
  
  //Serial print on Serial Monitor
  
   Serial.print("pH Val: ");
   Serial.print(ph_act);
   Serial.print(" ");
   Serial.print("Temperature: ");
   Serial.print(t);
   Serial.print(" ");
   Serial.println("DO:\t" + String(readDO(ADC_Voltage, t)) + "\t");

  //Blynk Print part
  
    Blynk.virtualWrite(V0, ph_act);
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, dov);

    lcd.setCursor(3,0);
    lcd.print("Temp = ");
    lcd.setCursor(10,0);
    lcd.print(t);
    lcd.setCursor(0,1);
    lcd.print("pH: ");
    lcd.setCursor(4,1);
    lcd.print(ph_act);
    lcd.setCursor(8,1);
    lcd.print(" Do:");
    lcd.setCursor(12,1);
    lcd.print(dov);
    
    
    
}