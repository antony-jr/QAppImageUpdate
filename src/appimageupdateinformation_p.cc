/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2018, Antony jr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @filename    : AppImageUpdateInformation_p.hpp
 * @description : This is where the extraction of embeded update information
 * from AppImages is implemented.
*/
#include <QBuffer>

#include "../include/appimageupdateinformation_p.hpp"

/*
 * An efficient logging system.
 * Warning: Hard coded to work only with this class.
*/
#ifndef LOGGING_DISABLED
#define LOGS *(p_Logger.data()) <<
#define LOGR <<
#define LOGE ; \
	     emit(logger(s_LogBuffer , s_AppImagePath)); \
	     s_LogBuffer.clear();
#else
#define LOGS (void)
#define LOGR ;(void)
#define LOGE ;
#endif // LOGGING_DISABLED

#define INFO_START LOGS "   INFO: " LOGR
#define INFO_END LOGE

#define WARNING_START LOGS "WARNING: " LOGR
#define WARNING_END LOGE

#define FATAL_START LOGS "  FATAL: " LOGR
#define FATAL_END LOGE


/*
 * Macros used for readbility and to reduce repeated code in the source.
 *
 * Warning:
 *     Hardcoded , Do not use this outside AppImageUpdateInformationPrivate
 *     class.
 *
 * Example:
 * 	MEMORY_ERROR()
*/
#define MEMORY_ERROR() emit(error(NotEnoughMemory));
#define APPIMAGE_OPEN_ERROR() emit(error(CannotOpenAppimage));
#define APPIMAGE_PERMISSION_ERROR() emit(error(NoReadPermission));
#define APPIMAGE_NOT_FOUND_ERROR() emit(error(AppimageNotFound));
#define APPIMAGE_READ_ERROR() emit(error(AppimageNotReadable));
#define APPIMAGE_INVALID_UI_ERROR() emit(error(InvalidUpdateInformation));
#define APPIMAGE_EMPTY_UI_ERROR() emit(error(EmptyUpdateInformation));
#define MAGIC_BYTES_ERROR() emit(error(InvalidMagicBytes));
#define ELF_FORMAT_ERROR() emit(error(UnsupportedElfFormat));
#define SECTION_HEADER_NOT_FOUND_ERROR() emit(error(SectionHeaderNotFound));
#define APPIMAGE_TYPE_ERROR() emit(error(InvalidAppimageType));
#define UNSUPPORTED_TRANSPORT_ERROR() emit(error(UnsupportedTransport));


/*
 * Sets the offset and length of the need section header
 * from a elf file.
 *
 * Example:
 *      long unsigned offset = 0 , length = 0;
 *      ElfXX_Ehdr *elfXX = (ElfXX_Ehdr *) data;
 *      ElfXX_Shdr *shdrXX = (ElfXX_Shdr *) (data + elfXX->e_shoff);
 *      strTab = (char *)(data + shdrXX[elfXX->e_shstrndx].sh_offset);
 *      lookupSectionHeaders(strTab , shdr , elf , ".section_header_name" , offset , length , progress);
 *
 * Note:
 * 	progress must be a Qt signal which takes int as a parameter.
 */
#define lookupSectionHeaders(strTab , shdr , elf , section , offset , length , progsig) \
						{ \
						for(int i = 0; i < elf->e_shnum; i++) { \
						  emit(progsig((int)((i * 100)/elf->e_shnum))); \
						  QCoreApplication::processEvents(); \
						  if(!strcmp(&strTab[shdr[i].sh_name] , section)){ \
							  offset = shdr[i].sh_offset; \
							  length = shdr[i].sh_size; \
							  emit(progsig(80)); \
							  break;\
						  } \
						 }\
					        }




using namespace AppImageUpdaterBridge;

/*
 * AppImage update information positions and magic values.
 * See https://github.com/AppImage/AppImageSpec/blob/master/draft.md
*/
static constexpr auto AppimageType1UpdateInfoPos = 0x8373;
static constexpr auto AppimageType1UpdateInfoLen = 0x200;
static constexpr auto AppimageType2UpdateInfoShdr = (char*)".upd_info";
static constexpr char AppimageUpdateInfoDelimiter = 0x7c;
static constexpr auto ElfMagicPos = 0x1;
static constexpr auto IsoMagicPos = 0x8001;
static constexpr auto ElfMagicValueSize= 0x4;
static constexpr auto IsoMagicValueSize= 0x6;
static const QByteArray ElfMagicValue = "ELF";
static const QByteArray IsoMagicValue = "CD001";

/*
 * e_ident[] identification indexes
 * See http://www.sco.com/developers/gabi/latest/ch4.eheader.html
 */
static constexpr auto EI_CLASS = 0x4; /* file class */
static constexpr auto EI_NIDENT = 0x10; /* Size of e_ident[] */

/* e_ident[] file class */
static constexpr auto ELFCLASS32 = 0x1; /* 32-bit objs */
static constexpr auto ELFCLASS64 = 0x2; /* 64-bit objs */

typedef quint8	Elf_Byte;
typedef quint32	Elf32_Addr;	/* Unsigned program address */
typedef quint32	Elf32_Off;	/* Unsigned file offset */
typedef quint32	Elf32_Sword;	/* Signed large integer */
typedef quint32	Elf32_Word;	/* Unsigned large integer */
typedef quint16	Elf32_Half;	/* Unsigned medium integer */

typedef quint64	Elf64_Addr;
typedef quint64	Elf64_Off;
typedef qint32	Elf64_Shalf;

typedef qint64	Elf64_Sword;
typedef quint64	Elf64_Word;

typedef qint64	Elf64_Sxword;
typedef quint64	Elf64_Xword;

typedef quint32	Elf64_Half;
typedef quint16	Elf64_Quarter;

/* ELF Header */
typedef struct elfhdr {
    unsigned char	e_ident[EI_NIDENT]; /* ELF Identification */
    Elf32_Half	e_type;		/* object file type */
    Elf32_Half	e_machine;	/* machine */
    Elf32_Word	e_version;	/* object file version */
    Elf32_Addr	e_entry;	/* virtual entry point */
    Elf32_Off	e_phoff;	/* program header table offset */
    Elf32_Off	e_shoff;	/* section header table offset */
    Elf32_Word	e_flags;	/* processor-specific flags */
    Elf32_Half	e_ehsize;	/* ELF header size */
    Elf32_Half	e_phentsize;	/* program header entry size */
    Elf32_Half	e_phnum;	/* number of program header entries */
    Elf32_Half	e_shentsize;	/* section header entry size */
    Elf32_Half	e_shnum;	/* number of section header entries */
    Elf32_Half	e_shstrndx;	/* section header table's "section
					   header string table" entry offset */
} Elf32_Ehdr;

typedef struct {
    unsigned char	e_ident[EI_NIDENT];	/* Id bytes */
    Elf64_Quarter	e_type;			/* file type */
    Elf64_Quarter	e_machine;		/* machine type */
    Elf64_Half	e_version;		/* version number */
    Elf64_Addr	e_entry;		/* entry point */
    Elf64_Off	e_phoff;		/* Program hdr offset */
    Elf64_Off	e_shoff;		/* Section hdr offset */
    Elf64_Half	e_flags;		/* Processor flags */
    Elf64_Quarter	e_ehsize;		/* sizeof ehdr */
    Elf64_Quarter	e_phentsize;		/* Program header entry size */
    Elf64_Quarter	e_phnum;		/* Number of program headers */
    Elf64_Quarter	e_shentsize;		/* Section header entry size */
    Elf64_Quarter	e_shnum;		/* Number of section headers */
    Elf64_Quarter	e_shstrndx;		/* String table index */
} Elf64_Ehdr;

/* Section Header */
typedef struct {
    Elf32_Word	sh_name;	/* name - index into section header
					   string table section */
    Elf32_Word	sh_type;	/* type */
    Elf32_Word	sh_flags;	/* flags */
    Elf32_Addr	sh_addr;	/* address */
    Elf32_Off	sh_offset;	/* file offset */
    Elf32_Word	sh_size;	/* section size */
    Elf32_Word	sh_link;	/* section header table index link */
    Elf32_Word	sh_info;	/* extra information */
    Elf32_Word	sh_addralign;	/* address alignment */
    Elf32_Word	sh_entsize;	/* section entry size */
} Elf32_Shdr;

typedef struct {
    Elf64_Half	sh_name;	/* section name */
    Elf64_Half	sh_type;	/* section type */
    Elf64_Xword	sh_flags;	/* section flags */
    Elf64_Addr	sh_addr;	/* virtual address */
    Elf64_Off	sh_offset;	/* file offset */
    Elf64_Xword	sh_size;	/* section size */
    Elf64_Half	sh_link;	/* link to another */
    Elf64_Half	sh_info;	/* misc info */
    Elf64_Xword	sh_addralign;	/* memory alignment */
    Elf64_Xword	sh_entsize;	/* table entry size */
} Elf64_Shdr;

struct AutoBoolCounter {
    AutoBoolCounter(bool *p)
        : p_Bool(p)
    {
        *p_Bool = true;
    }
    ~AutoBoolCounter()
    {
        *p_Bool = false;
    }

    void unlock()
    {
        *p_Bool = false;
    }
private:
    bool *p_Bool = nullptr;
};


/*
 * Returns a new QByteArray which contains the contents from the given QFile from the given offset to the given
 * max count. This function does not change the position of the QFile.
 *
 * Example:
 * 	QFile file("Some.AppImage")
 * 	file.open(QIODevice::ReadOnly);
 * 	QByteArray data = read(&file , 512 , 1024);
*/
static QByteArray read(QFile *IO, qint64 offset, qint64 max)
{
    QByteArray ret;
    qint64 before = IO->pos();
    IO->seek(offset);
    ret = IO->read(max);
    IO->seek(before);
    return ret;
}

static QByteArray readLine(QFile *IO)
{
    QByteArray ret;
    char c = 0;
    while(IO->getChar(&c) && c != '\n') {
        ret.append(c);
    }
    return ret;
}

static QByteArray getExecPathFromDesktopFile(QFile *file)
{
    QByteArray line;
    qint64 prevPos = file->pos();
    file->seek(0);
    while(!(line = readLine(file)).isEmpty()) {
        if(line.contains("Exec")) {
            for(auto i = 0; i < line.size() ; ++i) {
                if(line[i] == '=') {
                    line = line.mid(i+1);
                    break;
                }
            }
            break;
        }
    }
    file->seek(prevPos);
    return line;
}


/*
 * AppImageUpdateInformationPrivate is the worker class that provides the ability to easily get the update
 * information from an AppImage. This class can be constructed in two ways. The default construct sets the
 * QObject parent to be null and creates an empty AppImageUpdateInformationPrivate Object.
 *
 * Example:
 * 	QObject parent;
 * 	AppImageUpdateInformationPrivate AppImageInfoWithParent(&parent);
 *	AppImageUpdateInformationPrivate AppImageInfoWithoutParent;
*/
AppImageUpdateInformationPrivate::AppImageUpdateInformationPrivate(QObject *parent)
    : QObject(parent)
{
    emit statusChanged(Initializing);
#ifndef LOGGING_DISABLED
    try {
        p_Logger.reset(new QDebug(&s_LogBuffer));
    } catch ( ... ) {
        MEMORY_ERROR();
        throw;
    }
#endif // LOGGING_DISABLED
    emit statusChanged(Idle);
    return;
}

/*
 * Destructs the AppImageUpdateInformationPrivate , When the user provides the AppImage as a QFile ,
 * QFile is not closed , the user is fully responsible to deallocate or close the QFile.
*/
AppImageUpdateInformationPrivate::~AppImageUpdateInformationPrivate()
{
    return;
}

void AppImageUpdateInformationPrivate::setLoggerName(const QString &name)
{
    if(b_Busy) {
        return;
    }
#ifndef LOGGING_DISABLED
    s_LoggerName = QString(name);
#else
    (void)name;
#endif
    return;
}

/*
 * This method returns nothing and sets the AppImage referenced by the given QString , The QString is
 * expected to be a valid path either an absolute or a relative one. if the path is empty then this
 * exits doing nothing.
 *
 * Example:
 * 	AppImageUpdateInformationPrivate AppImageInfo;
 * 	AppImageInfo.setAppImage("PathTo.AppImage");
 *
 */
void AppImageUpdateInformationPrivate::setAppImage(const QString &AppImagePath)
{
    if(b_Busy) {
        return;
    }
    clear(); /* clear old data */
    if(AppImagePath.isEmpty()) {
        WARNING_START  " setAppImage : AppImagePath is empty , operation ignored." WARNING_END;
        return;
    }
    INFO_START  " setAppImage : " LOGR AppImagePath LOGR "." INFO_END;
    s_AppImagePath = AppImagePath;
    s_AppImageName = QFileInfo(AppImagePath).fileName();
    emit statusChanged(Idle);
    return;
}


/*
 * This is a overloaded method , Sets the AppImage with reference to the given QFile pointer ,
 * The given QFile has to be opened and must be readable.
 *
 * Example:
 * 	AppImageUpdateInformationPrivate AppImageInfo;
 * 	QFile file("PathTo.AppImage");
 * 	file.open(QIODevice::ReadOnly);
 * 	AppImageInfo.setAppImage(&file);
 * 	file.close();
*/
void AppImageUpdateInformationPrivate::setAppImage(QFile *AppImage)
{
    if(b_Busy) {
        return;
    }
    clear(); /* clear old data. */
    if(!AppImage) {
        WARNING_START " setAppImage : given AppImage QFile is nullptr , operation ignored. " WARNING_END
        return;
    }

    INFO_START  " setAppImage : " LOGR AppImage LOGR "." INFO_END;
    p_AppImage = AppImage;
    s_AppImagePath = QFileInfo(AppImage->fileName()).canonicalFilePath();
    s_AppImageName = QFileInfo(s_AppImagePath).fileName();
    emit statusChanged(Idle);
    return;
}

/*
 * If the given bool is true then it connects the logger signal to the
 * logPrinter slot to enable debugging messages. On false this disconnects the logPrinter.
 *
 * Example:
 * 	AppImageUpdateInformationPrivate AppImageInfo("PathTo.AppImage");
 * 	AppImageInfo.setShowLog(true);
*/
void AppImageUpdateInformationPrivate::setShowLog(bool logNeeded)
{
    if(b_Busy) {
        return;
    }
#ifndef LOGGING_DISABLED
    if(logNeeded) {
        connect(this, &AppImageUpdateInformationPrivate::logger,
                this, &AppImageUpdateInformationPrivate::handleLogMessage,
                Qt::UniqueConnection);
        return;
    }
    disconnect(this, &AppImageUpdateInformationPrivate::logger,
               this, &AppImageUpdateInformationPrivate::handleLogMessage);
#else
    (void)logNeeded;
#endif // LOGGING_DISABLED
}


void AppImageUpdateInformationPrivate::getInfo(void)
{
    if(b_Busy) {
        return;
    }
    AutoBoolCounter bc(&b_Busy);

    /*
    * Check if the user called this twice , If so , We don't need to waste our time on calculating the obvious.
    * Note: m_Info will always will be empty for a new AppImage , And so if it is not empty then that implies
    * that the user called getInfo() twice or more.
    */
    if(!m_Info.isEmpty()) {
        emit(info(m_Info));
        return;
    }

    /* If this class is constructed without an AppImage to operate on ,
     * Then lets guess it. */
    if(!p_AppImage && s_AppImagePath.isEmpty()) {
        /*
         * Check if QCoreApplication got something on argv[0]. The main payload is not the one we want to
         * operate this on but the AppImage itself , So we cannot use the actual application executable
         * path processed by qt , Instead we must use argv[0].
         *
        */
        auto arguments = QCoreApplication::arguments();
        if(!arguments.isEmpty()) {
            setAppImage(QFileInfo(arguments.at(0)).absolutePath() + QString::fromUtf8("/") + QFileInfo(arguments.at(0)).fileName());
        }
	
	
	if(s_AppImagePath.isEmpty()){
		emit(error(NoAppimagePathGiven));
		return;
	}
    }
    
    
    if(!p_AppImage) {
	/* Open appimage if the user only given the path. */
        try {
            p_AppImage = new QFile(this);
        } catch (...) {
            MEMORY_ERROR();
            return;
        }

        /*
             * Check if its really a file and not a folder.
            */
        if(!QFileInfo(s_AppImagePath).isFile()) {
            p_AppImage->deleteLater();
            p_AppImage = nullptr;
            emit statusChanged(Idle);
            FATAL_START " setAppImage : cannot use a directory as a file." FATAL_END;
            APPIMAGE_NOT_FOUND_ERROR();
            return;
        }

        p_AppImage->setFileName(s_AppImagePath);

        emit statusChanged(OpeningAppimage);
        QCoreApplication::processEvents();

        /* Check if the file actually exists. */
        if(!p_AppImage->exists()) {
            p_AppImage->deleteLater();
            p_AppImage = nullptr;
            emit statusChanged(Idle);
            FATAL_START  " setAppImage : cannot find the AppImage in the given path , file not found." FATAL_END;
            APPIMAGE_NOT_FOUND_ERROR();
            return;
        }

        /* Check if we have the permission to read it. */
        auto perm = p_AppImage->permissions();
        if(
            !(perm & QFileDevice::ReadUser) &&
            !(perm & QFileDevice::ReadGroup) &&
            !(perm & QFileDevice::ReadOther)
        ) {
            p_AppImage->deleteLater();
            p_AppImage = nullptr;
            emit statusChanged(Idle);
            FATAL_START  " setAppImage : no permission(" LOGR perm LOGR ") for reading the given AppImage." FATAL_END;
            APPIMAGE_PERMISSION_ERROR();
            return;
        }

        /*
         * Finally open the file.
        */
        if(!p_AppImage->open(QIODevice::ReadOnly)) {
            p_AppImage->deleteLater();
            p_AppImage = nullptr;
            emit statusChanged(Idle);
            FATAL_START  " setAppImage : cannot open AppImage for reading." FATAL_END;
            APPIMAGE_OPEN_ERROR();
            return;
        }
        emit statusChanged(Idle);
        QCoreApplication::processEvents();

    } else {
        emit statusChanged(OpeningAppimage);
        QCoreApplication::processEvents();

        /* Check if exists */
        if(!p_AppImage->exists()) {
            emit statusChanged(Idle);
            FATAL_START  " setAppImage : cannot find the AppImage from given QFile , file does not exists." FATAL_END;
            APPIMAGE_NOT_FOUND_ERROR();
            return;
        }

        /* Check if readable. */
        if(!p_AppImage->isReadable()) {
            emit statusChanged(Idle);
            FATAL_START  " setAppImage : invalid QFile given, not readable." FATAL_END;
            APPIMAGE_READ_ERROR();
            return;
        }

        /* Check if opened. */
        if(!p_AppImage->isOpen()) {
            emit statusChanged(Idle);
            FATAL_START  " setAppImage : invalid QFile given, not opened." FATAL_END;
            APPIMAGE_OPEN_ERROR();
            return;
        }

        emit statusChanged(Idle);
        QCoreApplication::processEvents();
    }


    QString AppImageSHA1;
    QString updateString;
    QStringList data;

    /*
     * Read the magic byte , i.e the AI stamp on the given binary. The characters 'AI'
     * are hardcoded at the offset 8 with a maximum of 3 characters.
     * The 3rd character decides the type of the AppImage.
    */
    emit statusChanged(ReadingAppimageMagicBytes);
    QCoreApplication::processEvents();

    auto magicBytes = read(p_AppImage, /*offset=*/8,/*maxchars=*/ 3);
    if (magicBytes[0] != 'A' && magicBytes[1] != 'I') {
        /*
         * If its not an AppImage then lets check if its a linux desktop file , If so then parse the 'Exec'
         * to find the actual AppImage.
        */
        magicBytes = read(p_AppImage, 0, 15);
        if(magicBytes == "[Desktop Entry]") {
            auto path = QString(getExecPathFromDesktopFile(p_AppImage));
            if(!path.isEmpty()) {
                if(QFileInfo(path).isRelative()) {
                    path = QFileInfo(p_AppImage->fileName()).path() + QString::fromUtf8("/") + QString(path);
                }
                bc.unlock(); /* unlock the bool counter. */
                setAppImage(path);
                getInfo();
                return;
            }
        }

        emit statusChanged(Idle);
        FATAL_START  " getInfo : invalid magic bytes("
        LOGR (unsigned)magicBytes[0] LOGR ","
        LOGR (unsigned)magicBytes[1] LOGR ")." FATAL_END;
        MAGIC_BYTES_ERROR();
        return;
    }



    /*
     * Calculate the AppImages SHA1 Hash which will be used later to find if we need to update the
     * AppImage.
    */
    emit statusChanged(CalculatingAppimageSha1Hash);
    QCoreApplication::processEvents();

    {
        qint64 bufferSize = 0;
        if(p_AppImage->size() >= 1073741824) { // 1 GiB and more.
            bufferSize = 104857600; // copy per 100 MiB.
        } else if(p_AppImage->size() >= 1048576 ) { // 1 MiB and more.
            bufferSize = 1048576; // copy per 1 MiB.
        } else if(p_AppImage->size() >= 1024) { // 1 KiB and more.
            bufferSize = 4096; // copy per 4 KiB.
        } else { // less than 1 KiB
            bufferSize = 1024; // copy per 1 KiB.
        }

        QCryptographicHash *SHA1Hasher = new QCryptographicHash(QCryptographicHash::Sha1);
        while(!p_AppImage->atEnd()) {
            SHA1Hasher->addData(p_AppImage->read(bufferSize));
            QCoreApplication::processEvents();
        }
        p_AppImage->seek(0); // rewind file to the top for later use.
        AppImageSHA1 = QString(SHA1Hasher->result().toHex().toUpper());
        delete SHA1Hasher;
    }

    emit statusChanged(FindingAppimageType);
    QCoreApplication::processEvents();

    /*
     * 0x1H -> Type 1 AppImage.
     * 0x2H -> Type 2 AppImage. (Latest Version)
    */
    int type = (int)magicBytes[2];
    if(type == 0x1) {
        INFO_START  " getInfo : AppImage is confirmed to be type 1." INFO_END;
        progress(/*percentage=*/80); /*Signal progress.*/
        emit statusChanged(ReadingAppimageUpdateInformation);
        QCoreApplication::processEvents();

        updateString = QString::fromUtf8(read(p_AppImage, AppimageType1UpdateInfoPos, AppimageType1UpdateInfoLen));
    } else if(type == 0x2) {

        INFO_START  " getInfo : AppImage is confirmed to be type 2." INFO_END;
        INFO_START  " getInfo : mapping AppImage to memory." INFO_END;

        {
            uint8_t *data = NULL;
            char *strTab = NULL;
            uchar *mapped = NULL;
            unsigned long offset = 0, length = 0;

            emit statusChanged(MappingAppimageToMemory);
            mapped = p_AppImage->map(/*offset=*/0, /*max=*/p_AppImage->size());

            if(mapped == NULL) {
                emit statusChanged(Idle);
                FATAL_START  " getInfo : not enough memory to map AppImage to memory." FATAL_END;
                MEMORY_ERROR();
                return;
            }

            emit statusChanged(FindingAppimageArchitecture);
            QCoreApplication::processEvents();
            data = (uint8_t*) mapped;

            if((((Elf32_Ehdr*)data)->e_ident[EI_CLASS] == ELFCLASS32)) {
                INFO_START  " getInfo : AppImage architecture is x86 (32 bits)." INFO_END;

                Elf32_Ehdr *elf32 = (Elf32_Ehdr *) data;
                Elf32_Shdr *shdr32 = (Elf32_Shdr *) (data + elf32->e_shoff);

                strTab = (char *)(data + shdr32[elf32->e_shstrndx].sh_offset);
                emit statusChanged(SearchingForUpdateInformationSectionHeader);

                lookupSectionHeaders(strTab, shdr32, elf32, AppimageType2UpdateInfoShdr,
                                     /*variable to set offset=*/offset, /*length of the header=*/length,
                                     /*Progress signal to use=*/progress);
            } else if((((Elf64_Ehdr*)data)->e_ident[EI_CLASS] == ELFCLASS64)) {
                INFO_START  " getInfo : AppImage architecture is x86_64 (64 bits)." INFO_END;

                Elf64_Ehdr *elf64 = (Elf64_Ehdr *) data;
                Elf64_Shdr *shdr64 = (Elf64_Shdr *) (data + elf64->e_shoff);

                strTab = (char *)(data + shdr64[elf64->e_shstrndx].sh_offset);
                emit statusChanged(SearchingForUpdateInformationSectionHeader);
                lookupSectionHeaders(strTab, shdr64, elf64, AppimageType2UpdateInfoShdr,
                                     offset, length, progress);
            } else {
                emit statusChanged(UnmappingAppimageFromMemory);
                p_AppImage->unmap(mapped);
                emit statusChanged(Idle);
                FATAL_START  " getInfo : Unsupported elf format." FATAL_END;
                ELF_FORMAT_ERROR();
                return;
            }

            emit statusChanged(UnmappingAppimageFromMemory);
            p_AppImage->unmap(mapped);

            if(offset == 0 || length == 0) {
                emit statusChanged(Idle);
                FATAL_START  " getInfo : cannot find '"
                LOGR AppimageType2UpdateInfoShdr LOGR "' section header." FATAL_END;
                SECTION_HEADER_NOT_FOUND_ERROR();
            } else {
                emit statusChanged(ReadingAppimageUpdateInformation);
                updateString = QString::fromUtf8(read(p_AppImage, offset, length));
            }

            emit statusChanged(Idle);
        }
    } else {
        WARNING_START  " getInfo : unable to confirm AppImage type." WARNING_END;
        if(
            (read(p_AppImage, ElfMagicPos, ElfMagicValueSize) == ElfMagicValue) &&
            (read(p_AppImage, IsoMagicPos, IsoMagicValueSize) == IsoMagicValue)
        ) {
            WARNING_START  " getInfo : guessing AppImage type to be 1." WARNING_END;
            emit(progress(80));
            emit statusChanged(ReadingAppimageUpdateInformation);
            updateString = QString::fromUtf8(read(p_AppImage, AppimageType1UpdateInfoPos, AppimageType1UpdateInfoLen));
        } else {
            emit statusChanged(Idle);
            FATAL_START  " getInfo : invalid AppImage type(" LOGR type LOGR ")." FATAL_END;
            APPIMAGE_TYPE_ERROR();
            return;
        }
    }

    emit statusChanged(Idle);
    QCoreApplication::processEvents();

    if(updateString.isEmpty()) {
        FATAL_START  " getInfo : update information is empty." FATAL_END;
        APPIMAGE_EMPTY_UI_ERROR();
        return;
    }

    INFO_START " getInfo : updateString(" LOGR updateString LOGR ")." INFO_END;

    /*
     * Split the raw update information with the specified
     * delimiter.
    */
    data = updateString.split(AppimageUpdateInfoDelimiter);

    // This will be sent along the update information.
    QJsonObject fileInformation {
        { "AppImageFilePath", s_AppImagePath },
        { "AppImageSHA1Hash", AppImageSHA1 }
    };

    QJsonObject updateInformation; // will be filled up later on.

    emit statusChanged(FinalizingAppimageEmbededUpdateInformation);

    if(data.size() < 2) {
        emit statusChanged(Idle);
        FATAL_START  " getInfo : update information has invalid delimiters." FATAL_END;
        APPIMAGE_INVALID_UI_ERROR();
        return;
    } else if(data.size() == 2) {
        {
            QJsonObject buffer {
                { "transport", data.at(0) },
                { "zsyncUrl", data.at(1) }
            };
            updateInformation = buffer;
        }
    } else if(data.size() == 5) {
        if(data.at(0) == "gh-releases-zsync") {
            {
                QJsonObject buffer {
                    {"transport", data.at(0) },
                    {"username", data.at(1) },
                    {"repo", data.at(2) },
                    {"tag", data.at(3) },
                    {"filename", data.at(4) }
                };
                updateInformation = buffer;
            }
        } else if(data.at(0) == "bintray-zsync") {
            {
                QJsonObject buffer {
                    {"transport", data.at(0) },
                    {"username", data.at(1) },
                    {"repo", data.at(2) },
                    {"packageName", data.at(3) },
                    {"filename", data.at(4) }
                };
                updateInformation = buffer;
            }
        } else {
            emit statusChanged(Idle);
            FATAL_START  " getInfo : unsupported transport mechanism given." FATAL_END;
            UNSUPPORTED_TRANSPORT_ERROR();
            return;
        }

    } else {
        emit statusChanged(Idle);
        FATAL_START " getInfo : update information has invalid number of entries(" LOGR data.size() LOGR ")." FATAL_END;
        APPIMAGE_INVALID_UI_ERROR();
        return;
    }

    {
        QJsonObject buffer {
            { "IsEmpty", updateInformation.isEmpty() },
            { "FileInformation", fileInformation },
            { "UpdateInformation", updateInformation }
        };
        m_Info = buffer;
    }

    emit statusChanged(Idle);
    emit(progress(100)); /*Signal progress.*/
    emit(info(m_Info));
    INFO_START  " getInfo : finished." INFO_END;
    return;
}

/*
 * This clears all the data held in the current object , making it
 * reusable.
 *
 * Example:
 * 	AppImageInfo.clear();
*/
void AppImageUpdateInformationPrivate::clear(void)
{
    if(b_Busy) {
        return;
    }
    m_Info = QJsonObject(); /* TODO: if QJsonObject has a clear in future , use it instead. */
#ifndef LOGGING_DISABLED
    s_LogBuffer.clear();
#endif
    s_AppImagePath.clear();
    s_AppImageName.clear();
    p_AppImage = nullptr; /* Drop all responsibilities. */
    return;
}

#ifndef LOGGING_DISABLED
/* This private slot proxies the log messages from the logger signal to qInfo(). */
void AppImageUpdateInformationPrivate::handleLogMessage(QString msg, QString path)
{
    (void)path;
    qInfo().noquote()  << "["
                       <<  QDateTime::currentDateTime().toString(Qt::ISODate)
                       << "] "
                       << s_LoggerName
                       << "("
                       << s_AppImageName << ")::" << msg;
    return;
}
#endif // LOGGING_DISABLED

/*
 * This static method returns a QString with respect to error codes ,Useful when logging and debuging.
 *
 * Example:
 * 	qDebug()
 * 	<< AppImageUpdateInformationPrivate::errorCodeToString(AppImageUpdateInformationPrivate::AppImageNotFound);
*/
QString AppImageUpdateInformationPrivate::errorCodeToString(short errorCode)
{
    QString ret = "AppImageUpdaterBridge::errorCode(";
    switch(errorCode) {
    case AppimageNotReadable:
        ret += "AppImageNotReadable";
        break;
    case NoReadPermission:
        ret += "NoReadPermission";
        break;
    case AppimageNotFound:
        ret += "AppimageNotFound";
        break;
    case CannotOpenAppimage:
        ret += "CannnotOpenAppimage";
        break;
    case EmptyUpdateInformation:
        ret += "EmptyUpdateInformation";
        break;
    case InvalidAppimageType:
        ret += "InvalidAppimageType";
        break;
    case InvalidMagicBytes:
        ret += "InvalidMagicBytes";
        break;
    case InvalidUpdateInformation:
        ret += "InvalidUpdateInformation";
        break;
    case NotEnoughMemory:
        ret += "NotEnoughMemory";
        break;
    case SectionHeaderNotFound:
        ret += "SectionHeaderNotFound";
        break;
    case UnsupportedElfFormat:
        ret += "UnsupportedElfFormat";
        break;
    case UnsupportedTransport:
        ret += "UnsupportedTransport";
        break;
    default:
        ret += "Unknown";
        break;
    }
    ret += ")";
    return ret;
}

QString AppImageUpdateInformationPrivate::statusCodeToString(short code)
{
    QString ret = "AppImageUpdaterBridge::statusCode(";
    switch(code) {
    case Initializing:
        ret += "Initializing";
        break;
    case Idle:
        ret += "Idle";
        break;
    case OpeningAppimage:
        ret += "OpeningAppimage";
        break;
    case CalculatingAppimageSha1Hash:
        ret += "CalculatingAppimageSha1Hash";
        break;
    case ReadingAppimageMagicBytes:
        ret += "ReadingAppimageMagicBytes";
        break;
    case FindingAppimageType:
        ret += "FindingAppimageType";
        break;
    case FindingAppimageArchitecture:
        ret += "FindingAppimageArchitecture";
        break;
    case MappingAppimageToMemory:
        ret += "MappingAppimageToMemory";
        break;
    case ReadingAppimageUpdateInformation:
        ret += "ReadingAppimageUpdateInformation";
        break;
    case SearchingForUpdateInformationSectionHeader:
        ret += "SearchingForUpdateInformationSectionHeader";
        break;
    case UnmappingAppimageFromMemory:
        ret += "UnmappingAppimageFromMemory";
        break;
    case FinalizingAppimageEmbededUpdateInformation:
        ret += "FinalizingAppimageEmbededUpdateInformation";
        break;
    default:
        ret += "Unknown";
    }
    ret += ")";
    return ret;
}
