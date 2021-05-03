all: chef saladmaker

chef: chef.o 
	g++ chef.o -o chef -lpthread

saladmaker: saladmaker.o 
	g++ saladmaker.o  -o saladmaker -lpthread

saladmaker.o: saladmaker.cpp  
	g++ -c saladmaker.cpp -lpthread

chef.o: chef.cpp 
	g++ -c chef.cpp -lpthread

clean:
	rm *.o chef  saladmaker


