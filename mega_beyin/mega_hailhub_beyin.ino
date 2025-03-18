// HAILHUB AUTOMATION PROJECT MAIN CONTROLLER CODE

//------------------------------------------------------------------------------------------
/*
Defining gloabal values for distance sensors to measure hail speed
*/
const int sensor1Pin = 2;  // Sensör 1 için kesme pini (INT0) digital 2nd pin of mega
const int sensor2Pin = 3;  // Sensör 2 için kesme pini (INT1) digital 3rd pin of mega


const float distance = 0.25; // Sensörler arası mesafe (metre cinsinden)
int threshold = 100;  // Sensör tetikleme eşiği (Deneyerek ayarlayabilirsin)
unsigned long t1 = 0, t2 = 0;  // Zaman damgaları
bool sensor1_triggered = false;
bool sensor2_triggered = false;
float speed ;
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/*


Defining global values for relay to launch the hail shot


*/
int relayPin = 7 ; // relay trigger pin is connected to digital7 pin
#define relaysignalpin A3 // signal coming from esp32 to trigger relayPin
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/*

Defining global values for pressure sensor 
*/
int value;
float pressure; 
#define signalPin A0


//------------------------------------------------------------------------------------------

void sensor1_ISR() {
    if (!sensor1_triggered) {
        t1 = micros();  // Mikro saniye cinsinden zaman kaydet
        sensor1_triggered = true;
    }
}

// Sensör 2 kesme fonksiyonu
void sensor2_ISR() {
    if (sensor1_triggered && !sensor2_triggered) {
        t2 = micros();  // Mikro saniye cinsinden zaman kaydet
        sensor2_triggered = true;
  }
}


void setup()
{
  Serial.begin(9600);
  Serial1.begin(115200);
  pinMode(A0,INPUT); // Pressure signal pin connected to A0
  attachInterrupt(digitalPinToInterrupt(sensor1Pin), sensor1_ISR, RISING);  // Sensör 1 kesmesi
  attachInterrupt(digitalPinToInterrupt(sensor2Pin), sensor2_ISR, RISING);  // Sensör 2 kesmesi

}

void loop()
{
//------------------------------------------------------------------------------------------
// Speed measuring code
  if (sensor1_triggered && sensor2_triggered) {
        float timeDiff = (t2 - t1) / 1000000.0;  // Mikro saniyeyi saniyeye çevir
        speed = distance / timeDiff;  // Hız hesapla (m/s)
        Serial.print("Hız: ");
        Serial.print(speed);
        Serial.println(" m/s");

        // Yeni ölçüm için değişkenleri sıfırla
        sensor1_triggered = false;
        sensor2_triggered = false;
    }

//------------------------------------------------------------------------------------------
// Pressure measuring code
  value = analogRead(A0);
  pressure = (value)/(100.0);
  Serial.print("Measured pressure is : ");
  Serial.print(pressure);
  Serial.print(" bar");
  Serial.println("---");
  delay(100);
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------ 
  // Send values to ESP32
  Serial1.print(speed,2);     
  Serial1.print(",");
  Serial1.println(pressure,2);

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Relay trigger code,

  if(analogRead(A3) > 200)
  {
    digitalWrite(relayPin, HIGH);
  }
  else 
  {
    digitalWrite(relayPin,LOW);
  }

//------------------------------------------------------------------------------------------

}
