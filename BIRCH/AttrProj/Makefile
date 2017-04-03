#for solaris 
ARCH = solaris 
include ../make_include

SHELL = /bin/csh 
 
PURELINKEROPT=-collector=g++ 
 
CFLAGS= -DSOLARIS -fexternal-templates -Wall -Wno-unused -O -DLOGGING  \
	-DUNIX -DGRAPHICS -DATTRPROJ 

CFLAGS= -O -DLOGGING -DGRAPHICS -DATTRPROJ 
 
CC	= $(CPLUS) $(CFLAGS) 
LD	= ld

LIBDIR =
 
#INCLUDES = -I/usr/local/packages/gcc-2.7.2/lib/g++-include \
#	-I/usr/local/packages/local/include 
#INCLUDES = -I/usr/local/packages/local/include 
 
#for solaris: command to to make depend 
MAKEDEPEND = $(CPLUS) -MM 
 
LIBOBJ = \
	ApInit.o \
	ApParseCat2.o \
	AttrList.o \
	AttrProj2.o \
	CompDate.o \
	CompositeParser.o \
	DList.o \
	DataSeg.o \
	DataSource.o \
	DataSourceBuf.o \
	DataSourceDQL.o \
	DataSourceFileDesc.o \
	DataSourceFileStream.o \
	DataSourceSegment.o \
	DataSourceTape.o \
	DCE.o \
	DevError.o \
	DevFileHeader.o \
	DevStatus.o \
	Exit.o \
	FileIndex.o \
	Group.o \
	GroupDir.o \
	ItemList.o \
	MemMgr.o \
	Parse.o \
	ProjectionList.o \
	RecInterp.o \
	TData.o \
	TDataAscii.o \
	TDataAsciiInterp.o \
	TDataBinary.o \
	TDataBinaryInterp.o \
	TDataSeqAscii.o \
	TDataSeqAsciiInterp.o \
	Time.o \
	TimeStamp.o \
	Timer.o \
	Util.o \
	VectorArray.o \
	Version.o \
	tapedrive.o

LIB	= AttrProjLib.o 

all	: $(LIB) 
 
depend	:  
	$(MAKEDEPEND) *.[cC] 

$(LIB): $(LIBOBJ)
	$(LD) -r -o $@ $(LIBOBJ)

ApInit.o	: ApInit.c 
	$(CC) $(INCLUDES) -c $<  
 
ApParseCat.o	: ApParseCat.c 
	$(CC) $(INCLUDES) -c $<  

ApParseCat2.o	: ApParseCat2.c 
	$(CC) $(INCLUDES) -c $<  
 
AttrList.o	: AttrList.c 
	$(CC) $(INCLUDES) -c $<  
 
AttrProj.o	: AttrProj.c 
	$(CC) $(INCLUDES) -c $<  

AttrProj2.o	: AttrProj2.c 
	$(CC) $(INCLUDES) -c $<  
 
CompDate.o	: CompDate.c 
	$(CC) $(INCLUDES) -c $<  
 
CompositeParser.o: CompositeParser.c 
	$(CC) $(INCLUDES) -c $<  
 
DList.o		: DList.c 
	$(CC) $(INCLUDES) -c $<  
 
DataSeg.o	: DataSeg.c 
	$(CC) $(INCLUDES) -c $<  
 
DataSource.o	: DataSource.c 
	$(CC) $(INCLUDES) -c $<  
 
DataSourceBuf.o	: DataSourceBuf.c 
	$(CC) $(INCLUDES) -c $<  
 
DataSourceFileDesc.o: DataSourceFileDesc.c 
	$(CC) $(INCLUDES) -c $<  
 
DataSourceFileStream.o: DataSourceFileStream.c 
	$(CC) $(INCLUDES) -c $<  
 
DataSourceSegment.o: DataSourceSegment.c 
	$(CC) $(INCLUDES) -c $<  
 
DataSourceTape.o: DataSourceTape.c 
	$(CC) $(INCLUDES) -c $<  
 
DataSourceWeb.o	: DataSourceWeb.c 
	$(CC) $(INCLUDES) -c $<  
 
DCE.o		: DCE.C
	$(CC) $(INCLUDES) -c $<  

DevError.o	: DevError.c 
	$(CC) $(INCLUDES) -c $<  
 
DevFileHeader.o	: DevFileHeader.c 
	$(CC) $(INCLUDES) -c $<  
 
DevStatus.o	: DevStatus.c 
	$(CC) $(INCLUDES) -c $<  
 
Exit.o		: Exit.c 
	$(CC) $(INCLUDES) -c $<  
 
Group.o		: Group.C 
	$(CC) $(INCLUDES) -c Group.C 
 
GroupDir.o	: GroupDir.C 
	$(CC) $(INCLUDES) -c  GroupDir.C 
 
ItemList.o	: ItemList.C 
	$(CC) $(INCLUDES) -c ItemList.C 

MemMgr.o	: MemMgr.C
	$(CC) $(INCLUDES) -c MemMgr.C 
 
Parse.o		: Parse.c 
	$(CC) $(INCLUDES) -c $< 
 
ProjectionList.o: ProjectionList.c 
	$(CC) $(INCLUDES) -c $<  
 
RecInterp.o	: RecInterp.c 
	$(CC) $(INCLUDES) -c $<  
 
TData.o		: TData.c 
	$(CC) $(INCLUDES) -c $<  
 
TData2DRec.o	: TData2DRec.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataAppend.o	: TDataAppend.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataAscii.o	: TDataAscii.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataAsciiInterp.o: TDataAsciiInterp.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataBinary.o	: TDataBinary.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataBinaryInterp.o: TDataBinaryInterp.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataMap.o	: TDataMap.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataMmap.o	: TDataMmap.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataRangeList.o: TDataRangeList.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataRangeSource.o: TDataRangeSource.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataRec.o	: TDataRec.c 
	$(CC) $(INCLUDES) -c $<  

TDataSeqAscii.o: TDataSeqAscii.c 
	$(CC) $(INCLUDES) -c $<  

TDataSeqAsciiInterp.o: TDataSeqAsciiInterp.c 
	$(CC) $(INCLUDES) -c $<  
 
TDataViewX.o	: TDataViewX.c 
	$(CC) $(INCLUDES) -c $<  
 
Time.o		: Time.c 
	$(CC) $(INCLUDES) -c $<  
 
TimeStamp.o	: TimeStamp.c 
	$(CC) $(INCLUDES) -c $<  
 
Timer.o		: Timer.c 
	$(CC) $(INCLUDES) -c $<  
 
Util.o		: Util.c 
	$(CC) $(INCLUDES) -c $<  
 
VectorArray.o	: VectorArray.c 
	$(CC) $(INCLUDES) -c $<  
 
Version.o	: Version.c 
	$(CC) $(INCLUDES) -c $<  
 
aptest2.o	: aptest2.c 
	$(CC) $(INCLUDES) -c $<  
 
show.o		: show.c 
	$(CC) $(INCLUDES) -c $<  
 
tapedrive.o	: tapedrive.C 
	$(CC) $(INCLUDES) -c tapedrive.C 
 
vector.o	: vector.c 
	$(CC) $(INCLUDES) -c $< 
 
clean	:  
	/bin/rm -f $(LIBOBJ) $(LIB)

