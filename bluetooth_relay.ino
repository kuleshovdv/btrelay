#include <EEPROM.h>
#include <sha1.h> // https://github.com/Cathedrow/Cryptosuite

#define SESSION_KEY_LENGTH  20
#define SECRET_KEY_LENGTH  20
#define SESSION_TIMEOUT  20000 //ms

#define EEPROM_SECRET_OFFSET  20

#define LED_PIN  13
#define RELAY_PIN  6
#define RELAY_COM_TIME  500 //ms
#define BLE_RESET_PIN 8

#define SETUPBUTTON_PIN  10

// Protocol define
#define SPLITTER  ":"
#define OPEN  "OPEN"
#define TICKET  "TKT"
#define KEY "PASS"

char secret[SECRET_KEY_LENGTH];// = "password";
unsigned long sessionStart = 0;
String sessionKey;

String message = "";

String printSessionKey(int passLength) {
  String result = "";
  for (int i = 0; i < passLength; i++) {
    result += "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[random(62)];
  }
  return result;
}

String printHash(uint8_t* hash) {
  String result = "";
  for (int i = 0; i < 20; i++) {
    result += "0123456789abcdef"[hash[i] >> 4];
    result += "0123456789abcdef"[hash[i] & 0xf];
  }
  return result;
}

void setup() {
  pinMode(BLE_RESET_PIN, OUTPUT);
  digitalWrite(BLE_RESET_PIN, HIGH);


  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN , LOW);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SETUPBUTTON_PIN, INPUT);
  EEPROM.get(EEPROM_SECRET_OFFSET, secret);
  Serial.begin(9600);
  //while (!Serial) {} // Leonagdo debug ONLY
  randomSeed(analogRead(A1));

  //Serial.println(secret);
}

void relayCom() { // main relay action
  digitalWrite(RELAY_PIN, HIGH);
  delay(RELAY_COM_TIME);
  digitalWrite(RELAY_PIN, LOW);
}

void changeBTName(String newName) {
  Serial.println("NEWNAME:" + newName);
  delay(100);
  digitalWrite(BLE_RESET_PIN, LOW);
  delay(1);
  digitalWrite(BLE_RESET_PIN, HIGH);
  delay(3000);
  Serial.print("AT");
  delay(100);
  Serial.print("AT+NAME" + newName);
  delay(100);
  Serial.print("AT+NAMB" + newName);
  delay(100);
  Serial.print("AT+RESET");
  delay(100);
}

String commandProcessor(String incomingString) {
  String result;

  digitalWrite(LED_PIN, HIGH);

  incomingString.trim();

  if (incomingString.startsWith("OPEN")) {
    sessionStart = millis();
    sessionKey = printSessionKey(SESSION_KEY_LENGTH);
    result = sessionKey;
  } else if (incomingString.startsWith("PASS:") && (millis() - sessionStart <= SESSION_TIMEOUT)) {
    incomingString.remove(0, incomingString.indexOf(":") + 1);
    Sha1.initHmac((const uint8_t*)secret, strlen(secret));
    Sha1.print(sessionKey);
    String ss = printHash(Sha1.resultHmac());
    //Serial.print("# "); Serial.println(ss);
    //Serial.print("< "); Serial.println(incomingString);
    if (incomingString.equalsIgnoreCase(ss)) {
      result = "OK";
      relayCom();
    } else {
      result = "NO";
    }
  } else if  (incomingString.startsWith("SECRET:") && digitalRead(SETUPBUTTON_PIN))  {
    incomingString.remove(0, incomingString.indexOf(":") + 1);
    incomingString.toCharArray(secret, SECRET_KEY_LENGTH);
    EEPROM.put(EEPROM_SECRET_OFFSET, secret);
    result = "OK";
  } else if  (incomingString.startsWith("BTNAME:") && digitalRead(SETUPBUTTON_PIN)) {
    incomingString.remove(0, incomingString.indexOf(":") + 1);
    incomingString.trim();
    if ((0 < incomingString.length()) &&(incomingString.length() <= 12)) {
      changeBTName(incomingString);
    } else {
      result = "NO";
    }
  } else if  (incomingString.startsWith("BTPIN:") && digitalRead(SETUPBUTTON_PIN)) {
    result = "COMING_SOON";
  }
  else {
    result = "BLUETOOTH CRYPTO RELAY 0.1";
  }

  digitalWrite(LED_PIN, LOW);
  return result;
}

void loop() {

  if (Serial.available() > 0) {
    // read the incoming byte:
    char incomingChar = Serial.read();
    if (incomingChar == '\n') {
      Serial.println(commandProcessor(message));
      message = "";
    } else {
      message += incomingChar;
    }
  }
}
