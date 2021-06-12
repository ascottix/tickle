/*
    Tickle 0.95
    Main for SDL version
 
    Copyright (c) 2014-2021 Alessandro Scotti
*/
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "SDL2/SDL.h"

#include "emu/emu.h"
#include "sdl_main.h"

char * basePath;

#ifdef WIN32
const wchar_t PATH_SEPARATOR = '\\';
#else
const wchar_t PATH_SEPARATOR = '/';
#endif

// For some reasons g++ may get confused by the indirect reference and will not link the machines
// (this happens e.g. on the RPI) so we have to include them explicitly.
#include <machine/1942.h>
#include <machine/fantasy.h>
#include <machine/frogger.h>
#include <machine/galaga.h>
#include <machine/galaxian.h>
#include <machine/invaders.h>
#include <machine/nibbler.h>
#include <machine/pacman.h>
#include <machine/pengo.h>
#include <machine/pinball_action.h>
#include <machine/pooyan.h>
#include <machine/rallyx.h>
#include <machine/scramble.h>
#include <machine/vanguard.h>

void dummy()
{
    delete M1942::createInstance();
    delete Fantasy::createInstance();
    delete Frogger::createInstance();
    delete Galaga::createInstance();
    delete Galaxian::createInstance();
    delete Nibbler::createInstance();
    delete Pacman::createInstance();
    delete Pengo::createInstance();
    delete PinballAction::createInstance();
    delete Pooyan::createInstance();
    delete RallyX::createInstance();
    delete Scramble::createInstance();
    delete SpaceInvaders::createInstance();
    delete Vanguard::createInstance();
}

// Load a file from an input stream into a newly allocated buffer
// (which must be deleted by the caller)
unsigned char * loadFileFromStream( TInputStream * is, unsigned size, TCRC32 * crc )
{
    unsigned char * result = new unsigned char [size];
    
    if( is->read( result, size ) == size ) {
        crc->reset();
        crc->update( result, size );
    }
    else {
        delete [] result;
        result = 0;
    }
    
    return result;
}

unsigned getFileSize( const char * name )
{
    unsigned result = 0;
    
    FILE * f = fopen( name, "rb" );
    
    if( f != 0 ) {
        fseek( f, 0L, SEEK_END );
        result = (unsigned) ftell( f );
        fclose( f );
    }
    
    return result;
}

// Load a single file required by the driver
unsigned char * loadFile( TMachine * machine, const TResourceFileInfo * info, const char * base, unsigned * bufsize )
{
    unsigned char * result = 0;
    
    TString home( basePath );

    if( home.wstr()[home.length()-1] != PATH_SEPARATOR ) {
        home.append(PATH_SEPARATOR);
    }
    
    if( base != 0 ) {
        home += base;
        home.append(PATH_SEPARATOR);
    }

    TString file = home + info->name;
    TCRC32 crc;
    TInputStream * is = TFileInputStream::open( file.cstr() );
    
    if( is != 0 ) {
        unsigned size = info->size;
        
        if( size == 0 ) {
            size = getFileSize( file.cstr() );
        }
        
        result = loadFileFromStream( is, size, &crc );
        
        *bufsize = size;
        
        delete is;
    }
    else {
        for( int i=0; i<machine->getResourceCount(); i++ ) {
            file = home + machine->getResourceName(i) + ".zip";
            
            TZipFile * zf = TZipFile::open( file.cstr() );
            
            if( zf != 0 ) {
                const TZipEntry * ze = zf->entry( info->name, true );
                
                if( ze != 0 ) {
                    is = ze->open();
                    
                    if( is != 0 ) {
                        *bufsize = info->size ? info->size : ze->size();
                        result = loadFileFromStream( is, *bufsize, &crc );
                    }
                    
                    delete is;
                }
                
                delete zf;
            }
            
            if( result != 0 )
                break;
        }
    }
    
    if( result != 0 ) {
        // Delete buffer if CRC check fails
        if( (info->crc != 0) && (crc.value() != info->crc) ) {
            delete result;
            result = 0;
        }
    }
    
    return result;
}

// Load the files required by the driver
bool loadMachineFiles( TMachine * machine, TList & failedList )
{
    bool result = true;
    const TMachineDriverInfo * info = machine->getDriverInfo();
    
    for( int i=0; i<info->resourceFileCount(); i++ ) {
        const TResourceFileInfo * file = info->resourceFile( i );
        unsigned size = 0;
        unsigned char * buf = loadFile( machine, file, "roms", &size );
        
        if( (buf == 0) && (strstr(file->name,".wav") != 0) ) {
            buf = loadFile( machine, file, "samples", &size );
        }
        
        if( buf != 0 ) {
            machine->setResourceFile( file->id, buf, size );
            delete buf;
        }
        else {
            failedList.add( (void *)file->name );
            result = false;
        }
    }
    
    return result;
}

TMachine * loadGame( const char * name )
{
    TMachine * m = 0;
    
    int index = TGameRegistry::instance().find( name );
    
    if( index < 0 ) {
        printf( "The specified driver was not found\n" );
    }
    
    const TGameRegistryItem * t = TGameRegistry::instance().item( index );
    
    if( t != 0 ) {
        m = TMachine::createInstance( t->factory() );
    }
    
    // Load machine
    if( m != 0 ) {
        TList badFiles;
        bool ok = loadMachineFiles( m, badFiles );
        
        if( ! ok ) {
            // Unable to load required files
            TString msg = "One or more files could not be loaded:\n";
            
            for( int i=0; i<badFiles.count(); i++ ) {
                msg += "- ";
                msg += (char *) badFiles.item(i);
                msg += "\n";
            }
            
            msg += "\nFiles were looked for in the \"roms\" folder (also \"samples\""
            "\nfor sounds) and in the following archives:\n";
            
            for( int j=0; j<m->getResourceCount(); j++ ) {
                msg += "- ";
                msg += m->getResourceName( j );
                msg += ".zip\n";
            }
            
            printf( "%s\n", msg.cstr() );
            
            if( ! ok ) {
                delete m;
                m = 0;
            }
        }
    }
    
    return m;
}

int main(int argc, char** argv) {
    printf( "Tickle 0.95\n" );
    
    // Initialize base directory information
    basePath = (char *) malloc(PATH_MAX+1);
    getcwd(basePath, PATH_MAX+1);
    
    // Load game
    TMachine * machine = 0;
    TEmuInputManager inputManager;
    TJoystick * joy[4] = { 0, 0, 0, 0 };
    SDLMainOptions options;
    
    const char * driver = 0;
    
    for( int i=1; i<argc; i++ ) {
        const char * a = argv[i];

        if( ! strcmp(a,"-list") ) {
            TGameRegistry & reg = TGameRegistry::instance();
            
            reg.sort();
            
            for( int i=0; i<reg.count(); i++ ) {
                const TGameRegistryItem * t = reg.item( i );
                printf( "%s (%s)\n", t->info()->driver, t->name() );
            }
            
            return  EXIT_SUCCESS;
        }
        else if( ! strcmp(a,"-help") || ! strcmp(a,"-?") ) {
            printf( "-fs    fullscreen mode (default is windowed)\n" );
            printf( "-list  list available drivers\n" );
            
            return  EXIT_SUCCESS;
        }
        else if( ! strcmp(a,"-fs") ) {
            options.fullscreen = true;
        }
        else {
            driver = a;
        }
    }
    
    if( driver ) {
        machine = loadGame( driver );
        if( ! machine ) {
            printf( "Cannot load driver: %s\n", driver );
            printf( "(Use the -list option to get a list of drivers)\n");
            return EXIT_FAILURE;
        }
    }
    else {
        printf( "No driver specified: entering logo mode. Press LCTRL+1+5 to enter test mode.\n" );
        printf( "(Use the -list option to get a list of drivers)\n");
        machine = TTickleMachine::create();
    }
    
    // Setup emulator
    inputManager.add( SDLK_LCTRL, idKeyP1Action1 );
    inputManager.add( SDLK_SPACE, idKeyP1Action1 );
    inputManager.add( SDLK_z, idKeyP1Action2 ); // Z
    inputManager.add( SDLK_x, idKeyP1Action3 ); // X
    inputManager.add( SDLK_c, idKeyP1Action4 ); // C
    
    inputManager.add( SDLK_e, idKeyP2Action1 ); // E
    inputManager.add( SDLK_r, idKeyP2Action2 ); // R
    inputManager.add( SDLK_t, idKeyP2Action3 ); // T
    inputManager.add( SDLK_g, idKeyP2Action4 ); // G
    
    inputManager.add( SDLK_1, idKeyStartPlayer1 ); // 1
    inputManager.add( SDLK_2, idKeyStartPlayer2 ); // 2
    inputManager.add( SDLK_5, idCoinSlot1 ); // 5
    inputManager.add( SDLK_6, idCoinSlot2 ); // 6
    
    inputManager.add( SDLK_0, idKeyService1 ); // 0
    
    // Joysticks for player 1
    joy[0] = inputManager.addJoystick( idJoyP1Joystick1, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN );
    joy[0]->bindButtonToKey( 0, idKeyP1Action1 );
    joy[0]->bindButtonToKey( 1, idKeyP1Action2 );
    joy[0]->bindButtonToKey( 2, idKeyP1Action3 );
    joy[0]->bindButtonToKey( 3, idKeyP1Action4 );
    joy[0]->bindButtonToKey( 4, idKeyP1Action1 );  // To move flippers with a gamepad!
    joy[0]->bindButtonToKey( 5, idKeyP1Action2 );
    joy[0]->bindButtonToKey( 6, idKeyStartPlayer1 );
    joy[0]->bindButtonToKey( 7, idCoinSlot1 );
    
    joy[2] = inputManager.addJoystick( idJoyP1Joystick2, SDLK_a, SDLK_d, SDLK_w, SDLK_s );
    
    // Joysticks for player 2
    joy[1] = inputManager.addJoystick( idJoyP2Joystick1, SDLK_j, SDLK_l, SDLK_i, SDLK_k );
    joy[1]->bindButtonToKey( 0, idKeyP2Action1 );
    joy[1]->bindButtonToKey( 1, idKeyP2Action2 );
    joy[1]->bindButtonToKey( 2, idKeyP2Action3 );
    joy[1]->bindButtonToKey( 3, idKeyP2Action4 );
    joy[1]->bindButtonToKey( 6, idKeyStartPlayer2 );
    joy[1]->bindButtonToKey( 7, idCoinSlot1 );

    // Initialize SDL
    SDLMain sdl;
    
    const TMachineDriverInfo * info = machine->getDriverInfo();
    
    options.w = 2*info->machineInfo()->screenWidth;
    options.h = 2*info->machineInfo()->screenHeight;
    
    if( ! sdl.init( options ) ) {
        return EXIT_FAILURE;
    }
    
    bool running;
    bool paused = false;
    
    running = sdl.go( machine );
    
    // Event loop
    while( running ) {
        SDL_Event e;
        
        while( SDL_PollEvent(&e) ) {
            switch( e.type ) {
                case SDL_USEREVENT:
                    switch( e.user.code ) {
                        case SDLTickleEvent_AddFrame:
                            // Update joystick status: for now, only 2 joysticks are supported
                            for( int i=0; i<2; i++ ) {
                                Sint16 x;
                                Sint16 y;
                                unsigned buttons;
                                
                                if( sdl.joystick_status( i, &x, &y, &buttons ) ) {
                                    joy[i]->setPosition( x, y );
                                    joy[i]->setButtons( buttons );
                                }
                            }
                            
                            inputManager.notifyJoysticks( machine );
                            
                            sdl.add_frame( machine );
                            break;
                        case SDLTickleEvent_RenderTexture:
                            sdl.render( (SDL_Texture *) e.user.data1 );
                            break;
                        case SDLTickleEvent_DestroyTexture:
                            SDL_DestroyTexture( (SDL_Texture *) e.user.data1 );
                            break;
                    }
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYUP:
                    if( ! inputManager.handle( e.key.keysym.sym, 0, machine ) ) {
                        // Unhandled key
                    }
                    break;
                case SDL_KEYDOWN:
                    if( ! inputManager.handle( e.key.keysym.sym, 1, machine ) ) {
                        // Unhandled key
                        switch( e.key.keysym.sym ) {
                            case SDLK_ESCAPE:
                                running = false;
                                break;
                            case SDLK_p:
                                paused = ! paused;
                                if( paused ) {
                                    sdl.audio_stop();
                                }
                                else {
                                    sdl.audio_play();
                                }
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    return EXIT_SUCCESS;
}
