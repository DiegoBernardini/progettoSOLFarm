# compilatore da usare
CC		=  gcc

AR 		=  ar
# flag di compilazione 
CFLAGS	        += -std=c99 -Wall -Werror -g
ARFLAGS         =  rvs
# nome della cartella che contiene i file includes
INCDIR 		= ./myInclude
#
INCLUDES 	= -I $(INCDIR)
LDFLAGS 	= -L.
OPTFLAGS	= -O3 
LIBS        = -pthread

# nome dell' eseguibile
EXE 		= farm

# aggiungere qui altri targets
TARGETS		= farm generafile

# phony target
.PHONY: all clean cleanall test
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<


$(EXE): cmdLineParser.o collector.o list.o main.o master.o threadpool.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $(EXE) $(LDFLAGS) $(LIBS)

generafile: ./generafile.c
	$(CC) $(CFLAGS) -o $@ $^	

all		: $(TARGETS)

clean 	:
	@ -rm -f *.o ./farm.sck 

cleanall	: clean
	\rm -f *~ *.a *.dat *.txt generafile $(TARGETS) -r testdir 

# comando utilizzato per eseguire il processo in 5 modi diversi
test: generafile farm
	./test.sh

# comando utilizzato per testare la gestione dei segnali
# test: farm
# 	./$(EXE) file* -d testdir -n 4 -q 2 -t 4000

###############################################################################################
#comandi utilizzati durante lo sviluppo del progetto
# 	@valgrind --leak-check=full --track-origins=yes ./$(EXE) file* -d testdir -n 4 -q 2 -t 400
# 	make clean
# 	@valgrind --leak-check=full ./$(EXE) -d ./dat -n 3
# 	./$(EXE) -d ./dat -n 3
# 	@ps -A -ostat,pid,ppid | grep Z 