from serial import *

class serialCom:
    def __init__(self, serialPort):
        self.port = serialPort
        try:
            self.port_serie = Serial(port=self.port, baudrate=115200, timeout=0)
            print self.port_serie.isOpen()
        except SerialException:
            print "impossible d'ouvrir le port : " + str(serialPort)
            raw_input()
            exit()

    def ping(self):

        self.port_serie.write("?\r\n")

    def ecouter(self):
        a = self.port_serie.readline()
        # print a
        return a

    def attendre(self):
        return self.port_serie.inWaiting()

    def parler(self, a_envoyer):
        # raw_input("ecrire ici")
        self.port_serie.write(a_envoyer + "\r\n")
        return a_envoyer

    def quitter(self):
        self.port_serie.close()