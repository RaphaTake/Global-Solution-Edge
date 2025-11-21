#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// CONFIGURAÇÕES WIFI
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

// THINGSPEAK
    #define THINGSPEAK_API_KEY "158O1MDXCF4AAI63"
#define THINGSPEAK_URL "http://api.thingspeak.com/update"

// PINOS
#define PINO_DHT 4
#define PINO_LDR 34
#define PINO_LED_WHITE 12
#define PINO_LED_BLUE 13
#define PINO_BUTTON 14

// OBJETOS
DHT dht(PINO_DHT, DHT22);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// VARIÁVEIS
bool modoEstudo = false;
bool contagemAtiva = false;
bool waitingForTheme = false;

unsigned long inicioEstudo = 0;
unsigned long tempoEstudo = 0;

unsigned long ultimoLCD = 0;

String temaEstudo = "";

int statusAmbienteFinal = -1; 

unsigned long ultimoClique = 0;
const unsigned long DEBOUNCE_MS = 400;

// URL Encode
String urlencode(String s) {
  String out = "";
  for (int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (isalnum(c)) out += c;
    else if (c == ' ') out += '+';
    else {
      char hex[5];
      sprintf(hex, "%%%02X", c);
      out += hex;
    }
  }
  return out;
}

// Envio ao ThingSpeak
int sendThingSpeak(float temp, float hum, int ldrValue, int modoVal, unsigned long tempoSeg, int ambiente, String tema) {

  if (WiFi.status() != WL_CONNECTED) return -1;

  HTTPClient http;
  http.begin(THINGSPEAK_URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "api_key=" + String(THINGSPEAK_API_KEY);
  postData += "&field1=" + String(temp, 2);
  postData += "&field2=" + String(hum, 2);
  postData += "&field3=" + String(ldrValue);
  postData += "&field4=" + String(modoVal);
  postData += "&field5=" + String(tempoSeg);
  postData += "&field6=" + String(ambiente);
  postData += "&field7=" + urlencode(tema);

  int code = http.POST(postData);

  Serial.print("POST -> field6=");
  Serial.print(ambiente);
  Serial.print(" | HTTP ");
  Serial.println(code);

  http.end();
  return code;
}

// FINALIZA ESTUDO
void finalizarEstudo() {
    contagemAtiva = false;
    modoEstudo = false;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int ldr = analogRead(PINO_LDR);

    bool tempOK = (temp >= 20 && temp <= 26);
    bool humOK = (hum >= 40 && hum <= 60);
    bool luzOK = (ldr >= 1500 && ldr <= 3000);

    statusAmbienteFinal = (tempOK && humOK && luzOK) ? 1 : 0;

    lcd.clear();
    lcd.print("Processando...");
    lcd.setCursor(0,1);
    lcd.print("aguarde envio");

    delay(2000);

    // Envio final para ThingSpeak
    sendThingSpeak(temp, hum, ldr, 0, tempoEstudo/1000, statusAmbienteFinal, temaEstudo);

    lcd.clear();
    lcd.print("Ambiente:");
    lcd.print(statusAmbienteFinal == 1 ? " BOM" : " RUIM");
    delay(2500);

    lcd.clear();
    lcd.print("Pressione botao");
    lcd.setCursor(0, 1); 
    lcd.print("p/ novo estudo");

    Serial.println("Estudo finalizado e enviado para ThingSpeak");
}

// SETUP
void setup() {
  Serial.begin(115200);

  pinMode(PINO_LED_WHITE, OUTPUT);
  pinMode(PINO_LED_BLUE, OUTPUT);
  pinMode(PINO_BUTTON, INPUT_PULLUP);

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Conectando WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    lcd.print(".");
  }

  lcd.clear();
  lcd.print("WiFi conectado");
  delay(1000);

  lcd.clear();
  lcd.print("Pressione botao");
  lcd.setCursor(0, 1);
  lcd.print("p/ iniciar estudo");
}

// LOOP
void loop() {
  // BOTÃO COM DEBOUNCE
  bool botao = (digitalRead(PINO_BUTTON) == LOW);

  if (botao && millis() - ultimoClique > DEBOUNCE_MS) {
    ultimoClique = millis();

    if (!contagemAtiva && !waitingForTheme) {
      waitingForTheme = true;
      temaEstudo = "";

      lcd.clear();
      lcd.print("Digite o tema");
      lcd.setCursor(0, 1);
      lcd.print("no Serial...");

      Serial.println("Digite o tema do estudo:");
      return;
    }

    if (contagemAtiva) {
      finalizarEstudo();
      return;
    }
  }

  // RECEBE TEMA VIA SERIAL
  if (waitingForTheme && Serial.available()) {
    temaEstudo = Serial.readStringUntil('\n');
    temaEstudo.trim();
    waitingForTheme = false;

    modoEstudo = true;
    contagemAtiva = true;
    statusAmbienteFinal = -1;

    inicioEstudo = millis();
    tempoEstudo = 0;

    lcd.clear();
    lcd.print("Estudo iniciado");
    lcd.setCursor(0, 1);
    lcd.print("Tema:");
    lcd.print(temaEstudo.substring(0, 10));

    Serial.print("Tema definido: ");
    Serial.println(temaEstudo);

    delay(1500);
    lcd.clear();
  }

  // ATUALIZA TEMPO
  if (contagemAtiva)
    tempoEstudo = millis() - inicioEstudo;

  unsigned long segundos = (tempoEstudo / 1000) % 60;
  unsigned long minutos = (tempoEstudo / 1000) / 60;

  // LEITURA DOS SENSORES
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int ldr = analogRead(PINO_LDR);

  // LEDs
  digitalWrite(PINO_LED_BLUE, modoEstudo ? HIGH : LOW);
  digitalWrite(PINO_LED_WHITE, (!modoEstudo && ldr < 2000) ? HIGH : LOW);

  // LCD atualização do tempo
  if (millis() - ultimoLCD >= 500 && contagemAtiva) {
      ultimoLCD = millis();

      lcd.setCursor(0, 0);
      lcd.print("Estudo: ON       ");  

      lcd.setCursor(0, 1);
      lcd.print("Tempo: ");
      lcd.print(minutos);
      lcd.print("m ");
      if (segundos < 10) lcd.print("0");  
      lcd.print(segundos);
      lcd.print("s   ");
  }

  delay(10);
}
