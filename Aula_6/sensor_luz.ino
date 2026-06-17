#include <WiFi.h>
#include <WebServer.h>


// Configurações do Ponto de Acesso Wi-Fi da ESP32-C3
const char* ssid = "Monitor_LDR_ESP32C3_Grupo_AA";
const char* password = "12345678";


WebServer server(80);


// Mapeamento de Pinos para a ESP32-C3
// ALTERADO: Agora utilizando a GPIO 4 (que suporta nativamente o canal ADC1_CH4)
const int LDR_PIN = 4;


// Variáveis globais de controle e armazenamento
int valorLDR = 0;
unsigned long ultimoTempoLeitura = 0;
const unsigned long intervaloLeitura = 1000; // 1000 ms = 1 segundo (Frequência de 1Hz)


// ==================================================
// INTERFACE HTML COM ATUALIZAÇÃO ASSÍNCRONA VIA AJAX
// ==================================================
const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-C3 LDR Monitoring</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background-color: #f4f7f6; padding: 20px; color: #333; }
        .container { max-width: 450px; margin: 30px auto; background: white; padding: 30px; border-radius: 14px; box-shadow: 0px 5px 20px rgba(0,0,0,0.08); }
        h2 { color: #0056b3; margin-bottom: 25px; }
        .leitura-box { font-size: 48px; font-weight: bold; font-family: monospace; color: #28a745; margin: 20px 0; padding: 15px; background-color: #eef9f0; border-radius: 8px; }
        .barra-container { background-color: #ddd; border-radius: 10px; height: 20px; width: 100%; overflow: hidden; margin-top: 10px; }
        .barra-progresso { background-color: #28a745; height: 100%; width: 0%; transition: width 0.4s ease; }
        .panel-status { margin-top: 25px; padding: 12px; border-radius: 8px; background-color: #f8f9fa; border-left: 5px solid #007bff; text-align: left; font-size: 14px; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Monitoramento de Luminosidade</h2>
       
        <p>Valor bruto lido pelo ADC (0 - 4095):</p>
        <div class="leitura-box" id="ldrValor">0</div>
       
        <div class="barra-container">
            <div class="barra-progresso" id="ldrBarra"></div>
        </div>
       
        <div class="panel-status">
            <b>Status da Conexão:</b> <span id="statusTexto">Atualizando dados...</span>
        </div>
    </div>


    <script>
        // Faz requisições ao endpoint /getldr a cada 1000ms (Frequência de 1Hz)
        setInterval(requisitarDadosLDR, 1000);


        function requisitarDadosLDR() {
            fetch('/getldr')
                .then(response => response.text())
                .then(valor => {
                    document.getElementById('ldrValor').innerText = valor;
                   
                    // Calcula a porcentagem com base nos 12 bits de resolução (4095)
                    let porcentagem = (parseInt(valor) / 4095) * 100;
                    document.getElementById('ldrBarra').style.width = porcentagem + "%";
                    document.getElementById('statusTexto').innerHTML = "<span style='color:green;'>Dados sincronizados via Wi-Fi.</span>";
                })
                .catch(err => {
                    document.getElementById('statusTexto').innerHTML = "<span style='color:red;'>Erro ao comunicar com a ESP32-C3.</span>";
                });
        }
    </script>
</body>
</html>
)rawliteral";


// ==========================================
// CONTROLADORES DAS REQUISIÇÕES (ENDPOINTS)
// ==========================================


void handleHome() {
  server.send(200, "text/html", PAGINA_HTML);
}


// Endpoint que retorna o valor de telemetria cru (texto puro) capturado pelo ADC
void handleGetLDR() {
  server.send(200, "text/plain", String(valorLDR));
}


// ==========================================
// CONFIGURAÇÃO DO HARDWARE (SETUP)
// ==========================================


void setup() {
  Serial.begin(115200);


  // Configuração do pino analógico (GPIO 4)
  pinMode(LDR_PIN, INPUT);
 
  // Define a atenuação correta para ler a faixa total de tensão (0V a 3.3V) em 12 bits
  analogSetPinAttenuation(LDR_PIN, ADC_11db);


  // Inicialização do Access Point
  WiFi.softAP(ssid, password);
  Serial.println("\n=== Servidor ADC Ativo (ESP32-C3 - Pino 4) ===");
  Serial.print("Conecte-se à rede Wi-Fi: ");
  Serial.println(ssid);
  Serial.print("Acesse o painel no navegador: http://");
  Serial.println(WiFi.softAPIP());


  // Configuração das rotas HTTP do Webserver
  server.on("/", handleHome);
  server.on("/getldr", handleGetLDR);


  server.begin();
}


// ==========================================
// LOOP PRINCIPAL DE EXECUÇÃO
// ==========================================


void loop() {
  // Atende as requisições HTTP vindas dos clientes conectados à interface web
  server.handleClient();


  // Estrutura de tempo não-bloqueante (Polling de baixa prioridade)
  unsigned long tempoAtual = millis();
  if (tempoAtual - ultimoTempoLeitura >= intervaloLeitura) {
    ultimoTempoLeitura = tempoAtual;
   
    // Efetua a amostragem analógica via Conversor Analógico-Digital no Pino 4
    valorLDR = analogRead(LDR_PIN);
   
    // Log de telemetria impresso no monitor serial para validação
    Serial.printf("[Telemetria] Valor bruto do ADC (Pino 4): %d\n", valorLDR);
  }
}
