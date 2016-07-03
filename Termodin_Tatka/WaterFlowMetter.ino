



#define WATER_FLOW_SENSOR_PIN 12


//#define Serial remoteConsole
#define WATER_FLOW_DEBUG false

void waterFlowSensor_setup()
{
  if(MAIN_DEBUG) Serial.println("F:waterFlowSensor_setup()");

  pinMode(WATER_FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(WATER_FLOW_SENSOR_PIN, ISR_flowCount, RISING); // Setup Interrupt  // see http://arduino.cc/en/Reference/attachInterrupt
  sei(); // Enable interrupts
}

float readFlowInLitres()
{
  if(MAIN_DEBUG) Serial.println("F:readFlowInLitres()");

  if(lastwaterFlowSensorCount_global < waterFlowSensorCount_global)
  {
    lastwaterFlowSensorCount_global = waterFlowSensorCount_global;
    float spotreba = convertWaterFlowSensorImpulsesToLitres(lastwaterFlowSensorCount_global);

    if(WATER_FLOW_DEBUG) Serial.println("Otacky: " + (String)waterFlowSensorCount_global);
    if(WATER_FLOW_DEBUG) Serial.print("Spotreba: ");
    if(WATER_FLOW_DEBUG) Serial.print(spotreba, 4);
    if(WATER_FLOW_DEBUG) Serial.println(" L");

    return spotreba;
  }
  return 0;
}
void ISR_flowCount()                  // Interrupt function
{
  waterFlowSensorCount_global++;
}

void resetflowCount()
{
  waterFlowDisplay_global += lastwaterFlowSensorCount_global;
  waterFlowSensorCount_global = 0;
  lastwaterFlowSensorCount_global = 0;
}

float convertWaterFlowSensorImpulsesToLitres(float count)
{
  float floatFlow = count;
  floatFlow /= 450;

  return floatFlow;
}
