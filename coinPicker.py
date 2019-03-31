import os
import glob
import time
import serial
import RPi.GPIO as GPIO
from bluetooth import *

#Initialize Bluetooth communication ports
server_sock=BluetoothSocket( RFCOMM )
server_sock.bind(("",PORT_ANY))
server_sock.listen(1)

port = server_sock.getsockname()[1]

uuid = "87eafe2d-5afb-4429-a08c-28dc32677ccd"

advertise_service( server_sock, "CoinPicker",
                   service_id = uuid,
                   service_classes = [ uuid, SERIAL_PORT_CLASS ],
                   profiles = [ SERIAL_PORT_PROFILE ], 
#                   protocols = [ OBEX_UUID ] 
                    )

ser = serial.Serial(
    port = '/dev/ttyUSB0',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

send_string = ""
motor_stop_counter = 0
motor_started = 0
send_flag = 0


while True:          
	print "Waiting for connection on RFCOMM channel %d" % port

	client_sock, client_info = server_sock.accept()
	print "Accepted connection from ", client_info

	try:
	        data = client_sock.recv(1024)
        	if len(data) == 0: break
	        print "received [%s]" % data

		if data == '102':
			print "upPushed"
			ser.write('102\n')
		elif data == '103':
			print "downPushed"
			ser.write('103\n')
		elif data == '100':
			print "leftPushed"
			ser.write('100\n')
		elif data == '101':
			print "rightPushed"
			ser.write('101\n')
		elif data == '104':
			print "stopPushed"
			ser.write('104\n')
                elif data == '105':
			print "pickupPushed"
			ser.write('105\n')
		else:
			print "WTF!" 
	        #client_sock.send(data)
		#print "sending [%s]" % data

	except IOError:
		pass

	except KeyboardInterrupt:

		print "disconnected"

		client_sock.close()
		server_sock.close()
		print "all done"

		break
