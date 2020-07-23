# Extractor de Miel con control de velocidad
## Hecho par el extractor de Miel de Fernando
Versión del estractor de mile de Julio de 2020
control de tiempo, sentido de giro y velocidad


NodeMCU      Nokia NK5110 LCD
 * D4 ----------> CLK
 * D3 ----------> Din
 * D2 ----------> DC
 * D1 ----------> CE
 * D0 ----------> RST
 * 
 * 
 * 
 *  PIN   ARDUINO 
 *  RST   Reset 3 
 *  CE       Chip Enable 4 
 *  DC       Data/Command  5 
 *  DIN      SPI IN  6 
 *  CLK      SPI Clock 7 
 *  VCC      3,3V  3.3V  Cuidado aquí, son 3,3 y no 5V
 *  BackLit  GND = Máximo brillo
 *  GND       Ground  GND 
 * 
 
 * 
 * D5 -----> prog.giro Derecha
 * D6 -----> prog.giro Izquierda
 * D7 -----> Botón
 * D8 -----> PWM output Referencia de frecuencia para giro 
 
 # se usa un módulo [PWM 0-10V]
 (http://www.icstation.com/voltage-converter-module-adjustable-converter-power-module-digital-analog-signal-p-12498.html)
 modulo convertidor de PWM (salida D8) a una tensión de 0-10V para control del variador de velocidad
 
 @octocat :+1: This PRprogram looks great - it's ready to merge! :shipit:
 
![Preview] (https://github.com/miguelalonso/ExtractorMiel/blob/master/ExtractorMiel_AP_Frecuencia/20200723_123219.jpg)
 
