



#define WATER_FLOW_SENSOR_PIN 12


//#define Serial remoteConsole
#define WATER_FLOW_DEBUG false

void waterFlowSensor_setup()
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("F:waterFlowSensor_setup()");

  pinMode(WATER_FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(WATER_FLOW_SENSOR_PIN, ISR_flowCount, RISING); // Setup Interrupt  // see http://arduino.cc/en/Reference/attachInterrupt
  sei(); // Enable interrupts
}

float readFlowInLitres()
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("F:readFlowInLitres()");

  if(lastWaterFlowSensorCount_global < waterFlowSensorCount_ISR_global)
  {
    lastWaterFlowSensorCount_global = waterFlowSensorCount_ISR_global;
    float spotreba = convertWaterFlowSensorImpulsesToLitres(lastWaterFlowSensorCount_global);

    if(WATER_FLOW_DEBUG) DEBUG_OUTPUT.println("Otacky: " + (String)waterFlowSensorCount_ISR_global);
    if(WATER_FLOW_DEBUG) DEBUG_OUTPUT.print("Spotreba: ");
    if(WATER_FLOW_DEBUG) DEBUG_OUTPUT.print(spotreba, 4);
    if(WATER_FLOW_DEBUG) DEBUG_OUTPUT.println(" L");
    if(WATER_FLOW_DEBUG) DEBUG_OUTPUT.print("waterFlowDisplay_global/450: ");
    if(WATER_FLOW_DEBUG) DEBUG_OUTPUT.println(waterFlowDisplay_global/450);

    return spotreba;
  }
  return 0;
}
void ISR_flowCount()                  // Interrupt function
{
  waterFlowSensorCount_ISR_global++;
}

void resetflowCount()
{
  waterFlowDisplay_global += waterFlowSensorCount_ISR_global;
  waterFlowSensorCount_ISR_global = 0;
  lastWaterFlowSensorCount_global = 0;
  lastWaterFlowResetTime_global = millis();
}

float convertWaterFlowSensorImpulsesToLitres(float count)
{
  float floatFlow = count;
  floatFlow /= 450;

  return floatFlow;
}

String getWaterFlowLitresInMinutes()
{
  unsigned long measuredImpulsesPerOneSecond = waterFlowSensorCount_ISR_global;
  delay(500);
  measuredImpulsesPerOneSecond = waterFlowSensorCount_ISR_global - measuredImpulsesPerOneSecond;

  float waterFlowInLitresPerMinute = convertWaterFlowSensorImpulsesToLitres(measuredImpulsesPerOneSecond) * 120;
  
  float spareHotWaterInLitres = 100 - convertWaterFlowSensorImpulsesToLitres(waterFlowDisplay_global);
  
  float spareTimeOfShoweringImMinutes = spareHotWaterInLitres / waterFlowInLitresPerMinute;

  int minutesToColdWater = trunc(spareTimeOfShoweringImMinutes);
  //int secondsToColdWater = (spareTimeOfShoweringImMinutes - minutesToColdWater ) * 60;

  if((millis() / 1000) % 3 == 0)
    return (String)waterFlowInLitresPerMinute + "l/m ";
  if((millis() / 1000) % 3 == 1)
    return (String)minutesToColdWater + "minut";
  if((millis() / 1000) % 3 == 2)  
    return nowTime_global;
  else
    return "";
}
