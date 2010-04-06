
//#define BCIEXTIF_API_DEF(type) type WINAPI
#define BCIEXTIF_API_DEF(type) __declspec(dllexport) type

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////
// System initialization
///////////////////////////
BCIEXTIF_API_DEF(void) BciextifDefSetStr( char* sVariable, char* sValue );
BCIEXTIF_API_DEF(bool) BciextifDefInit( char* sProgName );

BCIEXTIF_API_DEF(void) BciextifSetTrace( int traceLevel, char* filename );

BCIEXTIF_API_DEF(bool) BciextifGetError( int errorCode,
                                         char** errorMessage );

///////////////////////////
// BCI configuration
///////////////////////////

// if bci file is located in C:/Folder
// if bci file contains <bcibase_model folder="C:/Base/Foo" basefolder="C:/Base" subject="jean">
/////////////
// bUseAsRootFolder == true and bSmartOutputFolder == true
// -> Project folder will be C:/Folder/jean/<datetime>
// bUseAsRootFolder == true and bSmartOutputFolder == false
// -> Project folder will be C:/Folder
// bUseAsRootFolder == false and bSmartOutputFolder == true
// -> Project folder will be C:/Base/jean/<datetime>
// bUseAsRootFolder == false and bSmartOutputFolder == false
// -> Project folder will be C:/Base/Foo
BCIEXTIF_API_DEF(int) BciextifLoadConfigFromFile( char* filename,
                                                  bool bUseAsRootFolder,
                                                  bool bSmartOutputFolder );
BCIEXTIF_API_DEF(int) BciextifGetProjectFolder( char** folder );
BCIEXTIF_API_DEF(int) BciextifSaveConfigToFile( char* filename );

BCIEXTIF_API_DEF(int) BciextifGetChannelsCount( int* iCount );

BCIEXTIF_API_DEF(int) BciextifGetMaxChannelNameSize();
BCIEXTIF_API_DEF(int) BciextifGetChannelsNames( int iCount, char*** names );

///////////////////////////
// BCI loop initialization
///////////////////////////

BCIEXTIF_API_DEF(int) BciextifEnableEffector( char* name, 
                                              bool bEnable );

BCIEXTIF_API_DEF(int) BciextifInit( bool bInitEffectors,
                                    bool bInitSignals );

BCIEXTIF_API_DEF(int) BciextifStart();

BCIEXTIF_API_DEF(int) BciextifStop();

BCIEXTIF_API_DEF(int) BciextifExit( bool bRemoveChannels );

///////////////////////////
// BCI data access
///////////////////////////

BCIEXTIF_API_DEF(int) BciextifGetFrequency( 
                        char* channelID,
                        double* dFreq );

BCIEXTIF_API_DEF(int) BciextifGetTickCount( 
                        char* channelID,
                        int* iCount );

// Some caracteristic errors:
// 150 : datas are not ready yet, wait a bit
// 151 : acquisition device stopped:
//          - online mode: consider extending the experiment duration if any
//          - offline mode: you reached the end of the file
BCIEXTIF_API_DEF(int) BciextifReadData( 
                        char* channelID,
                        int iStart,
                        int* DataSize,
                        double Data[] );

BCIEXTIF_API_DEF(int) BciextifReadDataEx( 
                        char* channelID,
                        int iStart,
                        int* DataSize,
                        double Data[],
                        int* LostSize,
                        int Lost[] );

///////////////////////////
// Commands execution
///////////////////////////

BCIEXTIF_API_DEF(int) BciextifExecute( char* sCommand );

#ifdef __cplusplus
}
#endif

