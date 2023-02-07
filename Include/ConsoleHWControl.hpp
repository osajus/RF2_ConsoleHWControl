
#ifndef _INTERNALS_EXAMPLE_H
#define _INTERNALS_EXAMPLE_H

#include "InternalsPlugin.hpp"
#include <string>

#define PLUGIN_VERSION						02072023
#define MAX_FILENAME_LEN					255
#define DEBUGLOGGING
//#define LOG_FILENAME			"E:\\Steam\\steamapps\\common\\rFactor 2\\Bin64\\Plugins\\RF2_ConsoleHWControl.log"
#define LOG_FILENAME			"RF2_ConsoleHWControl.log"

// This is used for the app to use the plugin for its intended purpose
// REMINDER: exported function GetPluginVersion() should return 1
// if you are deriving from this InternalsPluginV01, 2 for InternalsPluginV02, etc.
class ConsoleHWControl : public InternalsPluginV07
{

 public:

  // Constructor/destructor
  ConsoleHWControl() {}
  ~ConsoleHWControl() {}

  // These are the functions derived from base class InternalsPlugin
  // that can be implemented.
  void Startup( long version );  // game startup
  void Shutdown();               // game shutdown

  void EnterRealtime();          // entering realtime
  void ExitRealtime();           // exiting realtime

  void StartSession();           // session has started
  void EndSession();             // session has ended

  // GAME OUTPUT
  long WantsTelemetryUpdates() { return( 0 ); }
  void UpdateTelemetry( const TelemInfoV01 &info );

  bool WantsGraphicsUpdates() { return( false ); }
  void UpdateGraphics( const GraphicsInfoV01 &info );

  // GAME INPUT
  bool HasHardwareInputs() { return( true ); } // CHANGE TO TRUE TO ENABLE HARDWARE EXAMPLE!
  void UpdateHardware( const double fDT ) { mET += fDT; } // update the hardware with the time between frames
  void EnableHardware() { mEnabled = true; }             // message from game to enable hardware
  void DisableHardware() { mEnabled = false; }           // message from game to disable hardware

  // See if the plugin wants to take over a hardware control.  If the plugin takes over the
  // control, this method returns true and sets the value of the double pointed to by the
  // second arg.  Otherwise, it returns false and leaves the double unmodified.
  bool CheckHWControl( const char * const controlName, double &fRetVal );
  bool ForceFeedback( double &forceValue );

  const char * GetRF2DataPath();

  bool WantsScoringUpdates() { return( true ); }
  void UpdateScoring( const ScoringInfoV01 &info );

  bool RequestCommentary( CommentaryRequestInfoV01 &info );  // SEE FUNCTION BODY TO ENABLE COMMENTARY EXAMPLE
  static void Log( const char *msg );

  static std::string changeControl;


 private:

  void open_shared_memory();
  void close_shared_memory() const;
  void clear_shared_memory() const;
  std::string read_shared_memory() const;
  bool update_from_shared_memory(std::string &message, unsigned char &destination) const;

  //bool Log_Extern(const char *msg);
  double mET;  // needed for the hardware example
  bool mEnabled; // needed for the hardware example
  short mLastLeaderLaps;
  HANDLE h_map_file_{};

};


#endif // _INTERNALS_EXAMPLE_H
