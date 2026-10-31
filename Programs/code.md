# AN Overview of the two programs!
## First, an explanation...
### rover.cpp
Note that lines 1-36 are the imports, and the lines between 37-149 set up the rover (mostly dimensions for the Ackermann steering geometry, a few speed limits, time waited for an echo from the ultrasonic sensors, among other relevant things).

Then, we get to struct Input. This is the code that allows for the rover to actually be controlled (there are two ways this is done, two "sources" of input: either the "g.pad", which is the Xbox Controller - note that I never tested this out, so minor code tweaks may be necessary to make this work - or "g.web," which is the web-browser-based controller for he rover). Both are around one "mutex," which is coding jargon for only one of these input methods may be used at a time (if you're into coding, this means you're preventing multiple threads, or processes, from accessing a single thing, a resource, at the same time). There's code later that allows for a smooth switching between these two input methods (at least, there should be, unless the Xbox code is defunct - which I cannot tell because I have not actually tested it).

There's also a handoff between the input commands and the autonomous for the rover. (See struct AutoCmd... etc.) Note that there's a mutex here too (so no more than ONE thing is controlling the rover), and the values here are "thread-safe." Now this is also coding jargon, but it prevents "race conditions" (yes, more coding jargon, but I'll explain it momentarily). Imagine we have multiple processes changing one variable (ie, turning at a specific radius). I have multiple processes trying to change the turn value for each servo; here, a mutex is nice because you want all the wheels to turn at an angle consistent with the other (this is a complicated multi-variable process). But if we just have one variable changing, then you can use an atomic to reach thread safety.
  
  a. What a race condition is: Say I have a variable N = 1. I have Process 1 and 2 that are trying to increment N, and I call Process 1, then Process 2, to get a double increment for N, reaching N = 3.
  
  b. Process 1 reads N as 1. Then Process 2 reads N as 1.
  
  c. Process 1 then increments N to 1++, which is 2. Then, Process 2 increments N to 1++, which is 2.
  
  d. Process 1 writes N as 2. Then, Process 2 writes N as 2.

  What happened? I called Process 1, then 2, but both began accessing N before the other finished writing it, so they achieved only one increment. A thread-safe variable doesn't allow this. It forces Process 1 to complete before Process 2 begins. Now, this does get complicated because it can cause deadlocks (think Process 1 holds a variable from one mutex, and Process 2 holds a variable from the other, but Process 1 needs a variable from Process 2's mutex - which it cannot reach until Process 2 completes - and Process 2 needs a variable from Process 1's mutex - which it cannot reach until Process 2 completes) - and so you need related values to all be together). This is why you might use mutexes over atomics, because atomics are only one-variable, but if multiple values are changed at once, they can be grouped into a mutex.

The camera thread works on updating the newest JPEG every time. There's no point in storing a video; that takes up too much space. It takes a frame, runs onboard processing with its YOLO11n model (which you should've downloaded), and then continues. The Viewer value runs as an RAII counter (also coding jargon; this means "Resource Acquisition Is Initialization," or basically, you open the webstream = you increment viewer, you close the webstream = you decrement viewer).

Lines 201-286 are pretty self-explanatory; they set up the servos, initialize them so that they have a "home" swerve drive position they can turn back to (ie, if you turned right, click the "home" button and your servos will point straight forward again), make sure they all turn at the same exact rate, put them in the right mode (wheel mode for the wheels and position mode for the swerve drive and camera turret servos) and set up the Ackermann geometry.

The actual control loop starts on 286. This reads inputs, then decides what the rover needs to do, and sends commands to the Pi, which sends them to the servo bus adapter, which sends them to the servos. It "ticks" every 20ms, which means it has a fixed 50Hz rate of commands.
  
  a. Of these, the actual "loop" begins after while(g_run), since before that, it just sets up a few positions.
  
  b. The critical line is 322, "if (now - web.stamp > cfg::WEB_TIMEOUT) web = Input{};" This one checks to see if the last command is now "outdated."
  
    I. So, basically, if you have a command so old that it now exceeds WEB_TIMEOUT, the rover will stop.
    
    II. The reason this is important is that if your commands are delayed beyond WEB_TIMEOUT, the rover will start, then stop, then start (and so on). This means the code is somewhat vulnerable to lag, but in this case, simply increase WEB_TIMEOUT.
    
  c. Now, the values from the "pad" (Xbox controller) and the "web" (the browser) are combined.
  
  d. Lines 328 and 329 are important: "if (autoOn && (std::fabs(pad.thr) > 0.3 || std::fabs(pad.str_analog) > 0.3 || pad.str_dir != 0)) g_autoCancel = true;" means that both the auto and the controlled (g.pad or g.web) commands cannot occur simultaneously, and so the latter supersedes the former.
  
  e. Then, we define vars for a bit.
  
  f. Then, the code writes how the controlled commands work. Given that I never tested this with the Xbox, it may not work fully, but the intention is that as you hold the stick and move forward, the rover accelerates; as you let go, it stops, and so on (this works on the web).
  
  g. Finally, we have the camera "turret" system, which works in the same way (but is way easier).
    // Note that the thing about these variables is that they are from -1 to 1.

  -------- To autonomy...
  a. With autonomy, the variables from above kind of get replaced. It does its own thing until either g.pad or g.web is manipulated, at which point autoCancel occurs.
  
  b. We also have the E-Stop; this sets thr to 0, which means nothing can move the rover when it has E-Stop turned on, not even g.pad or g.web.
  
  c. Then, you run solve(s, thr, cur), which starts moving the rover in the commanded manner.


  -------- Sync writes...
  
  a. Essentially, we're only sending packets of information (ie, the controls of what the rover needs to do) when the inputs change.
  
    I. What this means is, if I lose a packet of information, the rover just continues in its merry way as before.
    
    II. The compromise is "const bool refresh = now - lastSend > cfg::REFRESH;" which sends a slow refresh packet every 200 ms.
    
    III. If refresh doesn't send, the rover isn't receiving a signal; then REFRESH is not sent, and the rover stops.
    
    IV. This is done in the if loop.

  -------- Back to autonomy...
  a. The ultrasonic sensors are the eyes of the rover. Two of these are fixed (middle left side and back left), while one is moving and attached to the camera turret (front right at the furthest angle, straight forward at the other). Their position is read every five "ticks" (runs of the while loop) to see where they are.
  
  b. Then, the speeds of servo IDs 3 and 4 are read (these are the middle wheels), and their movement is used for odometry.


  --------- G-run stopping?
  a. If g_run ever stops, then the servos (by default) go back to the positions they stored at start-up (so next time, you start up with each servo for the swerve drive pointing straight forward). It is at this point that the camera turret is also moved back.

  --------- Way later, website work.
  a. So lines 1985-2017 make sure the Tailscale website (when you connect to it while the rover is on the home network) requires a password (the username doesn't matter, only the password does), thus making it safe; the Pi-hotspot-based Wi-Fi isn't protected because you need to be exceedingly close to the rover to actually control it. This works with an 8+ character password and makes sure that 8 failures for the password from one IP block it from accessing the website for 10 minutes.
  
  b. Measuring a large delay in the stream (the page sends a "key held" message to the Pi every 100ms, and if this doesn't get received for more than 0.3 s, the program determines there's lag) and reducing the camera feed quality to correct for the lag.
  
  c. Of course, the only thing the camera's doing is taking a frame and sending its processed version to the website while removing the old photo, giving the impression of a real-time video streamer.

  --------- The internet...
  a. Lines 1210-1269 control the rover's connection to the internet. Every 2s, the thread asks the network manager what connection is being used.
    
    I. If it's at home, connected to the home network, then it pings every 6s to check if it is still at home. If it is, then Tailscale continues, and the website at the IP address goes down. If it isn't, for more than three of these pings in a row, it disconnects to join its own hotspot. This is mode 1.
    
    II. Mode 2 is the hotspot. Now, Tailscale is down, and the website at the IP address is up.
    
    III. The "--home-retry" option needs to be enabled for the page to try to reconnect to mode 1 if the website has been inactive for 30s and the home network is actually in range. This requires the first condition because attempting reconnection takes roughly 30s.

  --------- The camera?
  // Rover.cpp doesn't actually control the camera. That's the entire reason we have rovercam.py, which is a child process (ie, the process it relies on to handle all the camera work and relay its JPEGs to a website as "prints", which is MJPEG). Much of the code for rover.cpp, however, is meant to set up the camera, rovercam.py (whilst not prioritizing it; ie, if the Pi gets overloaded for some reason, the video stream, NOT the driving, is the first thing to go), start the YOLO11n model, and clear rovercam as necessary.
  a. The last part is because, on every boot, the rover saves the stream to its own storage, in case the streaming stopped while driving (if the Pi got overloaded) and needs to be recovered.
  
  b. Based on the network the rover is connected to, different things happen; it sends and stores MJPEGs in a manner that reduces lag as much as possible.

  ---------- Back to autonomy!
  a. The rover's eyes are the HC-SR04 ultrasonic sensors (ie, the US-100 sensors in "HC-SR04" sensor mode).
    
    I. There's a bunch of work that went into these; the way these work is that the TRIG gets raised, sends out an ultrasonic signal (hence the name) that ideally reflects off an object and returns to the ultrasonic sensor to be measured, and you get an ECHO; you take the time between the TRIG being raised and the ECHO, use the speed of sound, divide by two (the ultrasonic signal traveled the distance to the object and BACK), and you have a value! However, the way this works is: if you don't get a signal, either you lost your connection to the ultrasonic sensors, or there's nothing there.
    // So, in order to fix this, we start with the assumption that everything is connected and stays connected (a reasonable assumption to make; even when I had to reassemble the drone after it broke after two backflips, all the wiring remained perfectly intact). This can get dangerous if it eventually does get disconnected, so we can check the status of the TRIG connection; if this wired connection is lost, the code notes it and stops. However, as there is no way to double-check ECHO unless you receive pings, this is still a possible point of failure.
    
    II. There are maximum times put in place; if it goes BEYOND this time, we simply assume no object is in the way (see above). Note that the three sensors are also triggered at slightly different times, so their echoes don't overlap (which would corrupt/alter the measured distance).

  b. Lines 1096-1116 set up the very narrow case where the rover has to fit inside a narrow door; it utilizes the measurements from the sensors and the rover's physical length to determine if it is possible to go through, and if not, it simply returns that it is impossible.
    
    I. It scores all possible arcs that head towards the "goal" and chooses one; then, it does this. Squeeze mode directs the rover straight forward or lets it reverse to approach in a different manner.

  c. In the case that the throttle is nonzero and the rover's not moving (this is used via the wheel ratio) for an extended period of time, the rover assumes an uncaught obstacle (ie, one not detected by the ultrasonics) was hit, and the rover marks it as a physical barrier and backs up to attempt a different arc.

### rovercam.py
  a. The IMX500 model needs to upload the neural network we downloaded (the YOLO11n) on boot onto the camera, so it takes a minute or so for the camera to set up. After this, the Pi's CPU leaves the IMX500 model to do the job of handling the camera, having already uploaded it there.
  
  b. We use a BGR888 format, which is what the IMX500 wants.
  
  c. Then we return the frame and its metadata; we copy this for reuse.
  
  d. We then need to work with the boxes. Each frame is analyzed, and get_outputs returns the top candidates (based on score order above a specific threshold previously outlined; this is something you should test for at different values). Because we're dealing with a COCO model (Common Objects in Context, we don't just have images of things; we have where they might be - so instead of a stock image of a mug, maybe a mug with coffee in it would be used to train the network), we have more accurate readings.
    
    I. We don't ever stop the video from running this model, though, hence the try/except. Then, we combine boxes with similar/the same titles (choosing the candidate with the better title based on the confidence score), smooth boxes (so that we're taking a "moving average" of the boxes instead of their exact position at any moment, which would be significantly more jittery), and remove all candidates below the threshold.
    
    II. The consequence of taking a "moving average" is that if we lose the object being tracked, it still stays for a bit (0.7ish seconds) before it is dropped; however, this is such a small amount of time that it doesn't seem to be laggy on our end.

  e. When we deal with the camera, the camera is upside down, so to make the stream right-side up, we need to rotate the image. We first scale it down as appropriate (since rotating a smaller image is WAYYYY easier to do, and way faster) based on the network the Pi is connected to, then rotate the image, and THEN draw the boxes.
  
  f. Finally, we store this final frame, as we send it to the website, to the Pi's own memory so it may be recovered later. Writing to the boot occurs after the stream, though, so since the SD card is used at different times, our stream doesn't get delayed much. This is then synced every 2 or so s.
  
  g. On a new boot, we change current to be boot_id so that a new file with the appropriate name (ie, boot-<time>.mjpg) is added to the file directory, older files are deleted, and our actual current.mjpg (since this becomes the most current model) starts recording.

## Second, setup:
This is TO BE UPDATED!
