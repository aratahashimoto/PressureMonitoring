#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <time.h>

const char* ssid = "Buffalo-G-AFA8";
const char* password = "cvxx6euvhber6";

Adafruit_BMP280 bmp;
WiFiServer server(80);

// 日本時間
const long gmtOffset_sec = 9 * 3600;
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);

  // I2C開始
  Wire.begin();

  // BMP280開始
  Serial.println("BMP280を確認しています...");

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280が見つかりませんでした。");
    while (1);
  }

  Serial.println("BMP280を認識しました！");

  // Wi-Fi接続
  WiFi.begin(ssid, password);

  Serial.print("Wi-Fiに接続中");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi接続成功！");
  Serial.print("ESP32のIPアドレス: ");
  Serial.println(WiFi.localIP());

  // 日本時間を取得
  configTime(
    gmtOffset_sec,
    daylightOffset_sec,
    "ntp.nict.jp"
  );

  Serial.println("時刻を取得しています...");

  struct tm timeinfo;

  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("時刻取得成功！");
  Serial.println(&timeinfo, "%Y/%m/%d %H:%M:%S");

  // Webサーバー開始
  server.begin();
}

void loop() {

  WiFiClient client = server.available();

  if (client) {

    // ブラウザからのリクエストを読み取る
    String request = client.readStringUntil('\r');
    client.flush();

    // BMP280からデータ取得
    float pressure = bmp.readPressure() / 100.0F;
    float temperature = bmp.readTemperature();

    // 現在時刻
    struct tm timeinfo;
    getLocalTime(&timeinfo);

    // HTTPレスポンス
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();

    // HTML
    client.println("<html>");
    client.println("<head>");
    client.println("<meta charset='UTF-8'>");
    client.println("<meta http-equiv='refresh' content='1'>");
    client.println("<title>ESP32 気圧モニター</title>");
    client.println("</head>");

    client.println("<body>");

    client.println("<h1>ESP32 気圧モニター</h1>");

    client.println("<h2>現在時刻</h2>");
    client.print("<p style='font-size:30px;'>");
    client.println(&timeinfo, "%Y/%m/%d %H:%M:%S");
    client.println("</p>");

    client.println("<h2>気圧</h2>");
    client.print("<p style='font-size:40px;'>");
    client.print(pressure);
    client.println(" hPa</p>");

    client.println("<h2>温度</h2>");
    client.print("<p style='font-size:30px;'>");
    client.print(temperature);
    client.println(" °C</p>");

    client.println("</body>");
    client.println("</html>");

    delay(1);
    client.stop();
  }
}
