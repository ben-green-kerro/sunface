#include <Arduino.h>


#include <WiFi.h>
#include <time.h>

const char* ssid = "EE-86SJFT";
const char* password = "yq7PRCx7dgni3kW6";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

const uint8_t LED_PIN = 4;
const uint32_t PWM_FREQ = 5000;
const uint8_t PWM_RES_BITS = 12;
const uint32_t PWM_MAX_DUTY = pow(2, PWM_RES_BITS) - 1;

int duty = 0;
int oldDuty = 0;

int startTime = (6 * 3600 + 45 * 60);
int peakTime = (7 * 3600 + 15 * 60);
int endTime = (8 * 3600 + 15 * 60);
int offTime = (8 * 3600 + 30 * 60);

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

int getSecondsSinceMidnight() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  int secondsSinceMidnight = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
  Serial.print(secondsSinceMidnight);
  Serial.print("    ");
  return secondsSinceMidnight;
}

float getGammaCorrectedBrightness(float normalisedBrightness, float gamma) {
  float gammaCorrectedBrightness = pow(normalisedBrightness, gamma);
  Serial.print(normalisedBrightness, 8);
  Serial.print("    ");

  Serial.print(gammaCorrectedBrightness, 8);
  Serial.print("    ");
  return gammaCorrectedBrightness;
}

int getDuty(float gammaCorrectedBrightness) {
  int duty = gammaCorrectedBrightness * PWM_MAX_DUTY;
  Serial.print(duty);
  Serial.print("    ");
  return duty;
}

int bounce() {
  for (float i = 0.0; i <= 1.0; i = i + 0.01) {
    ledcWrite(4, getDuty(getGammaCorrectedBrightness(i, 2.2)));
    delay(5);
  }
  for (float i = 1.0; i >= 0.0; i = i - 0.01) {
    ledcWrite(4, getDuty(getGammaCorrectedBrightness(i, 2.2)));
    delay(5);
  }
}

int pulse() {
  int t = millis();
  float phase = (2.0f * 3.14159 * (t % (unsigned long)2000.0)) / 2000.0;
  Serial.print(phase);
  Serial.print("    ");
  float brightness = 0.5f * (sinf(phase) + 1.0f);
  ledcWrite(4, getDuty(getGammaCorrectedBrightness(brightness / 2, 2.2)));
  Serial.print(brightness);
  Serial.print("    ");
  Serial.println();
}

void daily() {
  int secondsSinceMidnight = getSecondsSinceMidnight();
  float normalisedBrightness = 0.0;
  if (secondsSinceMidnight < startTime) {
    Serial.print("Phase 1    ");
    normalisedBrightness = 0.0;
  } else if (secondsSinceMidnight < peakTime) {
    Serial.print("Phase 2    ");
    int fadeTime = peakTime - startTime;
    int secondsSinceStart = secondsSinceMidnight - startTime;
    Serial.print(secondsSinceStart);
    Serial.print("    ");
    normalisedBrightness = secondsSinceStart / float(fadeTime);
  } else if (secondsSinceMidnight < endTime) {
    Serial.print("Phase 3    ");
    normalisedBrightness = 1.0;
  } else if (secondsSinceMidnight < offTime) {
    Serial.print("Phase 4    ");
    int fadeTime = offTime - endTime;
    int secondsSinceEnd = secondsSinceMidnight - endTime;
    normalisedBrightness = 1.0 - (secondsSinceEnd / float(fadeTime));
  } else {
    Serial.print("Phase 5    ");
    normalisedBrightness = 0.0;
  }

  float gammaCorrectedBrightness = getGammaCorrectedBrightness(normalisedBrightness, 2.2);
  int duty = getDuty(gammaCorrectedBrightness);
  ledcFade(4, oldDuty, duty, 1000);
  oldDuty = duty;
  delay(1000);  // wait for a second
  Serial.println();
}

void loop() {
  daily();
  // bounce();
  // pulse();
}
