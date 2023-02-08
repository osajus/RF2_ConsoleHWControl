/////
// 
//	RFactor 2 Console Hardware Control
// 
//	Allows external applications to toggle hardware controls within RFactor2.
//	This DLL will not do anything without a client application sending it commands
//  These commands are documented by Studio397.  Some work, some don't.
//	 https://www.studio-397.com/wp-content/uploads/2016/12/rFactorInternalsPlugin.pdf
// 
//	Code derived from:
//		Studio397 Example #7
//			https://www.studio-397.com/modding-resources/
//		rf2_Chat_transceiver by tappi287
//			https://github.com/tappi287/rf2_chat_transceiver
//		rFactor2-RaceStandings by cosimo
//			https://github.com/cosimo/rFactor2-RaceStandings
//		
//	02/07/2023
//
/////


#include "ConsoleHWControl.hpp"    // corresponding header file
#include <assert.h>
#include <math.h>               // for atan2, sqrt
#include <stdio.h>              // for sample output
#include <time.h>
#include <string>

#define BUF_SIZE 256

char datapath[MAX_FILENAME_LEN] = "";
bool debug = true;
bool apply = false;
TCHAR shared_memory_name[] = "RF2_ConsoleHWControl";
char msg[1024];


extern "C" __declspec( dllexport )
const char * __cdecl GetPluginName()                   { return( "RF2_ConsoleHWControl" ); }

extern "C" __declspec( dllexport )
PluginObjectType __cdecl GetPluginType()               { return( PO_INTERNALS ); }

extern "C" __declspec( dllexport )
int __cdecl GetPluginVersion()                         { return( 7 ); } // InternalsPluginV01 functionality (if you change this return value, you must derive from the appropriate class!)

extern "C" __declspec( dllexport )
PluginObject * __cdecl CreatePluginObject()            { return( (PluginObject *) new ConsoleHWControl ); }

extern "C" __declspec( dllexport )
void __cdecl DestroyPluginObject( PluginObject *obj )  { delete( (ConsoleHWControl *) obj ); }


bool create_shared_memory(HANDLE &h_map_file)
{
	h_map_file = ::CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		shared_memory_name);     // name of mapping object
	if (h_map_file == nullptr)
	{
		sprintf_s(msg, "Cannot create mapping object");
		::ConsoleHWControl::Log(msg);
		return false;
	}

	sprintf_s(msg, "Created shared memory object: %Ts", shared_memory_name);
	::ConsoleHWControl::Log(msg);
	return true;
}

bool open_shared_memory_handle(HANDLE &h_map_file)
{
	h_map_file = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, shared_memory_name);
	if (h_map_file == nullptr) { return false; }
	return true;
}

bool write_shared_memory(const HANDLE &h_map_file, const char *message_to_write)
{
	const std::string message = message_to_write;
	if (message.size() > BUF_SIZE)
	{
		sprintf_s(msg, "Message length too large: %zu", message.size());
		::ConsoleHWControl::Log(msg);
		return false;
	}

	const auto p_buf = MapViewOfFile(h_map_file,
		FILE_MAP_WRITE,      // read/write permission
		0,
		0,
		BUF_SIZE);

	if (p_buf == nullptr) {
		sprintf_s(msg, "Could not map view of file (%lu)", GetLastError());
		::ConsoleHWControl::Log(msg);
		return false;
	}

	sprintf_s(msg, "Writing to memory");
	::ConsoleHWControl::Log(msg);

	CopyMemory(p_buf, message.c_str(), message.size());
	UnmapViewOfFile(p_buf);
	return true;
}

void read_shared_memory_a(const HANDLE &h_map_file, std::string &result)
{
	if (h_map_file == nullptr) { return; }
	const char *buf = static_cast<char *>(
		::MapViewOfFile(h_map_file,
			FILE_MAP_READ, 0, 0, BUF_SIZE)
		);
	if (buf == nullptr) { return; }

	result = buf;
	UnmapViewOfFile(buf);
}

void ConsoleHWControl::Log(const char *msg)
{
  FILE *fo;
  time_t curtime;
  struct tm localtime;
  char timestamp[26];

  fo = fopen( LOG_FILENAME, "a" );
  if( fo != NULL )
  {
	curtime = time(NULL);
	localtime_s(&localtime, &curtime);
	asctime_s(timestamp, 26, &localtime);
	timestamp[24] = 0;
    fprintf( fo, "[%s] %s\n", timestamp, msg );
    fclose( fo );
  }
}

void ConsoleHWControl::Startup( long version )
{
	char temp[80];
	#ifdef DEBUGLOGGING
	sprintf_s( temp, "-STARTUP- (RF2 version %.3f)", (float) version / 1000.0f );
	Log( temp );
	sprintf_s( temp, "-PLUGIN CONSOLE HW Control (version %.0f)-", (float) PLUGIN_VERSION );
	Log( temp );
	#endif

	// default HW control enabled to true
	mEnabled = true;
	// Open shared memory
	open_shared_memory();
}

void ConsoleHWControl::Shutdown()
{
	#ifdef DEBUGLOGGING
	Log( "-SHUTDOWN-" );
	#endif

	close_shared_memory();
	mEnabled = false;
}

void ConsoleHWControl::StartSession()
{
	#ifdef DEBUGLOGGING
	Log( "--STARTSESSION--" );
	#endif
}

void ConsoleHWControl::EndSession()
{
	#ifdef DEBUGLOGGING
	Log( "--ENDSESSION--" );
	#endif
	mLastLeaderLaps = 0;
}

void ConsoleHWControl::EnterRealtime()
{
	// start up timer every time we enter realtime
	mET = 0.0;
	#ifdef DEBUGLOGGING
	Log( "---ENTERREALTIME---" );
	#endif
}

void ConsoleHWControl::ExitRealtime()
{
	#ifdef DEBUGLOGGING
	Log( "---EXITREALTIME---" );
	#endif
	clear_shared_memory();
}

void ConsoleHWControl::UpdateTelemetry( const TelemInfoV01 &info )
{
  return;
}

void ConsoleHWControl::UpdateGraphics( const GraphicsInfoV01 &info )
{
  return;
}

bool ConsoleHWControl::CheckHWControl( const char * const controlName, double &fRetVal ) {
	std::string rmsg = read_shared_memory();

	if (_stricmp(controlName, rmsg.c_str()) == 0) {
		#ifdef DEBUGLOGGING
		sprintf_s(msg, "currControl: %s, currVal: %f, changeControl: %s", controlName, fRetVal, rmsg.c_str());
		Log(msg);
		#endif
		fRetVal = 1.0;
		clear_shared_memory();
		rmsg.clear();
		return true;  
	}
	return false;
}

bool ConsoleHWControl::ForceFeedback( double &forceValue )
{
  return( false );
}

void ConsoleHWControl::UpdateScoring( const ScoringInfoV01 &info )
{
	return;
}

bool ConsoleHWControl::RequestCommentary( CommentaryRequestInfoV01 &info )
{
  return( false );
}

const char * ConsoleHWControl::GetRF2DataPath()
{
	FILE* datapath_file = fopen("Core\\data.path", "r");
	if (datapath_file != NULL) {
		fscanf(datapath_file, "%s", &datapath);
		fclose(datapath_file);
	}
	return ( datapath );
}

void ConsoleHWControl::open_shared_memory() {
	const bool created = create_shared_memory(h_map_file_);
	if (!created) {
		mEnabled = false;
		#ifdef DEBUGLOGGING
		sprintf_s(msg, "Could not create shared memory");
		::ConsoleHWControl::Log(msg);
		#endif
	}
}

void ConsoleHWControl::close_shared_memory() const {
	if (h_map_file_ == nullptr) { return; }
	CloseHandle(h_map_file_);
}

void ConsoleHWControl::clear_shared_memory() const {
	if (h_map_file_ == nullptr) { return; }
	const auto p_buf = MapViewOfFile(h_map_file_, FILE_MAP_ALL_ACCESS,
		0, 0, BUF_SIZE);
	if (p_buf == nullptr) {
		#ifdef DEBUGLOGGING
		sprintf_s(msg, "clear_shared_memory: Could not map view of file (%lu)", GetLastError());
		::ConsoleHWControl::Log(msg);
		#endif
		return;
	}
	ZeroMemory(p_buf, BUF_SIZE);
	UnmapViewOfFile(p_buf);
}

std::string ConsoleHWControl::read_shared_memory() const
{
	std::string result;
	if (!mEnabled) { return result; }

	::read_shared_memory_a(h_map_file_, result);
	return result;
}

bool ConsoleHWControl::update_from_shared_memory(std::string &message, unsigned char &destination) const
{
	// Read shared memory content
	const std::string mem_content = read_shared_memory();
	if (mem_content.empty()) { return false; }

	// Get the first chr as destination
	const std::string mem_destination = mem_content.substr(0, 1);
	if (mem_destination != "0" && mem_destination != "1") { return false; }

	// Get message content
	const std::string mem_message = mem_content.substr(1, 129);
	if (mem_message.empty()) { return false; }

	// Update message and destination by reference
	message = mem_message;
	destination = static_cast<unsigned char>(*mem_destination.c_str());

	// Clear shared memory
	clear_shared_memory();
	return true;
}

