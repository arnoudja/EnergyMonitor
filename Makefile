#
# Makefile to build energymonitor
# 

SRCS=	main.cpp \
	DisplayTable.cpp \
	EnergyData.cpp \
	EnergyMonitorConfig.cpp \
	OmnikGetStats.cpp \
	P1Monitor.cpp \
	PvOutput.cpp \
	Tft.cpp

OBJECTS=$(subst .cpp,.o,$(SRCS))

CXX=g++

CPPFLAGS= -Wall

LDFLAGS=

LDLIBS=-lm -L/usr/lib/i386-linux-gnu -lcurl -lftdi -lcairo -lwiringPi

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

all: $(OBJECTS)
	$(CXX) $(CPPFLAGS) -o energymonitor $(OBJECTS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o
	rm -f energymonitor
	rm -rf $(DEPDIR)

COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CPPFLAGS) -c
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

%.o : %.cpp $(DEPDIR)/%.d
	$(COMPILE.cpp) $(CPPFLAGS) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;

include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

