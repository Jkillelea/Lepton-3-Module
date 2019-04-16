# part of raw_capture. Copyright 2019 Jacob Killelea <jkillelea@protonmail.ch>

CC = gcc
CFLAGS = -Wall -march=native
CFLAGS += -O3
# CFLAGS += -g

LIB_PATH = ../raspberrypi_libs/lepton-sdk-fork
CFLAGS += -I$(LIB_PATH)

SDK_LIB = $(LIB_PATH)/Debug/libLEPTON_SDK.a

EXEC = raw_capture

OUTDIR = Build

OBJS = $(OUTDIR)/raw_capture.o $(OUTDIR)/util.o  $(OUTDIR)/packet.o $(OUTDIR)/segment.o

.PHONY: all
all: $(EXEC)

$(EXEC): $(OUTDIR) $(OBJS) $(SDK_LIB)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(SDK_LIB)

restart_camera: $(OUTDIR) restart_camera.c  $(SDK_LIB)
	$(CC) $(CFLAGS) -o $@ restart_camera.c $(SDK_LIB)


$(OUTDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

$(OUTDIR):
	mkdir -p $@

$(SDK_LIB):
	$(MAKE) -C $(LIB_PATH)

.PHONY: clean
clean:
	rm -rf $(OUTDIR)
	rm -rf $(EXEC)

.PHONY: sdkclean
sdkclean:
	$(MAKE) clean -C $(LIB_PATH)
