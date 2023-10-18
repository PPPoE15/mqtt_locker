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

const char* ssid     = "smart_locker";
const char* password = "12345678";
String input_ssid = "";
String input_pass = "";

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
            <input type='text' class='element' value='Znachenie' id='ssid_id' size=3 autofocus/>\
            <div id='read'></div>\
            <h2>Password</h2>\
            <input type='text' id='pass_id' size=3 autofocus>\
            <div>\
            <br><button id=\"connect_button\">connect</button>\
            </div>\
      <script>\
        $(function() {\
          function readValue(value) {\
            return $(value).val();\
          }\
          $('#read').html( readValue('.element') );\
          $('.element').keyup(function(){\
            $('#read').html( readValue('.element') );\
          });\
        });\
      </script>\
    </body>\
  </html>"); 
  server.send ( 200, "text/html", htmlResponse );  
  Serial.println("HANDLING ROOT");
}

void handleWork(){
  snprintf(htmlResponse, 100, "Hello");
}

void handleSave() {
  //Serial.println(server.args());
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

  if (input_ssid != "" && input_pass != ""){
    // Connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(input_ssid);
    WiFi.begin(input_ssid, input_pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.on ( "/", handleWork );
    //server.on ("/save", handleSave);
    Serial.println ( "HTTP server started" );
  } 
}

void setup() {
  // Start serial
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on ( "/", handleRoot );
  server.on ("/save", handleSave);
  server.begin();
  Serial.println("Server started");

}

void loop() {
  server.handleClient();
}





/*
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
*/