#include <TinyGPS++.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "test"
#define WIFI_PASSWORD "12345678"

#define API_KEY ""
#define DATABASE_URL ""

#define LED_BUILTIN 2

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK =  false;

#define TRIG_PIN 18
#define ECHO_PIN 19
float distance;
float latitude;     // Storing the Latitude
float longitude;    // Storing the Longitude
float velocity;     // Variable to store the velocity
float sats;         // Variable to store the number of satellites response
String bearing;     // Variable to store orientation or direction of GPS

TinyGPSPlus gps;
#define RXD2 16
#define TXD2 17
HardwareSerial mygps(1);

void setup()
{
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  mygps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print(".");
    delay(300);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  digitalWrite(LED_BUILTIN, HIGH);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signUp is ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Set the database URL again
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  while (mygps.available() > 0)
  {
    // sketch displays information every time a new sentence is correctly encoded.
    if (gps.encode(mygps.read()))
      displayInfo();
  }
}

void checkGPS()
{
  if (gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
  }
}

void displayInfo()
{
  if (gps.location.isValid())
  {
    sats = gps.satellites.value();       // get the number of satellites
    latitude = (gps.location.lat());     // Storing the Lat. and Lon.
    longitude = (gps.location.lng());
    velocity = gps.speed.kmph();         // get velocity
    bearing = TinyGPSPlus::cardinal(gps.course.value());     // get the direction


    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
    {
      sendDataPrevMillis = millis();

      Serial.print("LATITUDE:  ");
      Serial.println(latitude, 6);  // float to x decimal places
      Serial.print("LONGITUDE: ");
      Serial.println(longitude, 6);

      digitalWrite(TRIG_PIN, LOW);
      delayMicroseconds(2);
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG_PIN, LOW);

      unsigned long duration = pulseIn(ECHO_PIN, HIGH);
      // Speed of sound is 340 m/s or 0.034 cm/µs
      distance = duration * 0.034 / 2;
      Serial.print("DUSTBIN LEVEL: ");
      Serial.println(distance);

      String googleMapsLink = "https://www.google.com/maps?q=" + String(latitude, 6) + "," + String(longitude, 6);

      if (Firebase.RTDB.setFloat(&fbdo, "bin2/Sensor/LevelWater", distance))
      {
        Serial.println();
        Serial.print(distance);
        Serial.println("- successfully saved to:");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "bin2/gps/latitude", latitude))
      {
        Serial.println();
        Serial.print(latitude);
        Serial.println("- successfully saved to:");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "bin2/gps/longitude", longitude))
      {
        Serial.println();
        Serial.print(longitude);
        Serial.println("- successfully saved to:");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }

      if (Firebase.RTDB.setString(&fbdo, "bin2/googlemaps/link", googleMapsLink))
      {
        Serial.println();
        Serial.print(googleMapsLink);
        Serial.println("- successfully saved to:");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }

      else
      {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

    }
  }
}
