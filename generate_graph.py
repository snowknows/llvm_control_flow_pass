import sys
import os

def generateFile(filename):
	handleTxT(filename) 
	os.system("dot -Tpng " + filename.split(".txt")[0] + ".dot -o " + filename.split(".txt")[0] + ".png")

def handleTxT(filename):
	infile = open(filename)
	outfile = open(filename.split(".txt")[0] + ".dot", 'w')
	dlfile = open(filename.split(".txt")[0] + ".dl", 'w')
	funcCount = 0
	directcalldec = 0
	indirectcalldec = 0
	out = ""
	directcall = ""
	indirectcall = ""
	firstflag = False
	dlout = ""

	out += "digraph G {\n"

	for line in infile.readlines():
		if line == "\n":
			continue

		if line.find("%function name:") != -1:
			if firstflag:
				out += "\t}\n"
			else:
				firstflag = True
			out += "\tsubgraph cluster" + str(funcCount) + "{\n"
			out += "\t\tlabel = " + line.replace('\n','').split(':')[1] + ";\n";
			funcCount += 1
			dlout += "\n"
			continue

		if line.find("%direct call to:") != -1 :
			directcalldec = 2
			continue

		if line.find("%indirect call may to:") != -1:
			indirectcalldec = 2
			continue

		if directcalldec:
			node = line.replace('\n','').replace('./','').replace('/','_').replace('.','_').replace(':','_').split(',')
			directcall += "\t\t" + node[0] + " -> " + node[1] + ";\n"
			dlout += "succ(\"" + node[0] + "\",\"" + node[1] + "\").\n"
			directcalldec -= 1
			continue

		if indirectcalldec:
			node = line.replace('\n','').replace('./','').replace('/','_').replace('.','_').replace(':','_').split(',')
			indirectcall += "\t\t" + node[0] + " -> " + node[1] + " [style=dashed,color=red];\n"
			dlout += "succ(\"" + node[0] + "\",\"" + node[1] + "\").\n"
			indirectcalldec -= 1
			continue

		if line.find("start") != -1:
			node = line.replace('\n','').replace('./','').replace('/','_').replace('.','_').replace(':','_').split(',')
			directcall += "\t\t" + node[0] + " -> " + node[1] + ";\n"
			dlout += "succ(\"" + node[0] + "\",\"" + node[1] + "\").\n"
			continue

		node = line.replace('\n','').replace('./','').replace('/','_').replace('.','_').replace(':','_').split(',')
		out += "\t\t" + node[0] + " -> " + node[1] + ";\n"
		dlout += "succ(\"" + node[0] + "\",\"" + node[1] + "\").\n"


	out += "\t}\n"

	out += directcall
	out += indirectcall
	out +="}"

	# print out
	outfile.writelines(out)
	dlfile.writelines(dlout)
	infile.close()
	outfile.close()
	dlfile.close()


if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "need file"
		sys.exit()
	generateFile(sys.argv[1])