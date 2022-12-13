#include "RunningAverage.h"

RunningAverage frigoRA(100);
RunningAverage ariaRA(100);

#define POMPA 12
#define FRIGO 11
#define MN 10
#define MS 6

void setup() {

  Serial.begin(9600);

  digitalWrite(MN, HIGH);
  digitalWrite(FRIGO, HIGH);
  digitalWrite(POMPA, HIGH);

  pinMode(MN, OUTPUT);
  pinMode(FRIGO, OUTPUT);
  pinMode(POMPA, OUTPUT);
  pinMode(MS, OUTPUT);

  digitalWrite(MN, HIGH);
  digitalWrite(FRIGO, HIGH);
  digitalWrite(POMPA, HIGH);

  frigoRA.clear();
  ariaRA.clear();



}

int S = -1;        //STATO MACCHINA
bool P = true;        //PRIMO INGRESSO
bool E = false;       //CONDIZIONE USCITA
unsigned long T = 0;  //TIMER UNIVERSALE
uint8_t C = 0;        //CONTATORE CICLI

bool sFRIGO = false;
bool sPOMPA = false;
bool sMOTARIA = false;
bool sMOTNOARIA = false;

bool osFRIGO = false;
bool osPOMPA = false;
bool osMOTARIA = false;
bool osMOTNOARIA = false;

unsigned long tTemp1 = 0;  //cicli 10 ms
unsigned long tTemp2 = 0;  //cicli 60 s

float ariaTemp = 0.0;
float frigoTemp = 0.0;
float oldFrigo = 0.0;
float oldAria = 0.0;
float DT_FRIGO = 0.0;
float DT_ARIA = 0.0;

unsigned long tAct = 0;
unsigned long dStampa = 0;

unsigned long dpon = 0;
unsigned long dpoff = 0;

void loop() {

  readTemp();

  debug();

  act();

  pompa();

  if (S == -1) {
    if (P) {
      sFRIGO = true;
      sMOTNOARIA = false;
      sMOTARIA = false;
      P = false;
    }

    // condizione uscita
    if ((frigoTemp >= 700) || ((millis() - T) > 60000 * 4)) {
      E = true;
    }

    if (E) {
      S++;
      P = true;
      E = false;
      T = millis();
    }
  }
  else if (S == 0) {       // NO ARIA
    if (P) {
      sFRIGO = true;
      sMOTNOARIA = true;
      sMOTARIA = false;
      P = false;
    }

    // condizione uscita
    if ((DT_FRIGO <= 7) && ((millis() - T) > 60000 * 4)) {
      E = true;
    }

    if (E) {
      S++;
      P = true;
      E = false;
      T = millis();
    }
  }
  else if (S == 1) {  // PAUSA
    if (P) {
      sFRIGO = true;
      sMOTNOARIA = false;
      sMOTARIA = false;
      P = false;
    }

    // condizione uscita
    if ((millis() - T) > 10000) {
      E = true;
    }

    if (E) {
      S++;
      P = true;
      E = false;
      T = millis();
    }
  }
  else if (S == 2) {  // ARIA
    if (P) {
      sFRIGO = true;
      sMOTNOARIA = false;
      sMOTARIA = true;
      P = false;
    }

    // condizione uscita
    if ((DT_ARIA <= 0.5) && ((millis() - T) > 60000 * 4)) {
      E = true;
    }

    if (E) {
      S++;
      P = true;
      E = false;
      T = millis();
    }
  }
  else if (S == 3) {  // PAUSA
    if (P) {
      sFRIGO = true;
      sMOTNOARIA = false;
      sMOTARIA = false;
      P = false;
    }

    // condizione uscita
    if ((millis() - T) > 10000) {
      E = true;
    }

    if (E) {
      C++;
      if (C < 8) {
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
    if (P) {
      sFRIGO = false;
      sMOTNOARIA = false;
      sMOTARIA = false;
      sPOMPA = false;
      P = false;
    }
  }

}

void act() {
  if ((millis() - tAct) > 1000) {
    tAct = millis();

    if (sFRIGO != osFRIGO) {
      digitalWrite(FRIGO, !sFRIGO);
    }
    else if (sPOMPA != osPOMPA) {
      digitalWrite(POMPA, !sPOMPA);
    }
    else if (sMOTARIA != osMOTARIA) {
      digitalWrite(MS, LOW);
      digitalWrite(MN, !sMOTARIA);
    }
    else if (sMOTNOARIA != osMOTNOARIA) {
      digitalWrite(MS, HIGH);
      digitalWrite(MN, !sMOTNOARIA);
    }

    osPOMPA = sPOMPA;
    osFRIGO = sFRIGO;
    osMOTARIA = sMOTARIA;
    osMOTNOARIA = osMOTNOARIA;

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

  if ((millis() - tTemp1) > 10) {
    frigoRA.addValue(analogRead(A0));
    ariaRA.addValue(analogRead(A1));
    tTemp1 = millis();
  }

  if ((millis() - tTemp2) > 60000) {
    frigoTemp = frigoRA.getAverage();
    ariaTemp = ariaRA.getAverage();

    DT_FRIGO = frigoTemp - oldFrigo;
    DT_ARIA = ariaTemp - oldAria;

    oldFrigo = frigoTemp;
    oldAria = ariaTemp;

    tTemp2 = millis();
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
