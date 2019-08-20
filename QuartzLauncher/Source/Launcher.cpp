
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <Quartz.hpp>
#include <Launcher/Launcher.hpp>

#include <Quartz/Graphics/RHI/IRenderDevice.hpp>
#include <Quartz/Graphics/RHI/OpenGL/GLRenderDevice.hpp>
#include <imgui/imgui.h>

#include <Quartz/Graphics/Camera.hpp>
#include <Quartz/Graphics/ForwardMeshRenderer.hpp>
#include <Quartz/Graphics/ImGuiExtensions.hpp>

#if defined(QZ_PLATFORM_WINDOWS)
	#include <windows.h>
#elif defined(QZ_PLATFORM_LINUX)
	#include <stdlib.h>
	#include <unistd.h>
	#include <spawn.h>
	#include <sys/wait.h>
#else
	#error "Couldn't identify platform"
#endif

#define QZ_MAKE_UX_ELEM_NO_WINDOW__PRIVATE(name, corner, pos_macro, bg_alpha, content_macro) \
	static void show##name##Ui() \
	{ \
		static bool p_open = true; \
		pos_macro \
		ImGui::SetNextWindowBgAlpha(bg_alpha); \
		if (ImGui::Begin("Debug Overlay Hint", &p_open, \
				(corner != -1 ? ImGuiWindowFlags_NoMove : 0) | \
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | \
				ImGuiWindowFlags_AlwaysAutoResize | \
				ImGuiWindowFlags_NoSavedSettings | \
				ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) \
		{ \
			content_macro \
		} \
		ImGui::End(); \
	}
#define QZ_MAKE_UX_POS(x, y) \
	ImVec2 window_pos(x, y); \
	ImVec2 window_pos_pivot(0.0f, 0.0f); \
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
#define QZ_MAKE_UX_CORNER_POS(corner, dist) \
	ImGuiIO& io = ImGui::GetIO(); \
	ImVec2 window_pos = \
	    ImVec2((corner & 1) ? io.DisplaySize.x - dist : dist, \
	           (corner & 2) ? io.DisplaySize.y - dist : dist); \
	ImVec2 window_pos_pivot = \
	    ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f); \
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
#define QZ_MAKE_UX_ELEM_NO_WINDOW_DEFAULT(name, x, y, content) \
	QZ_MAKE_UX_ELEM_NO_WINDOW__PRIVATE(name, 0, QZ_MAKE_UX_POS(x, y), 0.0f, content)
#define QZ_MAKE_UX_ELEM_CORNERED_BG_ALPHA(name, corner, bg_alpha, content) \
	QZ_MAKE_UX_ELEM_NO_WINDOW__PRIVATE(name, corner, QZ_MAKE_UX_CORNER_POS(corner, 10.0f), bg_alpha, content)

using namespace launcher;
using namespace qz;

Launcher::Launcher()
{
	m_window = gfx::IWindow::create("Quartz Launcher", 1280, 720, 0,
	                                gfx::RenderingAPI::OPENGL);

	utils::Logger::instance()->initialise("Launcher.log",
	                                      utils::LogVerbosity::DEBUG);
	spawnProcess("/bin/ls", {"-l", "-a", "-s"});
}

QZ_MAKE_UX_ELEM_CORNERED_BG_ALPHA(Hint, 1, 0.3f, ImGui::Text("info box here");)

void Launcher::run()
{
	using namespace gfx::rhi::gl;
	using namespace gfx::rhi;

	m_window->setVSync(false);
	m_window->registerEventListener(this);

	m_renderDevice = new GLRenderDevice();
	m_renderDevice->create();

	gfx::ForwardMeshRenderer renderer(m_renderDevice);
	renderer.create();

	m_camera = new gfx::FPSCamera(m_window);
	m_camera->enable(false);

	float     fov               = 90.f;
	Matrix4x4 perspectiveMatrix = Matrix4x4::perspective(
	    static_cast<float>(1280) / 720, fov, 100.f, 0.1f);
	m_camera->setProjection(perspectiveMatrix);
	renderer.setProjectionMatrix(m_camera->getProjection());

	std::size_t fpsLastTime = SDL_GetTicks();
	int         fpsCurrent  = 0;
	int         fpsFrames   = 0;

	float last         = static_cast<float>(SDL_GetTicks());
	int   t            = 0;
	int   dtSampleRate = 60;
	bool  pauseDt      = false;
	bool  vsync        = false;
	bool  fullscreen   = false;
	while (m_window->isRunning())
	{
		fpsFrames++;
		if (fpsLastTime < SDL_GetTicks() - 1000)
		{
			fpsLastTime = SDL_GetTicks();
			fpsCurrent  = fpsFrames;
			fpsFrames   = 0;
		}

		const float now = static_cast<float>(SDL_GetTicks());
		const float dt  = now - last;
		last            = now;

		m_window->startFrame();

		m_debug.show();

		static bool wireframe = false;

		ImGui::Begin("Stats");
		ImGui::Checkbox("Wireframe", &wireframe);
		ImGui::Text("FPS: %i frame/s", fpsCurrent);
		ImGui::Text("Frame Time: %.2f ms/frame", static_cast<double>(dt));
		ImGui::Text("Vertices: %i", renderer.countTotalNumVertices());
		ImGui::SliderInt("Frame Time Sample Rate", &dtSampleRate, 1, 60);
		ImGui::Checkbox("Pause Frame Time", &pauseDt);

		if (t % dtSampleRate == 0 && !pauseDt)
		{
			ImGui::PlotVariable("Frame Time: ", dt);
		}
		else
		{
			ImGui::PlotVariable("Frame Time: ", FLT_MAX);
		}

		if (ImGui::Checkbox("VSync", &vsync))
		{
			m_window->setVSync(vsync);
		}

		if (ImGui::Checkbox("Fullscreen", &fullscreen))
		{
			m_window->setFullscreen(fullscreen);
		}

		if (ImGui::SliderFloat("FoV", &fov, 0.f, 180.f))
		{
			perspectiveMatrix = Matrix4x4::perspective(
				static_cast<float>(1280) / 720, fov, 100.f, 0.1f);
			renderer.setProjectionMatrix(perspectiveMatrix);
		}

		Vector3 cameraPosition = m_camera->getPosition();
		ImGui::Text("Camera Position %.2f, %.2f, %.2f", cameraPosition.x,
					cameraPosition.y, cameraPosition.z);

		ImGui::End();

		m_renderDevice->showShaderDebugUI();

		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		m_camera->tick(dt);

		renderer.setViewMatrix(m_camera->calculateViewMatrix());

		showHintUi();

		renderer.render();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		m_window->endFrame();

		if (t > 3600)
		{
			t = 0;
		}

		t++;
	}

	renderer.destroy();
}

void Launcher::onEvent(const events::Event& e)
{
	if (e.type == events::EventType::KEY_PRESSED)
	{
		// #todo
	}
	else if (e.type == events::EventType::WINDOW_RESIZED)
	{
		m_camera->resizeProjection(e);
	}
}

void Launcher::spawnProcess(const char* fullpath, std::initializer_list<const char*> args)
{
#if defined(QZ_PLATFORM_WINDOWS)
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;

	// set the size of the structures
	ZeroMemory(&siStartupInfo, sizeof(siStartupInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);
	ZeroMemory(&piProcessInfo, sizeof(piProcessInfo));

	std::string str_cmd(fullpath);

	size_t length = strlen(fullpath) + 1;
	for (const char* arg : args)
	{
		length += strlen(arg) + 1;
		str_cmd += " ";
		str_cmd += arg;
	}
	wchar_t* command = new wchar_t[length];
	mbstowcs(command, str_cmd.c_str(), length);

	if (CreateProcessW(NULL, (LPWSTR) command,
		0, 0, false, CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW | DETACHED_PROCESS,
		0, 0, &siStartupInfo, &piProcessInfo) == false)
	{
		LFATAL("Couldn't build process, error code: ", GetLastError());
	}
	delete[] command;
#elif defined(QZ_PLATFORM_LINUX)
	pid_t pid;
	char** argv = new char*[2 + args.size()];
	argv[0] = strdup(fullpath);
	std::size_t i = 0;
	for (const char* elem : args)
	{
		argv[1 + i] = strdup(elem);
		i++;
	}
	argv[1 + args.size()] = (char *) 0;

	int status = posix_spawn(&pid, fullpath, NULL, NULL, argv, environ);
	if (status != 0)
	{
		LFATAL("Failed posix_spawn: ", strerror(status));
	}

	for (int i=0; i < 2 + args.size(); ++i)
		delete[] argv[i];
	delete[] argv;
#endif  // QZ_PLATFORM
}

#undef main
int main(int argc, char** argv)
{
	Launcher launcher;
	launcher.run();

	return 0;
}
