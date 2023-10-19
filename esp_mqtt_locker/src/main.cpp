#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <WiFiAP.h>
  #include <EEPROM.h>
  WebServer server(80);
#else
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
#endif

const char* ssid_AP     = "smart_locker";
const char* password_AP = "12345678";
//const char* ssid     = "LIIS";
//const char* password = "qw8J*883";
struct WiFiPair {
  String input_ssid = "";
  String input_pass = "";
};

WiFiPair inputPair;



void saveEEPROMData() {
    EEPROM.commit();
    EEPROM.end();  // Освобождение ресурсов
}

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

bool doConnect(String ssid, String pass){
  Serial.println("trying to connect");
  //WiFi.disconnect(true, true);
  //WiFi.mode();
  WiFi.begin(ssid, pass);
  WiFi.setAutoReconnect(true);
  Serial.println("");

  // Wait connection for 30 sec
  int msConnection_counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    msConnection_counter++;
    if (msConnection_counter > 60){
      return false;
    }
  }
  Serial.println("\n");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

//Check if header is present and correct
bool is_authentified() {
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin() {
  String msg;
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    inputPair.input_ssid = server.arg("USERNAME");
    inputPair.input_pass = server.arg("PASSWORD");

    if (doConnect(inputPair.input_ssid, inputPair.input_pass)) {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      EEPROM.put(0, inputPair);
      saveEEPROMData();
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "You also can go <a href='/inline'>here</a></body></html>";
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentification is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
 
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}

//no need authentification
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  EEPROM.begin(200);
  EEPROM.get(0, inputPair);
  Serial.println(inputPair.input_ssid);
  Serial.println(inputPair.input_pass);
  if (doConnect(inputPair.input_ssid, inputPair.input_ssid)){ // successfully connected to wi-fi

  }
  else{
    EEPROM.put(0, 0);
    saveEEPROMData();
    Serial.println("\n\nConfiguring access point...");
    WiFi.disconnect();
    WiFi.eraseAP();
    if (!WiFi.softAP(ssid_AP, password_AP)) {
      log_e("Soft AP creation failed.");
      while(1);
    }
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works without need of authentification");
  });

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}