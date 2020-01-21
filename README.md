# control-board
Arduino code for Boxbots control board - 3CH R/C receiver input to TB6612 dual h-bridge output

### Add EnableInterrupt
Note that this Arduino code needs a library added to it called EnableInterrupt
https://www.arduinolibraries.info/libraries/enable-interrupt
I have copied version 1.1.0 to this repository so that you do not have to search for it.  However, feel free to download the latest revision at your leisure.  

The files for EnableInterrupt have been copied into the git archive and can be added to the sketch at:  
Sketch -> Include Library -> Add .Zip Library  

![alt text][AddLibrary]

### Schematic Image
![alt text][schematic]

### Board Images
![alt text][hardware1]
![alt text][hardware2]

### Happy roboting....

[AddLibrary]:/Images/LibraryAddImage.png "Add Library Image"
[schematic]:/Images/control_board_sch_v1.jpg "Schematic"
[hardware1]:/Images/IMG_20160724_100326.jpg "Hardware Image"
[hardware2]:/Images/IMG_20160724_100359.jpg "Hardware Image"