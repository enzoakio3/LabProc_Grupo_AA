import RPi.GPIO as GPIO
import time

LED_PIN = 17

# Utiliza a numeração BCM: GPIO17
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)

# Frequência inicial
pwm = GPIO.PWM(LED_PIN, 1)

try:
    # Inicia com duty cycle de 50%
    pwm.start(50)

    frequencias = [1, 5, 10, 50, 100, 500, 1000]

    for frequencia in frequencias:
        print(f"Testando frequência de {frequencia} Hz")
        pwm.ChangeFrequency(frequencia)

        # Mantém cada frequência durante 5 segundos
        time.sleep(5)

finally:
    pwm.stop()
    GPIO.output(LED_PIN, GPIO.LOW)
    GPIO.cleanup()
    print("Teste finalizado.")