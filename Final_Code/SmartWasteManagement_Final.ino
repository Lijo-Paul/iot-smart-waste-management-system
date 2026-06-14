#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

/* ================= WIFI ================= */

#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

/* ================= FIREBASE ================= */

#define DATABASE_URL "YOUR_DATABASE_URL"
#define DATABASE_SECRET "YOUR_DATABASE_SECRET"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/* ================= LCD ================= */

LiquidCrystal_I2C lcd(0x27, 16, 2);

/* ================= PINS ================= */

#define TRIG_PIN 5
#define ECHO_PIN 18
#define IR_PIN 19
#define SERVO_PIN 23

/* ================= SERVO ================= */

Servo lidServo;

const int SERVO_OPEN = 90;
const int SERVO_CLOSE = 0;

bool lidOpen = false;
unsigned long lidTimer = 0;

const unsigned long CLOSE_DELAY = 3000;

/* ================= BIN ================= */

const int BIN_DEPTH_CM = 40;
const int FULL_LEVEL_CM = 8;

/* ================= TIMERS ================= */

unsigned long firebaseTimer = 0;
unsigned long lcdTimer = 0;

/* ================= LCD CACHE ================= */

String lastLine1 = "";
String lastLine2 = "";

/* ===================================================== */

void lcdPrint(String l1, String l2)
{
  if (l1 == lastLine1 && l2 == lastLine2)
    return;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(l1);

  lcd.setCursor(0, 1);
  lcd.print(l2);

  lastLine1 = l1;
  lastLine2 = l2;
}

/* ===================================================== */

long getDistanceCM()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration =
      pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
    return -1;

  return duration * 0.034 / 2;
}

/* ===================================================== */

bool humanDetected()
{
  static unsigned long detectStart = 0;

  if (digitalRead(IR_PIN) == LOW)
  {
    if (detectStart == 0)
      detectStart = millis();

    if (millis() - detectStart > 250)
      return true;
  }
  else
  {
    detectStart = 0;
  }

  return false;
}

/* ===================================================== */

void smoothServo(int startPos, int endPos)
{
  if (startPos < endPos)
  {
    for (int p = startPos; p <= endPos; p++)
    {
      lidServo.write(p);
      delay(8);
    }
  }
  else
  {
    for (int p = startPos; p >= endPos; p--)
    {
      lidServo.write(p);
      delay(8);
    }
  }
}

/* ===================================================== */

void connectWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  lcdPrint("Connecting...", "WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");

  lcdPrint("WiFi Connected", "");
  delay(1500);
}

/* ===================================================== */

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(IR_PIN, INPUT);

  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  lidServo.setPeriodHertz(50);
  lidServo.attach(SERVO_PIN, 500, 2400);
  lidServo.write(SERVO_CLOSE);

  connectWiFi();

  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token =
      DATABASE_SECRET;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  lcdPrint("System Ready", "");
}

/* ===================================================== */

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  long distance = getDistanceCM();

  if (distance < 0)
  {
    lcdPrint("Sensor Error", "");
    delay(500);
    return;
  }

  int fillPercent =
      map(distance,
          BIN_DEPTH_CM,
          FULL_LEVEL_CM,
          0,
          100);

  fillPercent =
      constrain(fillPercent, 0, 100);

  bool fullBin =
      distance <= FULL_LEVEL_CM;

  /* ================= OPEN ================= */

  if (humanDetected() &&
      !lidOpen &&
      !fullBin)
  {
    smoothServo(
        SERVO_CLOSE,
        SERVO_OPEN);

    lidOpen = true;
    lidTimer = millis();

    lcdPrint("Welcome", "Bin Opening");
  }

  /* ================= CLOSE ================= */

  if (lidOpen &&
      millis() - lidTimer >
          CLOSE_DELAY)
  {
    smoothServo(
        SERVO_OPEN,
        SERVO_CLOSE);

    lidOpen = false;

    lcdPrint("Thank You", "Bin Closed");

    delay(1000);
  }

  /* ================= DISPLAY ================= */

  if (!lidOpen)
  {
    if (fullBin)
    {
      lcdPrint(
          "BIN FULL",
          "Clean Required");
    }
    else
    {
      lcdPrint(
          "Bin Level",
          String(fillPercent) + "%");
    }
  }

  /* ================= FIREBASE ================= */

  if (millis() - firebaseTimer >
      5000)
  {
    firebaseTimer = millis();

    Firebase.RTDB.setInt(
        &fbdo,
        "/bin/distance_cm",
        distance);

    Firebase.RTDB.setInt(
        &fbdo,
        "/bin/fill_percent",
        fillPercent);

    Firebase.RTDB.setString(
        &fbdo,
        "/bin/status",
        fullBin
            ? "FULL"
            : lidOpen
                  ? "OPEN"
                  : "IDLE");

    Serial.println("Firebase Updated");
  }

  delay(100);
}
