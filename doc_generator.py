# Code du generateur de doc bas niveau

import re

main_file = open("projects/complete_low_level/main.cpp", "r")
doc = open("documentation.txt", "w")

doc.write("DOCUMENTATION DU BAS NIVEAU :\n\n")

lines = main_file.read().splitlines()

template = "!strcmp"

balise = "/*			 _"

orders = []
comments = []

for i in range(len(lines)):
    if template in lines[i]:

        orders.append(re.findall("\"(.*)\"" ,lines[i]))
        comments.append(re.findall("\/\/(.*)", lines[i]))
        if len(comments[len(comments)-1]) != 0:
            doc.write(orders[len(orders)-1][0] + "   " + (10 - len(orders[len(orders)-1][0])) * " " + comments[len(orders)-1][0] + "\n")
        else:
            doc.write(orders[len(orders)-1][0] + "   " + (10 - len(orders[len(orders)-1][0])) * " " + "Documentation non fournie." + "\n")

    else :
        if balise in lines[i]:

            doc.write("\n\n")
            for j in range(4):
                doc.write(lines[i+j][2:])
                doc.write("\n")
            doc.write("\n\n\n")

main_file.close()
doc.close()

