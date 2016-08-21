import numpy as np


class ShittyClass:
    def __init__(self):
        self.speedG = []
        self.speedD = []
        self.maxG=0.0
        self.maxD=0.0
        self.VcG=[]
        self.VcD=[]
        self.VD=0.0
        self.VG=0.0
        self.KpG=0.125
        self.KpD=0.125
        self.AG=0
        self.AD=0
        self.done = False


    def doYourShit(self, serial):
        data = serial.rsplit("\t")
        if len(data) == 9:
            self.speedG.append(np.abs(float(data[2])))
            self.speedD.append(np.abs(float(data[3])))
            self.VcG.append(np.abs(float(data[4])))
            self.VcD.append(np.abs(float(data[5])))


    def computeThatShit(self):

        for i in self.speedG:
            if np.abs(i)>self.maxG:
                self.maxG=np.abs(i)

        for i in self.speedD:
            if np.abs(i)>self.maxD:
                self.maxD=np.abs(i)

        for i in self.VcG:
            if np.abs(i)>self.VG:
                self.VG=np.abs(i)

        for i in self.VcD:
            if np.abs(i)>self.VD:
                self.VD=np.abs(i)

        self.D_percG = self.maxG/(self.VG-300) 
        self.D_percD = self.maxD/(self.VD-300)
        print "DperG = "+str(self.D_percG)
        print "DperD = "+str(self.D_percD)

        if self.D_percG <= 1.0:
            self.KpG = self.KpG + self.KpG
        else:
            z2=1.0/(1.0 + ((3.14**2) / (np.log(self.D_percG)**2)))
            self.AG=self.KpG*z2

        if self.D_percD <= 1.0:
            self.KpD = self.KpD + self.KpD
        else:
            z2=1.0/(1.0 + ((3.14**2) / (np.log(self.D_percD)**2)))
            self.AD=self.KpD*z2

        if self.AG != 0 and self.AD != 0:
            self.done = True

    def resetYourShit(self):
        self.speedG = []
        self.speedD = []
        self.maxG=0.0
        self.maxD=0.0
        self.VcG=[]
        self.VcD=[]
        self.VD=0.0
        self.VG=0.0

    
       
        
            
    """def gimmeThoseKp(serial):
        global KpG
        global KpD
        data = serial.rsplit("\t")
        KpG = atof(data[0])
        KpD = atof(data[1])"""

    def computeThoseKp(self):
        self.KpG = self.AG/(0.707**2)
        self.KpD = self.AD/(0.707**2)

    def calculateTheZ(self):
        return np.sqrt(1.0/(1.0 + ((3.14**2) / (np.log(self.D_percG)**2))))
