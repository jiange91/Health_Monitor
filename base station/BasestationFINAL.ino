/*
 * Soft Home Monitoring Prototype Basestation
 * Code for the Basestation to recieve the data from shoes and cushion
 * Functions:
 *  1. Count the current and total standing time
 *  2. Count the current and total walking time
 *  3. Detect foot pressure asymmetry between two feet
 *  4. Count the total step 
 *  5.detect diabetes and obese
 *  6.detect 4 kinds of falls
 * Date: 2020.07.18
 */

/* Global variable define */
//Receiver program
#include <SPI.h>
#include "RF24.h"
RF24 radio(12, 5);

/* Global variable define */
const uint64_t rAddress[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL};
byte pipeNum = 0;

int sensorValuesR[3][8];     //A 3*8 two dimensional buffer array to store last 4 seconds of readings from the sensors
int sensorValuesL[3][8];     //A 3*8 two dimensional buffer array to store last 4 seconds of readings from the sensors
int sensorValuesCurrent[6];  //An array to store the current sensor readings
int sensorValuesCurrentL[3]; //A buffer array to store readings from the left sensors
int sensorValuesCurrentR[3]; //A buffer array to store readings from the right sensors
int sumSensorValuesL[8];     //An array to store the all the left foot's sensors readings' sum
int sumSensorValuesR[8];     //An array to store the all the Right foot's sensors readings' sum
/*
#define stood_threshold 60 //Pressure greater than this is seen as user have stood
#define asymmetry_threshold 40 //Differences greater than this value between the two sides' sensors are seen as unbalanced standing pressure
#define sedentary_time 30 //The user is sedentary standing if 30 seconds of standing is achieved
#define heelpressure_threshold 30 //The pressure of heel when heel lands on the groud
#define concentrated_Threshold 5 //Greater stdDev value than this is seen as diabetes and obese
#define sumFallSupport_Threshold 280 //Greater sum values of support leg pressure is seen as side fall
#define sumFallLift_Threshold 5 //Smaller sum values are seen as liftted leg in side fall
#define sideFall_Threshold 90 //Greater values are regarded as side fall
#define forwardFallHIGH_Threshold 90 //Greater values are regarded as forward fall
#define forwardFallLOW_Threshold 5 //Smaller values are regarded as forward fall
unsigned int totalStandingTime=0; //Unsigned variable that is used to count the total time of standing
unsigned int standingTime=0; //Unsigned variable that is used to count the time of current standing
unsigned int totalWalkingTime=0; //Unsigned variable that is used to count the total time of walking
unsigned int walkingTime=0; //Unsigned variable that is used to count the time of current walking
unsigned int steps_total; //Unsigned variable to count the total steps
unsigned int stepCounter;//

*/
void setup()
{
  Serial.begin(115200);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);

  radio.openReadingPipe(2, rAddress[2]);
  radio.openReadingPipe(3, rAddress[3]);

  radio.startListening();
  Serial.print("Initiated.");
}

void loop()
{

  //Write the values of sensors into the buffer
  //getSensorValue();
  /*
  boolean sedentaryStanding=0;
  boolean pressureAsymmetry=0;
  boolean pressureConcentratedL=0;
  boolean pressureConcentratedR=0;
  boolean stepsadd = 0;
  boolean fallRight = 0;
  boolean fallLeft = 0;
  boolean fallForward = 0;
  boolean fallBack = 0;
*/

  byte received[3];
  if (radio.available(&pipeNum))
  {
    while (radio.available(&pipeNum))
    {
      radio.read(&received, sizeof(byte[3]));
    }
    if (pipeNum == 2)
    {
      for (int x = 0; x < 3; x++)
      {
        received[x] = received[x] + '0';
        sensorValuesCurrentR[x] = received[x];
        Serial.println("Sensor values from Right shoe:");
        Serial.print(sensorValuesCurrentR[x]);
        Serial.print(",");
      }
      Serial.println("");
    }
    else if (pipeNum == 3)
    {
      for (int x = 0; x < 3; x++)
      {
        //received[x]=received[x]+'0';
        sensorValuesCurrentL[x] = received[x];
        Serial.println("Sensor values from Left shoe:");
        Serial.print(sensorValuesCurrentL[x]);
        Serial.print(",");
      }
      Serial.println("");
    }
  }
  /*
  //Called if user detected
  if(stood()){
      //Get whether the time of current sitting is too long, 1=unhealthy
      sedentaryStanding = getStandingTime();
    if(stoodCompl){
            
      //Get whether the pressure is concentrated, 1=unhealthy
      pressureConcentratedR = getPressureConcentrationR();

      //Get whether the pressure is concentrated, 1=unhealthy
      pressureConcentratedL = getPressureConcentrationL();

      //Get whether the pressure is unbalanced(asymmetry)
      pressureAsymmetry = getPressureBalance();

      //Get whether the elderly is falling towards the right, 1=fall
      fallRight = getFallRight();

      //Get whether the elderly is falling towards the left, 1=fall
      fallLeft = getFallLeft();
    
      //Get whether the elderly is falling towards the forward, 1=fall
      fallForward = getFallForward();
    
      //Get whether the elderly is falling towards the back, 1=fall
      fallBack = getFallBack();  
     
      }

    }else{      
      //Clear the counter of current sitting time if not sitted
      standingTime=0;
      }
  


  if(stepCounter){
      //Get whether steps increase
       if(sensorValuesCurrent[2]<heelpressure_threshold){
       stepCounter=1;
       }               
     }

  //Sensor values output for debug
  Serial.println("--Debug info--");
///*  
  Serial.println("SensorValuesR:");
  for(int b=0;b<3;++b){
    for(int a=0;a<=7;++a){
      Serial.print(sensorValuesR[b][a]);
      Serial.print(",");
    }
    Serial.println("");
    }


  Serial.println("SensorValuesL:");
  for(int b=0;b<3;++b){
    for(int a=0;a<=7;++a){
      Serial.print(sensorValuesL[b][a]);
      Serial.print(",");
    }
    Serial.println("");
    }

  Serial.println("sensorValuesCurrentR: ");
  for(int e=0;e<3;++e){
    Serial.print(sensorValuesCurrentR[e]);
    Serial.print(",");
    }
  Serial.println("");


  Serial.println("sensorValuesCurrentL: ");
  for(int f=0;f<3;++f){
    Serial.print(sensorValuesCurrentL[f]);
    Serial.print(",");
    }
  Serial.println("");

  Serial.println("sumSensorValuesR: ");
  for(int d=0;d<8;++d){
    Serial.print(sumSensorValuesR[d]);
    Serial.print(",");
    }
  Serial.println("");

  Serial.println("sumSensorValuesL: ");
  for(int d=0;d<8;++d){
    Serial.print(sumSensorValuesL[d]);
    Serial.print(",");
    }
  Serial.println("");
  

  Serial.print("Total standing time=");
  Serial.println(totalStandingTime);
  Serial.print("Current standing time=");
  Serial.println(standingTime);

  Serial.print("Whether stands too long: ");
  Serial.println(sedentaryStanding);
  
  Serial.print("Whether asymmetry pressure detected: ");
  Serial.println(pressureAsymmetry);

  Serial.print("Total steps=");
  Serial.println(steps_total);

  Serial.print("Whether the pressure of right foot is concerntrated: ");
  Serial.println(pressureConcentratedR);

  Serial.print("Whether the pressure of left foot is concerntrated: ");
  Serial.println(pressureConcentratedL);

  Serial.print("Whether falls towards the right: ");
  Serial.println(fallRight);
  
  Serial.print("Whether falls towards the left: ");
  Serial.println(fallLeft);
  
  Serial.print("Whether falls towards the forward: ");
  Serial.println(fallForward);

  Serial.print("Whether falls towards the Back: ");
  Serial.println(fallBack);
  
  Serial.println("-------------");
*/
  delay(1000);
}
