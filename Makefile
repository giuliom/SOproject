#################################################
#
# Makefile progetto lso 2010
# (fram 1)(fram 2)(fram 3)
#################################################

# ***** DA COMPLETARE ******  con i file da consegnare
# primo frammento
FILE_DA_CONSEGNARE1=./genList.c ./genHash.c 

# secondo frammento
FILE_DA_CONSEGNARE2=./logpro 

# terzo frammento
FILE_DA_CONSEGNARE3=./msgserv.c ./msgcli.c ./comsock.h ./comsock.c ./clientut.h ./clientut.c ./serverut.h ./serverut.c ./utils.h ./RelazioneSOprogetto.pdf ./options


# Compiler flags
CFLAGS = -Wall -pedantic -g -lgcov
# aggiungere -lgcov o -fprofile-arcs al linking

# Compilatore
CC = gcc


# Librerie
LIBDIR = ../lib
LIBS = -L $(LIBDIR)

# Nome libreria
LIBNAME = libmsg.a

# Lista degli object files (** DA COMPLETARE ***)
OBJS = genList.o genHash.o

# nomi eseguibili test primo frammento
exe1 = msg_test1
exe2 = msg_test2


# phony targets
.PHONY: clean lib test11 test12 docu consegna1
.PHONY: test21 consegna2
.PHONY: test31 test32 test33 consegna3


# creazione libreria
lib:  $(OBJS)
	-rm  -f $(LIBDIR)/$(LIBNAME)
	ar -r $(LIBNAME) $(OBJS)
	cp $(LIBNAME) $(LIBDIR)

# eseguibile di test 1 (primo frammento)
$(exe1): genList.o test-genList.o
	$(CC) -o $@ $^ 

# eseguibile di test 2 (primo frammento)
$(exe2): $(OBJS) test-genHash.o
	$(CC) -o $@ $^ 

# dipendenze oggetto main di test 11
test-genList.o: test-genList.c genList.h
	$(CC) $(CFLAGS) -c $<

# dipendenze oggetto main di test 12
test-genHash.o: test-genHash.c genHash.h genList.h
	$(CC) $(CFLAGS) -c $<

# make rule per gli altri .o del primo frammento (***DA COMPLETARE***)





msgserv: msgserv.o comsock.o serverut.o 
	$(CC) -o $@ $^ $(LIBS) -lmsg -lpthread

msgcli: msgcli.o comsock.o serverut.o clientut.o
	$(CC) -o $@ $^ $(LIBS) -lmsg -lpthread

# make rule per i .o del terzo frammento (***DA COMPLETARE***)

comsock.o: comsock.c utils.h
	$(CC) $(CFLAGS) -c -o $@ $< 

serverut.o: serverut.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS) -lmsg -lpthread 

clientut.o: clientut.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS) -lmsg -lpthread 








########### NON MODIFICARE DA QUA IN POI ################
# genera la documentazione con doxygen
docu: ../doc/Doxyfile
	make -C ../doc

#ripulisce
clean:
	-rm -f *.o *.~

# primo test primo frammento
test11: 
	make clean
	make $(exe1)
	echo MALLOC_TRACE e\' $(MALLOC_TRACE)
	@echo MALLOC_TRACE deve essere settata a \"./.mtrace\"
	-rm -f ./.mtrace
	./$(exe1)
	mtrace ./$(exe1) ./.mtrace
	@echo -e "\a\n\t\t *** Test 1-1 superato! ***\n"

# secondo test primo frammento
test12: 
	make clean
	make $(exe2)
	echo MALLOC_TRACE e\' $(MALLOC_TRACE)
	@echo MALLOC_TRACE deve essere settata a \"./.mtrace\"
	-rm -f ./.mtrace
	./$(exe2)
	mtrace ./$(exe2) ./.mtrace
	@echo -e "\a\n\t\t *** Test 1-2 superato! ***\n"

# test secondo frammento
test21: 
	cp DATA/logtest.* .            
	chmod 644 ./logtest.*       
	./logpro ./logtest.dat > ./outtest    
	diff ./outtest ./logtest.check  
	@echo -e "\a\n\t\t *** Test 2-1 superato! ***\n"

# primo test terzo frammento 
test31: msgserv msgcli lib
	-killall -w msgserv  
	-killall -w msgcli   
	cp DATA/userlist .   
	cp DATA/userlist2 .  
	cp DATA/out31.check . 
	chmod 644 userlist userlist2 out31.check
	-rm -fr tmp/   
	-mkdir tmp 
	./testparse 2>.err 
	diff ./out31 ./out31.check 
	@echo -e "\a\n\t\t *** Test finale 3-1 superato! ***\n" 

# secondo test terzo frammento 
test32: msgserv msgcli lib  
	-killall -w msgserv  
	-killall -w msgcli   
	cp DATA/userlist .   
	cp DATA/outcli?.check . 
	chmod 644 ./userlist 
	chmod 644 ./outcli?.check
	-rm -fr tmp/          
	-mkdir tmp 
	./testfunz 
	diff ./outcli1 ./outcli1.check 
	diff ./outcli2 ./outcli2.check 
	diff ./outcli3 ./outcli3.check 
	@echo -e "\a\n\t\t *** Test finale 3-2 superato! ***\n" 

# terzo test terzo frammento 
test33: msgserv msgcli lib   
	-killall -w msgserv  
	-killall -w msgcli   
	cp DATA/userlist .   
	cp DATA/logsort.check .   
	chmod 644 userlist  
	chmod 644 logsort.check
	-rm -fr tmp/           
	-mkdir tmp 
	./testpar  
	@echo -e "\a\n\t\t *** Test finale 3-3 superato! ***\n" 

# target di consegna primo frammento
SUBJECT1="lso10: consegna primo frammento"
consegna1: 
	make test11
	make test12
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f1.tar ./gruppo.txt ./Makefile $(FILE_DA_CONSEGNARE1)
	mpack -s $(SUBJECT1) ./$(USER)-f1.tar  susanna@di.unipi.it
	@echo -e "\a\n\t\t *** Frammento 1 consegnato  ***\n"

# target di consegna secondo frammento
SUBJECT2="lso10: consegna secondo frammento"
consegna2: 
	make test21
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f2.tar ./gruppo.txt $(FILE_DA_CONSEGNARE2)
	mpack -s $(SUBJECT2) ./$(USER)-f2.tar  susanna@di.unipi.it
	@echo -e "\a\n\t\t *** Frammento 2 consegnato  ***\n"

# target di consegna terzo frammento
SUBJECT3="lso10: consegna terzo frammento"
consegna3: 
	make test11
	make test12
	make test21
	make test31
	make test32
	make test33
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f3.tar ./gruppo.txt $(FILE_DA_CONSEGNARE3)
	mpack -s $(SUBJECT3) ./$(USER)-f3.tar  susanna@di.unipi.it
	@echo -e "\a\n\t\t *** Progetto finale consegnato  ***\n"

