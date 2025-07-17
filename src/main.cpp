#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"

// Define el pin donde está conectado el sensor DHT11
#define DHTPIN 2

// Define el tipo de sensor que estás utilizando (DHT11)
#define DHTTYPE DHT11

// Inicializa el sensor DHT con los pines y el tipo definidos
DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  // Inicia la comunicación serial a 9600 bps para depuración
  Serial.begin(9600);
  Serial.println("Prueba del sensor DHT11");

  // Inicia el sensor
  dht.begin();
}

void loop()
{
  // Esperar un par de segundos. El DHT11 no debe ser leído más de una vez cada 2 segundos.
  delay(2000); // 2 segundos

  // Leer la humedad relativa
  float h = dht.readHumidity();
  // Leer la temperatura en grados Celsius (es la lectura por defecto)
  float t = dht.readTemperature();

  // Comprobar si alguna de las lecturas falló (se retorna NaN - Not a Number)
  if (isnan(h) || isnan(t))
  {
    Serial.println("Error al leer del sensor DHT!");
    return; // Sale de la función loop y la inicia de nuevo
  }

  // Si las lecturas son exitosas, las imprime en el Monitor Serie
  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print("%  |  ");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" °C");
}