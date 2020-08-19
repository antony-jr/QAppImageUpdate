#include "rangedownloader.hpp"
#include "rangedownloader_p.hpp"

RangeDownloader::RangeDownloader(QObject *parent) 
		: QObject(parent) {
		m_Private.reset(new RangeDownloaderPrivate);
		auto obj = m_Private.data();

		connect(obj, &RangeDownloaderPrivate::started, 
			 this, &RangeDownloader::started,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::canceled, 
			 this, &RangeDownloader::canceled,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::finished, 
			this, &RangeDownloader::finished,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::error, 
			 this, &RangeDownloader::error,
			  Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::data, 
			 this, &RangeDownloader::data,
			 Qt::DirectConnection);

		connect(obj, &RangeDownloaderPrivate::rangeData, 
			 this, &RangeDownloader::rangeData,
			 Qt::DirectConnection);


}
