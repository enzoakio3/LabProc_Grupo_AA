#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>


// Configurações do Ponto de Acesso Wi-Fi da ESP32-C3
const char* ssid = "Monitor_SOS_ESP32C3_Grupo_AA";
const char* password = "12345678";


WebServer server(80);


// ==========================================
// CONFIGURAÇÕES DE HARDWARE DA ESP32-C3
// ==========================================
const int LDR_PIN = 4;        // GPIO 4 para leitura do ADC
const int BOTAO_PIN = 5;      // GPIO 5 para o Botão SOS (Interrupção externa)
const int PIN_LED_RGB = 8;    // GPIO 8 padrão do LED built-in NeoPixel na ESP32-C3
const int NUM_PIXELS = 1;


Adafruit_NeoPixel ledBuiltIn(NUM_PIXELS, PIN_LED_RGB, NEO_GRB + NEO_KHZ800);


// Variáveis de Telemetria e Tempo (LDR)
int valorLDR = 0;
unsigned long ultimoTempoLeitura = 0;
const unsigned long intervaloLeitura = 1000; // 1 segundo (Frequência = 1Hz)


// Variáveis do Pisca Amarelo (Baixa Luminosidade)
unsigned long ultimoTempoPisca = 0;
const unsigned long intervaloPisca = 2000;   // 2 segundos
bool estadoLedAmarelo = false;


// ==========================================
// VARIÁVEIS DE INTERRUPÇÃO E DEBOUNCE (SOS)
// ==========================================
volatile bool sinalBotaoInterrupcao = false;  // Sinaliza que houve transição no pino
unsigned long tempoUltimoDebounce = 0;
const unsigned long tempoDebounceDelay = 50; // 50ms de tempo de estabilização


// Controle do Alerta Vermelho de Alta Prioridade
bool alertaVermelhoAtivo = false;
unsigned long tempoInicioAlertaVermelho = 0;
const unsigned long duracaoAlertaVermelho = 3000; // 3 segundos (3000 ms)


// ==================================================
// ROTINA DE SERVIÇO DE INTERRUPÇÃO (ISR)
// ==================================================
// Executada instantaneamente na RAM para máxima performance.
// Apenas sinaliza o evento de forma assíncrona.
void IRAM_ATTR tratarBotaoSOS() {
  sinalBotaoInterrupcao = true;
}


// ==================================================
// INTERFACE HTML COM ATUALIZAÇÃO VIA JSON
// ==================================================
const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-C3 Smart Monitoring</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background-color: #f4f7f6; padding: 20px; color: #333; }
        .container { max-width: 450px; margin: 30px auto; background: white; padding: 30px; border-radius: 14px; box-shadow: 0px 5px 20px rgba(0,0,0,0.08); }
        h2 { color: #0056b3; margin-bottom: 25px; }
        .leitura-box { font-size: 48px; font-weight: bold; font-family: monospace; color: #28a745; margin: 20px 0; padding: 15px; background-color: #eef9f0; border-radius: 8px; }
        .barra-container { background-color: #ddd; border-radius: 10px; height: 20px; width: 100%; overflow: hidden; margin-top: 10px; }
        .barra-progresso { background-color: #28a745; height: 100%; width: 0%; transition: width 0.4s ease; }
        .panel-status { margin-top: 25px; padding: 12px; border-radius: 8px; background-color: #f8f9fa; border-left: 5px solid #007bff; text-align: left; font-size: 14px; }
        .alerta-sos { background-color: #f8d7da !important; color: #721c24 !important; border-left-color: #dc3545 !important; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Monitoramento Inteligente SOS</h2>
        <p>Luminosidade LDR (0 - 4095):</p>
        <div class="leitura-box" id="ldrValor">0</div>
        <div class="barra-container"><div class="barra-progresso" id="ldrBarra"></div></div>
        <div class="panel-status" id="painelStatus">
            <b>Status:</b> <span id="statusTexto">Iniciando sistema...</span>
        </div>
    </div>
    <script>
        setInterval(requisitarDados, 1000);
        function requisitarDados() {
            fetch('/getstatus')
                .then(response => response.json())
                .then(dados => {
                    document.getElementById('ldrValor').innerText = dados.ldr;
                    document.getElementById('ldrBarra').style.width = ((dados.ldr / 4095) * 100) + "%";
                    let painel = document.getElementById('painelStatus');
                    let texto = document.getElementById('statusTexto');
                   
                    if (dados.sos === 1) {
                        painel.classList.add('alerta-sos');
                        texto.innerHTML = "<b>[EMERGÊNCIA] BOTÃO SOS PRESSIONADO!</b>";
                    } else if (dados.ldr >= 2000) {
                        painel.classList.remove('alerta-sos');
                        texto.innerHTML = "<span style='color:orange;'>Modo Noturno (Baixa Luminosidade)</span>";
                    } else {
                        painel.classList.remove('alerta-sos');
                        texto.innerHTML = "<span style='color:green;'>Operação Normal de Fundo</span>";
                    }
                });
        }
    </script>
</body>
</html>
)rawliteral";


void handleHome() { server.send(200, "text/html", PAGINA_HTML); }


void handleGetStatus() {
  String json = "{\"ldr\":" + String(valorLDR) + ",\"sos\":" + String(alertaVermelhoAtivo ? 1 : 0) + "}";
  server.send(200, "application/json", json);
}


// ==========================================
// SETUP DO SISTEMA
// ==========================================
void setup() {
  Serial.begin(115200);


  ledBuiltIn.begin();
  ledBuiltIn.setBrightness(40);
  ledBuiltIn.clear();
  ledBuiltIn.show();


  pinMode(LDR_PIN, INPUT);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);


  // Configura o botão físico com Pull-Up Interno (Em repouso fica em HIGH, pressionado vai para LOW)
  pinMode(BOTAO_PIN, INPUT_PULLUP);
  // Interrupção dispara na transição de descida (FALLING), quando o botão conecta ao GND
  attachInterrupt(digitalPinToInterrupt(BOTAO_PIN), tratarBotaoSOS, FALLING);


  WiFi.softAP(ssid, password);
  server.on("/", handleHome);
  server.on("/getstatus", handleGetStatus);
  server.begin();
  Serial.println("\n=== Sistema SOS Pronto com Prioridade Concluída ===");
}


// ==========================================
// LOOP PRINCIPAL (GERENCIAMENTO DE PRIORIDADES)
// ==========================================
void loop() {
  server.handleClient();
  unsigned long tempoAtual = millis();


  // 1. Polling de Leitura Analógica do LDR (A cada 1 segundo)
  if (tempoAtual - ultimoTempoLeitura >= intervaloLeitura) {
    ultimoTempoLeitura = tempoAtual;
    valorLDR = analogRead(LDR_PIN);
  }


  // 2. Filtro de Debounce por Software (Tratamento do sinal da Interrupção)
  if (sinalBotaoInterrupcao) {
    sinalBotaoInterrupcao = false; // Consome o sinal da ISR
   
    // Se o tempo decorrido desde o último ruído for maior que o delay do debounce
    if ((tempoAtual - tempoUltimoDebounce) > tempoDebounceDelay) {
      // Confirmação física: verifica se o botão ainda está pressionado (LOW)
      if (digitalRead(BOTAO_PIN) == LOW) {
        tempoUltimoDebounce = tempoAtual; // Atualiza a marca de tempo estável
       
        // Ativa o alerta de Alta Prioridade
        alertaVermelhoAtivo = true;
        tempoInicioAlertaVermelho = tempoAtual;
       
        // Força o LED integrado a ficar Vermelho imediatamente
        ledBuiltIn.setPixelColor(0, ledBuiltIn.Color(255, 0, 0)); // R=255, G=0, B=0
        ledBuiltIn.show();
        Serial.println("[ALERTA CRÍTICO] SOS Pressionado! Interrupção tratada com Debounce.");
      }
    }
  }


  // 3. Máquina de Estados e Gerenciamento de Prioridades do LED Built-in
  if (alertaVermelhoAtivo) {
    // ESTADO 1 (MÁXIMA PRIORIDADE): Verifica se já se passaram os 3 segundos do SOS
    if (tempoAtual - tempoInicioAlertaVermelho >= duracaoAlertaVermelho) {
      alertaVermelhoAtivo = false; // Finaliza o tempo do alerta vermelho
      ledBuiltIn.clear();
      ledBuiltIn.show();
      Serial.println("[INFO] Alerta SOS encerrado. Devolvendo controle para telemetria de fundo.");
    }
    // Enquanto estiver nos 3 segundos, ignora completamente qualquer bloco abaixo (LDR)
  }
  else if (valorLDR >= 2000) {
    // ESTADO 2 (MÉDIA PRIORIDADE): Baixa Luminosidade - Pisca Amarelo a cada 2s
    if (tempoAtual - ultimoTempoPisca >= intervaloPisca) {
      ultimoTempoPisca = tempoAtual;
      estadoLedAmarelo = !estadoLedAmarelo;


      if (estadoLedAmarelo) {
        ledBuiltIn.setPixelColor(0, ledBuiltIn.Color(255, 180, 0)); // Amarelo equilibrado
      } else {
        ledBuiltIn.clear();
      }
      ledBuiltIn.show();
    }
  }
  else {
    // ESTADO 3 (BAIXA PRIORIDADE): Condições Normais - Apaga o LED
    if (estadoLedAmarelo || ledBuiltIn.getPixelColor(0) > 0) {
      ledBuiltIn.clear();
      ledBuiltIn.show();
      estadoLedAmarelo = false;
    }
  }
}
