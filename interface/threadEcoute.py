from threading import Thread
from serialConnexion import *
from Tkinter import *
from time import *

debugHeaderCode = [0x02, 0x20]
debugHeader = "".join([chr(c) for c in debugHeaderCode]) # Passage en string

positionHeaderCode = [0x12, 0x19]
positionHeader = "".join([chr(c) for c in positionHeaderCode])

#speedHeaderCode = [0x11, 0x14]
speedHeader = "".join([chr(c) for c in positionHeaderCode])



class threadEcoute(Thread):
    def __init__(self, serie, debugLogs, generalLogs, realPosition, realSpeed, positionZone, positionX, positionY, orientationLabel, vitesseD, vitesseG):
        Thread.__init__(self)
        self.serie = serie
        self.debugLogs = debugLogs
        self.generalLogs = generalLogs
        self.realPosition = realPosition
        self.realSpeed = realSpeed
        self.positionZone = positionZone
        self.positionX = positionX
        self.positionY = positionY
        self.vitesseD = vitesseD
        self.vitesseG = vitesseG
        self.orientationLabel = orientationLabel


        print("--------------------------\n\n")

    def run(self):

        position = [0, 0, 0]
        speed = [0, 0]
        timestamps = [0, 0, 0]

        while (True):

            a = ""

            while (True):
                if self.serie.in_waiting == 0:
                    sleep(0.1)
                    # print("waiting for message")
                else:
                    a += self.serie.read()
                    if a.find("\r\n") != -1:
                        break

            if (a[0:2] == debugHeader):

                self.debugLogs.config(state=NORMAL)
                self.debugLogs.insert(END, a[2:-2]+"\n")
                self.debugLogs.config(state=DISABLED)
                self.debugLogs.see("end")

            elif (a[0:2] == positionHeader):

                    position.pop(0)
                    position.append(a[2:-2])
                    timestamps.pop(0)
                    timestamps.append(time())

                    if (max(timestamps)-min(timestamps)<0.005): # Si les messages sont assez rapproches

                        self.realPosition = position
                        self.positionX.config(text="x : " + str(self.realPosition[0]))
                        self.positionY.config(text="y : " + str(self.realPosition[1]))
                        self.orientationLabel.config(text="Orientation : " + str(self.realPosition[2]))

            elif (a[0:2] == speedHeader):
                    speed.pop(0)
                    speed.append(a[2:-2])
                    timestamps.pop(0)
                    timestamps.append(time())

                    if (max(timestamps) - min(timestamps) < 0.005):
                        self.realSpeed=speed
                        self.vitesseD.config(text="vd : "+str(self.realSpeed[0]))
                        self.vitesseG.config(text="vg : "+str(self.realSpeed[1]))



            else:

                self.generalLogs.config(state=NORMAL)
                self.generalLogs.insert(END, a[:-2]+"\n")
                self.generalLogs.config(state=DISABLED)
                self.generalLogs.see("end")


    def stop(self):
        self.Terminated = True
