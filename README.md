# Yomuledim

This is the intergalactic yomuledim project, a talking teddy bear (AKA Nahadubi) which spits out all of Nahamu's top catchphrases.
With the help of many friends who served side by side with Nahamu, we collected and recorded, Nahamu's top 50 catchphrases and divided them to different catagories.


Nahadubi has three buttons:
  - Left hand button - Move one library 'forward', turn on the eyes according to the current library color, and trigger a random saying from the new catagory
  - Right hand button - Move one library 'backward', turn on the eyes according to the current library color, and trigger a random saying from the new catagory
  - Stomach - trigger a random saying from the current catagory

Power saving features:

  Since Nahadubi is running on batteries, power saving features were required
   - After 5 seconds of inactivity, the eyes will 'breath to death' and eventually turn off
   - After 60 seconds of inactivity, the arduino will activate sleep mode, and turning off all the unnecessery components.
   - Clicking on one of the hands, will send an interrupt and wake up Nahadubi and will stop his 'nimnum'
  
  
 Under 'utils' you can find two different utilities which will become handy when you will want to update the library and add/remove recordings.
 
 - header_file_generator.py: Generates a new header file according to a given folder full off recordings. The source folder needs to be divided into catagories (by folders). 
 - upload_records_by_order.py: Uploads the recordings from a source folder by their names (001_xxx.mp3 will be uploaded first, 002_xxx.mp3 will be second and so on). This is required since the mp3 player assigns the memory address of a mp3 file according to the order the files were uploaded. Example: the first file to be uploaded will be assigned 0x01 memory address, the tenth file will be assigned to 0x0A and so on.
