all:
	gcc serverM.c -o serverM
	gcc serverA.c -o serverA
	gcc serverR.c -o serverR
	gcc serverD.c -o serverD
	gcc client.c -o client

clean:
	rm serverM
	rm serverA
	rm serverR
	rm serverD
	rm client