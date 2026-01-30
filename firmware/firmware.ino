#include <Arduino.h>


#include <WiFi.h>
#include <time.h>

const char* ssid = "EE-86SJFT";
const char* password = "yq7PRCx7dgni3kW6";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

const uint8_t LED_PIN = 4;
const uint32_t PWM_FREQ = 20000;
const uint8_t PWM_RES_BITS = 10;
const uint32_t PWM_MAX_DUTY = pow(2, PWM_RES_BITS) - 1;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(4, OUTPUT);
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Maximum PWM duty: ");
  Serial.println(PWM_MAX_DUTY);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0/2", 1);
  tzset();

  printLocalTime();

  // ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RES_BITS);
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}

int oldDuty = 0;
void loop() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  // Serial.println(&timeinfo);

  int secondsSinceMidnight = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
  int t = constrain(secondsSinceMidnight - (21 * 3600 + 54 * 60), 0, 1800);
  float normalisedPhase = t / 1800.0;
  float gamma = 2.2;
  int duty = pow(normalisedPhase, gamma) * PWM_MAX_DUTY;
  // int duty = map(t, 0, 1800, 0, 8191);

  // ledcWrite(4, duty);
  ledcFade(4, oldDuty, duty, 1000);
  oldDuty = duty;

  Serial.print(secondsSinceMidnight);
  Serial.print("    ");
  Serial.print(t);
  Serial.print("    ");
  Serial.print(normalisedPhase);
  Serial.print("    ");
  Serial.println(duty);



  // printLocalTime();
  delay(1000);  // wait for a second
}
