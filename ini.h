/* ---------------------------------------------------------------------------------------
	NWNX INI Include - INI functions plugin
	Copyright (C) 2011 Michael Wallar (michael@wallar.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   --------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------
	Date		Name			Reason
   ---------------------------------------------------------------------------------------
	2011-10-	FoamBats4All	Initial release.
   --------------------------------------------------------------------------------------- */

#ifndef NWNX_PLUGIN_INI
#define NWNX_PLUGIN_INI

#define DLLEXPORT extern "C" __declspec(dllexport)
#define NWNX_PLUGIN_INI_VERSION "1.0.0"

#include "windows.h"
#include "stdio.h"
#include "azroles.h"
#include "ObjBase.h"
#include "shlobj.h"
#include "../plugin.h"
#include "../../misc/log.h"
#include "wx/tokenzr.h"
#include "wx/hashset.h"
#include "wx/filefn.h"
#include "wx/fileconf.h"
#include "strsafe.h"
#include "SimpleIni.h"
#include <vector>
#include <fstream>
#include <string>

typedef std::map<std::string, CSimpleIniA*> IniMap;
typedef std::map<std::string, std::string> IniFilenameMap;

class INI : public Plugin {

public:

	enum Flags {
		FLAG_NONE,
		FLAG_OPEN_FILE,
		FLAG_SAVE_FILE,
		FLAG_CLOSE_FILE,
		FLAG_CREATE_FILE,
		FLAG_DELETE_FILE,
		FLAG_FILE_OPENED,
		FLAG_GET_UNICODE,
		FLAG_SET_UNICODE,
		FLAG_GET_MULTIKEY,
		FLAG_SET_MULTIKEY,
		FLAG_GET_MULTILINE,
		FLAG_SET_MULTILINE,
		FLAG_GET_USESPACES,
		FLAG_SET_USESPACES,
		FLAG_GET_PATH,
		FLAG_FILE_EMPTY,
		FLAG_INVALID
	};
	
	// -------------------------------------------------------------------- //
	//	CONSTRUCTOR/DESTRUCTOR												//
	// -------------------------------------------------------------------- //
	
	INI(
		);

	~INI(
		);
	
	// -------------------------------------------------------------------- //
	//	BASIC FUNCTIONALITY													//
	// -------------------------------------------------------------------- //

	bool
	Init(
		char* nwnxhome
		);

	void
	GetFunctionClass(
		char* fClass
		);
	
	// -------------------------------------------------------------------- //
	//	NWNXGETVALUE / NWNXSETVALUE											//
	// -------------------------------------------------------------------- //

	int
	GetInt(
		char* psFunction,
		char* psParam1,
		int nParam2
		);

	void
	SetInt(
		char* psFile,
		char* psSetting,
		int nNull,
		int nValue
		);

	float
	GetFloat(
		char* psFunction,
		char* psParam1,
		int nParam2
		);

	void
	SetFloat(
		char* psFunction,
		char* psParam1,
		int nParam2,
		float fValue
		);

	char*
	GetString(
		char* psFunction,
		char* psParam1,
		int nParam2
		);
	
	void
	SetString(
		char* psFileID,
		char* psKey,
		int nParam2,
		char* psValue
		);

	void
	WriteNWN2String(
		char* pszNWN2String
		);

private:
	
	// -------------------------------------------------------------------- //
	//	SECTION/KEY PARSING													//
	// -------------------------------------------------------------------- //

	std::string
	GetSection(
		char* psInput
		);

	std::string
	GetKey(
		char* psInput
		);

	bool
	ValidKey(
		CSimpleIniA* iniFile,
		const char* psSection,
		const char* psKey
		);
	
	// -------------------------------------------------------------------- //
	//	FILE I/O															//
	// -------------------------------------------------------------------- //

	bool
	OpenFile(
		char* psFileID,
		char* psFile
		);

	bool
	SaveFile(
		char* psFileID
		);

	bool
	CloseFile(
		char* psFileID
		);

	bool
	CreateFile(
		char* psFile
		);

	bool
	DeleteFile(
		char* psFile
		);

	char*
	GetFilePath(
		char* psFileID
		);
	
	// -------------------------------------------------------------------- //
	//	SETTINGS															//
	// -------------------------------------------------------------------- //

	bool
	GetIsUnicode(
		char* psFileID
		);

	void
	SetUnicode(
		char* psFileID,
		bool bUnicode
		);
	
	bool
	GetIsMultikey(
		char* psFileID
		);

	void
	SetMultikey(
		char* psFileID,
		bool bMultikey
		);
	
	bool
	GetIsMultiline(
		char* psFileID
		);

	void
	SetMultiline(
		char* psFileID,
		bool bMultiline
		);
	
	bool
	GetUseSpaces(
		char* psFileID
		);

	void
	SetUseSpaces(
		char* psFileID,
		bool bUseSpaces
		);
	
	// -------------------------------------------------------------------- //
	//	STATES																//
	// -------------------------------------------------------------------- //

	bool
	FileOpened(
		char* psFileID
		);

	bool
	GetIsEmpty(
		char* psFileID
		);
	
	// -------------------------------------------------------------------- //

	// Basic functionality.
	wxFileConfig						  * m_Config;				// Configuration (ini file).
	wxLogNWNX							  * m_Logger;				// Logging.

	// Execution data.
	STARTUPINFO								m_StartupInfo;			// Process StartUp Information structure.
	PROCESS_INFORMATION						m_ProcessInfo;			// Process Information structure.
	char									m_CommandLine[ 2000 ];	// BIC Function's command-line (2000 bytes).

	// Stored ini files.
	IniFilenameMap							m_IniFileNames;			// Map: FileKey->FilePath.
	IniMap									m_IniFiles;				// Map: FileKey->IniData.

};

#endif