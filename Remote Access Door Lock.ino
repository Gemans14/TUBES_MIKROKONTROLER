#define BLYNK_TEMPLATE_ID "TMPL6YDzJaJnU" // Ganti dengan Template ID Anda
#define BLYNK_TEMPLATE_NAME "IoT RFID"    // Ganti dengan Template Name Anda

#include <SPI.h>
#include <MFRC522.h>
#include <BlynkSimpleEsp8266.h>

#define SS_PIN 2
#define RST_PIN 0
#define eLock D0 // Pin solenoid

MFRC522 rfid(SS_PIN, RST_PIN);

String strID;

// Informasi autentikasi Blynk
char auth[] = "Tux6ntkViyCzbXYUHSRtj_3eZbNKnsZ8"; // Ganti dengan token Blynk Anda
char ssid[] = "KOST BERSAMA";                     // Ganti dengan SSID WiFi Anda
char pass[] = "JOKOWI5D";                         // Ganti dengan password WiFi Anda

WidgetTerminal terminal(V2); // Terminal di Virtual Pin V2

// Daftar UID dan Nama
const int numCards = 3; // Jumlah kartu yang dikenal
String knownUIDs[numCards] = {
    "f3:24:3d:2", // UID Kartu 1
    "6e:35:91:8b", // UID Kartu 2
    "83:c:1:2e"    // UID Kartu 3
};
String names[numCards] = {
    "Peserta 1", // Nama untuk UID Kartu 1
    "Peserta 2", // Nama untuk UID Kartu 2
    "Umum"       // Nama untuk UID Kartu 3
};

void setup() {
  SPI.begin();
  rfid.PCD_Init();
  Serial.begin(115200);

  // Hubungkan ke Blynk
  Serial.println("Menghubungkan ke Blynk...");
  Blynk.begin(auth, ssid, pass);
  while (!Blynk.connected()) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nBlynk Terhubung!");

  Serial.println("I am waiting for card...");

  // Konfigurasi pin solenoid
  pinMode(eLock, OUTPUT);
  digitalWrite(eLock, HIGH); // Kunci solenoid default tertutup

  // Kirim pesan awal ke Terminal
  terminal.println("Perangkat RFID IoT siap.");
  terminal.println("Silakan scan kartu RFID.");
  terminal.flush();
}

void loop() {
  Blynk.run(); // Jalankan fungsi Blynk
  iot_rfid();  // Panggil fungsi membaca RFID
}

void iot_rfid() {
  // Membaca kartu RFID
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Proses UID kartu
  strID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    strID += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) strID += ":";
  }

  // Cek apakah UID ada di daftar
  String userName = "Tidak Dikenal";
  bool isAuthorized = false;
  for (int i = 0; i < numCards; i++) {
    if (strID.equalsIgnoreCase(knownUIDs[i])) {
      userName = names[i];
      isAuthorized = true;
      break;
    }
  }

  // Tampilkan nama pengguna di Serial Monitor
  Serial.println("UID Kartu Anda: " + strID);
  Serial.println("Nama: " + userName);

  // Tampilkan nama pengguna di Terminal Blynk
  terminal.println("UID Kartu Anda: " + strID);
  terminal.println("Nama: " + userName);
  terminal.flush();

  // Logika membuka kunci solenoid
  if (isAuthorized) {
    terminal.println("Akses Diterima! Membuka kunci...");
    bukaKunci(); // Fungsi untuk membuka kunci solenoid
  } else {
    terminal.println("Akses Ditolak!");
  }
  terminal.flush();

  // Hentikan komunikasi dengan kartu
  rfid.PICC_HaltA();
}

void bukaKunci() {
  digitalWrite(eLock, LOW); // Buka kunci solenoid
  delay(5000);              // Tunggu 5 detik
  digitalWrite(eLock, HIGH); // Kunci kembali solenoid
}
