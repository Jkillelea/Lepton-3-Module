CC = gcc

CFLAGS = -Wall
CFLAGS += -O3
# CFLAGS += -g

INCLUDES = ../raspberrypi_libs/lepton-sdk-fork/
CFLAGS += -I$(INCLUDES)

EXEC = raw_capture

OUTDIR = Build

OBJS = $(OUTDIR)/raw_capture.o

.PHONY: all
all: $(EXEC)

$(EXEC): $(OUTDIR) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OUTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

$(OUTDIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(OUTDIR)
	rm -rf $(EXEC)
