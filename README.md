# FLCOS Mini Projector with ESP32 and Composite input.


WARNING: Please do not build this board yet. There are still some checks to do.

TODO...

Voltage reg pinouts and ADJ pins (use adjustable and add resistor dividers, or look for fixed-output regs for the BOM?)
Enough decoupling?
FPC connector direction, top contact? (The FLCOS module will be mounted upside-down on the board)
Double-check the JST ZH pinout for the LEDs,
Maybe change the pin header to some sort of socket for the Composite input?
Check availability of all ICs on LCSC or Mouser, digikey etc.
Re-check FLCOS module measurements,
Hunt for parts, create a BOM.
Run ERC and DRC.
Order a few test PCBs.


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
I originally put some LM3404 LED driver ICs on the board, but they wouldn't have worked for a common-Anode LED.
So I changed those for the LM3414 instead.


The same module appears to be the same one used in the "JAKKS Eyclops" toy projector...
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

(IIRC, the current crystal freq on the TVP might only work for NTSC input?)


Most of the footprint libraries for the ESP32, SD slot, TVP5150, and LM3414 were taken from existing open-source projects.
I don't have the names or links to all of those projects now, but I will add them to the README if I can find them.


I used Eagle 9.0.1 to do the PCB layout.

Again, please don't build this yet until the first prototypes can be tested.
There likely WILL be some mistakes atm. This is not my best PCB layout. lol


ElectronAsh. ;)


This was a quick test of the Green LEDs. FLCOS chip itself is not powered.

Both Green LEDs in parallel, 220mA, lens was 150mm from the "screen".
The box is only 90mm wide.

![FLCOS Green LED test front view](/images/FLCOS_Green_LED_test_front_view.jpg)
![FLCOS Green LED test 150mm from screen](/images/FLCOS_Green_LED_test_150mm_from_screen.jpg)
![FLCOS Green LED test screen view](/images/FLCOS_Green_LED_test_screen_view.jpg)

![FLCOS Schematic Eagle sheet 1](/images/FLCOS_Schematic_Eagle_sheet_1.png)
![FLCOS Schematic Eagle sheet 2](/images/FLCOS_Schematic_Eagle_sheet_2.png)
![FLCOS Schematic Eagle sheet 3](/images/FLCOS_Schematic_Eagle_sheet_3.png)
![FLCOS Green LED test - front view](/images/FLCOS_Board_Eagle.png)
