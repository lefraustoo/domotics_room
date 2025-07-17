#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"

// --- Configuración del Sensor DHT ---
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Variables para medir el tiempo ---
float temperaturaAnterior;
unsigned long tiempoAnterior; // Usamos 'unsigned long' para millis()

// --- Variables para mediciones en tiempo real ---
float tempActual;
float humedadActual;

// Bandera para controlar la primera lectura
bool primeraLectura = true;

void setup()
{
  Serial.begin(9600);
  system("clear"); // Limpia la consola al inicio
  Serial.println("Monitoreo de humedad y cambios de temperatura con DHT11");

  dht.begin();

  delay(2000); // Esperamos un poco para que el sensor se estabilice

  // --- Tomamos la primera lectura como punto de referencia inicial ---
  temperaturaAnterior = dht.readTemperature();
  tiempoAnterior = millis(); // Guardamos el tiempo de la primera lectura

  // Verificamos que la primera lectura sea válida
  if (isnan(temperaturaAnterior))
  {
    Serial.println("Error al leer el sensor. Reiniciando...");
  }

  Serial.print("Temperatura inicial: ");
  Serial.print(temperaturaAnterior);
  Serial.println(" C. Esperando un cambio de 1 grado...");
}

void loop()
{
  // Hacemos una lectura cada 2 segundos para no sobrecargar el sensor
  delay(2000);

  tempActual = dht.readTemperature();
  humedadActual = dht.readHumidity();

  // Ignoramos la iteración si la lectura no es válida
  if (isnan(tempActual))
  {
    return;
  }

  // --- Comprobamos si la temperatura ha cambiado al menos 1 grado ---
  // La función abs() calcula el valor absoluto de la diferencia
  if (abs(tempActual - temperaturaAnterior) >= 1.0)
  {

    // Calculamos el tiempo transcurrido
    unsigned long tiempoActual = millis();
    unsigned long tiempoTranscurrido = tiempoActual - tiempoAnterior;
    float segundosTranscurridos = tiempoTranscurrido / 1000.0; // Convertimos de ms a s

    // --- Imprimimos el resultado ---
    Serial.print("Cambio detectado: de ");
    Serial.print(temperaturaAnterior);
    Serial.print(" C a ");
    Serial.print(tempActual);
    Serial.print(" C. | Tiempo transcurrido: ");
    Serial.print(segundosTranscurridos, 1); // Imprime con 1 decimal
    Serial.println(" segundos.");

    // --- Actualizamos los valores para el próximo ciclo de medición ---
    temperaturaAnterior = tempActual;
    tiempoAnterior = tiempoActual;
  }

  // Comprueba si alguna de las lecturas falló (retornan NaN - Not a Number)
  if (isnan(humedadActual) || isnan(tempActual))
  {
    Serial.println("Error al leer del sensor DHT!");
    return; // Sale de la función loop y la inicia de nuevo
  }

  // Si las lecturas son exitosas, las imprime en el Monitor Serie
  Serial.print("Humedad: ");
  Serial.print(humedadActual);
  Serial.print("%  |  ");
  Serial.print("Temperatura: ");
  Serial.print(tempActual);
  Serial.println(" °C");
}