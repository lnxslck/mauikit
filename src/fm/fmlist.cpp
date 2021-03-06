/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  camilo <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fmlist.h"
#include <QObject>
#include "fm.h"
#include "utils.h"

#include <QFileSystemWatcher>
#include <syncing.h>

FMList::FMList(QObject *parent) : QObject(parent)
{
	this->fm = new FM(this);
	connect(this->fm, &FM::cloudServerContentReady, [this](const FMH::MODEL_LIST &list, const QString &url)
	{
		if(this->path == url)
		{			
			this->pre();		
			this->list = list;
			this->pathEmpty = this->list.isEmpty();
			emit this->pathEmptyChanged();
			this->pos();
			this->setContentReady(true);	
			
		}	
	});
	
	connect(this->fm, &FM::warningMessage, [this](const QString &message)
	{
		emit this->warning(message);
	});
	
	connect(this->fm, &FM::loadProgress, [this](const int &percent)
	{
		emit this->progress(percent);
	});
	
	this->watcher = new QFileSystemWatcher(this);
	connect(this->watcher, &QFileSystemWatcher::directoryChanged, [this](const QString &path)
	{
		Q_UNUSED(path);
		this->reset();
	});
	
	connect(this->fm, &FM::newItem, [this] (const FMH::MODEL &item, const QString &url)
	{
		if(this->path == url)
		{
			emit this->preItemAppended();
			this->list << item;		
			this->pathEmpty = this->list.isEmpty();
			emit this->pathEmptyChanged();
			emit this->postListChanged();
		}
	});	
	
	connect(this, &FMList::pathChanged, this, &FMList::reset);
	// 	connect(this, &FMList::hiddenChanged, this, &FMList::setList);
	// 	connect(this, &FMList::onlyDirsChanged, this, &FMList::setList);
	// 	connect(this, &FMList::filtersChanged, this, &FMList::setList);
	auto value = UTIL::loadSettings("SaveDirProps", "SETTINGS", this->saveDirProps).toBool();
	this->setSaveDirProps(value);	
}

FMList::~FMList()
{}

void FMList::pre()
{
	emit this->preListChanged();
	// 	this->setContentReady(false);
}

void FMList::pos()
{
	// 	this->setContentReady(true);
	emit this->postListChanged();
}

void FMList::watchPath(const QString& path, const bool& clear)
{	
	if(!this->watcher->directories().isEmpty() && clear)
		this->watcher->removePaths(this->watcher->directories());
	
	if(path.isEmpty() || !FMH::fileExists(path))
		return;
	
	this->watcher->addPath(path);
}

void FMList::setList()
{	
	this->setContentReady(true);
	switch(this->pathType)
	{
		case FMList::PATHTYPE::SEARCH_PATH:
			this->list.clear();
			this->search(QString(this->path).right(this->path.length()- 1 - this->path.lastIndexOf("/")), this->prevHistory.length() > 1 ? this->prevHistory[this->prevHistory.length()-2] : this->path);
			return;
			
		case FMList::PATHTYPE::APPS_PATH:
			this->list = FM::getAppsContent(this->path);
			break;
			
		case FMList::PATHTYPE::TAGS_PATH:
			this->list = this->fm->getTagContent(QString(this->path).right(this->path.length()- 1 - this->path.lastIndexOf("/")));
			break;
			
		case FMList::PATHTYPE::PLACES_PATH:
			this->list = FM::getPathContent(this->path, this->hidden, this->onlyDirs, this->filters);
			break;
			
		case FMList::PATHTYPE::CLOUD_PATH:
			this->list.clear();
			if(this->fm->getCloudServerContent(this->path, this->filters, this->cloudDepth))
			{	
				this->setContentReady(false);				
				return;			
			}else break;
			
		case FMList::PATHTYPE::TRASH_PATH:
		case FMList::PATHTYPE::DRIVES_PATH:
		case FMList::PATHTYPE::BOOKMARKS_PATH:
			this->list = FMH::MODEL_LIST();
			break;
	}
	
	this->pathEmpty = this->list.isEmpty() && FM::fileExists(this->path);
	emit this->pathEmptyChanged();
	
	this->sortList();
}

void FMList::reset()
{
	this->pre();
	
	switch(this->pathType)
	{
		case FMList::PATHTYPE::APPS_PATH:
			
			this->hidden = false;			
			this->preview = false;
			break;
			
		case FMList::PATHTYPE::CLOUD_PATH:			
		case FMList::PATHTYPE::SEARCH_PATH:
		case FMList::PATHTYPE::TAGS_PATH:
			
			this->hidden = false;			
			this->preview = true;
			break;
			
		case FMList::PATHTYPE::PLACES_PATH:
		{
			if(this->saveDirProps)
			{
				auto conf = FMH::dirConf(this->path+"/.directory");				
				this->hidden = conf[FMH::MODEL_NAME[FMH::MODEL_KEY::HIDDEN]].toBool();				
				this->preview = conf[FMH::MODEL_NAME[FMH::MODEL_KEY::SHOWTHUMBNAIL]].toBool();
				this->foldersFirst = conf[FMH::MODEL_NAME[FMH::MODEL_KEY::FOLDERSFIRST]].toBool();
			}else
			{
				this->hidden = UTIL::loadSettings("HiddenFilesShown", "SETTINGS", this->hidden).toBool();
				this->preview = UTIL::loadSettings("ShowThumbnail", "SETTINGS", this->preview).toBool();
				this->foldersFirst = UTIL::loadSettings("FoldersFirst", "SETTINGS", this->foldersFirst).toBool();
			}
		
			break;
		}
		
		case FMList::PATHTYPE::TRASH_PATH:
		case FMList::PATHTYPE::DRIVES_PATH:
		case FMList::PATHTYPE::BOOKMARKS_PATH:
			break;
	}
	
	if(this->saveDirProps)
	{
		auto conf = FMH::dirConf(this->path+"/.directory");	
		this->sort = static_cast<FMList::SORTBY>(conf[FMH::MODEL_NAME[FMH::MODEL_KEY::SORTBY]].toInt());		
		this->viewType = static_cast<FMList::VIEW_TYPE>(conf[FMH::MODEL_NAME[FMH::MODEL_KEY::VIEWTYPE]].toInt());		
	}else
	{	
		this->sort = static_cast<FMList::SORTBY>(UTIL::loadSettings("SortBy", "SETTINGS", this->sort).toInt());
		this->viewType = static_cast<FMList::VIEW_TYPE>(UTIL::loadSettings("ViewType", "SETTINGS", this->viewType).toInt());
	}
	
	emit this->sortByChanged();
	emit this->viewTypeChanged();
	emit this->hiddenChanged();
	emit this->previewChanged();			
	
	
	this->setList();	
	this->pos();
}

FMH::MODEL_LIST FMList::items() const
{
	return this->list;
}


FMList::SORTBY FMList::getSortBy() const
{
	return this->sort;
}

void FMList::setSortBy(const FMList::SORTBY &key)
{	
	if(this->sort == key)
		return;
	
	this->pre();
	
	this->sort = key;
	this->sortList();
	
	if(this->pathType == FMList::PATHTYPE::PLACES_PATH && this->trackChanges && this->saveDirProps)
		FMH::setDirConf(this->path+"/.directory", "MAUIFM", "SortBy", this->sort);
	else
		UTIL::saveSettings("SortBy", this->sort, "SETTINGS");
	
	emit this->sortByChanged();
	
	this->pos();
}

void FMList::sortList()
{
	FMH::MODEL_KEY key = static_cast<FMH::MODEL_KEY>(this->sort);
	auto index = 0;
	
	if(this->foldersFirst)
	{
		qSort(this->list.begin(), this->list.end(), [](const FMH::MODEL& e1, const FMH::MODEL& e2) -> bool
		{
			const auto key = FMH::MODEL_KEY::MIME;
			if(e1[key] == "inode/directory")
				return true;
			
			return false;
		});
		
		for(auto item : this->list)
			if(item[FMH::MODEL_KEY::MIME] == "inode/directory")
				index++;
			else break;
			
		qSort(this->list.begin(),this->list.begin() + index, [key](const FMH::MODEL& e1, const FMH::MODEL& e2) -> bool
		{
			auto role = key;
			
			switch(role)
			{				
				case FMH::MODEL_KEY::SIZE:
				{				
					if(e1[role].toDouble() > e2[role].toDouble())
						return true;
					break;
				}
				
				case FMH::MODEL_KEY::MODIFIED:
				case FMH::MODEL_KEY::DATE:
				{
					auto currentTime = QDateTime::currentDateTime();
					
					auto date1 = QDateTime::fromString(e1[role], Qt::TextDate);
					auto date2 = QDateTime::fromString(e2[role], Qt::TextDate);
					
					if(date1.secsTo(currentTime) <  date2.secsTo(currentTime))
						return true;
					
					break;
				}
				
				case FMH::MODEL_KEY::LABEL:
				{
					const auto str1 = QString(e1[role]).toLower();
					const auto str2 = QString(e2[role]).toLower();
					
					if(str1 < str2)
						return true;				
					break;
				}
				
				default:
					if(e1[role] < e2[role])
						return true;
			}
			
			return false;
		});
	}
	
	qSort(this->list.begin() + index, this->list.end(), [key](const FMH::MODEL& e1, const FMH::MODEL& e2) -> bool
	{
		auto role = key;
		
		switch(role)
		{
			case FMH::MODEL_KEY::MIME:
				if(e1[role] == "inode/directory")
					return true;
				break;
				
			case FMH::MODEL_KEY::SIZE:
			{				
				if(e1[role].toDouble() > e2[role].toDouble())
					return true;
				break;
			}
			
			case FMH::MODEL_KEY::MODIFIED:
			case FMH::MODEL_KEY::DATE:
			{
				auto currentTime = QDateTime::currentDateTime();
				
				auto date1 = QDateTime::fromString(e1[role], Qt::TextDate);
				auto date2 = QDateTime::fromString(e2[role], Qt::TextDate);
				
				if(date1.secsTo(currentTime) <  date2.secsTo(currentTime))
					return true;
				
				break;
			}
			
			case FMH::MODEL_KEY::LABEL:
			{
				const auto str1 = QString(e1[role]).toLower();
				const auto str2 = QString(e2[role]).toLower();
				
				if(str1 < str2)
					return true;				
				break;
			}
			
			default:
				if(e1[role] < e2[role])
					return true;
		}
		
		return false;
	});
}

QString FMList::getPath() const
{
	return this->path;
}

void FMList::setPath(const QString &path)
{
	if(this->path == path)
		return;
	
	this->path = path;
	this->setPreviousPath(this->path);
	
	qDebug()<< "Prev History" << this->prevHistory;
	
	if(path.startsWith(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::SEARCH_PATH]+"/"))
	{
		this->pathExists = true;
		this->pathType = FMList::PATHTYPE::SEARCH_PATH;
		this->isBookmark = false;
		emit this->pathExistsChanged();
		emit this->pathTypeChanged();
		emit this->isBookmarkChanged();
		this->watchPath(QString());
		
	}else if(path.startsWith(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::CLOUD_PATH]+"/"))
	{
		this->pathExists = true;
		this->pathType = FMList::PATHTYPE::CLOUD_PATH;
		this->isBookmark = false;
		emit this->pathExistsChanged();
		emit this->pathTypeChanged();
		emit this->isBookmarkChanged();
		this->watchPath(QString());
		
	}else if(path.startsWith(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::APPS_PATH]+"/"))
	{
		this->pathExists = true;
		this->pathType = FMList::PATHTYPE::APPS_PATH;
		this->isBookmark = false;
		emit this->pathExistsChanged();
		emit this->pathTypeChanged();
		emit this->isBookmarkChanged();
		this->watchPath(QString());
		
	}else if(path.startsWith(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::TAGS_PATH]+"/"))
	{
		this->pathExists = true;
		this->isBookmark = false;
		this->pathType = FMList::PATHTYPE::TAGS_PATH;
		emit this->pathExistsChanged();
		emit this->pathTypeChanged();
		emit this->isBookmarkChanged();
		this->watchPath(QString());
		
	}else
	{
		this->watchPath(this->path);
		this->isBookmark = this->fm->isBookmark(this->path);
		this->pathExists = FMH::fileExists(this->path);
		this->pathType = FMList::PATHTYPE::PLACES_PATH;
		emit this->pathExistsChanged();
		emit this->pathTypeChanged();
		emit this->isBookmarkChanged();
	}
	
	emit this->pathChanged();
}

FMList::PATHTYPE FMList::getPathType() const
{
	return this->pathType;
}

QStringList FMList::getFilters() const
{
	return this->filters;
}

void FMList::setFilters(const QStringList &filters)
{
	if(this->filters == filters)
		return;
	
	this->filters = filters;
	
	emit this->filtersChanged();
	this->reset();
}

FMList::FILTER FMList::getFilterType() const
{
	return this->filterType;
}

void FMList::setFilterType(const FMList::FILTER &type)
{
	this->filterType = type;
	this->filters = FMH::FILTER_LIST[static_cast<FMH::FILTER_TYPE>(this->filterType)];
	
	emit this->filtersChanged();
	emit this->filterTypeChanged();
	
	this->reset();
}

bool FMList::getHidden() const
{
	return this->hidden;
}

void FMList::setHidden(const bool &state)
{
	if(this->hidden == state)
		return;
	
	this->hidden = state;
	
	if(this->pathType == FMList::PATHTYPE::PLACES_PATH && this->trackChanges && this->saveDirProps)
		FMH::setDirConf(this->path+"/.directory", "Settings", "HiddenFilesShown", this->hidden);
	else
		UTIL::saveSettings("HiddenFilesShown", this->hidden, "SETTINGS");
	
	emit this->hiddenChanged();
	this->reset();
}

bool FMList::getPreview() const
{
	return this->preview;
}

void FMList::setPreview(const bool &state)
{
	if(this->preview == state)
		return;
	
	this->preview = state;
	
	if(this->pathType == FMList::PATHTYPE::PLACES_PATH && this->trackChanges && this->saveDirProps)
		FMH::setDirConf(this->path+"/.directory", "MAUIFM", "ShowThumbnail", this->preview);
	else
		UTIL::saveSettings("ShowThumbnail", this->preview, "SETTINGS");
	
	emit this->previewChanged();
}

bool FMList::getOnlyDirs() const
{
	return this->onlyDirs;
}

void FMList::setOnlyDirs(const bool &state)
{
	if(this->onlyDirs == state)
		return;
	
	this->onlyDirs = state;
	
	emit this->onlyDirsChanged();
	this->reset();
}

QVariantMap FMList::get(const int &index) const
{
	if(index >= this->list.size() || index < 0)
		return QVariantMap();
	
	QVariantMap res;
	const auto model = this->list.at(index);
	
	for(auto key : model.keys())
		res.insert(FMH::MODEL_NAME[key], model[key]);
	
	return res;
}

void FMList::refresh()
{
	emit this->pathChanged();
}

void FMList::createDir(const QString& name)
{
	if(this->pathType == FMList::PATHTYPE::PLACES_PATH)
		this->fm->createDir(this->path, name);	
	else if(this->pathType == FMList::PATHTYPE::CLOUD_PATH)		
	{
		this->fm->createCloudDir(QString(this->path).replace(FMH::PATHTYPE_NAME[FMH::PATHTYPE_KEY::CLOUD_PATH]+"/"+this->fm->sync->getUser(), ""), name);
	}
}

void FMList::copyInto(const QVariantList& files)
{
	if(this->pathType == FMList::PATHTYPE::PLACES_PATH ||  this->pathType == FMList::PATHTYPE::CLOUD_PATH)
		this->fm->copy(files, this->path);		
}

void FMList::test()
{
	this->fm->sync->upload("", "");
}

void FMList::cutInto(const QVariantList& files)
{
	if(this->pathType == FMList::PATHTYPE::PLACES_PATH)
		this->fm->cut(files, this->path);	
// 	else if(this->pathType == FMList::PATHTYPE::CLOUD_PATH)		
// 	{
// 		this->fm->createCloudDir(QString(this->path).replace(FMH::PATHTYPE_NAME[FMList::PATHTYPE::CLOUD_PATH]+"/"+this->fm->sync->getUser(), ""), name);
// 	}
}

QString FMList::getParentPath()
{
	switch(this->pathType)
	{		
		case FMList::PATHTYPE::PLACES_PATH:
			return FM::parentDir(this->path);
		default:
			return this->getPreviousPath();
	}
	
}

QString FMList::getPosteriorPath()
{
	if(this->postHistory.isEmpty())
		return this->path;
	
	return this->postHistory.takeAt(this->postHistory.length()-1);
}

void FMList::setPosteriorPath(const QString& path)
{
	this->postHistory.append(path);
}

QString FMList::getPreviousPath() 
{	
	if(this->prevHistory.isEmpty())
		return this->path;
	
	if(this->prevHistory.length() < 2)
		return this->prevHistory.at(0);
	
	auto post = this->prevHistory.takeAt(this->prevHistory.length()-1);
	this->setPosteriorPath(post);
	
	return this->prevHistory.takeAt(this->prevHistory.length()-1);
}

void FMList::setPreviousPath(const QString& path)
{
	this->prevHistory.append(path);
}

bool FMList::getPathEmpty() const
{
	return this->pathEmpty;
}

bool FMList::getPathExists() const
{
	return this->pathExists;
}

bool FMList::getTrackChanges() const
{
	return this->trackChanges;
}

void FMList::setTrackChanges(const bool& value)
{
	if(this->trackChanges == value)
		return;
	
	this->trackChanges = value;
	emit this->trackChangesChanged();
}

bool FMList::getIsBookmark() const
{
	return this->isBookmark;
}

void FMList::setIsBookmark(const bool& value)
{
	if(this->isBookmark == value)
		return;
	
	if(this->pathType != FMList::PATHTYPE::PLACES_PATH)
		return;
	
	this->isBookmark = value;
	
	if(value)
		this->fm->bookmark(this->path);
	else
		this->fm->removeBookmark(this->path);
	
	emit this->isBookmarkChanged();
}

bool FMList::getFoldersFirst() const
{
	return this->foldersFirst;
}

void FMList::setFoldersFirst(const bool &value)
{
	if(this->foldersFirst == value)
		return;
	
	this->pre();
	
	this->foldersFirst = value;
	
	if(this->pathType == FMList::PATHTYPE::PLACES_PATH && this->trackChanges && this->saveDirProps)
		FMH::setDirConf(this->path+"/.directory", "MAUIFM", "FoldersFirst", this->foldersFirst);
	else
		UTIL::saveSettings("FoldersFirst", this->foldersFirst, "SETTINGS");
	
	emit this->foldersFirstChanged();
	
	this->sortList();
	
	this->pos();
}

void FMList::setSaveDirProps(const bool& value)
{
	if(this->saveDirProps == value)
		return;
	
	this->saveDirProps = value;	
	UTIL::saveSettings("SaveDirProps", this->saveDirProps, "SETTINGS");
	
	emit this->saveDirPropsChanged();
}

bool FMList::getSaveDirProps() const
{
	return this->saveDirProps;
}

void FMList::setContentReady(const bool& value)
{
	this->contentReady = value;
	emit this->contentReadyChanged();
}

bool FMList::getContentReady() const
{
	return this->contentReady;
}

FMList::VIEW_TYPE FMList::getViewType() const
{
	return this->viewType;
}

void FMList::setViewType(const FMList::VIEW_TYPE& value)
{
	if(this->viewType == value)
		return;
	
	this->viewType = value;
	
	if(this->trackChanges && this->saveDirProps)
		FMH::setDirConf(this->path+"/.directory", "MAUIFM", "ViewType", this->viewType);
	else
		UTIL::saveSettings("ViewType", this->viewType, "SETTINGS");
	
	emit this->viewTypeChanged();
}

void FMList::search(const QString& query, const QString &path, const bool &hidden, const bool &onlyDirs, const QStringList &filters)
{
	FMH::MODEL_LIST content;
	
	if (FM::isDir(path))
	{
		QDir::Filters dirFilter;
		
		dirFilter = (onlyDirs ? QDir::AllDirs | QDir::NoDotDot | QDir::NoDot :
		QDir::Files | QDir::AllDirs | QDir::NoDotDot | QDir::NoDot);
		
		if(hidden)
			dirFilter = dirFilter | QDir::Hidden | QDir::System;
		
		QDirIterator it (path, filters, dirFilter, QDirIterator::NoIteratorFlags);
		while (it.hasNext())
		{
			auto url = it.next();
			auto info = it.fileInfo();
			if(info.completeBaseName().contains(query, Qt::CaseInsensitive))
			{
				emit preItemAppended();
				this->list << FMH::getFileInfoModel(url);
				emit postItemAppended();
			}
		}
	}
}

int FMList::getCloudDepth() const
{
	return this->cloudDepth;
}

void FMList::setCloudDepth(const int& value)
{
	if(this->cloudDepth == value)
		return;
	
	this->cloudDepth = value;
	
	emit this->cloudDepthChanged();
	this->reset();
}

