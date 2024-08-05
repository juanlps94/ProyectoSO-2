all: pf1

clean:	
	rm -rf pf1 pf1.o *.sorted 
	
pf1:   pf1.o

pf1.o: pf1.c
