
import json,requests, os, sys, random, time, errno, itertools, math, serial
from PIL import Image, ImageDraw


lines = [[-60,3000]
,[19.2,-1958.4]
,[8.275862069,-1357.241379]
,[5.217391304,-1158.26087]
,[3.75,-1095]
,[2.774566474,-1032.138728]
,[2.142857143,-951.4285714]
,[1.785714286,-942.8571429]
]

angles = [15,10,5,0,-5,-10,-15,-20]

ser = serial.Serial(
    port = '/dev/ttyUSB0',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

def iter_incrementing_file_names(path):
    yield path
    prefix, ext = os.path.splitext(path)
    for i in itertools.count(start=1, step=1):
        yield prefix + str(i) + ext


def safe_open(path, mode):
    i = 0
    flags = os.O_CREAT | os.O_EXCL | os.O_WRONLY

    if 'b' in mode and platform.system() == 'Windows':
        flags |= os.O_BINARY

    for filename in iter_incrementing_file_names(path):
        try:
            file_handle = os.open(filename, flags)
        except OSError as e:
            if e.errno == errno.EEXIST:
                pass
            else:
                raise
        else:
            return(filename)

def distance(point,coef):
	return abs((coef[0]*point[0])-point[1]+coef[1])/math.sqrt((coef[0]*coef[0])+1)

def p2angle(pix_x,pix_y):
	dist_list = []
	x = 0
	
	for line in lines:
		dist_list.append([distance([pix_x,480-pix_y],line), x])
		x = x + 1

	dist_list.sort()
	return(angles[dist_list[0][1]])


def check_cam():
	Id = safe_open("img.jpg", "w")
	name = "location=" + Id
	os.system(str("gst-launch-1.0 v4l2src num-buffers=10 ! video/x-raw,format=NV12,width=640,height=480 ! jpegenc ! multifilesink " + name)) 
	
	model_id = '1aeb73ab-f09d-4be9-b4a3-739023c493cf'
	api_key = 'OsLUkksD-2QdPaGLMNRzwRlm6OXvBqX7'
	
	url = 'https://app.nanonets.com/api/v2/ObjectDetection/Model/' + model_id + '/LabelFile/'
	
	data = {'file': open(Id, 'rb'), 'modelId': ('', model_id)}
	
	response = requests.post(url, auth=requests.auth.HTTPBasicAuth(api_key, ''), files=data)
	
	textResp = response.text
	coin_locs = []
	
	response = json.loads(response.text)
	im = Image.open(Id)
	draw = ImageDraw.Draw(im, mode="RGBA")
	prediction = response["result"][0]["prediction"]
	for i in prediction:
	    pix_x = (i["xmin"]+i["xmax"])/2
	    pix_y = (i["ymin"]+i["ymax"])/2
	    #angle = math.atan((9 + (9/480)*abs(pix_y-480))/(1.5 - (0.25*(abs(pix_y-480)/480)) + (10.5/640)*pix_x - 0.5))*180/math.pi
	    #distance = math.sqrt((9 + (9/480)*abs(pix_y-480))**2 + (1.5 - (0.25*(abs(pix_y-480)/480)) + (10.5/640)*pix_x - 0.5)**2)
	    coin_locs.append(p2angle(pix_x,pix_y))
	    draw.rectangle((i["xmin"],i["ymin"], i["xmax"],i["ymax"]), fill=(random.randint(1, 255),random.randint(1, 255),random.randint(1, 255),127))
	
	#Save processed file
	Id2 = safe_open(str("Processed_" + Id), "w")
	im.save(Id2)
	
	coin_locs.sort()
	print(coin_locs) 
	
	with open(Id2 + ".txt", "w") as text_file:
		text_file.write(str(textResp) + str(coin_locs))
		
	try:
		mr = round(coin_locs[0])
	except IndexError:
		return 666
	else:
		return mr
		
	
def module():
	print("Waiting for `check`")

	x = ser.readline()
	while (x.find("check") == -1):
		print('\r' + x)
		x = ser.readline()
	print("Recieved `check`, analyzing cam now")

	temp1 = check_cam()
	print("Checked: " + str(temp1))

	if temp1 == 666:
		ser.flushInput()
		ser.flushOutput()
		ser.write('666' + '\r\n')
		ser.flushInput()
		ser.flushOutput()
		return 0
	else:
		ser.flushInput()
		ser.flushOutput()
		ser.write(str(temp1) + '\r\n')
		ser.flushInput()
		ser.flushOutput()

	
	print("Waiting for `confirm`")
	while (x.find("confirm") == -1):
		print('\r' + x)
		x = ser.readline()
	
	print("Recieved `confirm`")

	temp = check_cam()
	print("Confirmed: " + str(temp))

	if temp != 666:
		if abs(temp) < 11:
			print("Confirmed gold!")
			ser.flushInput()
			ser.flushOutput()
			ser.write('222' + '\r\n')
			ser.flushInput()
			ser.flushOutput()
		else:
			print("Not quite, try again")
			ser.flushInput()
			ser.flushOutput()
			ser.write('420' + '\r\n')
			ser.flushInput()
			ser.flushOutput()
	else:
		ser.flushInput()
		ser.flushOutput()
		ser.write('666' + '\r\n')
		ser.flushInput()
		ser.flushOutput()
		return 0


print("Waiting for `on`")
x = ser.readline()	
while (x.find("on") == -1):
	print("\r" + x)
	x = ser.readline()

ser.flushInput()
ser.flushOutput()
ser.write('111'+'\r\n')
ser.flushInput()
ser.flushOutput()

print("Sent `111` to confirm recieved `on`")

while True:
	module()

