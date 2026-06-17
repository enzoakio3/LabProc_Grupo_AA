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
    <title>Dashboard da Calculadora (Entrada 16b | Saída 32b)</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; background-color: #f4f7f6; padding: 20px; color: #333; }
        .container { max-width: 550px; margin: 30px auto; background: white; padding: 30px; border-radius: 14px; box-shadow: 0px 5px 20px rgba(0,0,0,0.08); }
        h2 { color: #0056b3; margin-bottom: 5px; }
        .config-zone { background: #e9ecef; padding: 12px; border-radius: 8px; margin-bottom: 20px; font-size: 14px; font-weight: bold; color: #495057; display: flex; justify-content: center; align-items: center; gap: 10px; }
        label { font-weight: bold; display: block; margin-top: 15px; text-align: left; margin-left: 10%; font-size: 14px; color: #555; }
        input { font-size: 18px; padding: 10px; width: 80%; text-align: center; margin-top: 5px; margin-bottom: 5px; border-radius: 8px; border: 2px solid #ddd; font-family: monospace; }
        input:focus { border-color: #007bff; outline: none; }
        input:disabled { background-color: #e9ecef; color: #6c757d; cursor: not-allowed; border-color: #dee2e6; }
        .btn-group { margin-top: 20px; margin-bottom: 20px; display: flex; flex-wrap: wrap; justify-content: center; }
        button { font-size: 14px; padding: 10px 18px; margin: 5px; cursor: pointer; border: none; border-radius: 8px; font-weight: bold; transition: 0.2s; }
        .btn-soma { background-color: #28a745; color: white; }
        .btn-soma:hover { background-color: #218838; }
        .btn-sub { background-color: #dc3545; color: white; }
        .btn-sub:hover { background-color: #c82333; }
        .btn-mul { background-color: #ffc107; color: #212529; }
        .btn-mul:hover { background-color: #e0a800; }
        .btn-div { background-color: #9400d3; color: white; }
        .btn-div:hover { background-color: #7a00b8; }
        .btn-fat { background-color: #17a2b8; color: white; }
        .btn-fat:hover { background-color: #138496; }
        .panel-resultado { margin-top: 25px; padding: 15px; border-radius: 8px; background-color: #f8f9fa; border-left: 5px solid #007bff; text-align: left; padding-left: 25px; }
        .res-linha { font-size: 15px; margin: 5px 0; word-break: break-all; }
        .binario-box { font-family: monospace; background: #e9ecef; padding: 6px; border-radius: 4px; display: inline-block; margin-top: 4px; font-size: 13px; letter-spacing: 1px; }
        .alert-overflow { background-color: #f8d7da; border-left: 5px solid #721c24; color: #721c24; font-weight: bold; }
        .tempo-tag { font-size: 12px; color: #6c757d; margin-top: 8px; font-family: monospace; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Dashboard da Calculadora</h2>
       
        <div class="config-zone">
            <span>Arquitetura: Entrada Fixa 16 Bits | Saída Expandida 32 Bits</span>
        </div>
       
        <label id="labelA">Operando A (binário de 16 bits):</label>
        <input type="text" id="opA" maxlength="16" placeholder="Insira 16 bits">
       
        <label id="labelB">Operando B (binário de 16 bits):</label>
        <input type="text" id="opB" maxlength="16" placeholder="Insira 16 bits">
       
        <div class="btn-group">
            <button class="btn-soma" onclick="configurarCampos('add'); calcular('add')">SOMA (+)</button>
            <button class="btn-sub" onclick="configurarCampos('sub'); calcular('sub')">SUB (-)</button>
            <button class="btn-mul" onclick="configurarCampos('mul'); calcular('mul')">MULT (*)</button>
            <button class="btn-div" onclick="configurarCampos('div'); calcular('div')">DIV (/)</button>
            <button class="btn-fat" onclick="configurarCampos('fat'); calcular('fat')">FAT (!)</button>
        </div>
       
        <div class="panel-resultado" id="resPainel">
            <div class="res-linha">Aguardando operação...</div>
        </div>
    </div>


    <script>
        const numBitsEntrada = 16;


        function configurarCampos(operacao) {
            let campoB = document.getElementById('opB');
            let labelB = document.getElementById('labelB');
            if (operacao === 'fat') {
                campoB.disabled = true;
                campoB.value = "0".repeat(numBitsEntrada);
                labelB.style.color = "#bbb";
            } else {
                campoB.disabled = false;
                if (campoB.value === "0".repeat(numBitsEntrada)) campoB.value = "";
                labelB.style.color = "#555";
            }
        }


        function calcular(operacao) {
            let a = document.getElementById('opA').value.trim();
            let b = document.getElementById('opB').value.trim();
           
            let regex = new RegExp("^[01]{" + numBitsEntrada + "}$");
           
            if (!regex.test(a) || !regex.test(b)) {
                alert("Erro: Insira exatamente " + numBitsEntrada + " caracteres binários (0 ou 1) nos campos ativos.");
                return;
            }
           
            let url = '/calc?a=' + a + '&b=' + b + '&op=' + operacao;
           
            fetch(url)
                .then(response => {
                    let statusHttp = response.status;
                    return response.text().then(texto => { return { status: statusHttp, body: texto }; });
                })
                .then(res => {
                    let painel = document.getElementById('resPainel');
                   
                    if (res.body.includes("OVERFLOW!") || res.body.includes("ERRO") || res.status === 422) {
                        painel.className = "panel-resultado alert-overflow";
                        painel.innerHTML = "<div style='color: #721c24; font-weight: bold; font-size: 20px; margin-bottom: 10px;'>⚠️ EXCEÇÃO DETECTADA!</div>" +
                                           "<div class='res-linha'><b>Código HTTP:</b> " + res.status + "</div>" +
                                           "<div class='res-linha'>" + res.body + "</div>";
                    } else {
                        painel.className = "panel-resultado";
                        if (operacao === 'add') painel.style.borderLeftColor = '#28a745';
                        else if (operacao === 'sub') painel.style.borderLeftColor = '#dc3545';
                        else if (operacao === 'mul') painel.style.borderLeftColor = '#ffc107';
                        else if (operacao === 'div') painel.style.borderLeftColor = '#9400d3';
                        else if (operacao === 'fat') painel.style.borderLeftColor = '#17a2b8';
                       
                        let opNome = 'Soma';
                        if (operacao === 'sub') opNome = 'Subtração';
                        if (operacao === 'mul') opNome = 'Multiplicação';
                        if (operacao === 'div') opNome = 'Divisão Inteira';
                        if (operacao === 'fat') opNome = 'Fatorial';


                        painel.innerHTML = "<div class='res-linha'><b>Operação:</b> " + opNome + "</div>" +
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


// Multiplicação por software operando em largura total de 32 bits
int32_t multiply32(int32_t a, int32_t b) {
  int64_t result = 0;
  int32_t absolutoB = abs(b);
  for (int32_t i = 0; i < absolutoB; i++) {
    result += a;
    // Monitora estouro real do barramento estrito de 32 bits
    if (result < -2147483648LL || result > 2147483647LL) return -999999;
  }
  if (b < 0) {
    result = -result;
  }
  if (result < -2147483648LL || result > 2147483647LL) return -999999;
  return (int32_t)result;
}


// Fatorial estendido com monitoramento estável até o limite físico de 32 bits
int32_t factorial32(int32_t n) {
  if (n < 0) return -1;
  if (n <= 1) return 1;
  int64_t result = 1;
  for (int32_t i = 2; i <= n; i++) {
    result = result * i;
    if (result > 2147483647LL) return -999999;
  }
  return (int32_t)result;
}


// Divisão Inteira por Software processando em 32 bits
bool divide32(int32_t dividendo, int32_t divisor, int32_t &quociente, int32_t &resto) {
  if (divisor == 0) return false;


  int32_t absDividendo = abs(dividendo);
  int32_t absDivisor = abs(divisor);
 
  quociente = 0;
  resto = absDividendo;


  while (resto >= absDivisor) {
    resto -= absDivisor;
    quociente++;
  }


  if ((dividendo < 0 ^ divisor < 0) && quociente != 0) {
    quociente = -quociente;
  }
  if (dividendo < 0 && resto != 0) {
    resto = -resto;
  }


  return true;
}


void calcular() {
  String paramA = server.arg("a");
  String paramB = server.arg("b");
  String op = server.arg("op");


  // Isolamento dos 16 bits de entrada
  uint16_t mascaraEntrada = 0xFFFF;
  uint16_t bitSinalEntrada = 0x8000;


  uint16_t rawA = (uint16_t)strtol(paramA.c_str(), NULL, 2) & mascaraEntrada;
  uint16_t rawB = (uint16_t)strtol(paramB.c_str(), NULL, 2) & mascaraEntrada;


  // Extensão de sinal correta de 16 bits para as variáveis internas de 32 bits
  int32_t valA = (rawA & bitSinalEntrada) ? (int32_t)(rawA - 65536) : (int32_t)rawA;
  int32_t valB = (rawB & bitSinalEntrada) ? (int32_t)(rawB - 65536) : (int32_t)rawB;


  // Definição da Barreira Física de Saída de 32 Bits em Complemento de Dois
  int64_t minLimite32 = -2147483648LL;
  int64_t maxLimite32 = 2147483647LL;


  int64_t resultadoAcumulado = 0;
  int32_t restoAcumulado = 0;
  bool overflow = false;
  bool erroDivisaoPorZero = false;
  unsigned long tempoDecorrido = 0;


  // ==========================================
  // PROCESSAMENTO ARITMÉTICO
  // ==========================================
  if (op == "add") {
    unsigned long tempoInicio = micros();
    resultadoAcumulado = (int64_t)valA + (int64_t)valB;
    tempoDecorrido = micros() - tempoInicio;
  }
  else if (op == "sub") {
    unsigned long tempoInicio = micros();
    resultadoAcumulado = (int64_t)valA - (int64_t)valB;
    tempoDecorrido = micros() - tempoInicio;
  }
  else if (op == "mul") {
    unsigned long tempoInicio = micros();
    int32_t prod = multiply32(valA, valB);
    tempoDecorrido = micros() - tempoInicio;
   
    if (prod == -999999) overflow = true;
    else resultadoAcumulado = prod;
  }
  else if (op == "div") {
    unsigned long tempoInicio = micros();
    int32_t q = 0, r = 0;
    bool divSucesso = divide32(valA, valB, q, r);
    tempoDecorrido = micros() - tempoInicio;


    if (!divSucesso) {
      erroDivisaoPorZero = true;
    } else {
      resultadoAcumulado = q;
      restoAcumulado = r;
    }
  }
  else if (op == "fat") {
    unsigned long tempoInicio = micros();
    int32_t fat = factorial32(valA);
    tempoDecorrido = micros() - tempoInicio;
   
    if (valA < 0 || fat == -999999) overflow = true;
    else resultadoAcumulado = fat;
  }


  // Validação real de estouro contra o limite de 32 bits
  if (resultadoAcumulado < minLimite32 || resultadoAcumulado > maxLimite32) {
    overflow = true;
  }


  uint32_t resultado32Bits = (uint32_t)resultadoAcumulado & 0xFFFFFFFF;
  int32_t decimalExibicao = (int32_t)resultado32Bits;


  // Log de monitoramento serial estruturado
  Serial.printf("Log: Op: %s | Arquitetura: 16b->32b | A: %d | B: %d | Res 32b: %d | Resto: %d | Tempo: %lu us\n",
                op.c_str(), valA, valB, decimalExibicao, restoAcumulado, tempoDecorrido);


  String respostaTexto = "";


  if (erroDivisaoPorZero) {
    digitalWrite(LED_BIT3, HIGH); digitalWrite(LED_BIT2, HIGH);
    digitalWrite(LED_BIT1, HIGH); digitalWrite(LED_BIT0, HIGH);
   
    respostaTexto = "ERRO DETECTADO: Divisão por Zero não é permitida.";
    server.send(422, "text/plain", respostaTexto);
  }
  else if (overflow) {
    digitalWrite(LED_BIT3, HIGH); digitalWrite(LED_BIT2, HIGH);
    digitalWrite(LED_BIT1, HIGH); digitalWrite(LED_BIT0, HIGH);
   
    respostaTexto = "OVERFLOW! O resultado extrapolou o limite global de 32 bits em Complemento de Dois (-2147483648 a 2147483647).";
    server.send(422, "text/plain", respostaTexto);
  }
  else {
    // Atualiza os LEDs físicos com os 4 bits menos significativos (LSB) do resultado de 32 bits
    digitalWrite(LED_BIT0, (resultado32Bits >> 0) & 0x01);
    digitalWrite(LED_BIT1, (resultado32Bits >> 1) & 0x01);
    digitalWrite(LED_BIT2, (resultado32Bits >> 2) & 0x01);
    digitalWrite(LED_BIT3, (resultado32Bits >> 3) & 0x01);


    String binarioStr = "";
    for (int i = 31; i >= 0; i--) {
      binarioStr += String((resultado32Bits >> i) & 0x01);
      if (i % 4 == 0 && i != 0) binarioStr += " ";
    }
   
    respostaTexto = "<b>Resultado Decimal (32-bits):</b> " + String(decimalExibicao);
    if (op == "div") {
       respostaTexto += " | <b>Resto:</b> " + String(restoAcumulado);
    }
    respostaTexto += "<br><b>Binário do Barramento:</b> <div class='binario-box'>" + binarioStr + "</div>" +
                     "<div class='tempo-tag'>⏱️ Tempo de execução no chip: " + String(tempoDecorrido) + " us</div>";
                   
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
  Serial.println("\n=== Sistema Prontificado (Entrada 16b -> Saída 32b) ===");
  Serial.print("Acesse o painel em: http://");
  Serial.println(WiFi.softAPIP());


  server.on("/", home);
  server.on("/calc", calcular);


  server.begin();
}


void loop() {
  server.handleClient();
}
