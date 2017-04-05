
#include <Wire.h>
String s;
int a = 15;
const int ledPin =  13;      // Numero del pin para el Led
byte rec = 0;
void setup()
{
  Wire.begin(0xA0);                // Nos ponemos la direccion 0xA0
  Wire.onRequest(requestEvent); // Activamos evento de peticion
  Wire.onReceive(receiveEvent);//Activamos evento de lectura.
  pinMode(ledPin, OUTPUT);//inicializamos el pin del Led como salida
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  setBits(0X00);
}

void loop()
{
  setBits(rec);
  delay(100);
}


//Interrupcion de dato recibido i2c
void receiveEvent(int howMany)
{
  while ( Wire.available() > 0) // Leemos todo el buffer
  {
    rec = Wire.read(); // almacenamos el byte
    funcion(rec);
  }
}
//Evento de peticion se activa cuando un master nos pide que le enviemos algun dato
void requestEvent()
{
  Wire.write(rec); // Enviamos el resultado

}
/**
   Establecer los bits correspondientes dado un número entero de 8 bits

   int num. Número de 8 bits a establecer en pines 2 hasta 9
   int bits. Tamaño del número binario.
 **/
void setBits(int num) {
  String s = getBinary(num, 8);
  digitalWrite (2, getBit(s.charAt(0)));
  digitalWrite (3, getBit(s.charAt(1)));
  digitalWrite (4, getBit(s.charAt(2)));
  digitalWrite (5, getBit(s.charAt(3)));
  digitalWrite (6, getBit(s.charAt(4)));
  digitalWrite (7, getBit(s.charAt(5)));
  digitalWrite (8, getBit(s.charAt(6)));
  digitalWrite (9, getBit(s.charAt(7)));
}

/**
   Convierte un entero en su equivalente binario con el tamaño
   de bits indicado.

   int number. Número entero a convertir
   int bits. Tamaño del número binario.

   Retorna. String, representación binaria del número entero.
 **/
String getBinary(int number, int bits) {
  String temp = String(number, BIN);;
  String s = "";
  if (temp.length() <= bits) {
    for (int i = 0; i < bits - temp.length(); i++) {
      s.concat("0");
    }
    s.concat(temp);
  }
  else {
    s = String(number, BIN);
  }
  return s;
}

int getBit(char c) {
  if (c == '0') return 0;
  else if (c == '1') return 1;
}
void funcion(int dato)
{
  if (dato > 50)
    digitalWrite(ledPin, HIGH);
  else
    digitalWrite(ledPin, LOW);
}
