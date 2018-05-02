
#include "AssetsOperator.h"
#include <stdio.h>
#include <assert.h>

CAssetsOperator::CAssetsOperator()
    : m_pFile(NULL)
{
}


CAssetsOperator::~CAssetsOperator()
{
    m_packHead.nFileAmount = 0;
    m_packHead.nFileEntryOffset = 0;
}

CAssetsOperator* CAssetsOperator::GetInstance()
{
    static CAssetsOperator inst;
    return &inst;
}

bool CAssetsOperator::OpenAssetsPackFile(const char* pszFileName)
{
    m_pFile = fopen(pszFileName, "rb+");

    if(m_pFile == NULL)
    {
		m_pFile = fopen(pszFileName, "wb+");

		if (m_pFile)
		{
			m_packHead.nFileEntryOffset = sizeof(PackHead);
			Write((const uchar*)&m_packHead, 0, m_packHead.nFileEntryOffset);
		}
		else
		{
			return false;
		}
    }

    return true;
}

void CAssetsOperator::ReadPackHead(PackHead& packHead)
{
    if(!m_pFile)
    {
        return;
    }

    int err = _fseeki64(m_pFile, 0, SEEK_SET);

    if(err == 0)
    {
        int nReaded = fread(&packHead, 1, sizeof(PackHead), m_pFile);

        if(nReaded == sizeof(PackHead))
        {
            m_packHead.nFileAmount = packHead.nFileAmount;
            m_packHead.nFileEntryOffset = packHead.nFileEntryOffset;
        }

        else
        {
            packHead.nFileAmount = 0;
            packHead.nFileEntryOffset = 0;
        }

    }
}

void CAssetsOperator::ReadFileEntry(FileEntryPtr& setFileEntry)
{
    if(!m_pFile)
    {
        return;
    }

    int nEntryOffset = m_packHead.nFileEntryOffset;
    int nEntrySize = sizeof(FileEntry);

    for(uint i = 0; i < m_packHead.nFileAmount; i++)
    {
        int err = _fseeki64(m_pFile, nEntryOffset, SEEK_SET);

        if(err == 0)
        {
            FileEntry *entry =  new FileEntry;
            int nReaded = fread(entry, 1, nEntrySize, m_pFile);

            if(nReaded == nEntrySize)
            {
				addToFileCache(setFileEntry, entry->nNameHash, entry);
            }

            nEntryOffset += nEntrySize;
        }
    }
	unsigned int file_size;
	file_size = HASH_COUNT(setFileEntry);
    // 读取的文件入口信息和文件头记录的文件数量不一致,错误了
	if (file_size != m_packHead.nFileAmount)
    {
		assert(0);
    }
}

uint CAssetsOperator::Read(uchar* pBuffer, uint nOffset, uint nLen)
{
    if(!m_pFile)
    {
        return 0;
    }

    int err = _fseeki64(m_pFile, nOffset, SEEK_SET);

    if(err == 0)
    {
        return fread(pBuffer, 1, nLen, m_pFile);
    }

    return 0;
}

uint CAssetsOperator::Write(const uchar* pBuffer, uint nOffset, uint nLen)
{
    if(!m_pFile)
    {
        return 0;
    }

    int err = _fseeki64(m_pFile, nOffset, SEEK_SET);

    if(err == 0)
    {
        return fwrite(pBuffer, 1, nLen, m_pFile);
    }

    return 0;
}

void CAssetsOperator::Close()
{
    if(m_pFile)
    {
        fclose(m_pFile);
        m_packHead.nFileAmount = 0;
        m_packHead.nFileEntryOffset = 0;
        m_pFile = NULL;
    }
}



void CAssetsOperator::addToFileCache(FileEntryPtr &_fileEntryCache, uint id, FileEntry* data)
{
	struct UT_FileEntry_MAP *s;

	s = (struct UT_FileEntry_MAP*)malloc(sizeof(struct UT_FileEntry_MAP));
	s->id = id;
	s->data = data;
	HASH_ADD_INT(_fileEntryCache, id, s);
}

FileEntry* CAssetsOperator::findInFileCache(FileEntryPtr& _fileEntryCache,uint id)
{
	struct UT_FileEntry_MAP* s;
	HASH_FIND_INT(_fileEntryCache, &id, s);
	return s->data;
}

void CAssetsOperator::deleteInFileCache(FileEntryPtr& _fileEntryCache,uint id)
{
	struct UT_FileEntry_MAP* s;
	HASH_FIND_INT(_fileEntryCache, &id, s);
	HASH_DEL(_fileEntryCache, s);
	free(s);
}
