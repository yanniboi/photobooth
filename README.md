#Wedding Photobooth

The wedding photobooth is a C++ application that takes pictures from a DSLR shows them on a screen and lets you print them if you like what you see.

##What we need

Camera
* Computer and Screen
* Router
* Tablet
* Printer
* Lights
* Portable Hard Drive
* Props
* Accessories (cables and stuff)

##Camera
DSLR to take the pictures. The camera probably needs to be plugged into the mains, but could work off one charge if needed. It also needs to plug into the computer via usb (or other). For a list of compatible models see http://gphoto.sourceforge.net/proj/libgphoto2/support.php
i
##Computer
We need a small computer to run the C++ application. The computer basically interfaces with the camera and the printer and shows the pictures on a screen. The computer also saves the images to the harddrive before sending them to a printer to be printed.

##Router
The router is there to connect the computer and the tablet on a private network. The application on the computer has a web api that gets triggered using http requests. Basically there are 2 request endpoints. One to take a picture and one to print. The tablet makes those requests.

##Printer
The printer prints the pictures (simples!). Any printer will do really. But preferably one that can print to photo-paper (ie. has the right size paper compartment) is preferred. We need to make sure we have plenty of photo paper and spare ink cartridges.

##Lights
Some basic studio lights to make sure the pictures come out well. These are optional if the Camera has a good enough lense to take pictures in low light, but we don’t want the pictures to be too dark. Studio lights are quite cheap (30 - 50 quid on ebay) and Terry McNamara has offered me some if we need them. Need power for the lights.

##Portable Hard Drive
To store pictures. The printed copies are obviously just jpegs, but DSLRs take high res RAW images that would be nice to keep but optionally we can just set the camera to only send lower resolution jpegs.

##Props
For dress up and funny pictures. People are more likely to take pictures if they can be a bit silly.

##Accessories (cables and stuff)
Power cables and extension leads
Usb cables
Network cable for router
Chargers for tablet and Camera
Tripod for Camera
Keyboard (for initial setup)
