#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define SS_PIN   21  // Pino do leitor RFID - SDA
#define RST_PIN  22  // Pino do leitor RFID - RST

#define BLYNK_TEMPLATE_ID "TMPL2tFYL3QGL"
#define BLYNK_TEMPLATE_NAME "LedESensor"
#define BLYNK_AUTH_TOKEN "GaIZru9LoYPliRFU0hTx98vVS7aAz-wE"
#define WIFI_SSID       "REP - 2.4Gh"
#define WIFI_PASSWORD   "3B9E4F45"
MFRC522 mfrc522(SS_PIN, RST_PIN);


volatile bool rfidEnabled = true;
volatile bool rfidDetected = false;
volatile unsigned long lastRFIDTime = 0;
volatile bool flowTimeout = false;
volatile unsigned long lastFlowTime = 0;
double volumeTotal = 0;

SemaphoreHandle_t flowMutex;



byte flag, sf;
float vazao;
byte statusLed    = 25;

TaskHandle_t RFIDTask;
TaskHandle_t FlowTask;

const int PINO_SENSOR = 15;
volatile int contador = 0;


float volumeML = 0;
float flowFactor = 7.5; // Fator de fluxo específico para o sensor YF-S201

void RFIDTaskCode(void* pvParameters);
void FlowTaskCode(void* pvParameters);


void setup() {
  Serial.begin(9600);
  SPI.begin();        // Inicializa a interface SPI
  mfrc522.PCD_Init(); // Inicializa o leitor RFID
  pinMode(PINO_SENSOR, INPUT);
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, LOW);
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
  Serial.println("APROXIME UM CARTÃO DO SENSOR");

  
  xTaskCreatePinnedToCore(
    RFIDTaskCode,
    "RFID Task",
     8192,
    NULL,
    1,
    &RFIDTask,
    1
  );

  xTaskCreatePinnedToCore(
    FlowTaskCode,
    "Flow Task",
    8192,
    NULL,
    1,
    &FlowTask,
    1
  );

}

void loop() {
  Blynk.run();

}

void RFIDTaskCode(void* pvParameters) {
  for (;;) {
      // Verifica se um cartão RFID está presente
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        Serial.print("Cartão RFID detectado: ");
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
        }
        lastRFIDTime = millis();
        mfrc522.PICC_HaltA();
        digitalWrite(statusLed, HIGH); // Acende o LED indicando que o cartão é válido

    }
  

    if (millis() - lastRFIDTime >= 10000) {
      digitalWrite(statusLed, LOW); // Desliga o LED quando o cartão não é válido
      Serial.println("APROXIME UM CARTÃO DO SENSOR");
    }



    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  
}

void FlowTaskCode(void* pvParameters) {
    sf = digitalRead(PINO_SENSOR);
    lastFlowTime = millis();
    if (sf && flag) {
    contador++;
    flag = 0;
    }
    if (!sf) {
      flag = 1;
      volumeTotal= contador * 2.083333333;
      Serial.println(volumeTotal);  
      Serial.println(" ml");
      Blynk.virtualWrite(V0,volumeTotal); // Envia o valor do volume  para o aplicativo Blynk;
    }
    //volumeTotal= contador * 2.083333333;
   // Serial.println(volumeTotal);  
    //Serial.println(" ml");
   // Blynk.virtualWrite(V0,volumeTotal); // Envia o valor do volume  para o aplicativo Blynk;

    // Serial.println(contador * 2.083333333);  
     //Serial.println(" ml");
     //Blynk.virtualWrite(V0,contador * 2.083333333); // Envia o valor do volume  para o aplicativo Blynk;
    // Lê o valor do sensor de fluxo

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
  }

