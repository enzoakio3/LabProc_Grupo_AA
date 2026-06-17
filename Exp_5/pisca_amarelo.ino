#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h> // Biblioteca necessária para o LED RGB embutido da ESP32-C3


// Configurações do Ponto de Acesso Wi-Fi da ESP32-C3
const char* ssid = "Monitor_LDR_ESP32C3_Grupo_AA";
const char* password = "12345678";


WebServer server(80);


// ==========================================
// CONFIGURAÇÕES DE HARDWARE DA ESP32-C3
// ==========================================
const int LDR_PIN = 4;        // GPIO 4 para leitura do ADC1_CH4
const int PIN_LED_RGB = 8;    // GPIO 8 é o padrão do LED built-in NeoPixel na ESP32-C3
const int NUM_PIXELS = 1;     // Há apenas 1 LED integrado na placa


// Inicializa o objeto do LED RGB embutido
Adafruit_NeoPixel ledBuiltIn(NUM_PIXELS, PIN_LED_RGB, NEO_GRB + NEO_KHZ800);


// Variáveis globais de controle e armazenamento
int valorLDR = 0;
unsigned long ultimoTempoLeitura = 0;
const unsigned long intervaloLeitura = 1000; // 1000 ms = 1 segundo (Frequência de 1Hz)


// Controle do pisca-pisca do LED em baixa luminosidade (Não-bloqueante)
unsigned long ultimoTempoPisca = 0;
const unsigned long intervaloPisca = 2000;   // 2000 ms = 2 segundos
bool estadoLedAmarelo = false;


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
        setInterval(requisitarDadosLDR, 1000);


        function requisitarDadosLDR() {
            fetch('/getldr')
                .then(response => response.text())
                .then(valor => {
                    document.getElementById('ldrValor').innerText = valor;
                    let porcentagem = (parseInt(valor) / 4095) * 100;
                    document.getElementById('ldrBarra').style.width = porcentagem + "%";
                   
                    if (parseInt(valor) >= 2000) {
                        document.getElementById('statusTexto').innerHTML = "<span style='color:orange;'><b>Baixa Luminosidade!</b> LED Amarelo Ativo.</span>";
                    } else {
                        document.getElementById('statusTexto').innerHTML = "<span style='color:green;'>Luminosidade Normal.</span>";
                    }
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


void handleGetLDR() {
  server.send(200, "text/plain", String(valorLDR));
}


// ==========================================
// CONFIGURAÇÃO DO HARDWARE (SETUP)
// ==========================================


void setup() {
  Serial.begin(115200);


  // Inicializa o LED embutido NeoPixel
  ledBuiltIn.begin();
  ledBuiltIn.setBrightness(50); // Define o brilho em aproximadamente 20% para não ofuscar os olhos
  ledBuiltIn.clear();
  ledBuiltIn.show();


  // Configuração do pino analógico do LDR
  pinMode(LDR_PIN, INPUT);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);


  // Inicialização do Access Point Wi-Fi
  WiFi.softAP(ssid, password);
  Serial.println("\n=== Servidor LDR com LED Alerta Ativo (ESP32-C3) ===");
  Serial.print("Acesse o painel no navegador: http://");
  Serial.println(WiFi.softAPIP());


  server.on("/", handleHome);
  server.on("/getldr", handleGetLDR);
  server.begin();
}


// ==========================================
// LOOP PRINCIPAL DE EXECUÇÃO
// ==========================================


void loop() {
  server.handleClient();


  unsigned long tempoAtual = millis();


  // 1. Polling de leitura do LDR (A cada 1 segundo)
  if (tempoAtual - ultimoTempoLeitura >= intervaloLeitura) {
    ultimoTempoLeitura = tempoAtual;
    valorLDR = analogRead(LDR_PIN);
    Serial.printf("[Telemetria] LDR: %d\n", valorLDR);
  }


  // 2. Controle do LED Built-In baseado no limiar de Baixa Luminosidade (>= 2000)
  if (valorLDR >= 1800) {
    // Se está escuro, verifica o temporizador de 2 segundos para piscar
    if (tempoAtual - ultimoTempoPisca >= intervaloPisca) {
      ultimoTempoPisca = tempoAtual;
      estadoLedAmarelo = !estadoLedAmarelo; // Inverte o estado (liga / desliga)


      if (estadoLedAmarelo) {
        // COR AMARELA: Mistura de Vermelho máximo (255) e Verde máximo (255), com Azul zerado (0)
        ledBuiltIn.setPixelColor(0, ledBuiltIn.Color(255, 255, 0));
      } else {
        ledBuiltIn.clear(); // Apaga o LED
      }
      ledBuiltIn.show(); // Atualiza fisicamente o LED na placa
    }
  }
  else {
    // Se a luminosidade voltar ao normal (< 2000), desliga o LED imediatamente
    if (estadoLedAmarelo || ledBuiltIn.getPixelColor(0) > 0) {
      ledBuiltIn.clear();
      ledBuiltIn.show();
      estadoLedAmarelo = false;
    }
  }
}
