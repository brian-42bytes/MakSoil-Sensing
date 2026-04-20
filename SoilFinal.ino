#include <Arduino.h>

#define TINY_GSM_MODEM_SIM7000

// ================== CONFIG ==================
#define RE_DE_PIN   25
#define SENSOR_RX   21
#define SENSOR_TX   22
#define SENSOR_BAUD 4800

#define APN         "web.gprs.mtn.co.ug"

const char* thingspeakHost = "api.thingspeak.com";
String apiKey = "IVAWTICG5ZGIOUT2";        //Write API Key

const int TIME_ZONE_OFFSET = 3;   // UTC+3 EAT

#include <TinyGsmClient.h>

HardwareSerial modbus(1);
HardwareSerial SerialAT(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

uint8_t response[32];

bool state = 0;

void setup() {
  Serial.begin(115200);
  modbus.begin(SENSOR_BAUD, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
  
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);

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
  Serial.println("✅ Modem ready.");

  // Enable GPS antenna power
  modem.sendAT("+SGPIO=0,4,1,1");
  modem.waitResponse();

  // Enable GNSS
  modem.enableGPS();
  delay(5000);

  modem.waitForNetwork(30000L);
  modem.gprsConnect(APN, "", "");
}

void loop() {
  // Read Soil Sensor
  uint8_t cmd[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  digitalWrite(RE_DE_PIN, HIGH);
  modbus.write(cmd, 8);
  modbus.flush();
  digitalWrite(RE_DE_PIN, LOW);
  delay(250);

  if (modbus.available() < 19) {//Alarm to alert for this jam
    Serial.println("No sensor response");
    delay(5000);
    return;
  }

  modbus.readBytes(response, 32);

  if (!(response[0] == 0x01 && response[1] == 0x03 && response[2] == 0x0E)) {//Alarm to alert for this jam
    Serial.println("Invalid sensor response");
    delay(1000);
    return;
  }

  // Parse sensor values
  float moisture     = ((response[3] << 8) | response[4]) / 10.0;
  float temperature  = ((response[5] << 8) | response[6]) / 10.0;
  float ec           = (response[7] << 8) | response[8];
  float ph           = ((response[9] << 8) | response[10]) / 10.0;
  float nitrogen     = (response[11] << 8) | response[12];
  float phosphorus   = (response[13] << 8) | response[14];
  float potassium    = (response[15] << 8) | response[16];

  float fertilityScore = ((nitrogen/250.0) + (phosphorus/60.0) + (potassium/200.0)) / 3.0 * 100.0;

  // Get GPS
  float lat = 0, lon = 0, alt = 0, accuracy = 0;
  int year=0, month=0, day=0, hour=0, minute=0, second=0;
  bool hasFix = modem.getGPS(&lat, &lon, nullptr, &alt, nullptr, nullptr, &accuracy,
                             &year, &month, &day, &hour, &minute, &second);

  // Local time
  int local_hour = hour + TIME_ZONE_OFFSET;
  int local_day = day;
  int local_month = month;
  int local_year = year;
  if (local_hour >= 24) {
    local_hour -= 24;
    local_day++;
    if (local_day > 31) { local_day = 1; local_month++; if (local_month > 12) { local_month = 1; local_year++; } }
  }

  // ================== PRINT READINGS (as requested) ==================
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
  } else {
    Serial.println("GPS: No fix yet");
  }

  // ================== SEND TO THINGSPEAK ==================
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

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + thingspeakHost + "\r\n" +
                 "Connection: close\r\n\r\n");
    
    delay(3000);
  } else {//Alarm to alert for this jam
    Serial.println("Failed to connect to ThingSpeak");
  }
  client.stop();


  delay(20000);
}