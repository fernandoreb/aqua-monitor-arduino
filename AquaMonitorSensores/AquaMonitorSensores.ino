#include <DallasTemperature.h>
#include <OneWire.h>


/****************************************************************************************
 *  Configuracoes para o Sensor de temperatura dallas DSB18B20 - Sonda a prova d'agua
 * O Sersor tem 3 pinos (visto de frente)
 * 1 - GND (fio preto)
 * 2 - Dados(fio amarelo ou azul)
 * 3 - VCC (fio vermelho)
 * O pino de dados(2) deve ser ligado ao VCC por um resistor de 4,7k
 ****************************************************************************************/

//Pino que define a leitura do sensor de luz
#define ANL_SEN_LUZ 5

//Pino do sensor de temperatura
#define SENSOR_TEMP 7
//Pinos de Entrada do sensor de nÌvel 
#define UM_CM 6
#define SEIS_CM 5
#define DOZE_CM 4
#define DE_OITO_CM 3

#define BAUD_9600 9600

#define CMD_TEMP_AGUA 1 
#define CMD_TEMP_TAMPA 2 
#define CMD_TEMP_AMB 3 
#define CMD_NIVEL_SUMP 4 
#define CMD_NIVEL_REPO 5 
#define CMD_NIVEL_MAXI 6 
#define CMD_LUZ_LIGADA 7

#define CMD_L_TEMP_AGUA 'T' 
#define CMD_L_TEMP_TAMPA 'E' 
#define CMD_L_TEMP_AMB 'A' 
#define CMD_L_NIVEL_SUMP 'S' 
#define CMD_L_NIVEL_REPO 'R' 
#define CMD_L_NIVEL_MAXI 'N' 
#define CMD_L_LUZ_LIGADA 'L'

#define CMD_MAX_SIZE 6
#define CMD_DELAY_ENVIO 25

#define TEMPERATURE_PRECISION 9

//IncializaÁ„o das libs e vari·veis do sensor de temperatura
float temperatura = 0.0;
int state;
int sensorLuz = 0;

OneWire ds(SENSOR_TEMP);
DallasTemperature sensors(&ds);

DeviceAddress sensorAgua, sensorTampa, sensorAmb;

char comando[CMD_MAX_SIZE] = {'X','9','9','.','9','9'};
char temp[CMD_MAX_SIZE];
int iComando = CMD_TEMP_AGUA;

void setup()
{
  Serial.begin(BAUD_9600); //Ativa Serial Monitor
  //Serial.println("Ativado"); // avisa que esta ok
  pinMode(UM_CM, INPUT);
  pinMode(SEIS_CM, INPUT);
  pinMode(DOZE_CM, INPUT);
  pinMode(DE_OITO_CM, INPUT);

  sensors.begin();
  
  //if (!sensors.getAddress(sensorAgua, 0)) Serial.println("Unable to find address for Device 0"); 
  //if (!sensors.getAddress(sensorTampa, 1)) Serial.println("Unable to find address for Device 1"); 
  //  if (!sensors.getAddress(sensorAmb, 2)) Serial.println("Unable to find address for Device 2"); 
  sensorAgua[0] = 0x28;
  sensorAgua[1] = 0x48;
  sensorAgua[2] = 0x59;
  sensorAgua[3] = 0x0D;
  sensorAgua[4] = 0x04;
  sensorAgua[5] = 0x00;
  sensorAgua[6] = 0x00;
  sensorAgua[7] = 0xAF;

  sensorAmb[0] = 0x28;
  sensorAmb[1] = 0x14; 
  sensorAmb[2] = 0xEB;
  sensorAmb[3] = 0x49;
  sensorAmb[4] = 0x02;
  sensorAmb[5] = 0x00;
  sensorAmb[6] = 0x00;
  sensorAmb[7] = 0x0F;

  sensorTampa[0] = 0x28;
  sensorTampa[1] = 0x96;
  sensorTampa[2] = 0xFD;
  sensorTampa[3] = 0x49;
  sensorTampa[4] = 0x02;
  sensorTampa[5] = 0x00;
  sensorTampa[6] = 0x00;
  sensorTampa[7] = 0x6B;
  
  // set the resolution to 9 bit
  sensors.setResolution(sensorAgua, TEMPERATURE_PRECISION);
  sensors.setResolution(sensorTampa, TEMPERATURE_PRECISION);
  sensors.setResolution(sensorAmb, TEMPERATURE_PRECISION);

}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void enviaComando()
{
  for(int i = 0; i < CMD_MAX_SIZE; i++)
    Serial.print(comando[i]);

  //Serial.println();
  //vw_send((uint8_t *)comando, CMD_MAX_SIZE);  
  //vw_wait_tx();
}

void loop()
{
  switch(iComando)
  {
	case CMD_TEMP_AGUA:
	  enviaTemperaturaAgua();
	break;
	case CMD_TEMP_TAMPA:
	  enviaTemperaturaTampa();
	break;
	case CMD_TEMP_AMB:
    	  enviaTemperaturaAmb();
        break;
	case CMD_NIVEL_SUMP:
	  enviaNivelSump();
	break;
        case CMD_NIVEL_REPO:
	  enviaNivelRepo();
	break;
	case CMD_NIVEL_MAXI:
	  enviaNivelMaxi();
        break;
	case CMD_LUZ_LIGADA:
  	  envialuzLigada();
	break;
        default:iComando = 0;
        //Serial.println();
  }
  iComando++;
  delay(CMD_DELAY_ENVIO);
}

void enviaNivelRepo()
{
  state = digitalRead(UM_CM);
  if(state == HIGH)
  {
     state = digitalRead(SEIS_CM);
     if(state == HIGH)
     {
       state = digitalRead(DOZE_CM);
       if(state == HIGH)
       {
         state = digitalRead(DE_OITO_CM);
         if(state == HIGH)
         {
           temp[0] = '2';temp[1] = '0';temp[2] = '.';temp[3] = '0';temp[4] = '0';
         }
         else
         {
           temp[0] = '1';temp[1] = '8';temp[2] = '.';temp[3] = '0';temp[4] = '0';
         }
       }
       else
       {
         temp[0] = '1';temp[1] = '2';temp[2] = '.';temp[3] = '0';temp[4] = '0';
       }
     }
     else
     {
       temp[0] = '0';temp[1] = '6';temp[2] = '.';temp[3] = '0';temp[4] = '0';
     } 
  }
  else
  {
    temp[0] = '0';temp[1] = '1';temp[2] = '.';temp[3] = '0';temp[4] = '0';
  }
  comando[0] = CMD_L_NIVEL_REPO;
  comando[1]=temp[0];
  comando[2]=temp[1];
  comando[3]=temp[2];
  comando[4]=temp[3];
  comando[5]=temp[4];

  enviaComando();
}

void enviaTemperaturaAgua()
{
  //Lendo a temperatura
  sensors.requestTemperatures(); // Send the command to get temperatures
  //temperatura = sensors.getTempCByIndex(0);
  temperatura = sensors.getTempC(sensorAgua);

  if(temperatura < 10)
    temperatura = 10.00f;
  if(temperatura > 99)
    temperatura = 99.00f;  

  dtostrf(temperatura, 4, 2, temp);
  comando[0] = CMD_L_TEMP_AGUA;
  comando[1] = temp[0];
  comando[2] = temp[1];
  comando[3] = temp[2];
  comando[4] = temp[3];
  comando[5] = temp[4];
  
  enviaComando();
}

void enviaTemperaturaTampa()
{
//TODO: implementar
  //temperatura = 28.06;
  sensors.requestTemperatures(); // Send the command to get temperatures
  //temperatura = sensors.getTempCByIndex(1);
   temperatura = sensors.getTempC(sensorTampa);
  //temperatura += 10.0f;

  if(temperatura < 10)
    temperatura = 10.00f;
  if(temperatura > 99)
    temperatura = 99.00f;  

  dtostrf(temperatura, 4, 2, temp);
  comando[0] = CMD_L_TEMP_TAMPA;
  comando[1] = temp[0];
  comando[2] = temp[1];
  comando[3] = temp[2];
  comando[4] = temp[3];
  comando[5] = temp[4];
  
  enviaComando();
}

void enviaTemperaturaAmb()
{
//TODO: implementar
  //temperatura = 15.10;
  sensors.requestTemperatures(); // Send the command to get temperatures
  //temperatura = sensors.getTempCByIndex(2);
  temperatura = sensors.getTempC(sensorAmb);
  //temperatura += 40.0f;

  if(temperatura < 10)
    temperatura = 10.00f;
  if(temperatura > 99)
    temperatura = 99.00f;  

  dtostrf(temperatura, 4, 2, temp);
  comando[0] = CMD_L_TEMP_AMB;
  comando[1] = temp[0];
  comando[2] = temp[1];
  comando[3] = temp[2];
  comando[4] = temp[3];
  comando[5] = temp[4];
  
  enviaComando();
}



void enviaNivelSump()
{
//TODO: implementar
  temp[0] = '1';temp[1]='5';temp[2]='.';temp[3]='0';temp[4]='0';temp[5]='\0';

  comando[0] = CMD_L_NIVEL_SUMP;
  comando[1]=temp[0];
  comando[2]=temp[1];
  comando[3]=temp[2];
  comando[4]=temp[3];
  comando[5]=temp[4];

  enviaComando();
}

void enviaNivelMaxi()
{
//TODO: implementar
  temp[0] = '1';temp[1]='0';temp[2]='0';temp[3]='0';temp[4]='0';temp[5]='\0';

  comando[0] = CMD_L_NIVEL_MAXI;
  comando[1]=temp[0];
  comando[2]=temp[1];
  comando[3]=temp[2];
  comando[4]=temp[3];
  comando[5]=temp[4];

  enviaComando();
}

void envialuzLigada()
{
  sensorLuz = analogRead(ANL_SEN_LUZ);
  if(sensorLuz < 1000)
  {
    temp[0] = '0';temp[1]='0';temp[2]='0';temp[3]='0';temp[4]='0';temp[5]='\0';
  }
  else  
  {
    temp[0] = '1';temp[1]='0';temp[2]='0';temp[3]='0';temp[4]='0';temp[5]='\0';
  }
  

  comando[0] = CMD_L_LUZ_LIGADA;
  comando[1]=temp[0];
  comando[2]=temp[1];
  comando[3]=temp[2];
  comando[4]=temp[3];
  comando[5]=temp[4];

  enviaComando();
}


