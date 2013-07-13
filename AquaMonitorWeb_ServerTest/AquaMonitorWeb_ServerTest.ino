#include <SPI.h>
#include <Ethernet.h>
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//Informe um mac-address
IPAddress ip(192,168,0, 5);//Informe um IP 

EthernetServer server(80);//Informando a porta para receber requisicoes

void setup() 
{
  // Inicializando modulo com mac e ip
  Ethernet.begin(mac, ip);
  server.begin();//Inicializando o servidor
}

void loop() {
  // Aguardando conexoes
  EthernetClient client = server.available();
  if (client) 
  {
    boolean currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();//Lendo carcteres recebidos
        Serial.write(c);
        //Se receber \n e a linha for em branco (dois \n) - fim http request
        if (c == '\n' && currentLineIsBlank) 
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Ola</title>");
          client.println("<html>");
          client.println("</head>");
          client.println("<center>");
          client.println("<h1>");
          client.println("<p>ArduinoCC</p>");
          client.println("</h1>");
          client.println("<h1>");
          client.println("<p>Arduino Ethernet</p>");
          client.println("</h1>");
          client.println("TESTE HTTP");
          client.println("</center>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // Nova linha
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // recebendo caractere apos nova linha
          currentLineIsBlank = false;
        }
      }
    }
    //Aguardando o navegador receber a pagina
    delay(1);

    client.stop();//Fechando conexao
  }
}
