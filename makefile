.SILENT:
.PHONY: build run clean

GXX=g++
CXXFLAGS=
 
file=main
outname=EdgeFrontier

Library_Path=Libs

PerceptronName=Perceptron
Perceptron_Path=Perceptron

MLPName=MLP
MLP_Path=MLP

# Detect OS and architecture
ifeq ($(OS),Windows_NT)
	OS := Windows_NT
	arch := x86_64
else
	OS := $(shell uname -s)
	arch := $(shell uname -m)
endif

# Define OS_sub and Arch variables
OS_sub := $(OS)
Arch := $(arch)

# Select compiler based on architecture and OS
ifeq ($(OS),Linux)
    # Check for x86_64, x86, i686, or i386 using filter
    ifneq ($(filter x86_64 x86 i686 i386,$(arch)),)
		GXX = g++
    else ifneq ($(filter arm64 aarch64 armv7l armv6l,$(arch)),)
	    GXX = aarch64-linux-gnu-g++
    else
        $(error "Unknown Linux architecture: $(arch)")
    endif
else ifeq ($(OS),Windows_NT)
	ifneq ($(filter x86_64 x86 i686 i386,$(arch)),)
		GXX = g++
#	else ifneq ($(filter arm64 aarch64 armv7l armv6l,$(arch)),)
#		GXX = aarch64-linux-gnu-g++
	else
		$(error "Unknown Windows architecture: $(arch)")
	endif
else
    $(error "Unknown OS: $(OS)")
endif

inputfile=$(file).cpp
outdir=EdgeFrontier
outfile=$(outname)_$(OS_sub)_$(Arch)

ifeq ($(OS),Windows_NT)
LDFLAGS=-lssl -lcrypto -lcurl -lpthread -lboost_system -lboost_thread -ljsoncpp -lsqlite3
PREFILE:
	$(GXX) .\$(Library_Path)\$(Perceptron_Path)\Perceptron.cpp -o .\$(Library_Path)\$(Perceptron_Path)\$(PerceptronName).o -c || $(MAKE) --no-print-directory clean
	echo "PREFILE Perceptron compiled successfully!"

	$(GXX) .\$(Library_Path)\$(MLP_Path)\MLP.cpp -o .\$(Library_Path)\$(MLP_Path)\$(MLPName).o -c || $(MAKE) --no-print-directory clean
	echo "PREFILE MultiLayerPerceptron compiled successfully!"

build: PREFILE
	$(MAKE) --no-print-directory set-folder
	$(GXX) $(CXXFLAGS) $(inputfile) .\$(Library_Path)\$(Perceptron_Path)\$(PerceptronName).o .\$(Library_Path)\$(MLP_Path)\$(MLPName).o -o $(outdir)\app\$(outfile).exe $(LDFLAGS)
run: build
	.\$(outdir)\app\$(outfile).exe
	$(MAKE) --no-print-directory clean
app:
	$(MAKE) --no-print-directory build
set-folder:
	mkdir $(outdir)\app $(outdir)\env $(outdir)\log $(outdir)\model
	copy dev.env $(outdir)\env
	copy model.json $(outdir)\model
clean:
	del $(outfile).exe *.exe .\$(Library_Path)\$(Perceptron_Path)\*.o .\$(Library_Path)\$(MLP_Path)\*.o
	rmdir /s /q $(outdir)
else
LDFLAGS=-lssl -lcrypto -lcurl -lpthread -lboost_system -lboost_thread -ljsoncpp -lsqlite3
PREFILE:
	$(GXX) ./$(Library_Path)/$(Perceptron_Path)/Perceptron.cpp -o ./$(Library_Path)/$(Perceptron_Path)/$(PerceptronName).o -c || $(MAKE) --no-print-directory clean
	echo "Build Perceptron : \033[1;32mSUCCESS\033[0m"

	$(GXX) ./$(Library_Path)/$(MLP_Path)/MLP.cpp -o ./$(Library_Path)/$(MLP_Path)/$(MLPName).o -c || $(MAKE) --no-print-directory clean
	echo "Build MultiLayerPerceptron : \033[1;32mSUCCESS\033[0m"

build: PREFILE
	$(MAKE) --no-print-directory set-folder
	$(GXX) $(CXXFLAGS) $(inputfile) ./$(Library_Path)/$(Perceptron_Path)/$(PerceptronName).o ./$(Library_Path)/$(MLP_Path)/$(MLPName).o -o $(outdir)/app/$(outfile) $(LDFLAGS)
	echo "Build $(outfile) : \033[1;32mSUCCESS\033[0m"
run: build
	./$(outdir)/app/$(outfile)
	$(MAKE) --no-print-directory clean
app:
	$(MAKE) --no-print-directory build
install:
	sudo apt-get install libboost-all-dev libjsoncpp-dev libsqlite3-dev libcurl4-openssl-dev nlohmann-json3-dev libssl-dev curl libcurl4-openssl-dev libssl-dev
set-folder:
	mkdir -p $(outdir)/app $(outdir)/env $(outdir)/log $(outdir)/model
	cp -r dev.env $(outdir)/env
	cp -r model.json $(outdir)/model	
	echo "Set folder : \033[1;32mSUCCESS\033[0m"
clean:
	rm -f $(outfile) *.exe ./$(Library_Path)/$(Perceptron_Path)/*.o ./$(Library_Path)/$(MLP_Path)/*.o *.o
	rm -rf $(outdir)
endif