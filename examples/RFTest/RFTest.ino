#include <RFremote.h>

const char *cmd = "here goes your command captured from RFrecvDemo";

RFrecv rfrecv;

void setup()
{
  Serial.begin(9600);
  delay(500);
  Serial.println("INICIADO!");
  
  rfrecv.begin();
}

void loop()
{
  if (rfrecv.available())
  {
    if (strncmp((char*)rfrecv.cmd, cmd, CMD_SIZE) == 0)
    {
      Serial.println("COMANDO CORRETO!");
    }
    else
    {
      Serial.println("ERRO!");
    }
  }
}