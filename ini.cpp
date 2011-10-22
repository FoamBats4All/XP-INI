#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500

#include "ini.h"

/* ---------------------------------------------------------------------------------------
	NWNX & DLL Specific Functions
   --------------------------------------------------------------------------------------- */

INI* plugin;

DLLEXPORT Plugin* GetPluginPointerV2(){
	return plugin;
}


BOOL APIENTRY
DllMain(
	HMODULE hModule,
	DWORD dwReason,
	LPVOID lpReserved
	)
{
	switch( dwReason ) {
		case DLL_PROCESS_ATTACH: {
			plugin = new INI();
			char szPath[ MAX_PATH ];
			GetModuleFileName( hModule, szPath, MAX_PATH );
			plugin->SetPluginFullPath( szPath );
			break;
		}
		case DLL_PROCESS_DETACH: {
			delete plugin;
			break;
		}
		default:
			break;
	}

	return true;
}

/* ---------------------------------------------------------------------------------------
	Implementation of INI Plugin
   --------------------------------------------------------------------------------------- */

// -------------------------------------------------------------------- //
//	CONSTRUCTOR/DESTRUCTOR												//
// -------------------------------------------------------------------- //

INI::INI(
	)
{
	// Establish header.
	header		=	"NWNX INI Plugin v";
	header	   +=	NWNX_PLUGIN_INI_VERSION;
	header	   +=	"\n(c) 2011 by  FoamBats4All\nVisit us at http://www.nwnx.org\n";

	// Establish description.
	description	=	"This plugin modifies external initialization (INI) files.";

	// Set subclass.
	subClass	=	"INI";

	// Write version.
	version		=	NWNX_PLUGIN_INI_VERSION;
}

INI::~INI(
	)
{
	// Close maps.
	for ( IniMap::iterator i = m_IniFiles.begin(); i != m_IniFiles.end(); i++ ) {
		delete i->second;
	}

	wxLogMessage( wxT( "* Plugin unloaded." ) );
}

// -------------------------------------------------------------------- //
//	BASIC FUNCTIONALITY													//
// -------------------------------------------------------------------- //

bool
INI::Init(
	char* nwnxhome
	)
{
	// Be assertive. [?]
	assert( GetPluginFileName() );

	// Establish log file.
	wxString logfile( nwnxhome ); 
	logfile.append( wxT( "\\" ) );
	logfile.append( GetPluginFileName( ) );
	logfile.append( wxT( ".txt" ) );
	m_Logger = new wxLogNWNX( logfile, wxString( header.c_str( ) ) );

	// Establish config file.
	wxString inifile( nwnxhome ); 
	inifile.append( wxT( "\\" ) );
	inifile.append( GetPluginFileName() );
	inifile.append( wxT( ".ini" ) );
	wxLogTrace( TRACE_VERBOSE, wxT( "* reading inifile %s" ), inifile );
	m_Config = new wxFileConfig( wxEmptyString, wxEmptyString, inifile, wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS );

	// Conclude initialization.
	wxLogMessage( wxT( "* Plugin initialized." ) );
	return true;
}

void
INI::GetFunctionClass(
	char* fClass
	)
{
	_tcsncpy_s( fClass, 128, wxT( "INI" ), 3 );
}

// -------------------------------------------------------------------- //
//	NWNXGETVALUE / NWNXSETVALUE											//
// -------------------------------------------------------------------- //

int
INI::GetInt(
	char* psFileID,
	char* psKey,
	int nFlag
	)
{
	// Log the NWNX4 string for debugging purposes.
	wxLogMessage( wxT( "* Plugin GetInt( psFileID = \"%s\", psKey = \"%s\", nFlag = %d )" ), psFileID, psKey, nFlag );

	// Special cases.
	switch ( nFlag ) {
		case FLAG_OPEN_FILE:
			return OpenFile( psFileID, psKey );
		case FLAG_SAVE_FILE:
			return SaveFile( psFileID );
		case FLAG_CLOSE_FILE:
			return CloseFile( psFileID );
		case FLAG_CREATE_FILE:
			return CreateFile( psFileID );
		case FLAG_DELETE_FILE:
			return DeleteFile( psFileID );
		case FLAG_FILE_OPENED:
			return SaveFile( psFileID );
		case FLAG_GET_UNICODE:
			return GetIsUnicode( psFileID );
		case FLAG_GET_MULTIKEY:
			return GetIsMultikey( psFileID );
		case FLAG_FILE_EMPTY:
			return GetIsEmpty( psFileID );
		case FLAG_GET_MULTILINE:
			return GetIsMultiline( psFileID );
		case FLAG_GET_USESPACES:
			return GetUseSpaces( psFileID );
	};

	// Error Check: Data specified?
	if ( psFileID == wxT( "" ) ) {
		wxLogMessage( wxT( "! Error: File not specified." ) );
		return 0;
	} else if ( psKey == wxT( "" ) ) {
		wxLogMessage( wxT( "! Error: Setting not specified." ) );
		return 0;
	}

	// File opened?
	if ( !FileOpened( psFileID ) ) {
		wxLogMessage( wxT( "! Error: File not opened." ) );
		return false;
	}

	// Get value.
	try {
		// Get data.
		CSimpleIniA* iniFile = m_IniFiles[psFileID];
		std::string section = GetSection( psKey );
		std::string key = GetKey( psKey );

		// Check data.
		if ( section == "" || key == "" ) {
			wxLogMessage( wxT( "! Error: Section or key invalid." ) );
			return 0;
		}

		// Key exists?
		if ( !ValidKey( iniFile, section.c_str(), key.c_str() ) ) {
			return 0;
		}

		// Return value.
		std::string value = iniFile->GetValue( section.c_str(), key.c_str(), "" );
		return atol( value.c_str() );
	} catch ( std::exception& e ) {
		wxLogMessage( wxT( "! Error: %s" ), e.what() );
	} catch ( ... ) {
		wxLogMessage( wxT( "! Error: Unknown exception in INI::GetInt." ) );
	}
	return 0;
}

void
INI::SetInt(
	char* psFileID,
	char* psKey,
	int nFlag,
	int nValue
	)
{
	// Log the NWNX4 string for debugging purposes.
	wxLogMessage( wxT( "* Plugin SetInt( psFileID = \"%s\", psKey = \"%s\", nFlag = %d, psValue = %d )" ), psFileID, psKey, nFlag, nValue );
	
	// Special cases.
	switch ( nFlag ) {
		case FLAG_SET_UNICODE:
			SetUnicode( psFileID, nValue );
			return;
		case FLAG_SET_MULTIKEY:
			SetMultikey( psFileID, nValue );
			return;
		case FLAG_SET_MULTILINE:
			SetMultiline( psFileID, nValue );
			return;
		case FLAG_SET_USESPACES:
			SetUseSpaces( psFileID, nValue );
			return;
	};

	// Error Check: Data specified?
	if ( psFileID == wxT( "" ) ) {
		wxLogMessage( wxT( "* File not specified." ) );
		return;
	} else if ( psKey == wxT( "" ) ) {
		wxLogMessage( wxT( "* Setting not specified." ) );
		return;
	}

	// File opened?
	if ( !FileOpened( psFileID ) ) {
		wxLogMessage( wxT( "! Error: File not opened." ) );
		return;
	}
	
	// Write value.
	try {
		CSimpleIniA* iniFile = m_IniFiles[psFileID];
		std::string section = GetSection( psKey );
		std::string key = GetKey( psKey );
		iniFile->SetLongValue( section.c_str(), key.c_str(), nValue );
	} catch ( std::exception& e ) {
		wxLogMessage( wxT( "! Error: %s" ), e.what() );
		return;
	} catch ( ... ) {
		wxLogMessage( wxT( "! Error: Unknown exception in INI::GetInt." ) );
		return;
	}
}

float
INI::GetFloat(
	char* psFileID,
	char* psKey,
	int nFlag
	)
{
	// Log the NWNX4 string for debugging purposes.
	wxLogMessage( wxT( "* Plugin GetFloat( psFileID = \"%s\", psKey = \"%s\", nFlag = %d )" ), psFileID, psKey, nFlag );

	// Error Check: Data specified?
	if ( psFileID == wxT( "" ) ) {
		wxLogMessage( wxT( "* File not specified." ) );
		return 0.0f;
	} else if ( psKey == wxT( "" ) ) {
		wxLogMessage( wxT( "* Setting not specified." ) );
		return 0.0f;
	}

	// File opened?
	if ( !FileOpened( psFileID ) ) {
		wxLogMessage( wxT( "! Error: File not opened." ) );
		return 0.0f;
	}

	// Get value.
	try {
		// Get data.
		CSimpleIniA* iniFile = m_IniFiles[psFileID];
		std::string section = GetSection( psKey );
		std::string key = GetKey( psKey );

		// Key exists?
		if ( !ValidKey( iniFile, section.c_str(), key.c_str() ) ) {
			wxLogMessage( wxT( "! Error: Section or key invalid." ) );
			return 0.0f;
		}

		// Return value.
		std::string value = iniFile->GetValue( section.c_str(), key.c_str(), "" );
		return atof( value.c_str() );
	} catch ( std::exception& e ) {
		wxLogMessage( wxT( "! Error: %s" ), e.what() );
	} catch ( ... ) {
		wxLogMessage( wxT( "! Error: Unknown exception in INI::GetInt." ) );
	}
	return 0.0f;
}

void
INI::SetFloat(
	char* psFileID,
	char* psKey,
	int nFlag,
	float fValue
	)
{
	// Log the NWNX4 string for debugging purposes.
	wxLogMessage( wxT( "* Plugin SetFloat( psFileID = \"%s\", psKey = \"%s\", nFlag = %d, psValue = %f )" ), psFileID, psKey, nFlag, fValue );

	// Error Check: Data specified?
	if ( psFileID == wxT( "" ) ) {
		wxLogMessage( wxT( "! Error: psFileID not specified." ) );
		return;
	} else if ( psKey == wxT( "" ) ) {
		wxLogMessage( wxT( "! Error: psKey not specified." ) );
		return;
	}

	// File opened?
	if ( !FileOpened( psFileID ) ) {
		wxLogMessage( wxT( "! Error: File not opened." ) );
		return;
	}

	// Write value.
	CSimpleIniA* iniFile = m_IniFiles[psFileID];
	std::string section = GetSection( psKey );
	std::string key = GetKey( psKey );
	iniFile->SetDoubleValue( section.c_str(), key.c_str(), fValue );
}

char*
INI::GetString(
	char* psFileID,
	char* psKey,
	int nFlag
	)
{
	// Log the NWNX4 string for debugging purposes.
	wxLogMessage( wxT( "* Plugin GetString( psFileID = \"%s\", psKey = \"%s\", nFlag = %d )" ), psFileID, psKey, nFlag );

	// Special Case: Get File Path
	if ( nFlag == FLAG_GET_PATH ) {
		return GetFilePath( psFileID );
	}

	// File opened?
	if ( !FileOpened( psFileID ) ) {
		wxLogMessage( wxT( "! Error: File not opened." ) );
		return false;
	}

	// Get value.
	try {
		// Get data.
		CSimpleIniA* iniFile = m_IniFiles[psFileID];
		std::string section = GetSection( psKey );
		std::string key = GetKey( psKey );

		// Key exists?
		if ( !ValidKey( iniFile, section.c_str(), key.c_str() ) ) {
			wxLogMessage( wxT( "! Error: Section or key invalid." ) );
			return "";
		}

		// Return value.
		std::string value = iniFile->GetValue( section.c_str(), key.c_str(), "" );
		sprintf_s( returnBuffer, MAX_BUFFER, "%s", value.c_str() );
		return returnBuffer;
	} catch ( std::exception& e ) {
		wxLogMessage( wxT( "! Error: %s" ), e.what() );
	} catch ( ... ) {
		wxLogMessage( wxT( "! Error: Unknown exception in INI::GetInt." ) );
	}
	return "";
}

void
INI::SetString(
	char* psFileID,
	char* psKey,
	int nFlag,
	char* psValue
	)
{
	// Log the NWNX4 string for debugging purposes.
	wxLogMessage( wxT( "* Plugin SetString( psFileID = \"%s\", psKey = \"%s\", nFlag = %d, psValue = \"%s\" )" ), psFileID, psKey, nFlag, psValue );

	// Error Check: Data specified?
	if ( psFileID == wxT( "" ) ) {
		wxLogMessage( wxT( "! Error: File not specified." ) );
		return;
	} else if ( psKey == wxT( "" ) ) {
		wxLogMessage( wxT( "! Error: Setting not specified." ) );
		return;
	}
	
	// File opened?
	if ( !FileOpened( psFileID ) ) {
		wxLogMessage( wxT( "! Error: File not opened." ) );
		return;
	}

	// Write value.
	CSimpleIniA* iniFile = m_IniFiles[psFileID];
	std::string section = GetSection( psKey );
	std::string key = GetKey( psKey );
	iniFile->SetValue( section.c_str(), key.c_str(), psValue );
}

void
INI::WriteNWN2String(
	char* pszNWN2String
	)
{
	nwnxcpy( returnBuffer, pszNWN2String );
}

// -------------------------------------------------------------------- //
//	SECTION/KEY PARSING													//
// -------------------------------------------------------------------- //

std::string
INI::GetSection(
	char* psInput
	)
{
	// Format: "<section>|<key>".
	std::string base( psInput );
	return base.substr( 0, base.find( "|" ) );
}

std::string
INI::GetKey(
	char* psInput
	)
{
	// Format: "<section>|<key>".
	std::string base( psInput );
	unsigned int sectionLength = GetSection( psInput).length() + 1;
	return base.substr( sectionLength, base.length() - sectionLength );
}

bool
INI::ValidKey(
	CSimpleIniA* iniFile,
	const char* psSection,
	const char* psKey
	)
{
	// Check section.
	if ( iniFile->GetSectionSize( psSection ) == -1 ) {
		return false;
	}

	// Sections exists. Return true.
	return true;
}

// -------------------------------------------------------------------- //
//	FILE I/O															//
// -------------------------------------------------------------------- //

bool
INI::OpenFile(
	char* psFileID,
	char* psFile
	)
{
	wxLogMessage( wxT( "* OpenFile( psFileID = \"%s\", psFile = \"%s\" )" ), psFileID, psFile );

	// Check if file exists.
	if ( !wxFile::Exists( psFile ) ) {
		wxLogMessage( wxT( "* File does not exist." ) );
		return false;
	}

	// Load the ini file.
	std::string fileID = psFileID;
	CSimpleIniA* iniFile = new CSimpleIniA( true, false, true );
	iniFile->SetUnicode();
	if ( iniFile->LoadFile( psFile ) < SI_OK ) {
		wxLogMessage( wxT( "! Could not load ini file: %s" ), psFile );
		return false;
	}
	m_IniFiles[fileID] = iniFile;
	m_IniFileNames[fileID] = psFile;
	return true;
}

bool
INI::SaveFile(
	char* psFileID
	)
{
	wxLogMessage( wxT( "* SaveFile( psFileID = \"%s\" )" ), psFileID );

	// Create output.
	SI_Error state = m_IniFiles[psFileID]->SaveFile( m_IniFileNames[psFileID].c_str() );
	if ( state < SI_OK ) {
		wxLogMessage( wxT( "* Failed to save file." ) );
		return false;
	}
		
	return true;
}

bool
INI::CloseFile(
	char* psFileID
	)
{
	wxLogMessage( wxT( "* CloseFile( psFileID = \"%s\" )" ), psFileID );
	delete m_IniFiles[psFileID];
	m_IniFiles.erase( psFileID );
	return true;
}

bool
INI::CreateFile(
	char* psFile
	)
{
	wxLogMessage( wxT( "* CreateFile( psFile = \"%s\" )" ), psFile );
	
	// Check if file exists.
	if ( wxFile::Exists( psFile ) ) return false;

	// Create file.
	wxFile f( psFile, wxFile::read_write );
	if ( !f.Create( psFile ) ) {
		wxLogMessage( wxT( "! Could not create file: %s" ), psFile );
		return false;
	}
	f.Close();
	return true;
}

bool
INI::DeleteFile(
	char* psFile
	)
{
	wxLogMessage( wxT( "* DeleteFile( psFile = \"%s\" )" ), psFile );
	
	// Check if file exists.
	if ( !wxFile::Exists( psFile ) ) return false;

	// Delete file.
	return ( wxRemoveFile( psFile ) );
}

char*
INI::GetFilePath(
	char* psFileID
	)
{
	// Log the NWNX4 string for debugging purposes.
	wxLogMessage( wxT( "* Plugin GetFilePath( psFileID = \"%s\" )" ), psFileID );

	// File opened?
	if ( !FileOpened( psFileID ) ) {
		wxLogMessage( wxT( "! Error: File not opened." ) );
		return false;
	}

	// Get path.
	std::string path = m_IniFileNames[psFileID];
	sprintf_s( returnBuffer, MAX_BUFFER, "%s", path.c_str() );
	return returnBuffer;
}
	
// -------------------------------------------------------------------- //
//	SETTINGS															//
// -------------------------------------------------------------------- //

bool
INI::GetIsUnicode(
	char* psFileID
	)
{
	if ( !FileOpened( psFileID ) ) return false;
	return m_IniFiles[psFileID]->IsUnicode();
}

void
INI::SetUnicode(
	char* psFileID,
	bool bUnicode
	)
{
	if ( !FileOpened( psFileID ) ) return;
	m_IniFiles[psFileID]->SetUnicode( bUnicode );
}

bool
INI::GetIsMultikey(
	char* psFileID
	)
{
	if ( !FileOpened( psFileID ) ) return false;
	return m_IniFiles[psFileID]->IsMultiKey();
}

void
INI::SetMultikey(
	char* psFileID,
	bool bMultikey
	)
{
	if ( !FileOpened( psFileID ) ) return;
	m_IniFiles[psFileID]->SetMultiKey( bMultikey );
}
	
bool
INI::GetIsMultiline(
	char* psFileID
	)
{
	if ( !FileOpened( psFileID ) ) return false;
	return m_IniFiles[psFileID]->IsMultiLine();
}

void
INI::SetMultiline(
	char* psFileID,
	bool bMultiline
	)
{
	if ( !FileOpened( psFileID ) ) return;
	m_IniFiles[psFileID]->SetMultiLine( bMultiline );
}
	
bool
INI::GetUseSpaces(
	char* psFileID
	)
{
	if ( !FileOpened( psFileID ) ) return false;
	return m_IniFiles[psFileID]->UsingSpaces();
}

void
INI::SetUseSpaces(
	char* psFileID,
	bool bUseSpaces
	)
{
	if ( !FileOpened( psFileID ) ) return;
	m_IniFiles[psFileID]->SetSpaces( bUseSpaces );
}
	
// -------------------------------------------------------------------- //
//	STATES																//
// -------------------------------------------------------------------- //

bool
INI::FileOpened(
	char* psFileID
	)
{
	return ( m_IniFiles.count(psFileID) > 0 );
}

bool
INI::GetIsEmpty(
	char* psFileID
	)
{
	if ( !FileOpened( psFileID ) ) return false;
	return m_IniFiles[psFileID]->IsEmpty();
}