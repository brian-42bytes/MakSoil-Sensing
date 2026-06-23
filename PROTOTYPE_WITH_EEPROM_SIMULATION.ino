#include "EEPROM.h"
#include <Arduino.h>
#include "SD.h"
#include "SPI.h"

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
  "LANGI"
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

const char* apiHost = "cropisfm.onrender.com";
const char* apiPath = "/api/load_sample_csv";
String apiKey = "ayqDIURT.pU4QKrAObn8O5ZEgbonC9u9XKGPEcIFP";

// ================== OBJECTS ==================
HardwareSerial modbus(1);
HardwareSerial SerialAT(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

uint8_t response[32];
uint8_t cmd[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

const int TIME_ZONE_OFFSET = 3;
const int BUFFER_SIZE = 5;

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

  Serial.println("=== System Ready - Collecting 5 readings ===");
}
int count = 0;
// ================== LOOP ==================
void loop() {/*
  getGPS();
  if (hasGPSFix) {
    Serial.print(lat);
    Serial.print(", "); 
    Serial.println(lon);
    Serial.print(alt);
    Serial.println("m");
    Serial.print(local_hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(" ");
    Serial.print(local_day);
    Serial.print("-");
    Serial.print(local_month);
    Serial.print("-");
    Serial.println(local_year);
  } else {
    count++;
    Serial.println("noGPS,noGPS,noGPS");
    Serial.print(10*count);
    Serial.println("Seconds passed.");
  }*/
  takeOneReading();

  if (bufferIndex >= BUFFER_SIZE) {
    getGPS(); //get time and gps data
    
    hasGPS = hasGPSFix;

    processBatch();
    bufferIndex = 0;
  }

  checkIncomingSMS();   

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
  modem.waitResponse(6000);
  modem.enableGPS();
  delay(2000);

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

  Serial.println("\n=== 5 READINGS PROCESSED ===");
  Serial.printf("Moisture: %.1f%% | Temp: %.1f°C | pH: %.1f | Fertility: %.1f%%\n", 
                avgMoisture, avgTemp, avgPH, avgFertility);

  saveBatchToSD();
  if (sendToAPI()){
    Serial.println("DATA sentnt well To API!");
  }
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

bool sendToAPI() {
  if (!sdOK) {
    Serial.println("Check SD Card");
    return false;
  }

  char filename[30];
  sprintf(filename, "/soil_%04d%02d.csv", 2026, 5);   // Adjust date from GPS

  File csvFile = SD.open(filename, FILE_READ);
  if (!csvFile) {
    Serial.println("Can\'t open CSV file");
    return false;
  }

  Serial.println("Uploading to API...");

  if (client.connect(apiHost, 80)) {
    String boundary = "ESP32Boundary123456789abcdef";

    String head = "--" + boundary + "\r\n";
    head += "Content-Disposition: form-data; name=\"file\"; filename=\"" + String(filename) + "\"\r\n";
    head += "Content-Type: text/csv\r\n\r\n";

    String tail = "\r\n--" + boundary + "--\r\n";

    int contentLength = head.length() + csvFile.size() + tail.length();

    // Send request
    client.print("POST " + String(apiPath) + " HTTP/1.1\r\n");
    client.print("Host: " + String(apiHost) + "\r\n");
    client.print("Authorization: Bearer " + apiKey + "\r\n");   // Using your key
    client.print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
    client.print("Content-Length: " + String(contentLength) + "\r\n\r\n");

    client.print(head);

    // Send file
    while (csvFile.available()) {
      client.write(csvFile.read());
    }

    client.print(tail);
    client.flush();

    delay(4000); 

    // Read server response
    String response = "";
    while (client.available()) {
      response += (char)client.read();
    }

    Serial.println("API Response:");
    Serial.println(response.substring(0, 400));

    client.stop();
    csvFile.close();

    if (response.indexOf("200") != -1 || response.indexOf("success") != -1) {
      Serial.println("✅ CSV successfully uploaded to your API!");
      return true;
    } else {
      Serial.println("Upload fail");
      return false;
    }
  } else {
    Serial.println("Could not connect to API");
    csvFile.close();
    return false;
  }


  Serial.println("✅ Data sent!");
  sendSMSAlert();
}

void sendSMSAlert() {
  String msg = "Soil Data Updated!\n";
  msg += "Fertility: " + String(avgFertility,1) + "%\n";
  msg += "Moisture: " + String(avgMoisture,1) + "% | pH: " + String(avgPH,1) + "\n";
  msg += "View: https://cropisfm.onrender.com/api/load_sample_csv" + String(SENSOR_ID);

  if (modem.sendSMS(contactPhone, msg)) {
    Serial.println("✅ SMS Sent Successfully!");
  } else {
    Serial.println("SMS failed");
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
          Serial.println("Invalid message format");
          modem.sendSMS(contactPhone, "Invalid format. Use:\nName\nLanguage\n+256xxxxxxxxx");
        }
      }
    }
  }
}