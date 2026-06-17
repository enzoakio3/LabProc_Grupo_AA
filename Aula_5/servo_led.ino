
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h> // Biblioteca do Servo


const char* ssid = "Painel_Integrado_ESP32_C3";
const char* password = "12345678";


WebServer server(80);
Servo meuServo;


const int SERVO_PIN = 4;
const int LED_PIN = 5;                                                                                                            


const int LEDC_RESOLUTION = 8;
int frequenciaAtual = 1000;
int dutyCycleAtual = 128;


// =========================================================================
// INTERFACE HTML UNIFICADA (LED + SERVO NA MESMA TELA)
// =========================================================================
const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Painel de Controle Central: LED & Servo</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background-color: #f4f7f6; padding: 20px; color: #333; }
        .container { max-width: 500px; margin: 20px auto; background: white; padding: 25px; border-radius: 14px; box-shadow: 0px 5px 20px rgba(0,0,0,0.08); }
        h2 { color: #0056b3; margin-bottom: 20px; border-bottom: 2px solid #e9ecef; padding-bottom: 10px; }
       
        .card { background: #f8f9fa; padding: 20px; border-radius: 10px; margin-bottom: 20px; text-align: left; box-shadow: inset 0 0 5px rgba(0,0,0,0.02); }
        .card-led { border-left: 5px solid #ffc107; }
        .card-servo { border-left: 5px solid #007bff; }
       
        label { font-weight: bold; display: block; font-size: 14px; color: #555; margin-bottom: 5px; }
        .slider-container { display: flex; align-items: center; justify-content: space-between; margin-top: 8px; }
        input[type=range] { flex: 1; margin-right: 15px; height: 8px; border-radius: 5px; background: #ddd; outline: none; }
        .valor-display { font-family: monospace; font-size: 18px; font-weight: bold; width: 60px; text-align: right; color: #007bff; }
       
        .btn-group { margin-top: 15px; display: flex; justify-content: space-between; gap: 5px; }
        button { font-size: 12px; padding: 8px 12px; flex: 1; cursor: pointer; border: none; border-radius: 6px; font-weight: bold; background-color: #e9ecef; color: #495057; transition: 0.2s; }
        button.active { background-color: #ffc107; color: #212529; }
        button.btn-servo { background-color: #6c757d; color: white; }
        button.btn-servo:hover { background-color: #495057; }
        button:hover { background-color: #adb5bd; color: white; }
       
        .panel-status { padding: 12px; border-radius: 8px; background-color: #e2f0d9; border-left: 5px solid #28a745; text-align: left; font-size: 13px; margin-top: 15px; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Painel de Controle Central</h2>
       
        <div class="card card-led">
            <label>Intensidade do LED (Duty Cycle 0-100%):</label>
            <div class="slider-container">
                <input type="range" id="sliderDuty" min="0" max="100" value="50" oninput="atualizarDuty(this.value)">
                <span class="valor-display" id="displayDuty">50%</span>
            </div>
           
            <label style="margin-top: 15px;">Frequência do PWM (Flicker Test):</label>
            <div class="btn-group">
                <button id="btn10Hz" onclick="alterarFrequencia(10)">10 Hz</button>
                <button id="btn60Hz" onclick="alterarFrequencia(60)">60 Hz</button>
                <button id="btn1kHz" class="active" onclick="alterarFrequencia(1000)">1 kHz</button>
                <button id="btn5kHz" onclick="alterarFrequencia(5000)">5 kHz</button>
            </div>
        </div>
       
        <div class="card card-servo">
            <label>Posição do Servomotor (0° a 180°):</label>
            <div class="slider-container">
                <input type="range" id="servoSlider" min="0" max="180" value="90" oninput="atualizarServo(this.value)">
                <span class="valor-display" id="displayServo" style="color:#007bff;">90°</span>
            </div>
           
            <div class="btn-group">
                <button class="btn-servo" onclick="definirPosicaoServo(0)">0° (Mín)</button>
                <button class="btn-servo" onclick="definirPosicaoServo(90)">90° (Centro)</button>
                <button class="btn-servo" onclick="definirPosicaoServo(180)">180° (Máx)</button>
            </div>
        </div>
       
        <div class="panel-status">
            <b>Status do Hardware:</b> <span id="statusTexto">Conectado ao ESP32-C3.</span>
        </div>
    </div>


    <script>
        let timeoutDuty = null;
        let timeoutServo = null;


        // --- Funções de Controle do LED ---
        function atualizarDuty(porcentagem) {
            document.getElementById('displayDuty').innerText = porcentagem + "%";
            clearTimeout(timeoutDuty);
            timeoutDuty = setTimeout(() => {
                enviarComando('/setledpwm?duty=' + porcentagem);
            }, 50);
        }


        function alterarFrequencia(freq) {
            const botoes = document.querySelectorAll('.card-led .btn-group button');
            botoes.forEach(btn => btn.classList.remove('active'));
           
            if(freq === 10) document.getElementById('btn10Hz').classList.add('active');
            if(freq === 60) document.getElementById('btn60Hz').classList.add('active');
            if(freq === 1000) document.getElementById('btn1kHz').classList.add('active');
            if(freq === 5000) document.getElementById('btn5kHz').classList.add('active');
           
            enviarComando('/setledpwm?freq=' + freq);
        }


        // --- Funções de Controle do Servo ---
        function atualizarServo(angulo) {
            document.getElementById('displayServo').innerText = angulo + "°";
            clearTimeout(timeoutServo);
            timeoutServo = setTimeout(() => {
                enviarComando('/setservo?pos=' + angulo);
            }, 50);
        }


        function definirPosicaoServo(angulo) {
            document.getElementById('servoSlider').value = angulo;
            document.getElementById('displayServo').innerText = angulo + "°";
            enviarComando('/setservo?pos=' + angulo);
        }


        // --- Emissor AJAX Genérico ---
        function enviarComando(url) {
            fetch(url)
                .then(response => response.text())
                .then(texto => {
                    document.getElementById('statusTexto').innerHTML = texto;
                })
                .catch(err => {
                    document.getElementById('statusTexto').innerHTML = "<span style='color:red;'>Erro na comunicação de rede.</span>";
                });
        }
    </script>
</body>
</html>
)rawliteral";


void handleHome() {
  server.send(200, "text/html", PAGINA_HTML);
}


void handleSetLED() {
  bool alterouAlgo = false;


  if (server.hasArg("freq")) {
    int novaFreq = server.arg("freq").toInt();
    if (novaFreq >= 1 && novaFreq <= 40000) {
      frequenciaAtual = novaFreq;
      ledcAttach(LED_PIN, frequenciaAtual, LEDC_RESOLUTION);
      ledcWrite(LED_PIN, dutyCycleAtual);
      alterouAlgo = true;
    }
  }


  if (server.hasArg("duty")) {
    int pctDuty = server.arg("duty").toInt();
    if (pctDuty >= 0 && pctDuty <= 100) {
      dutyCycleAtual = map(pctDuty, 0, 100, 0, 255);
      ledcWrite(LED_PIN, dutyCycleAtual);
      alterouAlgo = true;
    }
  }


  if (alterouAlgo) {
    int pctExibicao = map(dutyCycleAtual, 0, 255, 0, 100);
    String resposta = "LED -> Frequência: <b>" + String(frequenciaAtual) + " Hz</b> | Brilho: <b>" + String(pctExibicao) + "%</b>";
    server.send(200, "text/plain", resposta);
  } else {
    server.send(400, "text/plain", "Parâmetros inválidos.");
  }
}


void handleSetServo() {
  if (server.hasArg("pos")) {
    int angulo = server.arg("pos").toInt();
    if (angulo < 0) angulo = 0;
    if (angulo > 180) angulo = 180;
   
    meuServo.write(angulo);
   
    String resposta = "Servo -> Ângulo alterado para <b>" + String(angulo) + "°</b>";
    server.send(200, "text/plain", resposta);
  } else {
    server.send(400, "text/plain", "Parâmetro 'pos' ausente.");
  }
}


void setup() {
Serial.begin(115200);


  ledcAttach(LED_PIN, frequenciaAtual, LEDC_RESOLUTION);
  ledcWrite(LED_PIN, dutyCycleAtual);


  ESP32PWM::allocateTimer(1);
 
  meuServo.setPeriodHertz(50);
 
  meuServo.attach(SERVO_PIN, 500, 2400);
  meuServo.write(90);


  WiFi.softAP(ssid, password);
  Serial.println("\n=== Servidor Central LED & Servo Ativo ===");
  Serial.print("Wi-Fi: "); Serial.println(ssid);
  Serial.print("URL de Acesso: http://"); Serial.println(WiFi.softAPIP());


  server.on("/", handleHome);
  server.on("/setledpwm", handleSetLED);
  server.on("/setservo", handleSetServo);


  server.begin();
}


void loop() {
  server.handleClient();
}
