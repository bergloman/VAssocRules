all: a.exe

a.exe: main.o datatable.o str.o itemsets.o prune.o rulesclassifier.o apriori.o assocrules.o foil.o clustering.o clusterfuncs.o classifiertest.o
	g++ -O2 main.o datatable.o str.o itemsets.o prune.o rulesclassifier.o apriori.o assocrules.o foil.o clustering.o clusterfuncs.o -o a.exe
	
assocrules.o: apriori.h stdvik.h rules.h rulesclassifier.h assocrules.h assocrules.cpp
	g++ -O2 -c assocrules.cpp
	
apriori.o: prune.h apriori.h apriori.cpp
	g++ -O2 -c apriori.cpp
	
itemsets.o: stdvik.h itemsets.h itemsets.cpp
	g++ -O2 -c itemsets.cpp
	
main.o: str.h ds.h lx.h binfilestream.h stdvik.h rules.h main.cpp
	g++ -O2 -c main.cpp

prune.o: itemsets.h prune.h prune.cpp
	g++ -O2 -c prune.cpp
	
rulesclassifier.o: stdvik.h rules.h rulesclassifier.h rulesclassifier.cpp
	g++ -O2 -c rulesclassifier.cpp
	
datatable.o: datatable.h lx.h str.h binfilestream.h datatable.cpp
	g++ -O2 -c datatable.cpp

str.o: str.h str.cpp
	g++ -O2 -c str.cpp

foil.o: foil.h foil.cpp
	g++ -O2 -c foil.cpp

clustering.o: clustering.h clustering.cpp
	g++ -O2 -c clustering.cpp
	
clusterfuncs.o: clusterfuncs.h clusterfuncs.cpp
	g++ -O2 -c clusterfuncs.cpp

classifiertest.o: classifiertest.h classifiertest.cpp
	g++ -O2 -c classifiertest.cpp
	
clean:
	rm -rf *o hello