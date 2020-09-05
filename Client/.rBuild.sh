#!/bin/bash

SOURCE_DIR=/Users/ilia/Documents/Development/HDMI-CEC-Emulator/Client/src
HOST=172.16.232.130

echo "Cleaning remote source directory..."
ssh builder@$HOST 'rm -rf ~/build/*'
echo "Copying sources..."
scp -r $SOURCE_DIR builder@$HOST:~/build/
echo "Building..."
ssh builder@$HOST 'cd ~/build/src && gcc hdmi-cec-emulator.c -o hdmi-cec-emulator -lX11 -lXtst && scp ./hdmi-cec-emulator root@192.168.0.1:~/.bin && echo "File transferred to target system."'
if [ $? -ne "0" ]; then
	echo; echo; echo; echo "!!! Build failed!"
else
	echo; echo; echo; echo "Build successful!"
fi
echo


