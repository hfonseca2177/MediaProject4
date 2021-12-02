
#include <stdlib.h> // argc & V
#include <stdio.h> // fprintF
#include <string> // string
#include <sstream> // string stream
#include <glad/gl.h> 
#include <GLFW/glfw3.h> // for graphics
#include <FMOD/fmod.hpp> // Low Level Sound API
#include <rssgl/GLText.h> 
#include <glm/vec4.hpp> // vector4 color
#include <unordered_set>

//custom includes
#include"vColor.h"
#include "cSoundManager.h"

//Variables -------------------------
GLuint _windowWidth = 1366;
GLuint _windowHeight = 728;
vColor background_color = vColor(); // using default color
float panStep = 0.1f;
bool bypass = false;
float volstep = 0.1f;

//Global pointers
GLFWwindow* _window = NULL;
FMOD::System* _system = NULL;
RSS::GLText* _text;
cSoundManager* _soundManager;

const unsigned int MAX_TEXT_LINES = 25;

//normal string for App Name
std::string _appName = "Media Fundementals Project 4";


//callback for glfw
//error callback if we get error
void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

//To receive key press and release events, create a key callback function.
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {


	if (key == GLFW_KEY_RIGHT )
	{
		//change active group pan
		_soundManager->addpan(panStep);
		
	}
	if (key == GLFW_KEY_LEFT )
	{//change active group pan
		_soundManager->subpan(panStep);
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{	
		//pause and unpause active channel group
		_soundManager->pause();
	}

	//toggle bypass for dsp on the channel 
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{	
		_soundManager->changebypass(0);
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		_soundManager->changebypass(1);
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		_soundManager->changebypass(2);
	}
	

	//bypass all at once and unbypass
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		_soundManager->changebypassall(bypass);
	}
	//increase volume
	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
	{
		_soundManager->changeVol(volstep);
	}
	//decrease volume
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
	{
		_soundManager->changeVol(volstep*-1);
	}
	//channel groups control cycle
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		_soundManager->cycleChannelGroup();
	}
	//Modifying the Customizable dsp param
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		if (_soundManager->hasCustomizableParam())
		{
			_soundManager->AddToCustomizableParam();
		}
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		if (_soundManager->hasCustomizableParam())
		{
			_soundManager->SubToCustomizableParam();
		}
	}
	
	//key code
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			
		}
		
	}//custom bracket to hide stuff
}


//Prototypes
bool init(); //to init anything we want
bool GLFWinit(); // to specifically init GLFW items
bool initFMOD();// to init FMOD
bool initGL(); // For OpenGL
void shutdown(); // Close down Proccedure
void clearLines();//Clean text lines

int main(int argc, char* argv) {

	if (!init()) {

		fprintf(stderr, "initilizing Failed");
		exit(EXIT_FAILURE);
	}

	float previousTime = static_cast<float>(glfwGetTime());
	std::vector<std::string> lines; 
	
	std::stringstream info;
	while (!glfwWindowShouldClose(_window)) {

		float currentTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentTime - previousTime;
		previousTime = currentTime;

		glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clearing buffers
		
		clearLines();
		info.str("");

		unsigned int currentLine = 0;

		_text->addLine("-------------------------------------------------------------------------", currentLine++);
		//defind char array to store data from dsp
		#define BUFFER_SIZE 255
		char _buffer[BUFFER_SIZE];

		//get active group channel name 
		_soundManager->_activeChannelGroup->getName(_buffer, BUFFER_SIZE);
		//write the name on the screen
		_text->addLine(_buffer, currentLine++);

		HandleFmodError(_system->update(), "Update System State");

		
		//Loop through streaming sounds
		for (size_t i = 0; i < _soundManager->stream_Sounds.size(); i++)
		{
			cSoundResource* soundResource = _soundManager->stream_Sounds.at(i);

			//Update current sound infomation and state
			soundResource->Update();
			_text->addLine(soundResource->GetUrl(), currentLine++);
			_text->addLine(soundResource->GetCurrentState(), currentLine++);
			if (soundResource->IsActive())
			{	
				//Case starving mute channel
				HandleFmodError(soundResource->GetChannel()->setMute(soundResource->IsStarving()), "Mute starving stream");
				//Print out additional information
				soundResource->CheckTags();
				_text->addLine(soundResource->GetTags(), currentLine++);
				_text->addLine(soundResource->GetCurrentBuffering(), currentLine++);
			}
			//Play Resource
			if (!soundResource->channel)
			{
				_soundManager->PlayStreamSound(soundResource->GetIndex());
			}
			else
			{
				soundResource->SetState(eSoundResourceState::PLAYING);
			}
		
		}

		// int to store dsp count for each active group channel
		int dspount;
		//get the num of dsp
		_soundManager->_activeChannelGroup->getNumDSPs(&dspount);

		//loop through the dsps inside the active group channel and display information
		for (int x = 0; x < dspount-1; x++) {
			info.str("");
			//variables to store data inside them
			char name[BUFFER_SIZE] = "";
			FMOD::DSP* tempDSP;
			FMOD_DSP_TYPE type;
			unsigned int udiscard;
			int discard;
			bool bypass, active;

			//assign the values to the variables
			_soundManager->_activeChannelGroup->getDSP(x,&tempDSP);
			tempDSP->getInfo(name, &udiscard, &discard, &discard, &discard);
			tempDSP->getBypass(&bypass);
			tempDSP->getActive(&active);
			//add the data to string stream and display it
			info << name;
			info << "  -- Bypass: " << (bypass?" ON":" OFF") << " active: " << (active ? " Yes" : " No");
			_text->addLine(info.str(), currentLine++);
		}

		//is paused process
		bool ispaused;
		info.str("");
		_soundManager->_activeChannelGroup->getPaused(&ispaused);
		info << "is Paused : " << (ispaused ? " Paused" : " unPaused");
		_text->addLine(info.str(), currentLine++);
		
		//volume process
		info.str("");
		float vol;
		_soundManager->_activeChannelGroup->getVolume(&vol);
		info<<"Volume : " << vol;
		_text->addLine(info.str(), currentLine++);

		
		_text->addLine("", 21);
		_text->addLine("Pause[Space] Bypass DSPs[1, 2, 3] BypassAll[P] Volume[+-] Cycle Channels[tab] Exit[esc]", 22);
		

		//----------------------------------

		lines.clear();

		_text->render();

		glfwSwapBuffers(_window);
		glfwPollEvents();
	}

	shutdown();

	return 0;
}

//functions


bool init() {

	if (!GLFWinit()) {
		return false;
	}
	if (!initGL()) {
		return false;
	}
	if (!initFMOD()) {
		return false;
	}

	//GLText can only be init after openGl has been initilized
	_text = new RSS::GLText(_appName, _windowWidth, _windowHeight, MAX_TEXT_LINES);


	return true;
}

FMOD_RESULT F_CALLBACK FMODLogCallback(FMOD_DEBUG_FLAGS flags, const char* file, int line, const char* func, const char* message)
{
	if (flags & FMOD_DEBUG_LEVEL_ERROR)
	{
		fprintf(stderr, "Debug: %s\n", message);
		
	}
	else if (flags & FMOD_DEBUG_LEVEL_WARNING)
	{
		fprintf(stderr, "Warning: %s\n", message);
	}
	else
	{
		fprintf(stderr, "Error: %s\n", message);
	}
	return FMOD_OK;
}

bool initFMOD() {

	FMOD::Debug_Initialize(FMOD_DEBUG_LEVEL_WARNING | FMOD_DEBUG_TYPE_TRACE, FMOD_DEBUG_MODE_CALLBACK, FMODLogCallback);

	//Init Fmod
	if(HandleFmodError(FMOD::System_Create(&_system), "create FMOD SYSTEM")) return false;
	if(HandleFmodError(_system->init(2048, FMOD_INIT_NORMAL, NULL), "init FMOD System")) return false;
	if(HandleFmodError(_system->setStreamBufferSize(64 * 1024, FMOD_TIMEUNIT_RAWBYTES), "set a proper buffer size for streaming")) return false;
	
	//Init Sound Manager
	_soundManager = new cSoundManager(_system);
	
	return true;
}

bool GLFWinit() {
	//setting error callback 
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		fprintf(stderr, "unable to init GLFW");
		return false;
	}
	//requiring min GLFW version of 2.0 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// initilizing the window
	_window = glfwCreateWindow(_windowWidth, _windowHeight, _appName.c_str(), NULL, NULL);

	if (!_window) {
		//Oops something went Wrong
		//call glfwterminate then return false so init can exit 
		fprintf(stderr, "unable to init GLFW _window");
		glfwTerminate();
		return false;
	}

	//The key callback, like other window related callbacks, are set per-window.
	glfwSetKeyCallback(_window, key_callback);

	//Before you can use the OpenGL API, you must have a current OpenGL context
	//will remain until set context is destroyed or set again using the same command
	glfwMakeContextCurrent(_window);

	//lock buffer swap interval to 1
	//interval : min number of screen updates to wait until the buffer are swapped 
	glfwSwapInterval(1);


	return true;
}

bool initGL() {
	//init glad

	if (!gladLoadGL(glfwGetProcAddress)) {
		fprintf(stderr, "unable to init GLAD");
		return false;
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


	return true;
}

void shutdown() {

	glfwTerminate();


	FMOD_OPENSTATE openState;

	//cleanup each remote stream
	for (int i = 0; i < _soundManager->stream_Sounds.size(); i++) {
		HandleFmodError(_soundManager->stream_Sounds[i]->GetSound()->getOpenState(&openState, 0, 0, 0), "retrieve the open state");
		while (openState != FMOD_OPENSTATE_READY && openState != FMOD_OPENSTATE_PLAYING)
		{
			HandleFmodError(_system->update(), "update system");
			HandleFmodError(_soundManager->stream_Sounds[i]->GetSound()->getOpenState(&openState, 0, 0, 0), "retrieve the open state");
		}

		HandleFmodError(_soundManager->stream_Sounds[i]->GetSound()->release(), "release stream sound");
		
		HandleFmodError(_soundManager->stream_Sounds[i]->channel->stop(), "Release Channel");

	}//for loop

	for (int i = 0; i < _soundManager->groups.size(); i++) {
		if (_soundManager->groups[i])
		{
			HandleFmodError(_soundManager->groups[i]->release(), "release Sound group");
		}
	}//for loop

	//releaseing master group
	_soundManager->_masterChannelGroup->release();


	//Release system
	HandleFmodError(_system->close(),"Close System");
	HandleFmodError(_system->release(), "Release System");

	//Clean pointers
	if (_text) {
		delete _text;
		_text = nullptr;
	}

	if (_soundManager) {
		delete _soundManager;
	}



	exit(EXIT_SUCCESS);
}

void clearLines()
{
	for (int i = 0; i < MAX_TEXT_LINES; i++)
	{
		_text->addLine("", i);
	}
}