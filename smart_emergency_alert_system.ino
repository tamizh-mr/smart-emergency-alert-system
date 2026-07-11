#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SafetyML_Distress_Detection_inferencing.h>

// ================= WIFI =================
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ================= TELEGRAM =================
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"

String USER_CHAT_ID = "YOUR_USER_CHAT_ID";
String POLICE_CHAT_ID = "YOUR_POLICE_GROUP_CHAT_ID";
String FIRE_CHAT_ID = "YOUR_FIRE_GROUP_CHAT_ID";

// ================= LOCATION =================
String locationLink = "YOUR_GOOGLE_MAPS_LOCATION_LINK";

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ================= PINS =================
#define MQ2_PIN 34
#define MQ135_PIN 35

#define GREEN_LED 25
#define RED_LED 26

#define RELAY_MQ2 27
#define RELAY_MQ135 14

#define FLAME_PIN 33
#define BUZZER_PIN 32

#define MIC_PIN 36

// ================= ML SETTINGS =================
#define SAMPLE_RATE 16000
#define SAMPLE_COUNT EI_CLASSIFIER_RAW_SAMPLE_COUNT

static int16_t audio_buffer[SAMPLE_COUNT];

// ================= THRESHOLDS =================
int gasThreshold = 2000;
int airThreshold = 2500;
int micThreshold = 2000;

// ================= STATES =================
bool gasDetected = false;
bool airDetected = false;
bool flameDetected = false;

bool gasAlertSent = false;
bool airAlertSent = false;
bool flameAlertSent = false;

// ================= VOICE STATES =================
bool voiceAlert1Sent = false;
bool voiceAlert2Sent = false;
bool waitingResponse = false;

unsigned long firstVoiceTime = 0;
unsigned long voiceAlertTime = 0;

const unsigned long ESCALATION_TIME = 60000;
const unsigned long SECOND_ALERT_WINDOW = 60000;

int previousMicValue = 0;

// ===== ALERT CONTROL =====
unsigned long lastAlertTime = 0;
String lastAlertType = "";
const unsigned long ALERT_COOLDOWN = 60000;
String currentHazard = "";

// ===== BUZZER TIMER =====
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
const unsigned long BUZZER_DURATION = 10000;

// ================= EDGE IMPULSE FUNCTION =================
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
  for (size_t i = 0; i < length; i++) {
    out_ptr[i] = audio_buffer[offset + i];
  }
  return 0;
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(RELAY_MQ2, OUTPUT);
  pinMode(RELAY_MQ135, OUTPUT);

  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(RELAY_MQ2, LOW);
  digitalWrite(RELAY_MQ135, HIGH);

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  client.setInsecure();

  bot.sendMessage(USER_CHAT_ID,
  "✅ Smart AI Safety System Online\nMonitoring Gas, Air, Flame & Voice\nWiFi Connected","");
}

// ================= TELEGRAM HANDLER =================
void handleMessages(int num) {

  for (int i = 0; i < num; i++) {

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    text.trim();

    if (chat_id != USER_CHAT_ID) continue;

    if (text == "/safe") {

      waitingResponse = false;

      bot.sendMessage(USER_CHAT_ID,
      "✅ System marked SAFE\nNo escalation will occur","");
    }

    else if (text == "/danger") {

      waitingResponse = false;

      String alert =
      "🚨 EMERGENCY ALERT\n"
      "Hazard: " + currentHazard + "\n\n"
      "Location:\n" + locationLink;

      bot.sendMessage(POLICE_CHAT_ID, alert, "");
      bot.sendMessage(FIRE_CHAT_ID, alert, "");
    }
  }
}

// ================= VOICE DETECTION =================
void checkVoice() {

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int sample = analogRead(MIC_PIN);
    audio_buffer[i] = sample - 2048;
    delayMicroseconds(62);
  }

  signal_t signal;
  signal.total_length = SAMPLE_COUNT;
  signal.get_data = &raw_feature_get_data;

  ei_impulse_result_t result = {0};

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  float screamScore = 0;

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {

    if (strcmp(result.classification[ix].label, "scream") == 0) {
      screamScore = result.classification[ix].value;
    }
  }

  int micValue = screamScore * 4095;

  unsigned long currentMillis = millis();

  if (micValue > 3500 && !voiceAlert1Sent) {

    voiceAlert1Sent = true;
    firstVoiceTime = currentMillis;
    currentHazard = "Distress Call";

    if (lastAlertType != "voice" || (currentMillis - lastAlertTime > ALERT_COOLDOWN)) {

      bot.sendMessage(USER_CHAT_ID,
      "🎤 VOICE ALERT\nSomeone may be requesting help\nReply /safe or /danger","");

      lastAlertType = "voice";
      lastAlertTime = currentMillis;
    }
  }

  if (micValue > 3500 &&
      voiceAlert1Sent &&
      !voiceAlert2Sent &&
      (currentMillis - firstVoiceTime <= SECOND_ALERT_WINDOW)) {

      voiceAlert2Sent = true;
      waitingResponse = true;
      voiceAlertTime = currentMillis;
  }

  if (waitingResponse && (currentMillis - voiceAlertTime > ESCALATION_TIME)) {

    waitingResponse = false;

    String alert =
    "🚨 NO RESPONSE\nHazard: Distress Call\n\nLocation:\n" + locationLink;

    bot.sendMessage(POLICE_CHAT_ID, alert, "");
    bot.sendMessage(FIRE_CHAT_ID, alert, "");
  }
}

// ================= LOOP =================
void loop() {

  int gasValue = analogRead(MQ2_PIN);
  int airValue = analogRead(MQ135_PIN);
  int flameValue = digitalRead(FLAME_PIN);

  unsigned long now = millis();

  // ===== GAS DETECTION =====
  if (gasValue > gasThreshold && !gasAlertSent) {

    gasAlertSent = true;
    gasDetected = true;
    currentHazard = "Gas Leak";

    digitalWrite(RELAY_MQ2, HIGH);   // CUT MAIN POWER
    digitalWrite(RED_LED, HIGH);     // RED LED ON
    digitalWrite(GREEN_LED, LOW);    // GREEN LED OFF

    if (lastAlertType != "gas" || (now - lastAlertTime > ALERT_COOLDOWN)) {

      String msg =
      "⚠ GAS LEAK DETECTED\nValue: " + String(gasValue) +
      "\n\nLocation:\n" + locationLink +
      "\n\nReply /safe or /danger";

      bot.sendMessage(USER_CHAT_ID, msg, "");

      lastAlertType = "gas";
      lastAlertTime = now;
    }
  }

  if (gasValue < gasThreshold && gasDetected) {

    gasDetected = false;
    gasAlertSent = false;

    digitalWrite(RELAY_MQ2, LOW);   // POWER RESTORED
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }

  // ===== AIR POLLUTION =====
  if (airValue > airThreshold && !airAlertSent) {

    airAlertSent = true;
    currentHazard = "Air Pollution";

    if (lastAlertType != "air" || (now - lastAlertTime > ALERT_COOLDOWN)) {

      String msg =
      "⚠ AIR POLLUTION ALERT\nValue: " + String(airValue) +
      "\n\nLocation:\n" + locationLink +
      "\n\nReply /safe or /danger";

      bot.sendMessage(USER_CHAT_ID, msg, "");

      lastAlertType = "air";
      lastAlertTime = now;
    }
  }

  // ===== FIRE =====
  if (flameValue == LOW && !flameAlertSent) {

    flameAlertSent = true;
    currentHazard = "Fire";

    if (lastAlertType != "fire" || (now - lastAlertTime > ALERT_COOLDOWN)) {

      bot.sendMessage(USER_CHAT_ID,
      "🔥 FIRE DETECTED\nLocation:\n" + locationLink +
      "\n\nReply /safe or /danger","");

      lastAlertType = "fire";
      lastAlertTime = now;
    }

    digitalWrite(BUZZER_PIN, HIGH);
    buzzerStartTime = now;
    buzzerActive = true;
  }

  if (buzzerActive && (now - buzzerStartTime >= BUZZER_DURATION)) {

    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
  }

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {

    handleMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  checkVoice();

  delay(2000);
}
