/*!
 * @file AssetsOperator.h
 * @date 2017/08/18 16:24
 *
 * @author 阿力(qq:395942144)
 *
 * @brief 处理资源包文件的接口的实现
 *
*/
#ifndef ASSETS_OPERATOR_H
#define ASSETS_OPERATOR_H
#include "IAssetsOperator.h"
#include "Macros.h"
#include <set>
class CAssetsOperator : public IAssetsOperator
{
public:
	CAssetsOperator();
	~CAssetsOperator();

	static CAssetsOperator* GetInstance();

	virtual bool OpenAssetsPackFile(const char* pszFileName) override;

	virtual void ReadPackHead(PackHead& packHead) override;

	virtual void ReadFileEntry(FileEntryPtr& setFileEntry) override;

	virtual uint Read(uchar* pBuffer, uint nOffset, uint nLen) override;

	virtual uint Write(const uchar* pBuffer, uint nOffset, uint nLen) override;

	virtual void Close() override;
	void addToFileCache(FileEntryPtr &_fileEntryCache,uint id, FileEntry* data);
	FileEntry* findInFileCache(FileEntryPtr& _fileEntryCache,uint id);
	void deleteInFileCache(FileEntryPtr& _fileEntryCache,uint id);
private:
	FILE* m_pFile;
	PackHead m_packHead;

};
#endif // !ASSETS_OPERATOR_H



