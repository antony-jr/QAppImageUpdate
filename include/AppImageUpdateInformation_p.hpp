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
 * from AppImages is described.
*/
#ifndef APPIMAGE_UPDATE_INFORMATION_PRIVATE_HPP_INCLUDED
#define APPIMAGE_UPDATE_INFORMATION_PRIVATE_HPP_INCLUDED
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QtGlobal>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QSharedPointer>

namespace AppImageUpdaterBridge {
class AppImageUpdateInformationPrivate : public QObject
{
    Q_OBJECT
public:
    enum : short {
        APPIMAGE_NOT_READABLE = 1,
        NO_READ_PERMISSION,
        APPIMAGE_NOT_FOUND,
        CANNOT_OPEN_APPIMAGE,
        EMPTY_UPDATE_INFORMATION,
        INVALID_APPIMAGE_TYPE,
        INVALID_MAGIC_BYTES,
        INVALID_UPDATE_INFORMATION,
        NOT_ENOUGH_MEMORY,
        SECTION_HEADER_NOT_FOUND,
        UNSUPPORTED_ELF_FORMAT,
        UNSUPPORTED_TRANSPORT
    } error_code;

    enum : short {
	INITIALIZING = 0,
	IDLE = 1,
	OPENING_APPIMAGE,
	CALCULATING_APPIMAGE_SHA1_HASH,
	READING_APPIMAGE_MAGIC_BYTES,
	FINDING_APPIMAGE_TYPE,
	FINDING_APPIMAGE_ARCHITECTURE,
	MAPPING_APPIMAGE_TO_MEMORY,
	READING_APPIMAGE_UPDATE_INFORMATION,
	SEARCHING_FOR_UPDATE_INFORMATION_SECTION_HEADER,
	UNMAPPING_APPIMAGE_FROM_MEMORY,
	FINALIZING_APPIMAGE_EMBEDED_UPDATE_INFORMATION
    } status_code;

    explicit AppImageUpdateInformationPrivate(QObject *parent = nullptr);
    ~AppImageUpdateInformationPrivate();

    /* Public static methods. */
    static QString errorCodeToString(short);
    static QString statusCodeToString(short);

public Q_SLOTS:
    void setAppImage(const QString&);
    void setAppImage(QFile *);
#ifndef LOGGING_DISABLED
    void setShowLog(bool);
    void setLoggerName(const QString&);
#endif // LOGGING_DISABLED
    void getInfo(void);
    void clear(void);

#ifndef LOGGING_DISABLED
private Q_SLOTS:
    void handleLogMessage(QString , QString);
#endif // LOGGING_DISABLED

Q_SIGNALS:
    void info(QJsonObject);
    void progress(int);
    void error(short);
    void statusChanged(short);
#ifndef LOGGING_DISABLED
    void logger(QString , QString);
#endif // LOGGING_DISABLED
private:
    QJsonObject _jInfo;
    QString _sAppImageName,
            _sAppImagePath,
#ifndef LOGGING_DISABLED
	    _sLogBuffer,
	    _sLoggerName,
#endif // LOGGING_DISABLED
	    _sAppImageSHA1;
#ifndef LOGGING_DISABLED
    QSharedPointer<QDebug> _pLogger = nullptr;
#endif // LOGGING_DISABLED
    QSharedPointer<QFile>  _pAppImage = nullptr;
   
    /*
     * AppImage update information positions and magic values.
     * See https://github.com/AppImage/AppImageSpec/blob/master/draft.md
    */
    static constexpr auto APPIMAGE_TYPE1_UPDATE_INFO_POS = 0x8373;
    static constexpr auto APPIMAGE_TYPE1_UPDATE_INFO_LEN = 0x200;
    static constexpr auto APPIMAGE_TYPE2_UPDATE_INFO_SHDR = (char*)".upd_info";
    static constexpr char APPIMAGE_UPDATE_INFO_DELIMITER = 0x7c;
    static constexpr auto ELF_MAGIC_POS = 0x1;
    static constexpr auto ISO_MAGIC_POS = 0x8001;
    static constexpr auto ELF_MAGIC_VALUE_SIZE = 0x4;
    static constexpr auto ISO_MAGIC_VALUE_SIZE = 0x6;
    const QByteArray ELF_MAGIC_VALUE = "ELF";
    const QByteArray ISO_MAGIC_VALUE = "CD001";

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
};
}
#endif // APPIMAGE_UPDATE_INFORMATION_PRIVATE_HPP_INCLUDED
