#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#define TINY_GSM_MODEM_SIM7000

// ================== SENSOR_PIN CONFIG ==================
#define RE_DE_PIN   25
#define SENSOR_RX   21
#define SENSOR_TX   22
#define SENSOR_BAUD 4800

#define SD_CS       13     // Typical CS pin for LILYGO T-SIM7000G SD card

#define APN         "web.gprs.mtn.co.ug"

const char* thingspeakHost = "api.thingspeak.com";
String apiKey = "IVAWTICG5ZGI***#";        //Write API Key

const int TIME_ZONE_OFFSET = 3;   // UTC+3 EAT

#include <TinyGsmClient.h>

HardwareSerial modbus(1);
HardwareSerial SerialAT(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

uint8_t response[32];
uint8_t cmd[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

float moisture = 0.0, temperature = 0.0, ec = 0.0, ph = 0.0;
float nitrogen = 0.0, phosphorus = 0.0, potassium = 0.0, fertilityScore = 0.0;

float lat = 0.0, lon = 0.0, alt = 0.0, accuracy = 0.0;
int year=0, month=0, day=0, hour=0, minute=0;
int local_hour = 0, local_day = 0, local_month = 0, local_year = 0;

static bool state = 0;
static bool hasFix = 0;

File dataFile;
char filename[20];

void setup() {
  Serial.begin(115200);
  modbus.begin(SENSOR_BAUD, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
  
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);

  //Start SD Card
  if (!SD.begin(SD_CS)) {
    (state & (1 << 1)) ? state = state | (1 << 1) : state = state;
    Serial.println(" SD card failed!");
  } else {
    (state & (1 << 1)) ? state = state : state = state | (1 << 1);
    Serial.println(" SD card OK");
  }

  // Power on SIM7000G
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  delay(500);
  digitalWrite(4, HIGH);
  delay(2000);

  SerialAT.begin(115200, SERIAL_8N1, 26, 27);

  if (!modem.restart() && !modem.init()) {
    Serial.println("Modem init failed!");
    while (true);
  }
  state = (state | 128);
  Serial.println("✅ Modem Working!");

  modem.waitForNetwork(20000L);
  modem.gprsConnect(APN, "", "");

  // Check GPRS status first
  (!modem.isGprsConnected()) ? state = state | (1 << 2) : modem.gprsConnect(APN, "", "");
}

void loop() {
  //unsigned long now = millis(); // for more LED indicator and button functionality to be added later
  // Read Soil Sensor continously as long as device is powered
  readSensor();

  //read from gps ig modem initialized successfully
  if (state & 136) {
    gpsMode(5000); //wait longer for satelites to fix with the device
  } else {
    gpsMode(1000); //wait for a second if the fix already exists
  }

  //parse and print values from sensor if modem initialized and sensor responded with actual values
  if (state & 224) {
    parseValues();
    serialPrint(); //For visualization but not neccesary
  }
  
  //save to SD card and logOnline even ig gps is not running
  if ((state & 234) || (state & 226)) {
    logToSD();        // Save to SD card with timestamp + coordinates
    logToWeb();
  }
  


  delay(3000);  
}


// ====================================== HELPER FUNCTIONS ===========================================

void readSensor() {
  digitalWrite(RE_DE_PIN, HIGH);
  modbus.write(cmd, 8);
  modbus.flush();
  digitalWrite(RE_DE_PIN, LOW);
  delay(250);

  if (modbus.available() < 19) {//Alarm to alert for this jam
    Serial.println("No sensor response");
    (state & (128 >> 1)) ? state = state | (128 >> 1) : state = state;
    delay(5000);
    return;
  }

  (state & (128 >> 1)) ? state = state : state = state | (128 >> 1);
  modbus.readBytes(response, 32);

  if (!(response[0] == 0x01 && response[1] == 0x03 && response[2] == 0x0E)) {//Alarm to alert for this jam
    Serial.println("Invalid sensor response");
    (state & (128 >> 2)) ? state = state | (128 >> 2) : state = state;
    return;
  }

  (state & (128 >> 2)) ? state = state : state = state | (128 >> 2);
}

void parseValues() {
  moisture     = ((response[3] << 8) | response[4]) / 10.0;
  temperature  = ((response[5] << 8) | response[6]) / 10.0;
  ec           = (response[7] << 8) | response[8];
  ph           = ((response[9] << 8) | response[10]) / 10.0;
  nitrogen     = (response[11] << 8) | response[12];
  phosphorus   = (response[13] << 8) | response[14];
  potassium    = (response[15] << 8) | response[16];

  fertilityScore = ((nitrogen/250.0) + (phosphorus/60.0) + (potassium/200.0)) / 3.0 * 100.0;
}

void gpsMode(uint16_t gpsdelay) {
  modem.sendAT("+SGPIO=0,4,1,1");
  modem.waitResponse(5000);

  modem.enableGPS();
  delay(gpsdelay);
  hasFix = modem.getGPS(&lat, &lon, nullptr, &alt, nullptr, nullptr, &accuracy,
                        &year, &month, &day, &hour, &minute, nullptr);

  if (hasFix) {
    local_hour = hour + TIME_ZONE_OFFSET;
    local_day = day;
    local_month = month;
    local_year = year;
    if (local_hour >= 24) {
      local_hour -= 24;
      local_day++;
      if (local_day > 31) { local_day = 1; local_month++; if (local_month > 12) { local_month = 1; local_year++; } }
    }
  }
  modem.sendAT("+SGPIO=0,4,1,0");
  modem.waitResponse(5000);

  modem.disableGPS();
}

void logToSD() {
  sprintf(filename, "/Soil_%04d-%02d.csv", local_year, local_month);

  if (!SD.exists(filename)) {
    dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
      dataFile.println("DateTime,Moisture(%),Temperature(°C),EC(uS/cm),pH,N(mg/kg),P(mg/kg),K(mg/kg),FertilityScore(%),Latitude,Longitude,Altitude(m)");
      dataFile.close();
    }
  }

  dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    // Timestamp + all data
    dataFile.print(local_year);
    dataFile.print("-");
    dataFile.print(local_month < 10 ? "0" : "");
    dataFile.print(local_month);
    dataFile.print("-");
    dataFile.print(local_day < 10 ? "0" : "");
    dataFile.print(local_day);
    dataFile.print(" ");
    dataFile.print(local_hour < 10 ? "0" : "");
    dataFile.print(local_hour);
    dataFile.print(":");
    dataFile.print(minute < 10 ? "0" : "");
    dataFile.print(minute);
    dataFile.print(",");

    dataFile.print(moisture, 1); dataFile.print(",");
    dataFile.print(temperature, 1); dataFile.print(",");
    dataFile.print(ec); dataFile.print(",");
    dataFile.print(ph, 1); dataFile.print(",");
    dataFile.print(nitrogen); dataFile.print(",");
    dataFile.print(phosphorus); dataFile.print(",");
    dataFile.print(potassium); dataFile.print(",");
    dataFile.print(fertilityScore, 1); dataFile.print(",");

    if (hasFix && lat != 0 && lon != 0) {
      dataFile.print(lat, 6); dataFile.print(",");
      dataFile.print(lon, 6); dataFile.print(",");
      dataFile.println(alt, 1);
    } else {
      dataFile.println("noLAT,noLONG,noALT");
    }
    
    dataFile.close();

    (state & 1) ? state = state : state = state | 1;
    Serial.println("Logged to SD card");
  } else {
    (state & 1) ? state = state | 1 : state = state;
    Serial.println("Failed to open SD file");
  }
}

void logToWeb() {
  if (client.connect(thingspeakHost, 80)) {
    String url = "/update?api_key=" + apiKey +
                 "&field1=" + String(nitrogen) +           // Nitrogen_Level
                 "&field2=" + String(phosphorus) +         // Phosphorus_Level
                 "&field3=" + String(potassium) +          // Potassium_Level
                 "&field4=" + String(ph, 1) +              // Soil_pH
                 "&field5=" + String(moisture, 1) +        // Soil_Moisture
                 "&field6=" + String(ec) +                 // Electrical_Conductivity
                 "&field7=" + String(fertilityScore, 1) +  // Fertility_Score
                 "&field8=" + String(alt, 1);              // Altitude

    if (hasFix && lat != 0 && lon != 0) {
      //url += "&lat=" + String(lat,6) + "&long=" + String(lon,6);
      //Send coordinates to webapp for fertility mapping
    }

    (state & (128 >> 3)) ? state = state : state = state | (128 >> 3);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + thingspeakHost + "\r\n" +
                 "Connection: close\r\n\r\n");
    
    delay(3000);
  } else {//Alarm to alert for this jam
  (state & (128 >> 3)) ? state = state | (128 >> 3) : state = state;
    Serial.println("Failed to connect to ThingSpeak");
  }
  client.stop();
}


void serialPrint() {
  Serial.printf("Moisture: %.1f %%\n", moisture);
  Serial.printf("Temperature: %.1f °C\n", temperature);
  Serial.printf("EC: %.0f µS/cm\n", ec);
  Serial.printf("pH: %.1f\n", ph);
  Serial.printf("Nitrogen: %.0f mg/kg\n", nitrogen);
  Serial.printf("Phosphorus: %.0f mg/kg\n", phosphorus);
  Serial.printf("Potassium: %.0f mg/kg\n", potassium);
  Serial.printf("Fertility Score: %.1f%%\n", fertilityScore);

  if (hasFix && lat != 0 && lon != 0) {  
    Serial.printf("GPS: %.6f, %.6f | Accuracy: %.2f m | Altitude: %.1f m\n", lat, lon, accuracy, alt);
    Serial.printf("Local Time (EAT): %04d-%02d-%02d %02d:%02d\n", 
                  local_year, local_month, local_day, local_hour, minute);
  }
}
