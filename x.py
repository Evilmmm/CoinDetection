import json,requests, os, sys, random, time, errno, itertools, math
from PIL import Image, ImageDraw

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

for mem in range(0, 2):
	
	Id = safe_open("img.jpg", "w")
	name = "location=" + Id
	os.system(str("gst-launch-1.0 v4l2src num-buffers=10 ! video/x-raw,format=NV12,width=640,height=480 ! jpegenc ! multifilesink " + name)) 
	
	
	model_id = '1aeb73ab-f09d-4be9-b4a3-739023c493cf'
	api_key = 'OsLUkksD-2QdPaGLMNRzwRlm6OXvBqX7'
	
	url = 'https://app.nanonets.com/api/v2/ObjectDetection/Model/' + model_id + '/LabelFile/'
	
	#data = {'file': open('img.jpg', 'rb'), 'modelId': ('', model_id)}
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
	    angle = math.atan((9 + (9/480)*abs(pix_y-480))/(1.5 - (0.25*(abs(pix_y-480)/480)) + (10.5/640)*pix_x - 0.5))*180/math.pi
	    distance = math.sqrt((9 + (9/480)*abs(pix_y-480))**2 + (1.5 - (0.25*(abs(pix_y-480)/480)) + (10.5/640)*pix_x - 0.5)**2)
	    coin_locs.append([distance,angle])
	    draw.rectangle((i["xmin"],i["ymin"], i["xmax"],i["ymax"]), fill=(random.randint(1, 255),random.randint(1, 255),random.randint(1, 255),127))
	
	#Save processed file
	Id2 = safe_open(str("Processed_" + Id), "w")
	im.save(Id2) 
	
	with open(Id2 + ".txt", "w") as text_file:
	    text_file.write(str(textResp) + str(coin_locs))


