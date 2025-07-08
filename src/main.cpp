#include <WiFi.h>
#include <ESPmDNS.h>      // <-- Tambahkan library ini
#include <PubSubClient.h>

// --- 1. KONFIGURASI WIFI ---
const char* ssid = "Hijau Guest";
const char* password = "HijauAndal20";

// --- 2. KONFIGURASI MQTT HOME ASSISTANT ---
const char* mqtt_host = "homeassistant"; // Gunakan nama host, tanpa .local
const int   mqtt_port = 1883;
const char* mqtt_user = "testmqtt";
const char* mqtt_password = "Test123";

// --- 3. KONFIGURASI PESAN TES ---
const char* mqtt_topic = "esp32/test/message";
const char* test_message = "Halo dari ESP32 dengan mDNS!";

// Inisialisasi klien WiFi dan MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi untuk menghubungkan ke WiFi (Tidak ada perubahan)
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung!");
  Serial.print("Alamat IP ESP32: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk menghubungkan kembali ke MQTT (Tidak ada perubahan)
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    String clientId = "ESP32Client-Test-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("terhubung!");
    } else {
      Serial.print("gagal, kode error = ");
      Serial.print(client.state());
      Serial.println(" | Coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  // --- BAGIAN BARU: CARI IP DARI HOSTNAME ---
  Serial.println("Memulai mDNS...");
  if (!MDNS.begin("esp32")) { // 'esp32' adalah nama mDNS untuk perangkat ini
    Serial.println("Error memulai mDNS responder");
    return;
  }
  Serial.print("Mencari host: ");
  Serial.println(mqtt_host);

  // Cari IP address dari hostname, tunggu hingga 5 detik
  IPAddress mqtt_server_ip = MDNS.queryHost(mqtt_host, 5000);

  if (mqtt_server_ip == IPAddress(0, 0, 0, 0)) {
    Serial.println("Host tidak ditemukan! Pastikan Home Assistant aktif dan berada di jaringan yang sama.");
    // Kode akan berhenti di sini jika host tidak ditemukan
    while(true) { delay(1000); }
  }

  Serial.print("Host ditemukan! Alamat IP Home Assistant: ");
  Serial.println(mqtt_server_ip);
  // --- AKHIR BAGIAN BARU ---

  // Atur server MQTT menggunakan IP yang berhasil ditemukan
  client.setServer(mqtt_server_ip, mqtt_port);

  // Lanjutkan proses seperti biasa
  if (!client.connected()) {
    reconnect_mqtt();
  }
  
  Serial.print("Mengirim pesan tes ke topic: ");
  Serial.println(mqtt_topic);
  client.publish(mqtt_topic, test_message);
  Serial.println("Pesan tes berhasil dikirim!");
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
  delay(1000);
}