# control-board
Arduino code for Boxbots control board - 3CH R/C receiver input to TB6612 dual h-bridge output
This repository taken and modified from here:  https://github.com/BoxBots/control-board

### Add EnableInterrupt
Note that this Arduino code needs a library added to it called EnableInterrupt
https://www.arduinolibraries.info/libraries/enable-interrupt  
https://github.com/GreyGnome/EnableInterrupt/wiki/Usage  

I have copied version 1.1.0 to this repository so that you do not have to search for it.  However, feel free to download the latest revision at your leisure.  

The files for EnableInterrupt have been copied into the git archive and can be added to the sketch at:  
Sketch -> Include Library -> Add .Zip Library  

![alt text][AddLibrary]

## Helper Programs  
### receiver_test.ino  
The receiver_test.ino program is written to test the incoming PWM signals from the RC Receiver and print them to the serial monitor.  This way, you can check that you have your Arduino and pins setup correctly.  It will test the interrupt service routines (ISR) to make sure you're receiving acceptable data.  Then, you can move on to running the BoxBotCtrlBrd.ino program.  

### smoothing.ino  
The smoothing.ino program is written to expand upon the receiver_test.ino program and test smoothing on the PWM values read from the RC receiver.  This is to smooth out jerky motion from noise or errors may introduce into the PWM stream.  There's probably a better way to do this, but it is a start and will work better than nothing.  This is in the BoxBotCtrlBrd.ino program (not there yet remove this comment when inserted).  

### Schematic Image
![alt text][schematic]

### Board Images
![alt text][hardware1]
![alt text][hardware2]

### Controls
I connected the Nano to the Arduino IDE serial monitor and trimmed channel 1 steering and channel 2 steering soo that they both centered around 255.  

The Weapon button on channel 3 of the FS-GT2B 3 channel radio is a TOGGLE and NOT momentary.  IE press the button one time to change the state from OFF to ON.  Press the button one more time to toggle from ON to OFF.  

### Happy roboting....


### Related Links
Data Input Example:  https://forum.arduino.cc/index.php?topic=236162.0  
Steering example:  https://www.instructables.com/id/Rc-Controller-for-Better-Control-Over-Arduino-Proj/  
Decoding and mixing RC Signals:  https://forum.arduino.cc/index.php?topic=484179.0  
2D array Tutorial:  https://www.tutorialspoint.com/arduino/arduino_multi_dimensional_arrays.htm  

Smoothing Data Example:  https://www.arduino.cc/en/tutorial/smoothing

## Low Profile Headers (for future development)
https://www.adafruit.com/product/3008  
https://www.adafruit.com/product/3009  
https://www.digikey.com/product-detail/en/mill-max-manufacturing-corp/315-47-110-41-004000/ED10365-ND/8575758  

#### To Do  
fix steering/control  (add offset for output, both fwd & rev?)
Add weapon control  
Take pix of controller (skyway), receiver, motors and post to page.  
##### Someday Maybe (or not at all)  
It would be neat to write a class for the RC Radio input pins.  That way, if you had a robot with more than 3 channels, you could simply add instances of your class for each pin.  Seems like it might be a cool thing to do.  Here's an example on writing a library that could be useful in this 'nice to have:'  https://www.arduino.cc/en/Hacking/libraryTutorial


[AddLibrary]:/Images/LibraryAddImage.png "Add Library Image"
[schematic]:/Images/control_board_sch_v1.jpg "Schematic"
[hardware1]:/Images/IMG_20160724_100326.jpg "Hardware Image"
[hardware2]:/Images/IMG_20160724_100359.jpg "Hardware Image"