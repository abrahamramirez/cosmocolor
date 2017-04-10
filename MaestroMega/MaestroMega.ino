#include <SoftwareSerial.h>
#include <StringTokenizer.h>
#include <Wire.h>
#include <Regexp.h>
#include "max6675.h"

// Callback llamado para cada match  
void match_callback  (const char * match,          // matching string (not null-terminated)
                      const unsigned int length,   // length of matching string
                      const MatchState & ms){      // MatchState in use (to get captures)
  char cap [10];   // must be large enough to hold captures
  
  Serial.print ("Matched: ");
//  Serial.write ((byte *) match, length);
  String str485 = String(match);
  Serial.println (str485);
  Serial.println ();
  
  for (byte i = 0; i < ms.level; i++){
    Serial.print ("Capture "); 
    Serial.print (i, DEC);
    Serial.print (" = ");
    ms.GetCapture (cap, i);
    Serial.println (cap); 
  }  
} 

const int ledPin =  13;      // Numero del pin para el Led

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

const String phone = "+527711819555";  
unsigned long count;                // Contador coincidencias al validar reg exp

// Entradas a +12V nativas
const int in12V1 =  22;      
const int in12V2 =  23;      
const int in12V3 =  24;      
const int in12V4 =  25;      
const int in12V5 =  26;      
const int in12V6 =  27;      
const int in12V7 =  28;      

// Entradas a +24V nativas
const int in24V1 =  29;      
const int in24V2 =  30;      
const int in24V3 =  31;      
const int in24V4 =  32;      
const int in24V5 =  33;      
const int in24V6 =  34;      
const int in24V7 =  35;      

// Entradas +5V nativas
const int in5V1 =  36;      
const int in5V2 =  37;      
const int in5V3 =  38;      
const int in5V4 =  39;      
const int in5V5 =  40;    
const int in5V6 =  41;      
const int in5V7 =  42;     

// Entradas ADC nativas
const int inAD1 =  A0;
const int inAD2 =  A1; 
const int inAD3 =  A2; 
const int inAD4 =  A3; 
const int inAD5 =  A4; 
const int inAD6 =  A5; 
const int inAD7 =  A6;   

byte BITS0 = 0;
byte BITS1 = 0;
byte BITS2 = 0;
byte BITS3 = 0;
byte BITS4 = 0;
byte BITS5 = 0;
byte BITS6 = 0;
byte dato = 0;
byte direccion = 0;
byte resultado = 0;
short BIT0 = 0;
short BIT1 = 0;
short BIT2 = 0;
short BIT3 = 0;
short BIT4 = 0;
short BIT5 = 0;
short BIT6 = 0;
short BIT7 = 0;
short BIT8 = 0;
short BIT9 = 0;
short BIT10 = 0;
short BIT11 = 0;
short BIT12 = 0;
short BIT13 = 0;
short BIT14 = 0;
short BIT15 = 0;
short BIT16 = 0;
short BIT17 = 0;
short BIT18 = 0;
short BIT19 = 0;
short BIT20 = 0;

void setup() {
  Wire.begin();               //Iniciamos el bus I2C
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  
  Serial.println("------------------------------");
  Serial.println(" Inicializando Maestro Mega   ");
  Serial.println("------------------------------");
  
  pinMode(ledPin, OUTPUT);    //inicializamos el pin del Led como salida
  pinMode(in12V1, INPUT);
  pinMode(in12V2, INPUT);
  pinMode(in12V3, INPUT);
  pinMode(in12V4, INPUT);
  pinMode(in12V5, INPUT);
  pinMode(in12V6, INPUT);
  pinMode(in12V7, INPUT);
  pinMode(in24V1, INPUT);
  pinMode(in24V2, INPUT);
  pinMode(in24V3, INPUT);
  pinMode(in24V4, INPUT);
  pinMode(in24V5, INPUT);
  pinMode(in24V6, INPUT);
  pinMode(in24V7, INPUT);
  pinMode(in5V1, INPUT);
  pinMode(in5V2, INPUT);
  pinMode(in5V3, INPUT);
  pinMode(in5V4, INPUT);
  pinMode(in5V5, INPUT);
  pinMode(in5V6, INPUT);
  pinMode(in5V7, INPUT);
  pinMode(inAD1, INPUT);
  pinMode(inAD2, INPUT);
  pinMode(inAD3, INPUT);
  pinMode(inAD4, INPUT);
  pinMode(inAD5, INPUT);
  pinMode(inAD6, INPUT);
  pinMode(inAD7, INPUT);
  // Inicializar salidas inicialmente apagadas
  writeI2C(0XA0, 0);
  writeI2C(0XB0, 0);
  
}

String commands = "";
String temp = "";
String temp1 = "";
int bit1 = 0;
int val = 0;
boolean isUsartA = false;
boolean isUsartB = false;

// ---------------------------------------
// Interrupción cuando arriva dato USARTA
// ---------------------------------------
void serialEvent1() {
  commands = Serial1.readStringUntil('#');
  Serial.print("Serial1 data: ");
  Serial.print(commands);
  Serial.println();
  isUsartA = true;
  isUsartB = false;
}

// ---------------------------------------
// Interrupción cuando arriva dato USARTB
// ---------------------------------------
void serialEvent2() {
  commands = Serial2.readStringUntil('#');
  Serial.print("Serial2 data: ");
  Serial.print(commands);
  Serial.println();
}


// ---------------------------------------
// Interrupción cuando arriva dato USB
// ---------------------------------------
void serialEvent3() {
  commands = Serial3.readStringUntil('\r');
  Serial.print("Serial3 data: ");
  Serial.print(commands);
  Serial.println();
}


void loop() {
  // --------------------------------
  // Comandos para alterar salidas
  // --------------------------------
  if(commands.startsWith("@A")){
    temp = commands.substring(2);
    dato = temp.toInt();
    writeI2C(0XA0, dato);
    delay(100);
  }
  else if(commands.startsWith("@B")){
    temp = commands.substring(2);
    dato = temp.toInt();
    direccion = 0XB0;
    writeI2C(direccion, dato);
    delay(100);
  }
  // --------------------------------
  // Comandos para leer salidas
  // --------------------------------
  else if(commands.equals("@VA")){
    readI2C(0xA0);
    delay(100);
    Serial1.println(resultado);
    Serial2.println(resultado);
    Serial.println(resultado);
  }
  else if(commands.equals("@VB")){
    readI2C(0xB0);
    delay(100);
    Serial1.println(resultado);
    Serial2.println(resultado);
    Serial.println(resultado);
  }
  // --------------------------------
  // Comandos para alterar bits
  // --------------------------------
  else if(commands.startsWith("@S")){
    temp = commands.substring(2, 4);
    bit1 = temp.toInt();
    temp1 = commands.substring(4);
    val = temp1.toInt();
    setI2CBit(bit1,  val);
    delay(100);
  }
  // ------------------------------------------
  // Comandos para obtener el valor de un bit
  // ------------------------------------------
  else if(commands.startsWith("@G")){
    temp = commands.substring(2, 4);
    bit1 = temp.toInt();
    Serial1.println(getBit(bit1));
    Serial2.println(getBit(bit1));
    Serial.println(getBit(bit1));
    delay(100);
  }
  // -------------------------------------------------
  // Comandos para obtener todas las entradas nativas
  // 22 hasta 42
  // ------------------------------------------------
  else if(commands.startsWith("@I")){
    int outputs[22];
    char buffer[22];
    for(int i = 22; i <= 42; i++){
      int j = i - 22;
      outputs[j] = getBit(i);
    }
    sprintf(buffer, "%d%d%d%d%d%d%d %d%d%d%d%d%d%d %d%d%d%d%d%d%d", 
            outputs[0],outputs[1],outputs[2],outputs[3],
            outputs[4],outputs[5],outputs[6],outputs[7],
            outputs[8],outputs[9],outputs[10],outputs[11],
            outputs[12],outputs[13],outputs[14],outputs[15],
            outputs[16],outputs[17],outputs[18],outputs[19],
            outputs[20]
            );
    Serial1.println(buffer);
    Serial2.println(buffer);
    Serial.println(buffer);
    delay(100);
  }
  
  translateCommand(commands);
  delay(300);
  
  commands = "";
  delay(300);
}


/**
 * Traduce los comandos de alto nivel para operaciones de bajo nivel
 * los cuales pueden arrivar mediante cualquier puerto serial
 * 
 * String cmd. Método de alto nivel con argumentos.
 * 
 **/
void translateCommand(String cmd){
  String method = "";     // Método de alto nivel procedente de cualquier medio UART
  String smsNum = "";
  String smsText = "";
  String output = "";
  String str = "";
  String sTemp = "";
  String sTemp2 = ""; 
  String endChar = "#";
  String inputA = "@VA#";
  String inputB = "@VB#";
  int temp = 0;  
  int index = 0;          // Índice del array de comandos de bajo nivel
  int val = 0;
  int val2 = 0;
  String s = "";
  String mIndex = "";      
  String mVal = "";
  String sBit = "";
  String sVal = "";
  int iIndex = 0;
  int iVal = 0;

  if(!cmd.equals("")){
    char* cCmd = cmd.c_str();
    MatchState ms (cCmd);
    // -----------------------------------------
    // Validar función: setAllOut(16 bits)
    // -----------------------------------------
    count = ms.GlobalMatch("setAllOut%([0-1]+%)", match_callback); 
    if(count == 1){    
      // Extraer argumentos del método
      method = String(cCmd);
      method.trim();                    
      method.replace("\r", "");
      method.replace("\n", "");
      method.replace("setAllOut(", "");
      method.replace(")", "");
      
      sTemp = method.substring(0, 8);
      sTemp2 = method.substring(8, 16);

      // Convertir binario a decimal
      val = strtol(sTemp.c_str(), NULL, 2);  
      val2 = strtol(sTemp2.c_str(), NULL, 2); 

      writeI2C(0XA0, val);
      delay(100);
      writeI2C(0XB0, val2);
      delay(100);
      Serial3.println("OK");
    }

    // -----------------------------------------
    // Validar función: getOut(int bit)
    // -----------------------------------------
    count = ms.GlobalMatch("getOut%([0-9]+%)", match_callback);
    if(count == 1){    
      // Extraer argumentos del método 
      method = String(cCmd);
      method.trim();                    
      method.replace("\r", "");
      method.replace("\n", "");
      method.replace("getOut(", "");
      method.replace(")", "");
        
      Serial3.println(getBit(getSwapBit(method.toInt())));
      delay(100);
    }

    // -----------------------------------------
    // Validar función: getIn(int bit)
    // -----------------------------------------
    count = ms.GlobalMatch("getIn%([0-9]+%)", match_callback);
    if(count == 1){    
      // Extraer argumentos del método 
      method = String(cCmd);
      method.trim();                    
      method.replace("\r", "");
      method.replace("\n", "");
      method.replace("getIn(", "");
      method.replace(")", "");

      Serial3.println(getBit(method.toInt()));
      delay(100);
    }
    
    // ------------------------------------------------
    // Validar función: setOut(int salida, int valor)
    // ------------------------------------------------
    count = ms.GlobalMatch("setOut%([0-9]+,[0-1]+%)", match_callback); 
    if(count == 1){    
      // Extraer argumentos del método 
      method = String(cCmd);
      method.trim();                    
      method.replace("\r", "");
      method.replace("\n", "");
      method.replace("setOut(", "");
      method.replace(")", "");
      
      for(int i = 0; i <= method.length(); i++){
          char ch = method.charAt(i);
          if(ch != ','){
            sBit.concat(ch);
          }
          else  
            break;
      }
      sVal = method.substring(method.length() - 1, method.length());
   
      // Añadir comandos de bajo nivel a array
      int bit1 = sBit.toInt();
      int val = sVal.toInt();
      setI2CBit(bit1,  val);
      delay(100);
    }

    // ----------------------------------------
    // Validar función: getAllOut()
    // ----------------------------------------
    count = ms.GlobalMatch("getAllOut%(%)", match_callback);   
    if(count == 1){
      int outputs[17];
      char buffer[17];
      for(int i = 0; i <= 15; i++){
        outputs[i] = getBit(i);
      } 
      sprintf(buffer, "%d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d ", 
              outputs[7],outputs[6],outputs[5],outputs[4],
              outputs[3],outputs[2],outputs[1],outputs[0],
              outputs[15],outputs[14],outputs[13],outputs[12],
              outputs[11],outputs[10],outputs[9],outputs[8]
              );
      Serial3.println(buffer);
    }

    // ----------------------------------------
    // Validar función: getAllIn()
    // ----------------------------------------
    count = ms.GlobalMatch("getAllIn%(%)", match_callback);   
    if(count == 1){
      index = 0;
      int outputs[22];
      char buffer[22];
      for(int i = 22; i <= 42; i++){
        int j = i - 22;
        outputs[j] = getBit(i);
      }
      sprintf(buffer, "%d%d%d%d%d%d%d %d%d%d%d%d%d%d %d%d%d%d%d%d%d", 
              outputs[0],outputs[1],outputs[2],outputs[3],
              outputs[4],outputs[5],outputs[6],outputs[7],
              outputs[8],outputs[9],outputs[10],outputs[11],
              outputs[12],outputs[13],outputs[14],outputs[15],
              outputs[16],outputs[17],outputs[18],outputs[19],
              outputs[20]
              );
      Serial3.println(buffer);
      delay(100);
    }

    
  } 
}


/**
 * Retorna el bit equivalente para ajustar las salidas, p.e.
 * el bit 7 es la salida 0.
 * 
 * int digits. Número de dígitos de la cifra final.
 **/
int getSwapBit(int bit1){
  int res = 0;
  switch(bit1){
    case 0:  res = 7; break;
    case 1:  res = 6; break;
    case 2:  res = 5; break;
    case 3:  res = 4; break;
    case 4:  res = 3; break;
    case 5:  res = 2; break;
    case 6:  res = 1; break;
    case 7:  res = 0; break;
    
    case 8:  res = 15; break;
    case 9:  res = 14; break;
    case 10: res = 13; break;
    case 11: res = 12; break;
    case 12: res = 11; break;
    case 13: res = 10; break;
    case 14: res = 9; break;
    case 15: res = 8;  break;
  }
  return res;
}


/**
 * Completa tantos ceros a la izquierda de un string que contiene
 * un número como se indique.
 * 
 * String sNum. String que contiene un número
 * int digits. Número de dígitos de la cifra final.
 **/
String completeZeros(String sNum, int digits){
  int num = sNum.toInt();
  String s = "";
  String t = "";
  if(digits == 2){
    if(num < 10)  
      t = "0" + sNum;
    else 
      t = sNum;
  }
  else if(digits == 3){
    if(num < 10)
      t = "00" + sNum;
    else if(num < 100)
      t = "0" + sNum;
    else 
      t = sNum;
  }
  return t;
}

/**
 * Realiza un ciclo de lectura de una entrada analógica tantas
 * veces como se indique
 * 
 * int input. Entrada analógica
 * int iterations. Número de iteraciones en el ciclo.
 **/
int readSensor(int input, int iterations){
  int val = 0;
  for(int i = 0; i <= iterations; i++){
    val += analogRead(input);
    delay(10);
  }
  return (val / iterations);
}

/**
 * Establece un valor 0 o 1 en un bit de salida específico.
 * 
 * int bit. Número de salida a modificar.
 * int val. Valor a establecer en la salida, sólo acpeta 0 o 1.
 **/
void setI2CBit(int bit1,  int val){
  int port = 0;
  
  // Micro A: salidas 0-7
  if(bit1 >= 0 && bit1 <= 7){   
    readI2C(0xA0);          // Leer valor actual del micro A
    port = resultado;
    bitWrite(port, bit1, val);
    delay(100);
    writeI2C(0xA0, port);
  }
  // Micro B: salidas 8-16
  else if(bit1 >= 8 && bit1 <= 16){ 
    readI2C(0xB0);          // Leer valor actual del micro B
    port = resultado;
    bitWrite(port, bit1 - 8, val);
    delay(100);
    writeI2C(0xB0, port);
  }
}


/**
 * Retorna en estado de la entrada o salida indicada por el 
 * número de bit
 * 
 * int bit. Número de salida a modificar.
 **/
int getBit(int bit1){
  int port = 0;
  int resp = 0;
  
  // Micro A: salidas 0-7
  if(bit1 >= 0 && bit1 <= 7){ 
    readI2C(0xA0);          // Leer valor actual del micro A
    port = resultado;
    resp = bitRead(port, bit1);
    delay(100);
  }
  
  // Micro B: salidas 8-16
  else if(bit1 >= 8 && bit1 <= 16){
    readI2C(0xB0);          // Leer valor actual del micro B
    port = resultado;
    resp = bitRead(port, bit1 - 8);
    delay(100);
  }

  // Micro Mtro: entradas +12V: 22-28. || Entradas negadas!!!
  else if(bit1 >= 22 && bit1 <= 28){
    resp = !digitalRead(bit1);
    delay(100);
  }
  
  // Micro Mtro: entradas +24V: 29-35
  // Micro Mtro: entradas +5V:  36-42
  else if(bit1 >= 22 && bit1 <= 42){
    resp = digitalRead(bit1);
    delay(100);
  }
  return resp;
}


/**
 * Habilita las entradas indicadas
 * 
 * char* inputs. String separado por comas indicando las
 *               entradas p.e. "1,3,10,A0"
 **/
void enableInputs(char* inputs){
  StringTokenizer tokens(inputs, ",");
  while(tokens.hasNext()){
    int input = tokens.nextToken().toInt();
    pinMode(input, INPUT);
    Serial.println("Habilitadas");
    Serial.print(input); 
    Serial.print(", ");
  }
}


//esta función  puede variar de acuerdo a su necesidad
void SENSOR12VON(void)
{
  if (BIT0 == 1) {
    Serial1.println("SEN00@1#"), Serial2.println("SEN00@1#"), delay(100);
  }
  if (BIT1 == 1) {
    Serial1.println("SEN00@2#"), Serial2.println("SEN00@2#"), delay(100);
  }
  if (BIT2 == 1) {
    Serial1.println("SEN00@3#"), Serial2.println("SEN00@3#"), delay(100);
  }
  if (BIT3 == 1) {
    Serial1.println("SEN00@4#"), Serial2.println("SEN00@4#"), delay(100);
  }
  if (BIT4 == 1) {
    Serial1.println("SEN00@5#"), Serial2.println("SEN00@5#"), delay(100);
  }
  if (BIT5 == 1) {
    Serial1.println("SEN00@6#"), Serial2.println("SEN00@6#"), delay(100);
  }
  if (BIT6 == 1) {
    Serial1.println("SEN00@7#"), Serial2.println("SEN00@7#"), delay(100);
  }
  delay(100);
}
void SENSOR24VON(void)
{
  if (BIT7 == 1) {
    Serial1.println("SEN00@8#"), Serial2.println("SEN00@8#"), delay(100);
  }
  if (BIT8 == 1) {
    Serial1.println("SEN00@9#"), Serial2.println("SEN00@9#"), delay(100);
  }
  if (BIT9 == 1) {
    Serial1.println("SEN00@10#"), Serial2.println("SEN00@10#"), delay(100);
  }
  if (BIT10 == 1) {
    Serial1.println("SEN00@11#"), Serial2.println("SEN00@11#"), delay(100);
  }
  if (BIT11 == 1) {
    Serial1.println("SEN00@12#"), Serial2.println("SEN00@12#"), delay(100);
  }
  if (BIT12 == 1) {
    Serial1.println("SEN00@13#"), Serial2.println("SEN00@13#"), delay(100);
  }
  if (BIT13 == 1) {
    Serial1.println("SEN00@14#"), Serial2.println("SEN00@14#"), delay(100);
  }
  delay(1000);
}
void SENSORTTLON(void)
{
  if (BIT14 == 1) {
    Serial1.println("SEN00@15#"), Serial2.println("SEN00@15#");
  }
  if (BIT15 == 1) {
    Serial1.println("SEN00@16#"), Serial2.println("SEN00@16#");
  }
  if (BIT16 == 1) {
    Serial1.println("SEN00@17#"), Serial2.println("SEN00@17#");
  }
  if (BIT17 == 1) {
    Serial1.println("SEN00@18#"), Serial2.println("SEN00@18#");
  }
  if (BIT18 == 1) {
    Serial1.println("SEN00@19#"), Serial2.println("SEN00@19#");
  }
  if (BIT19 == 1) {
    Serial1.println("SEN00@20#"), Serial2.println("SEN00@20#");
  }
  if (BIT20 == 1) {
    Serial1.println("SEN00@21#"), Serial2.println("SEN00@21#");
  }
  delay(1000);
}
void SENSORADCON(void)
{
  if (BITS0 > 1000) {
    Serial1.println("SEN00@22#"), Serial2.println("SEN00@22#");
  }
  if (BITS1 > 1000) {
    Serial1.println("SEN00@23#"), Serial2.println("SEN00@23#");
  }
  if (BITS2 > 1000) {
    Serial1.println("SEN00@24#"), Serial2.println("SEN00@24#");
  }
  if (BITS3 > 1000) {
    Serial1.println("SEN00@25#"), Serial2.println("SEN00@25#");
  }
  if (BITS4 > 1000) {
    Serial1.println("SEN00@26#"), Serial2.println("SEN00@26#");
  }
  if (BITS5 > 1000) {
    Serial1.println("SEN00@27#"), Serial2.println("SEN00@27#");
  }
  if (BITS6 > 1000) {
    Serial1.println("SEN00@28#"), Serial2.println("SEN00@28#");
  }
  delay(1000);
}
void ENTRADASANALOGICAS(void)
{
  BITS0 = analogRead(A0);
  BITS1 = analogRead(A1);
  BITS2 = analogRead(A2);
  BITS3 = analogRead(A3);
  BITS4 = analogRead(A4);
  BITS5 = analogRead(A5);
  BITS6 = analogRead(A6);
  delay(1000);
}
void ENTRADASTTL(void)
{
  BIT14 = digitalRead(in5V1);
  BIT15 = digitalRead(in5V2);
  BIT16 = digitalRead(in5V3);
  BIT17 = digitalRead(in5V4);
  BIT18 = digitalRead(in5V5);
  BIT19 = digitalRead(in5V6);
  BIT20 = digitalRead(in5V7);
  delay(1000);
}
void ENTRADAS12(void)
{
  BIT0 = digitalRead(in12V1); //Lectura de sensores DIGITALES
  BIT1 = digitalRead(in12V2);
  BIT2 = digitalRead(in12V3);
  BIT3 = digitalRead(in12V4);
  BIT4 = digitalRead(in12V5);
  BIT5 = digitalRead(in12V6);
  BIT6 = digitalRead(in12V7);
  delay(1000);
}
void ENTRADAS24(void)
{
  BIT7 = digitalRead(in24V1);
  BIT8 = digitalRead(in24V2);
  BIT9 = digitalRead(in24V3);
  BIT10 = digitalRead(in24V4);
  BIT11 = digitalRead(in24V5);
  BIT12 = digitalRead(in24V6);
  BIT13 = digitalRead(in24V7);
  delay(1000);
}
void writeI2C(int direccion, int dato)
{
  Wire.beginTransmission(direccion);
  Wire.write(dato);
  Wire.endTransmission();
  delay(100);
  Serial1.print("OK\r");
  Serial2.print("OK\r");
  
}
void readI2C (int direccion)
{
  Wire.requestFrom(direccion, 1);    // Pedimos 1 bytes al slave
  while (Wire.available()){   // Mientras tengamos datos.
  
    resultado = Wire.read(); // Leemos el resultado
//    Serial.print("resp I2C: ");
//    Serial.print(resultado);
//    Serial.println();
  }
}


// **********************************
// In loop
// **********************************
//
//  while(Serial1.available() > 0) {
//    if(Serial1.read() == '@'){       // Si recibimos el inicio de trama
//      char micro = Serial1.read();    // leemos el siguiente caracter
//      Serial.print(micro);
//      switch (micro){
//        case 'A':
//          direccion = 0XA0;
//          dato = Serial1.parseInt(); //recibimos valor numérico
//          Serial.println(dato);
//          break;
//        case 'B':
//          direccion = 0xB0;
//          dato = Serial1.parseInt();  //recibimos valor numérico
//          Serial.println(dato);
//          break;
//        case 'V':
//          char i = Serial1.read();    //leemos el siguiente caracter
//          if (i == 'A') {
//            readI2C(0xA0);
//            Serial1.println(resultado);
//          }
//          if (i == 'B') {
//            readI2C(0xB0);
//            Serial1.println(resultado);
//          }
//          break;
//      }
//      if (Serial1.read() == '#'){     //Si el fin de trama es el correcto
//        writeI2C(direccion, dato);
//      }
//    }
//  }
//
//  while(Serial2.available() > 0) {
//    if(Serial2.read() == '@'){ //Si recibimos el inicio de trama
//      char micro = Serial2.read(); //leemos el siguiente caracter
//      Serial.print(micro);
//      switch (micro){
//        case 'A':
//          direccion = 0XA0;
//          dato = Serial2.parseInt(); //recibimos valor numérico
//          Serial.println(dato);
//          break;
//        case 'B':
//          direccion = 0xB0;
//          dato = Serial2.parseInt(); //recibimos valor numérico
//          Serial.println(dato);
//          break;
//        case 'V':
//          char I = Serial2.read(); //leemos el siguiente caracter
//          if (I == 'A') {
//            readI2C (0xA0); 
//            
//            Serial2.println(resultado);
//            delay(100);
//          }
//          if (I == 'B') {
//            readI2C (0xB0);
//            
//            Serial2.println(resultado);
//            delay(100);
//          }
//          break;
//      }
//      if (Serial2.read() == '#'){ //Si el fin de trama es el correcto
//        writeI2C(direccion, dato);
//      }
//    }
//  }//SERIAL 2
//  ENTRADAS12();
//  //ENTRADAS24();
//  // ENTRADASTTL();
//  // ENTRADASANALOGICAS();
//  // SENSOR24VON();
//  SENSOR12VON();
//  //  SENSORTTLON();
//  //  SENSORADCON();
//
//  delay(10);


