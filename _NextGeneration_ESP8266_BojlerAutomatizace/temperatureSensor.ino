#include <OneWire.h>
#include <DallasTemperature.h>
//#define Serial remoteConsole //forwards all Serial outputs to remote console object


#define ONE_WIRE_BUS_PIN 13  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature DS18B20(&oneWire);

DeviceAddress dallas1, dallas2;

//Setings definitions

#define TEMPERATURE_SAMPLES_COUNT 6 // number of samples to count average temperature
#define MAX_ACCEPTED_TEMP_SAMPLES_DIFFERENCE 1 //Maximal accepted temp difference between two measurments, in degrees of celsius // eliminates wrong measurments


float readTemperature(int sensorIndex)
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("F:readTemperature(int sensorIndex): " + (String)sensorIndex);
  DS18B20.requestTemperatures();

  //flowTemp_global = DS18B20.getTempCByIndex(1);

  float pipeTemp = DS18B20.getTempCByIndex(1);
  if(pipeTemp != -127)
    pipeTemp_global = pipeTemp;

  float tempAverage = 0;
  int measurmentCount = 0;
  float tempSamples[TEMPERATURE_SAMPLES_COUNT];

  int cnt = TEMPERATURE_SAMPLES_COUNT;
  while(cnt--)
  {
    DS18B20.requestTemperatures();
    tempSamples[cnt] = DS18B20.getTempCByIndex(0);

    if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.print(TEMPERATURE_SAMPLES_COUNT - cnt);
    if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.print(". temp Sample: ");
    if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.println(tempSamples[cnt]);
  }

  float tempDifferenceSample = 0;
  cnt = TEMPERATURE_SAMPLES_COUNT;
  while(--cnt > 0) //od 5ti do 1000
  {
    if(abs(tempSamples[cnt] - tempSamples[cnt - 1]) > MAX_ACCEPTED_TEMP_SAMPLES_DIFFERENCE)
    {
      if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.printf("Bad TEMP sample: (%d) (%d)\n", (int)(tempSamples[cnt] * 1000) / 1, (int)(tempSamples[cnt - 1] * 1000) / 1);
      if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.print("Temp difference: ");
      if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.println(abs(tempSamples[cnt] - tempSamples[cnt - 1]));
      return -127;
    }
    tempAverage += tempSamples[cnt];
  }
  tempAverage += tempSamples[0];

  if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.print("Temp measurment succesfull..");

  tempAverage /= TEMPERATURE_SAMPLES_COUNT;
  if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.print("Average temperature: ");
  if(TEMPERATURE_DEBUG) DEBUG_OUTPUT.println(tempAverage);

  return tempAverage;
}
