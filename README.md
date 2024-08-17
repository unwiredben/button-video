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
