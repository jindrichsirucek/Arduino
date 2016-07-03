#include <OneWire.h>
#include <DallasTemperature.h>
//#define Serial remoteConsole //forwards all Serial outputs to remote console object



OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature DS18B20(&oneWire);

DeviceAddress dallas1, dallas2;


//Tenký drátek: 2 suky = Vcc, 1suk = signal, zadnySuk = Gnd
//Setings definitions


#define TEMPERATURE_SAMPLES_COUNT 6 // number of samples to count average temperature
#define MAX_ACCEPTED_TEMP_SAMPLES_DIFFERENCE 1 //Maximal accepted temp difference in one measurment, in degrees of celsius


float readTemperature(int sensorIndex)
{
  if(MAIN_DEBUG) Serial.println("F:readTemperature(int sensorIndex): " + (String)sensorIndex);
  
  float tempAverage = 0;
  int measurmentCount = 0;
  float tempSamples[TEMPERATURE_SAMPLES_COUNT];

  int cnt = TEMPERATURE_SAMPLES_COUNT;
  while(cnt--)
  {
    DS18B20.requestTemperatures();
    tempSamples[cnt] = DS18B20.getTempCByIndex(sensorIndex);

    if(TEMPERATURE_DEBUG) Serial.print(TEMPERATURE_SAMPLES_COUNT - cnt);
    if(TEMPERATURE_DEBUG) Serial.print(". temp Sample: ");
    if(TEMPERATURE_DEBUG) Serial.println(tempSamples[cnt]);
  }

  float tempDifferenceSample = 0;
  cnt = TEMPERATURE_SAMPLES_COUNT;
  while(--cnt > 0) //od 5ti do 1
  {
    if(abs(tempSamples[cnt] - tempSamples[cnt - 1]) > MAX_ACCEPTED_TEMP_SAMPLES_DIFFERENCE)
    {
      if(TEMPERATURE_DEBUG) Serial.printf("Bad TEMP sample: (%d) (%d)\n", (int)(tempSamples[cnt] * 1000) / 1, (int)(tempSamples[cnt - 1] * 1000) / 1);
      if(TEMPERATURE_DEBUG) Serial.print("Temp difference: ");
      if(TEMPERATURE_DEBUG) Serial.println(abs(tempSamples[cnt] - tempSamples[cnt - 1]));
      return -127;
    }
    tempAverage += tempSamples[cnt];
  }
  tempAverage += tempSamples[0];

  if(TEMPERATURE_DEBUG) Serial.print("Temp measurment succesfull..");

  tempAverage /= TEMPERATURE_SAMPLES_COUNT;
  if(TEMPERATURE_DEBUG) Serial.print("Average temperature: ");
  if(TEMPERATURE_DEBUG) Serial.println(tempAverage);

  return tempAverage;
}
