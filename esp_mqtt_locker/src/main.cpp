#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <WiFiAP.h>
  WebServer server(80);
#else
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
#endif
const char* ssid     = "LIIS";
const char* password = "qw8J*883";
String input_ssid;
String input_pass;

char htmlResponse[3000];

void handleRoot() {
  snprintf ( htmlResponse, 3000,
  "<!DOCTYPE html>\
  <html lang=\"en\">\
    <head>\
      <meta charset=\"utf-8\">\
      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    </head>\
    <body>\
            <h1>Screen message</h1>\
            <h2>SSID name</h2>\
            <input type='text' id='ssid_id' size=3 autofocus>\
            <h2>Password</h2>\
            <input type='text' id='pass_id' size=3 autofocus>\
            <div>\
            <br><button id=\"connect_button\">connect</button>\
            </div>\
      <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>\    
      <script>\
        $('#connect_button').click(function(e){\
          e.preventDefault();\
          var ssid;\
          var pass;\
          ssid = $('#ssid_id').val();\
          pass = $('#pass_id').val();\
          $.get('/save?ssid=' + ssid, function(data){\
            console.log(data);\
          });\
          $.get('/save?pass=' + pass, function(data){\
            console.log(data);\
          });\
        });\  
      </script>\
    </body>\
  </html>"); 
  server.send ( 200, "text/html", htmlResponse );  
  Serial.println("HANDLING ROOT");
}

void handleSave() {
  //Serial.println(server.args());

  if (server.args()==1){
    if (server.arg("ssid")!=""){
      input_ssid = server.arg("ssid");
      Serial.println("SSID:");
      Serial.println(input_ssid);
    }

    if (server.arg("pass")!=""){
      input_pass = server.arg("pass");
      Serial.println("Pass:");
      Serial.println(input_pass);
    }
  }  
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  // Start serial
  Serial.begin(115200);
  delay(10);
  // Connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.on ( "/", handleRoot );
  server.on ("/save", handleSave);
  server.begin();
  Serial.println ( "HTTP server started" );
}

void loop() {
  server.handleClient();
}