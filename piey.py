#Main file for C-Bot's Optical Brain
import json,requests, os, sys, random, time, errno, itertools, math, serial
from PIL import Image, ImageDraw
import RPi.GPIO as GPIO

buzzer_pin = 27

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


notes = {
	'B0' : 31,
	'C1' : 33, 'CS1' : 35,
	'D1' : 37, 'DS1' : 39,
	'EB1' : 39,
	'E1' : 41,
	'F1' : 44, 'FS1' : 46,
	'G1' : 49, 'GS1' : 52,
	'A1' : 55, 'AS1' : 58,
	'BB1' : 58,
	'B1' : 62,
	'C2' : 65, 'CS2' : 69,
	'D2' : 73, 'DS2' : 78,
	'EB2' : 78,
	'E2' : 82,
	'F2' : 87, 'FS2' : 93,
	'G2' : 98, 'GS2' : 104,
	'A2' : 110, 'AS2' : 117,
	'BB2' : 123,
	'B2' : 123,
	'C3' : 131, 'CS3' : 139,
	'D3' : 147, 'DS3' : 156,
	'EB3' : 156,
	'E3' : 165,
	'F3' : 175, 'FS3' : 185,
	'G3' : 196, 'GS3' : 208,
	'A3' : 220, 'AS3' : 233,
	'BB3' : 233,
	'B3' : 247,
	'C4' : 262, 'CS4' : 277,
	'D4' : 294, 'DS4' : 311,
	'EB4' : 311,
	'E4' : 330,
	'F4' : 349, 'FS4' : 370,
	'G4' : 392, 'GS4' : 415,
	'A4' : 440, 'AS4' : 466,
	'BB4' : 466,
	'B4' : 494,
	'C5' : 523, 'CS5' : 554,
	'D5' : 587, 'DS5' : 622,
	'EB5' : 622,
	'E5' : 659,
	'F5' : 698, 'FS5' : 740,
	'G5' : 784, 'GS5' : 831,
	'A5' : 880, 'AS5' : 932,
	'BB5' : 932,
	'B5' : 988,
	'C6' : 1047, 'CS6' : 1109,
	'D6' : 1175, 'DS6' : 1245,
	'EB6' : 1245,
	'E6' : 1319,
	'F6' : 1397, 'FS6' : 1480,
	'G6' : 1568, 'GS6' : 1661,
	'A6' : 1760, 'AS6' : 1865,
	'BB6' : 1865,
	'B6' : 1976,
	'C7' : 2093, 'CS7' : 2217,
	'D7' : 2349, 'DS7' : 2489,
	'EB7' : 2489,
	'E7' : 2637,
	'F7' : 2794, 'FS7' : 2960,
	'G7' : 3136, 'GS7' : 3322,
	'A7' : 3520, 'AS7' : 3729,
	'BB7' : 3729,
	'B7' : 3951,
	'C8' : 4186, 'CS8' : 4435,
	'D8' : 4699, 'DS8' : 4978
}



melody = [
  notes['E7'], notes['E7'], 0, notes['E7'],
  0, notes['C7'], notes['E7'], 0,
  notes['G7'], 0, 0,  0,
  notes['G6'], 0, 0, 0,
 
  notes['C7'], 0, 0, notes['G6'],
  0, 0, notes['E6'], 0,
  0, notes['A6'], 0, notes['B6'],
  0, notes['AS6'], notes['A6'], 0,
 
  notes['G6'], notes['E7'], notes['G7'],
  notes['A7'], 0, notes['F7'], notes['G7'],
  0, notes['E7'], 0, notes['C7'],
  notes['D7'], notes['B6'], 0, 0,
 
  notes['C7'], 0, 0, notes['G6'],
  0, 0, notes['E6'], 0,
  0, notes['A6'], 0, notes['B6'],
  0, notes['AS6'], notes['A6'], 0,
 
  notes['G6'], notes['E7'], notes['G7'],
  notes['A7'], 0, notes['F7'], notes['G7'],
  0, notes['E7'], 0, notes['C7'],
  notes['D7'], notes['B6'], 0, 0
]
tempo = [
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
 
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
 
  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
 
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
 
  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
]

def buzz(frequency, length):	 #create the function "buzz" and feed it the pitch and duration)

	if(frequency==0):
		time.sleep(length)
		return
	period = 1.0 / frequency 		 #in physics, the period (sec/cyc) is the inverse of the frequency (cyc/sec)
	delayValue = period / 2		 #calcuate the time for half of the wave
	numCycles = int(length * frequency)	 #the number of waves to produce is the duration times the frequency
	
	for i in range(numCycles):		#start a loop from 0 to the variable "cycles" calculated above
		GPIO.output(buzzer_pin, True)	 #set pin 27 to high
		time.sleep(delayValue)		#wait with pin 27 high
		GPIO.output(buzzer_pin, False)		#set pin 27 to low
		time.sleep(delayValue)		#wait with pin 27 low

def setup():
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(buzzer_pin, GPIO.IN)
	GPIO.setup(buzzer_pin, GPIO.OUT)
	
def destroy():
	GPIO.cleanup()				# Release resource
	

def play(melody,tempo,pause,pace=0.800):
	
	for i in range(0, len(melody)):		# Play song
		
		noteDuration = pace/tempo[i]
		buzz(melody[i],noteDuration)	# Change the frequency along the song note
		
		pauseBetweenNotes = noteDuration * pause
		time.sleep(pauseBetweenNotes)

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
	name = Id
	os.system(str("raspistill -w 640 -h 480 -o " + name)) 
	
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
		if i["score"] > 0.65:
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
	
coin_melody = [
	notes['B5'],notes['E6']
]

coin_tempo = [
	4,16
]

got_coin = 0
	
def module():
	
	
	print("Waiting for `check`")
	
	x = ser.readline()
	while (x.find("check") == -1):
		print('\r' + x)
		x = ser.readline()
	print("Recieved `check`, analyzing cam now")
	
	global got_coin
	if got_coin == 1:
		got_coin = 0
		play(coin_melody, coin_tempo, 0.30, 1.2000)
		
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
	
	print("Received `confirm`")

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
			got_coin = 1
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

print("Sent `111` to confirm received `on`")

while True:
	module()

