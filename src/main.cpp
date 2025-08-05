/*
 * ===================================================================================
 * PROYECTO: CEREBRO DE HABITACIÓN INTELIGENTE
 * ===================================================================================
 * Autor: Leonardo Frausto (con ayuda de Gemini)
 * Fecha: 05 de agosto de 2025
 * Placa: ESP32-C3 Super Mini
 *
 * Descripción:
 * Este código unifica un sistema de monitoreo y control para una habitación.
 * Integra las siguientes funciones:
 * 1.  Monitoreo Ambiental: Lee temperatura y humedad con un sensor DHT22.
 * 2.  Calidad de Aire: Mide la concentración de gases con un sensor MQ-135.
 * 3.  Control de Acceso: Utiliza un lector RFID MFRC522 para registrar entradas.
 * 4.  Control de Climatización: Envía señales infrarrojas (IR) para controlar un AC.
 * 5.  Interfaz Web: Crea un servidor web para visualizar datos y controlar el AC
 * desde cualquier dispositivo en la misma red Wi-Fi.
 * ===================================================================================
 */

// --- INCLUSIÓN DE LIBRERÍAS ---
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ArduinoJson.h>

// --- CONFIGURACIÓN DE PINES (Ajusta según tus conexiones) ---
#define DHT_PIN 4      // Pin de datos del DHT22
#define DHT_TYPE DHT22 // Define el tipo de sensor DHT
#define MQ135_PIN A0   // Pin analógico para el MQ-135 (GPIO 1 en ESP32-C3)
#define RFID_SDA_PIN 8 // SDA/SS (Slave Select) para el MFRC522
#define RFID_RST_PIN 9 // Reset para el MFRC522
#define IR_LED_PIN 5   // Pin para el LED emisor de IR

// --- CONFIGURACIÓN DE WI-FI ---
const char *ssid = "TP-Link_05E0"; // << REEMPLAZA ESTO
const char *password = "21798825"; // << REEMPLAZA ESTO

// --- CONFIGURACIÓN DE CONTROL DE ACCESO ---
String uidAutorizado = "XX XX XX XX"; // << REEMPLAZA con el UID de tu tarjeta RFID
String ultimoAcceso = "Nadie ha entrado";

// --- CONFIGURACIÓN DE CÓDIGOS INFRARROJOS (IR) ---
// ¡IMPORTANTE! Debes capturar estos códigos de tu control remoto Midea.
const uint64_t MIDEA_AC_ON_CODE = 0xB24D807F;  // << CÓDIGO DE EJEMPLO
const uint64_t MIDEA_AC_OFF_CODE = 0xB24D40BF; // << CÓDIGO DE EJEMPLO

// --- INICIALIZACIÓN DE OBJETOS ---
DHT dht(DHT_PIN, DHT_TYPE);
MFRC522 mfrc522(RFID_SDA_PIN, RFID_RST_PIN);
IRsend irsend(IR_LED_PIN);
WebServer server(80);

// --- VARIABLES GLOBALES PARA ALMACENAR DATOS ---
float temperatura = 0.0;
float humedad = 0.0;
int calidadAire = 0;

// --- DECLARACIÓN DE FUNCIONES (Buena práctica en C++) ---
void handleRoot();
void handleAcOn();
void handleAcOff();
void handleGetData();
void leerSensores();
void revisarRFID();
void enviarCodigoIR(uint64_t code);

// --- FUNCIÓN DE CONFIGURACIÓN (Se ejecuta una sola vez) ---
void setup()
{
  Serial.begin(115200);
  delay(1000); // Delay para estabilizar la conexión serial USB

  dht.begin();
  SPI.begin();
  mfrc522.PCD_Init();
  irsend.begin();

  Serial.println("Sistema Domótico Iniciado.");
  Serial.print("Conectando a la red Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n¡Conexión Wi-Fi exitosa!");
  Serial.print("Dirección IP del ESP32: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/ac_on", handleAcOn);
  server.on("/ac_off", handleAcOff);
  server.on("/get_data", handleGetData);

  server.begin();
  Serial.println("Servidor web iniciado.");
}

// --- BUCLE PRINCIPAL (Se ejecuta continuamente) ---
void loop()
{
  server.handleClient();

  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead > 2000)
  {
    leerSensores();
    lastSensorRead = millis();
  }

  revisarRFID();
}

// --- DEFINICIÓN DE FUNCIONES ---

void leerSensores()
{
  humedad = dht.readHumidity();
  temperatura = dht.readTemperature();

  if (isnan(humedad) || isnan(temperatura))
  {
    Serial.println("Error al leer del sensor DHT!");
    return;
  }
  calidadAire = analogRead(MQ135_PIN);
}

void revisarRFID()
{
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      uid += (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    uid.trim();

    Serial.print("Tarjeta detectada con UID: ");
    Serial.println(uid);

    if (uid.equals(uidAutorizado))
    {
      Serial.println("Acceso Concedido.");
      ultimoAcceso = "Acceso Concedido: " + uid;
    }
    else
    {
      Serial.println("Acceso Denegado.");
      ultimoAcceso = "Acceso Denegado: " + uid;
    }
    mfrc522.PICC_HaltA();
  }
}

void enviarCodigoIR(uint64_t code)
{
  irsend.sendMidea(code);
  Serial.printf("Enviando código IR: 0x%llX\n", code);
}

// << NUEVA FUNCIÓN AÑADIDA >>
void handleNotFound()
{
  // Envía una respuesta 404 (Not Found) al navegador.
  // Esto es más limpio y profesional que dejar que el servidor dé un error.
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleRoot()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang='es'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>control de habitacion</title>
    <style>
        body { font-family: 'Segoe UI', sans-serif; background-color: #1e1e1e; color: #d4d4d4; display: flex; justify-content: center; align-items: center; min-height: 100vh; margin: 0; }
        .container { background-color: #2d2d2d; padding: 25px; border-radius: 12px; box-shadow: 0 10px 25px rgba(0,0,0,0.5); width: 90%; max-width: 500px; }
        h1 { color: #569cd6; text-align: center; border-bottom: 2px solid #569cd6; padding-bottom: 10px; }
        .card-container { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; margin: 20px 0; }
        .card { background-color: #3c3c3c; padding: 20px; border-radius: 8px; text-align: center; }
        .card h2 { margin-top: 0; font-size: 1em; color: #9cdcfe; }
        .card p { font-size: 2em; margin: 0; font-weight: bold; color: #ce9178; }
        .controls, .access { margin-top: 20px; }
        h3 { color: #4ec9b0; }
        .btn-container { display: flex; justify-content: center; gap: 15px; }
        .btn { background-color: #4ec9b0; color: #1e1e1e; border: none; padding: 12px 25px; border-radius: 5px; font-size: 1em; font-weight: bold; cursor: pointer; transition: background-color 0.3s; }
        .btn.off { background-color: #ce9178; }
        .btn:hover { opacity: 0.9; }
        #access-log { background-color: #3c3c3c; padding: 10px; border-radius: 5px; font-family: 'Courier New', monospace; font-size: 0.9em; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>domotics room v1</h1>
        <div class='card-container'>
            <div class='card'>
                <h2>Temperatura</h2>
                <p><span id='temp'>--</span> &deg;C</p>
            </div>
            <div class='card'>
                <h2>Humedad</h2>
                <p><span id='hum'>--</span> %</p>
            </div>
            <div class='card'>
                <h2>Calidad del Aire</h2>
                <p><span id='air'>--</span></p>
            </div>
        </div>
        <div class='controls'>
            <h3>Control del Aire Acondicionado</h3>
            <div class='btn-container'>
                <button class='btn' onclick="fetch('/ac_on')">ENCENDER</button>
                <button class='btn off' onclick="fetch('/ac_off')">APAGAR</button>
            </div>
        </div>
        <div class='access'>
            <h3>&Uacute;ltimo Acceso RFID</h3>
            <p id='access-log'>--</p>
        </div>
    </div>

    <script>
        function updateData() {
            fetch('/get_data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temp').innerText = data.temperatura.toFixed(1);
                    document.getElementById('hum').innerText = data.humedad.toFixed(1);
                    document.getElementById('air').innerText = data.calidadAire;
                    document.getElementById('access-log').innerText = data.ultimoAcceso;
                })
                .catch(error => console.error('Error al actualizar datos:', error));
        }
        setInterval(updateData, 3000);
        window.onload = updateData;
    </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleAcOn()
{
  enviarCodigoIR(MIDEA_AC_ON_CODE);
  server.send(200, "text/plain", "Comando ON enviado");
}

void handleAcOff()
{
  enviarCodigoIR(MIDEA_AC_OFF_CODE);
  server.send(200, "text/plain", "Comando OFF enviado");
}

void handleGetData()
{
  String json = "{";
  json += "\"temperatura\":" + String(temperatura) + ",";
  json += "\"humedad\":" + String(humedad) + ",";
  json += "\"calidadAire\":" + String(calidadAire) + ",";
  json += "\"ultimoAcceso\":\"" + ultimoAcceso + "\"";
  json += "}";
  server.send(200, "application/json", json);
}
