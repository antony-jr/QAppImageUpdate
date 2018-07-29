#ifndef APPIMAGE_INSPECTOR_HPP_INCLUDED
#define APPIMAGE_INSPECTOR_HPP_INCLUDED
#include <QtCore>
#include <ZsyncRemoteControlFileParser_p.hpp>
#include <AppImageUpdateInformation.hpp>

namespace AppImageUpdaterBridge {
	class AppImageInspector : public QObject {
		Q_OBJECT
	public:
	    explicit AppImageInspector(AppImageUpdateInformation *updateInformation = nullptr,
			    	       QNetworkAccessManager *networkManager = nullptr);
	    ~AppImageInspector();
	
	public Q_SLOTS:
	    bool isEmpty(void);
	    bool isUpdatesAvailable(void);
	    void clear(void);
	    void setShowLog(bool);
	    void checkForUpdates(void);
	    void getTargetFileCheckSumBlocks(void);
	    size_t getTargetFileBlocksCount(void);
    	    QUrl getControlFileUrl(void);
    	    QString getZsyncMakeVersion(void);
    	    QString getTargetFileName(void);
    	    QUrl getTargetFileUrl(void);
    	    QString getTargetFileSHA1(void);
    	    QDateTime getMTime(void);
    	    size_t getTargetFileBlockSize(void);
    	    size_t getTargetFileLength(void);
    	    qint32 getWeakCheckSumBytes(void);
    	    qint32 getStrongCheckSumBytes(void);
    	    qint32 getConsecutiveMatchNeeded(void);

	private Q_SLOTS:
	    void handleUpdateInformation(QJsonObject);
	    void handleUpdateInformationError(short);
	
        Q_SIGNALS:
	    void targetFileCheckSumBlock(Private::zs_blockid , Private::rsum , void*);
	    void endOfTargetFileCheckSumBlocks(void);
	    void updatesAvailable(bool);
	    void progress(int);
	    void error(short);
	    void logger(QString , QString); /* log msg , control file name. */
	
	private:
	    QSharedPointer<Private::ZsyncRemoteControlFileParserPrivate> _pControlFileParser = nullptr;
	    AppImageUpdateInformation *_pUpdateInformation = nullptr;
	};
}

#endif // APPIMAGE_INSPECTOR_HPP_INCLUDED
