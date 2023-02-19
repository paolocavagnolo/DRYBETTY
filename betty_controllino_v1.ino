#include <Ewma.h>
#include <Controllino.h>

Ewma frigoRA(0.01);
Ewma ariaRA(0.01);

void setup() {

  pinMode(CONTROLLINO_D0, OUTPUT);
  pinMode(CONTROLLINO_D1, OUTPUT);
  pinMode(CONTROLLINO_D2, OUTPUT);
  pinMode(CONTROLLINO_D3, OUTPUT);
  pinMode(CONTROLLINO_D4, OUTPUT);
  pinMode(CONTROLLINO_D5, OUTPUT);
  pinMode(CONTROLLINO_D6, OUTPUT);
  pinMode(CONTROLLINO_D7, OUTPUT);

  Serial.begin(9600);

}

int S = 0;        //STATO MACCHINA
bool P = true;        //PRIMO INGRESSO
bool E = false;       //CONDIZIONE USCITA
unsigned long T = 0;  //TIMER UNIVERSALE
uint8_t C = 0;        //CONTATORE CICLI

bool sFRIGO = false;
bool sPOMPA = false;
bool sMOTARIA = false;
bool sMOTNOARIA = false;
bool sVENTOLA = false;

bool osFRIGO = false;
bool osPOMPA = false;
bool osMOTARIA = false;
bool osMOTNOARIA = false;
bool osVENTOLA = false;

unsigned long tTemp1 = 0;  //cicli 10 ms
unsigned long tTemp3 = 0;  //cicli 60 s

float ariaTemp = 0.0;
float frigoTemp = 0.0;
float oldFrigo = 0.0;
float oldAria = 0.0;
float DT_FRIGO = 0.0;
float DT_ARIA = 0.0;
float Dtemp = 0.0;
float oldDtemp = 0.0;
float DDT = 0.0;

unsigned long tAct = 0;
unsigned long dStampa = 0;

unsigned long dpon = 0;
unsigned long dpoff = 0;

void loop() {

  readTemp();

  pompa();

  debug();

  act();

  if (S == 0) {       // NO ARIA
    if (P) {
      sFRIGO = true;
      sMOTNOARIA = false;
      sMOTARIA = false;
      sVENTOLA = true;
      P = false;
    }

    // condizione uscita
    if ((DT_FRIGO <= 7) && ((millis() - T) > 60000 * 2)) {
      E = true;
    }

    if (E) {
      S++;
      P = true;
      E = false;
      T = millis();
    }
  }

  else if (S == 1) {  // ARIA
    if (P) {
      sFRIGO = true;
      sMOTNOARIA = false;
      sMOTARIA = true;
      sVENTOLA = true;
      P = false;
    }

    // condizione uscita
    if ((millis() - T) > 60000 * 2) {
      if ((DT_ARIA <= 0.5) || (ariaTemp >= 560)) {
        E = true;
      }
    }

    if (E) {
      if (ariaTemp < 550) {
        S = 0;
      }
      else {
        S = 4;
      }
      P = true;
      E = false;
      T = millis();
    }

  }


  while (S == 4) {    // SPEGNI TUTTO
    // MOTORE
    digitalWrite(CONTROLLINO_D7, LOW);
    delay(100);
    digitalWrite(CONTROLLINO_D4, LOW);
    digitalWrite(CONTROLLINO_D3, LOW);

    // COMPRESSORE
    digitalWrite(CONTROLLINO_D2, LOW);
    delay(100);
    digitalWrite(CONTROLLINO_D6, HIGH); //STACCA NEUTRO COMP.

    delay(100);

    // POMPA
    digitalWrite(CONTROLLINO_D1, LOW);
    delay(100);
    digitalWrite(CONTROLLINO_D5, HIGH); //STACCA NEUTRO POMPA

    //VENTOLA
    digitalWrite(CONTROLLINO_D0, LOW);

  }

}


void act() {
  if ((millis() - tAct) > 100) {
    tAct = millis();

    if (sFRIGO != osFRIGO) {
      digitalWrite(CONTROLLINO_D2, sFRIGO);
      osFRIGO = sFRIGO;
    }
    else if (sPOMPA != osPOMPA) {
      digitalWrite(CONTROLLINO_D1, sPOMPA);
      osPOMPA = sPOMPA;
    }
    else if (sMOTARIA != osMOTARIA) {
      digitalWrite(CONTROLLINO_D7, sMOTARIA);
      digitalWrite(CONTROLLINO_D3, LOW);
      digitalWrite(CONTROLLINO_D4, HIGH);
      osMOTARIA = sMOTARIA;
    }
    else if (sMOTNOARIA != osMOTNOARIA) {
      digitalWrite(CONTROLLINO_D7, sMOTNOARIA);
      digitalWrite(CONTROLLINO_D3, HIGH);
      digitalWrite(CONTROLLINO_D4, LOW);
      osMOTNOARIA = osMOTNOARIA;
    }
    else if (sVENTOLA != osVENTOLA) {
      digitalWrite(CONTROLLINO_D0, sVENTOLA);
      osVENTOLA = sVENTOLA;
    }

  }
}

void pompa() {
  if (sPOMPA) {
    if ((millis() - dpon) > 60000 * 1) {
      dpoff = millis();
      sPOMPA = false;
    }
  }
  else {
    if ((millis() - dpoff) > 60000 * 5) {
      dpon = millis();
      sPOMPA = true;
    }
  }
}

void readTemp() {

  if ((millis() - tTemp1) > 100) {

    frigoTemp = frigoRA.filter(analogRead(A0));
    ariaTemp = ariaRA.filter(analogRead(A1));

    tTemp1 = millis();

  }

  if ((millis() - tTemp3) > 60000) {

    DT_FRIGO = frigoTemp - oldFrigo;
    DT_ARIA = ariaTemp - oldAria;
    Dtemp = frigoTemp - ariaTemp;
    DDT = Dtemp - oldDtemp;

    oldFrigo = frigoTemp;
    oldAria = ariaTemp;
    oldDtemp = Dtemp;

    tTemp3 = millis();
  }

}

void debug() {
  if ((millis() - dStampa) > 5000) {
    dStampa = millis();
    Serial.print(frigoTemp);
    Serial.print(",");
    Serial.print(DT_FRIGO);
    Serial.print(",");
    Serial.print(ariaTemp);
    Serial.print(",");
    Serial.print(DT_ARIA);
    Serial.print(",");
    Serial.println(S);
  }
}
