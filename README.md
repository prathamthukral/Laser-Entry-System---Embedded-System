# Laser_Entry_System
Hardware system of 2 lasers (paired with 2 photodiodes) that detects movement through a barrier.

Needs to be paired with a raspberry pi and linux virtual machine.

1. First needs to be compiled on the linux virtual machine and uploaded to a server.
2. Then raspberry pi needs to be setup and connected to the internet so that it can access the server.
3. Raspberry pi will then get the files from the server and save it locally to its own SD card.
4. Setup the circuitry.
5. Run program.

Features:
- Ability to run indefinitely as soon as the raspberry pi is logged into
- Ability to get configuration details from a .cfg file
- Ability to log errors and statistics into separate log files
- Utilizes a watchdog that catches an error and exits out of the program and the pi itself
