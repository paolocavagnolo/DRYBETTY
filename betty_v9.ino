#include <Ewma.h>
#include <Controllino.h>

Ewma frigoRA(0.01);
Ewma ariaRA(0.01);

#define MOT_NEUTRO CONTROLLINO_D7
#define MOT_ARIA CONTROLLINO_D3
#define MOT_NOARIA CONTROLLINO_D4

#define COMP_FASE CONTROLLINO_D2
#define COMP_NEUTRO CONTROLLINO_D6

#define POMPA_FASE CONTROLLINO_D1
#define POMPA_NEUTRO CONTROLLINO_D5

#define VENTOLA CONTROLLINO_D0

#define SWITCH CONTROLLINO_A5
#define FRIGO_SONDA A0
#define ARIA_SONDA A1

#define MAX_TEMP 500


int S = 0;            //STATO MACCHINA
bool P = true;        //PRIMO INGRESSO
bool E = false;       //CONDIZIONE USCITA
unsigned long T = 0;  //TIMER UNIVERSALE
uint8_t C = 0;        //CONTATORE CICLI

void setup() {

  pinMode(MOT_NEUTRO, OUTPUT);
  pinMode(MOT_ARIA, OUTPUT);
  pinMode(MOT_NOARIA, OUTPUT);
  pinMode(COMP_FASE, OUTPUT);
  pinMode(COMP_NEUTRO, OUTPUT);
  pinMode(POMPA_FASE, OUTPUT);
  pinMode(POMPA_NEUTRO, OUTPUT);
  pinMode(VENTOLA, OUTPUT);

  pinMode(SWITCH, INPUT);

  Serial.begin(9600);
}

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

bool switchF = 0;
unsigned long counterS = 0;

void loop() {

  readSwitch();

  readTemp();

  pompa();

  debug();

  act();


  if (S == 0) {  // NO ARIA

    if (P) {
      sFRIGO = true;
      sMOTNOARIA = false;
      sMOTARIA = false;
      sVENTOLA = true;
      P = false;
    }

    // condizione uscita
    if (switchF) {

      if ((millis() - T) > 60000 * 2) {
        if (DT_FRIGO <= 8) {
          E = true;
        }
      }

    } else {

      if ((millis() - T) > 10000) {
        E = true;
      }
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
    if (switchF) {

      if ((millis() - T) > 60000 * 5) {
        if ((DT_ARIA <= 0.8) || (ariaTemp >= MAX_TEMP)) {
          E = true;
        }
      }

    } else {

      if ((millis() - T) > 60000 * 4) {
        E = true;
      }
    }

    if (E) {
      if (ariaTemp < MAX_TEMP) {
        S = 0;
        C++;
      } else {
        S = 4;
      }
      P = true;
      E = false;
      T = millis();
    }
  }


  while (S == 4) {  // CHIUSURA

    if ((millis() - T) < (60000 * 5)) {

      // MOTORE
      digitalWrite(MOT_NEUTRO, HIGH);
      delay(100);
      digitalWrite(MOT_NOARIA, LOW);
      digitalWrite(MOT_ARIA, HIGH);

      // COMPRESSORE
      digitalWrite(COMP_FASE, LOW);
      delay(100);
      digitalWrite(COMP_NEUTRO, HIGH);  //STACCA NEUTRO COMP.

      delay(100);

      // POMPA
      digitalWrite(POMPA_FASE, LOW);
      delay(100);
      digitalWrite(POMPA_NEUTRO, HIGH);  //STACCA NEUTRO POMPA

      //VENTOLA
      digitalWrite(VENTOLA, LOW);

    } else {
      // MOTORE
      digitalWrite(MOT_NEUTRO, LOW);
      delay(100);
      digitalWrite(MOT_NOARIA, LOW);
      digitalWrite(MOT_ARIA, LOW);

      // COMPRESSORE
      digitalWrite(COMP_FASE, LOW);
      delay(100);
      digitalWrite(COMP_NEUTRO, HIGH);  //STACCA NEUTRO COMP.

      delay(100);

      // POMPA
      digitalWrite(POMPA_FASE, LOW);
      delay(100);
      digitalWrite(POMPA_NEUTRO, HIGH);  //STACCA NEUTRO POMPA

      //VENTOLA
      digitalWrite(VENTOLA, LOW);
    }
  }
}


void act() {
  if ((millis() - tAct) > 100) {
    tAct = millis();

    if (sFRIGO != osFRIGO) {
      digitalWrite(COMP_FASE, sFRIGO);
      osFRIGO = sFRIGO;
    } else if (sPOMPA != osPOMPA) {
      digitalWrite(POMPA_FASE, sPOMPA);
      osPOMPA = sPOMPA;
    } else if (sMOTARIA != osMOTARIA) {
      digitalWrite(MOT_NEUTRO, sMOTARIA);
      digitalWrite(MOT_ARIA, LOW);
      digitalWrite(MOT_NOARIA, HIGH);
      osMOTARIA = sMOTARIA;
    } else if (sMOTNOARIA != osMOTNOARIA) {
      digitalWrite(MOT_NEUTRO, sMOTNOARIA);
      digitalWrite(MOT_ARIA, HIGH);
      digitalWrite(MOT_NOARIA, LOW);
      osMOTNOARIA = osMOTNOARIA;
    } else if (sVENTOLA != osVENTOLA) {
      digitalWrite(VENTOLA, sVENTOLA);
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
  } else {
    if ((millis() - dpoff) > 60000 * 5) {
      dpon = millis();
      sPOMPA = true;
    }
  }
}

void readTemp() {

  if ((millis() - tTemp1) > 100) {

    frigoTemp = frigoRA.filter(analogRead(FRIGO_SONDA));
    ariaTemp = ariaRA.filter(analogRead(ARIA_SONDA));

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

void readSwitch() {
  if (switchF) {
    if (analogRead(SWITCH) > 512) {
      counterS++;
    } else {
      counterS = 0;
    }

    if (counterS > 2000) {
      switchF = false;
      counterS = 0;
    }
  } else {
    if (analogRead(SWITCH) < 512) {
      counterS++;
    } else {
      counterS = 0;
    }

    if (counterS > 2000) {
      switchF = true;
      counterS = 0;
    }
  }
}
