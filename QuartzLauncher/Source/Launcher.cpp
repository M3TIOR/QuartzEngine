
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
#include <Quartz/Voxels/Blocks.hpp>
#include <Quartz/Voxels/Terrain.hpp>

using namespace launcher;
using namespace qz;

Launcher::Launcher()
{
	m_window = gfx::IWindow::create("Quartz Launcher", 1280, 720, 0,
	                                gfx::RenderingAPI::OPENGL);

	utils::Logger::instance()->initialise("Launcher.log",
	                                      utils::LogVerbosity::DEBUG);
}

static void showHintUi()
{
	const float DISTANCE = 10.0f;
	const int   corner   = 1;
	static bool p_open   = true;

	ImGuiIO& io = ImGui::GetIO();

	ImVec2 window_pos =
	    ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE,
	           (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
	ImVec2 window_pos_pivot =
	    ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

	ImGui::SetNextWindowBgAlpha(0.3f);

	if (ImGui::Begin(
	        "Debug Overlay Hint", &p_open,
	        (corner != -1 ? ImGuiWindowFlags_NoMove : 0) |
	            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
	            ImGuiWindowFlags_AlwaysAutoResize |
	            ImGuiWindowFlags_NoSavedSettings |
	            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Text("info box here");
	}

	ImGui::End();
}

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

#undef main
int main(int argc, char** argv)
{
	Launcher launcher;
	launcher.run();

	return 0;
}
