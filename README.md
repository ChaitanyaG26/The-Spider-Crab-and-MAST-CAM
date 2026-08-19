# The Spider-Crab and MAST-CAM
A rover (rough "Perserverance-replica") equipped with rocker bogie (with a differential bar) suspension, capable of object detection + avoidance (with the RPi AI camera/US-100 sensors), remote control, and autonomous travel (with an onboard GPS). __THERE IS NO PCB ON THIS BUILD. IT IS SOLDERLESS.__ Modeled around the STS-3215 servo. (Special thanks here to Logan Wheatcroft for helping me spell "attach" and getting this project reviewed, and yes, his research project IS real despite what he may say, and of course, SWN from Print Legion for printing everything, alongside my brother -- who "singlehandedly got my project reviewed").

## Table of Contents
- [The Spider-Crab and MAST-CAM](#the-spider-crab-and-mast-cam)
- [Table of Contents](#table-of-contents)
- [Hack-Club Required Q&A](#hack-club-required-qa)
- [Full Assembly Screenshots](#full-assembly-screenshots)
- [BOM](#bom)
- [Printing Instructions](/docs/printing.md)
- [Setting Up The Pi](/docs/electronics-setup.md)
- [Wiring Instructions](/docs/wiring.md)
- [Assembly Instructions](/docs/assembly.md)

## Hack-Club Required Q&A
So, let's see:
1. What is this? This is designed to be a hyper-mobile rover that is capable of autonomous driving, pathfinding (because it is combined with a GPS), and providing camera footage. I hope that I will be able to modify it for usage out of one of two scenarios: 1. hopefully rescue missions on debris/unstable territory (since the rover is pretty lightweight, weighing in at just about 4.5 kgs, and is theoretically capable of some crazy feats as long as the net tractive force is stable), or 2. conducting BAER assessment (providing photos to researchers about theoretically dangerous territories subject to wildfires). If the rover lacks mobility in practice, I will probably move towards the latter concept, in which case I have plans of adding six "sample"-collecting add-ons to be attached to each wheel.
2. Why did I build this? Well, I've always been interested in Aerospace, and I've always thought the Perserverance rover was magical. I'm part of a drone team, and since we just built a drone that was not much unlike Ingenuity, I wondered if I could "complete" the duo here on Earth entirely from my own vision. This rover is the result of that work. I've gone through four separate unique versions of this rover (and even more "micro-versions" where I had to redesign parts inside each version to keep up with out-of-stock products and so on), but the hope is that this final rover will be just as mobile as the rover on Mars, MUCH faster (hopefully going at a mile an hour), WAY less power efficient (but to be fair, I don't exactly have the greatest parts at my disposal), and a final cost JUST under $1000 (so it's affordable), and accessible (meaning it is COMPLETELY idiot-proof -- I should be a good test subject for this, since I have not dealt with PCB design in the LEAST --, and has NO soldering/pcbs involved).

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

| Quantity | Item | Description | Source | Total Cost |
| :--- | :---: | :---: | :---: | ---: |
| 24 | [JST 3-Pin to Male](https://www.adafruit.com/product/3893) | Allows for power-injection to the Feetech servos. | Adafruit |  $144.95  |
| 3 | [US-100 Ultrasonic Sensor](https://www.adafruit.com/product/4019) | Allows the rover to navigate terrain/avoid objects. | |  |
| 1 | [Pi Camera Cable, 500mm](https://www.adafruit.com/product/5820) | Allows the camera module to freely move. | |  |
| 1 | [Raspberry Pi AI Camera](https://www.adafruit.com/product/6009) | Allows the rover to have object detection/recognition capabilities. | |  |
| 1 | [Raspberry Pi 5, 8GB](https://aaawave.com/products/raspberry-pi-5-development-broad?variant=51441087152449) | The "brains" of the rover. | AAAWave |  $161.63  |
| 1 | [Bus Servo Adapter](https://www.ebay.com/itm/206185697206?mkevt=1&mkpid=0&emsid=e11412.m144671.l197929&mkcid=7&ch=osgood&euid=6d034ae6c7bd442a96e4e9cbff91307c&bu=45719063109&exe=0&ext=0&osub=-1%7E1&crd=20260803165232&segname=11412) | Allows for the Pi to communicate with the serial bus servos. | eBay |  $12.23  |
| 6 | [Feetech STS3215](https://www.aliexpress.us/item/3256808670629323.html?spm=a2g0o.order_list.order_list_main.41.4d5c18021PISjv&gatewayAdapt=glo2usa) | Allows for the rover to move (both its wheels and the camera). | Aliexpress |  $404.07  |
| 1 | [Soldering Iron, HS02A](https://www.aliexpress.us/item/3256809503553320.html?spm=a2g0o.order_list.order_list_main.47.4d5c18021PISjv&gatewayAdapt=glo2usa) | Necessary to attach the heat-set inserts, tool. | |  |
| 1 | [Hot Air Gun](https://www.aliexpress.us/item/3256810350606440.html?spm=a2g0o.order_list.order_list_main.53.4d5c18021PISjv&gatewayAdapt=glo2usa) | Necessary to secure the shrink tubing to properly insulate wires. | |  |
| 1 | [Heat Shrink Tubing](https://www.aliexpress.us/item/3256807959988149.html?spm=a2g0o.order_list.order_list_main.35.4d5c18021PISjv&gatewayAdapt=glo2usa) | Necessary to electrically insulate certain wires. | |  |
| 1 | [WAGO 221-415](https://www.aliexpress.us/item/3256811952946871.html?spm=a2g0o.order_list.order_list_main.29.4d5c18021PISjv&gatewayAdapt=glo2usa) | Necessary to proper distribute power as desired. | |  |
| 1 | [USB-C/Screw Terminal](https://www.aliexpress.us/item/3256812221028465.html?spm=a2g0o.order_list.order_list_main.11.4d5c18021PISjv&gatewayAdapt=glo2usa) | Necessary to provide the RPi power through bare wires. | |  |
| 2 | [1M, 12AWG Wire](https://www.aliexpress.us/item/3256808222999562.html?spm=a2g0o.order_list.order_list_main.5.4d5c18021PISjv&gatewayAdapt=glo2usa) | Facilitates an electrical connection across parts. | |  |
| 1 | [RPi 5, Active Cooler](https://www.amazon.com/dp/B0CW164TCW?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0) | Allows for smooth Pi operation, prevents overheating. | Amazon |  $162.99  |
| 1 | [12V/30A Daiertek Switch](https://www.amazon.com/dp/B0FL6B75PM?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_0) | A manual power switch for the rover. | |  |
| 1 | [20A ANL Fuse](https://www.amazon.com/dp/B01LXQWZ7L?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_1) | Protects electronics downstream. | |  |
| 1 | [VK-162 GPS](https://www.amazon.com/dp/B078Y52FGQ?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_2) | Allows the Raspberry Pi to navigate via GPS. | |  |
| 1 | [Rod Ends (Small)](https://www.amazon.com/dp/B0828TMYLD?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_0) | Allow for the rocker-bogie suspension to function. | |  |
| 1 | [M3x5.7 Heat Set Inserts](https://www.amazon.com/dp/B0D7M4WWBW?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_1) | Allow for the printed parts to be fastened together. | |  |
| 1 | [2300 PCS, M3 Screws Kit](https://www.amazon.com/dp/B0FGX859K8?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_2) | Necessary hardware for the assembly to be possible. | |  |
| 1 | [Flanged Ball Bearing](https://www.amazon.com/dp/B0G1B7H48R?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_3) | Necessary hardware for the rocker-bogie to be possible. | |  |
| 1 | [Jumper Cables](https://www.amazon.com/dp/B07GD2PGY4?ref_=ppx_hzod_title_dt_b_fed_asin_title_2_4) | Allow for power-injection to the daisy-chained servos. | |  |
| 2 | [20A/300W Buck Converter](https://www.amazon.com/dp/B099S2VQ2Q?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0) | Allow for the 12V battery to transfer power downstream. | |  $129.46  |
| 1 | [PTFE/Plumber's Tape](https://www.amazon.com/dp/B091913Z7F?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_0) | Facilitate the physical "bushing" inside the wheels. | |  |
| 1 | [12V, 15Ah LiFePO4](https://www.amazon.com/dp/B0CSWMF35Q?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_1) | Powers the rover (hopefully for 2+ hours). | |  |
| 1 | [Female Spade Connectors](https://www.amazon.com/dp/B07C9CQ8RT?ref=ppx_yo2ov_dt_b_fed_asin_title) | Allow for solderless power supply. | |  |
| 1 | [Digital Multimeter](https://www.amazon.com/dp/B08CX9W7G3?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0) | Checking if the electronic components are viable. | |  $38.43  |
| 1 | [Ring Lugs Set](https://www.amazon.com/dp/B0F99P7SGT?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_0) | Allow for wires to be crimped onto the ANL fuse. | |  |
| 1 | [12V LiFePO4 Charger](https://www.amazon.com/dp/B0G3ZNWC4J?ref_=ppx_hzod_title_dt_b_fed_asin_title_1_1) | Charges the battery. | |  |
| 1 | [M4x10mm Screws](https://www.amazon.com/dp/B0FH2T8PD1?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0) | Necessary hardware for the assembly to be possible. | |  $4.30  |
| 1 | [ASA Filament,<br>2kgs (1.2ish needed)](https://www.amazon.com/dp/B0G2L8HKB8?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0) | Filament for prints. | |  $37.87  |
| 1 | [TPU 95A Filament,<br>1kg (0.8ish needed)](https://www.amazon.com/dp/B07VDP2S3P?ref_=ppx_hzod_title_dt_b_fed_asin_title_0_0) | Filament for prints. | |  $25.97  |
| 1 | [Crimpling "Plier"](https://www.amazon.com/dp/B073YG65N2?ref=ppx_yo2ov_dt_b_fed_asin_title) | Necessary to properly attach the solderless lugs/terminals. | | $8.99 |
| Total: |  |  |  |  $1,121.90  |

Recommended additional common supplies (double-check you have these):
1. An official raspberry pi 5 power supply (27W).
2. A 12V power supply (preferably 2-3A or less) with a DC5521 power jack.
3. USB to USB-C cable (or a USB to a USB-C female port, and then a USB-C to a USB-C).
4. Micro-HDMI to HDMI cable (this is HIGHLY recommended for setup, because a monitor makes everything easier, although THEORETICALLY you could set up the Pi completely headlessly via SSH).
5. Tape/ruler/marker/etc (these are helpful in construction).

[To be updated as the build proceeds].

## [Printing Instructions](docs/printing.md)
Follow the link above. A quick teaser of what is to come:
<img width="641" height="353" alt="image" src="https://github.com/user-attachments/assets/e4380bd2-8186-4307-b9e4-c62c8aa3f1fc" />


## [Setting Up The Pi](docs/electronics-setup.md)
Follow the link above. Another quick teaser:
<img width="768" height="1024" alt="image" src="https://github.com/user-attachments/assets/7f734557-85f6-4847-b192-476a2d73c774" />

## [Wiring Instructions](docs/wiring.md)
This is going to be updated once I finalize the wiring, but a teaser as of right now:
<img width="480" height="640" alt="image" src="https://github.com/user-attachments/assets/c68d8676-f866-45e5-9037-127745b9bcf2" />

## [Assembly Instructions](docs/assembly.md)
[To be created].
