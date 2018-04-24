
#######################################################################
#### Compilation options (32/64 bit)
CC          = g++


## Uncomment for using a dictionary represented as a bitmap-RRR
#export CXXFLAGS  = -std=c++11 -O9 -m64 -DNDEBUG  -DDICTIONARY_RRR -I libcds/includes/
export CXXFLAGS  = -std=c++11 -O9 -m64 -DNDEBUG -DEXPERIMENTS -DDICTIONARY_RRR -I libcds/includes/
#export CXXFLAGS  = -std=c++11 -g -O0 -m64 -Wall  -DDICTIONARY_RRR -I libcds/includes/


#######################################################################
#######################################################################
#### Set the enviroment to build the i-CSA / W-CSA

 export SRCDIRISI  = intIndex_qsort
 export NAMEINDEX   = wcsa
 export LIBINDEX    = swcsa.a
 export LIBINTINDEX = icsa.a
 export LIBCDS = libcds/libcds.a
 export LIBSAIS = libsais/libsais.a


SRCDIR = ./src
SRCDIRUTILS = utils
SRCDIRLIBCDS = ./src/libcds

#######################################################################

all: clean buildAll benchmark cleanO

wsi: intIndexPackage buildFacade.o parameters.o basics.o \
		bitmap.o huff.o fileInfo.o graphReader.o delta.o #icsa.o
	ar rc $(LIBINTINDEX) parameters.o buildFacade.o basics.o \
		bitmap.o huff.o fileInfo.o graphReader.o delta.o
	#ar q $(LIBINDEX) icsa.o psiHuffmanRLE.o psiDeltaCode.o psiGonzalo.o  ##they are included by icsa.a
	mv $(LIBINTINDEX) $(LIBINDEX)

benchmark: wsi
	 $(CC) $(CXXFLAGS) -o  benchmark $(SRCDIR)/benchmark.cpp $(LIBINDEX) $(LIBCDS) $(LIBSAIS) -lrt

exp-query: wsi
	 $(CC) $(CXXFLAGS) -o  exp-query $(SRCDIR)/exp-query.cpp $(LIBINDEX)  -lrt

buildAll:  wsi
	$(CC) -DFACADEWITHMAIN $(CXXFLAGS) -o BUILDALL$(NAMEINDEX) $(SRCDIR)/buildAll.c $(LIBINDEX) $(LIBCDS) $(LIBSAIS)

buildFacade.o: parameters.o basics.o bitmap.o \
		 huff.o fileInfo.o delta.o $(LIBINTINDEX)
	 $(CC) $(CXXFLAGS) -c  $(SRCDIR)/buildFacade.cpp $(LIBCDS) $(LIBSAIS)



###################################################
##FROM LIBCDS
###################################################

Array.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIRLIBCDS)/Array.cpp

BitString.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIRLIBCDS)/BitString.cpp

BitSequence.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIRLIBCDS)/BitSequence.cpp

BitSequenceRRR.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIRLIBCDS)/BitSequenceRRR.cpp

BitSequenceBuilderRRR.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIRLIBCDS)/BitSequenceBuilderRRR.cpp

TableOffsetRRR.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIRLIBCDS)/TableOffsetRRR.cpp

BitSequenceRG.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIRLIBCDS)/BitSequenceRG.cpp

BitSequenceBuilderRG.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIRLIBCDS)/BitSequenceBuilderRG.cpp

###################################################


################# SELF INDEX ON INTEGERS ##############################
#Builds the int-self-index in dir $(SRCDIRISI) (ex: i-ssa, i-csa,... )

intIndexPackage:
	cd  $(SRCDIR)/$(SRCDIRISI) && $(MAKE) -w
	@echo "[copying the int-index lib into current dir]"
	@cp $(SRCDIR)/$(SRCDIRISI)/$(LIBINTINDEX) .

####################### UTILS MODULES #################################

parameters.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIR)/$(SRCDIRUTILS)/parameters.c

fileInfo.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIR)/$(SRCDIRUTILS)/fileInfo.c

huff.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIR)/$(SRCDIRUTILS)/huff.c

basics.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIR)/$(SRCDIRUTILS)/basics.c

bitmap.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIR)/$(SRCDIRUTILS)/bitmap.cpp

graphReader.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIR)/graphReader.c

delta.o:
	$(CC) $(CXXFLAGS) -c $(SRCDIR)/$(SRCDIRUTILS)/delta.c


############################ CLEANING #################################

cleanO:
	rm -f *.o

clean:
	cd  $(SRCDIR)/$(SRCDIRISI) && $(MAKE) clean -w
	rm -rf *~ *% *.o core *.bak $(LIBINTINDEX) $(LIBINDEX)
	rm -rf BUILDALL$(NAMEINDEX) benchmark
