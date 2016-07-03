//ESP_Current_Measure.ino


void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);
  Serial.begin(115200);
  Serial.println("start");  
}

void loop() {
  

  Serial.println(getMaxValue());
}


/*Function: Sample for 1000ms and get the maximum value from the SIG pin*/
int getMaxValue()
{
  int sensorValue;             //value read from the sensor
  int sensorMax = 0;
  long pocetMereni = 0;
  while (pocetMereni < 1000) //sample for 1000ms
  {
    delay(1);
    sensorValue = analogRead(A0);
    if (sensorValue > sensorMax)
    {
      /*record the maximum sensor value*/
      sensorMax = sensorValue;
      //Serial.println(sensorMax);
    }
    pocetMereni++;
  }
  return sensorMax;
}