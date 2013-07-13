#include <VirtualWire.h> // lib para RF

//Pinos para o transmissor RF
#define TX_PIN 9
#define RX_PIN 10
#define PTT_PIN 11

#define BAUD_9600 9600

#define DOIS_MIL 2000

void setup()
{
  Serial.begin(BAUD_9600); //Ativa Serial Monitor
  vw_set_tx_pin(TX_PIN);
  vw_set_rx_pin(RX_PIN);
  vw_set_ptt_pin(PTT_PIN);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(DOIS_MIL);
}

char c = 0;
void loop()
{
   if(Serial.available())
   {
     c = Serial.read();
     vw_send((uint8_t *)&c, 1);
   }
}
