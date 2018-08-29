#ifndef MY_UPDATE_WIDGET_HPP_INCLUDED
#define MY_UDPATE_WIDGET_HPP_INCLUDED
#include <AppImageUpdaterWidget.hpp>
#include <QMessageBox>

class MyUpdateWidget : public AppImageUpdaterBridge::AppImageUpdaterWidget
{
	Q_OBJECT
public:
	MyUpdateWidget(QWidget *parent = nullptr)
	: AppImageUpdaterBridge::AppImageUpdaterWidget(0 , parent)
	{
		return;
	}

	bool continueWithUpdate(QJsonObject info)
	{
		QMessageBox box(this);
		box.setText(QString::fromUtf8("A New version of ") + 
			    info["AppImageFilePath"].toString() + 
			    QString::fromUtf8(" is available , Do you want to update ?"));
		box.addButton(QMessageBox::Yes);
		box.addButton(QMessageBox::No);
		box.setWindowTitle("Update Available!");
		return (box.exec() == QMessageBox::Yes);
	}

	bool openNewVersion(QJsonObject info)
	{
		QMessageBox box(this);
		box.setText(QString::fromUtf8("Update completed successfully! The new version is at ") + 
			    info["AbsolutePath"].toString() + 
			    QString::fromUtf8(" , Do you want to open it ?"));
		box.addButton(QMessageBox::Yes);
		box.addButton(QMessageBox::No);
		box.setWindowTitle("Update Complete!");
		return (box.exec() == QMessageBox::Yes);
	}
};



#endif
