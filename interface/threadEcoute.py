from threading import Thread
from serialConnexion import *
from Tkinter import *

debugHeader = [0x02, 0x20]


class threadEcoute(Thread):
    def __init__(self, serie, debugLogs, generalLogs):
        Thread.__init__(self)
        self.serie = serie
        self.debugLogs = debugLogs
        self.generalLogs = generalLogs
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

            if (a[0:4] == debugHeader):

                self.debugLogs.config(state=NORMAL)
                self.debugLogs.insert(END, a[4:-2]+"\n")
                self.debugLogs.config(state=DISABLED)
                self.debugLogs.see("end")


            else:
                # print a[:len(a)-2]
                print a[0:4]
                self.generalLogs.config(state=NORMAL)
                self.generalLogs.insert(END, a[:-2]+"\n")
                self.generalLogs.config(state=DISABLED)
                self.generalLogs.see("end")


    def stop(self):
        self.Terminated = True
