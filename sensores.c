#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include "Adafruit_HTU21DF.h"

#define SDA_PIN 21
#define SCL_PIN 22

// --- Sensores ---
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// --- Configuração de tempo ---
const int intervaloLeitura = 1000;
unsigned long tempoAnterior = 0;

// --- WIFI ---
const char* ssid     = "SAUDADES NAGUMO";
const char* password = "hulk4036";
// WiFiServer server(80); // Removido: Você é Cliente (envia dados), não Servidor.

// --- CLASSE PARA MÉDIA ---
class CalculadoraMedia {
  private:
    float soma = 0;
    int contador = 0;
    const int totalAmostras = 60; // 60 segundos = 1 minuto

  public:
    // Função que adiciona valor e retorna a média se estiver pronta (senão retorna -1)
    float adicionar(float valor) {
      if (isnan(valor)) return -1.0;

      soma += valor;
      contador++;

      if (contador >= totalAmostras) {
        float mediaFinal = soma / totalAmostras;
        // Resetar para o próximo minuto
        soma = 0;
        contador = 0;
        return mediaFinal;
      }
      return -1.0; // Ainda calculando
    }
};

// Criar um "calculador" INDEPENDENTE para cada variável
CalculadoraMedia calcPressao;
CalculadoraMedia calcTemp;
CalculadoraMedia calcAlt;
CalculadoraMedia calcUmid;


void setup() {
  Serial.begin(115200);

  // 1. Inicializa Sensores e I2C (Fora do loop do WiFi)
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!bmp.begin()) {
    Serial.println("Erro: BMP180 não encontrado!");
    while (1);
  }
  if (!htu.begin()) {
    Serial.println("Erro: HTU21D não encontrado!");
    while (1);
  }

  // 2. Conecta WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
}

void loop() {
  // Verifica conexão antes de tudo
  if (WiFi.status() == WL_CONNECTED) {
    
    unsigned long tempoAtual = millis(); // Faltava ponto e vírgula aqui

    // Loop de 1 segundo (Não usar delay aqui dentro!)
    if (tempoAtual - tempoAnterior >= intervaloLeitura) {
      tempoAnterior = tempoAtual;

      // --- Leitura dos sensores ---
      sensors_event_t event;
      bmp.getEvent(&event);
      
      float rawPressure = event.pressure;
      float rawTemp;
      bmp.getTemperature(&rawTemp);
      float rawAltitude = bmp.pressureToAltitude(1013.25, rawPressure);
      float rawHumity = htu.readHumidity();

      // --- Acumula nas médias individuais ---
      // Cada variável tem seu próprio balde de soma agora
      float mediaPressao = calcPressao.adicionar(rawPressure);
      float mediaTemp    = calcTemp.adicionar(rawTemp);
      float mediaAlt     = calcAlt.adicionar(rawAltitude);
      float mediaUmid    = calcUmid.adicionar(rawHumity);

      Serial.println("Amostra coletada. Calculando...");

      // --- Verifica se o minuto fechou (Se todos retornaram diferente de -1) ---
      if (mediaPressao != -1.0 && mediaTemp != -1.0) {
        
        Serial.println(">>> Minuto fechado! Enviando dados...");
        HTTPClient http;
        http.begin("http://192.168.1.14:5000/dados");
        http.addHeader("Content-Type", "application/json");

        // Montagem do JSON
        String json = "{";
        json += "\"temp\":" + String(mediaTemp) + ",";
        json += "\"pressao\":" + String(mediaPressao) + ",";
        json += "\"altitude\":" + String(mediaAlt) + ",";
        json += "\"umidade\":" + String(mediaUmid);
        json += "}";

        int codigo = http.POST(json);
        
        if (codigo > 0) {
          Serial.print("JSON Enviado. Código HTTP: ");
          Serial.println(codigo); // Esperado 200 ou 201
        } else {
          Serial.print("Erro no envio: ");
          Serial.println(http.errorToString(codigo).c_str());
        }

        http.end();
      }
    }
  } 
  else {
    Serial.println("WiFi Desconectado! Tentando reconectar...");
    WiFi.reconnect();
    delay(2000); // Aqui pode usar delay pois é erro de conexão
  }
}
  

