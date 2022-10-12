/* 
  Orto Prefetto Bio-Diverso Kit
  Future Education Modena 2022

  Monitora: 
  - Umidità/Temperatura (Shield MKR Env)
  - Illuminazione (Shield MKR Env)
  - Umidità del terreno (Grove Sensore Capacitivo analogico)
  - Temperatura del terreno (DS18B20 + connetore Grove)
*/

///// NON SPOSTARE QUESTE DEFINIZIONI  /////
//*******************************************
#define SECRET_SSID "FEM_WiFi"
//#define SECRET_PASS "wifipassword"
#define SECRET_PASS "0h4orXc@yS3do"
#define CHIAVE_CLOUD "FEMOrtoPerfetto_token"

char dboard_server[] = "demo.thingsboard.io"; // Indirizzo IP/Internet del Dashboard Server
int dboard_port = 80;                         // Porta IP del server

//Variabili
float temp_aria = 0;
float umid_aria = 0;
float luminosita = 0;
int umid_terreno1 = 0;
int umid_terreno2 = 0;
int umid_terreno3 = 0;
int temp_terra = 0;

//Collegare una resistenza da 1K o la scheda led Grove
#define PIN_LED1 5
//Collegare RESET a questo pin con un jumper
#define PIN_RESET 4

//Collegamento alla funzione del led
void accendi_LED_per(byte volte);

//Collegamento alla piattaforma GL-Blocks
#include "GL-Blocks-WiFi.h"
#include "GL-Blocks-Dashboard-OPB.h"
//*******************************************
///// NON SPOSTARE QUESTE DEFINIZIONI  /////

#include <Wire.h>  

//Libreria ENV Shield di Arduino
//Installare da "Gestione Librerie"
#include <Arduino_MKRENV.h>

#include <OneWire.h> 
#include <DallasTemperature.h>

// DS12B20 data plugged into pin 2 on the Arduino 
#define PIN_DS18B20 2
OneWire oneWire(PIN_DS18B20);
DallasTemperature temp_sens(&oneWire);

//Collegare i sensori capacitivi al connettore analogico indicato
#define PIN_UMIDITA_1 A1
#define PIN_UMIDITA_2 A3
#define PIN_UMIDITA_3 A4

/**
 * Preperazione di Arduino: setup() 
 * Eseguito una sola volta.
 */
void setup() {

  //Configura il pin per il reset
  digitalWrite(PIN_RESET, HIGH);
  delay(100);
  pinMode(PIN_RESET, OUTPUT);

  //Attiva il Serial Monitor
  Serial.begin(9600);  
  delay(2000); //Attesa setup della seriale
  Serial.println("FEM - Horto Mio Plus Kit");

  pinMode(PIN_LED1, OUTPUT);
  digitalWrite(PIN_LED1, LOW);

  accendi_LED_per(1); //Lampeggia 1 volta
  
  Wire.begin(); //Inzializza I2C per l'ENV shield 
  accendi_LED_per(2); //Lampeggia 2 volte

  ///// ATTIVAZIONE DEI SENSORI /////
 
  //Si connette L'ENV Shield
  if (!ENV.begin()) {
    Serial.println("Errore durante l'avvio del MKR Shield");
    while (1);
  }   

  //Start up del DS18B20 
  temp_sens.begin();

  accendi_LED_per(3); //Lampeggia 3 volte

  // Connessione al WiFi: vedi il file GL-Blocks-WiFi.h
  Connetti_WIFI();  
  
  accendi_LED_per(4); //Lampeggia 4 volte: PRONTI   

}

/**
 * Ciclo delle operazioni da eseguire sempre
 */
void loop() {    

  accendi_LED_per(3);

  //Temperatura dall'ENV Shield
  temp_aria = ENV.readTemperature();

  //Umidità aria dall'ENV Shield
  umid_aria = ENV.readHumidity();

  //Luminosità dall'ENV Shield
  luminosita = ENV.readIlluminance();

  //Delay per MUX analogico
  delay(1);
  
  temp_sens.requestTemperatures();
  temp_terra = temp_sens.getTempCByIndex(0);
  if (temp_terra < 0) temp_terra = 0; //Sensore non attivo

  //Lettura dei tre sensori di umidità terreno
  umid_terreno1 = leggi_sens_umidita(PIN_UMIDITA_1);
  umid_terreno2 = leggi_sens_umidita(PIN_UMIDITA_2);
  umid_terreno3 = leggi_sens_umidita(PIN_UMIDITA_3);

  accendi_LED_per(2); //Lampeggia il led per 2 volte
      
  mostra_valori_serial_monitor();

  //Verifica se si è ancora connessi al WiFi
  Connetti_WIFI();
  
  delay(1000); //Attende 1 secondo

  //Invia i dati alla dashboard
  Trasmetti_Dati_Cloud();    

  //30 sec tra un ciclo e il prossimo
  delay(30000); 
}

/**
 * Procedura di lettura umidità terreno
 */
int leggi_sens_umidita(int pin)
{
  delay(10); //Breve ritardo per letture analogiche consecutive  
  long va = analogRead(pin);
  int umid = (int)map(va, 520, 760, 100, 0); //Tarato sui sensori capacitivi Grove
  umid = constrain(umid, 0, 100);
  return umid;
}

/**
 * Lampeggia il LED sul pin PIN_LED1 per un numero di volte
 */
void accendi_LED_per(byte volte) 
{
  for (byte i=0; i<volte; i++) {
    digitalWrite(PIN_LED1, HIGH);
    delay(200);
    digitalWrite(PIN_LED1, LOW);
    delay(200);
  }
}

/**
 * Scrive i valori dei sensori sul serial Monitor (Serial)
 */
void mostra_valori_serial_monitor()
{
  Serial.println();    
  Serial.print("Temp. aria = ");
  Serial.print(temp_aria);
  Serial.println(" °C");

  Serial.print("Umid. aria = ");
  Serial.print(umid_aria);
  Serial.println(" %");

  Serial.print("Illuminazione = ");
  Serial.print(luminosita);
  Serial.println(" lux");

  Serial.print("Umid. terr. s.1 = ");
  Serial.print(umid_terreno1);
  Serial.println(" %");

  Serial.print("Umid. terr. s.2 = ");
  Serial.print(umid_terreno2);
  Serial.println(" %");

  Serial.print("Umid. terr. s.3 = ");
  Serial.print(umid_terreno3);
  Serial.println(" %");

  Serial.print("Temp. terra  = ");
  Serial.print(temp_terra);
  Serial.println(" °C");    
}
