



#define CURRENT_SENSOR_PIN A0



float readCurrent()
{
  int sensor_max;
  sensor_max = getMaxCurrentValue();
  Serial.print("sensor_max = ");
  Serial.println(sensor_max);
  //the VCC on the Grove interface of the sensor is 5v
  float amplitude_current = (float)sensor_max / 124 * 5 / 800 * 2000000;
  float effective_value = amplitude_current / 1.414; //minimum_current=1/124*5/800*2000000/1.414=8.6(mA)
  //Only for sinusoidal alternating current
  Serial.println("The amplitude of the current is(in mA)");
  Serial.println(amplitude_current, 1); //Only one number after the decimal point
  Serial.println("The effective value of the current is(in mA)");
  Serial.println(effective_value, 1);
  return effective_value;
}


/*Function: Sample for 1000ms and get the maximum value from the SIG pin*/
int getMaxCurrentValue()
{
  if(MAIN_DEBUG) Serial.println("F:getMaxCurrentValue()");
  int sensorValue;             //value read from the sensor
  int sensorMax = 0;
  int pocetMereni = 0;
  while(pocetMereni++ < 100) //sample for 1000ms
  {
    delay(2);
    sensorValue = analogRead(CURRENT_SENSOR_PIN);
    if(sensorValue > sensorMax)
    {
      /*record the maximum sensor value*/
      sensorMax = sensorValue;
      if(CURRENT_DEBUG) Serial.println("sensorMax value: " + (String)sensorMax);
    }
  }
  return sensorMax;
}





