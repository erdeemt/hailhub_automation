#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <esp_now.h>
#include <WiFi.h>

#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED   0xF800
#define GREEN 0x07E0
#define MISSILE_SWITCH_PIN 18  

MCUFRIEND_kbv tft;


// ------------ BAŞLANGIÇ DEĞERLERİ ------------------

float lastShotPressure = -1.0;
float lastShotSpeed = 0.0;
float currentPressure;
int relayState = 0;
int previousRelayState = 0;  // Önceki röle durumunu saklayacağız

uint8_t senderMAC[] = {0x0C, 0xB8, 0x15, 0x5A, 0xD9, 0x10};

/// ------------------ DİĞER ESP32'DEN ESPNOW İLE GELECEK VERİLER ------------------

typedef struct Sensor_Data {
    float basinc;
    float hiz;
} SensorData;

SensorData receivedData;


/// ------------------ DİĞER ESP32'YE ESPNOW İLE GÖNDERİLECEK VERİ ------------------
typedef struct Role_Data { 
    bool roleDurumu; 
} RoleData;

RoleData responseData; //


esp_now_peer_info_t peerInfo;

/// ------------------ GELEN VERİ ------------------

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    memcpy(&receivedData, incomingData, sizeof(receivedData));

    Serial.print("Gelen Basınç: "); Serial.println(receivedData.basinc);
    Serial.print("Gelen Hız: "); Serial.println(receivedData.hiz);

    currentPressure = receivedData.hiz;
    updateCurrentPressure();

    // Röle 1 OLDUĞU ANDA sadece 1 kere güncelle
    if (relayState == 1 && previousRelayState == 0) {  
        lastShotSpeed = receivedData.basinc;
        lastShotPressure = currentPressure;
        updateLastShotData();
    }

    previousRelayState = relayState;  // Önceki durumu güncelle
}

/// ------------------ EKRANDA GÖSTERİLEN RELAY DEĞERİNİN GÜNCELLENMESİ ------------------

void updateRelay(){
   static int lastRelayState = -1;  // Önceki relay durumunu sakla
    
    // Eğer relay durumu değiştiyse ekranı güncelle
    if (relayState != lastRelayState) {
        tft.setTextColor(WHITE, BLACK);
        tft.setCursor(100, 25);
        
        if (relayState == 0) {
            tft.setTextColor(RED, BLACK);
            tft.print("NO SIGNAL ");
        } else {
            tft.setTextColor(GREEN, BLACK);
            tft.print("TRIGGERED ");
        }

    lastRelayState = relayState;  // Son durumu güncelle
}
}

//------------------ EKRANDAKİ CURRENT PRESSURE DEĞERİNİN GÜNCELLENMESİ ------------------
void updateCurrentPressure() {
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(3);
    tft.setCursor(340, 120);
    tft.print(currentPressure, 1);
    tft.setCursor(420, 120);
    tft.print("bar");
}
//------------------ EKRANDAKİ LAST SHOT VERİLERİNİN GÜNCELLENMESİ ------------------
void updateLastShotData() {
    tft.setTextColor(WHITE, BLACK);

    tft.setTextSize(3);
    tft.setCursor(30, 240);
    tft.print(lastShotSpeed, 1);
    tft.setCursor(120, 240);
    tft.print("m/s");

    tft.setTextSize(3);
    tft.setCursor(290, 240);
    tft.print(lastShotPressure, 1);
    tft.setCursor(390, 240);
    tft.print("bar");
}
// ------------------ EKRAN------------------
void tftBaslangic() {
    tft.reset();
    uint16_t ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(1);
    tft.fillScreen(BLACK);  

    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 25);
    tft.print("RELAY :");

    tft.setTextSize(2);
    tft.setCursor(10, 200);
    tft.print("Last Shot Speed");

    tft.setTextSize(2);
    tft.setCursor(250, 200);
    tft.print("Last Shot Pressure");

    tft.setTextSize(3);
    tft.setCursor(10, 120);
    tft.print("Current Pressure = ");
}

void setup() {
  
    Serial.begin(115200);
    tftBaslangic();
    delay(2000);

    pinMode(MISSILE_SWITCH_PIN, INPUT_PULLUP);

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW başlatılamadı!");
        return;
    }

    memcpy(peerInfo.peer_addr, senderMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Peer eklenemedi.");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    int switchState = digitalRead(MISSILE_SWITCH_PIN);
    relayState = (switchState == LOW) ? 1 : 0; 

    responseData.roleDurumu = relayState;  // Struct içindeki değişkene değer ata

    esp_err_t result = esp_now_send(senderMAC, (uint8_t *) &responseData, sizeof(responseData));
    

    if (result == ESP_OK) {
        Serial.print("Röle durumu gönderildi: ");
        Serial.println(responseData.roleDurumu);
    } else {
        Serial.println("Röle durumu gönderilemedi.");
    }
    updateRelay();
    delay(500);
}
