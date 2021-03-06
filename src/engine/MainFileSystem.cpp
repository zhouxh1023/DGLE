/**
\author		Korotkov Andrey aka DRON
\date		26.09.2014 (c)Korotkov Andrey

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

#include "MainFileSystem.h"

using namespace std;

//CDummyFile//

class CDummyFile: public IFile
{
public:

	DGLE_RESULT DGLE_API Read(void *pBuffer, uint uiCount, uint &uiRead)
	{
		uiRead = 0;
		return E_NOTIMPL;
	}

	DGLE_RESULT DGLE_API Write(const void *pBuffer, uint uiCount, uint &uiWritten)
	{
		uiWritten = 0;
		return E_NOTIMPL;
	}

	DGLE_RESULT DGLE_API Seek(uint32 ui32Offset, E_FILE_SYSTEM_SEEK_FLAG eWay, uint32 &ui32Position)
	{
		ui32Position = 0;
		return E_NOTIMPL;
	}
	
	DGLE_RESULT DGLE_API GetSize(uint32 &ui32Size)
	{
		ui32Size = 0;
		return E_NOTIMPL;
	}

	DGLE_RESULT DGLE_API IsOpen(bool &bOpened)
	{
		bOpened = false;
		return S_OK;
	}

	DGLE_RESULT DGLE_API GetName(char *pcName, uint &uiCharsCount)
	{
		const char name[] = "dummy.nil";

		if (!pcName)
		{
			uiCharsCount = strlen(name) + 1;
			return S_OK;
		}

		if (uiCharsCount <= strlen(name))
		{
			if (uiCharsCount > 0)
				strcpy(pcName, "");

			uiCharsCount = strlen(name) + 1;
			
			return E_INVALIDARG;
		}

		strcpy(pcName, name);

		return S_OK;
	}

	DGLE_RESULT DGLE_API GetPath(char *pcPath, uint &uiCharsCount)
	{
		if (!pcPath)
		{
			uiCharsCount = 1;
			return S_OK;
		}

		if (uiCharsCount < 1)
		{
			uiCharsCount = 1;
			return E_INVALIDARG;
		}

		strcpy(pcPath, "");

		return S_OK;
	}
	
	DGLE_RESULT DGLE_API Free()
	{
		return S_OK;
	}

	IDGLE_BASE_IMPLEMENTATION(IFile, INTERFACE_IMPL_END)
};

//CMainFileSystem//

CMainFileSystem::CMainFileSystem(uint uiInstIdx):
CInstancedObj(uiInstIdx)
{
	_pDummyFile = new CDummyFile();

	Console()->RegComProc("mfs_list_virtual_fs", "Lists all registered virtual file systems.", &_s_ConListVFS, this);

	_pHDDFS = new CHDDFileSystem(InstIdx());
	_pDCPFS = new CDCPFileSystem(InstIdx());

	RegisterVirtualFileSystem("", "base Hard Disc Drive(HDD) file system.", (IFileSystem*)_pHDDFS, &_s_FSDeleteCallback, this);
	RegisterVirtualFileSystem("dcp", "Dgle Compiled Package(DCP) file archives.", (IFileSystem*)_pDCPFS, &_s_FSDeleteCallback, this);
	
	LOG("Filesystem Subsystem initialized.", LT_INFO);
}

CMainFileSystem::~CMainFileSystem()
{
	Console()->UnRegCom("mfs_list_virtual_fs");

	delete _pDummyFile;

	LOG("Filesystem Subsystem finalized.", LT_INFO);
}

void CMainFileSystem::UnregisterAndFreeAll()
{
	for (size_t i = 0; i < _clVFileSystems.size(); ++i)
		_clVFileSystems[i].pdc(_clVFileSystems[i].param, _clVFileSystems[i].fs);

	_clVFileSystems.clear();
	_strVFSsDescs.clear();
}

DGLE_RESULT DGLE_API CMainFileSystem::UnregisterVirtualFileSystem(const char *pcVFSExtension)
{
	for (size_t i = 0; i < _clVFileSystems.size(); ++i)
		if (_clVFileSystems[i].ext == ToUpperCase(string(pcVFSExtension)))
		{
			_clVFileSystems[i].pdc(_clVFileSystems[i].param, _clVFileSystems[i].fs);
			_clVFileSystems.erase(_clVFileSystems.begin()+i);
			return S_OK;
		}

	return E_INVALIDARG;
}

DGLE_RESULT DGLE_API CMainFileSystem::GetRegisteredVirtualFileSystems(char *pcTxt, uint &uiCharsCount)
{
	string exts;

	if (_clVFileSystems.size() > 1)
		for (size_t i = 1; i<_clVFileSystems.size(); ++i)
			exts += _clVFileSystems[i].ext + ";";

	if (!pcTxt)
	{
		uiCharsCount = exts.size() + 1;
		return S_OK;
	}

	if (exts.size() >= uiCharsCount)
	{
		uiCharsCount = exts.size() + 1;
		strcpy(pcTxt, "");
		return E_INVALIDARG;
	}

	strcpy(pcTxt, exts.c_str());

	return S_OK;
}

bool DGLE_API CMainFileSystem::_s_ConListVFS(void *pParameter, const char *pcParam)
{
	if (strlen(pcParam) != 0)
	{
		CON(CMainFileSystem, "No parameters expected.");
		return false;
	}
	else
	{
		CON(CMainFileSystem, string("---Supported VFile Systems---\n" + PTHIS(CMainFileSystem)->_strVFSsDescs + "-----------------------------").c_str());
		return true;
	}
}

void DGLE_API CMainFileSystem::_s_FSDeleteCallback(void *pParameter, IFileSystem *pVFS)
{
	if (PTHIS(CMainFileSystem)->_pHDDFS == pVFS)
		delete PTHIS(CMainFileSystem)->_pHDDFS;

	if (PTHIS(CMainFileSystem)->_pDCPFS == pVFS)
		delete PTHIS(CMainFileSystem)->_pDCPFS;
}

DGLE_RESULT DGLE_API CMainFileSystem::LoadFile(const char *pcFileName, IFile *&prFile)
{
	prFile = NULL;

	IFileSystem *p_fs = NULL;

	if (string(pcFileName).find("|") == string::npos)
		p_fs = (IFileSystem*)_pHDDFS;
	else
		for (size_t i = 0; i < _clVFileSystems.size(); ++i)
			if ((ToUpperCase(string(pcFileName)).find("." + ToUpperCase(_clVFileSystems[i].ext) + "|") != string::npos))
			{
				p_fs = _clVFileSystems[i].fs;
				break;
			}

	if (!p_fs)
	{
		LOG("Failed to find virtual file system to open file \"" + string(pcFileName) + "\".", LT_ERROR);
		return E_INVALIDARG;
	}

	bool exists = false;	
	p_fs->FileExists(pcFileName, exists);

	if (!exists)
	{
		LOG("File \"" + string(pcFileName) + "\" doesn't exists.", LT_WARNING);
		return S_FALSE;
	}

	return p_fs->OpenFile(pcFileName, (E_FILE_SYSTEM_OPEN_FLAGS)(FSOF_READ | FSOF_BINARY), prFile);
}

DGLE_RESULT DGLE_API CMainFileSystem::FreeFile(IFile *&prFile)
{
	const DGLE_RESULT res = prFile->Free();
	prFile = _pDummyFile;
	return res;
}

DGLE_RESULT DGLE_API CMainFileSystem::GetVirtualFileSystem(const char *pcVFSExtension, IFileSystem *&prVFS)
{
	if (pcVFSExtension == NULL)
	{
		prVFS = _clVFileSystems[0].fs;
		return S_OK;
	}

	for (size_t i = 0; i < _clVFileSystems.size(); ++i)
		if (_clVFileSystems[i].ext == ToUpperCase(string(pcVFSExtension)))
		{
			prVFS = _clVFileSystems[i].fs;
			return S_OK;
		}

	LOG("Virtual file system with extension \"" + string(pcVFSExtension) + "\" hasn't been registered.", LT_ERROR);

	return E_INVALIDARG;
}

DGLE_RESULT DGLE_API CMainFileSystem::GetVirtualFileSystemDescription(const char *pcVFSExtension, char *pcTxt, uint &uiCharsCount)
{
	for (size_t i = 0; i < _clVFileSystems.size(); ++i)
		if (_clVFileSystems[i].ext == ToUpperCase(string(pcVFSExtension)))
		{
			if (!pcTxt)
			{
				uiCharsCount = _clVFileSystems[i].discr.size() + 1;
				return S_OK;
			}

			if (_clVFileSystems[i].discr.size() >= uiCharsCount)
			{
				uiCharsCount = _clVFileSystems[i].discr.size() + 1;
				strcpy(pcTxt, "");
				return E_INVALIDARG;
			}

			strcpy(pcTxt, _clVFileSystems[i].discr.c_str());

			return S_OK;
		}

	strcpy(pcTxt, "");

	return S_FALSE;
}

DGLE_RESULT DGLE_API CMainFileSystem::RegisterVirtualFileSystem(const char *pcVFSExtension, const char *pcDiscription, IFileSystem *pVFS, void (DGLE_API *pDeleteCallback)(void *pParameter, IFileSystem *pVFS), void *pParameter)
{
	for (size_t i = 0; i < _clVFileSystems.size(); ++i)
		if (_clVFileSystems[i].ext == string(pcVFSExtension))
		{
			LOG("Virtual file system with extension \"" + string(pcVFSExtension) + "\" already registered.", LT_WARNING);
			return E_INVALIDARG;
		}
	
	_clVFileSystems.push_back(TVFileSystem(pcVFSExtension, pcDiscription, pVFS, pDeleteCallback, pParameter));

	_strVFSsDescs += string("- " + ToUpperCase(string(pcVFSExtension)) + " " + string(pcDiscription) + "\n");

	return S_OK;
}

DGLE_RESULT DGLE_API CMainFileSystem::GetType(E_ENGINE_SUB_SYSTEM &eSubSystemType)
{
	eSubSystemType = ESS_FILE_SYSTEM;
	return S_OK;
}