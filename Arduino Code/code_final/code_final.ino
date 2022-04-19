#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SSpin   21  // pin SDA du RC522 -> 21
#define RSTpin  2   // pin RST du RC522 -> 2


const char* ssid = "Eleves";            //Nom du réseau wifi
const char* password = "ml$@0931584S";  //Mot de passe du réseau wifi

String serverName = "https://b268076a-1104-466e-837a-a82b9ada121d.mock.pstmn.io/";  //Adresse de l'API
bool mode_veille = true;

MFRC522 rfid(SSpin, RSTpin); //Donner les pins 'SS' et 'RST' du capteur RFID

void setup()
{
  Serial.begin(115200);
  //Initialisation du capteur RFID
  SPI.begin();
  rfid.PCD_Init();

  wifi_connexion();

  Serial.println("\nRFID-RC522 - Reader");
}


void loop()
{
  if (standBy == false)
  {
    cardPresent();
  }
  else if (standBy == false)
  {
    standByRead();
    delay(600 0);
  }
}

//---------------/ Connexion au Wifi /---------------//

void wifi_connexion()
{
  Serial.print("Connexion WIFI à : ");
  Serial.print(ssid);
  //Connexion avec l'identifiant et le mdp
  WiFi.begin(ssid, password); //Essaye de se connecter au wifi

  while (WiFi.status() != WL_CONNECTED) { //Tant que la connexion au wifi ne se fait pas
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected"); //Quand la connexion au wifi est effectué
}

//---------------/ Détecter et lire la carte /---------------//

void cardPresent()
{
  if (rfid.PICC_IsNewCardPresent())  // on a détecté un tag
  {
    if (rfid.PICC_ReadCardSerial())  // on a lu avec succès son contenu
    {
      light_led(2, 500); //Indication de lecture de la carte
      String uid = "";

      for (byte i = 0; i < rfid.uid.size; i++)
      {
        String str = String(rfid.uid.uidByte[i], HEX); //Récupère les bytes en décimal et les transforme en hexadécimal
        uid = uid + str;  //Ajoute la valeur à l'UID
      }

      //Transforme les lettres minuscule en lettre majuscule
      for (byte i = 0; i < uid.length(); i++)
      {
        if (uid[i] >= 'a' && uid[i] <= 'z')
        {
          uid[i] = uid[i] - 32;
        }
      }

      Serial.println("ID Carte : " + uid);
      unlock_request(uid);
    }
  }
}

//---------------/ Requête pour déverouiller /---------------//

void unlock_request(String uid)
{
  StaticJsonBuffer<300> JSONBuffer;
  JsonObject& parsed = JSONBuffer.parseObject(payload); //Met les données de réponse en JSON
  String idCadenas = parsed["idCadenas"]; //Récupère la clé "idCadenas"
  String action = parsed["action"]; //Récupère la clé "action"

  if (idCadenas == getStringMacAddress()) //Si la réponse est celle pour le cadenas
  {
    if (action == "true") //Si il demande d'ouvrir
    {
      //Faire ouvrir le cadenas
      //Activer le micro-moteur
      light_led(2, 500); //Repère visuelle de l'ouverture
    }
  }

}

//---------------/ StandBy Read /---------------//

void standByRead()
{
  if (WiFi.status() == WL_CONNECTED) // Verifie si il est connecté au wifi
  {
    HTTPClient http;

    String serverPath = serverName + "cadenas/veille?idCadenas=" + getStringMacAddress(); //Rentre les données dans l'URL de requête de l'API

    Serial.println(serverPath);
    http.begin(serverPath.c_str());

    // Envoi la requete HTTP au serveur
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) //Si il y a une réponse
    {
      String payload = http.getString(); //recupère les données de réponse de l'API
      Serial.println(payload);

      StaticJsonBuffer<300> JSONBuffer;
      JsonObject& parsed = JSONBuffer.parseObject(payload); //Met les données de réponse en JSON
      String idCadenas = parsed["idCadenas"]; //Récupère la clé "idCadenas"
      String veille = parsed["veille"]; //Récupère la clé "action"

      if (idCadenas == getStringMacAddress()) //Si la réponse est celle pour le cadenas
      {
        if (veille == true) //Si il demande d'ouvrir
        {
          standBy = true;
        }
        else if (veille == false)
        {
          standBy = false;
        }
      }
    }
    else {  //Si il n'y a pas de réponse
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

//---------------/ Blink Led /---------------//

void light_led(int pin, int temps)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);   //Allume la led
  delay(temps);
  digitalWrite(pin, LOW);    //Éteint la led
  delay(temps);
}

//---------------/ Faire la requête /---------------//  \("{}")/

JsonObject makeRequest(string type)
{
  String serverPath;
  if (type == "unlock")
  {
    serverPath = serverName + "cadenas/ouvrir?idCadenas=" + getStringMacAddress() + "&idCarte=" + uid; //Rentre les données dans l'URL de requête de l'API
  }
  else if (type == "standby")
  {
    serverPath = serverName + "cadenas/veille?idCadenas=" + getStringMacAddress(); //Rentre les données dans l'URL de requête de l'API
  }

  if (WiFi.status() == WL_CONNECTED) // Verifie si il est connecté au wifi
  {
    HTTPClient http;
    Serial.println(serverPath);
    http.begin(serverPath.c_str());

    // Envoi la requete HTTP au serveur
    int httpResponseCode = http.GET();
    StaticJsonBuffer<300> JSONBuffer;
    JsonObject& json;

    if (httpResponseCode > 0) //Si il y a une réponse
    {
      String payload = http.getString(); //recupère les données de réponse de l'API
      Serial.println(payload);
      json = JSONBuffer.parseObject(payload); //Met les données de réponse en JSON
    }
    else //Si il n'y a pas de réponse
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      json = JSONBuffer.parseObject("{}"); //("{}")/\("{}")/\("{}")/\("{}")/\("{}")/\("{}")/\("{}")/\("{}")/\("{}")
                                           //\____________________________________________________________________/

    }

    http.end();

    return json;
  }
}

//---------------/ MAC Address /---------------//

String getStringMacAddress()
{
  String idmacAddress = WiFi.macAddress(); //Récupère l'adresse MAC de l'ESP
  //Enlève les ':' de l'adresse MAC
  idmacAddress.remove(2, 1);
  idmacAddress.remove(4, 1);
  idmacAddress.remove(6, 1);
  idmacAddress.remove(8, 1);
  idmacAddress.remove(10, 1);

  return idmacAddress; //Renvoi l'adresse MAC sans les ':'
}
