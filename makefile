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


#phony target
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
	@ -rm *.o ./farm.sck 
# 	\rm -f ./farm.sck
# clean		: 
# 	rm -f $(TARGETS)

cleanall	: clean
	rm -f *.o *~ *.a ./farm.sck  

test: generafile farm
	./test.sh


# test: farm
# 	@valgrind --leak-check=full --track-origins=yes ./$(EXE) -d ./dat -n 3 -q 5
# 	make clean
# 	@valgrind --leak-check=full ./$(EXE) -d ./dat -n 3
# 	./$(EXE) -d ./dat -n 3
# 	@ps -A -ostat,pid,ppid | grep Z 