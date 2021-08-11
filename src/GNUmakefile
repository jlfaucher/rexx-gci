#
# These	example	lines do the job for ORexx/Linux.
#
REXX_INCLUDE_PATH=/opt/orexx
REXX_LIBRARY_PATH=/opt/orexx/lib
HEADERDEF=-DUSE_REXX_H -DRexxAllocateMemory=malloc -DRexxFreeMemory=free \
	  -DPCSZ=PSZ

#
# There	is no need to change anything below for	known systems
###############################################################################
#
OBJ=obj

ifeq	  "Linux"		      "$(shell uname -s)"
 ifeq	   "ix86"		      "$(patsubst i%86,ix86,$(shell uname -m))"
		    CVT=linux.i86
		    OSL=unix
 else
  ifeq	   "x86_64"		      "$(shell uname -m)"
		    CVT=linux.x86_64
		    OSL=unix
  else
   ifeq	   "alpha"		      "$(shell uname -m)"
		    CVT=linux.alpha
		    OSL=unix
   else
    $(error			      Unsupported/unknown system)
   endif
  endif
 endif
else
 ifeq	   "Darwin Power Macintosh"  "$(shell uname -sm)"
		    CVT=macX.ppc
		    OSL=macX
 else
  ifeq	   "SunOS"		     "$(shell uname -s)"
		    CVT=solaris.sparc
		    OSL=unix
  else
   $(error			      Unsupported/unknown system)
  endif
 endif
endif

$(OBJ)/%.o: %.c
	gcc -Wall -I$(REXX_INCLUDE_PATH) $(HEADERDEF) -g -fPIC -DDEBUG \
	    -c -o $@ $<

OBJS=$(OBJ)/gci_convert.o $(OBJ)/gci_prepare.o $(OBJ)/gci_tree.o \
     $(OBJ)/gci_helper.o $(OBJ)/gci_rexxbridge.o $(OBJ)/gci_rxfuncdefine.o \
     $(OBJ)/gci_call.o $(OBJ)/gci_execute.o $(OBJ)/gci_oslink.o

libgci.so: $(OBJS)
	gcc -Wall -I. -g -shared			\
	    -fPIC -Wl,--version-script=gci_elf.def	\
	    -o $@ $^

$(OBJS): gci.h embedded.h

$(OBJ)/gci_convert.o: gci_convert.c gci_convert.h

$(OBJ)/gci_call.o: gci_call.c gci_convert.h

gci_convert.h: gci_convert.$(CVT)
	@-rm -f	gci_convert.h
	ln -s gci_convert.$(CVT) gci_convert.h

gci_oslink.c: gci_oslink.$(OSL)
	@-rm -f	gci_oslink.c
	ln -s gci_oslink.$(OSL)	gci_oslink.c

.PHONY:	clean cleaner distclean

clean:
	rm -f *.bak err

cleaner: clean
	rm -f $(OBJ)/*.o *.so

distclean: cleaner
	rm -f .gdbinit*	try* gci_convert.h gci_oslink.c
