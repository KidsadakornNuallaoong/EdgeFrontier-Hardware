.SILENT:
.PHONY: build run clean

GXX=g++
CXXFLAGS=
 
file=server

ifeq ($(OS), Windows_NT)
	OS_sub=windows
	Arch=x86_64
else
	OS_sub=linux
	Arch=x86_64
endif

inputfile=$(file).cpp
outdir=dist
outfile=$(file)_$(OS_sub)_$(Arch)

ifeq ($(OS),Windows_NT)
LDFLAGS=-lsioclient -lsioclient_tls -lws2_32 -lssl -lcrypto -lcurl -lpthread -lmswsock
build:
ifeq (,$(wildcard $(outdir)))
	mkdir $(outdir)
endif
	$(GXX) $(CXXFLAGS) $(inputfile) -o $(outdir)/$(outfile) $(LDFLAGS)
run: build
	$(outdir)/$(outfile)
	$(MAKE) --no-print-directory clean
app:
	$(MAKE) --no-print-directory build file=server
	$(MAKE) --no-print-directory build file=client
clean:
	del /f $(outfile).exe *.exe 2>nul || echo File not found
	rmdir /s /q $(outdir) 2>nul || echo Directory not found
else
LDFLAGS=-lssl -lcrypto -lcurl -lpthread -lboost_system -lboost_thread -lsioclient -lsioclient_tls
build:
ifeq (,$(wildcard $(outdir)))
	mkdir $(outdir)
endif
	$(GXX) $(CXXFLAGS) $(inputfile) -o ./$(outdir)/$(outfile) $(LDFLAGS)
run: build
	./$(outdir)/$(outfile)
	$(MAKE) --no-print-directory clean
app:
	$(MAKE) --no-print-directory build file=server
	$(MAKE) --no-print-directory build file=client
clean:
	rm -f $(outfile) *.exe
	rm -rf $(outdir)
endif