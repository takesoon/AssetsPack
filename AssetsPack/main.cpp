// AssetsPack.cpp : 定义控制台应用程序的入口点。
//
#include "AssetsOperator.h"
#include "AssetsPack.h"
#include "IFile.h"
#include <io.h>
#include <vector>
#include <fstream>
#include "md5.h"
#include <map>
#ifdef WIN32
#include<Windows.h>   
#include <tchar.h>    
#define VS_LOG(fmt,var) {TCHAR sOut[256];_stprintf_s(sOut,(fmt),var);OutputDebugString(sOut);}    
#else 
#define VS_LOG(fmt,var) 
#endif
std::string  replace(std::string   str, const   std::string&   old_value, const   std::string&   new_value)
{
	for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())   {
		if ((pos = str.find(old_value, pos)) != std::string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

std::string md5sum(unsigned char *message, unsigned int message_size){
	MD5 md5;
	md5.update(message, message_size);
	md5.finalize();
	char* digest = md5.hex_digest();
	std::string ret(digest);
	delete[] digest;
	return ret;
}
void TestAddFile()
{
    CAssetsPack* pAssetsPack = new CAssetsPack(CAssetsOperator::GetInstance());

    if(pAssetsPack->LoadPackFile("F:\\TestPackFile.dat"))
    {
        pAssetsPack->AddFile("ui\\LoginView\\bg.png", (const uchar*)"1234\0", 5);
        pAssetsPack->SavePack();
    }

    delete pAssetsPack;
}

void TestReadFile()
{
    CAssetsPack* pAssetsPack = new CAssetsPack(CAssetsOperator::GetInstance());

    do
	{
		pAssetsPack->LoadPackFile("D:\\as\\AssetsPack\\Debug\\asset.pak");
        IFile* pFile = pAssetsPack->OpenFile("script/main.lua");

        BREAK_IF(!pFile)

        uchar* pBuffer = pFile->GetBuffer();
		printf("pBuffer: %s\n", pBuffer);
        BREAK_IF(!pBuffer)

        pFile->Read(pBuffer);
        printf((const char*)pBuffer);

        delete pBuffer;
    }
    while(0);

    pAssetsPack->Close();
    delete pAssetsPack;
}

using std::string;
using std::vector;
using std::ios;

enum
{
	kNone,
	kSrcDir,
	kSrcRoot,
	kPackedFile
};

static string src_dir = "";
static string src_root = "";
static string packed_file = "def.pak";
static vector<string> all_files;
static std::map<string, string> files_md5;
static int cur_cmd_kind = kNone;

void parse_value(char* arg)
{
	switch (cur_cmd_kind)
	{
	case kSrcDir:
		src_dir = arg;
		src_dir = replace(src_dir, "\\", "/");
		break;
	case kSrcRoot:
		src_root = arg;
		src_root = replace(src_root, "\\", "/");
		break;
	case kPackedFile:
		packed_file = arg;
		break;
	default:
		break;
	}
	cur_cmd_kind = kNone;
}

void parse_key(char* arg)
{
	if (cur_cmd_kind != kNone)
	{
		parse_value(arg);
		return;
	}

	if (strcmpi(arg, "--dir") == 0)
	{
		cur_cmd_kind = kSrcDir;
	}
	else if (strcmpi(arg, "--root") == 0)
	{
		cur_cmd_kind = kSrcRoot;
	}
	else if (strcmpi(arg, "--target") == 0)
	{
		cur_cmd_kind = kPackedFile;
	}
	else
	{
		parse_value(arg);
	}

}

void list_files(const string& root)
{
	struct _finddata_t files;
	int file_handle;
	file_handle = _findfirst((root+"\\*.*").c_str(), &files);
	if (file_handle == -1)
	{
		printf("error\n");
		return;
	}
	do
	{
		if (strcmpi(files.name,".") == 0 || 
			strcmpi(files.name, "..") == 0 )
		{
		}
		else
		{
			if (files.attrib == _A_SUBDIR)
			{
				list_files(root+"\\"+files.name);
			}
			else
			{
				std::string firstName = files.name;
				std::string firstPos = firstName.substr(0, 1);
				if (strcmp(firstPos.c_str(), ".") == 1)
				{
					auto filePath = root + "\\" + files.name;
					filePath = replace(filePath, "\\", "/");
					all_files.push_back(filePath);
					//printf("file name : %s\n", files.name);
				}

			}
		}

	} while (0 == _findnext(file_handle, &files));
	_findclose(file_handle);
}

int pack()
{
	all_files.clear();
	list_files(src_dir);
	printf("all_files.size  : %d\n", all_files.size());
	remove(packed_file.c_str());

	CAssetsPack* assets_pack = new CAssetsPack(CAssetsOperator::GetInstance());

	if (!assets_pack->LoadPackFile(packed_file.c_str()))
	{
		return -1;
	}

	for (size_t i = 0; i < all_files.size(); i++)
	{
		string filepath = all_files[i];
		std::ifstream fin(filepath.c_str(), ios::binary);
		if (fin.good())
		{
			fin.seekg(0, ios::end);
			int file_size = fin.tellg();
			fin.seekg(0, ios::beg);
			char* buffer = new char[file_size];
			fin.read(buffer, file_size);
			if (src_root.size()>0)
			{
				filepath = filepath.substr(src_root.size() + 1, 256);
				std::string firstPos = filepath.substr(0, 1);
				if (strcmp(firstPos.c_str(), "/") == 0)
				{
					filepath = filepath.substr(1, filepath.length());
				}
			}

			std::string md5Val = md5sum((unsigned char*)buffer, file_size);
			files_md5.insert(pair<string, string>(filepath, md5Val));
			assets_pack->AddFile(filepath.c_str(), (const uchar*)buffer, file_size);
			//printf("m_setFileEntry=  : %d\n", assets_pack->m_setFileEntry.size());
			//printf("filepath=  : %s\n", filepath.c_str());
			//VS_LOG("filepath=  : %s\n", filepath.c_str());
			//VS_LOG("m_setFileEntry=  : %d\n", assets_pack->m_setFileEntry.size());
			fin.close();
			delete buffer;
		}
	}

	assets_pack->SavePack();

	return 0;
}



bool checkFile()
{
	printf("checkFile!!!\n");
	CAssetsPack* pAssetsPack = new CAssetsPack(CAssetsOperator::GetInstance());
	if (!pAssetsPack->LoadPackFile(packed_file.c_str()))
	{
		return false;
	}

	for (size_t i = 0; i < all_files.size(); i++)
	{
			string filepath = all_files[i];
			if (src_root.size() > 0)
			{
				filepath = filepath.substr(src_root.size() + 1, 256);
				std::string firstPos = filepath.substr(0, 1);
				if (strcmp(firstPos.c_str(), "/") == 0)
				{
					filepath = filepath.substr(1, filepath.length());
				}
			}
			uint nNameHash = XXHASH32(filepath.c_str());
			VS_LOG("pszFileName == %s ", filepath.c_str());
			VS_LOG("hash == %d ", nNameHash);
			if (filepath == "script/main.lua")
			{
				VS_LOG("hash == %d ", nNameHash);
			}
			IFile* pFile = pAssetsPack->OpenFile(filepath.c_str());
			if (!pFile )
			{
				printf("checkFile failed:  %s\n", filepath.c_str());
				VS_LOG("checkFile failed:  %s\n", filepath.c_str());
				return false;
			}
			else
			{
				uchar* pBuffer = pFile->GetBuffer();
			    std::string md5Val =  md5sum(pBuffer, pFile->GetFileSize());
				auto it = files_md5.find(filepath);
				if (it != files_md5.end() )
				{
					printf("checkFile md5Val:  %s\n", it->second.c_str());
					if (strcmpi(md5Val.c_str(), it->second.c_str()) == 1)
					{
						printf("checkFile failed:  %s\n", filepath.c_str());
						VS_LOG("checkFile failed:  %s\n", filepath.c_str());
						return false;
					}
				}
				printf("checkFile ok:  %s\n", filepath.c_str());
				VS_LOG("checkFile ok:  %s\n", filepath.c_str());


				delete pBuffer;
			}

	} while (0);

	pAssetsPack->Close();
	return true;
}
int main(int argc, char* argv[])
{

	for (int i = 1; i < argc;i++)
    {
		char* arg = argv[i];
		parse_key(arg);
    }
	printf("source dir: %s\n", src_dir.c_str());
	printf("source root: %s\n", src_root.c_str());
	printf("target packed file: %s\n", packed_file.c_str());
	printf("start packing...\n");

	if (pack() == 0)
	{
		printf("success!!!\n");
	}
	else
	{
		printf("pack failure!!!\n");
	}
	checkFile();
#if _DEBUG
    getchar();
#endif
    return 0;
}

