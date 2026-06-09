#include <WiFi.h>
#include <WebServer.h>


// Configurações do Ponto de Acesso Wi-Fi
const char* ssid = "Controle_PWM_ESP32_Grupo_AA";
const char* password = "12345678";


WebServer server(80);


// Definição do pino do LED externo (Ex: GPIO 4, ajuste conforme sua bancada)
const int LED_PIN = 4;


// Configurações do periférico LEDC (PWM de Hardware v3.x)
const int LEDC_RESOLUTION = 8;     // Resolução de 8 bits (Valores de duty cycle de 0 a 255)


// Variáveis de estado iniciais
int frequenciaAtual = 1000;        // Frequência inicial de 1 kHz (padrão)
int dutyCycleAtual = 128;          // Intensidade inicial em 50% (128/255)


// ==========================================
// INTERFACE HTML COM AJUSTE ASSÍNCRONO (PWM)
// ==========================================
const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 PWM Control Dashboard</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background-color: #f4f7f6; padding: 20px; color: #333; }
        .container { max-width: 450px; margin: 30px auto; background: white; padding: 30px; border-radius: 14px; box-shadow: 0px 5px 20px rgba(0,0,0,0.08); }
        h2 { color: #0056b3; margin-bottom: 25px; }
        label { font-weight: bold; display: block; margin-top: 20px; text-align: left; font-size: 15px; color: #555; }
        .slider-container { display: flex; align-items: center; justify-content: space-between; margin-top: 8px; }
        input[type=range] { flex: 1; margin-right: 15px; height: 8px; border-radius: 5px; background: #ddd; outline: none; }
        .valor-display { font-family: monospace; font-size: 18px; font-weight: bold; width: 60px; text-align: right; color: #007bff; }
        .btn-group { margin-top: 25px; display: flex; justify-content: space-around; }
        button { font-size: 14px; padding: 10px 18px; cursor: pointer; border: none; border-radius: 6px; font-weight: bold; background-color: #e9ecef; color: #495057; transition: 0.2s; }
        button.active { background-color: #007bff; color: white; }
        button:hover { background-color: #adb5bd; color: white; }
        .panel-status { margin-top: 25px; padding: 12px; border-radius: 8px; background-color: #f8f9fa; border-left: 5px solid #28a745; text-align: left; font-size: 14px; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Painel de Controle PWM</h2>
       
        <label>Intensidade do LED (Duty Cycle 0-100%):</label>
        <div class="slider-container">
            <input type="range" id="sliderDuty" min="0" max="100" value="50" oninput="atualizarDuty(this.value)">
            <span class="valor-display" id="displayDuty">50%</span>
        </div>
       
        <label>Frequência do PWM (Teste de Cintilação):</label>
        <div class="btn-group">
            <button id="btn10Hz" onclick="alterarFrequencia(10)">10 Hz</button>
            <button id="btn60Hz" onclick="alterarFrequencia(60)">60 Hz</button>
            <button id="btn1kHz" class="active" onclick="alterarFrequencia(1000)">1 kHz</button>
            <button id="btn5kHz" onclick="alterarFrequencia(5000)">5 kHz</button>
        </div>
       
        <div class="panel-status">
            <b>Status do Hardware:</b> <span id="statusTexto">Pronto para receber comandos.</span>
        </div>
    </div>


    <script>
        let timeoutDuty = null;


        function atualizarDuty(porcentagem) {
            document.getElementById('displayDuty').innerText = porcentagem + "%";
           
            clearTimeout(timeoutDuty);
            timeoutDuty = setTimeout(() => {
                enviarComando('/setpwm?duty=' + porcentagem);
            }, 50);
        }


        function alterarFrequencia(freq) {
            const botoes = document.querySelectorAll('.btn-group button');
            botoes.forEach(btn => btn.classList.remove('active'));
           
            if(freq === 10) document.getElementById('btn10Hz').classList.add('active');
            if(freq === 60) document.getElementById('btn60Hz').classList.add('active');
            if(freq === 1000) document.getElementById('btn1kHz').classList.add('active');
            if(freq === 5000) document.getElementById('btn5kHz').classList.add('active');
           
            enviarComando('/setpwm?freq=' + freq);
        }


        function enviarComando(url) {
            fetch(url)
                .then(response => response.text())
                .then(texto => {
                    document.getElementById('statusTexto').innerHTML = texto;
                })
                .catch(err => {
                    document.getElementById('statusTexto').innerHTML = "<span style='color:red;'>Erro ao comunicar com o ESP32.</span>";
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


void handleSetPWM() {
  bool alterouAlgo = false;


  // Processa mudança de Frequência se houver o parâmetro 'freq'
  if (server.hasArg("freq")) {
    int novaFreq = server.arg("freq").toInt();
    if (novaFreq >= 1 && novaFreq <= 40000) {
      frequenciaAtual = novaFreq;
     
      // Nova API v3.x: ledcAttach vincula diretamente o pino, frequência e resolução.
      // Canais internos são gerenciados automaticamente pelo core.
      ledcAttach(LED_PIN, frequenciaAtual, LEDC_RESOLUTION);
      ledcWrite(LED_PIN, dutyCycleAtual);
     
      alterouAlgo = true;
    }
  }


  // Processa mudança de Intensidade (Duty Cycle) se houver o parâmetro 'duty'
  if (server.hasArg("duty")) {
    int pctDuty = server.arg("duty").toInt(); // Vem de 0 a 100%
    if (pctDuty >= 0 && pctDuty <= 100) {
      dutyCycleAtual = map(pctDuty, 0, 100, 0, 255);
     
      // Nova API v3.x: ledcWrite agora aceita diretamente o PINO em vez do canal!
      ledcWrite(LED_PIN, dutyCycleAtual);
      alterouAlgo = true;
    }
  }


  if (alterouAlgo) {
    int pctExibicao = map(dutyCycleAtual, 0, 255, 0, 100);
    String stringResposta = "Frequência configurada em <b>" + String(frequenciaAtual) + " Hz</b> e Duty Cycle em <b>" + String(pctExibicao) + "%</b>.";
   
    Serial.printf("Ajuste via Web -> Freq: %d Hz | Duty: %d/255 (%d%%)\n", frequenciaAtual, dutyCycleAtual, pctExibicao);
   
    server.send(200, "text/plain", stringResposta);
  } else {
    server.send(400, "text/plain", "Parâmetros inválidos ou ausentes.");
  }
}


// ==========================================
// SETUP E CONFIGURAÇÃO DO HARDWARE
// ==========================================


void setup() {
  Serial.begin(115200);


  // Nova API v3.x: Inicialização simplificada e associada diretamente ao PINO
  ledcAttach(LED_PIN, frequenciaAtual, LEDC_RESOLUTION);
  ledcWrite(LED_PIN, dutyCycleAtual);


  // Inicialização do Ponto de Acesso
  WiFi.softAP(ssid, password);
  Serial.println("\n=== Servidor de Controle PWM Ativo (API v3.x) ===");
  Serial.print("Conecte na rede Wi-Fi: ");
  Serial.println(ssid);
  Serial.print("Acesse o painel no navegador: http://");
  Serial.println(WiFi.softAPIP());


  // Mapeamento das Rotas HTTP
  server.on("/", handleHome);
  server.on("/setpwm", handleSetPWM);


  server.begin();
}


void loop() {
  server.handleClient();
}
