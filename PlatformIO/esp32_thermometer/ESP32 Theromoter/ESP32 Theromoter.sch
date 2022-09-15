EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Sensor_Temperature:DS18B20 U?
U 1 1 5F707B4D
P 5450 3700
F 0 "U?" H 5220 3746 50  0001 R CNN
F 1 "DS18B20" H 5220 3700 50  0000 R CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 4450 3450 50  0001 C CNN
F 3 "http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf" H 5300 3950 50  0001 C CNN
	1    5450 3700
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5F7089DF
P 4650 3400
F 0 "R?" H 4720 3446 50  0001 L CNN
F 1 "4,7kOhm" H 4720 3400 50  0000 L CNN
F 2 "" V 4580 3400 50  0001 C CNN
F 3 "~" H 4650 3400 50  0001 C CNN
	1    4650 3400
	1    0    0    -1  
$EndComp
Text GLabel 4200 3700 0    50   Input ~ 0
ESP32_GPIO_13
Text GLabel 4200 3100 0    50   Input ~ 0
ESP32_GPIO_12
Text GLabel 4200 4300 0    50   Input ~ 0
GND
Wire Wire Line
	5450 4300 5450 4000
Wire Wire Line
	5450 3400 5450 3100
Wire Wire Line
	5450 3100 4650 3100
Wire Wire Line
	4650 3250 4650 3100
Wire Wire Line
	4650 3100 4200 3100
Connection ~ 4650 3100
Wire Wire Line
	4200 3700 4650 3700
Wire Wire Line
	4650 3550 4650 3700
Connection ~ 4650 3700
Wire Wire Line
	4650 3700 5150 3700
Wire Wire Line
	4200 4300 5450 4300
$EndSCHEMATC
