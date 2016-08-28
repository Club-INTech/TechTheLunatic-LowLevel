from threading import Thread
from serialConnexion import *


class threadEcoute(Thread):



    def __init__(self, com):
        Thread.__init__(self)
        self.com = com
        print("--------------------------\n\n")

    def run(self):

        while (True):

            a = ""

            while (True):
                if(serialCom.attendre(self.com)==0):
                    time.sleep(0.02)
                else:
                    a += serialCom.ecouter(self.com)
                    if a.find("\r\n")!=-1:
                        break

            if(a != ""):
                print a[:len(a)-2]

    def stop(self):
        self.Terminated = True