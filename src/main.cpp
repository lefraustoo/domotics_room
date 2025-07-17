#include <Arduino.h>
#include "DHT.h"

// --- Configuración del Sensor DHT ---
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Variables para medir el tiempo ---
float temperaturaAnterior;
unsigned long tiempoAnterior; // Usamos 'unsigned long' para millis()

// Bandera para controlar la primera lectura
bool primeraLectura = true;

void setup()
{
  Serial.begin(9600);
  dht.begin();

  // Esperamos un poco para que el sensor se estabilice
  delay(2000);

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

  float tempActual = dht.readTemperature();

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
}