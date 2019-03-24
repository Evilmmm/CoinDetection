import subprocess
import time

for x in range(0, 100):
	name = "location=image" + str(x) + ".jpg"
	subprocess.call(["gst-launch-1.0 v4l2src num-buffers=10 ! video/x-raw,format=NV12,width=640,height=480 ! jpegenc ! multifilesink", name]) 
	time.sleep(0.15)


