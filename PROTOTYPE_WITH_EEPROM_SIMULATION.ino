#include "EEPROM.h"
#include <Arduino.h>
#include "SD.h"
#include "SPI.h"
//#include "esp_task_wdt.h"

#define TINY_GSM_MODEM_SIM7000

#include <TinyGsmClient.h>

#define RE_DE_PIN   25
#define SENSOR_RX   21
#define SENSOR_TX   22
#define SENSOR_BAUD 4800
#define SD_CS       13

// ================== SETTINGS ==================
#define MAX_NAME_LEN     32
#define MAX_LANG_LEN     8
#define MAX_PHONE_LEN    14
#define CONFIG_ADDR      0
const char* languages[] =  {
  "ENGLISH",
  "KISWAHILI",
  "LUGANDA"
};

struct Config {
  uint16_t version = 1 ;
  char deviceName[MAX_NAME_LEN + 1] = "";
  char language[MAX_LANG_LEN + 1]   = "";
  char contactPhone[MAX_PHONE_LEN + 1] = "";
};

Config config;
bool changeConfig = false;

String deviceName;
String language;
String contactPhone;
String message;
String reply = "";

// ================== THINGSPEAK ==================
const char* SENSOR_ID = "CSP001";
const long CHANNEL_ID = 3347358;
#define APN         "web.gprs.mtn.co.ug"

const char* thingspeakHost = "api.thingspeak.com";
String apiKey = "IVAWTICG5ZGIOUT2";

// ================== OBJECTS ==================
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
};

Reading buffer[BUFFER_SIZE];
int bufferIndex = 0;

float avgMoisture = 0, avgTemp = 0, avgEC = 0, avgPH = 0;
float avgN = 0, avgP = 0, avgK = 0, avgFertility = 0;
float lat = 0, lon = 0, alt = 0;

bool sdOK = false;
bool hasGPSFix = false;
bool hasGPS = false;

int currentYear = 0, currentMonth = 0, currentDay = 0, currentHour = 0, minute = 0;
int local_hour = 0, local_day = 0, local_month = 0, local_year = 0;

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  modbus.begin(SENSOR_BAUD, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);

  /*/ Watchdog - Initialize only once
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 30000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);*/

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
  digitalWrite(4, HIGH); delay(4000);

  SerialAT.begin(115200, SERIAL_8N1, 26, 27);

  if (modem.restart() || modem.init()) {
    Serial.println("✅ Modem initialized");
  }

  modem.waitForNetwork(15000L);
  if (modem.gprsConnect(APN, "", "")) {
    Serial.println("✅ GPRS Connected");
  }

  // SMS Setup
  modem.sendAT("+CMGF=1");
  modem.waitResponse();
  modem.sendAT("+CNMI=1,2,0,0,0");
  modem.waitResponse();

  // Load settings from EEPROM
  loadConfig();

  Serial.println("=== System Ready - Collecting 10 readings ===");
}

// ================== LOOP ==================
void loop() {
  // esp_task_wdt_reset();
  // yield();

  takeOneReading();

  if (bufferIndex >= BUFFER_SIZE) {
    getGPS(); //get time and gps data
    
    hasGPS = hasGPSFix;

    processBatch();
    bufferIndex = 0;
  }

  checkIncomingSMS();     // ← Now called every loop

  delay(2000);
}

// ================== CORE FUNCTIONS ==================

void takeOneReading() {
  digitalWrite(RE_DE_PIN, HIGH);
  modbus.write(cmd, 8);
  modbus.flush();
  digitalWrite(RE_DE_PIN, LOW);
  delay(250);

  if (modbus.available() >= 19) {
    modbus.readBytes(response, 32);

    if (response[0] == 0x01 && response[1] == 0x03 && response[2] == 0x0E) {
      buffer[bufferIndex].moisture     = ((response[3] << 8) | response[4]) / 10.0;
      buffer[bufferIndex].temperature  = ((response[5] << 8) | response[6]) / 10.0;
      buffer[bufferIndex].ec           = ((response[7] << 8) | response[8]);
      buffer[bufferIndex].ph           = ((response[9] << 8) | response[10]) / 10.0;
      buffer[bufferIndex].nitrogen     = (response[11] << 8) | response[12];
      buffer[bufferIndex].phosphorus   = (response[13] << 8) | response[14];
      buffer[bufferIndex].potassium    = (response[15] << 8) | response[16];

      buffer[bufferIndex].fertilityScore = ((buffer[bufferIndex].nitrogen/250.0) + 
                                           (buffer[bufferIndex].phosphorus/60.0) + 
                                           (buffer[bufferIndex].potassium/200.0)) / 3.0 * 100.0;

      
      bufferIndex++;
      Serial.printf("Collected reading %d of 10\n", bufferIndex);
    }
  }
}

void getGPS() {
  modem.sendAT("+SGPIO=0,4,1,1");
  modem.waitResponse(3000);
  modem.enableGPS();
  delay(6000);

  hasGPSFix = modem.getGPS(&lat, &lon, nullptr, &alt, nullptr, nullptr, nullptr,
                          &currentYear, &currentMonth, &currentDay, &currentHour, &minute, nullptr);

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

  Serial.println("\n=== 10 READINGS PROCESSED ===");
  Serial.printf("Moisture: %.1f%% | Temp: %.1f°C | pH: %.1f | Fertility: %.1f%%\n", 
                avgMoisture, avgTemp, avgPH, avgFertility);

  saveBatchToSD();
  sendAveragedToThingSpeak();
}

void saveBatchToSD() {
  if (!sdOK) return;

  char filename[25];
  sprintf(filename, "/SoilData_%04d_%02d.csv", local_year, local_month);

  File f = SD.open(filename, FILE_APPEND);
  if (f) {
    if (f.size() == 0) {
      f.println("DateTime,Moisture(%),Temp(°C),EC(uS/cm),pH,N(mg/kg),P(mg/kg),K(mg/kg),Fertility(%),Lat,Lon,Alt");
    }

    for(int i = 0; i < bufferIndex; i++) {
      f.printf("%.1f,%.1f,%.0f,%.1f,%.0f,%.0f,%.0f,%.1f,", 
               buffer[i].moisture, buffer[i].temperature, buffer[i].ec, buffer[i].ph,
               buffer[i].nitrogen, buffer[i].phosphorus, buffer[i].potassium, buffer[i].fertilityScore);
      
    }

    if (hasGPS) {
      f.printf("%.6f,%.6f,%.1f\n", lat, lon, alt);
      f.printf("%04d-%02d-%02d %02d:%02d,", local_year, local_month, local_day, local_hour, minute);
    } else {
      f.println("noGPS,noGPS,noGPS");
    }

    f.close();
    Serial.println("✅ Batch saved to SD card");
  }
}

void sendAveragedToThingSpeak() {
  if (!client.connect(thingspeakHost, 80)) return;

  String url = "/update?api_key=" + apiKey +
               "&field1=" + String(avgN) +
               "&field2=" + String(avgP) +
               "&field3=" + String(avgK) +
               "&field4=" + String(avgPH,1) +
               "&field5=" + String(avgMoisture,1) +
               "&field6=" + String(avgEC) +
               "&field7=" + String(avgFertility,1) +
               "&field8=" + String(alt);

  if (lat != 0 && lon != 0) {
    url += "&lat=" + String(lat,6) + "&long=" + String(lon,6);
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\nHost: " + thingspeakHost + "\r\nConnection: close\r\n\r\n");
  delay(2000);
  client.stop();

  Serial.println("✅ Averaged data sent to ThingSpeak");
  sendSMSAlert();
}

void sendSMSAlert() {
  String msg = "Soil Data Updated!\n";
  msg += "Fertility: " + String(avgFertility,1) + "%\n";
  msg += "Moisture: " + String(avgMoisture,1) + "% | pH: " + String(avgPH,1) + "\n";
  msg += "View: https://thingspeak.com/channels/" + String(CHANNEL_ID);

  if (modem.sendSMS(contactPhone, msg)) {
    Serial.println("✅ SMS Sent Successfully!");
  } else {
    Serial.println("❌ SMS failed");
  }
}

void checkIncomingSMS() {
  while (SerialAT.available()) {
    String line = SerialAT.readStringUntil('\n');
    line.trim();

    if (line.startsWith("+CMT:")) {
      message = SerialAT.readStringUntil('\n');
      message.trim();
      Serial.println("New SMS: " + message);

      message.toUpperCase();

      if (message.indexOf("INFOSET") != -1) {
        changeConfig = !changeConfig;
        setConfig();
        changeConfig = !changeConfig;
      } 
      else if (message.indexOf("GPS") != -1) {
        reply = (hasGPSFix && lat != 0) ? "Lat: " + String(lat,6) + "\nLon: " + String(lon,6) : "No GPS fix";
      }
      else {
        reply = "Commands: INFOSET, GPS";
      }

      modem.sendSMS(contactPhone, reply);
    }
  }
}

// ================== EEPROM SETTINGS ==================
void loadConfig() {
  EEPROM.get(CONFIG_ADDR, config);
  if (config.version == 1 && config.deviceName[0] == '\0') {
    strcpy(config.deviceName, "THE FARMERS GROUP");
    strcpy(config.language, "ENGLISH");
    strcpy(config.contactPhone, "+256774291483");
    config.version = 1;
    setConfig();
  }
  deviceName   = config.deviceName;
  language     = config.language;
  contactPhone = config.contactPhone;
}

void setConfig() {
  if (!changeConfig) {
    EEPROM.put(CONFIG_ADDR, config);
    EEPROM.commit();
    deviceName   = config.deviceName;
    language     = config.language;
    contactPhone = config.contactPhone;
  } else {
    //set the new values from the sms client
    reply = "Type planely as below:\nNAME\nLANGUAGE\nCONTACT\n(in that order with each in own line, the contact format is +256xxxxxxxxx)";

    while (SerialAT.available()) {
      String tel;
      String name;
      String lang;
      String line = SerialAT.readStringUntil('\n');
      line.trim();

      if (line.startsWith("+CMT:")) {
        // Read the 3 lines of the message
        if (SerialAT.available()) {
          name = SerialAT.readStringUntil('\n');
          name.trim();
        }
        
        if (SerialAT.available()) {
          lang = SerialAT.readStringUntil('\n');
          lang.trim();
        }
        
        if (SerialAT.available()) {
          tel = SerialAT.readStringUntil('\n');
          tel.trim();
        }

        name.toUpperCase();
        lang.toUpperCase();

        // Debug output
        Serial.println("Name: " + name);
        Serial.println("Phone: " + tel);
        Serial.println("Value: " + lang);

        //validation
        if (name.length() > 0 && tel.startsWith("+256")) {
          Serial.println("✅ Valid message received");
          String reply = "Received: " + name + "\nNumber: " + tel + "\nValue: " + lang;

          //Acknowledge and save
          const char* contact = tel.c_str();
          const char* device = name.c_str();
          const char* lan_g = lang.c_str();
          strcpy(config.deviceName, device);
          strcpy(config.language, lan_g);
          strcpy(config.contactPhone, contact);
          config.version = config.version + 1;
          EEPROM.put(CONFIG_ADDR, config);
          EEPROM.commit();

          modem.sendSMS(contactPhone, reply);  
        } else {
          Serial.println("❌ Invalid message format");
          modem.sendSMS(contactPhone, "Invalid format. Use:\nName\nLanguage\n+256xxxxxxxxx");
        }
      }
    }
  }
}