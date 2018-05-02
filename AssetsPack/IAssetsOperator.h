/*!
* @file IAssetsOperator.h
* @date 2017/08/18 14:56
*
* @author ����(qq:395942144)
*
* @brief ������Դ���ļ��Ľӿڣ���ϵͳ�ļ��򽻵�����
*
*/
#ifndef IASSETS_OPERATOR_H
#define IASSETS_OPERATOR_H
#include "Macros.h"
#include <set>
#include "uthash.h"
#include "xxhash.h"
#pragma pack(1)
// ��Դ��ͷ����Ϣ
typedef struct tagPackHead
{
    uint nFileEntryOffset; // �ļ����ƫ��
    uint nFileAmount; // ���ļ�����
} PackHead;

// �ļ������Ϣ
typedef struct tagFileEntry
{
    uint nOffset;                   // �ļ����ڵ�ƫ��
    uint nNameHash;         // �ļ�����ϣֵ
    uint nFileSize;             // �ļ���С
    uchar ucFlag;               // �ļ���־
} FileEntry;


struct UT_FileEntry_MAP{
	uint id;
	FileEntry* data;
	UT_hash_handle hh;
};
typedef struct UT_FileEntry_MAP* FileEntryPtr;

#pragma pack()
class IAssetsOperator
{
public:
    virtual bool OpenAssetsPackFile(const char*) = 0;
    virtual void ReadPackHead(PackHead&) = 0;
	virtual void ReadFileEntry(FileEntryPtr&) = 0;
    virtual uint Read(uchar* pBuffer, uint nOffset, uint nLen) = 0;
    virtual uint Write(const uchar* pBuffer, uint nOffset, uint nLen) = 0;
    virtual void Close() = 0;
};
#endif // !IASSETS_OPERATOR_H
