#pragma once

#include <QObject>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qqmlintegration.h>
#endif
class FirmwareUpdaterData :
		public QObject
{
	Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	QML_NAMED_ELEMENT(FirmwareUpdater)
	QML_UNCREATABLE("")
#endif
	Q_ENUMS(FirmwareType)
	Q_ENUMS(ImageType)
	Q_ENUMS(UpdaterState)

public:
	enum FirmwareType {
		FirmwareRelease,
		FirmwareCandidate,
		FirmwareTesting,
		FirmwareDevelop,
	};

	enum ImageType {
		ImageTypeNormal,
		ImageTypeLarge,
	};

	// NOTE: these states have an offset to prevent them from being negative,
	// since at least qtquick 1 can't handle those reliably.
	enum UpdaterState {
		Idle = 1000,
		Checking,
		DownloadingAndInstalling,
		Rebooting,

		UpdateFileNotFound = Idle - 3,
		ErrorDuringUpdating = Idle - 2,
		ErrorDuringChecking = Idle - 1,
	};
};
