#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//char server[] = "192.168.0.102";
char server[] = "fargwebservices.fargweb.cloudbees.net";
IPAddress ip(192,168,0,9);

EthernetClient client;

void setup() {

  Serial.begin(9600);

  Serial.println("Inicilizando Ethernet...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }

  delay(2000);
  Serial.println("connecting...");

  if (client.connect(server, 80)) {
  //if (client.connect(server, 8080)) {
    Serial.println("connected");
    mandaDados();

  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

/*void mandaTeste()
{
      // Make a HTTP request:
    //client.println("GET / HTTP/1.1");
    //client.println("Host: fargwebservices.fargweb.cloudbees.net");
    //client.println("Connection: close");
    //client.println();
    
    client.print("POST /services/TesteWebService.TesteWebServiceHttpSoap12Endpoint/ HTTP/1.1");client.print("\r\n");
    //client.print("POST /FARG_WEBSERVER/services/TesteWebService.TesteWebServiceHttpSoap11Endpoint/ HTTP/1.1");client.print("\r\n");
    client.print("Accept-Encoding: gzip,deflate");client.print("\r\n");
    client.print("Content-Type: application/soap+xml;charset=UTF-8;action=\"urn:versao\"");client.print("\r\n");
    client.print("SOAPAction: \"urn:versao\"");client.print("\r\n");
    client.print("Content-Length: 185");client.print("\r\n");
    client.print("Host: fargwebservices.fargweb.cloudbees.net");client.print("\r\n");
    //client.print("Host: 192.168.0.102:8080");client.print("\r\n");
    client.print("Connection: Keep-Alive");client.print("\r\n");
    client.print("User-Agent: Apache-HttpClient/4.1.1 (java 1.5)");client.print("\r\n");
    client.print("\r\n");
    client.print("<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:web=\"http://webservices\">");client.print("\n");
    client.print("<soap:Header/>");client.print("\n");
      client.print("<soap:Body>");client.print("\n");
        client.print("<web:versao/>");client.print("\n");
      client.print("</soap:Body>");client.print("\n");
    client.print("</soap:Envelope>");//client.print("\n");
    //client.print("\r\n");
    //client.print("\r\n");
    //client.print("\r\n");

}*/

void mandaDados()
{
  client.print( "POST /services/ManutencaoAquario.ManutencaoAquarioHttpSoap12Endpoint/ HTTP/1.1");client.print("\r\n");
  client.print( "Accept-Encoding: gzip,deflate");client.print("\r\n");
  client.print( "Content-Type: application/soap+xml;charset=UTF-8;action=\"urn:AtualizaAquario\"");client.print("\r\n");
  client.print( "Content-Length: 792");client.print("\r\n");
  client.print( "Host: fargwebserver.fargweb.cloudbees.net");client.print("\r\n");
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
}

void loop()
{
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    while(true);
  }
}
