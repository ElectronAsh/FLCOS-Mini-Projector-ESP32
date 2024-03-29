# FLCOS Mini Projector with ESP32 and Composite input.


WARNING: Please do not build this board yet. There are still some checks to do.

UPDATE: 29th April 2023: I finally bought one of the Jakks toy projectors from eBay US.
I hooked up the USB Logic Analyzer to the "SPI" bus on the projector's FLCOS module.
The projector only ever write to three registers after power-up, then another register to enable the FLCOS after a video signal is detected.

So I did a few tweaks to my Arduino code, and now my own board works, too. ;)

I also confirmed that the AliExpress FLCOS modules DO indeed work fine in the Jakks projector, so they are the same/compatible part.

There are still a few issues to sort out, where the FLCOS won't always turn on when you first plug USB power in, but it will work after pulling
the reset (EN_N) Low on the ESP32, or re-flashing via the Arduino IDE.

I've included a few photos below of the projector showing "Gex" on the 3DO.
This is a PAL 3DO, so I tweaked the Arduino code to fit the projected image better.
(the FLCOS registers will default to NTSC, so I tweaked the values for PAL.)


TODO...

Voltage reg pinouts and ADJ pins (use adjustable and add resistor dividers, or look for fixed-output regs for the BOM?)
FPC connector direction, top contact? (The FLCOS module will be mounted upside-down on the board)
Double-check the JST ZH pinout for the LEDs. DONE
Maybe change the pin header to some sort of socket for the Composite input? DONE
Check availability of all ICs on LCSC or Mouser, digikey etc. DONE
Hunt for parts, create a BOM. DONE
Run ERC and DRC. DONE
Order a few test PCBs. DONE


This is a driver board for the FLCOS mini projector modules on AliExpress...
https://www.aliexpress.com/item/1005003386129982.html


There is a forum post here, with the full datasheet for what appears to be the same FLCOS chip used on the AliExpress module.
https://whycan.com/t_2420.html

("datasheet" is actually the product brief. "cps" has all of the register settings.)

The pinout of the lower flex cable on post #4 of the forum seems to match up with the ribbon on the FLCOS module.


The pinout of the RGB LED cable on the module is a bit strange.
There is no correlation between the wire colours, the silkscreen designators, and the RGB LED chip itself.

I tested the RGB LED with a bench PSU, though, and the pinout on the Eagle schematic *should* be correct now.
I just need to double-check that the JST ZH connector for the LED is not rotated 180-degrees or something.

(it's possible the manufacturer changed the RGB LED for a different one on request, which is why the LED cable pinout doesn't seem to tally with the silkscreen?)

The exact maximum LED current is unknown.
I tested one of the Green LEDs with 160mA at first, and it didn't seem to increase in brightness much above 200mA.

The forward voltage drop of the Blue and Green LEDs was around 3.3 Volts, according to my bench PSU (when current-limiting). (there are two Green LEDs on the die) 

The forward voltage drop of the Red LED was around 2.0 Volts.


The included RGB LED has all of Anodes connected together.
I originally put some LM3404 LED driver ICs on the board, but they wouldn't have worked for a common-Anode RGB LED chip, so I changed those for the LM3414 instead.


The FLCOS module appears to be the same one used in the "JAKKS Eyeclops" toy projector...
https://videotechnology.blogspot.com/2010/11/inside-jakks-pacific-eyeclops-mini.html

Before I received some modules, I did a lot of tracing of the JAKKS PCB to try to confirm the pinout against the TVP5150 TV decoder chip.
The pinout seemed to match up with the forum post above.


There was a discussion on Twitter in 2021 about these modules, too...
https://twitter.com/AshEvans81/status/1498184526236987392?s=20&t=zDXLWlJON1HiXY-NYlLocg


The PCB layout was also discussed on the #lab channel of bitluni's Discord server.

bitluni's awesome ESP32 VGA output project was the reason I wanted to add an ESP32 to the board.
It could make for a neat mini movie player or similar (streaming low-res files from SD card.)

https://github.com/bitluni/ESP32Lib/blob/8f4b1bc9c331546c57c1ee837ac37303b8d8739c/examples/VGADemo14Bit/VGADemo14Bit.ino


(bitluni's code is untested with the FLCOS modules atm, but I think it will work OK with a few tweaks. The FLCOS chip accepts video input as sequential R,G,B bytes.)


I kept the TVP5150 chip on the board, as this is the same chip used in the JAKKS projector.
It outputs standard bt.656 style digital video, and accepts Composite input.

(The TVP5150 can receive both NTSC and PAL Composite, using one 14.318 MHz crystal.)

Only the TVP5150 or ESP32 can drive the FLCOS module at any one time.
The TVP chip can set it's output pins to High-Z (this is done via an I2C register), so it should be possible to keep both the ESP32 and TVP5150 onboard.


Most of the footprint libraries for the ESP32, SD slot, TVP5150, and LM3414 were taken from existing open-source projects.
I don't have the names or links to all of those projects now, but I will add them to the README if I can find them.


I used Eagle 9.0.1 to do the PCB layout.


Build at your own risk!

It does finally "work" now, in terms of getting Composite video to display.
But there are a few issues, as mentioned in the update at the top of this README.

I haven't tried generating video on the ESP32 yet, so I'm not 100% sure that will work without PCB changes.


ElectronAsh. ;)


![FLCOS First Light - GEX Logo](/images/FLCOS_Built.jpg)
![FLCOS First Light - GEX Logo](/images/FLCOS_Built_2.jpg)
![FLCOS First Light - GEX Logo](/images/FLCOS_Working_Gex_Logo.jpg)


This was a quick test of the Green LEDs. FLCOS chip itself is not powered.

Both Green LEDs in parallel, 220mA, lens was 150mm from the "screen".
The box is only 90mm wide.

![FLCOS Green LED test front view](/images/FLCOS_Green_LED_test_front_view.jpg)
![FLCOS Green LED test 150mm from screen](/images/FLCOS_Green_LED_test_150mm_from_screen.jpg)
![FLCOS Green LED test screen view](/images/FLCOS_Green_LED_test_screen_view.jpg)

![FLCOS Module Disassembled](/images/FLCOS_Disassembled.jpg)

![FLCOS Schematic Eagle sheet 1](/images/FLCOS_Schematic_Eagle_sheet_1.png)
![FLCOS Schematic Eagle sheet 2](/images/FLCOS_Schematic_Eagle_sheet_2.png)
![FLCOS Schematic Eagle sheet 3](/images/FLCOS_Schematic_Eagle_sheet_3.png)
![FLCOS Green LED test - front view](/images/FLCOS_Board_Eagle.png)
