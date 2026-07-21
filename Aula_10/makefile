CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

CPPFLAGS = -I/usr/local/include
LDFLAGS = -L/usr/local/lib

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

teste_lcd: teste_lcd.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) teste_lcd.cpp \
	-o teste_lcd \
	$(LDFLAGS) \
	$(WIRINGPI) $(WIRINGPI_DEV)

teste_teclado: teste_teclado.cpp $(KEYPAD_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) teste_teclado.cpp \
	$(KEYPAD_FILES) \
	-o teste_teclado \
	$(LDFLAGS) \
	$(WIRINGPI) $(WIRINGPI_DEV)

teste_teclado_lcd: teste_teclado_lcd.cpp $(KEYPAD_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) teste_teclado_lcd.cpp \
	$(KEYPAD_FILES) \
	-o teste_teclado_lcd \
	$(LDFLAGS) \
	$(WIRINGPI) $(WIRINGPI_DEV)

teste_buzzer: teste_buzzer.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) teste_buzzer.cpp \
	-o teste_buzzer \
	$(LDFLAGS) \
	$(WIRINGPI)

teste_sensor: teste_sensor.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) teste_sensor.cpp \
	-o teste_sensor \
	$(LDFLAGS) \
	$(WIRINGPI)

teste_atuador: teste_atuador.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) teste_atuador.cpp \
	-o teste_atuador \
	$(LDFLAGS) \
	$(WIRINGPI)

teste_senha: teste_senha.cpp $(KEYPAD_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) teste_senha.cpp \
	$(KEYPAD_FILES) \
	-o teste_senha \
	$(LDFLAGS) \
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

.PHONY: all clean