import sys, os, cv2

print(cv2.__version__)
cv2.setNumThreads(4)

#please copy ~/opencv-3.3.0/data/harrcascades_cuda/haarcascade_frontalface_alt.xml to /home/linaro/
faceCascade = cv2.CascadeClassifier("/home/linaro/haarcascade_frontalface_alt.xml")
cap = cv2.VideoCapture("v4l2src ! video/x-raw,format=NV12,width=640,height=480 ! videoconvert ! appsink")

while cap.isOpened():
    ret, frame = cap.read()
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = faceCascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5,
        minSize=(30, 30),
        flags=cv2.CASCADE_SCALE_IMAGE
    )
    for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
    cv2.imshow("Frame", frame)
    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()