#include <QAppImageUpdate>
#include <QDebug>
#include <libtorrent/version.hpp>

int main(int ac, char **av) {
	qInfo().noquote() << "Libtorrent Version Number: " << LIBTORRENT_VERSION_NUM;
	return 0;
}
