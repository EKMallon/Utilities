<img src="https://github.com/EKMallon/The_Cave_Pearl_Project_CURRENT_codebuilds/blob/master/images/CavePearlProjectBanner_130x850px.jpg">

This repository contains code to support the addition of the SSD1306 OLED screen on the "Modules &amp; Jumper Wires" data logger described in 

**Cave Pearl Data Logger: A Flexible Arduino-Based Logging Platform for Long-Term Monitoring in Harsh Environments**
Sensors 2018, 18(2), 530; http://www.mdpi.com/1424-8220/18/2/530  (open access - free PDF download)  and on the project's blog at  https://thecavepearlproject.org/ 

The connection plan described in this tutorial: http://thecavepearlproject.org/2018/10/17/tutorial-adding-a-ssd1306-oled-screen-to-your-arduino-logger/  makes use shiftout to re-purpose unused analog lines to both power & drive the screen. The primary advantage is that the screen draws zero current when not in use. This is a mission critical factor for long term applications like dataloggers. Another significant adavantage is that this leaves the hardware SPI lines free for more important tasks such as saving data to the SD cards. 


<img src="https://github.com/EKMallon/Utilities/blob/master/Utility_10_SSD1306_128x64_OLED/images/SSD1306_OLED_wArduino_Pt10_TestAfterExpoxy.jpg">
