#include <WiFi.h>         // Library untuk koneksi Wi-Fi
#include <PubSubClient.h>  // Library untuk komunikasi MQTT
#include <DHTesp.h>        // Library untuk membaca sensor DHT (Suhu dan Kelembapan)

const int DHT_PIN = 15;    // Pin yang digunakan untuk sensor DHT
DHTesp dht;                // Objek untuk sensor DHT

const char* ssid = "Wokwi-GUEST";  // Nama Wi-Fi yang akan dihubungkan
const char* password = "";         // Password Wi-Fi (kosongkan jika tidak ada)
const char* mqtt_server = "test.mosquitto.org";  // Alamat broker MQTT

#define LED 26  // Pin LED untuk menyalakan lampu

WiFiClient espClient;        // Objek untuk Wi-Fi
PubSubClient client(espClient);  // Objek untuk MQTT

unsigned long lastMsg = 0;    // Variabel untuk menghitung waktu
float temp = 0;               // Variabel untuk menyimpan data suhu
float hum = 0;                // Variabel untuk menyimpan data kelembapan

// Fungsi untuk menghubungkan ke Wi-Fi
void setup_wifi() { 
  delay(10); 
  Serial.println(); 
  Serial.print("Connecting to "); 
  Serial.println(ssid); 

  WiFi.mode(WIFI_STA);          // Mengatur Wi-Fi mode ke Station (client)
  WiFi.begin(ssid, password);  // Memulai koneksi Wi-Fi

  // Menunggu hingga terhubung ke Wi-Fi
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }

  randomSeed(micros());  // Menyiapkan nilai acak
  Serial.println(""); 
  Serial.println("WiFi connected"); 
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());  // Menampilkan alamat IP yang didapat
}

// Fungsi callback yang dijalankan ketika menerima pesan MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived ["); 
  Serial.print(topic); 
  Serial.print("] "); 

  String data = "";
  for (int i = 0; i < length; i++) { 
    data += (char)payload[i];  // Membaca payload pesan
  }

  Serial.println(data);
  Serial.print("Message size: "); 
  Serial.println(length); 
  Serial.println();

  // Jika topik adalah "lampu", kontrol LED
  if (String(topic) == "lampu") { 
    if (data == "on") { 
      Serial.println("LED ON"); 
      digitalWrite(LED, HIGH);  // Menyalakan LED
    } else { 
      digitalWrite(LED, LOW);  // Mematikan LED
      Serial.println("LED OFF");
    }
  }
}

// Fungsi untuk mencoba menghubungkan ke broker MQTT
void reconnect() { 
  while (!client.connected()) { 
    Serial.print("Attempting MQTT connection..."); 
    String clientId = "ESP32Client-"; 
    clientId += String(random(0xffff), HEX);  // Membuat ID client unik

    // Jika berhasil terhubung
    if (client.connect(clientId.c_str())) { 
      Serial.println("Connected"); 
      client.subscribe("lampu");  // Subscribe ke topik "lampu" untuk mengontrol LED
      client.publish("/ThinkIOT/Publish", "PRAKTIK");  // Publish pesan ke topik "/ThinkIOT/Publish"
      client.subscribe("/ThinkIOT/Subscribe");  // Subscribe ke topik "/ThinkIOT/Subscribe"
    } else { 
      Serial.print("failed, rc="); 
      Serial.print(client.state()); 
      Serial.println(" try again in 5 seconds"); 
      delay(5000);  // Tunggu 5 detik jika gagal
    }
  }
}

// Fungsi setup untuk inisialisasi awal
void setup() { 
  pinMode(LED, OUTPUT);  // Set pin LED sebagai OUTPUT
  Serial.begin(115200);  // Inisialisasi komunikasi serial dengan baudrate 115200
  setup_wifi();  // Menghubungkan ke Wi-Fi
  client.setServer(mqtt_server, 1883);  // Menghubungkan ke broker MQTT
  client.setCallback(callback);  // Menetapkan fungsi callback untuk menangani pesan MQTT
  dht.setup(DHT_PIN, DHTesp::DHT22);  // Menyiapkan sensor DHT22 pada pin yang sudah ditentukan
}

// Fungsi loop untuk menjalankan proses secara terus-menerus
void loop() { 
  if (!client.connected()) { 
    reconnect();  // Menghubungkan kembali jika MQTT tidak terhubung
  } 
  client.loop();  // Memeriksa pesan yang diterima

  unsigned long now = millis();  // Mendapatkan waktu saat ini

  // Jika lebih dari 2 detik sejak pengukuran terakhir, baca data suhu dan kelembapan
  if (now - lastMsg > 2000) { 
    lastMsg = now;

    TempAndHumidity data = dht.getTempAndHumidity();  // Membaca data suhu dan kelembapan
    String temp = String(data.temperature, 2);  // Menyimpan suhu dalam format string dengan 2 desimal
    client.publish("/ThinkIOT/temp", temp.c_str());  // Publish suhu ke topik "/ThinkIOT/temp"

    String hum = String(data.humidity, 1);  // Menyimpan kelembapan dalam format string dengan 1 desimal
    client.publish("/ThinkIOT/hum", hum.c_str());  // Publish kelembapan ke topik "/ThinkIOT/hum"

    Serial.print("Temperature: "); 
    Serial.println(temp); 
    Serial.print("Humidity: ");
    Serial.println(hum);
  }
}
