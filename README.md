# Embedded-Temperature-Monitoring-System
1. Introduction
This report summarizes the summer research internship project based on the firmware
design and implementation of an automated embedded device as well as an insight into
the practical and real-time applications of digital temperature sensor devices for
extremely low sub-zero temperatures.
An embedded system is a computer system—a combination of a computer processor,
computer memory, and input/output peripheral devices—that has a dedicated function
within a larger system. It is embedded as part of a complete device often including
electrical or electronic hardware.
The embedded device design we worked on receives or fetches the voltage or resistance
sensor readings from up to eight different channels. Each of these eight channels may
have different types of sensors depending on the requirements. These readings are then
interpolated or converted to the appropriate temperature readings in the adjusted scale
as per the pre-coded lookup table. This data is displayed on the screen as the default
display that keeps refreshing after an interval of 5.0 seconds to show 4 channel sensor
readings at a time alternatively.
As a part of the User Interface (UI) of the embedded system, the user can operate the
device manually via the interfaced 4x3 keyboard and view the corresponding menu
options, data display by means of the 16x2 LCD display. Additionally, the mbed LPC1768
microcontroller can be accessed via USB as it has a built-in micro USB interface. It can also
be interfaced to provide connectivity via remote procedural calls (RPC) and Ethernet
implemented using the standard RS232 port. The device can then be controlled or
operated via the serial communication link via the PC terminal.
The data acquired by the device is continuously being logged and stored in a binary file
with the .bin extension which is automatically converted to a human-readable text
document file with the .txt extension. This log file can then be viewed on connecting the
microcontroller to a client PC via the micro USB connection of the mbed board to the PC.
Manual analysis can then be performed using the output data from the generated log file.
GitHub Link for the code: https://github.com/MukuFlash03/MBED_Temp_Meter_IUAC
2. Aim and Objectives
Fig. 1: Embedded System Components
4
The cryogenic temperature range has been defined as from −150 °C (−238 °F) to absolute
zero (−273 °C or −460 °F), the temperature at which molecular motion comes as close as
theoretically possible to ceasing completely.
Now, globally there exist many cryogenic labs and test facilities with extremely advanced
technological devices and equipment that have been working with such low temperatures.
These labs deal with low temperature substances and materials such as liquid helium and
liquid nitrogen refrigerants, superconducting materials such as niobium which need to be
handled with extreme precaution and under strict observations.
At IUAC, India’s premier research centre for nuclear accelerators, cryogenics is an
important department where the aforementioned facilities and research work is carried
out. The recent accelerator projects include the heavy ion RF-superconducting LINAC
booster for 15UD Pelletron Accelerator as well as Liquid Helium and Liquid Nitrogen Plant
facilities.
The aim of the research project was working towards the development and implementation
of an automated Digital Temperature Meter to record as well as compute temperatures
near absolute zero (0 Kelvin or -273° C) which are difficult to measure with ordinary
sensors, let alone be recorded in an organized manner, for future analysis.
Further, another reason was to emphasize on the production of locally developed products
especially in the times of the global pandemic where the entire economy and imports have
been affected. Hence, dependence on native products has been a major motivating factor
behind this project.
The objectives of the research work are as follows:
i. To interface various hardware components as per the low temperature conditions.
ii. To design flow-charts and model the temperature meter device.
iii. To develop and implement various software functionalities for the device.
iv. To ensure client connectivity and data accessibility to the user
