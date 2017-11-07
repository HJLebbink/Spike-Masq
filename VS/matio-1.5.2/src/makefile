#filename matio-1.5.2\makefile.hj
# *** MACROS

GCC = gcc
#GCC = icpc

INCPATH = 
OBJPATH = 

CFLAGS_GCC 		= -O3 -w $(INCPATH)
LFLAGS_GCC   	= $(OBJPATH)

OBJS=endian.o inflate.o io.o mat.o mat4.o mat5.o mat73.o matvar_cell.o matvar_struct.o read_data.o snprintf.o

all: gcc copy clean
gcc: make_lib

.c.o:
	@echo "Compiling" $< "..."
	$(GCC) $(CFLAGS_GCC) -c -fPIC -o $@ $<

make_lib: $(OBJS)
	@echo "Building library" $@ "..."

	ar rcs libmatio.a $(OBJS)

$(OBJS): mat4.h mat5.h mat73.h matio.h matio_private.h matio_pubconf.h matioConfig.h

copy:
	mv libmatio.a ./../libmatio.a


clean: 
	@-rm -f *.o
	@-rm -f *.optrpt