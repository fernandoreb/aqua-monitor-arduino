#include <SD.h>
#include <APIGrafica.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <Wire.h>
#include <RTClib.h>

/****************************************************************************************
 *  AQUA MONITOR - 
 * Esta aplicao foi projetada com os componentes abaxio:
 * 1 - ARDUINO MEGA 2560
 * 2 - LCD Nokia 
 * 3 - Sensor de temperatura DS18B20
 * 4 - Leitor de cartao SD
 * 5 RTC - Real Time Clock
 ****************************************************************************************/

/****************************************************************************************
 * Configuracoes para menus e controle do LDC - Parte grafica
 * Os pinos do LCD estao identificados no proprio LCD
 ****************************************************************************************/
//Pinos de 9 a 13 digitais - LCD
PCD8544 nokia = PCD8544(9, 10, 11,12,13); // 9 pin LCD control, SCLK, DN, D/C (CS, RST)
APIGrafica apiGraf = APIGrafica(nokia);//Controle dos menus

//Definicoes do menus e seus itens
#define MENU_PRINCIPAL 0

#define MENU_TEMPERATURA     100
#define  MENU_VALOR_TEMP     110
#define   MENU_TEMP_AGUA     111 
#define   MENU_TEMP_TAMPA    112
#define   MENU_TEMP_AMB      113
#define  MENU_RESFRIAMENTO   120
#define   MENU_RESF_LIGADES  121
#define   MENU_RESF_TEMPMAX  122
#define   MENU_RESF_AT_MAN   123
#define  MENU_AQUECIMENTO    130
#define   MENU_AQUEC_LIGADES 131
#define   MENU_AQUEC_TEMPMIN 132
#define   MENU_AQUEC_AT_MAN  133

#define MENU_NIVEL               200
#define  MENU_NIVEL_ATUAL        210
#define   MENU_NIATUAL_SUMP      211
#define   MENU_NIATUAL_REPO      212
#define   MENU_NIATUAL_LIGDES    213
#define  MENU_CONF_NIVEL         220
#define   MENU_CONFNIV_SUMP      221
#define   MENU_CONFNIV_REPO      222
#define   MENU_CONFNIV_ATMAN     223
#define  MENU_VAZAMENTO          230
#define   MENU_VAZATMAN_SUMP     231
#define   MENU_VAZATMAN_REPO     232
#define   MENU_VAZATMAN_NAOUSADO 233 

#define MENU_ALARMES            300
#define  MENU_ALARME_GSM        310
#define   MENU_ALARMEGSM_LIGDES 311
#define   MENU_ALARMEGSM_CONFNU 312 
#define  MENU_ALARME_TESTE      320
#define   MENU_ENTRADA_DADOS    321

//outras definicoes
#define LCD_CONTRAST 49
#define LOOP_PADRAO_APP 100

//Inicializacao do menu, com o menu principal
int menuAtivo = MENU_PRINCIPAL;
int menuAnterior = MENU_PRINCIPAL;
int itemAtivo = 0;

//buffer utilizado para escrever no LCD
char * buf;
#define BUFFER_TEXT_SIZE 10

/****************************************************************************************
 *  Configuracoes para o leitor de IR
 * O sensor possui 3 pinos (Visto de frente)
 * 1 - Dados
 * 2 - GND
 * 3 - VCC
 ****************************************************************************************/

//Pino digital para o leitor de IR
#define RECV_PIN 8
IRrecv irrecv(RECV_PIN); 
decode_results results;

//Comandos do controle remoto
#define IR_PWD 16580863
#define IR_VOLUP 16613503
#define IR_FUNC 16597183
#define IR_BWD 16589023
#define IR_PLAY 16621663
#define IR_FWD 16605343
#define IR_DOWN 16584943
#define IR_VOLDW 16617583
#define IR_UP 16601263
#define IR_0 16593103
#define IR_EQ 16625743
#define IR_ST 16609423
#define IR_1 16582903
#define IR_2 16615543
#define IR_3 16599223
#define IR_4 16591063
#define IR_5 16623703
#define IR_6 16607383
#define IR_7 16586983
#define IR_8 16619623
#define IR_9 16603303

/****************************************************************************************
 *  Configuracoes para o leitor de Cartoes SD
 * O leitor de cartao utiliza uma API padrao de leitores SD, 
 * que no arduino mega utiliza os pinos SPI abaixo para acessar o leitor
 * MOSI 51
 * MISO 50
 * SCK 52
 * CS 53
 ****************************************************************************************/
#define CS 53
File myFile;

//arquivo de configuracao
char * fileName = "config.txt";

/****************************************************************************************
 *  Configuracoes para interface de entrada de dados
 * Sao suportados 4 modos, similar a um teclado de celular
 ****************************************************************************************/

#define NUMERICO 0
#define ALFA_1 1
#define ALFA_2 2
#define ALFA_3 3
#define ALFA_4 4

byte modo = NUMERICO;

//Demais configura˜es
byte resfLigaDes = 0;
float resfTemMax = 20.0f;
byte resfAtMan = 0;
byte aqueLigaDes = 0;
float aqueTemMin = 10.0f;
byte aqueAtMan = 0;
byte nivelAutoManual = 0;
float confiNivelSump = 30.0f;
float confiNivelRepo = 40.0f;
byte confNivelAutoManual = 0;
byte vazAutoManualSump = 0;
byte vazAutoManualRepo = 0;
byte alarmeGSM = 0;
char * alarmeGSMNum;
char * ddd = "11";
byte showCursor = FALSE;
byte countChars = 0;
byte pwdPress = 0;

//contadores
byte iCont;
float temperatura;
float temperaturaTampa;
float temperaturaAmb;
float distancia;
float distanciaRep;
byte nivelMax;
byte nivelMaxSump;
byte luzLigada;


char dataAtual[11];
char horaAtual[6];

//Configuracao do RTC
RTC_DS1307 RTC;

/**
 Funcao de inicializacao padrao do arduino
*/

void setup(void) 
{ 
  //Inicializacao do LCD
  nokia.init(LCD_CONTRAST);
  menuPrincipal();

  //Habilitando leitura do IR
  irrecv.enableIRIn();
  buf = (char*)malloc(BUFFER_TEXT_SIZE*sizeof(char));
  alarmeGSMNum = (char*)malloc(BUFFER_TEXT_SIZE*sizeof(char));

  strcpy(alarmeGSMNum,"123456789");
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  pinMode(CS, OUTPUT);
  if(!SD.begin(CS)) {
    return;
  }

  loadConfigSD();
  
  Wire.begin();//Inicializacao do protocolo wire
  RTC.begin();//Inicializacao do modulo RTC
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    //Ajusta a data/hora do Clock com a data/hora em que o codigo foi compilado, basta descomentar a linha
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

byte initGSM = 4;
void testeGSM()
{
   /*if(initGSM == 1)
   {
     Serial.write("GSM init \n");
     //delay(35000);
     //Serial.write("GSM init \n");
     initGSM = 2;
     //Serial2.println("AT");
     //Serial2.println("AT+CMGF=1");
     delay(500);
   }
   else if(initGSM == 2)
   {
     Serial.write("GSM send message \n");
     Serial2.print("AT+CMGS=");
     Serial2.write(34);
     Serial2.print("+5511985551116");
     Serial2.write(34);
     Serial2.println("");
     delay(500);
     Serial2.print("eu te amu tu");
     Serial2.write(26);
     Serial2.println("");
     delay(15000);
     initGSM = 3;
   }*/   

   if(initGSM == 4)
   {
     Serial2.print("AT+CMGL=");
     Serial2.write(34);
     Serial2.print("ALL");
     Serial2.write(34);
     Serial2.println("");
     delay(500);
     //Serial2.println("AT+CMGL=1");
     initGSM = 5;
   }
     while(Serial2.available())
     {
      Serial.write(Serial2.read());
     }

}

/**
 Funcao de loop padrao do arduino
*/

void loop()
{
  sensores();
  controlaMenus();
  //testeGSM();
 }

 byte intenSel = 0;
 void controlaMenus()
 {
    if (irrecv.decode(&results)) 
    {
      irrecv.resume(); // Receive the next value
      //Serial.print("#define IR_ ");
      //Serial.print(results.value, DEC);
      //Serial.print("\n");
      if(results.value == IR_PWD)
      {
        if(pwdPress == 0)
          pwdPress = 1;
        else
          pwdPress = 0;
      } 

      //Movimenta a selecao dos itens de menu
     if(menuAtivo != MENU_TEMP_AGUA && menuAtivo != MENU_TEMP_TAMPA && menuAtivo != MENU_TEMP_AMB
        && menuAtivo != MENU_RESF_LIGADES && menuAtivo != MENU_RESF_TEMPMAX && menuAtivo != MENU_RESF_AT_MAN && menuAtivo != MENU_AQUEC_LIGADES && menuAtivo != MENU_AQUEC_TEMPMIN
        && menuAtivo != MENU_AQUEC_AT_MAN && menuAtivo != MENU_NIATUAL_SUMP && menuAtivo != MENU_NIATUAL_REPO && menuAtivo != MENU_NIATUAL_LIGDES && menuAtivo != MENU_CONFNIV_SUMP && menuAtivo != MENU_CONFNIV_REPO
        && menuAtivo != MENU_CONFNIV_ATMAN && menuAtivo != MENU_VAZATMAN_SUMP && menuAtivo != MENU_VAZATMAN_REPO && menuAtivo != MENU_ALARMEGSM_LIGDES && menuAtivo != MENU_ALARMEGSM_CONFNU && menuAtivo != MENU_ALARME_TESTE
        && menuAtivo != MENU_ENTRADA_DADOS)
     {
        if(results.value == IR_DOWN) menuProx(); 
        else if(results.value == IR_UP) menuAnt();
        if(results.value == IR_FUNC)
        {
          intenSel = apiGraf.getSelItem();
          itemAtivo = 0;
          menuAnterior =  menuAtivo;

          switch(menuAtivo)
          {
            case MENU_PRINCIPAL:
            if(intenSel == 0) menuAtivo = MENU_TEMPERATURA;
            if(intenSel == 1) menuAtivo = MENU_NIVEL;
            if(intenSel == 2) menuAtivo = MENU_ALARMES;
            break;

            case MENU_TEMPERATURA:
            if(intenSel == 0) menuAtivo = MENU_VALOR_TEMP;
            if(intenSel == 1) menuAtivo = MENU_RESFRIAMENTO;
            if(intenSel == 2) menuAtivo = MENU_AQUECIMENTO;
            break;
            
            case MENU_VALOR_TEMP:
            if(intenSel == 0) menuAtivo = MENU_TEMP_AGUA;
            if(intenSel == 1) menuAtivo = MENU_TEMP_TAMPA;
            if(intenSel == 2) menuAtivo = MENU_TEMP_AMB;
            break;

            case MENU_RESFRIAMENTO:
            if(intenSel == 0) menuAtivo = MENU_RESF_LIGADES;
            if(intenSel == 1) menuAtivo = MENU_RESF_TEMPMAX;
            if(intenSel == 2) menuAtivo = MENU_RESF_AT_MAN;
            break;

            case MENU_AQUECIMENTO:
            if(intenSel == 0) menuAtivo = MENU_AQUEC_LIGADES;
            if(intenSel == 1) menuAtivo = MENU_AQUEC_TEMPMIN;
            if(intenSel == 2) menuAtivo = MENU_AQUEC_AT_MAN;
            break;

            case MENU_NIVEL:
            if(intenSel == 0) menuAtivo = MENU_NIVEL_ATUAL;
            if(intenSel == 1) menuAtivo = MENU_CONF_NIVEL;
            if(intenSel == 2) menuAtivo = MENU_VAZAMENTO;
            break;

            case MENU_NIVEL_ATUAL:
            if(intenSel == 0) menuAtivo = MENU_NIATUAL_SUMP;
            if(intenSel == 1) menuAtivo = MENU_NIATUAL_REPO;
            if(intenSel == 2) menuAtivo = MENU_NIATUAL_LIGDES;
            break;

            case MENU_CONF_NIVEL:
            if(intenSel == 0) menuAtivo = MENU_CONFNIV_SUMP;
            if(intenSel == 1) menuAtivo = MENU_CONFNIV_REPO;
            if(intenSel == 2) menuAtivo = MENU_CONFNIV_ATMAN;
            break;

            case MENU_VAZAMENTO:
            if(intenSel == 0) menuAtivo = MENU_VAZATMAN_SUMP;
            if(intenSel == 1) menuAtivo = MENU_VAZATMAN_REPO;
            //if(intenSel == 2) menuAtivo = MENU_VAZATMAN_NAOUSADO;
            break;

            case MENU_ALARMES:
            if(intenSel == 0) menuAtivo = MENU_ALARME_GSM;
            if(intenSel == 1) menuAtivo = MENU_ALARME_TESTE;
            //if(intenSel == 2) menuAtivo = MENU_ALARMEGSM_CONFNU;
            break;

            case MENU_ALARME_GSM:
            if(intenSel == 0) menuAtivo = MENU_ALARMEGSM_LIGDES;
            if(intenSel == 1) menuAtivo = MENU_ALARMEGSM_CONFNU;
            break;

            default: 
              menuAtivo = MENU_PRINCIPAL;
              menuAnterior =  menuAtivo;
          }

        }//if(results.value == IR_FUNC)
        if(results.value == IR_ST)
        {
          menuAtivo = menuAnterior;
        }
     }
     else if( menuAtivo == MENU_ENTRADA_DADOS)
     {
        if(results.value == IR_FUNC)
         {
            menuAtivo = menuAnterior;
         }
         else
         {
           if(results.value == IR_PLAY)
           {
             if(menuAnterior == MENU_RESF_TEMPMAX)
             {
               if(validaTempNivel())
               {
                 resfTemMax = atof(buf);
                 saveConfig();
                 menuAtivo = menuAnterior;
               }
             }
             if(menuAnterior == MENU_AQUEC_TEMPMIN)
             {
               if(validaTempNivel())
               {
                 aqueTemMin = atof(buf);
                 saveConfig();
                 menuAtivo = menuAnterior;
               }
             }
             if(menuAnterior == MENU_CONFNIV_SUMP)
             {
               if(validaTempNivel())
               {
                 confiNivelSump = atof(buf);
                 saveConfig();
                 menuAtivo = menuAnterior;
               }
             }
             if(menuAnterior == MENU_CONFNIV_REPO)
             {
               if(validaTempNivel())
               {
                 confiNivelRepo = atof(buf);
                 saveConfig();
                 menuAtivo = menuAnterior;
               }
             }
             if(menuAnterior == MENU_ALARMEGSM_CONFNU)
             {
               if(validaNumeroCel())
               {
                 strcpy(alarmeGSMNum,buf);
                 saveConfig();
                 menuAtivo = menuAnterior;
               }
             }
           }
           else montaBufferChar(results.value);
         }
      }
      else
      {
        if(results.value == IR_FUNC) 
          menuAtivo = menuAnterior;
        else 
        {
          if(results.value == IR_ST)
          {
             if(menuAtivo == MENU_RESF_LIGADES ){ resfLigaDes == 0?resfLigaDes=1:resfLigaDes=0; saveConfig();}
             if(menuAtivo == MENU_RESF_AT_MAN ){ resfAtMan == 0?resfAtMan=1:resfAtMan=0; saveConfig();}
             if(menuAtivo == MENU_AQUEC_LIGADES ){ aqueLigaDes == 0?aqueLigaDes=1:aqueLigaDes=0; saveConfig();}
             if(menuAtivo == MENU_AQUEC_AT_MAN ){ aqueAtMan == 0?aqueAtMan=1:aqueAtMan=0; saveConfig();}
             if(menuAtivo == MENU_NIATUAL_LIGDES ){ nivelAutoManual == 0?nivelAutoManual=1:nivelAutoManual=0; saveConfig();}
             if(menuAtivo == MENU_CONFNIV_ATMAN ){ confNivelAutoManual == 0?confNivelAutoManual=1:confNivelAutoManual=0; saveConfig();}
             if(menuAtivo == MENU_VAZATMAN_SUMP ){ vazAutoManualSump == 0?vazAutoManualSump=1:vazAutoManualSump=0; saveConfig();}
             if(menuAtivo == MENU_VAZATMAN_REPO ){ vazAutoManualRepo == 0?vazAutoManualRepo=1:vazAutoManualRepo=0; saveConfig();}
             if(menuAtivo == MENU_ALARMEGSM_LIGDES ){ alarmeGSM == 0?alarmeGSM=1:alarmeGSM=0; saveConfig();}

             if(menuAtivo == MENU_RESF_TEMPMAX){ menuAnterior = MENU_RESF_TEMPMAX; menuAtivo = MENU_ENTRADA_DADOS; countChars = 0; showCursor = FALSE;}
             if(menuAtivo == MENU_AQUEC_TEMPMIN){ menuAnterior = MENU_AQUEC_TEMPMIN; menuAtivo = MENU_ENTRADA_DADOS; countChars = 0; showCursor = FALSE;}
             if(menuAtivo == MENU_CONFNIV_SUMP){ menuAnterior = MENU_CONFNIV_SUMP; menuAtivo = MENU_ENTRADA_DADOS; countChars = 0; showCursor = FALSE;}
             if(menuAtivo == MENU_CONFNIV_REPO){ menuAnterior = MENU_CONFNIV_REPO; menuAtivo = MENU_ENTRADA_DADOS; countChars = 0; showCursor = FALSE;}
             if(menuAtivo == MENU_ALARMEGSM_CONFNU){ menuAnterior = MENU_ALARMEGSM_CONFNU; menuAtivo = MENU_ENTRADA_DADOS; countChars = 0; showCursor = FALSE;}
             
          }
        }
      }
      results.value = 0;
      switch(menuAtivo)
      {
            case MENU_TEMPERATURA: menuAnterior = MENU_PRINCIPAL; menuTemperatura(); break;
            case MENU_VALOR_TEMP: menuAnterior = MENU_TEMPERATURA; menuValTemp(); break;
            case MENU_TEMP_AGUA: menuAnterior = MENU_VALOR_TEMP; menuValTempAgua(); break;
            case MENU_TEMP_TAMPA: menuAnterior = MENU_VALOR_TEMP; menuValTempTampa(); break;
            case MENU_TEMP_AMB: menuAnterior = MENU_VALOR_TEMP; menuValTempAmb(); break;
            case MENU_RESFRIAMENTO: menuAnterior = MENU_TEMPERATURA; menuResfriamento(); break;
            case MENU_RESF_LIGADES: menuAnterior = MENU_RESFRIAMENTO; menuResfLigaDes(); break;
            case MENU_RESF_TEMPMAX: menuAnterior = MENU_RESFRIAMENTO; menuResfTempMax(); break;
            case MENU_RESF_AT_MAN: menuAnterior = MENU_RESFRIAMENTO; menuResfAtMan(); break;
            case MENU_AQUECIMENTO: menuAnterior = MENU_TEMPERATURA; menuAquecimento(); break;
            case MENU_AQUEC_LIGADES: menuAnterior = MENU_AQUECIMENTO; menuAquecLigaDes(); break;
            case MENU_AQUEC_TEMPMIN: menuAnterior = MENU_AQUECIMENTO; menuAquecTemMin(); break;
            case MENU_AQUEC_AT_MAN: menuAnterior = MENU_AQUECIMENTO; menuAquecAtMan(); break;
            case MENU_NIVEL: menuAnterior = MENU_PRINCIPAL; menuNivel(); break;
            case MENU_NIVEL_ATUAL: menuAnterior = MENU_NIVEL; menuNivelAtual(); break;
            case MENU_NIATUAL_SUMP: menuAnterior = MENU_NIVEL_ATUAL; menuNivAtualSump(); break;
            case MENU_NIATUAL_REPO: menuAnterior = MENU_NIVEL_ATUAL; menuNivAtualRepo(); break;
            case MENU_NIATUAL_LIGDES: menuAnterior = MENU_NIVEL_ATUAL; menuNivAtualLigDes(); break;
            case MENU_CONF_NIVEL: menuAnterior = MENU_NIVEL; menuConfNivel(); break;
            case MENU_CONFNIV_SUMP: menuAnterior = MENU_CONF_NIVEL; menuConfNivelSump(); break;
            case MENU_CONFNIV_REPO: menuAnterior = MENU_CONF_NIVEL; menuConfNivelRepo(); break;
            case MENU_CONFNIV_ATMAN: menuAnterior = MENU_CONF_NIVEL; menuConfNivelAtMan(); break;
            case MENU_VAZAMENTO: menuAnterior = MENU_NIVEL; menuVazamento(); break;
            case MENU_VAZATMAN_SUMP: menuAnterior = MENU_VAZAMENTO; menuVazamentoSump(); break;
            case MENU_VAZATMAN_REPO: menuAnterior = MENU_VAZAMENTO; menuVazamentoRepo(); break;
            case MENU_ALARMES: menuAnterior = MENU_PRINCIPAL; menuAlarme(); break;
            case MENU_ALARME_GSM: menuAnterior = MENU_ALARMES; menuAlarmeGSM(); break;
            case MENU_ALARMEGSM_LIGDES: menuAnterior = MENU_ALARME_GSM; menuAlarmeGSMLigDes(); break;
            case MENU_ALARMEGSM_CONFNU: menuAnterior = MENU_ALARME_GSM;  menuAlarmeGSMConfNu(); break;
            case MENU_ALARME_TESTE: menuAnterior = MENU_ALARMES; menuTeste(); break;
            case MENU_ENTRADA_DADOS: menuEntradaDados();break;
            
            default: menuPrincipal();
     }//switch(menuAtivo)  
  }//if (irrecv.decode(&results)) 
  else
  {
    delay(3000);
  }
  
  if(pwdPress == 1)
  {
     desenhaDataHora(); 
     delay(1000);
  }
  else
  {  switch(menuAtivo)
    {
      case MENU_TEMP_AGUA: menuAnterior = MENU_VALOR_TEMP; menuValTempAgua(); break;
      case MENU_TEMP_TAMPA: menuAnterior = MENU_VALOR_TEMP; menuValTempTampa(); break;
      case MENU_TEMP_AMB: menuAnterior = MENU_VALOR_TEMP; menuValTempAmb(); break;
      case MENU_NIATUAL_SUMP: menuAnterior = MENU_NIVEL_ATUAL; menuNivAtualSump(); break;
      case MENU_NIATUAL_REPO: menuAnterior = MENU_NIVEL_ATUAL; menuNivAtualRepo(); break;
    }
  }
  
 }

/**
Funcao que valida se valor que esta na variavel buf eh uma temperatura valida
*/
int validaTempNivel()
{
   for(iCont = 0; iCont < countChars; iCont++)
   {
     if(buf[iCont]<48 )
     {
       if(buf[iCont] != 46)
       return 0;
     }
     if(buf[iCont]>57 )
       return 0;
     if(buf[iCont] == 0)
       return 1;
   }

   buf[countChars] = 0;
   return 1;
 }

/**
 Funcao que valida se valor que esta na variavel buf eh um numero de telefone valido
*/
int validaNumeroCel()
{
   for(iCont = 0; iCont < countChars; iCont++)
   {
     if(buf[iCont]<48 )
       return 0;
     if(buf[iCont]>57 )
       return 0;
     if(buf[iCont] == 0)
       return 1;
   }
   buf[countChars] = 0;
   return 1;
 }

/**
 Funcoes de que desenha os menus 
*/
void menuPrincipal()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("AquaMonitor",3); 
   apiGraf.setItem(0,"Temperatura");
   apiGraf.setItem(1,"Nivel Agua");
   apiGraf.setItem(2,"Alarmes");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuTemperatura()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Temperatura",3); 
   apiGraf.setItem(0,"Valor Atual");
   apiGraf.setItem(1,"Resfriamento");
   apiGraf.setItem(2,"Aquecimento");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuValTemp()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Temperatura",3); 
   apiGraf.setItem(0,"Agua");
   apiGraf.setItem(1,"Tampa");
   apiGraf.setItem(2,"Ambiente");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuResfriamento()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Resfriamento",3); 
   apiGraf.setItem(0,"Ligar/Desl");
   apiGraf.setItem(1,"Temp Max");
   apiGraf.setItem(2,"Automa/Manu");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuAquecimento()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Aquecimento",3); 
   apiGraf.setItem(0,"Ligar/Desl");
   apiGraf.setItem(1,"Temp Min");
   apiGraf.setItem(2,"Automa/Manu");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuNivel()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Nivel",3); 
   apiGraf.setItem(0,"Nivel Atual");
   apiGraf.setItem(1,"Configuracoes");
   apiGraf.setItem(2,"Vazamentos");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuNivelAtual()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Nivel Atual",3); 
   apiGraf.setItem(0,"Sump");
   apiGraf.setItem(1,"Reposicao");
   apiGraf.setItem(2,"Ligar/Des");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuConfNivel()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Conguracoes",3); 
   apiGraf.setItem(0,"Min Sump");
   apiGraf.setItem(1,"Min Repo");
   apiGraf.setItem(2,"Autom/Man rep");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuVazamento()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Vazamento",3); 
   apiGraf.setItem(0,"Auto/Man sump");
   apiGraf.setItem(1,"Auto/Man rep");
   apiGraf.setItem(2,"");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuAlarme()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Alarmes",3); 
   apiGraf.setItem(0,"GSM");
   apiGraf.setItem(1,"TESTES");
   apiGraf.setItem(2,"");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuAlarmeGSM()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("GSM",3); 
   apiGraf.setItem(0,"Ligar/Desli");
   apiGraf.setItem(1,"Nunero");
   apiGraf.setItem(2,"");
   apiGraf.selItem(itemAtivo);
   apiGraf.setTextLeftBTN("Ok");
   apiGraf.setTextRightBTN("Voltar");
   apiGraf.drawMenu();
   apiGraf.updateDisplay();
}

void menuResfLigaDes()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Resf manual",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");

   if(resfLigaDes == 1)
     apiGraf.setTextRightBTN("Des");
   else
     apiGraf.setTextRightBTN("Lig");

   apiGraf.drawMenu();
   if(resfLigaDes == 1)
     nokia.drawstring(0,20,"Ligado");
   else  
     nokia.drawstring(0,20,"Desligado");
   apiGraf.updateDisplay();
}

void menuResfTempMax()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Temp Max",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.setTextRightBTN("Conf");
   apiGraf.drawMenu();
   
   dtostrf(resfTemMax, 4, 2, buf);

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf); 
   apiGraf.updateDisplay();
}

 void menuResfAtMan()
 {
   apiGraf.clearDisplay();
   apiGraf.createMenu("Resf automati",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   if(resfAtMan == 1)
     apiGraf.setTextRightBTN("At");
   else
     apiGraf.setTextRightBTN("Man");
   apiGraf.drawMenu();

   if(resfAtMan == 0)
     nokia.drawstring(0,20,"Manual");
   else
     nokia.drawstring(0,20,"Automatico");
    
   apiGraf.updateDisplay();
}

void menuAquecLigaDes()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Aquec manual",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   
   if(aqueLigaDes == 1)
     apiGraf.setTextRightBTN("Des");
   else
     apiGraf.setTextRightBTN("Lig");
   apiGraf.drawMenu();
   if(aqueLigaDes == 1)
     nokia.drawstring(0,20,"Ligado");
   else
     nokia.drawstring(0,20,"Desligado");
   apiGraf.updateDisplay();
}

void menuAquecTemMin()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Temp Min",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.setTextRightBTN("Conf");
   apiGraf.drawMenu();

   dtostrf(aqueTemMin, 4, 2, buf);

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf); 

   apiGraf.updateDisplay();
}

 void menuAquecAtMan()
 {
   apiGraf.clearDisplay();
   apiGraf.createMenu("Aquec auto",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");

   if(aqueAtMan==0)
     apiGraf.setTextRightBTN("At");
   else
     apiGraf.setTextRightBTN("Man");
     
   apiGraf.drawMenu();
   
   if(aqueAtMan==0)
     nokia.drawstring(0,20,"Manual");
   else
     nokia.drawstring(0,20,"Automatico");
   apiGraf.updateDisplay();
}

void menuNivAtualRepo()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Nivel reps",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   //apiGraf.setTextRightBTN("At");
   apiGraf.drawMenu();

   dtostrf(distanciaRep, 4, 2, buf);

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf); 

   apiGraf.updateDisplay();
}

void menuNivAtualLigDes()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Bomba rep",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");

   if(nivelAutoManual==0)
     apiGraf.setTextRightBTN("Lig");
   else
     apiGraf.setTextRightBTN("Des");   
   apiGraf.drawMenu();
 
   if(nivelAutoManual==0)
     nokia.drawstring(0,20,"Desligada");
   else
     nokia.drawstring(0,20,"Ligado");

   apiGraf.updateDisplay();
}

void menuConfNivelSump()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Minimo sump",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.setTextRightBTN("Conf");
   apiGraf.drawMenu();
   
   dtostrf(confiNivelSump, 4, 2, buf);
   
   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf); 

   apiGraf.updateDisplay();
}

void menuConfNivelRepo()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Minimo repo",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.setTextRightBTN("Conf");
   apiGraf.drawMenu();

   dtostrf(confiNivelRepo, 4, 2, buf);

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf); 
   
   apiGraf.updateDisplay();
}

void menuConfNivelAtMan()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Repos auto",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   
   if(confNivelAutoManual == 0)
     apiGraf.setTextRightBTN("Aut");
   else
     apiGraf.setTextRightBTN("Man");
   
   apiGraf.drawMenu();
   if(confNivelAutoManual == 0)
     nokia.drawstring(0,20,"Manual");
   else
     nokia.drawstring(0,20,"Automatico");
   
   apiGraf.updateDisplay();
}

void menuVazamentoSump()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Maximo Sump",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
  
   if(vazAutoManualSump == 0)
     apiGraf.setTextRightBTN("Lig");
   else  
     apiGraf.setTextRightBTN("Des");
   apiGraf.drawMenu();

   if(vazAutoManualSump == 1)
     nokia.drawstring(0,20,"Ligado");
   else
     nokia.drawstring(0,20,"Desligado");

   apiGraf.updateDisplay();
}

void menuVazamentoRepo()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Maximo Sump",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");

   if(vazAutoManualRepo == 0)
     apiGraf.setTextRightBTN("Lig");
   else  
     apiGraf.setTextRightBTN("Des");

   apiGraf.drawMenu();

   if(vazAutoManualRepo == 1)
     nokia.drawstring(0,20,"Ligado");
   else
     nokia.drawstring(0,20,"Desligado");

   apiGraf.updateDisplay();
}

void menuAlarmeGSMLigDes()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Monitor GSM",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");

   if(alarmeGSM == 0)
     apiGraf.setTextRightBTN("Lig");
   else  
     apiGraf.setTextRightBTN("Des");

   apiGraf.drawMenu();

   if(alarmeGSM == 1)
     nokia.drawstring(0,20,"Ligado");
   else
     nokia.drawstring(0,20,"Desligado");

   apiGraf.updateDisplay();
}

void menuAlarmeGSMConfNu()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Nuemero GSM",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.setTextRightBTN("Conf");
   apiGraf.drawMenu();

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,alarmeGSMNum); 
   apiGraf.updateDisplay();
}

void menuValTempAgua()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Temperatura",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.drawMenu();

   dtostrf(temperatura, 4, 2, buf);

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf);

   apiGraf.updateDisplay();
}

void menuValTempTampa()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Temperatura",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.drawMenu();

   dtostrf(temperaturaTampa, 4, 2, buf);

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf);

   apiGraf.updateDisplay();
}

void menuValTempAmb()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Temperatura",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.drawMenu();

   dtostrf(temperaturaAmb, 4, 2, buf);

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf);

   apiGraf.updateDisplay();
}

void menuNivAtualSump()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Nivel Sump",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.drawMenu();

   dtostrf(distancia, 4, 2, buf);

   nokia.setTextColor(BLACK);
   nokia.drawstring(0,20,buf); 

   apiGraf.updateDisplay();
}

void desenhaDataHora()
{
  DateTime now = RTC.now();
  char cTemp[5];
  int iTemp;
   apiGraf.clearDisplay();
   nokia.setTextColor(BLACK);
   
   iTemp = now.day();
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     buf[0] = '0';
     buf[1] = cTemp[0];
   }
   else
   {
     buf[0] = cTemp[0];
     buf[1] = cTemp[1];
   }
   buf[2] = '/';
   iTemp = now.month();
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     buf[3] = '0';
     buf[4] = cTemp[0];
   }
   else
   {
     buf[3] = cTemp[0];
     buf[4] = cTemp[1];
   }
   buf[5] = '/';
   iTemp = now.year();
   sprintf(cTemp, "%d", iTemp);
   buf[6] = cTemp[0];
   buf[7] = cTemp[1];
   buf[8] = cTemp[2];
   buf[9] = cTemp[3];
   buf[10] = '\0';
   nokia.drawstring(5+7,10,buf);
   
   iTemp = now.hour();
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     buf[0] = '0';
     buf[1] = cTemp[0];
   }
   else
   {
     buf[0] = cTemp[0];
     buf[1] = cTemp[1];
   }
   buf[2] = ':';
   iTemp = now.minute();
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     buf[3] = '0';
     buf[4] = cTemp[0];
   }
   else
   {
     buf[3] = cTemp[0];
     buf[4] = cTemp[1];
   }
   buf[5] = ':';
   iTemp = now.second();
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     buf[6] = '0';
     buf[7] = cTemp[0];
   }
   else
   {
     buf[6] = cTemp[0];
     buf[7] = cTemp[1];
   }
   buf[8] = '\0';
   nokia.drawstring(9+7,20,buf);
  
   apiGraf.updateDisplay();

}

void menuTeste()
{
   //apiGraf.clearDisplay();
  /*apiGraf.createMenu("Monitor",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   //apiGraf.setTextRightBTN("btnR");
   apiGraf.drawMenu();
    /*sensor = analogRead(SENSOR_MOV1);
    nokia.setTextColor(BLACK);
    if(sensor > 200)
      nokia.drawstring(0,20,"S1 Inc!!");
    else
      nokia.drawstring(0,20,"S1 Sem Inc");*/
    //apiGraf.updateDisplay();

    //menuEntradaDados();
}

/**
 Funcao que desenha o proximo item de menu selecionado no menu
*/
void menuProx()
{
    apiGraf.nextItem();
    itemAtivo = apiGraf.getSelItem();
}

/**
 Funcao que desenha o item de menu anterior selecionado no menu
*/ 
void menuAnt()
{
    apiGraf.previousItem();
    itemAtivo = apiGraf.getSelItem();
}

/**
 Funcoes que desenham e controlam a entrada de dados
*/ 
void menuEntradaDados()
{
   apiGraf.clearDisplay();
   apiGraf.createMenu("Teclado",3); 
   apiGraf.setItem(0,"");
   apiGraf.setItem(1,"");
   apiGraf.setItem(2,"");
   apiGraf.selItem(-1);
   apiGraf.setTextLeftBTN("Voltar");
   apiGraf.setTextRightBTN("Salvar");
   apiGraf.drawMenu();

   if(showCursor == TRUE && countChars < BUFFER_TEXT_SIZE-1)
   {
     buf[countChars]='_';
     showCursor = FALSE;
   }
   else if(showCursor == FALSE && countChars < BUFFER_TEXT_SIZE-1)
   {
      buf[countChars]=' ';
      showCursor = TRUE;  
   }
   buf[countChars+1]=0;

   nokia.setTextColor(BLACK);
   if(modo == NUMERICO) nokia.drawstring(0,15,"Modo 1");
   if(modo == ALFA_1) nokia.drawstring(0,15,"Modo a");
   if(modo == ALFA_2) nokia.drawstring(0,15,"Modo b");
   if(modo == ALFA_3) nokia.drawstring(0,15,"Modo c");
   if(modo == ALFA_4) nokia.drawstring(0,15,"Modo s/z");
   nokia.drawstring(0,25,buf); 
   
   apiGraf.updateDisplay();

   delay(200);
 }

//Monta a variavel global buf
void montaBufferChar(unsigned long IRCode)
{
  if(IRCode == IR_ST)
  {
    modo++;
    if(modo > ALFA_4)
      modo = 0;
  }
  
  if(IRCode == IR_EQ && countChars > 0)
  {
    buf[countChars] = 0;
    countChars--;
    return;
  }

  char cTemp = getCharterOfIR(IRCode,modo);
  if(cTemp >= 0 && countChars < BUFFER_TEXT_SIZE)
  {
    buf[countChars]=cTemp;
    countChars++; 
  }
}

//Retorna o caractere de acordo com o codigo do IR e modo selecionado
char getCharterOfIR(unsigned long IRCode, byte mode)
{
  switch(mode)
  {
    case NUMERICO:  return getNumerico(IRCode);
    case ALFA_1:  return getAlfa1(IRCode);
    case ALFA_2:  return getAlfa2(IRCode);
    case ALFA_3:  return getAlfa3(IRCode);
    case ALFA_4:  return getAlfa4(IRCode);
  }
}

//Retorna o numero correspondente ao IR
char getNumerico(unsigned long IRCode)
{
    switch(IRCode)
    {
      case IR_0: return '0';
      case IR_1: return '1';
      case IR_2: return '2';
      case IR_3: return '3';
      case IR_4: return '4';
      case IR_5: return '5';
      case IR_6: return '6';
      case IR_7: return '7';
      case IR_8: return '8';
      case IR_9: return '9';
      default: return -1;
    }
}

//Retorna o caractere correspondente ao IR ao modo 1
char getAlfa1(unsigned long IRCode)
{
    switch(IRCode)
    {
      case IR_0: return ' ';
      case IR_1: return '.';
      case IR_2: return 'a';
      case IR_3: return 'd';
      case IR_4: return 'g';
      case IR_5: return 'j';
      case IR_6: return 'm';
      case IR_7: return 'p';
      case IR_8: return 't';
      case IR_9: return 'w';
      default: return getNumerico(IRCode);
    }
}

//Retorna o caractere correspondente ao IR ao modo 2
char getAlfa2(unsigned long IRCode)
{
    switch(IRCode)
    {
      case IR_2: return 'b';
      case IR_3: return 'e';
      case IR_4: return 'h';
      case IR_5: return 'k';
      case IR_6: return 'n';
      case IR_7: return 'q';
      case IR_8: return 'u';
      case IR_9: return 'x';
      default: return getNumerico(IRCode);
    }
}

//Retorna o caractere correspondente ao IR ao modo 3
char getAlfa3(unsigned long IRCode)
{
    switch(IRCode)
    {
      case IR_2: return 'c';
      case IR_3: return 'f';
      case IR_4: return 'i';
      case IR_5: return 'l';
      case IR_6: return 'o';
      case IR_7: return 'r';
      case IR_8: return 'v';
      case IR_9: return 'y';
      default: return getNumerico(IRCode);
    }
}

//Retorna o caractere correspondente ao IR ao modo 4
char getAlfa4(unsigned long IRCode)
{
    switch(IRCode)
    {
      case IR_7: return 's';
      case IR_9: return 'z';
      default: return getNumerico(IRCode);
    }
}

/**
 Funcao que controla a chamada aos sensores
*/
char s;
byte state = 0;
char comando;
char valorComando[6];
byte posicaoLeitura;
byte tamanhoComando;
#define CMD_INICIO 0
#define CMD_LENDO 1
#define CMD_FINALIZADO 2
#define CMD_MAX_TAM 6

#define CMD_L_TEMP_AGUA 'T' 
#define CMD_L_TEMP_TAMPA 'E' 
#define CMD_L_TEMP_AMB 'A' 
#define CMD_L_NIVEL_SUMP 'S' 
#define CMD_L_NIVEL_REPO 'R' 
#define CMD_L_NIVEL_MAXI 'N' 
#define CMD_L_LUZ_LIGADA 'L'

void sensores()
{
	while(Serial2.available())
	{
		s = Serial2.read();
		if(state == CMD_INICIO)
		{
			comando = s;
			state = CMD_LENDO;
			posicaoLeitura = 1;
			if(s == CMD_L_TEMP_AGUA || s == CMD_L_TEMP_TAMPA || s == CMD_L_TEMP_AMB || s == CMD_L_NIVEL_SUMP || s == CMD_L_NIVEL_REPO)
			{
				tamanhoComando = CMD_MAX_TAM;
				valorComando[5]='\0';
			}
			if(s == CMD_L_NIVEL_MAXI || s == CMD_L_LUZ_LIGADA)
			{
				tamanhoComando = CMD_MAX_TAM;
				valorComando[5]='\0';
			}
			
		}
		else if(state == CMD_LENDO && posicaoLeitura <= tamanhoComando)
		{
			valorComando[posicaoLeitura-1] = s;
			posicaoLeitura++;
		}
		else
		{
			posicaoLeitura = 0;
			state = CMD_FINALIZADO;
		}
		
	}
	
	if(state == CMD_FINALIZADO)
	{
		state = CMD_INICIO;
	        switch(comando)
		{
			case CMD_L_TEMP_AGUA: 
				temperatura = atof(valorComando);
                                Serial3.print(CMD_L_TEMP_AGUA);
                                Serial3.print(valorComando[0]);
                                Serial3.print(valorComando[1]);
                                Serial3.print(valorComando[2]);
                                Serial3.print(valorComando[3]);
                                Serial3.print(valorComando[4]);
			break;
			case CMD_L_TEMP_TAMPA: 
				temperaturaTampa = atof(valorComando);
                                Serial3.print(CMD_L_TEMP_TAMPA);
                                Serial3.print(valorComando[0]);
                                Serial3.print(valorComando[1]);
                                Serial3.print(valorComando[2]);
                                Serial3.print(valorComando[3]);
                                Serial3.print(valorComando[4]);
			break;
			case CMD_L_TEMP_AMB: 
				temperaturaAmb = atof(valorComando);
                                Serial3.print(CMD_L_TEMP_AMB);
                                Serial3.print(valorComando[0]);
                                Serial3.print(valorComando[1]);
                                Serial3.print(valorComando[2]);
                                Serial3.print(valorComando[3]);
                                Serial3.print(valorComando[4]);
			break;
			case CMD_L_NIVEL_SUMP: 
				distancia = atof(valorComando);
                                Serial3.print(CMD_L_NIVEL_SUMP);
                                Serial3.print(valorComando[0]);
                                Serial3.print(valorComando[1]);
                                Serial3.print('0');
                                Serial3.print('0');
                                Serial3.print('0');

			break;
			case CMD_L_NIVEL_REPO: 
				distanciaRep = atof(valorComando);
                                Serial3.print(CMD_L_NIVEL_REPO);
                                Serial3.print(valorComando[0]);
                                Serial3.print(valorComando[1]);
                                Serial3.print('0');
                                Serial3.print('0');
                                Serial3.print('0');

			break;
			case CMD_L_NIVEL_MAXI: 
                                Serial3.print(CMD_L_NIVEL_MAXI);
                                Serial3.print('0');
				if(valorComando[1] == '1'){
				  nivelMax = 1;
                                  Serial3.print('1');
                                }
                                else{
                                  nivelMax = 0;
                                  Serial3.print('0');
                                }
                                
                                Serial3.print('0');
                                Serial3.print('0');
                                Serial3.print('0');
			break;
			case CMD_L_LUZ_LIGADA:
                                Serial3.print(CMD_L_LUZ_LIGADA);
                                Serial3.print('0');
				if(valorComando[1] == '1'){
				  luzLigada = 1;
                                  Serial3.print('1');
                                }
                                else{
                                  luzLigada = 0;
                                  Serial3.print('0');
                                }
                                Serial3.print('0');
                                Serial3.print('0');
                                Serial3.print('0');
			break;
		}
	}

  carregaDataHora();

}
int horaAnterior = -1;
int minuto = 0;
int intervalo = 2;
int enviar = 0;

void carregaDataHora()
{
  DateTime now = RTC.now();
  char cTemp[5];
  int iTemp;
   
   iTemp = now.day();
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     dataAtual[0] = '0';
     dataAtual[1] = cTemp[0];
   }
   else
   {
     dataAtual[0] = cTemp[0];
     dataAtual[1] = cTemp[1];
   }
   dataAtual[2] = '/';
   iTemp = now.month();
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     dataAtual[3] = '0';
     dataAtual[4] = cTemp[0];
   }
   else
   {
     dataAtual[3] = cTemp[0];
     dataAtual[4] = cTemp[1];
   }
   dataAtual[5] = '/';
   iTemp = now.year();
   sprintf(cTemp, "%d", iTemp);
   dataAtual[6] = cTemp[0];
   dataAtual[7] = cTemp[1];
   dataAtual[8] = cTemp[2];
   dataAtual[9] = cTemp[3];
   dataAtual[10] = '\0';
   
   iTemp = now.hour();
   if(horaAnterior == -1)
     horaAnterior = iTemp;
   if(horaAnterior != iTemp){
     minuto = 0;
     horaAnterior = iTemp;
   }
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     horaAtual[0] = '0';
     horaAtual[1] = cTemp[0];
   }
   else
   {
     horaAtual[0] = cTemp[0];
     horaAtual[1] = cTemp[1];
   }
   horaAtual[2] = ':';
   iTemp = now.minute();
   if(iTemp - minuto >= intervalo)
   {
     enviar = 1;
     minuto = iTemp;
   }
   sprintf(cTemp, "%d", iTemp);
   if(iTemp < 10)
   {
     horaAtual[3] = '0';
     horaAtual[4] = cTemp[0];
   }
   else
   {
     horaAtual[3] = cTemp[0];
     horaAtual[4] = cTemp[1];
   }
   buf[5] = '\0';
   
   if(enviar == 1)
   {
     enviaPorWeb();
     enviar = 0;
   }

}

void enviaPorWeb()
{
    Serial3.print('D');
  Serial3.print(dataAtual[0]);
  Serial3.print(dataAtual[1]);
  Serial3.print(dataAtual[2]);
  Serial3.print(dataAtual[3]);
  Serial3.print(dataAtual[4]);
  Serial3.print(dataAtual[5]);
  Serial3.print(dataAtual[6]);
  Serial3.print(dataAtual[7]);
  Serial3.print(dataAtual[8]);
  Serial3.print(dataAtual[9]);
  
  Serial3.print('H');
  Serial3.print(horaAtual[0]);
  Serial3.print(horaAtual[1]);
  Serial3.print(horaAtual[2]);
  Serial3.print(horaAtual[3]);
  Serial3.print(horaAtual[4]);
  
  Serial3.print('W');

}

 
/**
 Funcao que carrega as configuracoes da aplicacao, que estao no SD
*/

void loadConfigSD()
{ 

  if (!SD.exists(fileName)) 
  {
    return;
  }

  myFile = SD.open(fileName, FILE_READ);

  if (myFile) 
  {
   char tmp;
   char tmp1[6];
   char tmp2[10];
   byte k;

   if(myFile.available())
   {
     resfLigaDes = 1;
     tmp = myFile.read();myFile.read();
     //Serial.print("resfLigaDes=");Serial.print(tmp);Serial.print("\n");
     if(tmp == '0')
       resfLigaDes = 0;
       
     for(k=0; k<=5;k++)
       tmp1[k] = myFile.read();
       
     tmp1[k-1] = 0;
     resfTemMax = atof(tmp1);  
     //Serial.print("resfTemMax=");Serial.print(tmp1);Serial.print("\n");

     resfAtMan = 1;

     tmp = myFile.read();myFile.read();
     if(tmp == '0')
       resfAtMan = 0;
     //Serial.print("resfAtMan=");Serial.print(tmp);Serial.print("\n");
     aqueLigaDes = 1;
     tmp = myFile.read();myFile.read();
     //Serial.print("aqueLigaDes=");Serial.print(tmp);Serial.print("\n");
     if(tmp == '0')
       aqueLigaDes = 0;

     for(k=0; k<=5;k++)
       tmp1[k] = myFile.read();

     tmp1[k-1] = 0;
     aqueTemMin = atof(tmp1);  
     //Serial.print("aqueTemMin=");Serial.print(tmp1);Serial.print("\n");
     aqueAtMan = 1;
     tmp = myFile.read();myFile.read();
     if(tmp == '0')
       aqueAtMan = 0;  
     //Serial.print("aqueAtMan=");Serial.print(tmp);Serial.print("\n");
     nivelAutoManual = 1;
     tmp = myFile.read();myFile.read();
     //Serial.print("nivelAutoManual=");Serial.print(tmp);Serial.print("\n");
     if(tmp == '0')
       nivelAutoManual = 0;
     
     for(k=0; k<=5;k++)
       tmp1[k] = myFile.read();
       
     tmp1[k-1] = 0;  
     confiNivelSump = atof(tmp1);  
     //Serial.print("confiNivelSump=");Serial.print(tmp1);Serial.print("\n");
     
     for(k=0; k<=5;k++)
       tmp1[k] = myFile.read();

     tmp1[k-1] = 0;
     confiNivelRepo = atof(tmp1);  
     //Serial.print("confiNivelRepo=");Serial.print(tmp1);Serial.print("\n");

     confNivelAutoManual = 1;
     tmp = myFile.read();myFile.read();
     if(tmp == '0')
       confNivelAutoManual = 0;
     //Serial.print("confNivelAutoManual=");Serial.print(tmp);Serial.print("\n");

     vazAutoManualSump = 1;
     tmp = myFile.read();myFile.read();
     if(tmp == '0')
       vazAutoManualSump = 0;  
     //Serial.print("vazAutoManualSump=");Serial.print(tmp);Serial.print("\n");

     vazAutoManualRepo = 1;
     tmp = myFile.read();myFile.read();
     if(tmp == '0')
       vazAutoManualRepo = 0;
     //Serial.print("vazAutoManualRepo=");Serial.print(tmp);Serial.print("\n");
     
     alarmeGSM = 1;
     tmp = myFile.read();myFile.read();
     if(tmp == '0')
       alarmeGSM = 0; 
     //Serial.print("alarmeGSM=");Serial.print(tmp);Serial.print("\n");

     for(k=0; k<=9;k++)
       tmp2[k] = myFile.read();
     tmp2[k-1] = 0;
     strcpy(alarmeGSMNum,tmp2);  
     //Serial.print("alarmeGSMNum=");Serial.print(alarmeGSMNum);Serial.print("\n");
   }
   myFile.close();
  }
}

/**
 Funcao que grava as configuracoes da aplicacao no SD
*/
void saveConfig()
{
  if (SD.exists(fileName)) 
  {
    SD.remove(fileName);
  }

  myFile = SD.open(fileName, FILE_WRITE);
  if(myFile)
  {
    myFile.print(resfLigaDes);myFile.print("\n");
    myFile.print(resfTemMax);myFile.print("\n");
    myFile.print(resfAtMan);myFile.print("\n");
    myFile.print(aqueLigaDes);myFile.print("\n");
    myFile.print(aqueTemMin);myFile.print("\n");
    myFile.print(aqueAtMan);myFile.print("\n");
    myFile.print(nivelAutoManual);myFile.print("\n");
    myFile.print(confiNivelSump);myFile.print("\n");
    myFile.print(confiNivelRepo);myFile.print("\n");
    myFile.print(confNivelAutoManual);myFile.print("\n");
    myFile.print(vazAutoManualSump);myFile.print("\n");
    myFile.print(vazAutoManualRepo);myFile.print("\n");
    myFile.print(alarmeGSM);myFile.print("\n");
    myFile.print(alarmeGSMNum);myFile.print("\n");
    myFile.close();
  }
}
