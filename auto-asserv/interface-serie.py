from serial import Serial, SerialTimeoutException, SerialException
from asserv2 import *
import matplotlib.pyplot as plt
import time


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
        a=self.port_serie.readline() 
        #print a
        return a
    def attendre(self):
        return self.port_serie.inWaiting()

    def parler(self, a_envoyer):
        #raw_input("écrire ici")
        self.port_serie.write(a_envoyer+"\r\n")
        return a_envoyer

    def quitter(self):
        self.port_serie.close()

com=serialCom("COM6")
print("--------------------------\n\n")
m=""
a=""

while(m!='exit'):
    
    m=raw_input("ordre : ")
    serialCom.parler(com, m)
    if(m=="testSpeed"):

        speedG=[]
        speedD=[]
        VcD=[]
        VcG=[]
        pwmD=[]
        pwmG=[]
        
        time.sleep(5)
        
        while(serialCom.attendre(com)>0):
            a=serialCom.ecouter(com)
            data = a.rsplit("\t")
            if len(data) == 9:
                speedG.append(np.abs(float(data[2])))
                speedD.append(np.abs(float(data[3])))
                VcG.append(np.abs(float(data[4])))
                VcD.append(np.abs(float(data[5])))
                pwmG.append(float(data[6]))
                pwmD.append(float(data[7]))
                
        T= np.linspace(0, 1.5, len(speedG))
        plt.plot(T, speedG, 'r')
        plt.plot(T, speedD, 'k')
        plt.plot(T, VcG, 'b')
        
        plt.autoscale()
        plt.show()
        plt.plot(T, pwmG, 'g')
        plt.show()
        plt.cla()


    elif (m=="testPosition"):
        trans=[]
        Vt=[]
        St=[]
        
        time.sleep(5)
        while(serialCom.attendre(com)>0):
            a=serialCom.ecouter(com)
            data = a.rsplit("\t")
            if len(data) == 7:
                trans.append(np.abs(float(data[0])))
                Vt.append(np.abs(float(data[1])))
                St.append(np.abs(float(data[2])))
                
        T= np.linspace(0, 1.5, len(trans))
        plt.plot(T, trans, 'r')
        plt.plot(T, Vt, 'k')
        plt.plot(T, St, 'b')
        
        plt.autoscale()
        plt.show()
        plt.cla()
        
    elif (m=="testRotation"):
        trans=[]
        Vt=[]
        St=[]
        
        time.sleep(5)
        while(serialCom.attendre(com)>0):
            a=serialCom.ecouter(com)
            data = a.rsplit("\t")
            if len(data) == 7:
                trans.append(np.abs(float(data[3])))
                Vt.append(np.abs(float(data[4])))
                St.append(np.abs(float(data[5])))
                
        T= np.linspace(0, 1.5, len(trans))
        plt.plot(T, trans, 'r')
        plt.plot(T, Vt, 'k')
        plt.plot(T, St, 'b')
        
        plt.autoscale()
        plt.show()
        plt.cla()
        
    else:
        time.sleep(0.1)
        while(serialCom.attendre(com)>0):
            a+=serialCom.ecouter(com)

        print a        
    a=""
    







    
