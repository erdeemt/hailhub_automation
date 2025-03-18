#include <esp_now.h>
#include <WiFi.h>

// Alıcı ESP32'nin MAC adresi (ESP32-2'nin MAC adresini buraya yaz)
uint8_t receiverMac[] = {0xE4, 0x65, 0xB8, 0xDA, 0x69, 0x94};

String incomingData;

// Gönderilecek veri yapısı
typedef struct {
    float hiz;
    float basinc;
} SensorData;
SensorData sensorData;

// Alınan veri yapısı
typedef struct {
    bool roleDurumu;
} RoleData;
RoleData roleData;

// Callback fonksiyonu: Veri gönderildiğinde çalışır
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Gönderim Durumu: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Başarılı" : "Başarısız");
}

// Callback fonksiyonu: Veri alındığında çalışır
void OnDataRecv(const esp_now_recv_info* recv_info, const uint8_t *incomingData, int len) {
    memcpy(&roleData, incomingData, sizeof(roleData));
    Serial.print("Alınan Röle Durumu: ");
    Serial.println(roleData.roleDurumu);
}

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, 16, 17);  // ESP RX=16, TX=17
    pinMode(5, OUTPUT);
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW başlatılamadı!");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv); // Bu satırda artık uyumlu geri çağırma fonksiyonu kullanıyoruz.

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Eşleşme başarısız!");
        return;
    }
}

void loop() {

  if(roleData.roleDurumu){
    digitalWrite(5,HIGH);
    }else{
    digitalWrite(5,LOW);
  }


  if (Serial1.available()) {
    incomingData = Serial1.readStringUntil('\n');  // Satır sonuna kadar oku
    // Verileri ayır ve değişkenlere ata
    int commaIndex = incomingData.indexOf(',');
    if (commaIndex != -1) {
      sensorData.hiz = incomingData.substring(0, commaIndex).toInt();
      sensorData.basinc = incomingData.substring(commaIndex + 1).toInt();
      }
  }

    Serial.print("Gönderilen Hız: ");
    Serial.print(sensorData.hiz);
    Serial.print(" | Gönderilen Basınç: ");
    Serial.println(sensorData.basinc);

    esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&sensorData, sizeof(sensorData));

    if (result == ESP_OK) {
        Serial.println("Veri başarıyla gönderildi.");
    } else {
        Serial.println("Veri gönderme başarısız!");
    }







    delay(500); // 2 saniye bekle
}
}