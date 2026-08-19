## Printing Instructions

PLEASE PLEASE PLEASE make it easy on yourself and use the 3MF **NOT** the STEP files for actually uploading onto a slicer.
Whenever you load the part, make sure that if you get the following screen, you click **yes**:
<img width="667" height="191" alt="image" src="https://github.com/user-attachments/assets/e115a54a-08e0-41dc-8084-4b1005eced47" />

If you look at the naming convention for each 3MF files, it is clearly explained what settings are recommended (IE, ASA_#9_20%_3d-cubic\_4walls should be printed in ASA at 20% infill in a cubic sparse infill pattern with 4 walls). Now, this does get more complicated with the TPU. The TPU 3MF files include simple cylinders positioned around holes. These cylinders are modifiers. ***While the rest of the print is to be at 15% gyroid infill with 2 walls, these modifiers should be changed to SIX walls.*** This is VERY crucial. Heat Set Inserts are very annoying with TPU (as I've heard), and for them to be properly fastened/secured, the TPU must be made rigid. To save your time and filament whilst making heat-sets viable, these modifiers are meant to circle each cylindrical hole, providing enough material for the heat-set inserts to properly bite into the thermoplastic. If everything goes right, you should expect around 545g worth of filament being used for TPU\_1. Keep supports on for all prints.

### Adding Modifiers to a Print:
<img width="1106" height="973" alt="image" src="https://github.com/user-attachments/assets/ca8a8dc9-9583-45ff-8c00-9e97710b48b9" />

Note that the provided settings are the basic settings. If you have other settings for supports, etc, you may use those (as those do not impact strength).

IN TOTAL YOU MAY EXPECT: 817.79g TPU being used (this is with supports) over 51.64 hours of printing. 1099.53g ASA being used (this is also with supports) over 56.05 hours of printing, for a total of around 107.69 hours of printing.

Tolerances are in-built into the prints, so long as everything is printed in the right material (ASA or TPU), as outlined. Remember, any other materials would either be more expensive alternatives, or less viable ones (ie, TPU is the best filament here because it is relatively common, flexible, but has a high temperature deflection point relative to the use-case the rover is ideal for, and ASA is the best rigid common filament with another reasonable temperature deflection point). You can see the overall assembly at this [link](https://cad.onshape.com/documents/4276ee27674d5cd4437829d8/w/b68ed14a30814a824c43fb7e/e/30365733099079a1b69868d1?renderMode=0&uiState=6a78f4664b6102bbb3720076). navigate to "The Spyder-Crab", and then the assembly of the same name), and simply hide the relevant objects to see precisely where everything goes.

Since there's so many parts (600 by my estimation), I will NOT be going over how each of the parts fit, although I believe most of it is obvious: in general, holes of diameter 4mm have a heat-set insert, holes of diamter 3mm allow for M3 screws to be passed, and so on. There are other specific cut-outs for flanged ball bearings (and so on for other parts, such as the rod-end).

Photos of the print plates:
### ASA #1 AND #2 (remember, these are the EXACT same plate, but one of the two needs to be printed twice or **BOTH NEED TO BE PRINTED ONCE EACH**):
<img width="1244" height="896" alt="image" src="https://github.com/user-attachments/assets/08ebd970-02eb-41a4-93cd-d96e18a13cee" />

### ASA #3:
<img width="1244" height="894" alt="image" src="https://github.com/user-attachments/assets/d3de6fe3-866b-45ea-9991-0b0dbd7e6416" />

### ASA #4:
<img width="1243" height="893" alt="image" src="https://github.com/user-attachments/assets/46add675-29fc-4e27-9f84-cb1ffedf5f87" />

### ASA #5:
<img width="1241" height="892" alt="image" src="https://github.com/user-attachments/assets/67b5da37-e100-4fc7-bb83-c7ed83b9a324" />

### ASA #6:
<img width="1242" height="893" alt="image" src="https://github.com/user-attachments/assets/4847d1ab-3d46-4a86-9e59-cb7fdb3b1552" />

### ASA #7:
<img width="1242" height="894" alt="image" src="https://github.com/user-attachments/assets/5081a057-bce0-4d7d-8b62-180470534e7f" />

### ASA #8:
<img width="1246" height="895" alt="image" src="https://github.com/user-attachments/assets/9be6e14b-dd17-433c-8f76-f9eea6c6849e" />

### ASA #9:
<img width="1244" height="898" alt="image" src="https://github.com/user-attachments/assets/f58eb30d-d895-449a-a17a-9a40e6532cbb" />

### TPU #1:
<img width="1242" height="893" alt="image" src="https://github.com/user-attachments/assets/ec7b078e-4427-4112-866d-ffe64d4749db" />

### TPU #2:
<img width="1244" height="897" alt="image" src="https://github.com/user-attachments/assets/aadd1307-10e3-4340-9ba1-9510dddac69e" />

Eventually, I will make a complete tutorial for how the rover will be built (step-by-step), with photos (or a video).
