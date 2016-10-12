from threading import Thread
from serialConnexion import *
from Tkinter import *
from time import *

debugHeaderCode = [0x02, 0x20]
debugHeader = "".join([chr(c) for c in debugHeaderCode]) # Passage en string

positionHeaderCode = [0x12, 0x19]
positionHeader = "".join([chr(c) for c in positionHeaderCode])



class threadEcoute(Thread):
    def __init__(self, serie, debugLogs, generalLogs, realPosition):
        Thread.__init__(self)
        self.serie = serie
        self.debugLogs = debugLogs
        self.generalLogs = generalLogs


        print("--------------------------\n\n")

    def run(self):

        position = [0, 0, 0]
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

                    if (max(timestamps)-min(timestamps)<0.05): # Si les messages sont assez rapproches

                        realPosition = position
                        print "POSITION OK"

            else:

                self.generalLogs.config(state=NORMAL)
                self.generalLogs.insert(END, a[:-2]+"\n")
                self.generalLogs.config(state=DISABLED)
                self.generalLogs.see("end")


    def stop(self):
        self.Terminated = True
