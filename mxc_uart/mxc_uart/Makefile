TOOL_DIR =/opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin
CC=$(TOOL_DIR)/arm-none-linux-gnueabi-gcc
PLATFORM_INC_DIR = /home/hh/ltib/rpm/BUILD/linux-3.0.35/include
IPU_INC_DIR = /home/hh/ltib/rpm/BUILD/imx-lib-3.0.35-4.1.0/ipu
VPU_INC_DIR = /home/hh/ltib/rpm/BUILD/imx-vpu-lib-3.0.35-4.1.0
CFLAGS = -I$(PLATFORM_INC_DIR) -I$(IPU_INC_DIR) -I$(VPU_INC_DIR)
LIB_DIR = /home/hh/ltib/rootfs/usr/lib
LFLAGS += -Wl,-rpath-link,$(LIB_DIR) -Wl,-rpath-link,$(LIB_DIR)/arm-linux-gnueabi -L $(LIB_DIR) -lg2d -lvpu -lipu

OBJS = main.o \
      dec.o \
      enc.o \
      comm.o \
      capture.o \
      display.o \
      fb.o \
      utils.o \
      loopback.o \
      transcode.o

TARGET = mxc_vpu_test.out

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJS)  $(LFLAGS)

clean:
	rm -f *.o *.out

