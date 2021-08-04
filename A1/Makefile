allocate: main.o  processorSchedularFunctions.o Schedualer.o challangeScheduler.o
	gcc -o allocate main.o processorSchedularFunctions.o Schedualer.o challangeScheduler.o -lm -g

main.o: main.c
	gcc -c -Wall main.c -lm -g

Schedualer.o: Schedualer.c
	gcc -c -Wall Schedualer.c -lm -g

processorSchedularFunctions.o: processorSchedularFunctions.c
	gcc -c -Wall processorSchedularFunctions.c -lm -g

challangeScheduler.o: challangeScheduler.c
	gcc -c -Wall challangeScheduler.c -lm -g
	
clean:
	rm -rf *.o allocate
