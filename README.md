# Global Solution Edge Computing

## 👥Integrantes
- **Felipe de Andrade Godoi** - RM: 566209  
- **Raphael Naome Taketa** - RM: 562321  
- **Victor Ribeiro Guimarães Lira** - RM: 566213  

---

## 📋Descrição do Problema

Estamos na era da **requalificação contínua**, onde aprender e reaprender se tornou o **novo superpoder**.  
Muitos trabalhadores participam de cursos, treinamentos e estudos para **atualizar habilidades**, mas não percebem que **o ambiente em que estudam influencia diretamente a eficácia do aprendizado**.  

Fatores como **temperatura, umidade, iluminação e gestão do tempo** podem impactar a concentração, retenção de informações e produtividade durante os estudos.  

O desafio deste projeto é criar uma solução que **monitore o ambiente de estudo do trabalhador**, fornecendo feedback em tempo real e permitindo ajustes imediatos para **maximizar o aprendizado e tornar a requalificação contínua mais eficiente**.

---

## 💡Solução

Desenvolvemos um **sistema de monitoramento de ambiente de estudo para trabalhadores**, que inclui:

- **ESP32** para leitura de sensores e envio de dados;
- **Sensor DHT22** para medir temperatura e umidade;
- **Sensor LDR** para medir luminosidade;
- **LCD I2C** 16x2 para exibição local de informações e feedback;
- **Botão físico** para iniciar e finalizar o período de estudo;
- **ThingSpeak** para armazenamento dos dados na nuvem;
- **Dashboard HTML/JavaScript** para visualização em tempo real de temperatura, umidade, luminosidade, tempo de estudo e status do ambiente.

Essa solução ajuda os trabalhadores a **monitorar condições ideais para estudo**, aumentando a produtividade, o foco e a eficiência na aquisição de novas habilidades.

---

## 📝Instruções de Uso

1. **Montagem do Hardware:**
   - Conecte o **DHT22** ao pino 4 do ESP32;
   - Conecte o **LDR** ao pino 34;
   - Conecte LEDs e botão nos pinos 12, 13 e 14;
   - Conecte o **LCD I2C** ao barramento I2C (SDA/SCL).

2. **Configuração do Código (.ino):**
   - Abra o arquivo `.ino` no Arduino IDE;
   - Configure o **SSID e senha do Wi-Fi**;
   - Configure a **API Key do ThingSpeak**.

3. **Execução:**
   - Carregue o código no ESP32;
   - Pressione o botão para iniciar o estudo;
   - Digite o **tema do estudo** no Serial Monitor;
   - Observe o LCD durante o período de estudo;
   - Ao finalizar, o ESP32 envia o **status final do ambiente** e o **tempo total de estudo** para o ThingSpeak.

4. **Dashboard Web:**
   - Abra o arquivo `dashboard.html` em qualquer navegador;
   - O dashboard mostra gráficos de **temperatura, umidade, luminosidade e tempo de estudo**;
   - Exibe o **status do ambiente** e o **tema do estudo** em tempo real.

---

## Imagens do Projeto

# 💻Projeto no Wokwi
<img width="829" height="494" alt="image" src="https://github.com/user-attachments/assets/fd3abf48-12f9-4106-ba39-180042ec2ad6" />

# 📊Dashboard
<img width="1919" height="969" alt="image" src="https://github.com/user-attachments/assets/2a339ab4-6011-48b8-9aee-b7587366295b" />


---

## 🔗Link da Simulação

- **Simulação no Wokwi:** https://wokwi.com/projects/447600218460029953 

---

## 📽️Link do Vídeo

- **Vídeo Explicativo:** https://youtu.be/mvCQLeyjkJY

---




## Explicação Técnica

### HTTP e Endpoints
- Utilizamos **HTTP POST** para enviar dados do ESP32 para o ThingSpeak.  
- Cada envio contém os campos (`field1` a `field7`) com:  
  - Temperatura, umidade, luminosidade, tempo de estudo, status do ambiente e tema do estudo.  
- O ThingSpeak armazena os dados, que são lidos pelo **dashboard HTML** via requisições **HTTP GET** para exibir gráficos em tempo real.

---

## 🧑‍💻Códigos utilizados no projeto

# Sketch.ino
```ino
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

```

---

# DashBoard.html

```html
<!DOCTYPE html>
<html lang="pt-br">
<head>
<meta charset="UTF-8">
<title>Dashboard - Ambiente de Estudo</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<style>
    body {
        margin: 0;
        padding: 0;
        background: #f3f4f6;
        font-family: Arial, sans-serif;
    }

    h1 {
        text-align: center;
        padding: 20px;
        color: #333;
    }

    .grid {
        display: grid;
        grid-template-columns: repeat(auto-fill, minmax(380px, 1fr));
        gap: 20px;
        padding: 20px;
    }

    .card {
        background: white;
        padding: 20px;
        border-radius: 14px;
        box-shadow: 0 4px 12px rgba(0,0,0,0.12);
        transition: 0.3s;
    }

    .card:hover {
        transform: scale(1.02);
    }

    .status-box {
        padding: 14px;
        text-align: center;
        border-radius: 10px;
        font-size: 22px;
        font-weight: bold;
        color: white;
        margin-top: 10px;
    }

    .ultimoValor {
        text-align: center;
        font-size: 18px;
        margin-bottom: 10px;
        color: #333;
    }
</style>
</head>

<body>

<h1>📊 Dashboard – Monitoramento de Ambiente de Estudo</h1>

<div class="grid">

    <div class="card">
        <div class="ultimoValor" id="tempUltimo">Última: -- °C</div>
        <canvas id="tempChart"></canvas>
    </div>
    <div class="card">
        <div class="ultimoValor" id="humUltimo">Última: -- %</div>
        <canvas id="humChart"></canvas>
    </div>
    <div class="card">
        <div class="ultimoValor" id="ldrUltimo">Última: --</div>
        <canvas id="ldrChart"></canvas>
    </div>
    <div class="card">
        <div class="ultimoValor" id="timerUltimo">Última: -- s</div>
        <canvas id="timerChart"></canvas>
    </div>

    <div class="card">
        <h2>Ambiente Ideal?</h2>
        <div id="statusBox" class="status-box">Carregando...</div>
    </div>

    <div class="card">
        <h2>Tema do Estudo</h2>
        <div id="temaBox" class="status-box" style="background:#3498db;">Carregando...</div>
    </div>

</div>

<script>

const channelID = 3174046;
const readAPI = "158O1MDXCF4AAI63";
const url = `https://api.thingspeak.com/channels/${channelID}/feeds.json?api_key=${readAPI}&results=60`;


function criaGrafico(ctx, label, borderColor, tipo="line") {
    return new Chart(ctx, {
        type: tipo,
        data: {
            labels: [],
            datasets: [{
                label: label,
                data: [],
                borderWidth: 3,
                borderColor: borderColor,
                backgroundColor: borderColor + "55",
                tension: 0.3
            }]
        },
        options: {
            responsive: true,
            scales: {
                y: { beginAtZero: false }
            }
        }
    });
}

let tempChart = criaGrafico(document.getElementById("tempChart"), "Temperatura (°C)", "#ff3b30", "line");
let humChart = criaGrafico(document.getElementById("humChart"), "Umidade (%)", "#0a84ff", "line");
let ldrChart = criaGrafico(document.getElementById("ldrChart"), "Luminosidade (LDR)", "#8e44ad", "bar");
let timerChart = criaGrafico(document.getElementById("timerChart"), "Tempo Estudo (s)", "#f39c12", "bar");


async function atualizarDados() {
    const req = await fetch(url);
    const data = await req.json();

    let labels = data.feeds.map(f => f.created_at);

    tempChart.data.labels = labels;
    humChart.data.labels = labels;
    ldrChart.data.labels = labels;
    timerChart.data.labels = labels;

    tempChart.data.datasets[0].data = data.feeds.map(f => f.field1);
    humChart.data.datasets[0].data = data.feeds.map(f => f.field2);
    ldrChart.data.datasets[0].data = data.feeds.map(f => f.field3);
    timerChart.data.datasets[0].data = data.feeds.map(f => f.field5);

    const ultimo = data.feeds[data.feeds.length - 1];

    document.getElementById("tempUltimo").innerText = `Última: ${ultimo.field1 || "--"} °C`;
    document.getElementById("humUltimo").innerText = `Última: ${ultimo.field2 || "--"} %`;
    document.getElementById("ldrUltimo").innerText = `Última: ${ultimo.field3 || "--"}`;
    document.getElementById("timerUltimo").innerText = `Última: ${ultimo.field5 || "--"} s`;

    const ambienteRaw = ultimo.field6;
    const ambiente = Number(ambienteRaw);
    let box = document.getElementById("statusBox");

    if (ambienteRaw === null || ambienteRaw === "" || (ambiente !== 0 && ambiente !== 1)) {
        box.style.background = "#7f8c8d";
        box.innerText = "Aguardando finalizar o estudo...";
    } 
    else if (ambiente === 1) {
        box.style.background = "#2ecc71";
        box.innerText = "✔ Ambiente Ideal!";
    } 
    else {
        box.style.background = "#e74c3c";
        box.innerText = "✘ Ambiente Ruim";
    }

    const tema = ultimo.field7 || "Nenhum tema encontrado";
    document.getElementById("temaBox").innerText = tema;

    tempChart.update();
    humChart.update();
    ldrChart.update();
    timerChart.update();
}

atualizarDados();
setInterval(atualizarDados, 10000);

</script>

</body>
</html>


---
