#include "AssetsPack.h"
#include "xxhash.h"
#include "IFile.h"
#include "IAssetsOperator.h"
#include "AssetsFile.h"
#include <string.h>


CAssetsPack::CAssetsPack(IAssetsOperator* pAssetsOperator)
    : m_pAssetsOperator(pAssetsOperator)
    , m_bLoaded(false)
{
    m_packHead.nFileAmount = 0;
    m_packHead.nFileEntryOffset = sizeof(PackHead);
	_fileEntryCache = nullptr;
}


CAssetsPack::~CAssetsPack()
{
	CleanCache();
}

bool CAssetsPack::LoadPackFile(const char* pszAssetsPackFile)
{
    if(!m_pAssetsOperator)
    {
        return false;
    }

    bool ret = m_pAssetsOperator->OpenAssetsPackFile(pszAssetsPackFile);

    if(!ret)
    {
        return false;
    }

	CleanCache();
    m_pAssetsOperator->ReadPackHead(m_packHead);
	m_pAssetsOperator->ReadFileEntry(_fileEntryCache);

    m_bLoaded = true;

    return true;
}

std::string  CAssetsPack::replace(std::string   str, const   std::string&   old_value, const   std::string&   new_value)
{
	for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())   {
		if ((pos = str.find(old_value, pos)) != std::string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

IFile* CAssetsPack::OpenFile(const char* pszFileName)
{
	uint nNameHash = XXHASH32(pszFileName);
	FileEntry * pEntry = FindInFileCache(nNameHash);
	if (pEntry)
	{
		IFile* file = new(std::nothrow) CAssetsFile(m_pAssetsOperator, pEntry->nOffset, pEntry->nFileSize);
		return file;
	}
	return nullptr;
}

bool CAssetsPack::IsFileExist(const char* pszFileName)
{
    uint nNameHash = XXHASH32(pszFileName);
	FileEntry * pEntry = FindInFileCache(nNameHash);
	if (pEntry&&pEntry->ucFlag != FF_DELETE )
	{
		return true;
	}
    return false;
}

bool CAssetsPack::DelFile(const char* pszFileName)
{
    uint nNameHash = XXHASH32(pszFileName);
	FileEntry * pEntry = FindInFileCache(nNameHash);

	if (pEntry&&pEntry->ucFlag != FF_DELETE)
	{
		pEntry->ucFlag = FF_DELETE;
		return true;
	}
    return false;
}

bool CAssetsPack::AddFile(const char* pszFileName, const uchar* pBuffer, uint nFileSize)
{
    // 添加文件前先把同名文件给删了
    DelFile(pszFileName);

    uint nNameHash = XXHASH32(pszFileName);

    FileEntry *newEntry = new FileEntry;
    newEntry->ucFlag = FF_NORMAL;
	newEntry->nFileSize = nFileSize;
	newEntry->nNameHash = nNameHash;

    FileEntry* pDirtyEntry = GetDirtyEntry(nFileSize);

    // 如果刚好要增加的文件小于其中某个已删除文件的大小，那么直接写入到那个位置
    if(pDirtyEntry)
    {
        pDirtyEntry->ucFlag = FF_NORMAL;
        pDirtyEntry->nFileSize = nFileSize;
		newEntry->nOffset = pDirtyEntry->nOffset;
    }

    else
    {
        // 文件偏移放到末尾
		newEntry->nOffset = m_packHead.nFileEntryOffset;
        m_packHead.nFileEntryOffset += nFileSize;
		AddToFileCache( newEntry);
        m_packHead.nFileAmount++;
    }

    return nFileSize == m_pAssetsOperator->Write(pBuffer, newEntry->nOffset, nFileSize);

}

// 获取适合大小的已删除的文件位置信息
FileEntry* CAssetsPack::GetDirtyEntry(uint nFileSize)
{
	FileEntryPtr s;
	for (s = _fileEntryCache; s != NULL; s = (FileEntryPtr)s->hh.next) {
		FileEntry  * pEntry = s->data;
		if (pEntry->ucFlag == FF_DELETE && pEntry->nFileSize >= nFileSize)
		{
			return pEntry;
		}
	}
	return nullptr;
}



void CAssetsPack::AddToFileCache(FileEntry* data)
{
	struct UT_FileEntry_MAP *s;

	s = (struct UT_FileEntry_MAP*)malloc(sizeof(struct UT_FileEntry_MAP));
	s->id = data->nNameHash;
	s->data = data;
	HASH_ADD_INT(_fileEntryCache, id, s);
}

FileEntry* CAssetsPack::FindInFileCache(uint id)
{
	struct UT_FileEntry_MAP* s;
	HASH_FIND_INT(_fileEntryCache, &id, s);
	if (s)
	{
		return s->data;
	}
	return nullptr;
}

void CAssetsPack::DeleteInFileCache(uint id)
{
	struct UT_FileEntry_MAP* s;
	HASH_FIND_INT(_fileEntryCache, &id, s);
	if (s)
	{
		HASH_DEL(_fileEntryCache, s);
		free(s);
	}
}

void CAssetsPack::CleanCache() {
	struct UT_FileEntry_MAP *current_user, *tmp;

	HASH_ITER(hh, _fileEntryCache, current_user, tmp) {
		HASH_DEL(_fileEntryCache, current_user);
		delete current_user->data;
		free(current_user);
	}
}

void CAssetsPack::SavePack()
{
    if(!m_pAssetsOperator)
    {
        return;
    }

    // 写入文件头信息
    m_pAssetsOperator->Write((const uchar*)&m_packHead, 0, sizeof(PackHead));

    // 写入文件列表信息
    uint nEntryOffset = m_packHead.nFileEntryOffset;
    uint nEntrySize = sizeof(FileEntry);
	FileEntryPtr s;
	for (s = _fileEntryCache; s != NULL; s = (FileEntryPtr)s->hh.next) {
		FileEntry  * pEntry = s->data;
		m_pAssetsOperator->Write((const uchar*)pEntry, nEntryOffset, nEntrySize);
		nEntryOffset += nEntrySize;
	}
    // 关闭文件
    m_pAssetsOperator->Close();
}

void CAssetsPack::Close()
{
    m_pAssetsOperator->Close();
}
