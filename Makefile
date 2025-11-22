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

CFLAGS=-Wall -I$(INCDIR) -I$(LIBDIR) $$(pkg-config --cflags --libs libpipewire-0.3)
LIBFLAGS=-lm

$(OBJDIR)/%.o: $(SRCDIR)/%.$(FILETYPE) $(INCPATHS) $(LIBPATHS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(OBJPATHS) | $(OUTDIR)
	$(CC) $(CFLAGS) $(LIBFLAGS) -o $(OUTDIR)/$@ $^

$(OBJDIR):
	mkdir -p $@

$(OUTDIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -r $(OUTDIR)