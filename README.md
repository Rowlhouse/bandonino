# Bandonino
Teensy powered midi "bandoneon", using mechanical key switches and a load-cell for bellows emulation.

It's powered, and outputs MIDI, over USB. I use my Android phone, running Audio Evolution Mobile, which does low latency MIDI conversion for real-time playing, as well as recording etc.

The bellows emulation uses a 3KG load cell, mounted using rubber grommets to allow for a little bit of movement (1cm or so).

Left/right sides emit midi on different channels so that they can be processed separately. It also outputs a third channel for a metronome.

# Goals

1. To help me learn the keyboard layout
2. To confirm my choice of layout (unisonoric - Peguri/Manoury layout)
3. To provide a learning experience that is at least somewhat "musically satisfying", in order to help me wait for the real thing to arrive.
4. To be a genuine, expressive, musical instrument, that goes beyond "press button, make sound", that might be interesting to use in situations where a real bandoneon isn't appropriate.

# Building 

More complete notes will follow, but for now, the layouts for the key switches are in the Construction directory.

The wiring is "inspired" by the KOOPInstruments projects - e.g. https://www.koopinstruments.com/instrument-projects/melodicade-mx so it uses a matrix (per side) of rows and columns of switches and diodes. Columns are activated in turn, and the buttons from each row are then read.

Most of the build was "improvised" (with 3mm and 6mm plywood sheets), once I'd designed the keyboard layout. The photos below document most of the process.

I was aiming for a "full size" instrument, though with angled ends so it didn't need to be as wide. I also wanted it to have some weight, so it would "open" the bellows to some degree automatically. It could be built much smaller and lighter.

The switches are mounted on plywood, with holes drilled in to (a) position them and (b) let the wires through. Then the matrix is made on the other side by soldering in a million diodes and little wire joiners.

The buttons are 3D printed, with graduated heights. Each button has a little flange. The buttons pass through holes in 2mm fibreglass plate (make the holes 1mm bigger than the buttons!). Now the plate has its height set so that it partially depresses the buttons. This reduces the travel needed to activate each one. 

## Note

If building again I would make two changes:

1. Reduce the button diameter by 0.5mm. This would have two benefits:
    1. increase the spacing between the holes in the plates (I used 2mm fibreglass), and reduce the risk of them breaking into each other.
    2. Make it possible to drill the holes in the plates with a standard size drill bit. 
2. Get a definitive layout from a friendly maker! I used photos to be as accurate as possible, but I think a few buttons are a few mm out.

# Electronics

The main items used are (the links aren't endorsements - just what I used/for reference!)

Teensy 4.1<br>
https://www.amazon.co.uk/dp/B0BHFBY68Q?psc=1&ref=ppx_yo2ov_dt_b_product_details

Gateron KS-9 RGB Mechanical MX Type Key Switch - Clear top (120 Pcs, Red)<br>
https://www.amazon.co.uk/gp/product/B07X3T14CV/ref=ppx_od_dt_b_asin_title_s00?ie=UTF8&psc=1<br>
Note that these need to have the 1mm plastic lugs removed from the sides as they have to be placed very close to each other!

Waveshare 1.5inch OLED Display Module 128x128 Pixels 16-bit Grey Level<br>
https://www.amazon.co.uk/dp/B079NNZ9V1?psc=1&ref=ppx_yo2ov_dt_b_product_details

Stemedu Load Cell 3KG Amplifier HX711<br>
https://www.amazon.co.uk/dp/B07L82YWPV?psc=1&ref=ppx_yo2ov_dt_b_product_details<br>
You need to switch (with delicate soldering!) the amp to use the 80Hz mode. I broke mine so used this load cell with a different HX711 amplifier:
https://www.amazon.co.uk/dp/B081JMSMXV

1N4148 Diodes<br>
https://www.amazon.co.uk/dp/B08TYC1ZRB?psc=1&ref=ppx_yo2ov_dt_b_product_details

KY-040 360 Degree Rotary Encoder<br>
https://www.amazon.co.uk/dp/B07B68H6R8?psc=1&ref=ppx_yo2ov_dt_b_product_details

# Inputs

| Input | Teensy Pin | Notes |
| ----- | ---------- | ----- |
| Left col 0 | 0 | |
| Left col 1 | 2 | |
| Left col 2 | 3 | |
| Left col 3 | 4 | |
| Left col 4 | 5 | |
| |  |  |
| Right col 0 | 6 | |
| Right col 1 | 7 | |
| Right col 2 | 8 | |
| Right col 3 | 9 | |
| Right col 4 | 10 | |
| Right col 6 | 11 | |
| |  |  |
| Left row 0 | 32 | |
| Left row 1 | 31 | |
| Left row 2 | 30 | |
| Left row 3 | 29 | |
| Left row 4 | 28 | |
| Left row 5 | 27 | |
| Left row 6 | 26 | |
| Left row 7 | 25 | |
| |  |  |
| Right row 0 | 33 | |
| Right row 1 | 34 | |
| Right row 2 | 35 | |
| Right row 3 | 36 | |
| Right row 4 | 37 | |
| Right row 5 | 38 | |
| Right row 6 | 39 | |
| Right row 7 | 40 | |
| Right row 8 | 41 | |
| |  |  |
| Load cell DT or DO/RX | 15 (A1) | From the strain guage to HX711 |
| Load cell SCK or CK/TX | 16 (A2) | red->out+, white->A- green->A+ black->GND |
| Load cell VCC | 5V | 5V seemed more reliable than 3.3 |
| |  |  |
| Rot encoder CLK | 22 | |
| Rot encoder DT | 23 (A9) | |
| Rot encoder SW | 17 | |
| Rot encoder + | 3.3V | |
| |  |  |
| OLED I2C CLK | 19 (SCL) | Set jumpers BS1 = BS2 = 1 to enable I2C |
| OLED I2C DIN | 18 (SDA) | |
| OLED I2C VCC | 3.3V | |





# Software

I used a 128x128 display so that the menu system could include live note display - the left/right hands display on bass/treble clefs, or can be displayed as stacked notes. This has been really useful as I learn, and I'm pleased to say is becoming less useful as I improve!

The menu system itself is not written to be a standalone system, but could easily be adapted into a different project.

The main loop runs at a solid 80Hz, keeping up with the load cell/amplifier. When using note display, updating the display as notes change can drop a frame or so, but this is inaudible.

It supports writing/reading all the settings to an SD card - they can be saved explicitly, but also the current setting is saved automatically, and then restored when powering on.

# Libraries/building

I used/you'll need:

* https://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx
* https://github.com/bogde/HX711
* https://github.com/adafruit/Adafruit_SSD1327 v 1.0.4
* https://arduinojson.org/

I think all of these are available through Aruino Sketch. The full list of libraries is:

    RotaryEncoder at version 1.5.3 : [...]\Documents\Arduino\libraries\RotaryEncoder
    HX711 Arduino Library at version 0.7.5 : [...]\Documents\Arduino\libraries\HX711_Arduino_Library
    Adafruit SSD1327 at version 1.0.4 : [...]\Documents\Arduino\libraries\Adafruit_SSD1327
    Adafruit GFX Library at version 1.11.9 : [...]\Documents\Arduino\libraries\Adafruit_GFX_Library
    Adafruit BusIO at version 1.15.0 : [...]\Documents\Arduino\libraries\Adafruit_BusIO
    Wire at version 1.0 : [...]\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.59.0\libraries\Wire
    SPI at version 1.0 : [...]\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.59.0\libraries\SPI
    ArduinoJson at version 7.0.4 : [...]\Documents\Arduino\libraries\ArduinoJson
    SD at version 2.0.0 : [...]\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.59.0\libraries\SD
    SdFat at version 2.1.2 : [...]\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.59.0\libraries\SdFat

You'll need to select the board as Teensy 4.1 in Arduino Sketch, and USB type is Serial + MIDI

# Videos/photos

## Playing video demo

After learning to play for four weeks: https://youtu.be/XA5RVsj3e9U?si=FQ6Nt5ClDMPxDO7N

The audio is recorded on my phone as midi, and mixed there (so this is exactly what I hear if I play with headphones).

I've decided that the bellows emulation is reasonably effective (or at least, not totally misleading!) when "opening" the bellow - using instrument/arm weight, and modulating these without actively pulling. 

I don't think it emulates "closing" the bellows very well, so I've settled on only playing opening the bellows. This isn't ideal - but I think for proper bandoneon bellows technique, real bellows are needed!

## System demos

Newer:
https://youtu.be/Qa_-vbLSPUA?si=eyVYrDUC1PoKSc3D

Older:
https://youtu.be/M0wQ6heuBrk?si=m23eztmR4TV7uDbV

## Build photos etc

https://photos.app.goo.gl/t3dRVAY1abLQWBCG9

## Smaller set of photos/videos

https://photos.app.goo.gl/e3H986FrXZXdoZyL7






