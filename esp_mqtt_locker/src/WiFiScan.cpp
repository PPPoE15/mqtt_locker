#include <WiFi.h>

String WifiScan(){
  String content;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");

  Serial.println("Scan start");

  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
    content += "Scan done <br>";
  if (n == 0) {
      Serial.println("no networks found");
        content += "No networks found <br>";
  } else {
      Serial.println("Networks list:");
        content += "Networks list: <br>";
      Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        content += "Nr | SSID                             | RSSI | CH | Encryption <br>";
      for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          Serial.printf("%2d",i + 1);
            content += Serial.printf("%2d",i + 1);
          Serial.print(" | ");
            content += "Networks list: <br>";
          Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            content += "Networks list: <br>";
          Serial.print(" | ");
            content += "Networks list: <br>";
          Serial.printf("%4d", WiFi.RSSI(i));
            content += "Networks list: <br>";
          Serial.print(" | ");
            content += "Networks list: <br>";
          Serial.printf("%2d", WiFi.channel(i));
            content += "Networks list: <br>";
          Serial.print(" | ");
            content += "Networks list: <br>";
          switch (WiFi.encryptionType(i))
          {
          case WIFI_AUTH_OPEN:
              Serial.print("open");
              break;
          case WIFI_AUTH_WEP:
              Serial.print("WEP");
              break;
          case WIFI_AUTH_WPA_PSK:
              Serial.print("WPA");
              break;
          case WIFI_AUTH_WPA2_PSK:
              Serial.print("WPA2");
              break;
          case WIFI_AUTH_WPA_WPA2_PSK:
              Serial.print("WPA+WPA2");
              break;
          case WIFI_AUTH_WPA2_ENTERPRISE:
              Serial.print("WPA2-EAP");
              break;
          case WIFI_AUTH_WPA3_PSK:
              Serial.print("WPA3");
              break;
          case WIFI_AUTH_WPA2_WPA3_PSK:
              Serial.print("WPA2+WPA3");
              break;
          case WIFI_AUTH_WAPI_PSK:
              Serial.print("WAPI");
              break;
          default:
              Serial.print("unknown");
          }
          Serial.println();
          delay(10);
      }
  }
  Serial.println("");
  content += "<br>";

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();

  return content;
}