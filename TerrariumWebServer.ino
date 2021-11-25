// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: A0

#define DHT11_PIN 14
#define pinRelay4 26
#define pinRelay3 25
#define pinRelay2 33
#define pinRelay1 32
#define tempVOut 4
#define tempData 2

#define stepperA 36
#define stepperB 39
#define stepperC 34
#define stepperD 35

//#include <SD.h>
//#include <SPI.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <SimpleDHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Stepper.h>

#define STEPS 2038 // the number of steps in one revolution of your motor (28BYJ-48)

const char* ssid       = "******************";
const char* password   = "******************";
const char* ntpServer = "oceania.pool.ntp.org";//"pool.ntp.org";
const int   daylightOffset_sec = 3600;
const char* googleServer = "maker.ifttt.com";

IPAddress dns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress timeServer(13, 55, 50, 68); //0.au.pool.ntp.org
WiFiServer server(80);
SimpleDHT11 dht11(DHT11_PIN);
OneWire oneWire(tempData);
DallasTemperature sensors(&oneWire);
Stepper stepper(STEPS, stepperA, stepperB, stepperC, stepperD);

struct tm timeinfo;

const char * root_ca=\
"-----BEGIN CERTIFICATE-----\n" \
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n" \
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n" \
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n" \
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n" \
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n" \
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n" \
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n" \
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n" \
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n" \
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n" \
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n" \
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n" \
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n" \
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n" \
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n" \
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n" \
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n" \
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n" \
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n" \
"-----END CERTIFICATE-----\n";


bool waterCyclingRunning = false;
int waterCyclingSpeed = 3;
int waterCyclingStepsPerDrop = 180;
int waterCyclingDelayPerDrop = 2000;
int waterCyclingLastDrop = 0;
int waterRecyclingSpeed = 12;
int waterRecyclingStepsPerDrop = 180;
int waterRecyclingDelayPerDrop = 50;
int waterRecyclingLastDrop = 0;
bool doWaterRecycling = false;
bool doRecycleReverse = false;
int recycleLastEvent = 0;
int Relay4OffDelay = 300000;
int Relay4OnDelay = 120000;
int Relay4TempOnAfter = 25;
int Relay4TempOffAfter = 26;
int Relay4HumidOnAfter = 68;
int Relay4HumidOffAfter = 65;
int Relay3OffDelay = 300000;
int Relay3OnDelay = 120000;
int Relay3OnHourMin = 8;
int Relay3OnHourMax = 18;
int Relay2OffDelay = 300000;
int Relay2OnDelay = 300000;
int Relay2TempOnAfter = 20;
int Relay2TempOffAfter = 24;
int Relay1OffDelay = 30000;
int Relay1OnDelay = 30000;

int relay4Delay = 0;
int relay3Delay = 0;
int relay2Delay = 0;
int relay1Delay = 0;
bool relay4State = false;
bool relay3State = false;
bool relay2State = false;
bool relay1State = false;
bool forceRelay4 = false;
bool forceRelay3 = false;
bool forceRelay2 = false;
bool forceRelay1 = false;
bool forceRelay4State = false;
bool forceRelay3State = false;
bool forceRelay2State = false;
bool forceRelay1State = false;
String GOOGLE_SCRIPT_ID = "************************";
float waterTemp = 0;
float temperature = 0;
float humidity = 0;
float avgWaterTemp = 0.0;
float avgAirTemp = 0.0;
float avgHumidity = 0.0;
byte dataBufferIndex = 0;
long lastReadingTime = 0;
long lastTimeUpdate = 0;
long lastBufferWrite = 0;
time_t prevDisplay = 0; // when the digital clock was displayed
float timezone = 9.50;
bool timeSet = false;
byte second, minute, hour, day, date, month, year;
byte prevSecond, prevMinute, prevHour, prevDay, prevDate, prevMonth;
long  gmtOffset_sec = (long)(3600 * timezone);
bool isWifiOn = false;


void setup() {
  pinMode(pinRelay4, OUTPUT);
  pinMode(pinRelay3, OUTPUT);
  pinMode(pinRelay2, OUTPUT);
  pinMode(pinRelay1, OUTPUT);
  pinMode(tempVOut, OUTPUT);
  digitalWrite(pinRelay4, LOW);
  digitalWrite(pinRelay3, LOW);
  digitalWrite(pinRelay2, LOW);
  digitalWrite(pinRelay1, LOW);
  digitalWrite(tempVOut, HIGH);
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  turnOnWifi();
  // start listening for clients
  server.begin();

  // start the water sensor
  sensors.begin();


  updateLocalTime();
  // give the sensor and Ethernet shield time to set up:
  delay(1000);
  Serial.println("Server Active!");
}

void loop() {
  updateLocalTime();
  // check for a reading no more than once a second.
  if (millis() - lastReadingTime > 300) {
    getData();
    updateRelays();
    // timestamp the last time you got a reading:
    lastReadingTime = millis();
  }
  if (millis() - lastBufferWrite > 1470) {
    avgAirTemp += temperature;
    avgWaterTemp += waterTemp;
    avgHumidity += humidity;
    dataBufferIndex++;
    lastBufferWrite = millis();
    Serial.print(temperature);
    Serial.print("c - ");
    Serial.print(waterTemp);
    Serial.print("c - ");
    Serial.print(humidity);
    Serial.println("%");

    if (waterCyclingRunning){
        //only update the stepper speed every 1.5 seconds
        stepper.setSpeed(waterCyclingSpeed);
    }
  }

  if (millis() - lastTimeUpdate > 120000) { // every 2 minutes w log the data
    logData();
    lastTimeUpdate = millis();
    /* // disabling the wifi shutoff for review later
    if (hour > 20 and isWifiOn) {
      turnOffWifi();
    }
    else if (hour > 8 and hour < 20 and !isWifiOn) {
      turnOnWifi();
    }
    */
  }
  if (isWifiOn) {
    listenForClients();
  }
}


void updateRelays(){
  
  // =============================================
  //  Temp Huimidity control Dual Fans - Relay 4 Conditions
  // =============================================

  if (forceRelay4 and forceRelay4State){
      digitalWrite(pinRelay4, HIGH);
      relay4State = true;
      relay4Delay = millis();
  }
  else if (forceRelay4 and !forceRelay4State){
      digitalWrite(pinRelay4, LOW);
      relay4State = false;
      relay4Delay = millis();
  }
  else if (!relay4State) { 
    if (relay4Delay<millis() and  (temperature > 25 or humidity > 65)) {
      digitalWrite(pinRelay4, HIGH);
      relay4State = true;
      relay4Delay = millis()+300000;// can be turned off if conditions met after 5 mins
    }
  }
  else if (relay4State){
    if (relay4Delay<millis() and  humidity < 1) {
      digitalWrite(pinRelay4, HIGH);
      relay4State = true;
      relay4Delay = millis()+300000;// can be turned off if conditions met after 5 mins
    }
  }
  else if (relay4State){
    if (relay4Delay<millis() and  (temperature < 26 and humidity < 65)) {
      digitalWrite(pinRelay4, LOW);
      relay4State = false;
      relay4Delay = millis()+120000;// can be turned on if conditions met after 2 mins
    }
  }
  
  // =============================================
  //  Main Lights - Relay 3 Conditions
  // =============================================

  if (forceRelay3 and forceRelay3State){
      digitalWrite(pinRelay3, HIGH);
      relay3State = true;
      relay3Delay = millis();
  }
  else if (forceRelay3 and !forceRelay3State){
      digitalWrite(pinRelay3, LOW);
      relay3State = false;
      relay3Delay = millis();
  }
  else if (relay3Delay<millis() and !relay3State){
    if (hour >= 8 and hour <= 18) {// turn lights on
      digitalWrite(pinRelay3, HIGH);
      relay3State = true;
      relay3Delay = millis()+300000;// can be turned off if conditions met after 5 mins
    }
  }
  else if (relay3Delay<millis() and relay3State){
    if (hour <= 7 or hour >= 19) {
      digitalWrite(pinRelay3, LOW);
      relay3State = false;
      relay3Delay = millis()+120000;// can be turned on if conditions met after 2 mins
    }
  }
  
  // =============================================
  //  Water Heater - Relay 2 Conditions
  // =============================================

  if (forceRelay2 and forceRelay2State){
      digitalWrite(pinRelay2, HIGH);
      relay2State = true;
      relay2Delay = millis();
  }
  else if (forceRelay2 and !forceRelay2State){
      digitalWrite(pinRelay2, LOW);
      relay2State = false;
      relay2Delay = millis();
  }
  else if (!relay2State){
    if (waterTemp <= 20) {
      digitalWrite(pinRelay2, HIGH);
      relay2State = true;
      relay2Delay = millis()+30000;// can be turned on if conditions met after 30 seconds
    }
  }
  else if (relay2State){
    if (waterTemp >= 24) {
      digitalWrite(pinRelay2, LOW);
      relay2State = false;
      relay2Delay = millis()+30000;// can be turned on if conditions met after 30 seconds
    }
  }
  
  // =============================================
  //  Water Filter Pump - Relay 1 Conditions  Going to be switch to something else
  // =============================================

  if (forceRelay1 and forceRelay1State){
      digitalWrite(pinRelay1, HIGH);
      relay1State = true;
  }
  else if (forceRelay1 and !forceRelay1State){
      digitalWrite(pinRelay1, LOW);
      relay1State = false;
  }
  else if (!relay1State){
    if (hour >= 0) {
      digitalWrite(pinRelay1, HIGH);
      relay1State = true;
    }
  }
  else if (relay1State){
    if (hour > 25) {
      digitalWrite(pinRelay1, LOW);
      relay1State = false;
    }
  }
}

void logData() {
  Serial.print(temperature);
  Serial.print("c - ");
  Serial.print(waterTemp);
  Serial.print("c - ");
  Serial.print(humidity);
  Serial.print("% --- ");
  avgAirTemp = avgAirTemp/dataBufferIndex;
  avgWaterTemp = avgWaterTemp/dataBufferIndex;
  avgHumidity = avgHumidity/dataBufferIndex;
  dataBufferIndex = 0;
  Serial.print(avgAirTemp);
  Serial.print("c - ");
  Serial.print(avgWaterTemp);
  Serial.print("c - ");
  Serial.print(avgHumidity);
  Serial.println("%");

  HTTPClient http;
  String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?" + \
   "airtemp="+ String(avgAirTemp) + \
   "&watertemp=" + String(avgWaterTemp) + \
   "&humidity=" + String(avgHumidity) + \
   "&relayAbool=" + String(relay4State) + \
   "&relayBbool=" + String(relay3State) + \
   "&relayCbool=" + String(relay2State) + \
   "&relayDbool=" + String(relay1State); 

  Serial.println(url);
  //Serial.println("Making a request");
  http.begin(url, root_ca); //Specify the URL and certificate
  int httpCode = http.GET();  
  http.end();
  //Serial.println(": done "+httpCode);
}

void getData() {

  sensors.requestTemperatures(); 
  waterTemp = sensors.getTempCByIndex(0);
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    return;
  }
}

void listenForClients() {
  // listen for incoming clients
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {
    Serial.println("Got a client");
    // an http request ends with a blank line
    String currentLine = "";                // make a String to hold incoming data from the client
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          currentLine = "";
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<link rel=\"stylesheet\" type=\"text/css\" href=\"https://www.html.am/templates/downloads/bryantsmith/theslant/style.css\">");
          client.println("<body>");
          client.println("    <div id=\"page\">");
          client.println("        <div id=\"header\">");
          client.println("            <h1><font style=\"vertical-align: inherit;\"></font></h1>");
          client.println("            <ul>");
          client.println("                <li><a href=\"#\"><font style=\"vertical-align: inherit;\">home</font></a></li>");
          client.println("            </ul>");
          client.println("        </div>");
          client.println("        <div id=\"main\">");
          client.println("            <div class=\"main_top\">");
          client.println("                <h1><font style=\"vertical-align: inherit;\">Terrarium Status</font></h1>");
          client.println("            </div>");
          client.println("            <div class=\"main_body\">");
          client.print("                <p><font style=\"vertical-align: inherit;\">");
          if (hour>11) {
            client.print(hour-12);
            client.print(":");
            client.print(minute);
            client.print(" pm, ");
          }
          else {
            client.print(hour);
            client.print(":");
            client.print(minute);
            client.print(" am, ");
          }
          client.print(date);
          client.print("/");
          client.print(month);
          client.print("/");
          client.print(year);
          client.println("    <a href=\"/\">Refresh</a></font></p>");
          client.print("                <p><font style=\"vertical-align: inherit;\">");
          client.print("Air Temperature: ");
          client.print(temperature);
          client.print("c");
          client.println("</font></p>");
          client.print("                <p><font style=\"vertical-align: inherit;\">");
          client.print("Water Temperature: ");
          client.print(waterTemp    );
          client.print("c");
          client.println("</font></p>");
          client.print("                <p><font style=\"vertical-align: inherit;\">");
          client.print("Humidity: ");
          client.print(humidity);
          client.print("%");
          client.println("</font></p>");
          client.print("                <p><font style=\"vertical-align: inherit;\">");
          if (relay4State) {
            client.print("Dual Fans: ON ");
          }
          else{
            client.print("Dual Fans: OFF ");
          }
          if (forceRelay4) {
            client.print(": Manually Set | ");
          }
          else{
            client.print(": Automatic | ");
          }
          client.println("<a href=\"/TurnOn4\">Turn On</a> | <a href=\"/TurnOff4\">Turn Off</a> | <a href=\"/ForceOn4\">Manual</a> | <a href=\"/ForceOff4\">Auto</a></font></p>");
          client.print("                <p><font style=\"vertical-align: inherit;\">");
          if (relay3State) {
            client.print("Main Lights: ON ");
          }
          else{
            client.print("Main Lights: OFF ");
          }
          if (forceRelay3) {
            client.print(": Manually Set | ");
          }
          else{
            client.print(": Automatic | ");
          }
          client.println("<a href=\"/TurnOn3\">Turn On</a> | <a href=\"/TurnOff3\">Turn Off</a> | <a href=\"/ForceOn3\">Manual</a> | <a href=\"/ForceOff3\">Auto</a></font></p>");
          client.print("                <p><font style=\"vertical-align: inherit;\">");
          if (relay2State) {
            client.print("Water Heater: ON ");
          }
          else{
            client.print("Water Heater: OFF ");
          }
          if (forceRelay2) {
            client.print(": Manually Set | ");
          }
          else{
            client.print(": Automatic | ");
          }
          client.println("<a href=\"/TurnOn2\">Turn On</a> | <a href=\"/TurnOff2\">Turn Off</a> | <a href=\"/ForceOn2\">Manual</a> | <a href=\"/ForceOff2\">Auto</a></font></p>");
          client.print("                <p><font style=\"vertical-align: inherit;\">");
          if (relay1State) {
            client.print("Water Filter: ON ");
          }
          else{
            client.print("Water Filter: OFF ");
          }
          if (forceRelay1) {
            client.print(": Manually Set | ");
          }
          else{
            client.print(": Automatic | ");
          }
          client.println("<a href=\"/TurnOn1\">Turn On</a> | <a href=\"/TurnOff1\">Turn Off</a> | <a href=\"/ForceOn1\">Manual</a> | <a href=\"/ForceOff1\">Auto</a></font></p>");
          client.println("            </div>");
          client.println("            <div class=\"main_bottom\"></div>");
          client.println("        </div>");
          client.println("        <div id=\"footer\">");
          client.println("        </div>");
          client.println("</body>");

          // print the current readings, in HTML format:
          //client.println(timeClient.getFormattedTime());
          break;
        }
        //else {    // if you got a newline, then clear currentLine:
        //} 
        else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /TurnOn4")) {
          forceRelay4State = true;
          forceRelay4 = true;
        }
        else if (currentLine.endsWith("GET /TurnOff4")) {
          forceRelay4State = false;
          forceRelay4 = true;
        }
        else if (currentLine.endsWith("GET /TurnOn3")) {
          forceRelay3State = true;
          forceRelay3 = true;
        }
        else if (currentLine.endsWith("GET /TurnOff3")) {
          forceRelay3State = false;
          forceRelay3 = true;
        }
        else if (currentLine.endsWith("GET /TurnOn2")) {
          forceRelay2State = true;
          forceRelay2 = true;
        }
        else if (currentLine.endsWith("GET /TurnOff2")) {
          forceRelay2State = false;
          forceRelay2 = true;
        }
        else if (currentLine.endsWith("GET /TurnOn1")) {
          forceRelay1State = !forceRelay1State;
          forceRelay1 = true;
        }
        else if (currentLine.endsWith("GET /TurnOff1")) {
          forceRelay1State = false;
          forceRelay1 = true;
        }
        else if (currentLine.endsWith("GET /ForceOn4")) {
          forceRelay4 = true;
        }
        else if (currentLine.endsWith("GET /ForceOff4")) {
          forceRelay4 = false;
          forceRelay4State = true;
        }
        else if (currentLine.endsWith("GET /ForceOn3")) {
          forceRelay3 = true;
        }
        else if (currentLine.endsWith("GET /ForceOff3")) {
          forceRelay3 = false;
          forceRelay3State = true;
        }
        else if (currentLine.endsWith("GET /ForceOn2")) {
          forceRelay2 = true;
        }
        else if (currentLine.endsWith("GET /ForceOff2")) {
          forceRelay2 = false;
          forceRelay2State = true;
        }
        else if (currentLine.endsWith("GET /ForceOn1")) {
          forceRelay1 = true;
        }
        else if (currentLine.endsWith("GET /ForceOff1")) {
          forceRelay1 = false;
          forceRelay1State = true;
        }
        else if (currentLine.endsWith("GET /reverseRecycle")) {
          doRecycleReverse = true;
        }
        else if (currentLine.endsWith("GET /forwardRecycle")) {
          doRecycleReverse = false;
        }
        else if (currentLine.endsWith("GET /doWaterRecycling")) {
          doWaterRecycling = true;
        }
        else if (currentLine.endsWith("GET /doWaterCycling")) {
          doWaterRecycling = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
  }
}


void updateWaterPump() {
    if (waterCyclingRunning) {
        if (doWaterRecycling) {
            if (millis()+waterRecyclingDelayPerDrop>recycleLastEvent){
                if (doRecycleReverse){
                    stepper.step(-waterCyclingStepsPerDrop);
                    recycleLastEvent = millis();
                }
                else {
                    stepper.step(waterCyclingStepsPerDrop);
                    recycleLastEvent = millis();
                }
            }
        }
        else if (millis()+waterCyclingDelayPerDrop>waterCyclingLastDrop){
            stepper.step(waterCyclingStepsPerDrop);
            waterCyclingLastDrop = millis();
        }
    }
}


void getWebTime() {
  //connect to WiFi
  //passing all these varables results in a rapid wifi on off in under half a second
  //ssid, passwd  channel, bssid  ip, mask, gw, dn
  //WiFi.config(ip, dns, gateway, subnet);
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  updateLocalTime();
  printLocalTime();
  Serial.println("Updated time from web time");
}

// Time based functions

void printLocalTime() {
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void updateLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time B");
    return;
  }
  // if minute has changed update all fields, else just update second
  if (minute != byte(timeinfo.tm_min)) {

    year   = int(timeinfo.tm_year);
    month  = byte(timeinfo.tm_mon)+1;
    date   = byte(timeinfo.tm_mday);
    day    = byte(timeinfo.tm_wday);
    hour   = byte(timeinfo.tm_hour);
    minute = byte(timeinfo.tm_min);
  }
  if (second != byte(timeinfo.tm_sec)) {
    second = byte(timeinfo.tm_sec);
  }
}


void turnOnWifi() {
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  int startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    // if we haven't connected in 10 seconds...:
    if (millis() > startTime + 10000) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      return;
    }
  }
  Serial.println("Connected!");
  isWifiOn = true;
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  getWebTime();
}

void turnOffWifi() {

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  isWifiOn = false;
  Serial.println("Wifi Disconnected!");

}
