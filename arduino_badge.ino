
#include "PN532_HSU.h"
#include "PN532.h"
#include "NfcAdapter.h"

PN532_HSU interface(Serial1); //Connexion du module Grove NFC sur le serial 1
NfcAdapter nfc = NfcAdapter(interface);

String UID_scan;

/**************************************************************************************************/

void setup()
{
  Serial.begin(9600);
  Serial.println("NDEF Reader");
  nfc.begin(); //Initialisation du module
}

void loop()
{
  if (nfc.tagPresent())
  {
    NfcTag tag = nfc.read(); //Lecture de la carte NFC
    UID_scan = tag.getUidString(); //Acquisition du code UID de la carte
    Serial.println("ID Card : " + UID_scan); //Renvoi sur le moniteur du code UID
    
    digitalWrite(2, HIGH);
    delay(500);             
    digitalWrite(2, LOW);    
    delay(500);   
    
  }
  delay(2000);
}
