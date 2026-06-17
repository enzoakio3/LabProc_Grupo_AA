#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Calculadora_ESP32_Grupo_AA";
const char* password = "12345678";

WebServer server(80);

const int LED_BIT3 = 7;
const int LED_BIT2 = 6;
const int LED_BIT1 = 5;
const int LED_BIT0 = 4;

// ==========================================
// INTERFACE HTML COM JAVASCRIPT EMBUTIDO
// ==========================================
const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard da Calculadora</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background-color: #f4f7f6; padding: 20px; color: #333; }
        .container { max-width: 420px; margin: 30px auto; background: white; padding: 30px; border-radius: 14px; box-shadow: 0px 5px 20px rgba(0,0,0,0.08); }
        h2 { color: #0056b3; margin-bottom: 25px; }
        label { font-weight: bold; display: block; margin-top: 15px; text-align: left; margin-left: 10%; font-size: 14px; color: #555; }
        input { font-size: 20px; padding: 10px; width: 80%; text-align: center; margin-top: 5px; margin-bottom: 5px; border-radius: 8px; border: 2px solid #ddd; font-family: monospace; }
        input:focus { border-color: #007bff; outline: none; }
        .btn-group { margin-top: 20px; margin-bottom: 20px; }
        button { font-size: 16px; padding: 12px 24px; margin: 5px; cursor: pointer; border: none; border-radius: 8px; font-weight: bold; transition: 0.2s; }
        .btn-soma { background-color: #28a745; color: white; }
        .btn-soma:hover { background-color: #218838; }
        .btn-sub { background-color: #dc3545; color: white; }
        .btn-sub:hover { background-color: #c82333; }
        .panel-resultado { margin-top: 25px; padding: 15px; border-radius: 8px; background-color: #f8f9fa; border-left: 5px solid #007bff; text-align: left; padding-left: 25px; }
        .res-linha { font-size: 16px; margin: 5px 0; }
        .res-destaque { font-weight: bold; font-size: 18px; color: #111; }
        .alert-overflow { background-color: #f8d7da; border-left: 5px solid #721c24; color: #721c24; font-weight: bold; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Dashboard da Calculadora</h2>
        
        <label>Operando A (4 bits binários):</label>
        <input type="text" id="opA" placeholder="Ex: 0011" maxlength="4">
        
        <label>Operando B (4 bits binários):</label>
        <input type="text" id="opB" placeholder="Ex: 0010" maxlength="4">
        
        <div class="btn-group">
            <button class="btn-soma" onclick="calcular('add')">SOMA (+)</button>
            <button class="btn-sub" onclick="calcular('sub')">SUB (-)</button>
        </div>
        
        <div class="panel-resultado" id="resPainel">
            <div class="res-linha">Aguardando operação...</div>
        </div>
    </div>

    <script>
        function calcular(operacao) {
            let a = document.getElementById('opA').value.trim();
            let b = document.getElementById('opB').value.trim();
            
            if (!/^[01]{4}$/.test(a) || !/^[01]{4}$/.test(b)) {
                alert("Erro: Insira exatamente 4 caracteres binários (0 ou 1) in ambos os campos.");
                return;
            }
            
            let url = '/calc?a=' + a + '&b=' + b + '&op=' + operacao;
            
            fetch(url)
                .then(response => {
                    // Guarda o status HTTP recebido do ESP32 (ex: 200 ou 422)
                    let statusHttp = response.status;
                    
                    // Retorna o texto puro e repassa o status para o próximo bloco
                    return response.text().then(texto => { return { status: statusHttp, body: texto }; });
                })
                .then(res => {
                    let painel = document.getElementById('resPainel');
                    
                    // Se o texto contiver OVERFLOW ou o status for diferente de 200, renderiza erro
                    if (res.body.includes("OVERFLOW!") || res.status === 422) {
                        painel.className = "panel-resultado alert-overflow";
                        painel.innerHTML = "<div style='color: #721c24; font-weight: bold; font-size: 20px; margin-bottom: 10px;'>⚠️ OVERFLOW DETECTADO!</div>" +
                                           "<div class='res-linha'><b>Código HTTP:</b> " + res.status + " Unprocessable Entity</div>" +
                                           "<div class='res-linha'>" + res.body + "</div>";
                    } else {
                        // Fluxo normal de sucesso (HTTP 200)
                        painel.className = "panel-resultado";
                        painel.style.borderLeftColor = operacao === 'add' ? '#28a745' : '#dc3545';
                        painel.innerHTML = "<div class='res-linha'><b>Operação:</b> " + (operacao === 'add' ? 'Soma' : 'Subtração') + "</div>" +
                                           "<div class='res-linha'><b>Código HTTP:</b> 200 OK</div>" +
                                           "<div class='res-linha'>" + res.body + "</div>";
                    }
                })
                .catch(err => {
                    alert("Erro ao comunicar com o servidor da calculadora.");
                });
        }
    </script>
</body>
</html>
)rawliteral";

void home() {
  server.send(200, "text/html", PAGINA_HTML);
}

void calcular() {
  String paramA = server.arg("a");
  String paramB = server.arg("b");
  String op = server.arg("op");

  uint8_t a = (uint8_t)strtol(paramA.c_str(), NULL, 2) & 0x0F;
  uint8_t b = (uint8_t)strtol(paramB.c_str(), NULL, 2) & 0x0F;

  uint8_t sinal_a = (a >> 3) & 0x01;
  uint8_t sinal_b = (b >> 3) & 0x01;

  uint8_t resultado = 0;
  bool overflow = false;

  if (op == "add") {
    resultado = (a + b) & 0x0F;
    uint8_t sinal_res = (resultado >> 3) & 0x01;

    if ((sinal_a == sinal_b) && (sinal_res != sinal_a)) {
      overflow = true;
    }
  } else if (op == "sub") {
    resultado = (a - b) & 0x0F;
    uint8_t sinal_res = (resultado >> 3) & 0x01;

    if ((sinal_a != sinal_b) && (sinal_res != sinal_a)) {
      overflow = true;
    }
  }

  int8_t decimal = resultado;
  if (resultado > 7) {
    decimal -= 16;
  }

  // COMPLEMENTO DE 1
  uint8_t res_comp1 = 0;
  bool overflow_comp1 = false;
  int8_t decimal_comp1 = 0;

  if (op == "add") {
    uint16_t soma_temp = a + b;
    if (soma_temp > 0x0F) {
      res_comp1 = (soma_temp + 1) & 0x0F;
    } else {
      res_comp1 = soma_temp & 0x0F;
    }
    
    uint8_t sinal_res1 = (res_comp1 >> 3) & 0x01;
    if ((sinal_a == sinal_b) && (sinal_res1 != sinal_a)) overflow_comp1 = true;

  } else if (op == "sub") {
    uint8_t b_invertido = (~b) & 0x0F;
    uint8_t sinal_b_inv = (b_invertido >> 3) & 0x01;
    
    uint16_t soma_temp = a + b_invertido;
    if (soma_temp > 0x0F) {
      res_comp1 = (soma_temp + 1) & 0x0F;
    } else {
      res_comp1 = soma_temp & 0x0F;
    }

    uint8_t sinal_res1 = (res_comp1 >> 3) & 0x01;
    if ((sinal_a == sinal_b_inv) && (sinal_res1 != sinal_a)) overflow_comp1 = true;
  }

  if ((res_comp1 >> 3) & 0x01) {
    decimal_comp1 = -((~res_comp1) & 0x07); 
  } else {
    decimal_comp1 = res_comp1;
  }

  Serial.printf("Inputs: A = %d (bin), B = %d (bin) | Operação: %s\n", a, b, op.c_str());

  if (overflow_comp1) {
    Serial.println("OVERFLOW!");
  } else {
    if (res_comp1 == 0x0F) {
      Serial.printf("Decimal: -0 | Binário: 1111\n");
    } else {
      Serial.printf("Decimal: %d | Binário: %d%d%d%d\n", 
                    decimal_comp1, (res_comp1>>3)&1, (res_comp1>>2)&1, (res_comp1>>1)&1, res_comp1&1);
    }
  }

  String respostaTexto = "";

  if (overflow) {
    digitalWrite(LED_BIT3, HIGH); digitalWrite(LED_BIT2, HIGH);
    digitalWrite(LED_BIT1, HIGH); digitalWrite(LED_BIT0, HIGH);

    respostaTexto = "OVERFLOW! O resultado estourou a capacidade de 4 bits em Complemento de Dois (-8 a +7).";
    
    server.send(422, "text/plain", respostaTexto);
  } else {
    digitalWrite(LED_BIT0, (resultado >> 0) & 0x01);
    digitalWrite(LED_BIT1, (resultado >> 1) & 0x01);
    digitalWrite(LED_BIT2, (resultado >> 2) & 0x01);
    digitalWrite(LED_BIT3, (resultado >> 3) & 0x01);

    String binarioStr = "";
    for (int i = 3; i >= 0; i--) {
      binarioStr += String((resultado >> i) & 0x01);
    }
    respostaTexto = "<b>Resultado Decimal:</b> " + String(decimal) + " | <b>Binário:</b> " + binarioStr;

    server.send(200, "text/plain", respostaTexto);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BIT3, OUTPUT); pinMode(LED_BIT2, OUTPUT);
  pinMode(LED_BIT1, OUTPUT); pinMode(LED_BIT0, OUTPUT);
  digitalWrite(LED_BIT3, LOW); digitalWrite(LED_BIT2, LOW);
  digitalWrite(LED_BIT1, LOW); digitalWrite(LED_BIT0, LOW);

  WiFi.softAP(ssid, password);
  Serial.println("\n=== Sistema da Calculadora Operando ===");
  Serial.print("Conecte no Wi-Fi e acesse: http://");
  Serial.println(WiFi.softAPIP());

  //  Endpoints
  server.on("/", home);
  server.on("/calc", calcular);

  server.begin();
}

void loop() {
  server.handleClient();
}
