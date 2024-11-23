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
LDFLAGS=-lsioclient -lsioclient_tls -lws2_32 -lssl -lcrypto -lcurl -lpthread -lmswsock -lboost_system -lboost_thread -ljsoncpp -lsqlite3
build:
ifeq (,$(wildcard $(outdir)))
	mkdir $(outdir)/app
endif
	$(GXX) $(CXXFLAGS) $(inputfile) -o $(outdir)/app/$(outfile) $(LDFLAGS)
run: build
	$(outdir)/app/$(outfile)
	$(MAKE) --no-print-directory clean
app:
	$(MAKE) --no-print-directory build
set-folder:
	mkdir $(outdir)/app $(outdir)/env $(outdir)/log
clean:
	del /f $(outfile).exe *.exe $(Library_Path)\$(Perceptron_Path)\*.o $(Library_Path)\$(MLP_Path)\*.o 2>nul || echo File not found
	rmdir /s /q $(outdir) 2>nul || echo Directory not found
else
LDFLAGS=-lssl -lcrypto -lcurl -lpthread -lboost_system -lboost_thread -lsioclient -lsioclient_tls -ljsoncpp -lsqlite3
PREFILE:
	$(GXX) ./$(Library_Path)/$(Perceptron_Path)/Perceptron.cpp -o ./$(Library_Path)/$(Perceptron_Path)/$(PerceptronName).o -c || $(MAKE) --no-print-directory clean
	echo "\033[1;32mPREFILE Perceptron compiled successfully!\033[0m"

	$(GXX) ./$(Library_Path)/$(MLP_Path)/MLP.cpp -o ./$(Library_Path)/$(MLP_Path)/$(MLPName).o -c || $(MAKE) --no-print-directory clean
	echo "\033[1;32mPREFILE MultiLayerPerceptron compiled successfully!\033[0m"

build: PREFILE
ifeq (,$(wildcard $(outdir)))
	mkdir $(outdir)/app
endif
	$(GXX) $(CXXFLAGS) $(inputfile) ./$(Library_Path)/$(Perceptron_Path)/$(PerceptronName).o ./$(Library_Path)/$(MLP_Path)/$(MLPName).o -o $(outdir)/app/$(outfile) $(LDFLAGS)
run: build
	./$(outdir)/app/$(outfile)
	$(MAKE) --no-print-directory clean
app:
	$(MAKE) --no-print-directory set-folder
	$(MAKE) --no-print-directory build
install:
	sudo apt-get install libboost-all-dev libjsoncpp-dev libsqlite3-dev libcurl4-openssl-dev nlohmann-json3-dev libssl-dev curl libcurl4-openssl-dev libssl-dev
set-folder:
	mkdir -p $(outdir)/app $(outdir)/env $(outdir)/log
	cp -r dev.env $(outdir)/env
clean:
	rm -f $(outfile) *.exe ./$(Library_Path)/$(Perceptron_Path)/*.o ./$(Library_Path)/$(MLP_Path)/*.o
	rm -rf $(outdir)
endif