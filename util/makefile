OBJDIR=obj

GCOMP=g++

include ../compiler.cfg

DEP=depend
DEPENDS:=$(patsubst %.c, %.dep, $(wildcard *.c)) $(patsubst %.cc, %.dep, $(wildcard *.cc))
CURR_DEP:=$(wildcard $(DEP))
CURR_DEPENDS:=$(wildcard $(DEPENDS))
BIN = makerep example1 filemake min example1 cexample1 cexample2 cropen ctestReadWrite
CURR_BIN=$(wildcard $(BINARIES))

%.dep: %.c
	gcc $(CSTANDARD) -MM $< -MT $(patsubst %.c, $(OBJDIR)/%.co, $<) $(CINC_FOLDER) -o $@ 

%.dep: %.cc
	g++ $(CXXSTANDARD) -MM $< -MT $(patsubst %.cc, $(OBJDIR)/%.cpo, $<) $(CXXINC_FOLDER) -o $@ 

$(OBJDIR)/%.cpo: %.cc
	$(CXX) $(CXXFLAGS) $(CXXINC_FOLDER) -c $< -o $@

$(OBJDIR)/%.co: %.c
	$(CC) $(CFLAGS) $(CINC_FOLDER) -c $< -o $@

.PHONY: all
all: $(DEP) cropen cexample1 cexample2 ctestReadWrite example1 filemake makerep min


.PHONY: clean
clean:
	rm -f obj/*.co obj/*.cpo $(BIN) $(CURR_DEPENDS) $(CURR_DEP) *.dep $(CURR_BIN)

cropen: obj/cropen.co obj/sglobal.co obj/ctest.co ../lib/libcpiol.so
	$(CC) $(CFLAGS) $(CLDFLAGS) $+ -o $@ 
cexample1: obj/example1.co obj/sglobal.co ../lib/libcpiol.so
	$(CC) $(CFLAGS) $(CLDFLAGS) $+ -o $@ 
cexample2: obj/example2.co obj/sglobal.co ../lib/libcpiol.so
	$(CC) $(CFLAGS) $(CLDFLAGS) $+ -o $@ 
ctestReadWrite: obj/testReadWrite.co obj/sglobal.co ../lib/libcpiol.so
	$(CC) $(CFLAGS) $(CLDFLAGS) $+ -o $@ 
assess: obj/assess.cpo obj/sglobal.cpo ../lib/libpiol.so
	$(CXX) $(CXXFLAGS) $(CXXLDFLAGS) $+ -o $@ 
example1: obj/example1.cpo obj/sglobal.cpo ../lib/libpiol.so
	$(CXX) $(CXXFLAGS) $(CXXLDFLAGS) $+ -o $@ 
filemake: obj/filemake.cpo obj/sglobal.cpo ../lib/libpiol.so
	$(CXX) $(CXXFLAGS) $(CXXLDFLAGS) $+ -o $@ 
makerep: obj/makerep.cpo obj/sglobal.cpo ../lib/libpiol.so
	$(CXX) $(CXXFLAGS) $(CXXLDFLAGS) $+ -o $@ 
min: obj/min.cpo obj/sglobal.cpo ../lib/libpiol.so
	$(CXX) $(CXXFLAGS) $(CXXLDFLAGS) $+ -o $@ 

ifeq "$(CURR_DEP)" "$(DEP)"
-include $(DEP)
endif

$(DEP): $(DEPENDS)
	cat *.dep > $(DEP)
