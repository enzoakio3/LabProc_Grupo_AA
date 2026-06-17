#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h> // Inclui a biblioteca específica para ESP32


const char* ssid = "Servo_ESP32_Grupo_AA";
const char* password = "12345678";


WebServer server(80);
Servo meuServo; // Cria o objeto para controlar o servo


// =========================================================================
// CONFIGURAÇÃO DO HARDWARE
// =========================================================================
const int pinoServo = 4; // Seu pino GPIO físico do ESP32-C3


// =========================================================================
// INTERFACE HTML + SLIDER JAVASCRIPT
// =========================================================================
const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controle de Posição do Servo</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background-color: #f4f7f6; padding: 20px; color: #333; }
        .container { max-width: 500px; margin: 30px auto; background: white; padding: 30px; border-radius: 14px; box-shadow: 0px 5px 20px rgba(0,0,0,0.08); }
        h2 { color: #007bff; margin-bottom: 20px; }
        .slider-zone { margin: 40px 0; }
        input[type=range] { width: 90%; height: 15px; border-radius: 8px; background: #ddd; outline: none; -webkit-appearance: none; }
        input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 25px; height: 25px; border-radius: 50%; background: #007bff; cursor: pointer; transition: 0.1s; }
        input[type=range]::-webkit-slider-thumb:hover { background: #0056b3; transform: scale(1.1); }
        .angulo-display { font-size: 48px; font-weight: bold; color: #343a40; font-family: monospace; margin-top: 10px; }
        .btn-group { display: flex; justify-content: space-around; margin-top: 20px; }
        button { font-size: 14px; padding: 10px 20px; cursor: pointer; border: none; border-radius: 8px; font-weight: bold; background-color: #6c757d; color: white; transition: 0.2s; }
        button:hover { background-color: #5a6268; }
        .status-tag { font-size: 12px; color: #6c757d; margin-top: 25px; font-family: monospace; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Controle do Servomotor</h2>
       
        <div class="slider-zone">
            <div class="angulo-display"><span id="valAngulo">90</span>°</div>
            <input type="range" id="servoSlider" min="0" max="180" value="90" oninput="atualizarServo(this.value)">
        </div>
       
        <div class="btn-group">
            <button onclick="definirPosicao(0)">0° (Mínimo)</button>
            <button onclick="definirPosicao(90)">90° (Centro)</button>
            <button onclick="definirPosicao(180)">180° (Máximo)</button>
        </div>
       
        <div class="status-tag" id="statusEnvio">Conectado ao ESP32-C3</div>
    </div>


    <script>
        let timeoutPosicao = null;


        function atualizarServo(angulo) {
            document.getElementById('valAngulo').innerText = angulo;
            clearTimeout(timeoutPosicao);
            timeoutPosicao = setTimeout(() => {
                enviarComando(angulo);
            }, 50);
        }


        function definirPosicao(angulo) {
            document.getElementById('servoSlider').value = angulo;
            document.getElementById('valAngulo').innerText = angulo;
            enviarComando(angulo);
        }


        function enviarComando(angulo) {
            let url = '/setServo?pos=' + angulo;
            let statusTag = document.getElementById('statusEnvio');
           
            fetch(url)
                .then(response => {
                    if(response.ok) {
                        statusTag.innerText = "Ângulo atualizado: " + angulo + "°";
                        statusTag.style.color = "#28a745";
                    } else {
                        statusTag.innerText = "Erro no servidor.";
                        statusTag.style.color = "#dc3545";
                    }
                })
                .catch(err => {
                    statusTag.innerText = "Erro de comunicação.";
                    statusTag.style.color = "#dc3545";
                });
        }
    </script>
</body>
</html>
)rawliteral";


void home() {
  server.send(200, "text/html", PAGINA_HTML);
}


void moverServo() {
  String paramPos = server.arg("pos");
  int angulo = paramPos.toInt();


  // Restringe o ângulo por segurança
  if (angulo < 0) angulo = 0;
  if (angulo > 180) angulo = 180;


  // Comando simples da biblioteca para mover o motor
  meuServo.write(angulo);


  Serial.printf("Log Web: Ângulo solicitado: %d° enviado via biblioteca no Pino %d\n", angulo, pinoServo);


  server.send(200, "text/plain", "OK");
}


void setup() {
  Serial.begin(115200);


  // Aloca automaticamente os timers internos de PWM do ESP32 para o Servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
 
  // Configura a frequência padrão de servos (50Hz)
  meuServo.setPeriodHertz(50);
 
  // Inicializa o servo associando-o ao pino físico correspondente.
  // Os valores 500 e 2400 são os limites padrão de largura de pulso (em microssegundos) para abranger de 0° a 180°.
  meuServo.attach(pinoServo, 500, 2400);


  // Define a posição inicial no centro (90 graus)
  meuServo.write(90);


  // Inicialização do Access Point
  WiFi.softAP(ssid, password);
  Serial.println("\n=== Sistema de Controle Prontificado (Com Biblioteca) ===");
  Serial.print("Conecte no Wi-Fi e acesse: http://");
  Serial.println(WiFi.softAPIP());


  // Rotas do Servidor Web
  server.on("/", home);
  server.on("/setServo", moverServo);


  server.begin();
}


void loop() {
  server.handleClient();
}
