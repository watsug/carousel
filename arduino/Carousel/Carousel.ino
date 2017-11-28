int pinDIR = 13;
int pinSTP = 12;
int pinSLP = 11;
int pinRST = 10;
int pinM2  = 9;
int pinM1  = 8;
int pinM0  = 7;
int pinEN  = 6;
int pinZERO  = 2;

const int DELAY_PER_STEP = 10;
const int STEPS_PER_SLOT = 20;
const int MAX_STEPS = 200;
const int SLOT_TIME = 2000;
const int DIR_LEFT = HIGH;
const int DIR_RIGHT = LOW;
const int ZERO_FIX = 2;
const int SLOTS = 10;

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

  pinMode(pinZERO, INPUT);

  // DVR8825 setup
  digitalWrite(pinSLP, HIGH);
  digitalWrite(pinRST, HIGH);
  digitalWrite(pinEN,  LOW);

  // resoulution setup - 32 sub-steps
  digitalWrite(pinM2, LOW);
  digitalWrite(pinM1, LOW);
  digitalWrite(pinM0, LOW);

  // direction setup
  digitalWrite(pinDIR, DIR_LEFT);
}

void oneStep(int dir)
{
  digitalWrite(pinDIR, dir);
  digitalWrite(pinSTP, HIGH);
  delay(DELAY_PER_STEP);
  digitalWrite(pinSTP, LOW);
  delay(DELAY_PER_STEP);
}

void advance(int steps, int dir)
{
  for (int i=steps; i > 0; i--)
  {
    oneStep(dir);
  }
}

void findZero()
{
  for (int i=MAX_STEPS; i > 0; i--)
  {
    oneStep(DIR_LEFT);
    int val = digitalRead(pinZERO);
    if (val == 1)
    {
      break;
    }
  }
  delay(SLOT_TIME);
  int width = 0;
  for (int i=MAX_STEPS; i > 0; i--)
  {
    oneStep(DIR_LEFT);
    width++;
    int val = digitalRead(pinZERO);
    if (val == 0)
    {
      break;
    }
  }
  delay(SLOT_TIME);
  advance((width / 2) + ZERO_FIX, DIR_RIGHT);
  delay(SLOT_TIME);
}

void setCard(int slotNumber)
{
  int sN = slotNumber <= 0 ? 1 : (slotNumber > SLOTS ? SLOTS : slotNumber);
  findZero();
  for (int i=sN; i > 0; i--)
  {
    advance(STEPS_PER_SLOT, DIR_LEFT);
  }
}

void loop()
{
  setCard(random(1, 10));
  delay(5000);
}
