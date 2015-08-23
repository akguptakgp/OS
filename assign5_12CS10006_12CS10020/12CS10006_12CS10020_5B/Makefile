compile: sem_creat.out sem_rm.out customer.out barber.out 
sem_creat.out: sem_creat.c
	cc -w -o sem_creat.out sem_creat.c
sem_rm.out: sem_rm.c
	cc -w -o sem_rm.out sem_rm.c
customer.out: customer.c
	cc -w -o customer.out customer.c
barber.out: barber.c
	cc -w -o barber.out barber.c
clean:
	rm *.out
init: 
	./sem_creat.out	
clear:
	./sem_rm.out						 


