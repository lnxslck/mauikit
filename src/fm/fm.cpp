/*
 *   Copyright 2018 Camilo Higuita <milo.h@aol.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "fm.h"
#include "utils.h"
#include "tagging.h"
#include "syncing.h"

#include <QObject>

#include <QFlags>
#include <QDateTime>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QLocale>
#include <QRegularExpression>

#if defined(Q_OS_ANDROID)
#include "mauiandroid.h"
#else
#include "mauikde.h"
#endif

/*
FM *FM::instance = nullptr;

FM* FM::getInstance()
{
    if(!instance)
    {
        instance = new FM();
        qDebug() << "getInstance(): First instance\n";
        instance->init();
        return instance;
    } else
    {
        qDebug()<< "getInstance(): previous instance\n";
        return instance;
    }
}*/

void FM::init()
{
    this->tag = Tagging::getInstance();
    this->sync = new Syncing(this);
    connect(this->sync, &Syncing::listReady, [this](const FMH::MODEL_LIST &list, const QString &url)
    {
        emit this->cloudServerContentReady(list, url);
    });
	
	connect(this->sync, &Syncing::itemReady, [this](const FMH::MODEL &item, const QString &url, const Syncing::SIGNAL_TYPE &signalType)
	{		
		switch(signalType)
		{
			case Syncing::SIGNAL_TYPE::OPEN:
				this->openUrl(item[FMH::MODEL_KEY::PATH]);
				break;
				
			case Syncing::SIGNAL_TYPE::DOWNLOAD:
				emit this->cloudItemReady(item, url);
				break;
				
			case Syncing::SIGNAL_TYPE::COPY:
			{
				QVariantMap data;
				for(auto key : item.keys())
					data.insert(FMH::MODEL_NAME[key], item[key]);				
				
				this->copy(QVariantList {data}, this->sync->getCopyTo());
				break;	
			}	
			default: return;
		}
	});
	
	connect(this->sync, &Syncing::error, [this](const QString &message)
	{		
		emit this->warningMessage(message);
	});
	
	connect(this->sync, &Syncing::progress, [this](const int &percent)
	{		
		emit this->loadProgress(percent);
	});
	
	connect(this->sync, &Syncing::dirCreated, [this](const FMH::MODEL &dir, const QString &url)
	{		
		emit this->newItem(dir, url);
	});
	
	connect(this->sync, &Syncing::uploadReady, [this](const FMH::MODEL &item, const QString &url)
	{		
		emit this->newItem(item, url);
	});
}

FM::FM(QObject *parent) : FMDB(parent) 
{
	this->init();
}

FM::~FM() {}

QVariantMap FM::toMap(const FMH::MODEL& model)
{
	QVariantMap map;
	for(auto key : model.keys())
		map.insert(FMH::MODEL_NAME[key], model[key]);
	
	return map;		
}

FMH::MODEL_LIST FM::packItems(const QStringList &items, const QString &type)
{
    FMH::MODEL_LIST data;

    for(auto path : items)
        if(UTIL::fileExists(path))
        {
            auto model = FMH::getFileInfoModel(path);
            model.insert(FMH::MODEL_KEY::TYPE, type);
            data << model;
        }

    return data;
}

QVariantList FM::get(const QString &queryTxt)
{
    QVariantList mapList;

    auto query = this->getQuery(queryTxt);

    if(query.exec())
    {
        while(query.next())
        {
            QVariantMap data;
            for(auto key : FMH::MODEL_NAME.keys())
                if(query.record().indexOf(FMH::MODEL_NAME[key]) > -1)
                    data[FMH::MODEL_NAME[key]] = query.value(FMH::MODEL_NAME[key]).toString();

            mapList<< data;

        }

    }else qDebug()<< query.lastError()<< query.lastQuery();

    return mapList;
}

FMH::MODEL_LIST FM::getPathContent(const QString& path, const bool &hidden, const bool &onlyDirs, const QStringList& filters, const QDirIterator::IteratorFlags &iteratorFlags)
{
    FMH::MODEL_LIST content;

    if (FM::isDir(path))
    {
        QDir::Filters dirFilter;

        dirFilter = (onlyDirs ? QDir::AllDirs | QDir::NoDotDot | QDir::NoDot :
                                QDir::Files | QDir::AllDirs | QDir::NoDotDot | QDir::NoDot);

        if(hidden)
            dirFilter = dirFilter | QDir::Hidden | QDir::System;

        QDirIterator it (path, filters, dirFilter, iteratorFlags);
        while (it.hasNext())
        {
            auto url = it.next();
            content << FMH::getFileInfoModel(url);
        }
    }

    return content;
}

FMH::MODEL_LIST FM::getAppsContent(const QString& path)
{
    FMH::MODEL_LIST res;
#if (defined (Q_OS_LINUX) && !defined (Q_OS_ANDROID))
    if(path.startsWith(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::APPS_PATH]+"/"))
        return MAUIKDE::getApps(QString(path).replace(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::APPS_PATH]+"/",""));
    else
        return MAUIKDE::getApps();
#endif
    return res;
}

FMH::MODEL_LIST FM::getDefaultPaths()
{
    return packItems(FMH::defaultPaths, FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::PLACES_PATH]);
}

FMH::MODEL_LIST FM::getCustomPaths()
{
#ifdef Q_OS_ANDROID
    return FMH::MODEL_LIST();
#endif

    return FMH::MODEL_LIST
    {
        FMH::MODEL
        {
            {FMH::MODEL_KEY::ICON, "system-run"},
            {FMH::MODEL_KEY::LABEL, FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::APPS_PATH]},
            {FMH::MODEL_KEY::PATH, FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::APPS_PATH]+"/"},
            {FMH::MODEL_KEY::TYPE, FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::PLACES_PATH]}
        }
    };
}

FMH::MODEL_LIST FM::search(const QString& query, const QString &path, const bool &hidden, const bool &onlyDirs, const QStringList &filters)
{
    FMH::MODEL_LIST content;

    if (FM::isDir(path))
    {
        QDir::Filters dirFilter;

        dirFilter = (onlyDirs ? QDir::AllDirs | QDir::NoDotDot | QDir::NoDot :
                                QDir::Files | QDir::AllDirs | QDir::NoDotDot | QDir::NoDot);

        if(hidden)
            dirFilter = dirFilter | QDir::Hidden | QDir::System;

        QDirIterator it (path, filters, dirFilter, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            auto url = it.next();
            auto info = it.fileInfo();
            qDebug()<< info.completeBaseName() <<  info.completeBaseName().contains(query);
            if(info.completeBaseName().contains(query, Qt::CaseInsensitive))
            {
                content << FMH::getFileInfoModel(url);
            }
        }
    }

    qDebug()<< content;
    return content;
}

FMH::MODEL_LIST FM::getDevices()
{
    FMH::MODEL_LIST drives;

#if defined(Q_OS_ANDROID)
    drives << packItems({MAUIAndroid::sdDir()}, FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::DRIVES_PATH]);
    return drives;
#else
    return drives;
#endif

    //     auto devices = QStorageInfo::mountedVolumes();
    //     for(auto device : devices)
    //     {
    //         if(device.isValid() && !device.isReadOnly())
    //         {
    //             QVariantMap drive =
    //             {
    //                 {FMH::MODEL_NAME[FMH::MODEL_KEY::ICON], "drive-harddisk"},
    //                 {FMH::MODEL_NAME[FMH::MODEL_KEY::LABEL], device.displayName()},
    //                 {FMH::MODEL_NAME[FMH::MODEL_KEY::PATH], device.rootPath()},
    //                 {FMH::MODEL_NAME[FMH::MODEL_KEY::TYPE], FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::DRIVES]}
    //             };
    //
    //             drives << drive;
    //         }
    //     }

    //    for(auto device : QDir::drives())
    //    {
    //        QVariantMap drive =
    //        {
    //            {"iconName", "drive-harddisk"},
    //            {"label", device.baseName()},
    //            {"path", device.absoluteFilePath()},
    //            {"type", "Drives"}
    //        };

    //        drives << drive;
    //    }

    return drives;
}

FMH::MODEL_LIST FM::getTags(const int &limit)
{
    Q_UNUSED(limit);

    FMH::MODEL_LIST data;

    if(this->tag)
    {
        for(auto tag : this->tag->getUrlsTags(false))
        {
            qDebug()<< "TAG << "<< tag;
            auto label = tag.toMap().value(TAG::KEYMAP[TAG::KEYS::TAG]).toString();
            data << FMH::MODEL
            {
            {FMH::MODEL_KEY::PATH, label},
            {FMH::MODEL_KEY::ICON, "tag"},
            {FMH::MODEL_KEY::LABEL, label},
            {FMH::MODEL_KEY::TYPE,  FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::TAGS_PATH]}
        };
        }
    }

    return data;
}

FMH::MODEL_LIST FM::getBookmarks()
{
    QStringList bookmarks;
    for(auto bookmark : this->get("select * from bookmarks"))
        bookmarks << bookmark.toMap().value(FMH::MODEL_NAME[FMH::MODEL_KEY::PATH]).toString();

    return packItems(bookmarks, FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::BOOKMARKS_PATH]);
}

bool FM::getCloudServerContent(const QString &path, const QStringList &filters, const int &depth)
{
    auto user = path.split("/")[1];
   
    auto data = this->get(QString("select * from clouds where user = '%1'").arg(user));

    if(data.isEmpty())
        return false;
	
    auto map = data.first().toMap();

    user = map[FMH::MODEL_NAME[FMH::MODEL_KEY::USER]].toString();
    auto server = map[FMH::MODEL_NAME[FMH::MODEL_KEY::SERVER]].toString();
    auto password = map[FMH::MODEL_NAME[FMH::MODEL_KEY::PASSWORD]].toString();
    this->sync->setCredentials(server, user, password);

    this->sync->listContent(path, filters, depth);
	return true;
}

FMH::MODEL_LIST FM::getCloudAccounts()
{
    auto accounts = this->get("select * from clouds");
    FMH::MODEL_LIST res;
    for(auto account : accounts)
    {
        auto map = account.toMap();
        res << FMH::MODEL {
        {FMH::MODEL_KEY::PATH, QStringLiteral("Cloud/")+map[FMH::MODEL_NAME[FMH::MODEL_KEY::USER]].toString()},
        {FMH::MODEL_KEY::ICON, "folder-cloud"},
		{FMH::MODEL_KEY::LABEL, map[FMH::MODEL_NAME[FMH::MODEL_KEY::USER]].toString()},
		{FMH::MODEL_KEY::USER, map[FMH::MODEL_NAME[FMH::MODEL_KEY::USER]].toString()},
		{FMH::MODEL_KEY::SERVER, map[FMH::MODEL_NAME[FMH::MODEL_KEY::SERVER]].toString()},
		{FMH::MODEL_KEY::TYPE,  FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::CLOUD_PATH]}};
}
return res;
}

void FM::createCloudDir(const QString &path, const QString &name)
{
	qDebug()<< "trying to create folder at"<< path;
	this->sync->createDir(path, name);
}

void FM::openCloudItem(const QVariantMap &item)
{
	qDebug()<< item;
	FMH::MODEL data;
	for(auto key : item.keys())
		data.insert(FMH::MODEL_NAME_KEY[key], item[key].toString());
	
	this->sync->resolveFile(data, Syncing::SIGNAL_TYPE::OPEN);
}

void FM::getCloudItem(const QVariantMap &item)
{
	qDebug()<< item;
	FMH::MODEL data;
	for(auto key : item.keys())
		data.insert(FMH::MODEL_NAME_KEY[key], item[key].toString());
	
	this->sync->resolveFile(data, Syncing::SIGNAL_TYPE::DOWNLOAD);
}

QVariantList FM::getCloudAccountsList()
{
	QVariantList res;
	
	auto data = this->getCloudAccounts();
	
	for(auto item : data)
	{
		QVariantMap map;
		
		for(auto key : item.keys())
			map.insert(FMH::MODEL_NAME[key], item[key]);
		
		res << map;		
	}
	
	return res;
}

bool FM::addCloudAccount(const QString &server, const QString &user, const QString &password)
{
    QVariantMap account = {
        {FMH::MODEL_NAME[FMH::MODEL_KEY::SERVER], server},
        {FMH::MODEL_NAME[FMH::MODEL_KEY::USER], user},
        {FMH::MODEL_NAME[FMH::MODEL_KEY::PASSWORD], password}
    };

    if(this->insert(FMH::TABLEMAP[FMH::TABLE::CLOUDS], account))
    {
		emit this->cloudAccountInserted(user);
		return true;
	}
	
	return false;
}

bool FM::removeCloudAccount(const QString &server, const QString &user)
{
	FMH::DB account = {
		{FMH::MODEL_KEY::SERVER, server},
		{FMH::MODEL_KEY::USER, user},
	};
	
	if(this->remove(FMH::TABLEMAP[FMH::TABLE::CLOUDS], account))
	{
		emit this->cloudAccountRemoved(user);
		return true;
	}
	
	return false;
}

QString FM::resolveUserCloudCachePath(const QString &server, const QString &user)
{
	return FMH::CloudCachePath+"opendesktop/"+user;
}

QString FM::resolveLocalCloudPath(const QString& path)
{
	return QString(path).replace(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::CLOUD_PATH]+"/"+this->sync->getUser(), "");
}

FMH::MODEL_LIST FM::getTagContent(const QString &tag)
{
    FMH::MODEL_LIST content;

    qDebug()<< "TAG CONTENT FOR TAG"<< tag;

    for(auto data : this->tag->getUrls(tag, false))
    {
        const auto url = data.toMap().value(TAG::KEYMAP[TAG::KEYS::URL]).toString();
        auto item = FMH::getFileInfoModel(url);
        content << item;
    }

    return content;
}

QVariantMap FM::getDirInfo(const QString &path, const QString &type)
{
    return FMH::getDirInfo(path, type);
}

QVariantMap FM::getFileInfo(const QString &path)
{
    return FMH::getFileInfo(path);
}

bool FM::isDefaultPath(const QString &path)
{
    return FMH::defaultPaths.contains(path);
}

QString FM::parentDir(const QString &path)
{
    auto dir = QDir(path);
    dir.cdUp();
    return dir.absolutePath();
}

bool FM::isDir(const QString &path)
{
    return QFileInfo(path).isDir();
}

bool FM::isApp(const QString& path)
{
    return /*QFileInfo(path).isExecutable() ||*/ path.endsWith(".desktop");
}

bool FM::isCloud(const QString &path)
{
	return path.startsWith(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::CLOUD_PATH]);
}

bool FM::bookmark(const QString &path)
{
    if(FMH::defaultPaths.contains(path))
        return false;

    if(!FMH::fileExists(path))
        return false;

    QFileInfo file (path);
    QVariantMap bookmark_map {
        {FMH::MODEL_NAME[FMH::MODEL_KEY::PATH], path},
        {FMH::MODEL_NAME[FMH::MODEL_KEY::LABEL], file.baseName()},
        {FMH::MODEL_NAME[FMH::MODEL_KEY::DATE], QDateTime::currentDateTime()}
    };

    if(this->insert(FMH::TABLEMAP[FMH::TABLE::BOOKMARKS], bookmark_map))
    {
        emit this->bookmarkInserted(path);
        return true;
    }

    return false;
}

bool FM::removeBookmark(const QString& path)
{
    FMH::DB data = {{FMH::MODEL_KEY::PATH, path}};
    if(this->remove(FMH::TABLEMAP[FMH::TABLE::BOOKMARKS], data))
    {
        emit this->bookmarkRemoved(path);
        return true;
    }

    return false;
}

bool FM::isBookmark(const QString& path)
{
    return this->checkExistance(QString("select * from bookmarks where path = '%1'").arg(path));
}

bool FM::fileExists(const QString &path)
{
    return FMH::fileExists(path);
}

void FM::saveSettings(const QString &key, const QVariant &value, const QString &group)
{
    UTIL::saveSettings(key, value, group);
}

QVariant FM::loadSettings(const QString &key, const QString &group, const QVariant &defaultValue)
{
    return UTIL::loadSettings(key, group, defaultValue);
}

QString FM::formatSize(const int &size)
{
	QLocale locale;
	return locale.formattedDataSize(size);
}

QString FM::formatDate(const QString &dateStr, const QString &format)
{
	const auto date = QDateTime::fromString(dateStr, Qt::TextDate);
	return date.toString(format);
}

QString FM::homePath()
{
    return FMH::HomePath;
}

bool FM::cut(const QVariantList &data, const QString &where)
{	
	FMH::MODEL_LIST items;
	
	for(auto k : data)
	{	
		auto map = k.toMap();
		FMH::MODEL model;
		
		for(auto key : map.keys())
			model.insert(FMH::MODEL_NAME_KEY[key], map[key].toString());				
		
		items << model;
	}
	
	for(auto item : items)
	{
		auto path = item[FMH::MODEL_KEY::PATH];
		
		if(this->isCloud(path))
		{
			this->sync->setCopyTo(where);			
			this->sync->resolveFile(item, Syncing::SIGNAL_TYPE::COPY);
			
		}else if(UTIL::fileExists(path))
		{
			QFile file(path);
			file.rename(where+"/"+QFileInfo(path).fileName());
		}
	}
	
	return true;
}

bool FM::copy(const QVariantList &data, const QString &where)
{
	FMH::MODEL_LIST items;
	
	for(auto k : data)
	{	
		auto map = k.toMap();
		FMH::MODEL model;
		
		for(auto key : map.keys())
			model.insert(FMH::MODEL_NAME_KEY[key], map[key].toString());				
		
		items << model;
	}	

	QStringList cloudPaths;
	
    for(auto item : items)
    {
		auto path = item[FMH::MODEL_KEY::PATH];
        if(this->isDir(path))
        {
            auto state = copyPath(path, where+"/"+QFileInfo(path).fileName(), false);
            if(!state) return false;

        }else if(this->isCloud(path))
		{
			this->sync->setCopyTo(where);			
			this->sync->resolveFile(item, Syncing::SIGNAL_TYPE::COPY);
			
		}else if(UTIL::fileExists(path))
        {
            QFile file(path);
            qDebug()<< path << "is a file" << where;

			if(this->isCloud(where))				
				cloudPaths << path;				
			else
				file.copy(where+"/"+FMH::getFileInfoModel(path)[FMH::MODEL_KEY::LABEL]);			
		}
    }
    
    if(!cloudPaths.isEmpty())
	{
		qDebug()<<"UPLOAD QUEUE" << cloudPaths;
		
		const auto firstPath = cloudPaths.takeLast();
		this->sync->setUploadQueue(cloudPaths);
		
		if(where.split("/").last().contains("."))		
		{
			QStringList whereList = where.split("/");
			whereList.removeLast();
			auto whereDir = whereList.join("/");			
			qDebug()<< "Trying ot copy to cloud" << where << whereDir;
			
			this->sync->upload(this->resolveLocalCloudPath(whereDir), firstPath);
		} else
			this->sync->upload(this->resolveLocalCloudPath(where), firstPath);		
	}

    return true;
}

bool FM::copyPath(QString sourceDir, QString destinationDir, bool overWriteDirectory)
{
    QDir originDirectory(sourceDir);

    if (!originDirectory.exists())    
        return false;
    
    QDir destinationDirectory(destinationDir);

    if(destinationDirectory.exists() && !overWriteDirectory)    
        return false;    
    else if(destinationDirectory.exists() && overWriteDirectory)    
        destinationDirectory.removeRecursively();    

    originDirectory.mkpath(destinationDir);

    foreach(QString directoryName, originDirectory.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString destinationPath = destinationDir + "/" + directoryName;
        originDirectory.mkpath(destinationPath);
        copyPath(sourceDir + "/" + directoryName, destinationPath, overWriteDirectory);
    }

    foreach (QString fileName, originDirectory.entryList(QDir::Files))
    {
        QFile::copy(sourceDir + "/" + fileName, destinationDir + "/" + fileName);
    }

    /*! Possible race-condition mitigation? */
    QDir finalDestination(destinationDir);
    finalDestination.refresh();

    if(finalDestination.exists())    
        return true;    

    return false;
}

bool FM::removeFile(const QString &path)
{
    if(QFileInfo(path).isDir())
        return removeDir(path);
    else return QFile(path).remove();
}

bool FM::removeDir(const QString &path)
{
    bool result = true;
    QDir dir(path);

    if (dir.exists(path))
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if (info.isDir())
            {
                result = removeDir(info.absoluteFilePath());
            }
            else
            {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result)
            {
                return result;
            }
        }
        result = dir.rmdir(path);
    }

    return result;
}

bool FM::rename(const QString &path, const QString &name)
{
    QFile file(path);
    auto url = QFileInfo(path).dir().absolutePath();
    qDebug()<< "RENAME FILE TO:" << path << name << url;

    return file.rename(url+"/"+name);
}

bool FM::createDir(const QString &path, const QString &name)
{
    return QDir(path).mkdir(name);
}

bool FM::createFile(const QString &path, const QString &name)
{
    QFile file(path + "/" + name);

    if(file.open(QIODevice::ReadWrite))
    {
        file.close();
        return true;
    }

    return false;
}

bool FM::openUrl(const QString &url)
{
    return QDesktopServices::openUrl(QUrl::fromUserInput(url));
}

void FM::openLocation(const QStringList &urls)
{
	for(auto url : urls)
		QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(url).dir().absolutePath()));
}

void FM::runApplication(const QString& exec)
{
#if (defined (Q_OS_LINUX) && !defined (Q_OS_ANDROID))
    return  MAUIKDE::launchApp(exec);
#endif
}

QVariantMap FM::dirConf(const QString &path)
{
    return FMH::dirConf(path);
}

void FM::setDirConf(const QString &path, const QString &group, const QString &key, const QVariant &value)
{
    FMH::setDirConf(path, group, key, value);
}

