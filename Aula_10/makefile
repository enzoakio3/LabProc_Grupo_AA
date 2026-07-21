CXX = g++
CXXFLAGS = -Wall -Wextra
WIRINGPI = -lwiringPi
WIRINGPI_DEV = -lwiringPiDev

KEYPAD_FILES = Keypad.cpp Key.cpp

all: \
	teste_lcd \
	teste_teclado \
	teste_teclado_lcd \
	teste_buzzer \
	teste_sensor \
	teste_atuador \
	teste_senha

teste_lcd:
	$(CXX) $(CXXFLAGS) teste_lcd.cpp \
	-o teste_lcd \
	$(WIRINGPI) $(WIRINGPI_DEV)

teste_teclado:
	$(CXX) $(CXXFLAGS) teste_teclado.cpp \
	$(KEYPAD_FILES) \
	-o teste_teclado \
	$(WIRINGPI)

teste_teclado_lcd:
	$(CXX) $(CXXFLAGS) teste_teclado_lcd.cpp \
	$(KEYPAD_FILES) \
	-o teste_teclado_lcd \
	$(WIRINGPI) $(WIRINGPI_DEV)

teste_buzzer:
	$(CXX) $(CXXFLAGS) teste_buzzer.cpp \
	-o teste_buzzer \
	$(WIRINGPI)

teste_sensor:
	$(CXX) $(CXXFLAGS) teste_sensor.cpp \
	-o teste_sensor \
	$(WIRINGPI)

teste_atuador:
	$(CXX) $(CXXFLAGS) teste_atuador.cpp \
	-o teste_atuador \
	$(WIRINGPI)

teste_senha:
	$(CXX) $(CXXFLAGS) teste_senha.cpp \
	$(KEYPAD_FILES) \
	-o teste_senha \
	$(WIRINGPI)

clean:
	rm -f \
	teste_lcd \
	teste_teclado \
	teste_teclado_lcd \
	teste_buzzer \
	teste_sensor \
	teste_atuador \
	teste_senha