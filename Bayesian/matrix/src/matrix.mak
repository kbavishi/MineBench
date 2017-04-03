#-----------------------------------------------------------------------
# File    : matrix.mak
# Contents: build vector and matrix management modules
# Author  : Christian Borgelt
# History : 27.01.2003 file created
#-----------------------------------------------------------------------
CC       = cl.exe
LD       = link.exe
DEFS     = /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS"
CFLAGS   = /nologo /ML /W3 /GX /O2 $(DEFS) /I $(UTILDIR) /YX /FD /c
LDFLAGS  = /nologo /subsystem:console /incremental:no /machine:I386
LIBS     = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
           advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
           odbc32.lib odbccp32.lib

THISDIR  = ..\..\matrix\src
UTILDIR  = ..\..\util\src
OBJ1_O   = mat_read.obj $(UTILDIR)/tfscan.obj
OBJ2_O   = mat_read.obj matrix2.obj $(UTILDIR)\tfscan.obj
PRGS     = transp.exe invert.exe solve.exe

#-----------------------------------------------------------------------
# Build Programs
#-----------------------------------------------------------------------
all:          $(PRGS)

transp.exe:   $(OBJ1_O) transp.obj matrix.mak
	$(LD) $(LDFLAGS) $(OBJ1_O) transp.obj $(LIBS) /out:$@

invert.exe:   $(OBJ2_O) invert.obj matrix.mak
	$(LD) $(LDFLAGS) $(OBJ2_O) invert.obj $(LIBS) /out:$@

solve.exe:    $(OBJ2_O) solve.obj matrix.mak
	$(LD) $(LDFLAGS) $(OBJ2_O) solve.obj  $(LIBS) /out:$@

#-----------------------------------------------------------------------
# Main Programs
#-----------------------------------------------------------------------
transp.obj:   matrix.h transp.c matrix.mak
	$(CC) $(CFLAGS) transp.c /Fo$@

invert.obj:   matrix.h invert.c matrix.mak
	$(CC) $(CFLAGS) invert.c /Fo$@

solve.obj:    matrix.h solve.c matrix.mak
	$(CC) $(CFLAGS) solve.c /Fo$@

#-----------------------------------------------------------------------
# Vector and Matrix Management
#-----------------------------------------------------------------------
matrix1.obj:  matrix.h matrix1.c makefile
	$(CC) $(CFLAGS) matrix1.c /Fo$@

mat_read.obj: $(UTILDIR)\tfscan.h matrix.h matrix1.c makefile
	$(CC) $(CFLAGS) /D MAT_READ matrix1.c /Fo$@

matrix2.obj:  matrix.h matrix2.c makefile
	$(CC) $(CFLAGS) matrix2.c /Fo$@

matrix3.obj:  matrix.h matrix3.c makefile
	$(CC) $(CFLAGS) matrix3.c /Fo$@

#-----------------------------------------------------------------------
# External Modules
#-----------------------------------------------------------------------
$(UTILDIR)\tfscan.obj:
	cd $(UTILDIR)
	$(MAKE) /f util.mak tfscan.obj
	cd $(THISDIR)

#-----------------------------------------------------------------------
# Install
#-----------------------------------------------------------------------
install:
	-@copy *.exe c:\home\bin

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------
clean:
	$(MAKE) /f matrix.mak localclean
	cd $(UTILDIR)
	$(MAKE) /f util.mak clean
	cd $(THISDIR)

localclean:
	-@erase /Q *~ *.obj *.idb *.pch $(PRGS)
