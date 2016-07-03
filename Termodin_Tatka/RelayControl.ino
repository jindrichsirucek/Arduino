//RelayControl.ino



#define LED_PIN 2
#define TXD_PIN 1
#define RXD_PIN 3

// GPIO 14, 16 are working properly!! others are not: 13, 1, 3, 15 
//GPIO 0 and 2,can be used only in special cases viz: http://www.instructables.com/id/ESP8266-Using-GPIO0-GPIO2-as-inputs/

//!!GPIO 16 is left next to ch_pd pin, uncorectly signed as GPIO15 on universal white board!!!
#define BOILER_HEATING_RELAY_PIN 14  //LOW turn on ProgramaticHeating, HIGH turn off ProgramaticHeating
#define BOILER_ELECTRICITY_RELAY_PIN 16     //Most of the time it will be HIGH(When LOW it  unplugs manual layer of control a boiler - used only when you want shut down heating over internet)

void relayBoard_setup()
{
  if(MAIN_DEBUG) Serial.println("F:relayBoard_setup()");
  pinMode(BOILER_ELECTRICITY_RELAY_PIN, OUTPUT);
  pinMode(BOILER_HEATING_RELAY_PIN, OUTPUT);

  //!!!! LOW - turn on a relay, HIGH it realeses!! !!!
  digitalWrite(BOILER_ELECTRICITY_RELAY_PIN, HIGH);//When LOW unplugs manual layer of control a boiler - used only when you want shut down heating over internet
  digitalWrite(BOILER_HEATING_RELAY_PIN, HIGH);// No heating at setup phase
}


void turnOnBoilerHeating()
{
  if(MAIN_DEBUG) Serial.println("F:turnOnBoilerHeating()");
  if(RELAY_DEBUG) Serial.println("RELAY_DEBUG: heatingBoilerRelayState_global: " + (String)((heatingBoilerRelayState_global == ON_RELAY_STATE) ? "ON_RELAY_STATE" : "OFF_RELAY_STATE"));
  if(digitalRead(BOILER_HEATING_RELAY_PIN) != OFF_RELAY_STATE)
  {
    if(RELAY_DEBUG) Serial.println("---->RELAY_DEBUG: turnOnBoilerHeating at temp: " + (String)lastTemp_global);
    digitalWrite(BOILER_HEATING_RELAY_PIN, LOW);
    heatingBoilerRelayState_global = (digitalRead(BOILER_HEATING_RELAY_PIN)) ? OFF_RELAY_STATE : ON_RELAY_STATE;
    sendNodeState("Heating_ON");
  }
  
}

void turnOffBoilerHeating()
{
  if(MAIN_DEBUG) Serial.println("F:turnOffBoilerHeating()");
  if(RELAY_DEBUG) Serial.println("RELAY_DEBUG: heatingBoilerRelayState_global: " + (String)((heatingBoilerRelayState_global == OFF_RELAY_STATE) ? "OFF_RELAY_STATE" : "ON_RELAY_STATE"));

  if(digitalRead(BOILER_HEATING_RELAY_PIN) != ON_RELAY_STATE)
  {
    if(RELAY_DEBUG) Serial.println("---->RELAY_DEBUG: turnOffBoilerHeating at temp: " + (String)lastTemp_global);
    digitalWrite(BOILER_HEATING_RELAY_PIN, HIGH);
    heatingBoilerRelayState_global = (digitalRead(BOILER_HEATING_RELAY_PIN)) ? OFF_RELAY_STATE : ON_RELAY_STATE;
    sendNodeState("Heating_OFF");
  }
  
}

void cutOffBoilerElectricity()
{
  if(MAIN_DEBUG) Serial.println("F:cutOffBoilerElectricity()");

  if(digitalRead(BOILER_ELECTRICITY_RELAY_PIN) != OFF_RELAY_STATE)
  {
    if(RELAY_DEBUG) Serial.println("---->RELAY_DEBUG:: cutOffBoilerElectricity");
    //digitalWrite(BOILER_HEATING_RELAY_PIN, HIGH);//turns off normal relay, boiler termostat can still turn on heating - clasical driving
    //heatingBoilerRelayState_global = OFF_RELAY_STATE;

    digitalWrite(BOILER_ELECTRICITY_RELAY_PIN, LOW);//cuts off electricity from boiler completely
    cutOffBoilerElectricityRelayStateFlag_global = (digitalRead(BOILER_ELECTRICITY_RELAY_PIN)) ? OFF_RELAY_STATE : ON_RELAY_STATE;
    sendNodeState("BoilerElectricity_CUT_OFF");
  }
}


void turnOnBoilerElectricity()
{
  if(MAIN_DEBUG) Serial.println("F:turnOnBoilerElectricity()");

  if(digitalRead(BOILER_ELECTRICITY_RELAY_PIN) != ON_RELAY_STATE)
  {
    if(RELAY_DEBUG) Serial.println("---->RELAY_DEBUG:: turnOnBoilerElectricity");
    //digitalWrite(BOILER_HEATING_RELAY_PIN, HIGH);//turns off normal relay, boiler termostat can still turn on heating - clasical driving
    //heatingBoilerRelayState_global = ON_RELAY_STATE;

    digitalWrite(BOILER_ELECTRICITY_RELAY_PIN, HIGH);//cuts off electricity from boiler completely
    cutOffBoilerElectricityRelayStateFlag_global = (digitalRead(BOILER_ELECTRICITY_RELAY_PIN)) ? OFF_RELAY_STATE : ON_RELAY_STATE;
    sendNodeState("BoilerElectricity_TURN_ON");
  }
}