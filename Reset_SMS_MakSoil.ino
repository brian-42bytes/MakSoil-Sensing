
#include "EEPROM.h"
#include <Arduino.h>
#include "SD.h"
#include "SPI.h"
#include "esp_task_wdt.h"

#define TINY_GSM_MODEM_SIM7000

#include <TinyGsmClient.h>

#define RE_DE_PIN   25
#define SENSOR_RX   21
#define SENSOR_TX   22
#define SENSOR_BAUD 4800
#define SD_CS       13

//*************************************************************************
// ======================= PARAMETERS FOR SETTINGS =============================
//*************************************************************************
#define MAX_NAME_LEN     32
#define MAX_LANG_LEN     8      // e.g. "en", "sw", "fr"
#define MAX_PHONE_LEN    14     // phone number
#define CONFIG_ADDR      0

struct Config {
  uint16_t version = 1;

  char deviceName[MAX_NAME_LEN + 1] = "";
  char language[MAX_LANG_LEN + 1]     = "";     // default English
  char contactPhone[MAX_PHONE_LEN + 1] = "";      // empty by default
};

Config config;

String deviceName = "THE FARMERS GROUP";
String language   = "100";
String contactPhone = "+256000000000";

//****************************************************************************
// ===================== ACCOUNT AND REMOTE_DB ACCESS ================================
//****************************************************************************

#define SENSOR_ID "CSP001" //Cosin Sensor Prototype 001
const long CHANNEL_ID = 1234567;  

#define APN         "web.gprs.mtn.co.ug"

const char* thingspeakHost = "api.thingspeak.com";
String apiKey = "IVAWTICG5ZGI####"; 

//**************************************************************************
// ====================== OTHER VARIABLES AND OBJECT ===========================
//**************************************************************************

HardwareSerial modbus(1);
HardwareSerial SerialAT(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

uint8_t response[32];
uint8_t cmd[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

const int TIME_ZONE_OFFSET = 3;
const int BUFFER_SIZE = 10; 

struct Reading {
  float moisture, temperature, ec, ph;
  float nitrogen, phosphorus, potassium, fertilityScore;
  int year, month, day, hour, minute;
  float lat, lon, alt;
  bool hasGPS;
};

Reading buffer[BUFFER_SIZE];
int bufferIndex = 0;

float avgMoisture = 0, avgTemp = 0, avgEC = 0, avgPH = 0;
float avgN = 0, avgP = 0, avgK = 0, avgFertility = 0;
float currentLat = 0, currentLon = 0, currentAlt = 0;

bool sdOK = false;
bool hasGPSFix = false;

int currentYear, currentMonth, currentDay, currentHour,currentMinute;
int local_hour=0, local_day=0, local_month=0, local_year=0;

//*********************************************************************************************************************
// ====================================================== SETUP ==========================================================
//*********************************************************************************************************************

void setup() {

  Serial.begin(115200);
  EEPROM.begin(512);
  modbus.begin(SENSOR_BAUD, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
  
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);
      
  // Watchdog Timer (prevents resetting)
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 30000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
  

  // SD Card
  SPI.begin(14, 2, 15, SD_CS);
  if (SD.begin(SD_CS)) {
    sdOK = true;
    Serial.println("✅ SD Card initialized successfully");
  } else {
    Serial.println("❌ SD Card initialization failed");
  }

  // SIM7000G Power On
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);  delay(1000);
  digitalWrite(4, HIGH); delay(6000);

  SerialAT.begin(115200, SERIAL_8N1, 26, 27);

  if (modem.restart() || modem.init()) {
    Serial.println("✅ Modem initialized");
  } else {
    Serial.println("❌ Modem initialization failed");
  }

  modem.waitForNetwork(30000L);
  if (modem.gprsConnect(APN, "", "")) {
    Serial.println("✅ GPRS Connected");
  }

  // Setup SMS reception
  Serial.println("Setting up SMS reception...");
  modem.sendAT("+CMGF=1");      // Text mode
  modem.waitResponse();
  modem.sendAT("+CNMI=1,2,0,0,0"); // Forward new SMS to serial immediately
  modem.waitResponse();
  Serial.println("SMS reception ready");

  Serial.println("=== System Ready - Collecting 10 readings before sending ===");
}


//*********************************************************************************************************************
// ====================================================== LOOP ==========================================================
//*********************************************************************************************************************

void loop() {
  
  esp_task_wdt_reset();
  yield();

  takeOneReading();

  if (bufferIndex >= BUFFER_SIZE) {
    processBatch();
    bufferIndex = 0;
  }

  delay(10000);   // Take a reading every 10 seconds
}


//**********************************************************************************
// ====================== CORE FUNCTIONS ======================
//**********************************************************************************

//***************************************** COLLECTING READINGS *****************************************
void takeOneReading() {
  digitalWrite(RE_DE_PIN, HIGH);
  modbus.write(cmd, 8);
  modbus.flush();
  digitalWrite(RE_DE_PIN, LOW);
  delay(250);

  if (modbus.available() >= 19) {
    modbus.readBytes(response, 32);

    if (response[0] == 0x01 && response[1] == 0x03 && response[2] == 0x0E) {
      
      // Store reading
      buffer[bufferIndex].moisture     = ((response[3] << 8) | response[4]) / 10.0;
      buffer[bufferIndex].temperature  = ((response[5] << 8) | response[6]) / 10.0;
      buffer[bufferIndex].ec           = (response[7] << 8)  | response[8];
      buffer[bufferIndex].ph           = ((response[9] << 8) | response[10]) / 10.0;
      buffer[bufferIndex].nitrogen     = (response[11] << 8) | response[12];
      buffer[bufferIndex].phosphorus   = (response[13] << 8) | response[14];
      buffer[bufferIndex].potassium    = (response[15] << 8) | response[16];

      buffer[bufferIndex].fertilityScore = ((buffer[bufferIndex].nitrogen/250.0) + 
                                           (buffer[bufferIndex].phosphorus/60.0) + 
                                           (buffer[bufferIndex].potassium/200.0)) / 3.0 * 100.0;

      getGPS();
      buffer[bufferIndex].year = local_year;
      buffer[bufferIndex].month = local_month;
      buffer[bufferIndex].day = local_day;
      buffer[bufferIndex].hour = local_hour;
      buffer[bufferIndex].minute = currentMinute;
      buffer[bufferIndex].lat = currentLat;
      buffer[bufferIndex].lon = currentLon;
      buffer[bufferIndex].alt = currentAlt;
      buffer[bufferIndex].hasGPS = hasGPSFix;

      bufferIndex++;
      Serial.printf("Collected reading %d of 10\n", bufferIndex);
    }
  }
}

//***************************************** GPS POWER AND READINGS *****************************************
void getGPS() {
  modem.sendAT("+SGPIO=0,4,1,1");
  modem.waitResponse(3000);
  modem.enableGPS();
  delay(6000);

  hasGPSFix = modem.getGPS(&currentLat, &currentLon, nullptr, &currentAlt, nullptr, nullptr, nullptr,
                          &currentYear, &currentMonth, &currentDay, &currentHour, &currentMinute, nullptr);

  if (hasGPSFix) {
    local_hour = currentHour + TIME_ZONE_OFFSET;
    local_day = currentDay;
    local_month = currentMonth;
    local_year = currentYear;
    if (local_hour >= 24) {
      local_hour -= 24;
      local_day++;
      if (local_day > 31) {
        local_day = 1;
        local_month++;
        if (local_month > 12) {
          local_month = 1;
          local_year++;
        }
      }
    }
  }

  modem.sendAT("+SGPIO=0,4,1,0");
  modem.waitResponse(2000);
  modem.disableGPS();
}

//***************************************** PROSSESSING READINGS *****************************************
void processBatch() {
  float sumM=0, sumT=0, sumEC=0, sumPH=0, sumN=0, sumP=0, sumK=0, sumF=0;
  int count = 0;

  for(int i = 0; i < bufferIndex; i++) {
    sumM += buffer[i].moisture;     sumT += buffer[i].temperature;
    sumEC += buffer[i].ec;          sumPH += buffer[i].ph;
    sumN += buffer[i].nitrogen;     sumP += buffer[i].phosphorus;
    sumK += buffer[i].potassium;    sumF += buffer[i].fertilityScore;
    count++;
  }

  avgMoisture = sumM / count;
  avgTemp = sumT / count;
  avgEC = sumEC / count;
  avgPH = sumPH / count;
  avgN = sumN / count;
  avgP = sumP / count;
  avgK = sumK / count;
  avgFertility = sumF / count;

  Serial.println("\n=== 10 READINGS PROCESSED - AVERAGES CALCULATED ===");
  Serial.printf("Moisture: %.1f%% | Temp: %.1f°C | pH: %.1f | Fertility: %.1f%%\n", 
                avgMoisture, avgTemp, avgPH, avgFertility);

  saveBatchToSD();
  sendAveragedToThingSpeak();
}

//***************************************** SAVING TO SD CARD *****************************************
void saveBatchToSD() {
  if (!sdOK) return;

  char filename[20];
  
  sprintf(filename, "/SoilData_%04d%02d.csv", local_year, local_month);   // You can improve date later

  File f = SD.open(filename, FILE_APPEND);
  if (f) {
    if (f.size() == 0) {
      f.println("DateTime,Moisture(%),Temp(°C),EC(uS/cm),pH,N(mg/kg),P,K,Fertility(%),Lat,Lon,Alt");
    }

    for(int i = 0; i < bufferIndex; i++) {
      f.printf("%02d-%02d:%02%,%.1f,%.1f,%.0f,%.1f,%.0f,%.0f,%.0f,%.1f,", buffer[i].day, buffer[i].hour, buffer[i].minute, buffer[i].moisture,
               buffer[i].temperature, buffer[i].ec, buffer[i].ph, buffer[i].nitrogen, buffer[i].phosphorus, buffer[i].potassium, buffer[i].fertilityScore);
      
      if (currentLat != 0 && currentLon != 0) {
        f.printf("%.6f,%.6f,%.1f\n", currentLat, currentLon, currentAlt);
      } else {
        f.println("noGPS,noGPS,noGPS");
      }

    }
    
    f.close();
    Serial.println("✅ Batch saved to SD card");
  }
}

//***************************************** SENDING TO REMOTE DATABASE FOR INTERPRETATION *****************************************
void sendAveragedToThingSpeak() {
  if (!client.connect(thingspeakHost, 80)) {
    Serial.println("❌ Failed to connect to ThingSpeak");
    return;
  }

  String url = "/update?api_key=" + apiKey +
               "&field1=" + String(avgN) +
               "&field2=" + String(avgP) +
               "&field3=" + String(avgK) +
               "&field4=" + String(avgPH,1) +
               "&field5=" + String(avgMoisture,1) +
               "&field6=" + String(avgEC) +
               "&field7=" + String(avgFertility,1);

  if (currentLat != 0 && currentLon != 0) {
    url += "&lat=" + String(currentLat,6) + "&long=" + String(currentLon,6);
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\nHost: " + thingspeakHost + "\r\nConnection: close\r\n\r\n");
  delay(2000);
  client.stop();

  Serial.println("✅ Averaged data sent to ThingSpeak");
  sendSMSAlert();
}

//***************************************** SMS SENDING *****************************************
void sendSMSAlert() {
  String msg = "Soil Data Updated!\n";
  msg += "Fertility Score: " + String(avgFertility,1) + "%\n";
  msg += "Moisture: " + String(avgMoisture,1) + "% | pH: " + String(avgPH,1) + "\n";
  msg += "View full data: https://thingspeak.com/channels/" + String(CHANNEL_ID);

  if (modem.sendSMS(contactPhone, msg)) {
    Serial.println("✅ SMS Sent Successfully!");
  } else {
    Serial.println("❌ SMS sending failed");
  }
}

//***************************************** SMS RECEAVING for SETTINGS *****************************************
void checkIncomingSMS() {
  while (SerialAT.available()) {
    String line = SerialAT.readStringUntil('\n');
    line.trim();

    if (line.startsWith("+CMT:")) {        // New SMS notification
      // The next line should be the message body
      String message = SerialAT.readStringUntil('\n');
      message.trim();

      Serial.println("New SMS Received: " + message);

      message.toUpperCase();

      String reply = "";

      if (message.indexOf("STATUS") != -1 || message.indexOf("READ") != -1) {
        reply = "Current Soil Status:\n";
        reply += "Fertility: " + String(avgFertility, 1) + "%\n";
        reply += "Moisture: " + String(avgMoisture, 1) + "%\n";
        reply += "pH: " + String(avgPH, 1);
      } 
      else if (message.indexOf("GPS") != -1) {
        reply = "GPS Location:\n";
        if (hasGPSFix && currentLat != 0 && currentLon != 0) {
          reply += "Lat: " + String(currentLat, 6) + "\n";
          reply += "Lon: " + String(currentLon, 6);
        } else {
          reply += "No GPS fix yet";
        }
      } 
      else {
        reply = "Available commands:\nSTATUS\nGPS";
      }

      modem.sendSMS(contactPhone, reply);
      Serial.println("✅ Reply SMS sent");
    }
  }
}


//***************************************** LOAD ALL SETTINGS *****************************************
void loadConfig() {
  EEPROM.get(CONFIG_ADDR, config);

  // First time or corrupted memory
  if (config.version != 1 || config.deviceName[0] == '\0') {
    strcpy(config.deviceName, "THE FARMER");
    strcpy(config.language, "100");
    strcpy(config.contactPhone, "");        // or your default number
    config.version = 1;
    saveConfig();
    Serial.println("First run - Default settings saved");
  }

  // Copy to String variables for easy use
  deviceName   = String(config.deviceName);
  language     = String(config.language);
  contactPhone = String(config.contactPhone);

  Serial.println("✅ Loaded Settings:");
  Serial.println("   Name: " + deviceName);
  Serial.println("   Lang: " + language);
  Serial.println("   Phone: " + contactPhone);
}

//***************************************** SAVE NEW SETTINGS *****************************************
void saveConfig() {
  EEPROM.put(CONFIG_ADDR, config);

#if defined(ESP8266) || defined(ESP32)
  EEPROM.commit();
#endif

  // Update String variables
  deviceName   = String(config.deviceName);
  language     = String(config.language);
  contactPhone = String(config.contactPhone);

  Serial.println("💾 All settings saved to EEPROM");
}

//***************************************** Update SETTINGS *****************************************
void updateDeviceName(String newName) {
  newName.trim();
  if (newName.length() > 0 && newName.length() <= MAX_NAME_LEN) {
    strcpy(config.deviceName, newName.c_str());
    saveConfig();
  }
}

void updateLanguage(String newLang) {
  newLang.trim();
  if (newLang.length() > 0 && newLang.length() <= MAX_LANG_LEN) {
    strcpy(config.language, newLang.c_str());
    saveConfig();
  }
}

void updateContactPhone(String newPhone) {
  newPhone.trim();
  if (newPhone.length() <= MAX_PHONE_LEN) {
    strcpy(config.contactPhone, newPhone.c_str());
    saveConfig();
  }
}