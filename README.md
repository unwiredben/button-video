# button-video

This is a simplified version of my SuperCon 2023 badge hack, but rebuilt for the
WaveShare 0.99" TFT round screen with RP2040.  Details on the hardware
platform are at https://www.waveshare.com/wiki/RP2040-LCD-0.99-B

This screen is much smaller at 128x115, although it acts like a 128x128
display.  The display needs a 180-degree rotation applied to play the
video in the correct format.

I was able to get 1 minute of video into about 1.6MB, so was able to use
a slightly cut-down version of the "Star Trek: Strange New Worlds" intro
for the demo video.  Since the frame size is smaller, there was no need
to reduce the color resolution, and I left the video as full color.

# Hardware

From looking at the schematic, the pinout for the side connector (along with
the color of the breakout cable wire) is

* GND - gray
* 3V3 - blue
* GPIO26 (SCA1) - orange
* GPIO27 (SCL1) - red
* GPIO28 - yellow
* GPIO29 - green

If you're connecting this as a SAO, you want to have this pinout

```
+-----      --------+
| 3V3   SDA   GPIO1 |
| GND   SCL   GPIO2 |
+-------------------+
```
so my suggestion is GND to GND, 3V3 to 3V3, GPIO26 to SDA, GPIO27 to SCL,
GPIO28 to GPIO1, and GPIO29 to GPIO2.

# Performance

As currently configured, this program runs the RP2040 at 275Mhz, a bit over
twice it's standard speed. If you're using this for a SAO, you should reduce
the clock rate to 125Mhz. In my testing, that resulted in a frame rate under
30fps, but allowed the unit to work at 2.96V and 40mA.  At the fast clock
speed, I wasn't able to reliably run the unit below 3.1V and 70mA.

# Links

* https://github.com/unwired/vector-video - my hack from SuperCon
* https://github.com/phoboslab/pl_mpeg - the awesome single file MPEG-1 video decoder that I used
* https://github.com/Bodmer/TFT_eSPI/ - the display library I used.
  I should make a new definition file for this specific board, since I had
  to define the MISO/RST/CLK/BL/RST pins from the schematic.
* https://github.com/earlephilhower/arduino-pico - a better Pico SDK than
  the default mBed one
* https://platformio.org/ - the VSCode IDE extension that made building C++
  code for the badge relatively easy

# FFMPEG conversion examples

```
ffmpeg -ss 00:00:30 -i STSNW.mp4 -vf "crop=800:800" -an -t 00:01:00 -s 128x128 -vcodec mpeg1video ST_SNW_Intro.mpg
```

# FFMPEG to C include

I'm doing this manually using the command

```
xxd -i video.mpg | sed -e "s/unsigned/const unsigned/" > MPEG1Video.h
```

The naming of the data in the header file depends on the input filename, so you
may want an addition sed stage to convert to a generic name to avoid needing to
change your player source.
