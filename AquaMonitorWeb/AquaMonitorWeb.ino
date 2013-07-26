#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//char server[] = "192.168.0.102";
char server[] = "fargwebservices-2.fargweb.cloudbees.net";
IPAddress ip(192,168,0,8);

EthernetClient client;

/**
 Funcao que controla a chamada aos sensores
*/
char s;
byte state = 0;
char comando;
char valorComando[11];
byte posicaoLeitura;
byte tamanhoComando;

#define CMD_INICIO 0
#define CMD_LENDO 1
#define CMD_FINALIZADO 2
#define CMD_MAX_TAM 11

#define CMD_L_TEMP_AGUA 'T' 
#define CMD_L_TEMP_TAMPA 'E' 
#define CMD_L_TEMP_AMB 'A' 
#define CMD_L_NIVEL_SUMP 'S' 
#define CMD_L_NIVEL_REPO 'R' 
#define CMD_L_NIVEL_MAXI 'N' 
#define CMD_L_LUZ_LIGADA 'L'
#define CMD_L_ENVIA_WEB 'W'
#define CMD_L_DATA      'D'
#define CMD_L_HORA      'H'

char temperatura[6];
char temperaturaTampa[6];
char temperaturaAmb[6];
char distancia[3];
char distanciaRep[3];
char nivelMax[2];
char luzLigada[2];
char data[11];
char hora[6];

void monitor()
{
	while(Serial.available())
	{
		s = Serial.read();
		if(state == CMD_INICIO)
		{
			comando = s;
                        //Serial.print("caracter:");Serial.println(s);
			state = CMD_LENDO;
			posicaoLeitura = 1;
			if(s == CMD_L_TEMP_AGUA || s == CMD_L_TEMP_TAMPA || s == CMD_L_TEMP_AMB || s == CMD_L_NIVEL_SUMP || s == CMD_L_NIVEL_REPO || s == CMD_L_HORA)
			{
				tamanhoComando = 5;
				valorComando[5]='\0';
			}
			if(s == CMD_L_NIVEL_MAXI || s == CMD_L_LUZ_LIGADA)
			{
				tamanhoComando = 1;
				valorComando[1]='\0';
			}
                        if(s == CMD_L_ENVIA_WEB)
			{
				//tamanhoComando = 1;
				//valorComando[2]='\0';
                                state = CMD_INICIO;
                                if (!client.connected()) {
                                    //Serial.println("disconnecting....");
                                    client.stop();
                                    //Serial.println("conectando....");
                                    client.connect(server, 80);
                                    enviaDados();
                                 }
                                 else{
                                   enviaDados();
                                 }
                                   /*Serial.println(temperatura);
                                   Serial.println(temperaturaTampa);
                                   Serial.println(temperaturaAmb);
                                   Serial.println(distancia);
                                   Serial.println(distanciaRep);
                                   Serial.println(nivelMax);
                                   Serial.println(luzLigada);
                                   Serial.println(data);
                                   Serial.println(hora);*/
                                   
                                
			}
                        if(s == CMD_L_DATA)
			{
				tamanhoComando = 10;
				valorComando[10]='\0';
                                //enviaDados();
			}

		}
		else if(state == CMD_LENDO && posicaoLeitura <= tamanhoComando)
		{
			valorComando[posicaoLeitura-1] = s;
                        //Serial.print("caracter:");Serial.println(s);
			posicaoLeitura++;
                        if(posicaoLeitura>tamanhoComando){
                          //Serial.println("CMD_FINALIZADO:");
			  posicaoLeitura = 0;
			  state = CMD_FINALIZADO;
                        }
		}
		/*else
		{
                        Serial.println("CMD_FINALIZADO:");
			posicaoLeitura = 0;
			state = CMD_FINALIZADO;
		}*/
		
	}
	
	if(state == CMD_FINALIZADO)
	{
		state = CMD_INICIO;
	        switch(comando)
		{
			case CMD_L_TEMP_AGUA: 
                                temperatura[0] = valorComando[0];
                                temperatura[1] = valorComando[1];
                                temperatura[2] = valorComando[2];
                                temperatura[3] = valorComando[3];
                                temperatura[4] = valorComando[4];                                
			break;
			case CMD_L_TEMP_TAMPA: 
                                temperaturaTampa[0] = valorComando[0];
                                temperaturaTampa[1] = valorComando[1];
                                temperaturaTampa[2] = valorComando[2];
                                temperaturaTampa[3] = valorComando[3];
                                temperaturaTampa[4] = valorComando[4];                                
			break;
			case CMD_L_TEMP_AMB: 
                                temperaturaAmb[0] = valorComando[0];
                                temperaturaAmb[1] = valorComando[1];
                                temperaturaAmb[2] = valorComando[2];
                                temperaturaAmb[3] = valorComando[3];
                                temperaturaAmb[4] = valorComando[4];                                
			break;
			case CMD_L_NIVEL_SUMP: 
				distancia[0]=valorComando[0];
                                distancia[1]=valorComando[1];
			break;
			case CMD_L_NIVEL_REPO: 
				distanciaRep[0]=valorComando[0];
                                distanciaRep[1]=valorComando[1];
			break;
			case CMD_L_NIVEL_MAXI: 
				nivelMax[0] = valorComando[0];
			break;
			case CMD_L_LUZ_LIGADA: 
				luzLigada[0] = valorComando[0];
			break;
                        case CMD_L_DATA: 
                                data[0] = valorComando[0];
                                data[1] = valorComando[1];
                                data[2] = valorComando[2];
                                data[3] = valorComando[3];
                                data[4] = valorComando[4];                                
                                data[5] = valorComando[5];
                                data[6] = valorComando[6];
                                data[7] = valorComando[7];
                                data[8] = valorComando[8];
                                data[9] = valorComando[9];                                

			break;
                        case CMD_L_HORA: 
                                hora[0] = valorComando[0];
                                hora[1] = valorComando[1];
                                hora[2] = valorComando[2];
                                hora[3] = valorComando[3];
                                hora[4] = valorComando[4];  
                         break;       
		}
	}
}

void setup() {

  Serial.begin(9600);
  
  //Serial.println("Inicializando Ethernet...");
  if (Ethernet.begin(mac) == 0) {
    //Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }

  delay(2000);
  //Serial.println("connecting...");

  //if (client.connect(server, 80)) {
  //if (client.connect(server, 8080)) {
  //  Serial.println("connected");
    //mandaDados();

  //} 
  //else {
    // kf you didn't get a connection to the server:
  //  Serial.println("connection failed");
  //}
  
  temperatura[0] = '2';
  temperatura[1] = '7';
  temperatura[2] = '.';
  temperatura[3] = '0';
  temperatura[4] = '3';                                
  temperaturaTampa[0] = '3';
  temperaturaTampa[1] = '0';
  temperaturaTampa[2] = '.';
  temperaturaTampa[3] = '2';
  temperaturaTampa[4] = '3';                                
  temperaturaAmb[0] = '1';
  temperaturaAmb[1] = '7';
  temperaturaAmb[2] = '.';
  temperaturaAmb[3] = '0';
  temperaturaAmb[4] = '2';
  distancia[0] = '0';
  distancia[1] = '6';
  distanciaRep[0] = '1';
  distanciaRep[1] = '5';
  nivelMax[0] = '1';
  luzLigada[0] = '1';
  data[0] = '2';
  data[1] = '4';
  data[2] = '/';
  data[3] = '0';
  data[4] = '7';
  data[5] = '/';
  data[6] = '2';
  data[7] = '0';
  data[8] = '1';
  data[9] = '3';
  hora[0] = '2';
  hora[1] = '2';
  hora[2] = ':';
  hora[3] = '0';
  hora[4] = '7';
  
  temperatura[5] = '\0';
  temperaturaTampa[5] = '\0';
  temperaturaAmb[5] = '\0';
  distancia[2] = '\0';
  distanciaRep[2] = '\0';
  nivelMax[1] = '\0';
  luzLigada[1] = '\0';
  data[10] = '\0';
  hora[5] = '\0';
}

/*void mandaDados()
{
  client.print( "POST /services/ManutencaoAquario.ManutencaoAquarioHttpSoap12Endpoint/ HTTP/1.1");client.print("\r\n");
  client.print( "Accept-Encoding: gzip,deflate");client.print("\r\n");
  client.print( "Content-Type: application/soap+xml;charset=UTF-8;action=\"urn:AtualizaAquario\"");client.print("\r\n");
  client.print( "Content-Length: 792");client.print("\r\n");
  client.print( "Host: fargwebserver-2.fargweb.cloudbees.net");client.print("\r\n");
  client.print( "Connection: Keep-Alive");client.print("\r\n");
  client.print( "User-Agent: Apache-HttpClient/4.1.1 (java 1.5)");client.print("\r\n");
  client.print( "\r\n");
  client.print( "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:web=\"http://webservices\">[\n]");
  client.print( "   <soap:Header/>[\n]");
  client.print( "   <soap:Body>[\n]");
  client.print( "      <web:AtualizaAquario>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:dataMedicao>01/07/2013</web:dataMedicao>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:horaMedicao>21:36</web:horaMedicao>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:luzLigada>1</web:luzLigada>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:nivelRepo>06</web:nivelRepo>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:nivelSump>15</web:nivelSump>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:tempAgua>27.5</web:tempAgua>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:tempAmb>22.0</web:tempAmb>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:tempTampa>30.0</web:tempTampa>[\n]");
  client.print( "      </web:AtualizaAquario>[\n]");
  client.print( "   </soap:Body>[\n]");
  client.print( "</soap:Envelope>");
  client.print( "\r\n");
}*/

void enviaDados()
{
  
  client.print( "POST /services/ManutencaoAquario.ManutencaoAquarioHttpSoap12Endpoint/ HTTP/1.1");client.print("\r\n");
  client.print( "Accept-Encoding: gzip,deflate");client.print("\r\n");
  client.print( "Content-Type: application/soap+xml;charset=UTF-8;action=\"urn:AtualizaAquario\"");client.print("\r\n");
  client.print( "Content-Length: 792");client.print("\r\n");
  client.print( "Host: fargwebserver-2.fargweb.cloudbees.net");client.print("\r\n");
  client.print( "Connection: Keep-Alive");client.print("\r\n");
  client.print( "User-Agent: Apache-HttpClient/4.1.1 (java 1.5)");client.print("\r\n");
  client.print( "\r\n");
  client.print( "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:web=\"http://webservices\">[\n]");
  client.print( "   <soap:Header/>[\n]");
  client.print( "   <soap:Body>[\n]");
  client.print( "      <web:AtualizaAquario>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:dataMedicao>");client.print(data);client.print( "</web:dataMedicao>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:horaMedicao>");client.print(hora);client.print( "</web:horaMedicao>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:luzLigada>");client.print(luzLigada);client.print( "</web:luzLigada>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:nivelRepo>");client.print(distanciaRep);client.print( "</web:nivelRepo>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:nivelSump>");client.print(distancia);client.print( "</web:nivelSump>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:tempAgua>");client.print(temperatura);client.print( "</web:tempAgua>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:tempAmb>");client.print(temperaturaAmb);client.print( "</web:tempAmb>[\n]");
  client.print( "         <!--Optional:-->[\n]");
  client.print( "         <web:tempTampa>");client.print(temperaturaTampa);client.print( "</web:tempTampa>[\n]");
  client.print( "      </web:AtualizaAquario>[\n]");
  client.print( "   </soap:Body>[\n]");
  client.print( "</soap:Envelope>");
  client.print( "\r\n");
}

void loop()
{
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    //Serial.print(c);
    
  }
  
  monitor();

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    //Serial.println();
    //Serial.println("disconnecting.");
    client.stop();
    // do nothing forevermore:
    //while(true);
  }
  //delay(1000);
}
