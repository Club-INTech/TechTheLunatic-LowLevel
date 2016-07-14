# -*-coding:utf-8-*-
__author__ = "Spraaforskaren"
"""
Script utilisé pour obtenir la consigne en vitesse, la vitesse, la réponse du vitesse afin d'en tirer des
mesures caractéristiques pour l'asservissement d'un système.
"""
from serial import Serial, SerialTimeoutException, SerialException
import matplotlib.pyplot as plt
import time
import numpy as np


class Parametrage:
    def __init__(self):
        self.initialiser()
        self.KpG = 0.125
        self.KpD = 0.125
        self.AG = 0
        self.AD = 0
        self.done = False
    def initialiser(self):
        self.speedG = []
        self.speedD = []
        self.maxG = 0.0
        self.maxD = 0.0
        self.VcG = []
        self.VcD = []
        self.VD = 0.0
        self.VG = 0.0
    def mesurer_vitesses(self, serial):
        data = serial.rsplit("\t")
        if len(data) == 9:
            self.speedG.append(np.abs(float(data[2])))
            self.speedD.append(np.abs(float(data[3])))
            self.VcG.append(np.abs(float(data[4])))
            self.VcD.append(np.abs(float(data[5])))
        else:
            print "mauvaise mesure"

    def calculer_constantes(self):
        for i in self.speedG:
            if np.abs(i) > self.maxG:
                self.maxG = np.abs(i)

        for i in self.speedD:
            if np.abs(i) > self.maxD:
                self.maxD = np.abs(i)

        for i in self.VcG:
            if np.abs(i) > self.VG:
                self.VG = np.abs(i)

        for i in self.VcD:
            if np.abs(i) > self.VD:
                self.VD = np.abs(i)

        self.D_percG = self.maxG/(self.VG-300)
        self.D_percD = self.maxD/(self.VD-300)
        print "DperG = "+str(self.D_percG)
        print "DperD = "+str(self.D_percD)

        if self.D_percG <= 1.0:
            self.KpG = self.KpG + self.KpG
        else:
            z2 = 1.0/(1.0 + ((3.14**2) / (np.log(self.D_percG)**2)))
            self.AG = self.KpG*z2

        if self.D_percD <= 1.0:
            self.KpD = self.KpD + self.KpD
        else:
            z2=1.0/(1.0 + ((3.14**2) / (np.log(self.D_percD)**2)))
            self.AD = self.KpD*z2

        if self.AG != 0 and self.AD != 0:
            self.done = True

    def calculer_Kp(self):
        self.KpG = self.AG/(0.707**2)
        self.KpD = self.AD/(0.707**2)

    def caculer_Z(self):
        return np.sqrt(1.0/(1.0 + ((3.14**2) / (np.log(self.D_percG)**2))))

class CommunicationSerie:
    def __init__(self, serialPort):
        self.port = serialPort
        try:
            self.port_serie = Serial(port=self.port, baudrate=115200, timeout=0)
            print self.port_serie.isOpen()
        except SerialException:
            print "impossible d'ouvrir le port : " + str(serialPort)

    def ping(self):
        self.port_serie.write("?\r\n")

    def ecouter(self):
        a = self.port_serie.readline()
        #print a
        return a

    def parler(self, a_envoyer):
        self.port_serie.write(a_envoyer+"\r\n")
        return a_envoyer

    def attendre(self):
        return self.port_serie.inWaiting()

    def quitter(self):
        self.port_serie.close()

if __name__ == "__main__":
    com = CommunicationSerie("COM5")
    print com
    print("--------------------------\n\n")
    m = ""
    a = ""
    while m != "exit":
        m = raw_input("ordre : ")
        com.parler(m)
        if m in ("testSpeed", "testSpeedReverse"):
            speedG = []
            speedD = []
            VcD = []
            VcG = []
            pwmD = []
            pwmG = []
            time.sleep(5)
            while(com.attendre() > 0):
                a = com.ecouter()
                data = a.rsplit("\t")
                if len(data) == 9:
                    speedG.append(np.abs(float(data[2])))
                    speedD.append(np.abs(float(data[3])))
                    VcG.append(np.abs(float(data[4])))
                    VcD.append(np.abs(float(data[5])))
                    pwmG.append(float(data[6]))
                    pwmD.append(float(data[7]))
            T = np.linspace(0, 1.5, len(speedG))
            plt.plot(T, speedG, 'r')
            plt.plot(T, speedD, 'k')
            plt.plot(T, VcG, 'b')
            plt.autoscale()
            plt.show()
            plt.plot(T, pwmG, 'g')
            plt.show()
            plt.cla()
            with open("donnees_vitesse.txt", "w") as fichier:
                for t in range(len(T)):
                    fichier.write(str(T[t])+"\t"+str(speedG[t])+"\t"+str(VcG[t])+"\t"+str(pwmG[t])+"\n")
        elif m == "exit":
            break
        else:
            time.sleep(0.1)
            while(com.attendre() > 0):
                a += com.ecouter()
            print a

        a = ""
        
