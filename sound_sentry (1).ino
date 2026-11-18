/*
  Sound Sentry – Classroom Noise Monitor
  Arduino Uno + MAX4466 + LEDs + Buzzer
  Red=D2  Green=D3  Yellow=D4  Buzzer=D8  Mic=A0
  Serial 9600: time_s,peak,level,status
*/

const int MIC_PIN    = A0;
const int LED_RED    = 2;
const int LED_GREEN  = 3;
const int LED_YELLOW = 4;
const int BUZZER     = 8;

// ===== ADJUST IF NEEDED =====
const int QUIET_MAX   = 45;   // below → Green (Quiet)
const int CAUTION_MAX = 110;  // below → Yellow, above → Red
// =============================

const unsigned long WINDOW_MS = 50;
const unsigned long REPORT_MS = 200;
const unsigned long HOLD_MS   = 1500;

unsigned long lastReport = 0;
unsigned long warningUntil = 0;
int currentLevel = 0; // 0 Quiet, 1 Caution, 2 Warning

void setLEDs(int level) {
  digitalWrite(LED_GREEN,  level == 0 ? HIGH : LOW);
  digitalWrite(LED_YELLOW, level == 1 ? HIGH : LOW);
  digitalWrite(LED_RED,    level == 2 ? HIGH : LOW);
}

void startupSequence() {
  setLEDs(0); delay(300);
  setLEDs(1); delay(300);
  setLEDs(2); delay(300);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER, HIGH); delay(80);
    digitalWrite(BUZZER, LOW);  delay(80);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);
  delay(200);
  startupSequence();
  Serial.println("time_s,peak,level,status");
}

void loop() {
  unsigned long start = millis();
  int hi = 0, lo = 1023;
  while (millis() - start < WINDOW_MS) {
    int v = analogRead(MIC_PIN);
    if (v > hi) hi = v;
    if (v < lo) lo = v;
  }
  int peak = hi - lo;
  if (peak < 8) peak = 0;

  int level = 0;
  if (peak > CAUTION_MAX) level = 2;
  else if (peak > QUIET_MAX) level = 1;

  if (level == 2) warningUntil = millis() + HOLD_MS;
  if (millis() < warningUntil) level = 2;

  currentLevel = level;
  setLEDs(currentLevel);

  if (currentLevel == 2) {
    digitalWrite(BUZZER, (millis() / 120) % 2);  // loud repeated beeps
  } else {
    digitalWrite(BUZZER, LOW);
  }

  if (millis() - lastReport >= REPORT_MS) {
    lastReport = millis();
    int mapped = constrain(map(peak, 0, 400, 0, 100), 0, 100);
    const char* status = level == 0 ? "Quiet" :
                         level == 1 ? "Caution" : "Warning";
    Serial.print(millis() / 1000.0, 2); Serial.print(',');
    Serial.print(peak);                 Serial.print(',');
    Serial.print(mapped);               Serial.print(',');
    Serial.println(status);
  }
}
