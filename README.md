# The-Spider-Crab-and-MAST-CAM
A rover (rough "perserverance-replica") equipped with rocker bogie (with a differential bar) suspension, capable of object detection + avoidance (with the RPi AI camera/US-100 sensors), remote control, and autonomous travel (with an onboard GPS). Modeled around the STS-3215 servo.

### Hack-Club Required Q&A
So, let's see:
1. What is this? This is designed to be a hyper-mobile rover that is capable of autonomous driving, pathfinding (because it is combined with a GPS), and providing camera footage. I hope that I will be able to modify it for usage out of one of two scenarios: 1. hopefully rescue missions on debris/unstable territory (since the rover is pretty lightweight, weighing in at just about 4.5 kgs, and is theoretically capable of some crazy feats as long as the net tractive force is stable), or 2. conducting BAER assessment (providing photos to researchers about theoretically dangerous territories subject to wildfires). If the rover lacks mobility in practice, I will probably move towards the latter concept, in which case I have plans of adding six "sample"-collecting add-ons to be attached to each wheel.
2. Why did I build this? Well, I've always been interested in Aerospace, and I've always thought the Perserverance rover was magical. I'm part of a drone team, and since we just built a drone that was not much unlike Ingenuity, I wondered if I could "complete" the duo here on Earth entirely from my own vision. This rover is the result of that work. I've gone through four separate unique versions of this rover (and even more "microversions" where I had to redesign parts inside each version to keep up with out-of-stock products and so on), but the hope is that this final rover will be just as mobile as the rover on Mars, MUCH faster (hopefully going at a mile an hour), WAY less power efficient (but to be fair, I don't exactly have the greatest parts at my disposal), and a final cost JUST under $1000 (so it's affordable), and accessible (meaning it is COMPLETELY idiot-proof -- I should be a good test subject for this, since I have not dealt with PCB design in the LEAST --, and has NO soldering/pcbs involved).

### Screenshots of the full assembly.
Okay, before we get started, let's get a few hero renders out of the way.
<img width="1160" height="734" alt="image" src="https://github.com/user-attachments/assets/6676c012-a744-4464-b192-14bec569b187" />
<img width="1209" height="728" alt="image" src="https://github.com/user-attachments/assets/54e44a33-d582-49d8-9911-47fb82f72d3f" />
<img width="827" height="618" alt="image" src="https://github.com/user-attachments/assets/5dba669b-d5bb-4bf9-8e4a-0668c4139220" />
<img width="779" height="692" alt="image" src="https://github.com/user-attachments/assets/4282b201-71b7-4e5f-95c7-68f8b08242f5" />
<img width="900" height="641" alt="image" src="https://github.com/user-attachments/assets/5e02f464-8cb5-4562-817a-8b53bcaf70d9" />
<img width="811" height="653" alt="image" src="https://github.com/user-attachments/assets/907d392b-3f84-469b-9ffb-63fdbcba52d7" />
Eventually, when I build this, I should be able to atach a few "normal" photos of how it'll look.

### BOM.
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

### Instructions to print:

PLEASE PLEASE PLEASE make it easy on yourself and use the 3MF **NOT** the STEP files for actually uploading onto a slicer.
Whenever you load the part, make sure that if you get the following screen, you click **yes**:
<img width="667" height="191" alt="image" src="https://github.com/user-attachments/assets/e115a54a-08e0-41dc-8084-4b1005eced47" />

If you look at the naming convention for each 3MF files, it is clearly explained what settings are recommended (IE, ASA_#9_20%_3d-cubic\_4walls should be printed in ASA at 20% infill in a cubic sparse infill pattern with 4 walls). Now, this does get more complicated with the TPU. The TPU 3MF files include simple cylinders positioned around holes. These cylinders are modifiers. ***While the rest of the print is to be at 15% gyroid infill with 2 walls, these modifiers should be changed to SIX walls.*** This is VERY crucial. Heat Set Inserts are very annoying with TPU (as I've heard), and for them to be properly fastened/secured, the TPU must be made rigid. To save your time and filament whilst making heat-sets viable, these modifiers are meant to circle each cylindrical hole, providing enough material for the heat-set inserts to properly bite into the thermoplastic. If everything goes right, you should expect around 545g worth of filament being used for TPU\_1. Keep supports on for all prints.

Note that the provided settings are the basic settings. If you have other settings for supports, etc, you may use those (as those do not impact strength).

IN TOTAL YOU MAY EXPECT: 817.79g TPU being used (this is with supports) over 51.64 hours of printing. 1099.53g ASA being used (this is also with supports) over 56.05 hours of printing, for a total of around 107.69 hours of printing.

Tolerances are in-built into the prints, so long as everything is printed in the right material (ASA or TPU), as outlined. Remember, any other materials would either be more expensive alternatives, or less viable ones (ie, TPU is the best filament here because it is relatively common, flexible, but has a high temperature deflection point relative to the use-case the rover is ideal for, and ASA is the best rigid common filament with another reasonable temperature deflection point). You can see the overall assembly at this [link](https://cad.onshape.com/documents/4276ee27674d5cd4437829d8/w/b68ed14a30814a824c43fb7e/e/30365733099079a1b69868d1?renderMode=0&uiState=6a78f4664b6102bbb3720076). navigate to "The Spyder-Crab", and then the assembly of the same name), and simply hide the relevant objects to see precisely where everything goes.

Since there's so many parts (600 by my estimation), I will NOT be going over how each of the parts fit, although I believe most of it is obvious: in general, holes of diameter 4mm have a heat-set insert, holes of diamter 3mm allow for M3 screws to be passed, and so on. There are other specific cut-outs for flanged ball bearings (and so on for other parts, such as the rod-end).

Eventually, I will make a complete tutorial for how the rover will be built (step-by-step), with photos (or a video).

### Instructions to wire:

THE ROVER IS COMPLETELY SOLDERLESS. IT IS DESIGNED TO NOT REQUIRE A PCB, BUT RATHER EASILY SOURCED PARTS ONLINE. There is nothing but a bit of cutting wires/sliding them into screw terminals or WAGO lever nuts. The rough diagram can be seen in the following image, subject to change (as soon as I build it and test it myself).
<img width="1165" height="922" alt="0" src="https://github.com/user-attachments/assets/3638eef6-f6de-4540-a2af-545747c95600" />

As you can see here, this is the rough wiring layout. Connect the battery to crimped spade terminal wires (6-12 gauge), connect the other end to an ANL fuse (with ring lugs, again, crimped), to the power switch and then the buck converter (set at 12V). Route the wire from the buck converter to three separate lines via the WAGO (respectively), to the Pi, and to the servo power lines. The rest of the diagram can be read as follows. Remember, there must be power injection with the servos on top of the star topography, as there would be significant browning near stall. This can be done by taking each male-JST connector, and directing that to a female-female wire, and leading that to another male-JST connector (keeping the ground and signal wires the same, but exchanging the hot/positive lead for a voltage line out of another WAGO lever nut. Once I build this, I will add photos and a detailed guide of how this can be accomplished (after all, right now this is just all theoretical). Soon, everything should be up! Right now, it is as bare-bones as it gets, but I should have this sorted out in just a little bit of time.

### Instructions to code:
Setting up the Pi: Downloading the Imager.
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
    2. Check the VK-162 is there with the command __ls /dev/ttyACM*__ in the Raspberry Pi's CLI. The output            should be __dev/ttyACM0.__
    3. Run __lsusb | grep -i "u-blox"__. The result should be __Bus 003 Device 002: ID 1546:01a7 U-Blox AG__
       __[u-blox 7]__ or an equivalent.
    4. Install gpsd:
       __sudo apt update__, then __sudo apt install -y gpsd gpsd-clients python3-gps__
    5. Configure gpsd:
       __sudo nano /etc/default/gpsd__, you must add: __DEVICES="/dev/ttyACM0"__, __GPSD_OPTIONS="-n"__, __START_DAEMON="true"__, and __USBAUTO="true"__
    6. Restart and test: __sudo systemctl restart gpsd__, then __cgps -s__.
2. If you want to, you can change the fan settings.
     1. Type __sudo nano /boot/firmware/config.txt__ into the CLI.
     2. Scroll to the very bottom and add the lines below:
        __dtparam=fan_temp0=40000__
        __dtparam=fan_temp1=45000__
        __dtparam=fan_temp2=50000__
        __dtparam=fan_temp3=55000__
        __dtparam=uart0=on__
3. Setting up the Raspberry Pi AI Camera:
    1. The black side on the Pi is on the opposite side from the USB ports, whilst the black side on the Camera faces on the opposite side from the main camera module. Wire in this fashion. Tweezers are helpful.
    2. Run __sudo apt update && sudo apt full-upgrade__.
    3. Run __sudo reboot__.
    4. Run __rpicam-hello -t 0s --post-process-file /usr/share/rpi-camera-assets/imx500_mobilenet_ssd.json --viewfinder-width 1920 --viewfinder-height 1080 --framerate 30__. This should return a live feed with post-processed object detection
    5. Run __rpicam-hello -t 0s --post-process-file /usr/share/rpi-camera-assets/imx500_posenet.json --viewfinder-width 1920 --viewfinder-height 1080 --framerate 30__. This should return a live feed with post-processed pose detection.
    6. If you wish to change the focal length (and thus the focus), take the plastic knob, point the hollow end towards the camera module, press, and turn. Test out various focal lengths (but be careful as to not damage the electronic) through the hole until you find an appropriate one.
