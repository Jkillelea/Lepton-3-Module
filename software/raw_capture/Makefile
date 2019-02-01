CC = gcc

CFLAGS = -Wall
CFLAGS += -O3
# CFLAGS += -g

LIB_PATH = ../raspberrypi_libs/lepton-sdk-fork/
CFLAGS += -I$(LIB_PATH)

SDK_LIB = $(LIB_PATH)/Debug/libLEPTON_SDK.a

EXEC = raw_capture

OUTDIR = Build

OBJS = $(OUTDIR)/raw_capture.o $(SDK_LIB)

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
