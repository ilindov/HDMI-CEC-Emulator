import os

os.system("LD_PRELOAD=/storage/.bin/libXi.so.6:/storage/.bin/libXtst.so.6 nohup /storage/.bin/hdmi-cec-emulator &>/dev/null &")
