# The Spider-Crab and MAST-CAM
A rover (rough "perserverance-replica") equipped with rocker bogie (with a differential bar) suspension, capable of object detection + avoidance (with the RPi AI camera/US-100 sensors), remote control, and autonomous travel (with an onboard GPS). Modeled around the STS-3215 servo. (Special thanks here to Logan Wheatcroft for helping me spell "attach"). __THERE IS NO PCB ON THIS BUILD. IT IS SOLDERLESS.__

## Table of Contents
- [The Spider-Crab and MAST-CAM](#the-spider-crab-and-mast-cam)
- [Table of Contents](#table-of-contents)
- [Hack-Club Required Q&A](#hack-club-required-qa)
- [Full Assembly Screenshots](#full-assembly-screenshots)
- [BOM](#bom)
- [Printing Instructions](#printing-instructions)
- [Wiring Instructions](#wiring-instructions)
- [Setting Up The Pi](#setting-up-the-pi)

## Hack-Club Required Q&A
So, let's see:
1. What is this? This is designed to be a hyper-mobile rover that is capable of autonomous driving, pathfinding (because it is combined with a GPS), and providing camera footage. I hope that I will be able to modify it for usage out of one of two scenarios: 1. hopefully rescue missions on debris/unstable territory (since the rover is pretty lightweight, weighing in at just about 4.5 kgs, and is theoretically capable of some crazy feats as long as the net tractive force is stable), or 2. conducting BAER assessment (providing photos to researchers about theoretically dangerous territories subject to wildfires). If the rover lacks mobility in practice, I will probably move towards the latter concept, in which case I have plans of adding six "sample"-collecting add-ons to be attached to each wheel.
2. Why did I build this? Well, I've always been interested in Aerospace, and I've always thought the Perserverance rover was magical. I'm part of a drone team, and since we just built a drone that was not much unlike Ingenuity, I wondered if I could "complete" the duo here on Earth entirely from my own vision. This rover is the result of that work. I've gone through four separate unique versions of this rover (and even more "microversions" where I had to redesign parts inside each version to keep up with out-of-stock products and so on), but the hope is that this final rover will be just as mobile as the rover on Mars, MUCH faster (hopefully going at a mile an hour), WAY less power efficient (but to be fair, I don't exactly have the greatest parts at my disposal), and a final cost JUST under $1000 (so it's affordable), and accessible (meaning it is COMPLETELY idiot-proof -- I should be a good test subject for this, since I have not dealt with PCB design in the LEAST --, and has NO soldering/pcbs involved).

## Full Assembly Screenshots
Okay, before we get started, let's get a few hero renders out of the way.
<img width="1160" height="734" alt="image" src="https://github.com/user-attachments/assets/6676c012-a744-4464-b192-14bec569b187" />
<img width="1209" height="728" alt="image" src="https://github.com/user-attachments/assets/54e44a33-d582-49d8-9911-47fb82f72d3f" />
<img width="827" height="618" alt="image" src="https://github.com/user-attachments/assets/5dba669b-d5bb-4bf9-8e4a-0668c4139220" />
<img width="779" height="692" alt="image" src="https://github.com/user-attachments/assets/4282b201-71b7-4e5f-95c7-68f8b08242f5" />
<img width="900" height="641" alt="image" src="https://github.com/user-attachments/assets/5e02f464-8cb5-4562-817a-8b53bcaf70d9" />
<img width="811" height="653" alt="image" src="https://github.com/user-attachments/assets/907d392b-3f84-469b-9ffb-63fdbcba52d7" />

Eventually, when I build this, I should be able to attach a few "normal" photos of how it'll look.

## BOM
Find the BOM above in the attached files.
Or, alternatively, here it is below:

| Quantity | Item | Description | Manufacturer | Link | Total Cost |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 24 | JST PH 2mm 3-Pin to Male | Allows for power-injection to the Feetech servos. | Adafruit | https://www.adafruit.com/product/3893 |  $ 144.95  |
| 3 | US-100 Ultrasonic Sensor | Allows the rover to navigate terrain/avoid objects. |  | https://www.adafruit.com/product/4019 |  |
| 1 | Raspberry Pi Camera Cable, 500mm | Allows the camera module to freely rotate across its axes of freedom. |  | https://www.adafruit.com/product/5820 |  |
| 1 | Raspberry Pi AI Camera | Allows the rover to have object detection/recognition capabilities. |  | https://www.adafruit.com/product/6009 |  |
| 1 | Raspberry Pi 5, 8GB | The "brains" of the rover. | AAAWave | https://aaawave.com/products/raspberry-pi-5-development-broad?variant=51441087152449 |  $ 161.63  |
| 1 | Waveshare Serial Bus Driver | Allows for the Pi to communicate with the serial bus servos. | eBay | https://www.ebay.com/itm/206185697206?mkevt=1&mkpid=0&emsid=e11412.m144671.l197929&mkcid=7&ch=osgood&euid=6d034ae6c7bd442a96e4e9cbff91307c&bu=45719063109&exe=0&ext=0&osub=-1%7E1&crd=20260803165232&segname=11412 |  $ 12.23  |
| 6 | Feetech STS3215 | Allows for the rover to move (both its wheels and the camera). | Aliexpress | https://www.aliexpress.us/item/3256808670629323.html?spm=a2g0o.order_list.order_list_main.41.4d5c18021PISjv&gatewayAdapt=glo2usa |  $ 404.07  |
| 1 | Soldering Iron, HS02A | Necessary to attach the heat-set inserts, tool. |  | https://www.aliexpress.us/item/3256809503553320.html?spm=a2g0o.order_list.order_list_main.47.4d5c18021PISjv&gatewayAdapt=glo2usa |  |
| 1 | Hot Air Gun | Necessary to secure the shrink tubing to properly insulate wires. |  | https://www.aliexpress.us/item/3256810350606440.html?spm=a2g0o.order_list.order_list_main.53.4d5c18021PISjv&gatewayAdapt=glo2usa |  |
| 1 | Heat Shrink Tubing Kit | Necessary to electrically insulate certain wires. |  | https://www.aliexpress.us/item/3256807959988149.html?spm=a2g0o.order_list.order_list_main.35.4d5c18021PISjv&gatewayAdapt=glo2usa |  |
| 1 | WAGO 221-415 Lever Nuts | Necessary to proper distribute power as desired. |  | https://www.aliexpress.us/item/3256811952946871.html?spm=a2g0o.order_list.order_list_main.29.4d5c18021PISjv&gatewayAdapt=glo2usa |  |
| 1 | Crimpling "Plier" | Necessary to properly attach the solderless lugs/terminals. |  | https://www.aliexpress.us/item/2251832197019276.html?spm=a2g0o.order_list.order_list_main.17.4d5c18021PISjv&gatewayAdapt=glo2usa |  |
| 1 | USB-C to Screw Terminals | Necessary to provide the RPi power through bare wires. |  | https://www.aliexpress.us/item/3256812221028465.html?spm=a2g0o.order_list.order_list_main.11.4d5c18021PISjv&gatewayAdapt=glo2usa |  |
| 2 | 1M, 12AWG Wire | Facilitates an electrical connection across parts. |  | https://www.aliexpress.us/item/3256808222999562.html?spm=a2g0o.order_list.order_list_main.5.4d5c18021PISjv&gatewayAdapt=glo2usa |  |
| 1 | RPi 5, Active Cooler | Allows for smooth Pi operation, prevents overheating. | Amazon | https://www.amazon.com/dp/B0CW164TCW?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0 |  $ 162.99  |
| 1 | 12V, 30A Daiertek Switch | A manual power switch for the rover. |  | https://www.amazon.com/dp/B0FL6B75PM?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_0 |  |
| 1 | 20A ANL Fuse (and Holder) | Protects electronics downstream. |  | https://www.amazon.com/dp/B01LXQWZ7L?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_1 |  |
| 1 | VK-162 GPS | Allows the Raspberry Pi to navigate via GPS. |  | https://www.amazon.com/dp/B078Y52FGQ?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_2 |  |
| 1 | Rod Ends (Small) | Allow for the rocker-bogie suspension to function. |  | https://www.amazon.com/dp/B0828TMYLD?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_0 |  |
| 1 | 120PCS, M3x5.7 Heat Set Inserts | Allow for the printed parts to be fastened together. |  | https://www.amazon.com/dp/B0D7M4WWBW?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_1 |  |
| 1 | 2300 PCS, M3 Screws Kit | Necessary hardware for the assembly to be possible. |  | https://www.amazon.com/dp/B0FGX859K8?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_2 |  |
| 1 | Flanged Ball Bearing | Necessary hardware for the rocker-bogie to be possible. |  | https://www.amazon.com/dp/B0G1B7H48R?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_3 |  |
| 1 | Breadboard Jumper Wires | Allow for power-injection to the daisy-chained servos. |  | https://www.amazon.com/dp/B07GD2PGY4?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_4 |  |
| 2 | Adjustable DC Buck Converter | Allow for the 12V battery to transfer power downstream. |  | https://www.amazon.com/dp/B099S2VQ2Q?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0 |  $ 129.46  |
| 1 | PTFE/Plumber's Tape | Facilitate the physical "bushing" inside the wheels. |  | https://www.amazon.com/dp/B091913Z7F?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_0 |  |
| 1 | 12V, 15Ah LiFePO4 | Powers the rover (hopefully for 2+ hours). |  | https://www.amazon.com/dp/B0CSWMF35Q?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_1 |  |
| 1 | Spade Terminal Connectors | Allow for power to be transmitted from the battery downstream solderlessly. |  | https://www.amazon.com/dp/B0B4H54KPS?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_0 |  |
| 1 | Digital Multimeter | Checking if the electronic components are viable. |  | https://www.amazon.com/dp/B08CX9W7G3?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0 |  $ 38.43  |
| 1 | Ring Lugs Set | Allow for wires to be crimped onto the ANL fuse. |  | https://www.amazon.com/dp/B0F99P7SGT?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_0 |  |
| 1 | 12V LiFePO4 Charger | Charges the battery. |  | https://www.amazon.com/dp/B0G3ZNWC4J?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_1 |  |
| 1 | M4x10mm Screws | Necessary hardware for the assembly to be possible. |  | https://www.amazon.com/dp/B0FH2T8PD1?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0 |  $ 4.30  |
| 1 | ASA Filament, 2kgs (1.2ish needed) | Filament for prints. |  | https://www.amazon.com/dp/B0G2L8HKB8?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0 |  $ 37.87  |
| 1 | TPU 95A Filament, 1g (0.8ish needed) | Filament for prints. |  | https://www.amazon.com/dp/B07VDP2S3P?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0 |  $ 25.97  |
| Total: |  |  |  |  |  $ 1,121.90  |

Recommended additional common supplies (double-check you have these):
1. An official raspberry pi 5 power supply (27W).
2. A 12V power supply (preferably 2-3A or less) with a DC5521 power jack.
3. USB to USB-C cable (or a USB to a USB-C female port, and then a USB-C to a USB-C).
4. Tape/ruler/marker/etc (these are helpful in construction).

[To be updated as the build proceeds].

## Printing Instructions

PLEASE PLEASE PLEASE make it easy on yourself and use the 3MF **NOT** the STEP files for actually uploading onto a slicer.
Whenever you load the part, make sure that if you get the following screen, you click **yes**:
<img width="667" height="191" alt="image" src="https://github.com/user-attachments/assets/e115a54a-08e0-41dc-8084-4b1005eced47" />

If you look at the naming convention for each 3MF files, it is clearly explained what settings are recommended (IE, ASA_#9_20%_3d-cubic\_4walls should be printed in ASA at 20% infill in a cubic sparse infill pattern with 4 walls). Now, this does get more complicated with the TPU. The TPU 3MF files include simple cylinders positioned around holes. These cylinders are modifiers. ***While the rest of the print is to be at 15% gyroid infill with 2 walls, these modifiers should be changed to SIX walls.*** This is VERY crucial. Heat Set Inserts are very annoying with TPU (as I've heard), and for them to be properly fastened/secured, the TPU must be made rigid. To save your time and filament whilst making heat-sets viable, these modifiers are meant to circle each cylindrical hole, providing enough material for the heat-set inserts to properly bite into the thermoplastic. If everything goes right, you should expect around 545g worth of filament being used for TPU\_1. Keep supports on for all prints.

Note that the provided settings are the basic settings. If you have other settings for supports, etc, you may use those (as those do not impact strength).

IN TOTAL YOU MAY EXPECT: 817.79g TPU being used (this is with supports) over 51.64 hours of printing. 1099.53g ASA being used (this is also with supports) over 56.05 hours of printing, for a total of around 107.69 hours of printing.

Tolerances are in-built into the prints, so long as everything is printed in the right material (ASA or TPU), as outlined. Remember, any other materials would either be more expensive alternatives, or less viable ones (ie, TPU is the best filament here because it is relatively common, flexible, but has a high temperature deflection point relative to the use-case the rover is ideal for, and ASA is the best rigid common filament with another reasonable temperature deflection point). You can see the overall assembly at this [link](https://cad.onshape.com/documents/4276ee27674d5cd4437829d8/w/b68ed14a30814a824c43fb7e/e/30365733099079a1b69868d1?renderMode=0&uiState=6a78f4664b6102bbb3720076). navigate to "The Spyder-Crab", and then the assembly of the same name), and simply hide the relevant objects to see precisely where everything goes.

Since there's so many parts (600 by my estimation), I will NOT be going over how each of the parts fit, although I believe most of it is obvious: in general, holes of diameter 4mm have a heat-set insert, holes of diamter 3mm allow for M3 screws to be passed, and so on. There are other specific cut-outs for flanged ball bearings (and so on for other parts, such as the rod-end).

Eventually, I will make a complete tutorial for how the rover will be built (step-by-step), with photos (or a video).

## Wiring Instructions

THE ROVER IS COMPLETELY SOLDERLESS. IT IS DESIGNED TO NOT REQUIRE A PCB, BUT RATHER EASILY SOURCED PARTS ONLINE. There is nothing but a bit of cutting wires/sliding them into screw terminals or WAGO lever nuts. The rough diagram can be seen in the following image, subject to change (as soon as I build it and test it myself).
<img width="480" height="640" alt="image" src="https://github.com/user-attachments/assets/4522ce63-89ad-4b6a-9d7c-93abc6f2a03d" />
<img width="1165" height="922" alt="0" src="https://github.com/user-attachments/assets/3638eef6-f6de-4540-a2af-545747c95600" />

As you can see here, this is the rough wiring layout. Connect the battery to crimped spade terminal wires (6-12 gauge), connect the other end to an ANL fuse (with ring lugs, again, crimped), to the power switch and then the buck converter (set at 12V). Route the wire from the buck converter to three separate lines via the WAGO (respectively), to the Pi, and to the servo power lines. The rest of the diagram can be read as follows. Remember, there must be power injection with the servos on top of the star topography, as there would be significant browning near stall. This can be done by taking each male-JST connector, and directing that to a female-female wire, and leading that to another male-JST connector (keeping the ground and signal wires the same, but exchanging the hot/positive lead for a voltage line out of another WAGO lever nut. Once I build this, I will add photos and a detailed guide of how this can be accomplished (after all, right now this is just all theoretical). Soon, everything should be up! Right now, it is as bare-bones as it gets, but I should have this sorted out in just a little bit of time.

This is going to be updated once I finalize the wiring, but a few teasers as of right now:
<img width="480" height="640" alt="image" src="https://github.com/user-attachments/assets/c68d8676-f866-45e5-9037-127745b9bcf2" />
<img width="2048" height="321" alt="image" src="https://github.com/user-attachments/assets/1d55326c-1cd8-486a-910c-ed33ce500ce3" />
<img width="1024" height="768" alt="image" src="https://github.com/user-attachments/assets/7706d069-737c-484b-ba25-d64f9b2986c0" />

## Setting Up The Pi
Downloading the Imager.
Take your SD card, and plug into your computer. Try using either a computer with an SD card slot (with the microSD card in its SD card "sleeve") or get a USB-SD card reader.
1. Choose the Raspberry Pi 5 as the device.
2. Choose the default software (the Raspberry Pi 8GB can tolerate the 64-bit Raspberry Pi OS).
3. Give the device a hostname/username/password you will remember. Write this down (it'll save you later).
4. If you get a dialog to connect to a network, provide your home wifi.
5. Enable SSH. You can enable a public SSH key, but I find that the password authentication setup is easier.
6. Flash your imager onto the appropriate storage device (the SD card you just plugged in).
7. Remove your SD card and plug that into the Raspberry Pi 5.
8. Connect the Raspberry Pi 5 to a monitor via a Micro-HDMI to an HDMI cable.

Setting up the Pi on Boot:
1. Once you are logged in, you can begin by setting up the VK-162. This is probably the easiest part of the setup.
    1. Plug in the VK-162 to a USB slot on the Raspberry Pi.
    2. Check the VK-162 is there with the command `ls /dev/ttyACM*` in the Raspberry Pi's CLI. The output            should be `dev/ttyACM0`
    3. Run `lsusb | grep -i "u-blox"` The result should be `Bus 003 Device 002: ID 1546:01a7 U-Blox AG`
       `[u-blox 7]` or an equivalent.
    4. Install gpsd:
       `sudo apt update`, then `sudo apt install -y gpsd gpsd-clients python3-gps`
    5. Configure gpsd:
       `sudo nano /etc/default/gpsd`, you must add: `DEVICES="/dev/ttyACM0"`, `GPSD_OPTIONS="-n"`, `START_DAEMON="true"`, and `USBAUTO="true"`
    6. Restart and test: `sudo systemctl restart gpsd`, then `cgps -s`
2. If you want to, you can change the fan settings.
     1. Type `sudo nano /boot/firmware/config.txt` into the CLI.
     2. Scroll to the very bottom and add the lines below:
        `dtparam=fan_temp0=40000`
        `dtparam=fan_temp1=45000`
        `dtparam=fan_temp2=50000`
        `dtparam=fan_temp3=55000`
        `dtparam=uart0=on`
3. Setting up the Raspberry Pi AI Camera:
    1. The black side on the Pi is on the opposite side from the USB ports, whilst the black side on the Camera faces on the opposite side from the main camera module. Wire in this fashion. Tweezers are helpful.
    2. Run `sudo apt update && sudo apt full-upgrade`
    3. Run `sudo reboot`
    4. Run `rpicam-hello -t 0s --post-process-file /usr/share/rpi-camera-assets/imx500_mobilenet_ssd.json --viewfinder-width 1920 --viewfinder-height 1080 --framerate 30` This should return a live feed with post-processed object detection
    5. Run `rpicam-hello -t 0s --post-process-file /usr/share/rpi-camera-assets/imx500_posenet.json --viewfinder-width 1920 --viewfinder-height 1080 --framerate 30` This should return a live feed with post-processed pose detection.
    6. If you wish to change the focal length (and thus the focus), take the plastic knob, point the hollow end towards the camera module, press, and turn. Test out various focal lengths (but be careful as to not damage the electronic) through the hole until you find an appropriate one.
4. Setting up the __FIRST__ STS3215 Servo:
    1. Connect the servo horns to each servo. Usually, you can simply push into the toothed brass attachment until the horn is flush with the bottom and a click is heard. If the horn is refusing to be attached properly, use your thumb to push until you seat the horn a bit into the tooth (you can tell this is the case if you leave your hand and the horn doesn't move). Then, use your palm to connect it until a loud click is heard. The tolerances are so tight that the latter technique is often necessary (and in my experience the best one). Screw in everything (but leave a bit of leeway, so the horn is stable but the screw can still "wiggle" a bit).
    2. Make sure your bus servo adapter has the yellow jumpers set to "B" as labeled on the board. Find a USB to a USB-C cable. If you do not have one, a traditional USB to a USB-C female socket is acceptable when connected with a USB-C to a USB-C cable (in fact, the latter architecture is the one I chose). Attach the USB-C port to the bus servo adapter, and the USB to the Pi. Wire a servo through one of the JST ports to the servo's JST port (just be careful for the orientation). Plug in your bus servo adapter with a DC5521 power jack (connected to a 12V power supply for setup). Wait a few seconds, and then run dmesg | tail -1. Remember this, this is the port you will use in Step 8.
    3. Label the serial bus servo you have just plugged in with tape and a marker as 1. This is your servo ID (which must be unique for EACH of the 12 servos to allow for a functioning daisy-chain network). Label the rest of the 12 servos with similar numbering. You will label each servo with that ID during the following setup.
    4. Run `sudo apt update && sudo apt install unrar -y` This downloads a program capable of unzipping rar files on Linux (which is the format provided by Waveshare's ST-series servos).
    5. [Download the Waveshare ST-series Python control library.](https://files.waveshare.com/wiki/Bus_Servo_Driver_HAT_A/SCServo_Linux.rar) This will be the code that will help you setup the serial bus servos. Download onto a predetermined location, and then run `unrar x SCServo_Linux.rar` This will unzip the file, allowing for setup. Then, run `sudo apt install cmake -y` CMake will be the application that actually sets up your code and allows you to control the servos.
    6. Then, open your SCServoLinux file in a new terminal (right click the SCServoLinux file -inside the SCServoLinux_220329, and then click "Open in Terminal"). Then, run `cmake .` Then, run `make SCServo` It should say "[100%] Built target SCServo"
    7. Then, type in `cd examples/SMS_STS/WritePos` Run `nano WritePos.cpp` Change the 17th line to say "1000000" instead of "115200." This is the servo's baud rate, and it is 1000000 (the hardcoded value is wrong). Run Ctrl+O, click Enter, then run Ctrl+X. Now, you may run `cmake .` and then `make WritePos`
    8. Go back to the port from Step 2. Run `sudo ./WritePos [port]`, where your final command should look like `sudo ./WritePos /dev/ttyACM2` When you want this to stop, you can run Ctrl+C. Your servo should spin.
5. Setting up __THE SECOND__ Servo:
    1. Disconnect power from the serial bus servo adapter and the USB-C cable connecting the board to the Pi. Once power is off, disconnect the first serial bus servo and replace it for the one labeled "2" and wire that.
    2. Run `mkdir ~/SCServo_Linux_220329/SCServo_Linux/examples/SMS_STS/ChangeID` Find the ChangeID.cpp file attached above and drop it into the newly created ChangeID folder.
    3. Run:
       ```
       cp ~/SCServo_Linux_220329/SCServo_Linux/examples/SMS_STS/WritePos/CMakeLists.txt \
       ~/SCServo_Linux_220329/SCServo_Linux/examples/SMS_STS/ChangeID/
       cd ~/SCServo_Linux_220329/SCServo_Linux/examples/SMS_STS/ChangeID
       sed -i 's/WritePos/ChangeID/g' CMakeLists.txt
       ```
       and then run, in the ChangeID folder:
       ```
       cmake .
       make
       sudo ./ChangeID /dev/[port]
       ```
       Where [port] is in the format "ttyACM0".
    4. Run `nano WritePos.cpp` and check if "sm_st.WritePosEx" has the number 2 following it. This is the ID the Pi will ping. If it is not 2, change it to be 2 (ONLY IF YOU DID CHANGE THE NUMBER, run Ctrl+O, Enter, Ctrl+X. Then, open the WritePos file in a separate terminal and run `cmake .`, then `make`). Then, run `sudo ./WritePos /dev/[port]` Your second servo should know spin.
6. Setting up __SUBSEQUENT__ Servos:
    1. Disconnect power from the serial bus servo adapter and the USB-C cable connecting the board to the Pi. Once power is off, disconnect the first serial bus servo and replace it for the next labeled servo and wire that.
    2. Run `nano ChangeID.cpp` in the ChangeID folder terminal and change the `#define NEW_ID 2` to have your new number replacing the 2 (or whatever other number is there). Ctrl+O, Enter, Ctrl+X, and in that same terminal, run `cmake .` `make` and finally `sudo ./ChangeID /dev/[port]`
    3. Run `nano WritePos.cpp` in the WritePos folder terminal and change the `WritePosEx` command to have your new ID number. Ctrl+O, Enter, Ctrl+X, run `cmake .` `make` and finally `sudo ./WritePos /dev/[port]` so that your newest servo spins.
7. Setting up the US-100s:
    1. Run `sudo apt full-upgrade` and then `sudo apt install gpiod` Then, run `sudo apt install python3-gpiozero`
    2. Run `gpiodetect` and check the "gpiochip" that has "pinctrl-rp1". For most boots, this is "gpiochip0." Since I have the version of the pi with this boot, I have saved the code to run with chip=0. If you have a different gpio chip connected to "pinctrl," change the provided code to have that chip number at the 6th line's `chip=` section.
    3. Remove the black jumper out of each US-100 sensor (this is because we will be using each in "HC-SR04" mode, NOT UART mode. Then wire EACH sensor with the following arrangement (pull up a GPIO pin layout for the Raspberry Pi online while setting up), noting that since all three are on different GPIO pins, you may connect all three sensors at the same time to fully test sensor capability:

       a. The first sensor should have its VCC port connected to PIN 1, one of its ground ports connected to PIN 9, its Trig/TX pin connected to GPIO 5 OR PIN 29, and its Echo pin connected to GPIO 6 or PIN 31.
       
       b. The second sensor should have its VCC port connected to PIN 17, one of its ground ports connected to PIN 6, its Trig/TX pin connected to GPIO 17 or PIN 11, and its Echo pin connected to GPIO 27 or PIN 13.
       
       c. The third sensor should have its VCC port connected to PIN 2, one of its ground ports connected to PIN 14, its Trig/TX pin connected to GPIO 23 or PIN 16, and its Echo pin connected to GPIO 24 or PIN 18.

       <img width="800" height="450" alt="image" src="https://github.com/user-attachments/assets/66fc82b8-9b74-4fde-8409-ae129edd2ee5" />

    5. Through the top left of the desktop, open programming and then Thonny. Copy/paste the following code into it.
        ```
        import time
        from gpiozero import DistanceSensor
        from gpiozero.pins.lgpio import LGPIOFactory
        
        # Run gpiodetect, whichever chip has "pinctrl" associated with it is the chip below
        factory = LGPIOFactory(chip=0)
        
        sensors = {
            "left":   DistanceSensor(echo=6,  trigger=5,  pin_factory=factory, max_distance=4.5),
            "center": DistanceSensor(echo=27, trigger=17, pin_factory=factory, max_distance=4.5),
            "right":  DistanceSensor(echo=24, trigger=23, pin_factory=factory, max_distance=4.5),
        }
        
        
        def read_all():
            # Time sleep meant to eliminate cross-talk
            readings = {}
            for name, sensor in sensors.items():
                readings[name] = sensor.distance * 100
                time.sleep(0.03)
            return readings
        
        
        if __name__ == "__main__":
            try:
                while True:
                    readings = read_all()
                    print(
                        f"L: {readings['left']:.1f}cm  "
                        f"C: {readings['center']:.1f}cm  "
                        f"R: {readings['right']:.1f}cm"
                    )
                    time.sleep(0.1)
            except KeyboardInterrupt:
                pass
        ```
    7. Run the code. You should now have working ultrasonic sensors. If this works, you're all set up for any of the later code! If one does not work, wire it individually in the same configuration to troubleshoot. Common errors involve incorrect wiring (ie, wrong gpio pins are connected OR each DuPont connector is improperly secured) and/or failing to download the correct modules/copying the proper code. This works on the Pi 5 ONLY; older, outdated modules like pigpio are necessary for older models.
  
[To be continued].
