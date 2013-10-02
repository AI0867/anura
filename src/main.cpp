/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "graphics.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <cstdio>
#if !defined(_MSC_VER)
#include <sys/wait.h>
#endif

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>

#ifdef TARGET_OS_HARMATTAN
#include <glib-object.h>
#endif

#include "asserts.hpp"
#include "background_task_pool.hpp"
#include "checksum.hpp"
#include "controls.hpp"
#include "custom_object.hpp"
#include "custom_object_functions.hpp"
#include "custom_object_type.hpp"
#include "draw_scene.hpp"
#ifndef NO_EDITOR
#include "editor.hpp"
#endif
#include "difficulty.hpp"
#include "external_text_editor.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "foreach.hpp"
#include "formula_callable_definition.hpp"
#include "formula_object.hpp"
#include "formula_profiler.hpp"
#include "framed_gui_element.hpp"
#include "graphical_font.hpp"
#include "gui_section.hpp"
#include "i18n.hpp"
#include "input.hpp"
#include "ipc.hpp"
#include "iphone_device_info.h"
#include "joystick.hpp"
#include "json_parser.hpp"
#include "level.hpp"
#include "level_object.hpp"
#include "level_runner.hpp"
#include "load_level.hpp"
#include "loading_screen.hpp"
#include "message_dialog.hpp"
#include "module.hpp"
#include "multiplayer.hpp"
#include "player_info.hpp"
#include "preferences.hpp"
#include "preprocessor.hpp"
#include "raster.hpp"
#include "sound.hpp"
#include "stats.hpp"
#include "string_utils.hpp"
#include "surface_cache.hpp"
#include "tbs_internal_server.hpp"
#include "texture.hpp"
#include "texture_frame_buffer.hpp"
#include "tile_map.hpp"
#include "unit_test.hpp"
#include "variant_utils.hpp"

#if defined(__APPLE__) && SDL_VERSION_ATLEAST(2, 0, 0)
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC
    #define decimal decimal_carbon
        #import <Cocoa/Cocoa.h>
        #undef decimal
    #endif
#endif

#if defined(USE_BOX2D)
#include "b2d_ffl.hpp"
#endif

#if defined(TARGET_PANDORA) || defined(TARGET_TEGRA)
#include "eglport.h"
#elif defined(TARGET_BLACKBERRY)
#include <EGL/egl.h>
#endif

#define DEFAULT_MODULE	"frogatto"

#if !SDL_VERSION_ATLEAST(2, 0, 0)
#if defined(USE_SHADERS)
#include "wm.hpp"
window_manager wm;
#endif
#endif

namespace {

bool show_title_screen(std::string& level_cfg)
{
	//currently the titlescreen is disabled.
	return false;
}

void print_help(const std::string& argv0)
{
	std::cout << "Usage: " << argv0 << " [OPTIONS]\n" <<
"\n" <<
"User options:\n" <<
//"      --bigscreen              FIXME\n" <<
"      --config-path=PATH       sets the path to the user config dir\n" <<
"      --fullscreen             starts in fullscreen mode\n" <<
"      --height[=]NUM           sets the game window height to which contents\n" <<
"                                 are scaled\n" <<
"      --host                   set the game server host address\n" <<
"      --[no-]joystick          enables/disables joystick support\n" <<
"      --level[=]LEVEL_FILE     starts the game using the specified level file,\n" <<
"                                 relative to the level path\n" <<
"      --level-path=PATH        sets the path to the game level files\n" <<
"      --[no-]music             enables/disables game music\n" <<
"      --native                 one pixel in-game equals one pixel on monitor\n" <<
"      --relay                  use the server as a relay in multiplayer rather\n" <<
"                                 than trying to initiate direct connections\n" <<
"      --[no-]resizable         allows/disallows to resize the game window\n" <<
"      ----module-args=ARGS     map of arguments passed to the module\n" <<
"      --scale                  enables an experimental pixel art interpolation\n" <<
"                                 algorithm for scaling the game graphics (some\n" <<
"                                 issues with this still have to be solved)\n" <<
"      --[no-]send-stats        enables/disables sending game statistics over\n"
"                                 the network\n" <<
"      --server=URL             sets the server to use for the TBS client based\n"
"                                 on the given url\n" <<
"      --user=USERNAME          sets the username to use as part of the TBS\n"
"                                 server and module system\n" <<
"      --pass=PASSWORD          sets the password to use as part of the TBS\n"
"                                 server and module system\n" <<
"      --[no-]sound             enables/disables sound and music support\n" <<
"      --widescreen             sets widescreen mode, increasing the game view\n" <<
"                                 area for wide screen displays\n" <<
"      --width[=]NUM            sets the game window width to which contents are\n" <<
"                                 scaled\n" <<
"      --windowed               starts in windowed mode\n" <<
"      --wvga                   sets the display size to 800x480\n" <<
"\n" <<
"Diagnostic options:\n" <<
"      --[no-]debug             enables/disables debug mode\n" <<
"      --[no-]fps               enables/disables framerate display\n" <<
"      --set-fps=FPS            sets the framerate to FPS\n" <<
"      --potonly                use power of two-sized textures only\n" <<
"      --textures16             use 16 bpp textures only (default on iPhone)\n" <<
"      --textures32             use 32 bpp textures (default on PC/Mac)\n" <<
"\n" <<
"Developer options:\n" <<
"      --benchmarks             runs all the engine's benchmarks (intended to\n" <<
"                                 measure the speed of certain low-level\n" <<
"                                 functions), only useful if you're actually\n" <<
"                                 hacking on the engine to optimize the speed\n" <<
"                                 of these\n" <<
"      --benchmarks=NAME        runs a single named benchmark code\n" <<
"      --[no-]compiled          enable or disable precompiled game data\n" <<
"      --edit                   starts the game in edit mode.\n" <<
//"      --profile                FIXME\n" <<
//"      --profile=FILE           FIXME\n" <<
"      --show-hitboxes          turns on the display of object hitboxes\n" <<
"      --show-controls          turns on the display of iPhone control hitboxes\n" <<
"      --simipad                changes various options to emulate an iPad\n" <<
"                                 environment\n" <<
"      --simiphone              changes various options to emulate an iPhone\n" <<
"                                 environment\n" <<
"      --no-autopause           Stops the game from pausing automatically\n" <<
"                                 when it loses focus\n" <<
"      --tests                  runs the game's unit tests and exits\n" <<
"      --no-tests               skips the execution of unit tests on startup\n"
"      --utility=NAME           runs the specified UTILITY( NAME ) code block,\n" <<
"                                 such as compile_levels or compile_objects,\n" <<
"                                 with the specified arguments\n"
	;
}

}

#if defined(UTILITY_IN_PROC)
boost::shared_ptr<char> child_args;

#if defined(_MSC_VER)
const std::string shared_sem_name = "Local\anura_local_process_semaphore";
HANDLE child_process;
HANDLE child_thread;
HANDLE child_stderr;
HANDLE child_stdout;
#else
const std::string shared_sem_name = "/anura_local_process_semaphore";
pid_t child_pid;
#endif

bool create_utility_process(const std::string& app, const std::vector<std::string>& argv)
{
#if defined(_MSC_VER)
	char app_np[MAX_PATH];
	// Grab the full path name
	DWORD chararacters_copied = GetModuleFileNameA(NULL, app_np,  MAX_PATH);
	ASSERT_LOG(chararacters_copied > 0, "Failed to get module name: " << GetLastError());
	std::string app_name_and_path(app_np, chararacters_copied);

	// windows version
	std::string command_line_params;
	command_line_params += app_name_and_path + " ";
	for(size_t n = 0; n != argv.size(); ++n) {
		command_line_params += argv[n] + " ";
	}
	child_args = boost::shared_ptr<char>(new char[command_line_params.size()+1]);
	memset(child_args.get(), 0, command_line_params.size()+1);
	memcpy(child_args.get(), &command_line_params[0], command_line_params.size());

	STARTUPINFOA siStartupInfo; 
	PROCESS_INFORMATION piProcessInfo;
	SECURITY_ATTRIBUTES saFileSecurityAttributes;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);
	saFileSecurityAttributes.nLength = sizeof(saFileSecurityAttributes);
	saFileSecurityAttributes.lpSecurityDescriptor = NULL;
	saFileSecurityAttributes.bInheritHandle = true;
	child_stderr = siStartupInfo.hStdError = CreateFileA("stderr_server.txt", GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, &saFileSecurityAttributes, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_LOG(siStartupInfo.hStdError != INVALID_HANDLE_VALUE, 
		"Unable to open stderr_server.txt for child process.");
	child_stdout = siStartupInfo.hStdOutput = CreateFileA("stdout_server.txt", GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, &saFileSecurityAttributes, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_LOG(siStartupInfo.hStdOutput != INVALID_HANDLE_VALUE, 
		"Unable to open stdout_server.txt for child process.");
	siStartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	siStartupInfo.dwFlags = STARTF_USESTDHANDLES;
	std::cerr << "CREATE CHILD PROCESS: " << app_name_and_path << std::endl;
	ASSERT_LOG(CreateProcessA(app_name_and_path.c_str(), child_args.get(), NULL, NULL, true, CREATE_DEFAULT_ERROR_MODE, 0, 0, &siStartupInfo, &piProcessInfo),
		"Unable to create child process for utility: " << GetLastError());
	child_process = piProcessInfo.hProcess;
	child_thread = piProcessInfo.hThread;
#else
	// everyone else version using fork()
	//...
	child_pid = fork();
	if(child_pid == 0) {
		FILE* fout = std::freopen("stdout_server.txt","w", stdout);
		FILE* ferr = std::freopen("stderr_server.txt","w", stderr);
		std::cerr.sync_with_stdio(true);
	}
	ASSERT_LOG(child_pid >= 0, "Unable to fork process: " << errno);
#endif
	// Create a semaphore to signal termination.
	ASSERT_LOG(ipc::semaphore::create(shared_sem_name, 0), 
		"Unable to create shared semaphore");
#if defined(_MSC_VER)
	return false;
#else
	return child_pid == 0;
#endif
}

void terminate_utility_process()
{
	ipc::semaphore::post();
#if defined(_MSC_VER)
	WaitForSingleObject(child_process, INFINITE);
	CloseHandle(child_process);
	CloseHandle(child_thread);
	CloseHandle(child_stderr);
	CloseHandle(child_stdout);
#else
	// .. close child or whatever.
	int status;
	if(waitpid(child_pid, &status, 0) != child_pid) {
		std::cerr << "Error waiting for child process to finish: " << errno << std::endl;
	}
#endif
}
#endif


#if defined(__ANDROID__)
#include <jni.h>
#include <android/asset_manager_jni.h>
AAssetManager* static_assetManager = 0;
extern "C" void app_set_asset_manager(AAssetManager* assetMan)
{
	static_assetManager = assetMan;	
}
namespace sys {
AAssetManager* GetJavaAssetManager()
{
	return static_assetManager;
}
}
#endif

#if defined(__native_client__)
void register_file_and_data(const char* filename, const char* mode, char* buffer, int size)
{
}
#endif

int load_module(const std::string& mod, std::vector<std::string>* argv)
{
	variant mod_info = module::get(mod);
	if(mod_info.is_null()) {
		return -1;
	}
	module::reload(mod);
	if(mod_info["arguments"].is_list()) {
		const std::vector<std::string>& arguments = mod_info["arguments"].as_list_string();
		argv->insert(argv->end(), arguments.begin(), arguments.end());
	}	
	return 0;
}

#if defined(__native_client__)
extern "C" int game_main(int argcount, char* argvec[])
#else
extern "C" int main(int argcount, char* argvec[])
#endif
{
#if defined(__native_client__)
	std::cerr << "Running game_main" << std::endl;

	chdir("/frogatto");
	{
		char buf[256];
		const char* const res = getcwd(buf,sizeof(buf));
		std::cerr << "Current working directory: " << res << std::endl;
	}
#endif 

#if defined(_MSC_VER)
	std::shared_ptr<FILE> f_stderr = std::shared_ptr<FILE>(freopen ("stderr.txt", "wt", stderr), [](FILE* f){fclose(f); delete f;});
#endif

#if defined(__APPLE__) && TARGET_OS_MAC
    chdir([[[NSBundle mainBundle] resourcePath] fileSystemRepresentation]);
#endif

	#ifdef NO_STDERR
	std::freopen("/dev/null", "w", stderr);
	std::cerr.sync_with_stdio(true);
	#endif

	std::cerr << "Frogatto engine version " << preferences::version() << "\n";
	LOG( "After print engine version" );

	#if defined(TARGET_BLACKBERRY)
		chdir("app/native");
		std::cout<< "Changed working directory to: " << getcwd(0, 0) << std::endl;
	#endif

	game_logic::init_callable_definitions();

	std::string level_cfg = "titlescreen.cfg";
	bool unit_tests_only = false, skip_tests = false;
	bool run_benchmarks = false;
	std::vector<std::string> benchmarks_list;
	std::string utility_program;
	std::vector<std::string> util_args;
	std::string server = "wesnoth.org";
#if defined(UTILITY_IN_PROC)
	bool create_utility_in_new_process = false;
	std::string utility_name;
#endif
	bool is_child_utility = false;

	const char* profile_output = NULL;
	std::string profile_output_buf;

#if defined(__ANDROID__)
	//monstartup("libapplication.so");
#endif

	std::string orig_level_cfg = level_cfg;
	std::string override_level_cfg = "";

	int modules_loaded = 0;

	std::vector<std::string> argv;
	for(int n = 1; n < argcount; ++n) {
#if defined(UTILITY_IN_PROC)
		std::string sarg(argvec[n]);
		if(sarg.compare(0, 15, "--utility-proc=") == 0) {
			create_utility_in_new_process = true;
			utility_name = "--utility-child=" + sarg.substr(15);
		} else {
			argv.push_back(argvec[n]);
		}
#else
		argv.push_back(argvec[n]);
#endif
        
        if(argv.size() >= 2 && argv[argv.size()-2] == "-NSDocumentRevisionsDebugMode" && argv.back() == "YES") {
            //XCode passes these arguments by default when debugging -- make sure they are ignored.
            argv.resize(argv.size()-2);
        }
	}

	std::cerr << "Build Options:";
	for(auto bo : preferences::get_build_options()) {
		std::cerr << " " << bo;
	}
	std::cerr << std::endl;

#if defined(UTILITY_IN_PROC)
	if(create_utility_in_new_process) {
		argv.push_back(utility_name);
#if defined(_MSC_VER)
		// app name is ignored for windows, we get windows to tell us.
		is_child_utility = create_utility_process("", argv);
#else 
		is_child_utility = create_utility_process(argvec[0], argv);
#endif
		if(!is_child_utility) {
			argv.pop_back();
		}
#if defined(_MSC_VER)
		atexit(terminate_utility_process);
#endif
	}
#endif

	if(sys::file_exists("./master-config.cfg")) {
		std::cerr << "LOADING CONFIGURATION FROM master-config.cfg" << std::endl;
		variant cfg = json::parse_from_file("./master-config.cfg");
		if(cfg.is_map()) {
			if( cfg["id"].is_null() == false) {
				std::cerr << "SETTING MODULE PATH FROM master-config.cfg: " << cfg["id"].as_string() << std::endl;
				preferences::set_preferences_path_from_module(cfg["id"].as_string());
				//XXX module::set_module_name(cfg["id"].as_string(), cfg["id"].as_string());
			}
			if(cfg["arguments"].is_null() == false) {
				std::vector<std::string> additional_args = cfg["arguments"].as_list_string();
				argv.insert(argv.begin(), additional_args.begin(), additional_args.end());
				std::cerr << "ADDING ARGUMENTS FROM master-config.cfg:";
				for(size_t n = 0; n < cfg["arguments"].num_elements(); ++n) {
					std::cerr << " " << cfg["arguments"][n].as_string();
				}
				std::cerr << std::endl;
			}
		}
	}

	stats::record_program_args(argv);

	for(size_t n = 0; n < argv.size(); ++n) {
		const int argc = argv.size();
		const std::string arg(argv[n]);
		std::string arg_name, arg_value;
		std::string::const_iterator equal = std::find(arg.begin(), arg.end(), '=');
		if(equal != arg.end()) {
			arg_name = std::string(arg.begin(), equal);
			arg_value = std::string(equal+1, arg.end());
		}
		if(arg_name == "--module") {
			if(load_module(arg_value, &argv) != 0) {
				std::cerr << "FAILED TO LOAD MODULE: " << arg_value << "\n";
				return -1;
			}
			++modules_loaded;
		} else if(arg == "--tests") {
			unit_tests_only = true;
		}
	}

	if(modules_loaded == 0 && !unit_tests_only) {
		if(load_module(DEFAULT_MODULE, &argv) != 0) {
			std::cerr << "FAILED TO LOAD MODULE: " << DEFAULT_MODULE << "\n";
			return -1;
		}
	}

	preferences::load_preferences();
	LOG( "After load_preferences()" );

	// load difficulty settings after module, before rest of args.
	difficulty::manager();

	for(size_t n = 0; n < argv.size(); ++n) {
		const size_t argc = argv.size();
		const std::string arg(argv[n]);
		std::string arg_name, arg_value;
		std::string::const_iterator equal = std::find(arg.begin(), arg.end(), '=');
		if(equal != arg.end()) {
			arg_name = std::string(arg.begin(), equal);
			arg_value = std::string(equal+1, arg.end());
		}
		std::cerr << "ARGS: " << arg << std::endl;
		if(arg.substr(0,4) == "-psn") {
			// ignore.
		} else if(arg_name == "--module") {
			// ignore already processed.
		} else if(arg_name == "--profile" || arg == "--profile") {
			profile_output_buf = arg_value;
			profile_output = profile_output_buf.c_str();
		} else if(arg_name == "--utility" || arg_name == "--utility-child") {
			if(arg_name == "--utility-child") {
				is_child_utility = true;
			}
			utility_program = arg_value;
			for(++n; n < argc; ++n) {
				const std::string arg(argv[n]);
				util_args.push_back(arg);
			}

			break;
		} else if(arg == "--benchmarks") {
			run_benchmarks = true;
		} else if(arg_name == "--benchmarks") {
			run_benchmarks = true;
			benchmarks_list = util::split(arg_value);
		} else if(arg == "--tests") {
			// ignore as already processed.
		} else if(arg == "--no-tests") {
			skip_tests = true;
		} else if(arg_name == "--width") {
			std::string w(arg_value);
			preferences::set_actual_screen_width(boost::lexical_cast<int>(w));
		} else if(arg == "--width" && n+1 < argc) {
			std::string w(argv[++n]);
			preferences::set_actual_screen_width(boost::lexical_cast<int>(w));
		} else if(arg_name == "--height") {
			std::string h(arg_value);
			preferences::set_actual_screen_height(boost::lexical_cast<int>(h));
		} else if(arg == "--height" && n+1 < argc) {
			std::string h(argv[++n]);
			preferences::set_actual_screen_height(boost::lexical_cast<int>(h));
		} else if(arg_name == "--level") {
			override_level_cfg = arg_value;
		} else if(arg == "--level" && n+1 < argc) {
			override_level_cfg = argv[++n];
		} else if(arg_name == "--host") {
			server = arg_value;
		} else if(arg == "--host" && n+1 < argc) {
			server = argv[++n];
		} else if(arg == "--compiled") {
			preferences::set_load_compiled(true);
#ifndef NO_EDITOR
		} else if(arg == "--edit") {
			preferences::set_edit_on_start(true);
#endif
		} else if(arg == "--no-compiled") {
			preferences::set_load_compiled(false);
#if defined(TARGET_PANDORA)
		} else if(arg == "--no-fbo") {
			preferences::set_fbo(false);
		} else if(arg == "--no-bequ") {
			preferences::set_bequ(false);
#endif
		} else if(arg == "--help" || arg == "-h") {
			print_help(std::string(argvec[0]));
			return 0;
		} else {
			const bool res = preferences::parse_arg(argv[n].c_str());
			if(!res) {
				std::cerr << "unrecognized arg: '" << arg << "'\n";
				return -1;
			}
		}
	}

	checksum::manager checksum_manager;
#ifndef NO_EDITOR
	sys::filesystem_manager fs_manager;
#endif // NO_EDITOR

	preferences::expand_data_paths();

	background_task_pool::manager bg_task_pool_manager;
	LOG( "After expand_data_paths()" );

	std::cerr << "Preferences dir: " << preferences::user_data_path() << '\n';

	//make sure that the user data path exists.
	if(!preferences::setup_preferences_dir()) {
		std::cerr << "cannot create preferences dir!\n";
	}

	std::cerr << "\n";

	const tbs::internal_server_manager internal_server_manager_scope(preferences::internal_tbs_server());

	if(utility_program.empty() == false 
		&& test::utility_needs_video(utility_program) == false) {
#if defined(UTILITY_IN_PROC)
		if(is_child_utility) {
			ASSERT_LOG(ipc::semaphore::create(shared_sem_name, 1) != false, 
				"Unable to create shared semaphore: " << errno);
			std::cerr.sync_with_stdio(true);
		}
#endif
		test::run_utility(utility_program, util_args);
		return 0;
	}

#if defined(TARGET_PANDORA)
    EGL_Open();
#endif

#if defined(__ANDROID__)
	std::freopen("stdout.txt","w",stdout);
	std::freopen("stderr.txt","w",stderr);
	std::cerr.sync_with_stdio(true);
#endif

	LOG( "Start of main" );

	if(!skip_tests && !test::run_tests()) {
		return -1;
	}

	if(unit_tests_only) {
		return 0;
	}

#if !defined(__native_client__)
	Uint32 sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK;
#if defined(_WINDOWS) || TARGET_OS_IPHONE
	sdl_init_flags |= SDL_INIT_TIMER;
#endif
	if(SDL_Init(sdl_init_flags) < 0) {
		std::cerr << "could not init SDL\n";
		return -1;
	}
	LOG( "After SDL_Init" );
#endif

#ifdef TARGET_OS_HARMATTAN
	g_type_init();
#endif
	i18n::init ();
	LOG( "After i18n::init()" );

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#if TARGET_OS_IPHONE || defined(TARGET_BLACKBERRY) || defined(__ANDROID__)
	//on the iPhone and PlayBook, try to restore the auto-save if it exists
	if(sys::file_exists(preferences::auto_save_file_path()) && sys::read_file(std::string(preferences::auto_save_file_path()) + ".stat") == "1") {
		level_cfg = "autosave.cfg";
		sys::write_file(std::string(preferences::auto_save_file_path()) + ".stat", "0");

	}
#endif

	if(override_level_cfg.empty() != true) {
		level_cfg = override_level_cfg;
		orig_level_cfg = level_cfg;
	}

#if !SDL_VERSION_ATLEAST(2, 0, 0) && defined(USE_SHADERS)
	wm.create_window(preferences::actual_screen_width(),
		preferences::actual_screen_height(),
		0,
		(preferences::resizable() ? SDL_RESIZABLE : 0) | (preferences::fullscreen() ? SDL_FULLSCREEN : 0));
#else

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
	int width, height;
	iphone_screen_res(&width, &height);
	preferences::set_actual_screen_width(width);
	preferences::set_actual_screen_height(height);
	int multiplier = 2;
	if (width > 320)
	{
		//preferences::set_use_pretty_scaling(true);
		multiplier = 1;
	}
	preferences::set_virtual_screen_width(height*multiplier);
	preferences::set_virtual_screen_height(width*multiplier);
	preferences::set_control_scheme(height % 1024 ? "iphone_2d" : "ipad_2d");
	
	SDL_WindowID windowID = SDL_CreateWindow (NULL, 0, 0, preferences::actual_screen_width(), preferences::actual_screen_height(),
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
		SDL_WINDOW_BORDERLESS);
	if (windowID == 0) { 
		std::cerr << "Could not create window: " << SDL_GetError() << "\n"; 
		return -1;
	}
	
	//	if (SDL_GL_CreateContext(windowID) == 0) {
	//		std::cerr << "Could not create GL context: " << SDL_GetError() << "\n";
	//		return -1;
	//	}
	if (SDL_CreateRenderer(windowID, -1, 0) != 0) {
		std::cerr << "Could not create renderer\n";
		return -1;
	}
	
#else
#ifdef TARGET_OS_HARMATTAN
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	if (SDL_SetVideoMode(preferences::actual_screen_width(),preferences::actual_screen_height(),0,SDL_OPENGLES | SDL_FULLSCREEN) == NULL) {
		std::cerr << "could not set video mode\n";
		return -1;
	}

	preferences::init_oes();
	SDL_ShowCursor(0);
#else

#if defined(TARGET_PANDORA)
	if (SDL_SetVideoMode(preferences::actual_screen_width(),preferences::actual_screen_height(),16,SDL_FULLSCREEN) == NULL) {
		std::cerr << "could not set video mode\n";
		return -1;
	}
    EGL_Init();
    preferences::init_oes();
#elif defined(TARGET_TEGRA)
	//if (SDL_SetVideoMode(preferences::actual_screen_width(),preferences::actual_screen_height(),0,preferences::resizable() ? SDL_RESIZABLE : 0|preferences::fullscreen() ? SDL_FULLSCREEN : 0) == NULL) {
	if (SDL_SetVideoMode(preferences::actual_screen_width(),preferences::actual_screen_height(),0,SDL_FULLSCREEN) == NULL) {
		std::cerr << "could not set video mode\n";
		return -1;
	}
    EGL_Init();
    preferences::init_oes();
#elif defined(TARGET_BLACKBERRY)
	if (SDL_SetVideoMode(preferences::actual_screen_width(),preferences::actual_screen_height(),0,SDL_OPENGL|SDL_FULLSCREEN) == NULL) {
		std::cerr << "could not set video mode\n";
		return -1;
	}
	preferences::init_oes();
#elif defined(__ANDROID__)
#if SDL_VERSION_ATLEAST(2, 0, 0)
	int num_video_displays = SDL_GetNumVideoDisplays();
	SDL_Rect r;
	if(num_video_displays < 0) {
		std::cerr << "no video displays available" << std::endl;
		return -1;
	}
	if(SDL_GetDisplayBounds(0, &r) < 0) {
        preferences::set_actual_screen_width(r.w);
        preferences::set_actual_screen_height(r.h);
		if(r.w < 640) {
        	preferences::set_virtual_screen_width(r.w*2);
        	preferences::set_virtual_screen_height(r.h*2);
		} else {
			preferences::set_virtual_screen_width(r.w);
			preferences::set_virtual_screen_height(r.h);
		}
		preferences::set_control_scheme(r.h >= 1024 ? "ipad_2d" : "android_med");
    }
#else
    SDL_Rect** r = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_OPENGL);
    if( r != (SDL_Rect**)0 && r != (SDL_Rect**)-1 ) {
        preferences::set_actual_screen_width(r[0]->w);
        preferences::set_actual_screen_height(r[0]->h);
		if(r[0]->w < 640) {
        	preferences::set_virtual_screen_width(r[0]->w*2);
        	preferences::set_virtual_screen_height(r[0]->h*2);
		} else {
			preferences::set_virtual_screen_width(r[0]->w);
			preferences::set_virtual_screen_height(r[0]->h);
		}
		preferences::set_control_scheme(r[0]->h >= 1024 ? "ipad_2d" : "android_med");
    }
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if(!graphics::set_video_mode(preferences::actual_screen_width(), preferences::actual_screen_height())) {
#else
    if (SDL_SetVideoMode(preferences::actual_screen_width(),preferences::actual_screen_height(),0,SDL_FULLSCREEN|SDL_OPENGL) == NULL) {
#endif
		std::cerr << "could not set video mode\n";
		return -1;
    }
#elif defined(__native_client__)
    SDL_Rect** r = SDL_ListModes(NULL, SDL_OPENGL);
	std::cerr << "Video modes";
	if(r == (SDL_Rect**)0) {
		std::cerr << "No modes available";
		return -1;
	}
	if(r == (SDL_Rect**)-1) {
		std::cerr << "All modes available";
	} else {
		for(int i = 0; r[i]; ++i) {
			std::cerr << r[i]->w << r[i]->h << std::endl;
		}
	}

    if (SDL_SetVideoMode(preferences::actual_screen_width(),preferences::actual_screen_height(),0,0) == NULL) {
		std::cerr << "could not set video mode\n";
		return -1;
    }
#else
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if(preferences::auto_size_window()) {
		const SDL_DisplayMode mode = graphics::set_video_mode_auto_select();
		preferences::set_actual_screen_width(mode.w);
		preferences::set_actual_screen_height(mode.h);
		preferences::set_virtual_screen_width(mode.w);
		preferences::set_virtual_screen_height(mode.h);
	} else if(!graphics::set_video_mode(preferences::actual_screen_width(), preferences::actual_screen_height(), SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL|(preferences::fullscreen() ? SDL_WINDOW_FULLSCREEN : 0))) {
#else
	if(SDL_SetVideoMode(preferences::actual_screen_width(),preferences::actual_screen_height(),0,SDL_OPENGL|(preferences::resizable() ? SDL_RESIZABLE : 0)|(preferences::fullscreen() ? SDL_FULLSCREEN : 0)) == NULL) {
#endif
		std::cerr << "could not set video mode\n";
		return -1;
	}
#ifndef __APPLE__
	graphics::surface wm_icon = graphics::surface_cache::get("window-icon.png");
	if(!wm_icon.null()) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_SetWindowIcon(graphics::get_window(), wm_icon.get());
#else
		SDL_WM_SetIcon(wm_icon, NULL);
#endif
	}
#endif // __APPLE__
#endif
#endif
#endif

#endif

#if !SDL_VERSION_ATLEAST(2, 0, 0) && defined(__GLEW_H__)
	GLenum glew_status = glewInit();
	ASSERT_EQ(glew_status, GLEW_OK);
#endif

#if defined(USE_SHADERS)
	if(glCreateShader == NULL) {
		const GLubyte* glstrings;
		if(glGetString != NULL && (glstrings = glGetString(GL_VERSION)) != NULL) {
			std::cerr << "OpenGL version: " << reinterpret_cast<const char *>(glstrings) << std::endl;
		}
		std::cerr << "glCreateShader is NULL. Check that your current video card drivers support "
			<< "an OpenGL version >= 2. Exiting." << std::endl;
		return 0;
	}
	// Has to happen after the call to glewInit().
	gles2::init_default_shader();
#endif

//	srand(time(NULL));

	const stats::manager stats_manager;
#ifndef NO_EDITOR
	const external_text_editor::manager editor_manager;
#endif // NO_EDITOR

#if defined(USE_BOX2D)
	box2d::manager b2d_manager;
#endif

	std::cerr << std::endl;
	const GLubyte* glstrings;
	if((glstrings = glGetString(GL_VENDOR)) != NULL) {
		std::cerr << "OpenGL vendor: " << reinterpret_cast<const char *>(glstrings) << std::endl;
	} else {
		GLenum err = glGetError();
		std::cerr << "Error in vendor string: " << std::hex << err << std::endl;
	}
	if((glstrings = glGetString(GL_VERSION)) != NULL) {
		std::cerr << "OpenGL version: " << reinterpret_cast<const char *>(glstrings) << std::endl;
	} else {
		GLenum err = glGetError();
		std::cerr << "Error in version string: " << std::hex << err << std::endl;
	}
	if((glstrings = glGetString(GL_EXTENSIONS)) != NULL) {
		std::cerr << "OpenGL extensions: " << reinterpret_cast<const char *>(glstrings) << std::endl;
	} else {
		GLenum err = glGetError();
		std::cerr << "Error in extensions string: " << std::hex << err << std::endl;
	}
#ifdef GL_SHADING_LANGUAGE_VERSION
	if((glstrings = glGetString(GL_SHADING_LANGUAGE_VERSION)) != NULL) {
		std::cerr << "GLSL Version: " << reinterpret_cast<const char *>(glstrings) << std::endl;
	} else {
		GLenum err = glGetError();
		std::cerr << "Error in GLSL string: " << std::hex << err << std::endl;
	}
#endif
	std::cerr << std::endl;

	GLint stencil_bits = 0;
	glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);
	std::cerr << "Stencil bits: " << stencil_bits << std::endl;

#if defined(USE_SHADERS)
#if !defined(GL_ES_VERSION_2_0)
	GLfloat min_pt_sz;
	glGetFloatv(GL_POINT_SIZE_MIN, &min_pt_sz);
	GLfloat max_pt_sz;
	glGetFloatv(GL_POINT_SIZE_MAX, &max_pt_sz);
	std::cerr << "Point size range: " << min_pt_sz << " < size < " << max_pt_sz << std::endl;
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
#endif

	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
#if !defined(USE_SHADERS)
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::cerr << "JOYSTICKS: " << SDL_NumJoysticks() << "\n";

	const load_level_manager load_manager;

	{ //manager scope
	const font::manager font_manager;
	const sound::manager sound_manager;
#if !defined(__native_client__)
	const joystick::manager joystick_manager;
#endif 
	
	graphics::texture::manager texture_manager;

#ifndef NO_EDITOR
	editor::manager editor_manager;
#endif

	variant preloads;
	loading_screen loader;
	try {
		variant gui_node = json::parse_from_file(preferences::load_compiled() ? "data/compiled/gui.cfg" : "data/gui.cfg");
		gui_section::init(gui_node);
		loader.draw_and_increment(_("Initializing GUI"));
		framed_gui_element::init(gui_node);

		sound::init_music(json::parse_from_file("data/music.cfg"));
		graphical_font::init_for_locale(i18n::get_locale());
		preloads = json::parse_from_file("data/preload.cfg");
		int preload_items = preloads["preload"].num_elements();
		loader.set_number_of_items(preload_items+7); // 7 is the number of items that will be loaded below
		custom_object::init();
		loader.draw_and_increment(_("Initializing custom object functions"));
		init_custom_object_functions(json::parse_from_file("data/functions.cfg"));
		loader.draw_and_increment(_("Initializing textures"));
		loader.load(preloads);
		loader.draw_and_increment(_("Initializing tiles"));
		tile_map::init(json::parse_from_file("data/tiles.cfg"));


		game_logic::formula_object::load_all_classes();

	} catch(const json::parse_error& e) {
		std::cerr << "ERROR PARSING: " << e.error_message() << "\n";
		return 0;
	}
	loader.draw(_("Loading level"));

#if defined(__native_client__)
	while(1) {
	}
#endif

#if defined(__APPLE__) && !(TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE) && !defined(USE_SHADERS)
	GLint swapInterval = 1;
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &swapInterval);
#endif

	loader.finish_loading();
	//look to see if we got any quit events while loading.
	{
	SDL_Event event;
	while(input::sdl_poll_event(&event)) {
		if(event.type == SDL_QUIT) {
			return 0;
		}
	}
	}

	formula_profiler::manager profiler(profile_output);

#ifdef USE_SHADERS
	texture_frame_buffer::init(preferences::actual_screen_width(), preferences::actual_screen_height());
#else
	texture_frame_buffer::init();
#endif

	if(run_benchmarks) {
		if(benchmarks_list.empty() == false) {
			test::run_benchmarks(&benchmarks_list);
		} else {
			test::run_benchmarks();
		}
		return 0;
	} else if(utility_program.empty() == false && test::utility_needs_video(utility_program) == true) {
		test::run_utility(utility_program, util_args);
		return 0;
	}

	bool quit = false;

	while(!quit && !show_title_screen(level_cfg)) {
		boost::intrusive_ptr<level> lvl(load_level(level_cfg));
		

#if !defined(__native_client__)
		//see if we're loading a multiplayer level, in which case we
		//connect to the server.
		multiplayer::manager mp_manager(lvl->is_multiplayer());
		if(lvl->is_multiplayer()) {
			multiplayer::setup_networked_game(server);
		}

		if(lvl->is_multiplayer()) {
			last_draw_position() = screen_position();
			std::string level_cfg = "waiting-room.cfg";
			boost::intrusive_ptr<level> wait_lvl(load_level(level_cfg));
			wait_lvl->finish_loading();
			wait_lvl->set_multiplayer_slot(0);
			if(wait_lvl->player()) {
				wait_lvl->player()->set_current_level(level_cfg);
			}
			wait_lvl->set_as_current_level();

			level_runner runner(wait_lvl, level_cfg, orig_level_cfg);

			multiplayer::sync_start_time(*lvl, boost::bind(&level_runner::play_cycle, &runner));

			lvl->set_multiplayer_slot(multiplayer::slot());
		}
#endif

		last_draw_position() = screen_position();

		assert(lvl.get());
		if(!lvl->music().empty()) {
			sound::play_music(lvl->music());
		}

		if(lvl->player() && level_cfg != "autosave.cfg") {
			lvl->player()->set_current_level(level_cfg);
			lvl->player()->get_entity().save_game();
		}

		set_scene_title(lvl->title());

		try {
			quit = level_runner(lvl, level_cfg, orig_level_cfg).play_level();
			level_cfg = orig_level_cfg;
		} catch(multiplayer_exception&) {
		}
	}

	level::clear_current_level();

	} //end manager scope, make managers destruct before calling SDL_Quit
//	controls::debug_dump_controls();
#if defined(TARGET_PANDORA) || defined(TARGET_TEGRA)
    EGL_Destroy();
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// Be nice and destroy the GL context and the window.
	graphics::set_video_mode(0, 0, CLEANUP_WINDOW_CONTEXT);
#elif defined(USE_SHADERS)
	wm.destroy_window();
#endif
	SDL_Quit();
	
	preferences::save_preferences();
	std::cerr << SDL_GetError() << "\n";

#if !defined(_MSC_VER) && defined(UTILITY_IN_PROC)
	if(create_utility_in_new_process) {
		terminate_utility_process();
	}
#endif

#ifdef _MSC_VER
	ExitProcess(0);
#endif

	std::set<variant*> loading;
	swap_variants_loading(loading);
	if(loading.empty() == false) {
		fprintf(stderr, "Illegal object: %p\n", (void*)(*loading.begin())->as_callable_loading());
		ASSERT_LOG(false, "Unresolved unserialized objects: " << loading.size());
	}

	return 0;
}
