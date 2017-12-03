int pinDIR = 13;
int pinSTP = 12;
int pinSLP = 11;
int pinRST = 10;
int pinM2  = 9;
int pinM1  = 8;
int pinM0  = 7;
int pinEN  = 6;
int pinPSAVE = 5;
int pinZERO  = 2;

const int DELAY_PER_STEP = 10;
const int DELAY_PER_STEP_SLOW = 35;
const int STEPS_PER_SLOT = 20;
const int MAX_STEPS = 200;
const int SLOT_TIME = 1000;
const int DIR_LEFT = HIGH;
const int DIR_RIGHT = LOW;
const int ZERO_FIX = 1;
const int SLOTS = 10;
const int MIN_HALF_WIDH = 3;

void setup()
{
  pinMode(pinDIR, OUTPUT);
  pinMode(pinSTP, OUTPUT);
  pinMode(pinSLP, OUTPUT);
  pinMode(pinRST, OUTPUT);

  pinMode(pinM2, OUTPUT);
  pinMode(pinM1, OUTPUT);
  pinMode(pinM0, OUTPUT);
  pinMode(pinEN, OUTPUT);
  pinMode(pinPSAVE, OUTPUT);

  pinMode(pinZERO, INPUT);

  // DVR8825 setup
  digitalWrite(pinSLP, HIGH);
  digitalWrite(pinRST, HIGH);
  digitalWrite(pinEN,  LOW);
  digitalWrite(pinPSAVE,  HIGH);

  // resoulution setup - 32 sub-steps
  digitalWrite(pinM2, LOW);
  digitalWrite(pinM1, LOW);
  digitalWrite(pinM0, LOW);

  // direction setup
  digitalWrite(pinDIR, DIR_LEFT);
}

void powerSave(int mode)
{
  digitalWrite(pinPSAVE, mode > 0 ? HIGH : LOW);
}

void oneStep(int dir, int dps)
{
  digitalWrite(pinDIR, dir);
  digitalWrite(pinSTP, HIGH);
  delay(dps);
  digitalWrite(pinSTP, LOW);
  delay(dps);
}

void advance(int steps, int dir, int dps)
{
  for (int i=steps; i > 0; i--)
  {
    oneStep(dir, dps);
  }
}

void findZero()
{
  powerSave(0);
  for (int i=MAX_STEPS; i > 0; i--)
  {
    oneStep(DIR_LEFT, DELAY_PER_STEP);
    int val = digitalRead(pinZERO);
    if (val == 1)
    {
      break;
    }
  }
  delay(SLOT_TIME);
  for (int i=MAX_STEPS; i > 0; i--)
  {
    oneStep(DIR_RIGHT, DELAY_PER_STEP_SLOW);
    int val = digitalRead(pinZERO);
    if (val == 0)
    {
      break;
    }
  }
  delay(SLOT_TIME);
  int width = 0;
  for (int i=MAX_STEPS; i > 0; i--)
  {
    oneStep(DIR_LEFT, DELAY_PER_STEP_SLOW);
    width++;
    if (width > MIN_HALF_WIDH)
    {
      int val = digitalRead(pinZERO);
      if (val == 0)
      {
        break;
      }
    }
  }
  delay(SLOT_TIME);
  advance((width / 2) + ZERO_FIX, DIR_RIGHT, DELAY_PER_STEP_SLOW);
  delay(SLOT_TIME);

  powerSave(1);
}

void setCard(int slotNumber)
{
  int sN = slotNumber <= 0 ? 1 : (slotNumber > SLOTS ? SLOTS : slotNumber);
  findZero();
  powerSave(0);
  for (int i=sN; i > 0; i--)
  {
    advance(STEPS_PER_SLOT, DIR_LEFT, DELAY_PER_STEP);
  }
  powerSave(1);
}

void loop()
{
  setCard(random(1, 10));
  delay(15000);
}
