# Bandonino
Teensy powered midi "bandoneon", using mechanical key switches and a load-cell for bellows emulation.

It's powered, and outputs MIDI, over USB. I use my Android phone, running Audio Evolution Mobile, which does low latency MIDI conversion for real-time playing, as well as recording etc.

The bellows emulation uses a 3KG load cell, mounted using rubber grommets to allow for a little bit of movement (1cm or so).

Left/right sides emit midi on different channels so that they can be processed separately. It also outputs a third channel for a metronome.

# Goals

1. To help me learn the keyboard
2. To confirm my choice of keyboard
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
https://www.amazon.co.uk/gp/product/B07X3T14CV/ref=ppx_od_dt_b_asin_title_s00?ie=UTF8&psc=1

Waveshare 1.5inch OLED Display Module 128x128 Pixels 16-bit Grey Level<br>
https://www.amazon.co.uk/dp/B079NNZ9V1?psc=1&ref=ppx_yo2ov_dt_b_product_details

Stemedu Load Cell 3KG Amplifier HX711<br>
https://www.amazon.co.uk/dp/B07L82YWPV?psc=1&ref=ppx_yo2ov_dt_b_product_details<br>
You need to switch (with delicate soldering!) the amp to use the 80Hz mode.

1N4148 Diodes<br>
https://www.amazon.co.uk/dp/B08TYC1ZRB?psc=1&ref=ppx_yo2ov_dt_b_product_details

KY-040 360 Degree Rotary Encoder<br>
https://www.amazon.co.uk/dp/B07B68H6R8?psc=1&ref=ppx_yo2ov_dt_b_product_details


# Software

I used a 128x128 display so that the menu system could include live note display - the left/right hands display on bass/treble clefs, or can be displayed as stacked notes. This has been really useful as I learn, and I'm pleased to say is becoming less useful as I improve!

The menu system itself is not written to be a standalone system, but could easily be adapted into a different project.

The main loop runs at a solid 80Hz, keeping up with the load cell/amplifier. When using note display, updating the display as notes change can drop a frame or so, but this is inaudible.

It supports writing/reading all the settings to an SD card - they can be saved explicitly, but also the current setting is saved automatically, and then restored when powering on.

# Videos/photos

## Playing video demo

After trying to learn to play for four weeks: https://youtu.be/XA5RVsj3e9U?si=FQ6Nt5ClDMPxDO7N

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






