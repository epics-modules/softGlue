# BEGIN softGlue.cmd ----------------------------------------------------------
# This must run after industryPack.cmd

#devAsynSoftGlueDebug=1
#drvIP_EP201Debug=1

# Write content to the FPGA.    This command will fail if the FPGA already has
# content loaded, as it will after a soft reboot.  To load new FPGA content,
# you must power cycle the ioc.
# initIP_EP200_FPGA(ushort_t carrier, ushort_t slot, char *filename)
initIP_EP200_FPGA(0, 2, "$(SOFTGLUE)/db/EP200_FPGA.hex")

# Each instance of a fieldIO_registerSet component is initialized as follows:
#
#int initIP_EP201(const char *portName, ushort_t carrier, ushort_t slot,
#	int msecPoll, int dataDir, int sopcOffset, int interruptVector,
#	int risingMask, int fallingMask)
# Note that while the addresses and interrupt vector look adjustable, they
# really are not.  
# 16 input bits
initIP_EP201("SGI1",0,2,1000,0x0,  0x0 ,0x80,0x7f,0x7f)
# 16 output bits (can't generate interrupts)
initIP_EP201("SGO1",0,2,1000,0x101, 0x10 ,0x81,0x00,0x00)

# All instances of a single-register component are initialized with a single
# call, as follows:
#
#initIP_EP201SingleRegisterPort(const char *portName, ushort_t carrier,
#	ushort_t slot)
#
# For example:
# initIP_EP201SingleRegisterPort("SOFTGLUE", 0, 2)
initIP_EP201SingleRegisterPort("SOFTGLUE", 0, 2)


# Load a single database that all database fragments supporting single-register
# components can use to show which signals are connected together.  This
# database is not needed for the functioning of the components, it's purely
# for the user interface.
dbLoadRecords("$(SOFTGLUE)/db/softGlue_SignalShow.db","P=xxx:,H=softGlue:")

# Load a set of database fragments for each single-register component.
dbLoadRecords("$(SOFTGLUE)/db/softGlue_FPGAContent.db", "P=xxx:,H=softGlue:,PORT=SOFTGLUE")

# Interrupt support.
dbLoadRecords("$(SOFTGLUE)/db/softGlue_FPGAInt.db", "P=xxx:,H=softGlue:,IPORT=SGI1,IADDR=0,OPORT=SGO1,OADDR=0x10")

# some stuff just to make working easier
args = "P=xxx:,H=softGlue:,PORT=SOFTGLUE,IPORT=SGI1,IADDR=0,OPORT=SGO1,OADDR=0x10"
dbLoadRecords("$(SOFTGLUE)/db/softGlue_convenience.db", args)

# END softGlue.cmd ------------------------------------------------------------
