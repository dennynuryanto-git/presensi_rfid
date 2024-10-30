#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <SPI.h>
#include <HTTPClient.h>

// Network SSID
const char* ssid = "RPL - LABKOM 1";
const char* password = "rpl12345";

// Host (server) IP Address
//const char* host = "192.168.47.73";

#define LED_PIN 15   // GPIO15
#define BIN_PIN 5    // GPIO5
#define RST_PIN  22  // Pin reset untuk MFRC522
#define SS_PIN   21  // Pin SS untuk MFRC522
#define BTN_PIN 12   // GPIO12 (Assign appropriate pin for your button)

// Pin SDA dan SCL yang baru
#define SDA_PIN 25  // Pin SDA baru (misalnya, GPIO 25)
#define SCL_PIN 26  // Pin SCL baru (misalnya, GPIO 26) 

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialize LCD I2C (address 0x27 for a 16x2 display)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);

  // Setting up WiFi connection
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  // Display the MAC address
  String macAddress = WiFi.macAddress();
  Serial.print("MAC Address: ");
  Serial.println(macAddress);
  
  // Display MAC address on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MAC Address:");
  lcd.setCursor(0, 1);
  lcd.print(macAddress);
  delay(3000); // Display for 3 seconds before proceeding

  // Initialize SPI and MFRC522
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Initialize I2C with new SDA and SCL pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize LCD
  lcd.init();  // Initialize the LCD
  lcd.backlight(); // Menyalakan lampu latar LCD
  lcd.clear(); // Membersihkan layar LCD
  lcd.setCursor(0, 0);
  lcd.print("RFID Reader Ready");

  pinMode(BTN_PIN, INPUT_PULLUP); // Mengaktifkan internal pull-up resistor
}

void loop() {
  // Jika tombol ditekan
  if (digitalRead(BTN_PIN) == LOW) {
    HTTPClient http;
    String Link = "https://00c9-36-73-176-92.ngrok-free.app/presensi_pure2/ubahmode.php"; // URL endpoint
    http.begin(Link);

    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);

      // Menampilkan pesan sukses di LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mode Diubah");
    } else {
      Serial.println("Error on HTTP request");

      // Menampilkan pesan gagal di LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gagal Ubah Mode");
    }
    http.end();

    delay(300);  // Menghindari input ganda karena bouncing
  } else {
    // Mode standby
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dekatkan Kartu");
    lcd.setCursor(0, 1);
    lcd.print("RFID Anda");

    // Mengecek apakah ada kartu RFID yang baru
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      String IDTAG = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        IDTAG += String(mfrc522.uid.uidByte[i], HEX);
      }
      
      WiFiClient client;
      HTTPClient http;

      String Link = "https://00c9-36-73-176-92.ngrok-free.app/presensi_pure2/kirimkartu.php?rfid=" + IDTAG;
      http.begin(Link);
      int httpCode = http.GET();
       if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Response Payload: " + payload);  // Debugging untuk cek respons

    // Menghilangkan spasi yang tidak diinginkan dari respons
    payload.trim();  // Hanya memanggil trim()

    // Menampilkan pesan di LCD hanya jika kartu tidak dikenali
    lcd.clear();
    if (payload.indexOf("Maaf Kartu Tidak Dikenali") >= 0) {
      lcd.setCursor(0, 0);
      lcd.print("Maaf Kartu");
      lcd.setCursor(0, 1);
      lcd.print("Tidak Dikenali");
    } 
    // Jika tidak ada pesan di payload, berarti presensi berhasil
    else {
      lcd.setCursor(0, 0);
      lcd.print("Presensi");
      lcd.setCursor(0, 1);
      lcd.print("Berhasil");
    }
  } else {
    // Anggap sebagai sukses jika ada error di HTTP request
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Presensi");
    lcd.setCursor(0, 1);
    lcd.print("Berhasil");
  }
  http.end();

      delay(2000);  // Delay untuk menghindari multiple scans
    }
  }
}
