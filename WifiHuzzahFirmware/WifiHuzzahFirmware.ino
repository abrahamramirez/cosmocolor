#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
const char* ssid = "INFINITUME7EC5E";
const char* password = "E3F7CFC6AA";
//const char* ssid = "INFINITUM4725";           // Nombre de la red a conectar
//const char* password = "U89fotYc2N";          // Contraseña de la red a conectar
unsigned int localUdpPort = 4210;             // Puerto UDP para recepción de paquetes
char incomingPacket[255];                     // Buffer para paquetes entrantes
String cmd;                                   // Comando AT personalizado
String strSsid;                               // Variable temporal para SSID
String strPass;                               // Variable temporal para password
int timeout = 0;                              // Tiempo de espera máximo de conexión a la red
boolean beginUdp = false;                     // Bandera para escuchar puerto UDP
boolean isConnected = false;                  // Bandera para indicar estado de conexión a red
String fromIp = "";
int fromPort = 0;


void setup(){
  Serial.begin(9600);
  Serial.println();
  delay(2000);

  WiFi.begin(ssid, password);
  // Intentar conectar hasta agotar tiempo de espera
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print("-");      // Caracter a imprimir mientras se realiza la conexión
    timeout++;
    if(timeout == 20){
      break;
    }
  }
  if(timeout == 100)
    Serial.println("TIMEOUT_ERROR");  
  else{
    Udp.begin(localUdpPort);
    beginUdp = true;
    isConnected = true;
    Serial.print(WiFi.localIP().toString());
    Serial.println("OK"); 
  }
  delay(3000);
}


void loop(){
  if(Serial.available() > 0){     
    cmd = Serial.readStringUntil('\r');             // Obtener comando AT

    if(cmd.equals("AT")){
      Serial.println("OK");  
    }
    // --------------------------------------
    // Comando para esteblecer SSID
    //
    else if(cmd.startsWith("AT+SSID=")){          
      int from = 0;
      // Descomponer cadena para obtener SSID
      for(int i = 0; i <= cmd.length(); i++){
        if(cmd.charAt(i) == '='){
          from = i;
          break;  
        } 
      }
      strSsid = cmd.substring(from + 1);
      // Convertir String a *char
      ssid = strSsid.c_str();
//      Serial.println("OK");
    }
    // ---------------------------------
    // Comando para establecer Password
    //
    else if(cmd.startsWith("AT+PASS=")){          
      int from = 0;
      // Descomponer cadena para obtener Pass
      for(int i = 0; i <= cmd.length(); i++){
        if(cmd.charAt(i) == '='){
          from = i;
          break;  
        } 
      }
      strPass = cmd.substring(from + 1);
      // Convertir String a *char
      password = strPass.c_str();
//      Serial.println("OK");
    }
    // ---------------------------------
    // Comando para obtener SSID
    //
    else if(cmd.equals("AT+SSID?")){  
      if(ssid != ""){
        Serial.println(ssid);
      }
      else{
        Serial.println("ERROR");  
      }
    }
    // ---------------------------------
    // Comando para obtener password
    //
    else if(cmd.equals("AT+PASS?")){  
      if(password != ""){
        Serial.println(password);
      }
      else{
        Serial.println("ERROR");  
      }
    }
    // --------------------------------------
    // Comando para establecer conexión
    //
    else if(cmd.equals("AT+CON")){
      isConnected = false;
      WiFi.begin(ssid, password);
      // Intentar conectar hasta agotar tiempo de espera
      while (WiFi.status() != WL_CONNECTED){
        delay(400);
        Serial.print("-");      // Caracter a imprimir mientras se realiza la conexión
        timeout++;
        if(timeout == 20){
          break;
        }
      }
      if(timeout == 100)
        Serial.println("TIMEOUT ERROR");  
      else{
        Udp.begin(localUdpPort);
        beginUdp = true;
        Serial.print(WiFi.localIP().toString());
        Serial.println("OK"); 
      }
    }
    // ---------------------------
    // Comando para obtener IP
    //
    else if(cmd.equals("AT+IP?")){  
      if(isConnected){
//        Serial.print("           ");                // Necesario para que la IP sea impresa completa
        Serial.print(WiFi.localIP().toString());
        Serial.println("OK");
      }
      else{
        Serial.println("ERROR");  
      }
    }
    // ---------------------------------
    // Comando para desconectar de red
    //
    else if(cmd.equals("AT+DIS")){
      WiFi.disconnect();
      isConnected = false;
      Serial.println("OK");  
    }
    // ----------------------------------------------
    // Comando para comenzar a recibir paquetes UDP
    //
    else if(cmd.equals("AT+UDP=TRUE")){
      if(isConnected){
        beginUdp = true;
        Serial.println("OK"); 
      }
      else{
        Serial.println("ERROR");  
      }
    }
    // --------------------------------------------
    // Comando para parar de recibir paquetes UDP
    //
    else if(cmd.equals("AT+UDP=FALSE")){
      beginUdp = false;
      Serial.println("OK"); 
    }
    // ---------------------------------------------------------------
    // Comando para parar conocer el estado de recepción de paquetes
    //
    else if(cmd.equals("AT+UDP?")){
      Serial.println(beginUdp); 
    }
    // ---------------------------------------------
    // Comando para enviar un texto mediante WiFi
    //
    else if(cmd.startsWith("AT+SEND=")){
      int count = 0;
      String s = "";
      String ip = "";
      String port = "";
      String text = "";
      int index = 0;
      // Descomponer cadena para obtener IP, Pto y Texto
      for(int i = 0; i <= cmd.length(); i++){
        if(cmd.charAt(i) == '='){
          count = i;
          break;  
        } 
      }
      s = cmd.substring(count + 1);
//      Serial.print("s= ");
//      Serial.println(s);
      // Obtener IP
      index = s.indexOf(",");
      ip = s.substring(0, index + 1);
      ip.replace(",", "");
//      Serial.print("ip= ");
//      Serial.println(ip);
      
      // Obtener puerto
      s.remove(0, index + 1);
//      Serial.print("s= ");
//      Serial.println(s);
      index = s.indexOf(",");
      port = s.substring(0, index + 1);
      port.replace(",", "");
//      Serial.print("port= ");
//      Serial.println(port);

      // Obtener texto
      s.remove(0, index + 1);
//      Serial.print("s= ");
//      Serial.println(s);
      text = s;
//      Serial.print("text= ");
//      Serial.println(text);
      
      // Enviar texto UDP mediante Wifi
      Udp.beginPacket(ip.c_str(), port.toInt());
      Udp.write(text.c_str());
      Udp.endPacket(); 
      Serial.println("OK"); 
    }
  }
  // -----------------------------------------------------
  // Si la config. es la adecuada escuchar 
  // constantemente a paquetes UDP entrantes
  //
  int packetSize = Udp.parsePacket();
  if(beginUdp){
    if (packetSize){
      // Recibir paquetes UDP entrantes
//      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      int len = Udp.read(incomingPacket, 255);
      if (len > 0){
        incomingPacket[len] = 0;
      }
      Serial.printf("%s\n", incomingPacket);
      // Enviar una respuesta a la IP que envío el paquete UDP
//      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
//      Udp.write(replyPacket);
//      Udp.endPacket();
    } 
  }

  
}



/**

// Original code

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
const char* ssid = "";              // Nombre de la red a conectar
const char* password = "";          // Contraseña de la red a conectar
unsigned int localUdpPort = 4210;   // Puerto UDP para recepción de paquetes
char incomingPacket[255];           // Buffer para paquetes entrantes
char replyPacket[] = "OK UDP";      // Respuesta al remitente que envío el paquete UDP
String cmd;                         // Comando AT personalizado
String strSsid;                     // Variable temporal para SSID
String strPass;                     // Variable temporal para password
int timeout = 0;                    // Tiempo de espera máximo de conexión a la red
boolean beginUdp = false;           // Bandera para escuchar puerto UDP
boolean isConnected = false;        // Bandera para indicar estado de conexión a red

void setup(){
  Serial.begin(9600);
  Serial.println();
  
}


void loop(){
  if(Serial.available() > 0){     
    cmd = Serial.readStringUntil('\r');             // Obtener comando AT

    if(cmd.equals("AT")){
      Serial.println("OK");  
    }
    // --------------------------------------
    // Comando para esteblecer SSID
    //
    else if(cmd.startsWith("AT+SSID=")){          
      int from = 0;
      // Descomponer cadena para obtener SSID
      for(int i = 0; i <= cmd.length(); i++){
        if(cmd.charAt(i) == '='){
          from = i;
          break;  
        } 
      }
      strSsid = cmd.substring(from + 1);
      // Convertir String a *char
      ssid = strSsid.c_str();
//      Serial.println("OK");
    }
    // ---------------------------------
    // Comando para establecer Password
    //
    else if(cmd.startsWith("AT+PASS=")){          
      int from = 0;
      // Descomponer cadena para obtener Pass
      for(int i = 0; i <= cmd.length(); i++){
        if(cmd.charAt(i) == '='){
          from = i;
          break;  
        } 
      }
      strPass = cmd.substring(from + 1);
      // Convertir String a *char
      password = strPass.c_str();
//      Serial.println("OK");
    }
    // ---------------------------------
    // Comando para obtener SSID
    //
    else if(cmd.equals("AT+SSID?")){  
      if(ssid != ""){
        Serial.println(ssid);
      }
      else{
        Serial.println("ERROR");  
      }
    }
    // ---------------------------------
    // Comando para obtener password
    //
    else if(cmd.equals("AT+PASS?")){  
      if(password != ""){
        Serial.println(password);
      }
      else{
        Serial.println("ERROR");  
      }
    }
    // --------------------------------------
    // Comando para establecer conexión
    //
    else if(cmd.equals("AT+CON")){
      isConnected = false;
      WiFi.begin(ssid, password);
      // Intentar conectar hasta agotar tiempo de espera
      while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print("-");      // Caracter a imprimir mientras se realiza la conexión
        timeout++;
        if(timeout == 20){
          break;
        }
      }
      if(timeout == 100)
        Serial.println("TIMEOUT ERROR");  
      else{
        Udp.begin(localUdpPort);
        beginUdp = true;
        Serial.print(WiFi.localIP().toString());
        Serial.println("OK"); 
      }
    }
    // ---------------------------
    // Comando para obtener IP
    //
    else if(cmd.equals("AT+IP?")){  
      if(isConnected){
        Serial.print("           ");                // Necesario para que la IP sea impresa completa
        Serial.print(WiFi.localIP().toString());
        Serial.println("OK");
      }
      else{
        Serial.println("ERROR");  
      }
    }
    // ---------------------------------
    // Comando para desconectar de red
    //
    else if(cmd.equals("AT+DIS")){
      WiFi.disconnect();
      isConnected = false;
      Serial.println("OK");  
    }
    // ----------------------------------------------
    // Comando para comenzar a recibir paquetes UDP
    //
    else if(cmd.equals("AT+UDP=TRUE")){
      if(isConnected){
        beginUdp = true;
        Serial.println("OK"); 
      }
      else{
        Serial.println("ERROR");  
      }
    }
    // --------------------------------------------
    // Comando para parar de recibir paquetes UDP
    //
    else if(cmd.equals("AT+UDP=FALSE")){
      beginUdp = false;
      Serial.println("OK"); 
    }
    // ---------------------------------------------------------------
    // Comando para parar conocer el estado de recepción de paquetes
    //
    else if(cmd.equals("AT+UDP?")){
      Serial.println(beginUdp); 
    }
  }
  // -----------------------------------------------------
  // Si la config. es la adecuada escuchar 
  // constantemente a paquetes UDP entrantes
  //
  int packetSize = Udp.parsePacket();
  if(beginUdp){
    if (packetSize){
      // Recibir paquetes UDP entrantes
//      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      int len = Udp.read(incomingPacket, 255);
      if (len > 0){
        incomingPacket[len] = 0;
      }
      Serial.printf("%s\n", incomingPacket);
      // Enviar una respuesta a la IP que envío el paquete UDP
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(replyPacket);
      Udp.endPacket();
    } 
  }
  
}



*/
