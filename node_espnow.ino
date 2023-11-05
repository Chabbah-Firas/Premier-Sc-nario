#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>   // Inclure la bibliothèque pour le capteur DHT11

// ADRESSE MAC DU RECEVEUR À REMPLACER
uint8_t adresseDiffusion[] = {0x50, 0x02, 0x91, 0xD5, 0xE1, 0x77};

typedef struct struct_message {
  char a[32];
  float temp;
  float humid;
  String room;
  bool e;
} struct_message;

struct_message mesDonnees;

unsigned long dernierTemps = 0;
unsigned long delaiTimer = 1000;  // Intervalle d'envoi des mesures

#define BROCHE_DHT 2    // Broche à laquelle le capteur DHT11 est connecté
#define TYPE_DHT DHT11   // Type de capteur (DHT11 dans ce cas)
DHT dht(BROCHE_DHT, TYPE_DHT);  // Initialiser le capteur DHT11

// Fonction de rappel (callback) lorsque les données sont envoyées
void LorsqueDonneesEnvoyees(uint8_t *adresse_mac, uint8_t statutEnvoi) {
  Serial.print("Statut d'envoi du dernier paquet : ");
  Serial.print("statutEnvoi : ");
  Serial.println(statutEnvoi);
  Serial.print("adresse_mac : ");
  Serial.println(*adresse_mac);
  Serial.print("Statut d'envoi du dernier paquet : ");
  if (statutEnvoi == 0){
    Serial.println("Envoi réussi");
  }
  else{
    Serial.println("Échec d'envoi");
  }
}

void setup() {
 
  // Initialiser le Moniteur Série
  Serial.begin(115200);

  // Mettre l'appareil en mode Station Wi-Fi
  WiFi.mode(WIFI_STA);
  // definir la pin D0 pour wake up de la mode deep sleep
  pinMode(16, WAKEUP_PULLUP);
  
  // Initialiser ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Erreur lors de l'initialisation d'ESP-NOW");
    return;
  }

  // Une fois qu'ESPNow est initialisé avec succès, nous nous inscrivons pour le rappel (callback) d'envoi
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(LorsqueDonneesEnvoyees);

  // Ajouter le destinataire (peer)
  esp_now_add_peer(adresseDiffusion, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  dht.begin();   // Initialiser le capteur DHT11
}

void loop() {
  if((millis() - dernierTemps) > delaiTimer) {
    // Lire les valeurs depuis le capteur DHT11
    float humidite = dht.readHumidity();
    float temperature = dht.readTemperature();
    //variable boolean pour faire une verification de la valeur de capteur
    bool ok;
    //delay(100);
    //vérifier le bon fonctionnement de capteur (dans notre cas c'est un capteur d'humidity & de temperature
   /* if ((humidite!=0)&&(temperature!=0)){
      ok=true;
      }else{ok= false;}*/

    
    // Définir les valeurs à envoyer
    strcpy(mesDonnees.a, "L'agricultaire X");
    mesDonnees.temp = temperature;// Envoyer la valeur de température du DHT11
    mesDonnees.humid = humidite; // Envoyer la valeur de humidité du DHT11
    mesDonnees.room = "1";
    mesDonnees.e =ok;

    // Envoyer le message via ESP-NOW
    esp_now_send(adresseDiffusion, (uint8_t *) &mesDonnees, sizeof(mesDonnees));
    //delay(100);
    // Afficher la température et l'humidité
    Serial.println();
    Serial.print("Température : ");
    Serial.print(temperature);
    Serial.print(" °C, Humidité : ");
    Serial.print(humidite);
    Serial.println(" %");
    // Entrer en mode deep sleep pendant une heure (3600 secondes)
     Serial.println("Je vais entrer en mode deep sleep pendant une heure");
     ESP.deepSleep(3600e6); 
    dernierTemps = millis();
  
  }
  
}
