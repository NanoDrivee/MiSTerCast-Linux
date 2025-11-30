CC=g++
FILETYPE=cpp

SRCDIR=src
OUTDIR=out

INC=WaylandVideoCapture.h
INCDIR=include
INCPATHS=$(patsubst %,$(INCDIR)/%,$(INC))

LIB=
LIBDIR=lib
LIBPATHS=$(patsubst %,$(LIBDIR)/%,$(LIB))

OBJ=testmain.o WaylandVideoCapture.o
OBJDIR=$(OUTDIR)/obj
OBJPATHS=$(patsubst %,$(OBJDIR)/%,$(OBJ))

CFLAGS=-Wall -I$(INCDIR) -I$(LIBDIR) -ggdb $$(pkg-config --cflags --libs libpipewire-0.3) $$(pkg-config --cflags --libs libportal)
LIBFLAGS=-lm

$(OBJDIR)/%.o: $(SRCDIR)/%.$(FILETYPE) $(INCPATHS) $(LIBPATHS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

debug: $(OBJPATHS) | $(OUTDIR)
	$(CC) $(CFLAGS) $(LIBFLAGS) -o $(OUTDIR)/$@ $^

$(OBJDIR):
	mkdir -p $@

$(OUTDIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -r $(OUTDIR)