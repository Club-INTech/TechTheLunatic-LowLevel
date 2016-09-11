from serial import Serial, SerialTimeoutException, SerialException
from Tkinter import *
from threadEcoute import *


try:
    serie = Serial(port="/dev/ttyUSB0", baudrate=115200, timeout=0)
    print ("serie OK")
except SerialException:
    print "impossible d'ouvrir le port serie ttyUSB0"
    raw_input()
    exit()



fenetre = Tk()

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
us = LabelFrame(state, text="Ultra Sons :")
us.pack(side=LEFT, expand="yes", fill="both")


Label(position, text="x : ").pack()
Label(position, text="y : ").pack()

Label(orientation, text=" ... rad ").pack()

Label(us, text="Avant gauche : ").pack()
Label(us, text="Avant droit : ").pack()
Label(us, text="Arriere gauche : ").pack()
Label(us, text="Arriere droit : ").pack()



saisieBlock = PanedWindow(gauche, orient=HORIZONTAL, height=1)
saisieBlock.pack(fill="both", expand="no", side=BOTTOM)

saisie = Entry(saisieBlock)
saisie.pack(fill="both", expand="yes", side=LEFT)

envoyer = Button(saisieBlock, text="Envoyer", command=serie.write("?\r\n"))
envoyer.pack(fill="both", expand="no", side=RIGHT)

generalZone = LabelFrame(gauche, text="Vue Haut Niveau", padx=5, pady=5)
generalZone.pack(fill="both", expand="yes")

debugZone = LabelFrame(droite, text="Logs de Debug", padx=5, pady=5)
debugZone.pack(fill="both", expand="yes", side=TOP)

generalLogs = Text(generalZone, background="black", foreground="white")
generalLogs.pack(fill="both", expand="yes")

debugLogs = Text(debugZone,  background="black", foreground="white")
debugLogs.pack(fill="both", expand="yes")


def alert():
    showinfo("alerte", "Bravo!")



menubar = Menu(fenetre)

menu1 = Menu(menubar, tearoff=0)
menu1.add_command(label="Creer", command=alert)
menu1.add_command(label="Editer", command=alert)
menu1.add_separator()
menu1.add_command(label="Quitter", command=fenetre.quit)
menubar.add_cascade(label="Fichier", menu=menu1)


menu3 = Menu(menubar, tearoff=0)
menu3.add_command(label="A propos", command=alert)
menubar.add_cascade(label="Aide", menu=menu3)

fenetre.config(menu=menubar)



bouton=Button(droite, text="Fermer", command=fenetre.quit)
bouton.pack(side=BOTTOM)









ecoute = threadEcoute(serie, debugLogs, generalLogs)

ecoute.start()




fenetre.mainloop()