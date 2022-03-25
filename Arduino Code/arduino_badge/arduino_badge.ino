#include "PN532_HSU.h"    //Librairie de lecture du capteur NFC
#include "PN532.h"        //Librairie de lecture du capteur NFC
#include "NfcAdapter.h"   //Librairie du capteur NFC

PN532_HSU interface(Serial1); //Connexion du module Grove NFC sur le serial 1
NfcAdapter nfc = NfcAdapter(interface);

String UID_scan;

void setup()
{

  Serial.begin(115200);

  //Connexion NDEF Reader
  nfc.begin(); //Initialisation du module
  Serial.println("NDEF Reader");

}

void loop()
{


}
