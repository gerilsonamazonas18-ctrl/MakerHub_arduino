#include <SPI.h>       // Comunicação SPI (Ethernet e SD)
#include <Ethernet.h>  // Servidor Web
#include <SD.h>        // Cartão SD

// ==================================================
// CONFIGURAÇÃO DE HARDWARE
// ==================================================

// Pinos Chip Select do Shield W5100
const int SD_CS = 4;
const int ETH_CS = 10;

// Endereço MAC do servidor
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Servidor HTTP na porta 80
EthernetServer server(80);

// ==================================================
// ROTA: /
// Envia a página principal (index.htm)
// ==================================================
void enviarIndex(EthernetClient &client) {
    File pagina = SD.open("index.htm");

    if (!pagina) {

        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<h1>Erro</h1>");
        client.println("<p>index.htm nao encontrado</p>");

        Serial.println("Erro ao abrir index.htm");
        return;
  }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

  // Envia o arquivo diretamente do SD para o navegador
    while (pagina.available()) {
        client.write(pagina.read());
    }

    pagina.close();
    Serial.println("Pagina enviada");
}

// ==================================================
// ROTA: /status
// Retorna informações do servidor em JSON
// ==================================================
void enviarStatus(EthernetClient &client) {

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  client.print("{");
  client.print("\"server\":\"MakerHub\",");
  client.print("\"status\":\"online\",");
  client.print("\"ip\":\"");
  client.print(Ethernet.localIP());
  client.print("\",");
  client.print("\"gateway\":\"");
  client.print(Ethernet.gatewayIP());
  client.print("\"");
  client.print("}");

  Serial.println("Status enviado");
}

// ==================================================
// ROTA: /inventario
// Lê invent.csv do cartão SD
// ==================================================
void enviarInventory(EthernetClient &client) {
    File inventario = SD.open("invent.csv");

    if (!inventario) {

        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<h1>invent.csv nao encontrado</h1>");

        Serial.println("Erro ao abrir invent.csv");
        return;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();

    while (inventario.available()) {
        client.write(inventario.read());
    }

    inventario.close();
    Serial.println("Inventario enviado");
}

// ==================================================
// ROTA: /device
// Lista dispositivos cadastrados
// ==================================================
void enviarDevice(EthernetClient &client) {

    File arquivo = SD.open("device.csv");

    if (!arquivo) {
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/plain");
        client.println();
        client.println("device.csv nao encontrado");
        return;
    }
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();

    while (arquivo.available()) {
        client.write(arquivo.read());
    }
    arquivo.close();
    Serial.println("Device enviado");
}

// ==================================================
// ROTA: /logs
// Exibe logs do sistema
// ==================================================
void enviarLogs(EthernetClient &client) {
    File arquivo = SD.open("logs.txt");

    if (!arquivo) {
        client.println("HTTP/1.1 404 Not Found");
        client.println();
        return;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println();

    while (arquivo.available()) {
        client.write(arquivo.read());
    }
    arquivo.close();
    Serial.println("Logs enviados");
}

// ==================================================
// ROTA: /config
// Exibe configurações do sistema
// ==================================================
void enviarConfig(EthernetClient &client) {
    File arquivo = SD.open("config.txt");

    if (!arquivo) {
        client.println("HTTP/1.1 404 Not Found");
        client.println();
        return;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println();

    while (arquivo.available()) {
        client.write(arquivo.read());
    }

    arquivo.close();
    Serial.println("Config enviada");
}

// ==================================================
// SETUP
// Executado apenas uma vez na inicialização
// ==================================================
void setup() {

    Serial.begin(9600);

    pinMode(SD_CS, OUTPUT);
    pinMode(ETH_CS, OUTPUT);

    // Desabilita Ethernet para inicializar SD
    digitalWrite(ETH_CS, HIGH);

    if (!SD.begin(SD_CS)) {
        Serial.println("\nErro ao iniciar SD");
        while (true);
    }

    Serial.println("SD OK");

    // Lista arquivos encontrados no cartão
    File root = SD.open("/");
    Serial.println("\nConteudo do SD:");

    while (true) {
        File entry = root.openNextFile();
        if (!entry)
        break;

        Serial.print(entry.name());
        Serial.print("  ");
        Serial.print(entry.size());
        Serial.println(" bytes");
        entry.close();
    }
    root.close();

    // Desabilita SD para inicializar Ethernet
    digitalWrite(SD_CS, HIGH);

    // Solicita IP ao roteador via DHCP
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Falha ao obter IP via DHCP");
        while (true);
    }

    // Inicia servidor HTTP
    server.begin();

    Serial.println("\nServidor iniciado");
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
    Serial.print("Mascara: ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(Ethernet.dnsServerIP());

    Serial.println("\nArquivos encontrados:");

    if (SD.exists("index.htm"))
        Serial.println("✓ index.htm");
    if (SD.exists("invent.csv"))
        Serial.println("✓ invent.csv");
    if (SD.exists("device.csv"))
        Serial.println("✓ device.csv");
    if (SD.exists("logs.txt"))
        Serial.println("✓ logs.txt");
    if (SD.exists("config.txt"))
        Serial.println("✓ config.txt");
}

// ==================================================
// LOOP PRINCIPAL
// Executado continuamente
// ==================================================
void loop() {
    // Verifica se algum cliente acessou o servidor
    EthernetClient client = server.available();

    if (!client)
        return;

    Serial.println("\nCliente conectado");

    // Lê a primeira linha da requisição HTTP
    String request = client.readStringUntil('\r');
    Serial.println(request);

    // Sistema de roteamento
    if (request.indexOf("GET /status") >= 0) {
        enviarStatus(client);
    } else if (request.indexOf("GET /inventario") >= 0) {
        enviarInventory(client);
    } else if (request.indexOf("GET /device") >= 0) {
        enviarDevice(client);
    } else if (request.indexOf("GET /logs") >= 0) {
        enviarLogs(client);
    } else if (request.indexOf("GET /config") >= 0) {
        enviarConfig(client);

    } else {

        // Página padrão
        enviarIndex(client);
    }

    delay(1);

    // Fecha conexão para liberar recursos do W5100
    client.stop();

    Serial.println("Cliente desconectado");
}