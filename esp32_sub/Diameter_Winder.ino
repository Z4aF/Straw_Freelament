#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <max6675.h>
#include <PID_v1.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <esp_log.h>

// ── PIN ASSIGNMENTS ─────────────────────────────────────────────
const int thermoDO = 19;
const int thermoCS = 5;
const int thermoCLK = 18;

#define SSR_PIN 25
const int MOTOR_RPWM = 26;

// ── MANUAL CONTROLS ─────────────────────────────────────────────
#define PIN_ONOFF 15
#define PIN_BTN 2
#define PIN_ENC_CLK 32
#define PIN_ENC_DT 33

// ── NETWORK & AUTH ──────────────────────────────────────────────
const char* SSID = "yourID";
const char* PASS = "yourPassword";
const char* ADMIN_USER = "admin";
const char* ADMIN_PASS = "secret";

// ── GLOBAL OBJECTS ──────────────────────────────────────────────
unsigned long motorStartTime = 0;
bool motorStartTimerActive = false;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// PID
double currentTemp, setpoint, pidOutput;
PID myPID(&currentTemp, &pidOutput, &setpoint, 1.0, 0.5, 0.2, DIRECT);

// Motor
int targetRpm = 0;

// ── MANUAL STATE ────────────────────────────────────────────────
volatile int8_t encoderDelta = 0;
bool encoderMoved = false;
bool manualEnabled = false;
uint8_t adjustMode = 0;

// debounce
int lastOnoffReading = HIGH;
uint32_t lastOnoffDebounce = 0;
int lastBtnReading = HIGH;
uint32_t lastBtnDebounce = 0;
const uint32_t DEBOUNCE_MS = 50;

// timing
unsigned long lastTempRead = 0;
unsigned long lastPrint = 0;

// ── ENCODER ISR ────────────────────────────────────────────────
void IRAM_ATTR onEncoder() {
  bool clk = digitalRead(PIN_ENC_CLK);
  bool dt = digitalRead(PIN_ENC_DT);
  encoderDelta += (clk == dt) ? +1 : -1;
  encoderMoved = true;
}

// ── SETUP ───────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  esp_log_level_set("i2c.master", ESP_LOG_NONE);
  esp_log_level_set("i2c", ESP_LOG_NONE);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Set Value: RPM:");
  lcd.setCursor(0, 2);
  lcd.print("Current Temp:");

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  pinMode(SSR_PIN, OUTPUT);
  pinMode(MOTOR_RPWM, OUTPUT);
  pinMode(PIN_ONOFF, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_ENC_CLK, INPUT_PULLUP);
  pinMode(PIN_ENC_DT, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), onEncoder, CHANGE);

  setpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 255);

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  sendUniqueCodeToPHP();

  server.on("/data", HTTP_GET, handleData);
  server.on("/set", HTTP_POST, handleSet);
  server.on("/setMotor", HTTP_GET, handleSetMotor);
  server.on("/tune", HTTP_GET, handleGetTunings);
  server.on("/tune", HTTP_POST, handleSetTunings);

  server.on("/", []() {
    File f = SPIFFS.open("/index.html", "r");
    server.streamFile(f, "text/html");
    f.close();
  });

  server.begin();
}

// ── LOOP ────────────────────────────────────────────────────────
void loop() {
  server.handleClient();

  int onoff = digitalRead(PIN_ONOFF);

  if (onoff != lastOnoffReading) {
    lastOnoffDebounce = millis();
    lastOnoffReading = onoff;
  }

  if (millis() - lastOnoffDebounce > DEBOUNCE_MS) {
    manualEnabled = (onoff == LOW);
  }

  int btn = digitalRead(PIN_BTN);

  if (btn != lastBtnReading) {
    lastBtnDebounce = millis();
    lastBtnReading = btn;
  }

  if (millis() - lastBtnDebounce > DEBOUNCE_MS) {
    if (btn == LOW && manualEnabled) {
      adjustMode = 1 - adjustMode;
      while (digitalRead(PIN_BTN) == LOW) delay(5);
    }
  }

  if (manualEnabled && encoderMoved) {
    if (adjustMode == 0) {
      setpoint = constrain(setpoint + (encoderDelta * 5.0), 0, 210);
    } else {
      targetRpm = constrain(targetRpm + encoderDelta, 0, 80);
    }
    encoderDelta = 0;
    encoderMoved = false;
  }

  if (millis() - lastTempRead >= 500) {
    currentTemp = thermocouple.readCelsius();
    lastTempRead = millis();
  }

  if (!isnan(currentTemp)) {
    myPID.Compute();
    int out = (currentTemp < setpoint) ? pidOutput : 0;
    analogWrite(SSR_PIN, out);
  }

  int pwmVal = 0;
  if (motorStartTimerActive && millis() - motorStartTime >= 360000) {
    pwmVal = map(targetRpm, 0, 80, 0, 255);
  }
  analogWrite(MOTOR_RPWM, pwmVal);
}

// ── FUNCTIONS ───────────────────────────────────────────────────
void handleData() {
  char buf[128];
  snprintf(buf, sizeof(buf),
    "{\"temp\":%.1f,\"set\":%.1f,\"output\":%.1f,\"rpm\":%d}",
    currentTemp, setpoint, pidOutput, targetRpm);

  server.send(200, "application/json", buf);
}

void handleSet() {
  if (server.hasArg("set")) {
    setpoint = server.arg("set").toFloat();
  }
  handleData();
}

void handleSetMotor() {
  if (server.hasArg("rpm")) {
    targetRpm = constrain(server.arg("rpm").toInt(), 0, 80);
  }
  server.send(200, "application/json",
    String("{\"rpm\":") + targetRpm + "}");
}

void handleGetTunings() {
  server.send(200, "application/json",
    "{\"kp\":1.0,\"ki\":0.5,\"kd\":0.2}");
}

void handleSetTunings() {
  double Kp = server.arg("kp").toFloat();
  double Ki = server.arg("ki").toFloat();
  double Kd = server.arg("kd").toFloat();
  myPID.SetTunings(Kp, Ki, Kd);
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void sendUniqueCodeToPHP() {
  HTTPClient http;
  http.begin("http://172.20.10.8/capstone/send_code.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  http.POST("unique_code=ESP32CAPSTONE");
  http.end();
}