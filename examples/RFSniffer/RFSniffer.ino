#define PORTA_RF  2
#define INT_RF    0
#define LED_DEBUG 13
#define DEBUG     1
#define BUFF_SIZE 180

volatile unsigned long tempo;
volatile unsigned int  intervalo;

volatile unsigned char lock;
volatile unsigned char eof;

unsigned int  buffIdx;
unsigned int buff[BUFF_SIZE];

void setup()
{
  resetVars();
  
  
  Serial.begin(9600);
  delay(500);
  Serial.println("INICIADO!");
  
  
  pinMode(PORTA_RF, INPUT);
  digitalWrite(PORTA_RF, HIGH);
  
  if (DEBUG)
  {
    pinMode(LED_DEBUG, OUTPUT);
    digitalWrite(LED_DEBUG, LOW);
  }
  
  attachInterrupt(INT_RF, intRF, CHANGE);
}

void loop()
{
  if (lock && !eof)
  {
    buff[buffIdx] = intervalo;
    
    lock = false;
    
    buffIdx++;
    if (buffIdx > BUFF_SIZE - 1)
    {
      eof = true;
      buffIdx = 0;
    }
  }
  
  if (eof)
  {
    // Imprime resultado
    Serial.println("buff:");
    for (int i = 0; i < BUFF_SIZE / 3; i++)
    {
      Serial.print(buff[i], DEC);
      if (i < BUFF_SIZE - 1) Serial.print(",");
    }
    Serial.println();
    for (int i = BUFF_SIZE / 3; i < (BUFF_SIZE / 3) * 2; i++)
    {
      Serial.print(buff[i], DEC);
      if (i < BUFF_SIZE - 1) Serial.print(",");
    }
    Serial.println();
    for (int i = (BUFF_SIZE / 3) * 2; i < BUFF_SIZE; i++)
    {
      Serial.print(buff[i], DEC);
      if (i < BUFF_SIZE - 1) Serial.print(",");
    }
    Serial.println();
    Serial.println();
    
    resetVars();
    
    delay(200);
  }
}

void intRF()
{
  if (!lock && !eof)
  {
    lock = true;
    
    intervalo = micros() - tempo;
    tempo = micros();
    
    if (DEBUG) 
      digitalWrite(LED_DEBUG, !digitalRead(LED_DEBUG));
  }
}

void resetVars()
{
  tempo = micros();
  intervalo = 0;
  lock = false;
  eof = false;
  
  buffIdx = 0;
  for (int i = 0; i < BUFF_SIZE; i++)
  {
    buff[i] = 0;
  }
}

