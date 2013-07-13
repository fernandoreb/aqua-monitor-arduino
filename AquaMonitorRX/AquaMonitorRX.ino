#include <VirtualWire.h> // lib para RF

void setup()
{
 Serial.begin(9600); //Ativa Serial Monitor
  vw_set_tx_pin(10);
  vw_set_ptt_pin(11);
  vw_set_rx_pin(9);
  vw_set_ptt_inverted(true);
  vw_setup(2000);
  vw_rx_start();
 
}

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
 
  if (vw_get_message(buf, &buflen))
  {
  
    /*if ((char)buf[0] == '1')
    {
       Serial.print("D10.0020.1227.33");
       
    }
    if ((char)buf[0] == '2')
    {
      Serial.print("D12.0023.1218.35");
    }
    Serial.println();
    Serial.print((char)buf[0]);
    Serial.println();*/
    for(int i = 0; i < buflen; i++)
    {
      Serial.print((char)buf[i]);  
    }
    
    //Serial.println();
  }
  
  //delay(5000);
  
}


