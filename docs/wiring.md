## Setting Up The Pi
Note that this step should take place BEFORE the wiring is finalized (ie, the servos are included in the wiring), so everything can be tested properly and debugged (at least everything relevant to the coding).

<img width="1024" height="768" alt="image" src="https://github.com/user-attachments/assets/cff378a4-e67e-4ebd-aaf7-3c19dcf5902b" />
A quick preview at what y'all are getting into.


Downloading the Imager.
Take your SD card, and plug into your computer. Try using either a computer with an SD card slot (with the microSD card in its SD card "sleeve") or get a USB-SD card reader.
1. Choose the Raspberry Pi 5 as the device.
<img width="846" height="597" alt="image" src="https://github.com/user-attachments/assets/590922dc-5bc6-4e13-b5b0-14115f079394" />

2. Choose the default software (the Raspberry Pi 8GB can tolerate the 64-bit Raspberry Pi OS).
<img width="845" height="597" alt="image" src="https://github.com/user-attachments/assets/206ec632-2515-4be1-84c0-8099e414b5f0" />

3. Choose your storage device as the SD card (which should be inserted into your computer).

4. Give the device a hostname/username/password you will remember. Write this down (it'll save you later).
5. If you get a dialog to connect to a network, provide your home wifi (and password, of course).
6. Enable SSH. You can enable a public SSH key, but I find that the password authentication setup is easier.
7. Flash your imager onto the appropriate storage device (the SD card you just plugged in).
8. Remove your SD card and plug that into the Raspberry Pi 5.
9. Connect the Raspberry Pi 5 to a monitor via a Micro-HDMI to an HDMI cable.

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
<img width="1024" height="768" alt="image" src="https://github.com/user-attachments/assets/16159cc3-a911-4db9-8e35-fb1a93f39c4d" />

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
  
<img width="576" height="768" alt="image" src="https://github.com/user-attachments/assets/ac6feb4f-fb10-4587-9e8c-6e1e1b127f59" />

Kudos if your screen also looks something like this.


4. Setting up the __FIRST__ STS3215 Servo:
    1. Connect the servo horns to each servo. Usually, you can simply push into the toothed brass attachment until the horn is flush with the bottom and a click is heard. If the horn is refusing to be attached properly, use your thumb to push until you seat the horn a bit into the tooth (you can tell this is the case if you leave your hand and the horn doesn't move). Then, use your palm to connect it until a loud click is heard. The tolerances are so tight that the latter technique is often necessary (and in my experience the best one). Screw in everything (but leave a bit of leeway, so the horn is stable but the screw can still "wiggle" a bit).
    2. Make sure your bus servo adapter has the yellow jumpers set to "B" as labeled on the board. Find a USB to a USB-C cable. If you do not have one, a traditional USB to a USB-C female socket is acceptable when connected with a USB-C to a USB-C cable (in fact, the latter architecture is the one I chose). Attach the USB-C port to the bus servo adapter, and the USB to the Pi. Wire a servo through one of the JST ports to the servo's JST port (just be careful for the orientation). Plug in your bus servo adapter with a DC5521 power jack (connected to a 12V power supply for setup). Wait a few seconds, and then run `dmesg | tail -1`. Remember this, this is the port you will use in Step 8.
    3. Label the serial bus servo you have just plugged in with tape and a marker as 1. This is your servo ID (which must be unique for EACH of the 12 servos to allow for a functioning daisy-chain network). Label the rest of the 12 servos with similar numbering. You will label each servo with that ID during the following setup.
    4. Run `sudo apt update && sudo apt install unrar -y` This downloads a program capable of unzipping rar files on Linux (which is the format provided by Waveshare's ST-series servos).
    5. [Download the Waveshare ST-series Python control library.](https://files.waveshare.com/wiki/Bus_Servo_Driver_HAT_A/SCServo_Linux.rar) This will be the code that will help you setup the serial bus servos. Download onto a predetermined location, and then run `unrar x SCServo_Linux.rar` This will unzip the file, allowing for setup. Then, run `sudo apt install cmake -y` CMake will be the application that actually sets up your code and allows you to control the servos.
    6. Then, open your SCServoLinux file in a new terminal (right click the SCServoLinux file -inside the SCServoLinux_220329, and then click "Open in Terminal"). Then, run `cmake .` Then, run `make SCServo` It should say "[100%] Built target SCServo"
    7. Then, type in `cd examples/SMS_STS/WritePos` Run `nano WritePos.cpp` Change the 17th line to say "1000000" instead of "115200." This is the servo's baud rate, and it is 1000000 (the hardcoded value is wrong). Run Ctrl+O, click Enter, then run Ctrl+X. Now, you may run `cmake .` and then `make WritePos`
    8. Go back to the port from Step 2. Run `sudo ./WritePos [port]`, where your final command should look like `sudo ./WritePos /dev/ttyACM2` When you want this to stop, you can run Ctrl+C. Your servo should spin.
<img width="576" height="768" alt="image" src="https://github.com/user-attachments/assets/968955a3-efb9-47e7-9868-e9dcbd140674" />
<img width="2160" height="2880" alt="image" src="https://github.com/user-attachments/assets/96c39880-5312-439f-8933-21c98caed42a" />

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
  
Yes, you should label your servos.
<img width="1024" height="768" alt="image" src="https://github.com/user-attachments/assets/1e56e45d-c3c9-417d-8a56-1700159f1e9c" />
See how nice and easy that looks?

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

<img width="1024" height="768" alt="image" src="https://github.com/user-attachments/assets/89e088e2-7d9f-43d9-844a-a5fa66d7e09e" />

Hopefully, your screen is not very different or far away from this.

  
P.S, if everything seems like a wired mess, do NOT fret.

<img width="576" height="768" alt="image" src="https://github.com/user-attachments/assets/dfc89ec6-c46e-4c87-9bcf-b817d608e569" />

Hopefully both of us can fix this eventually.

 
[To be continued]. If ANY of the steps above return an error or an issue in setup, you may contact me at chaitanyag2612@gmail.com for support.
