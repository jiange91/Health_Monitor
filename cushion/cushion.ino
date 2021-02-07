/*
 * Soft Home Monitoring Prototype Cycle 2
 * Functions:
 *  1. Count the current and total sitting time
 *  2. Detect sitting pressure concentration
 *  3. Detect sitting pressure asymmetry
 *  4. Detet slow rising up
 *  5. Monitor postural shifts on seat
 *  6. Feedback to serial monitor
 *  7. Wireless feedback to base station w/ nrf24l01
 * Version: 4.0
 * Author: Changyuan XIE
 * Data: 2020.07.30
 */

/*Import spi and rf24 libraries*/
#include <SPI.h>
#include "RF24.h"

/*SPI bus define, nrf addresses define*/
RF24 radio(10, 9);
const uint64_t rAddress[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL};

/* Pin define */
const int sensor[8] = {A0, A1, A2, A3, A4, A5, A6, A7};

/* Global variables define */
//Sampling rate controls
unsigned long time_now = 0;
const unsigned int cycle = 500;
//Sensors readings buffers
int sensorValues[8][10];
int sensorValuesCurrent[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int sensorValuesSum[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//Boolean indicators
boolean sedentarySitting = 0, pressureConcentrated = 0, pressureAsymmetry = 0, sitShift = 0, moveRate;
//Thresholds
#define sitPressure_t 120
#define sedentary_t 120
#define sit_comp_time 9 //means (9+1)/2=5 seconds period before check for values diffs in determine whether sits completed
#define concentrated_upT 6
#define concentrated_downT 5
#define asymmetry_t 5
#define noshift_count_t 60
#define shift_threshold 5
#define sumRecordSum_upT 3500
#define sumRecordSum_downT 600
//Integer counters
unsigned long totalSittingTime = 0, sittingTime = 0;
int noShiftCounter = noshift_count_t;
int riseDetectCounter = 12;
int healthyScore;
unsigned int indicatorTime[5] = {0, 0, 0, 0, 0};

void setup()
{
  Serial.begin(115200);
  //Initialize elements of the buffer array
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      sensorValues[i][j] = 0;
    }
  }

  //Initialize nrf sending
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(rAddress[5]);

  Serial.println(F("Initiated"));
}

void loop()
{

  //Sync time_now per cycle
  time_now = millis();

  fifo();
  getSensorValues();
  //sensorValuesCalibration();
  sensorValuesRemap();
  assignSumAndCurrent();

  analyse();
  getHealthScore();

  printOut();
  //Serial.println("looped");

  //Sending results to base station
  nrfEmitting(healthyScore);

  //delay(500);
  while (millis() < time_now + cycle)
  { /*wait [cycle] ms*/
  }
}

void nrfEmitting(int x)
{
  byte package[7];
  package[0] = (byte *)x;
  package[1] = getSitted();
  package[2] = sedentarySitting;
  package[3] = pressureConcentrated;
  package[4] = pressureAsymmetry;
  package[5] = sitShift;
  package[6] = moveRate;
  Serial.println(F("Now sending: "));
  radio.write(&package, sizeof(byte[7]));
  Serial.print(package[0]);
  Serial.print(",");
  Serial.print(package[1]);
  Serial.print(",");
  Serial.print(package[2]);
  Serial.print(",");
  Serial.print(package[3]);
  Serial.print(",");
  Serial.print(package[4]);
  Serial.print(",");
  Serial.print(package[5]);
  Serial.print(",");
  Serial.print(package[6]);
  Serial.println("-----");
}

void getHealthScore()
{
  /*
   * totalSittingTime
   * sittingTime
   * 
   * sedentarySitting
   * pressureConcentrated
   * pressureAsymmetry
   * sitShift
   * moveRate
   */
  boolean indicator[5] = {sedentarySitting, pressureConcentrated, pressureAsymmetry, sitShift, moveRate};
  healthyScore = 100;
  for (int i = 0; i < 5; i++)
  {
    if (indicator[i])
    {
      indicatorTime[i] += 1;
    }
    else
    {
      indicatorTime[i] = 0;
    }
    healthyScore -= logHealthScore(indicatorTime[i]);
    healthyScore = (int *)healthyScore;
    //Debug
    //Serial.print(indicatorTime[i]);
    //Serial.print(",");
    //Serial.println(healthyScore);
  }
  if (healthyScore < 0)
  {
    healthyScore = 0;
  }
  //Debug
  //Serial.println("");
}

float logHealthScore(float a)
{
  float minus = ((-exp(-a / 50)) + 1) * 25;
  //Debug
  //Serial.print(minus);
  //Serial.print(",");
  return minus;
}

void fifo()
{
  for (int i = 0; i < 8; ++i)
  {
    for (int j = 9; j > 0; --j)
    {
      sensorValues[i][j] = sensorValues[i][j - 1];
    }
  }
  for (int k = 9; k > 0; --k)
  {
    sensorValuesSum[k] = sensorValuesSum[k - 1];
  }
}

void getSensorValues()
{
  sensorValues[0][0] = analogRead(sensor[3]);
  sensorValues[1][0] = analogRead(sensor[5]);
  sensorValues[2][0] = analogRead(sensor[2]);
  sensorValues[3][0] = analogRead(sensor[4]);
  sensorValues[4][0] = analogRead(sensor[7]);
  sensorValues[5][0] = analogRead(sensor[1]);
  sensorValues[6][0] = analogRead(sensor[6]);
  sensorValues[7][0] = analogRead(sensor[0]);
}

void sensorValuesRemap()
{
  for (int i = 0; i < 8; i++)
  {
    sensorValues[i][0] = constrain(sensorValues[i][0], 0, 400);
    sensorValues[i][0] = map(sensorValues[i][0], 0, 400, 99, -5);
    sensorValues[i][0] = constrain(sensorValues[i][0], 0, 99);
  }
}

void assignSumAndCurrent()
{
  sensorValuesSum[0] = 0;
  for (int i = 0; i < 8; i++)
  {
    sensorValuesCurrent[i] = sensorValues[i][0];
    sensorValuesSum[0] += sensorValuesCurrent[i];
  }
}

void analyse()
{
  indicatorInit();

  if (getSitted())
  {
    sittingTimeCount();
    sedentarySitting = getSedentary();
    if (sittedComp)
    {
      //Get whether the pressure is concentrated, 1=unhealthy
      pressureConcentrated = getPressureConcentration();

      //Get whether the pressure is unbalanced(asymmetry)
      pressureAsymmetry = getPressureBalance();

      //Get whether lack of postural shift
      sitShift = getShift();

      //Initialize moveRate
      moveRate = 0;
    }
  }
  else
  {
    //Clear the counter of current sitting time if not sitted
    sittingTime = 0;

    //Get whether the rising rate is too low
    if (riseDetectCounter == 12)
    {
      moveRate = getRiseMoveRate();
    }
    else
    {
      riseDetectCounter += 1;
    }
  }
}

void indicatorInit()
{
  sedentarySitting = 0;
  pressureConcentrated = 0;
  pressureAsymmetry = 0;
  sitShift = 0;
}

boolean getRiseMoveRate()
{
  int previous = 0;
  for (int i = 0; i < 10; ++i)
  {
    previous += sensorValuesSum[i];
  }
  //Debug
  Serial.print("Rise move rate parameter: ");
  Serial.println(previous);
  Serial.println("");

  if (previous < sumRecordSum_upT && previous > sumRecordSum_downT)
  {
    riseDetectCounter = 0;
    return 1;
  }
  else if (previous > sumRecordSum_upT)
  {
    riseDetectCounter = 0;
    return 0;
  }
  else
  {
    return 0;
  }
}

boolean getShift()
{
  float std;
  std = getStdDev(sensorValuesSum, 10);
  //Debug
  Serial.print("Shift std: ");
  Serial.println(std);

  if (std > shift_threshold)
  {
    noShiftCounter = noshift_count_t;
  }
  else
  {
    noShiftCounter -= 1;
  }

  //Debug
  Serial.print("noShiftCounter: ");
  Serial.println(noShiftCounter);

  if (noShiftCounter < 0)
  {
    noShiftCounter = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

boolean getPressureBalance()
{
  int sides[2];
  sides[0] = sensorValuesCurrent[0] + sensorValuesCurrent[1] + sensorValuesCurrent[4] + sensorValuesCurrent[5];
  sides[1] = sensorValuesCurrent[2] + sensorValuesCurrent[3] + sensorValuesCurrent[6] + sensorValuesCurrent[7];
  int *arr = sides;
  float stdSides = getStdDev(arr, 2);

  //Debug
  Serial.print("pressure balane stdDev: ");
  Serial.println(stdSides);

  if (stdSides >= asymmetry_t)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

boolean getPressureConcentration()
{
  int *arr = sensorValuesCurrent;
  float std = getStdDev(arr, 8);
  //Debug
  Serial.print("Concentration std: ");
  Serial.println(std);
  if (std > concentrated_upT || std < concentrated_downT)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void sittingTimeCount()
{
  totalSittingTime++;
  sittingTime++;
}

boolean getSedentary()
{
  if (sittingTime >= sedentary_t)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void printOut()
{
  Serial.println("--Serial print info-----------");
  Serial.println("Sensors readings: ");
  for (int a = 0; a < 8; a++)
  {
    for (int b = 0; b < 10; b++)
    {
      Serial.print(sensorValues[a][b]);
      Serial.print(",");
    }
    Serial.println("");
  }

  Serial.println("sensorValuesCurrent: ");
  for (int c = 0; c < 8; c++)
  {
    Serial.print(sensorValuesCurrent[c]);
    Serial.print(",");
  }
  Serial.println("");

  Serial.println("sensorValuesSum: ");
  for (int d = 0; d < 10; d++)
  {
    Serial.print(sensorValuesSum[d]);
    Serial.print(",");
  }
  Serial.println("");

  Serial.print("Total sitting time=");
  Serial.println(totalSittingTime);
  Serial.print("Current sitting time=");
  Serial.println(sittingTime);

  Serial.print("Sedentary: ");
  Serial.println(sedentarySitting);

  Serial.print("Pressure concentrated: ");
  Serial.println(pressureConcentrated);

  Serial.print("Pressure asymmetried: ");
  Serial.println(pressureAsymmetry);

  Serial.print("Lack of postural shifts: ");
  Serial.println(sitShift);

  Serial.print("Rising too slow: ");
  Serial.println(moveRate);

  Serial.print("Health Score: ");
  if (getSitted() || riseDetectCounter != 12)
  {
    Serial.println(healthyScore);
  }
  else
  {
    Serial.println("not sitted");
  }

  Serial.println("----------------------");
}

boolean getSitted()
{
  if (sensorValuesSum[0] >= sitPressure_t)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

boolean sittedComp()
{
  boolean s[8];
  for (int i = 0; i < 8; i++)
  {
    int diffs = sensorValues[i][0] - sensorValues[i][sit_comp_time];
    if (abs(diffs) <= 60)
    {
      s[i] = 1;
    }
    else
    {
      s[i] = 0;
    }
  }
  if (s[0] || s[1] || s[2] || s[3] || s[4] || s[5] || s[6] || s[7])
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

float getMean(int *val, int arrayCount)
{
  long total = 0;
  for (int i = 0; i < arrayCount; i++)
  {
    total = total + val[i];
  }
  float avg = total / (float)arrayCount;
  return avg;
}

float getStdDev(int *val, int arrayCount)
{
  float avg = getMean(val, arrayCount);
  long total = 0;
  for (int i = 0; i < arrayCount; i++)
  {
    total = total + (val[i] - avg) * (val[i] - avg);
  }

  float variance = total / (float)arrayCount;
  float stdDev = sqrt(variance);
  return stdDev;
}
