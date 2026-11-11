# Astrolog (Version 8.00) File: Makefile (Unix version)
#
# IMPORTANT NOTICE: Astrolog and all chart display routines and anything
# not enumerated elsewhere in this program are Copyright (C) 1991-2026 by
# Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
# Permission is granted to freely use, modify, and distribute these
# routines provided these credits and notices remain unmodified with any
# altered or distributed versions of the program.
#
# First created 11/21/1991.
#
# This Makefile is included only for convenience. One could easily compile
# Astrolog on most Unix systems by hand with the command:
# % cc -c -O *.cpp; cc -o astrolog *.o -lm -lX11
# Generally, all that needs to be done to compile once astrolog.h has been
# edited, is compile each source file, and link them together with the math
# library, and if applicable, the main X library.
#
NAME = astrolog
OBJS = astrolog.o atlas.o calc.o charts0.o charts1.o charts2.o charts3.o\
 data.o express.o general.o intrpret.o io.o matrix.o placalc.o placalc2.o\
 xdata.o xgeneral.o xdevice.o xcharts0.o xcharts1.o xcharts2.o xscreen.o\
 swecl.o swedate.o swehouse.o swejpl.o swemmoon.o swemplan.o sweph.o\
 swephlib.o

# If you don't have X windows, delete the "-lX11" part from the line below:
# If not compiling with GNUC, delete the "-ldl" part from the line below:
LIBS = -lm -lX11 -ldl -s
CPPFLAGS = -O -Wno-write-strings -Wno-narrowing -Wno-comment
RM = rm -f

$(NAME): $(OBJS)
	cc -o $(NAME) $(OBJS) $(LIBS)

clean:
	$(RM) $(OBJS) $(NAME)
	$(RM) -r $(QTDIR) $(QTNAME)

# Qt 6 GUI version, with the same menus and dialogs as the Windows version.
# Runs natively on Wayland and X11 (e.g. KDE Plasma). Build with "make qt".
# Requires the Qt 6 development package, e.g. "qt6-base-devel" on Void
# Linux, "qt6-base-dev" on Debian and Ubuntu, or "qt6-qtbase-devel" on
# Fedora, plus pkg-config. See README.md for details. Objects go in
# their own directory so they don't mix with the X11 version's objects.
# "make rcqt" regenerates rcqt.inc from astrolog.rc (needs python3).
QTNAME = astrolog-qt
QTDIR = obj-qt
QTPKG = Qt6Widgets Qt6PrintSupport
QTOBJS = $(addprefix $(QTDIR)/,$(OBJS) wdriver.o wdialog.o wqt.o)
QTCPPFLAGS = -O -DQT -std=c++17 -fPIC -Wno-write-strings -Wno-narrowing\
 -Wno-comment $(shell pkg-config --cflags $(QTPKG))
QTLIBS = $(shell pkg-config --libs $(QTPKG)) -lm -ldl

qt: $(QTNAME)

$(QTNAME): $(QTOBJS)
	$(CXX) -o $(QTNAME) $(QTOBJS) $(QTLIBS)

$(QTDIR)/%.o: %.cpp astrolog.h extern.h wqt.h resource.h | $(QTDIR)
	$(CXX) $(QTCPPFLAGS) -c $< -o $@

$(QTDIR)/wqt.o: rcqt.inc

$(QTDIR):
	mkdir -p $(QTDIR)

rcqt: tools/rc2qt.py astrolog.rc resource.h
	python3 tools/rc2qt.py astrolog.rc resource.h rcqt.inc

.PHONY: qt rcqt clean
#
