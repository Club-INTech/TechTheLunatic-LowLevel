from serial import Serial, SerialTimeoutException, SerialException
from Tkinter import *
from matplotlib import pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
from tkMessageBox import showinfo
from numpy import arange, exp

from threadEcoute import *


try:
    serie = Serial(port="/dev/ttyUSB1", baudrate=115200, timeout=0)
    print ("serie OK")


    fenetre = Tk()
    fenetre.wm_title("Interface serie")

    label = Label(fenetre, text="INTerface")
    label.pack()

    gauche = PanedWindow(fenetre, orient=VERTICAL)
    gauche.pack(side=LEFT, expand="yes", fill=BOTH, pady=2, padx=2)

    droite = PanedWindow(fenetre, orient=VERTICAL)
    droite.pack(side=RIGHT, expand="yes", fill=BOTH, pady=2, padx=2)

    state = PanedWindow(gauche, orient=HORIZONTAL)
    state.pack(fill="both", expand="no", side=TOP)

    position = LabelFrame(state, text="Position :")
    position.pack(side=LEFT, expand="yes", fill="both")
    orientation = LabelFrame(state, text="Orientation :")
    orientation.pack(side=LEFT, expand="yes", fill="both")
    vitesse = LabelFrame(state, text= "Vitesse :")
    vitesse.pack(side=LEFT, expand="yes", fill="both")
    us = LabelFrame(state, text="Ultra Sons :")
    us.pack(side=LEFT, expand="yes", fill="both")

    realPosition = [0, 0, 0]
    realSpeed = [0,0]

    positionX = Label(position, text="x : "+str(realPosition[0]))
    positionX.pack()
    positionY = Label(position, text="y : "+str(realPosition[1]))
    positionY.pack()
    vitesseDroite=Label(vitesse, text="d : "+str(realSpeed[0]))
    vitesseDroite.pack()
    vitesseGauche=Label(vitesse, text="g : "+str(realSpeed[1]))
    vitesseGauche.pack()
    orientationLabel = Label(orientation, text=str(realPosition[2])+" rad ")
    orientationLabel.pack()

    Label(us, text="Avant gauche : ").pack()
    Label(us, text="Avant droit : ").pack()
    Label(us, text="Arriere gauche : ").pack()
    Label(us, text="Arriere droit : ").pack()



    saisieBlock = PanedWindow(gauche, orient=HORIZONTAL, height=1)
    saisieBlock.pack(fill="both", expand="no", side=BOTTOM)

    saisie = Entry(saisieBlock)
    saisie.pack(fill="both", expand="yes", side=LEFT)


    def callback1(event):
        serie.write(str(saisie.get())+"\r\n")
        saisie.delete(0, 'end')


    def callback():
        serie.write(str(saisie.get()) + "\r\n")
        saisie.delete(0, 'end')



    envoyer = Button(saisieBlock, text="Envoyer", command= callback )
    envoyer.pack(fill="both", expand="no", side=RIGHT)

    saisie.bind("<Return>", callback1)

    generalZone = LabelFrame(gauche, text="Vue Haut Niveau", padx=5, pady=5)
    generalZone.pack(fill="both", expand="yes")

    debugZone = LabelFrame(droite, text="Logs de Debug", padx=5, pady=5)
    debugZone.pack(fill="both", expand="yes", side=TOP)

    generalLogs = Text(generalZone, background="black", foreground="white")
    generalLogs.pack(fill=BOTH, expand="yes")
    generalLogs.config(state=DISABLED)

    debugLogs = Text(debugZone,  background="black", foreground="white")
    debugLogs.pack(side=TOP, fill="both", expand="yes")
    debugLogs.config(state=NORMAL)

    def alert():
        showinfo("alerte", "Bravo!")



    menubar = Menu(fenetre)

    menu1 = Menu(menubar, tearoff=0)
    menu1.add_command(label="Creer", command=alert)
    menu1.add_command(label="Editer", command=alert)
    menu1.add_separator()
    menu1.add_command(label="Quitter", command=fenetre.destroy)
    menubar.add_cascade(label="Fichier", menu=menu1)


    menu3 = Menu(menubar, tearoff=0)
    menu3.add_command(label="A propos", command=alert)
    menubar.add_cascade(label="Aide", menu=menu3)

    fenetre.config(menu=menubar)

    bouton=Button(droite, text="Fermer", command=fenetre.destroy)
    bouton.pack(side=BOTTOM)

    ##Graph##
    label="Graphe"
    figure=Figure(figsize=(8,4),dpi=100)
    vitesseGauche=figure.add_subplot(111)
    t=arange(0.0,2.0,0.01)
    s=1-exp(-t)
    vitesseGauche.plot(t,s)
    vitesseGauche.grid()
    #canvas
    can=FigureCanvasTkAgg(figure, master=fenetre)
    can.show()
    can.get_tk_widget().pack(side=RIGHT, fill=BOTH, expand=1)



    ecoute = threadEcoute(serie, debugLogs, generalLogs, realPosition, realSpeed, position, positionX, positionY, vitesseDroite, vitesseGauche, orientationLabel)

    ecoute.start()

    fenetre.mainloop()

except SerialException:
    print "impossible d'ouvrir le port serie"
    raw_input()
    exit()