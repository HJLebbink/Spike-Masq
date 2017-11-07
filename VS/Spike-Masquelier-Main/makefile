# *** MACROS

CC = icpc
#CC = icpc
GCC = g++

INCPATH = -I/home/henk/sc/ScHJ/CWS -I/usr/lib/boost_1_58_0
OBJPATH = -L/home/henk/sc/ScHJ/CWS/Tools
#REPORT = -qopt-report=5 -qopt-report-phase=vec
REPORT =
CFLAGS_GCC 		=  -O3 -Wunused-variable -std=c++14 -msse4.1 -fdiagnostics-color=always -fopenmp $(INCPATH)
LFLAGS_GCC 		= $(OBJPATH) -std=c++14 -fopenmp

CFLAGS_INTEL = -fast -Wall -std=c++11 -openmp $(INCPATH) $(REPORT)
#CFLAGS_INTEL 	= -O3 -Wall -std=c++11 -openmp $(INCPATH) $(REPORT)
LFLAGS_INTEL 	= $(OBJPATH) -openmp

all: gcc
gcc: compile_gcc link_gcc
intel: compile_intel link_intel


compile_gcc:
	$(GCC) $(CFLAGS_GCC) -c Main-Spike-Masquelier.cpp -o Main-Spike-Masquelier.o

link_gcc:
	$(GCC) $(LFLAGS_GCC) Main-Spike-Masquelier.o -o Main-Spike-Masquelier.out
	
	
compile_intel:
	$(CC) $(CFLAGS_INTEL) -c Main-Spike-Masquelier.cpp -o Main-Spike-Masquelier.o

link_intel: 
	$(CC) $(LFLAGS_INTEL) Main-Spike-Masquelier.o -o Main-Spike-Masquelier.out

#copy: 
#	sudo scp dpflops.out mic0:
#	sudo scp /opt/intel/composerxe/lib/mic/libiomp5.so mic0:/tmp

#run: 
#	@echo "do manually:"
#	@echo "export LD_LIBRARY_PATH=/tmp"
#	@echo "export KNP_AFFINITY=compact"
#	@echo "export OMP_NUM_THREADS=228"
#	@echo "./dpflops.out"
#	@echo "exit"
#	sudo ssh mic0


clean: 
	@-rm -f Spike-Masquelier-Main.out Spike-Masquelier-Main.o
	@-rm -f *.optrpt
