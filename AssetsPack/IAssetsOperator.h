/*!
* @file IAssetsOperator.h
* @date 2017/08/18 14:56
*
* @author 阿力(qq:395942144)
*
* @brief 处理资源包文件的接口，跟系统文件打交道的类
*
*/
#ifndef IASSETS_OPERATOR_H
#define IASSETS_OPERATOR_H
#include "Macros.h"
#include <set>
#include "uthash.h"
#include "xxhash.h"
#pragma pack(1)
// 资源包头部信息
typedef struct tagPackHead
{
    uint nFileEntryOffset; // 文件入口偏移
    uint nFileAmount; // 总文件数量
} PackHead;

// 文件入口信息
typedef struct tagFileEntry
{
    uint nOffset;                   // 文件所在的偏移
    uint nNameHash;         // 文件名哈希值
    uint nFileSize;             // 文件大小
    uchar ucFlag;               // 文件标志
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
