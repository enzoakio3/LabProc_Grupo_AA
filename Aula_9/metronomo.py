import time
import RPi.GPIO as GPIO

# Pinos validados nos testes isolados
SERVO_PIN = 18
BUZZER_PIN = 4

# Configuração do metrônomo
PERIODO = 1.0              # 1 segundo = 1 Hz = 60 BPM
FREQUENCIA_SERVO = 50      # PWM do servomotor
FREQUENCIA_BUZZER = 1000   # Frequência do som
DURACAO_SOM = 0.08         # 80 ms

# Limites que devem ser ajustados após o teste isolado
SERVO_ESQUERDA = 5.5
SERVO_DIREITA = 9.5

GPIO.setmode(GPIO.BCM)

GPIO.setup(SERVO_PIN, GPIO.OUT)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

servo = GPIO.PWM(SERVO_PIN, FREQUENCIA_SERVO)
buzzer = GPIO.PWM(BUZZER_PIN, FREQUENCIA_BUZZER)

servo.start(0)
buzzer.start(0)


def executar_batida(posicao_servo):
    """
    Executa uma batida do metrônomo:
    - movimenta o servo;
    - liga o buzzer;
    - desliga o buzzer após um pulso curto.
    """

    servo.ChangeDutyCycle(posicao_servo)

    buzzer.ChangeFrequency(FREQUENCIA_BUZZER)
    buzzer.ChangeDutyCycle(50)

    time.sleep(DURACAO_SOM)

    buzzer.ChangeDutyCycle(0)

    # Mantém os pulsos do servo por um pequeno intervalo
    # para que ele alcance a posição
    time.sleep(0.12)

    # Interrompe o PWM para reduzir tremores e ruído
    servo.ChangeDutyCycle(0)


try:
    print("Metrônomo iniciado: 60 BPM")
    print("Pressione Ctrl+C para encerrar.")

    proximo_instante = time.monotonic()
    posicao_atual = SERVO_ESQUERDA
    numero_batida = 0

    while True:
        # Aguarda até o instante planejado
        agora = time.monotonic()
        tempo_restante = proximo_instante - agora

        if tempo_restante > 0:
            time.sleep(tempo_restante)

        # Mede o instante real de início
        instante_real = time.monotonic()

        # Diferença entre o instante planejado e o real
        jitter_ms = (instante_real - proximo_instante) * 1000

        numero_batida += 1

        print(
            f"Batida {numero_batida:03d} | "
            f"jitter = {jitter_ms:+.3f} ms"
        )

        executar_batida(posicao_atual)

        # Alterna a posição do servo
        if posicao_atual == SERVO_ESQUERDA:
            posicao_atual = SERVO_DIREITA
        else:
            posicao_atual = SERVO_ESQUERDA

        # O próximo instante é baseado no instante planejado,
        # não no término da execução atual
        proximo_instante += PERIODO

        # Detecta atraso maior que um período
        atraso = time.monotonic() - proximo_instante

        if atraso >= PERIODO:
            print("Aviso: execução perdeu uma ou mais batidas.")

            # Reagenda sem executar várias batidas imediatamente
            while proximo_instante <= time.monotonic():
                proximo_instante += PERIODO

except KeyboardInterrupt:
    print("\nMetrônomo interrompido pelo usuário.")

finally:
    buzzer.ChangeDutyCycle(0)
    servo.ChangeDutyCycle(0)

    buzzer.stop()
    servo.stop()

    GPIO.cleanup()

    print("GPIOs liberadas.")