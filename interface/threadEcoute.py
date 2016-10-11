from threading import Thread
from serialConnexion import *
from Tkinter import *

debugHeaderCode = [0x02, 0x20]
debugHeader = "".join([chr(c) for c in debugHeaderCode]) # Passage en string

positionXHeaderCode = [0x03, 0x013]
positionXHeader = "".join([chr(c) for c in positionXHeaderCode])

positionYHeaderCode = [0x03, 0x013]
positionXHeader = "".join([chr(c) for c in positionXHeaderCode])

class threadEcoute(Thread):
    def __init__(self, serie, debugLogs, generalLogs, position, timestamps):
        Thread.__init__(self)
        self.serie = serie
        self.debugLogs = debugLogs
        self.generalLogs = generalLogs
        self.position=position
        self.timestamps=timestamps
        print("--------------------------\n\n")

    def run(self):

        while (True):

            a = ""

            while (True):
                if self.serie.in_waiting == 0:
                    time.sleep(0.1)
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

                if (time-timestamp.max() > 0.05):
                    self.position[0] =


            else:

                self.generalLogs.config(state=NORMAL)
                self.generalLogs.insert(END, a[:-2]+"\n")
                self.generalLogs.config(state=DISABLED)
                self.generalLogs.see("end")


    def stop(self):
        self.Terminated = True
