
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <tuple>
#include <iomanip>
#include <thread>
#include <random>

// ambition includes should be done like this, using <>
#include <ambition/Initial3D.hpp>
#include <ambition/Ambition.hpp>
#include <ambition/Log.hpp>
#include <ambition/Config.hpp>
#include <ambition/Concurrent.hpp>
#include <ambition/Window.hpp>
#include <ambition/Shader.hpp>
#include <ambition/SceneGraph.hpp>
#include <ambition/ByteBuffer.hpp>
#include <ambition/GPUCache.hpp>
#include <ambition/TerrainManager.hpp>
#include <ambition/Chrono.hpp>


#include "loadOBJ.hpp"
#include "loadBitmap.hpp"
#include "loadShader.hpp"


using namespace std;
using namespace ambition;
using namespace scenegraph;



ShaderManager *shaderman = nullptr;

// framebuffers and textures for scene rendering, including shadows
size2i size_fbo_scene(1, 1);
GLuint fbo_scene = 0;
GLuint tex_scene_depth = 0;
GLuint tex_scene_z = 0;
GLuint tex_scene_normal = 0;
GLuint tex_scene_diffuse = 0;
GLuint tex_scene_l0 = 0;

// exposure for hdr
float exposure = 26.0f;

SceneGraph *scene;
Controller *cameraController;
Camera *cam;

unsigned draw_call_count = 0;

bool sun_moving = false;
double sun_speed = math::pi() / 600.0; // == 10 minutes daytime
vec3d sun_radiance = vec3d(25e3, 25e3, 25e3);

quatd sun_ori = quatd::axisangle(vec3d::i(), math::pi() / 2);

double dither = 127.0;


// block on user input until all currently loaded shaders have been successfully reloaded
void reload_shaders() {
	cout << endl << "Reloading shader programs..." << endl << endl;
	vector<string> prog_names = shaderman->getLoadedProgramNames();
	while (true) {
		try {
			shaderman->unloadAll();
			for (const string &name : prog_names) {
				shaderman->getProgram(name);
			}
			break;
		} catch (shader_error &e) {
			cout << endl << "One or more shader programs failed to load." << endl;
			cout << "Press ENTER to retry." << endl << endl;
			cin.get();
		}
	}
	cout << endl << "Shader programs reloaded." << endl << endl;
}

void initSceneFB(size2i size) {
	if (size.w <= 0) size.w = 1;
	if (size.h <= 0) size.h = 1;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkGL();
	
	// make new scene framebuffer
	if (fbo_scene) glDeleteFramebuffers(1, &fbo_scene);
	glGenFramebuffers(1, &fbo_scene);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_scene);
	
	// delete existing textures
	if (tex_scene_depth) glDeleteTextures(1, &tex_scene_depth);
	if (tex_scene_z) glDeleteTextures(1, &tex_scene_z);
	if (tex_scene_normal) glDeleteTextures(1, &tex_scene_normal);
	if (tex_scene_diffuse) glDeleteTextures(1, &tex_scene_diffuse);
	if (tex_scene_l0) glDeleteTextures(1, &tex_scene_l0);
	
	// create new texture handles
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex_scene_depth);
	glGenTextures(1, &tex_scene_z);
	glGenTextures(1, &tex_scene_normal);
	glGenTextures(1, &tex_scene_diffuse);
	glGenTextures(1, &tex_scene_l0);
	
	// depth
	glBindTexture(GL_TEXTURE_2D, tex_scene_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.w, size.h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex_scene_depth, 0);
	checkGL();
	
	// note: using RGB16F crashed on a uni machine with mesa, was fine with Quadro 600
	
	// z - because depth buffers are gay we need a second buffer for view-space depth
	glBindTexture(GL_TEXTURE_2D, tex_scene_z);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, size.w, size.h, 0, GL_RGBA, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_scene_z, 0);
	checkGL();
	
	// normal
	glBindTexture(GL_TEXTURE_2D, tex_scene_normal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16_SNORM, size.w, size.h, 0, GL_RGBA, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_scene_normal, 0);
	checkGL();
	
	// diffuse
	glBindTexture(GL_TEXTURE_2D, tex_scene_diffuse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16_SNORM, size.w, size.h, 0, GL_RGBA, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tex_scene_diffuse, 0);
	checkGL();
	
	// emissivity (radiance)
	glBindTexture(GL_TEXTURE_2D, tex_scene_l0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.w, size.h, 0, GL_RGBA, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tex_scene_l0, 0);
	checkGL();
	
	// set up draw buffers and check fbo
	GLenum bufs_scene[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, bufs_scene);
	checkFB();
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	size_fbo_scene = size;
}

void draw_fullscreen() {
	static GLuint vao = 0;
	if (vao == 0) {
		glGenVertexArrays(1, &vao);
	}
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindVertexArray(0);
}

void draw_stars_points() {
	static const int num_stars = 100000;
	static GLuint vao = 0;
	if (vao == 0) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		GLuint vbo_pos;
		glGenBuffers(1, &vbo_pos);
		
		std::default_random_engine re;
		std::uniform_real_distribution<float> rdn(-1, 1);
		std::uniform_real_distribution<float> rdb(0, 1);
		std::vector<float> points;

		for (int i = 0; i < num_stars; i++) {
			vec3f n = ~(vec3f(rdn(re), rdn(re), rdn(re)));
			points.push_back(n.x());
			points.push_back(n.y());
			points.push_back(n.z());
			// TODO realistic (ir)radiance values, probs needs auto exposure
			points.push_back(0.04f * math::pow(rdb(re), 1.0f * math::log2(float(i))) + 0.001);
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
		glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), &points[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, num_stars);
	glBindVertexArray(0);
}

void display(int w, int h) {
	if (w == 0 || h == 0) return;
	
	// the far plane
	float zfar = 20000000.0f;
	
	cameraController->update();
	cam->setPerspectiveProjection(math::pi() / 3, double(w) / h, 0.1, zfar);
	
	mat4d proj_matrix = cam->getProjectionTransform();
	// TODO properly get planet -> view matrix
	mat4d view_matrix = !cameraController->getTransform();
	
	//
	// assemble scene buffer
	//
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_scene);
	glViewport(0, 0, size_fbo_scene.w, size_fbo_scene.h);
	checkFB();
	checkGL();
	
	// begin scene background
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	// draw a black background, this runs instead of glClear()
	// TODO skybox instead ???
	GLuint prog_background = shaderman->getProgram("scene_background.glsl");
	glUseProgram(prog_background);
	glUniform1f(glGetUniformLocation(prog_background, "zfar"), zfar);
	draw_fullscreen();
	
	
	// stars ?!
	if (true) {
		glDisable(GL_DEPTH_TEST);
		GLuint prog_stars = shaderman->getProgram("scene_stars.glsl");
		glUseProgram(prog_stars);
		glUniform1f(glGetUniformLocation(prog_stars, "zfar"), zfar);
		// fix the stars to the sun for now
		glUniformMatrix4fv(glGetUniformLocation(prog_stars, "univViewMatrix"), 1, true, mat4f(view_matrix * mat4d::rotate(sun_ori)));
		glUniformMatrix4fv(glGetUniformLocation(prog_stars, "projectionMatrix"), 1, true, mat4f(proj_matrix));
		// blending is only valid because this shader only writes emission values
		// blending is to prevent flicker when one star occludes another
		glEnable(GL_DEPTH_CLAMP);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		draw_stars_points();
		glDisable(GL_DEPTH_CLAMP);
		glDisable(GL_BLEND);
	}
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	checkGL();

	//traverse the scenegraph for geometry
	SceneRenderer sr;
	sr.traverse(scene);
	draw_call_count = sr.getDrawQueue().getDrawCalls().size();

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_CULL_FACE);
	
	if (true) {
		sr.getDrawQueue().execute(shaderman);
	}

	glDisable(GL_CULL_FACE);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_DEPTH_TEST);
	
	glFinish();
	checkGL();

	// bind textures for deferred shading etc
	enum { tu_z = 0, tu_normal, tu_diffuse, tu_l0 };
	glActiveTexture(GL_TEXTURE0 + tu_z);
	glBindTexture(GL_TEXTURE_2D, tex_scene_z);
	glActiveTexture(GL_TEXTURE0 + tu_normal);
	glBindTexture(GL_TEXTURE_2D, tex_scene_normal);
	glActiveTexture(GL_TEXTURE0 + tu_diffuse);
	glBindTexture(GL_TEXTURE_2D, tex_scene_diffuse);
	glActiveTexture(GL_TEXTURE0 + tu_l0);
	glBindTexture(GL_TEXTURE_2D, tex_scene_l0);

	//
	// deferred shading
	//

	// main deferred shading pass
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glViewport(0, 0, w, h);
	glDisable(GL_DEPTH_TEST);
	checkFB();
	checkGL();
	
	// run shader
	GLuint prog_def1 = shaderman->getProgram("deferred1.glsl");
	glUseProgram(prog_def1);
	glUniform1f(glGetUniformLocation(prog_def1, "dither"), dither);
	glUniform1f(glGetUniformLocation(prog_def1, "noise_time"), fmod(glfwGetTime(), 1.0));
	glUniform3fv(glGetUniformLocation(prog_def1, "light_l"), 1, vec3f(sun_radiance));
	glUniform1i(glGetUniformLocation(prog_def1, "sampler_z"), tu_z);
	glUniform1i(glGetUniformLocation(prog_def1, "sampler_normal"), tu_normal);
	glUniform1i(glGetUniformLocation(prog_def1, "sampler_diffuse"), tu_diffuse);
	glUniform1i(glGetUniformLocation(prog_def1, "sampler_l0"), tu_l0);
	glUniform1f(glGetUniformLocation(prog_def1, "zfar"), zfar);
	glUniform1f(glGetUniformLocation(prog_def1, "exposure"), exposure);
	glUniform3fv(glGetUniformLocation(prog_def1, "light_norm_v"), 1, (view_matrix * vec4d(sun_ori * vec3d::k(-1), 0)).xyz<float>());
	glUniform3fv(glGetUniformLocation(prog_def1, "planet_pos_v"), 1, (view_matrix * vec4d(vec3d(0, 0, 0), 1)).xyz<float>());
	glUniformMatrix4fv(glGetUniformLocation(prog_def1, "inv_proj_matrix"), 1, true, mat4f(!proj_matrix));
	draw_fullscreen();
	glFinish();

	// unbind textures
	glActiveTexture(GL_TEXTURE0 + tu_z);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + tu_normal);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + tu_diffuse);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + tu_l0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glFinish();
	checkGL();
	
	
	//
	// post-processing
	//
	
	glFinish();
	checkGL();
	
	// cleanup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
	glFinish();
	checkGL();

	
}

template <typename ClockT>
void testClock(const string &name) {
	auto time0 = ClockT::now();
	auto time1 = time0;
	while (time0 == time1) {
		time1 = ClockT::now();
	}
	auto time2 = time1;
	int calls = 0;
	while (time1 == time2) {
		time2 = ClockT::now();
		calls++;
	}

	log(name) << "Interval: " << chrono::duration_cast<chrono::duration<double>>(time2 - time1).count();
	log(name) << "Calls: " << calls;
}


int main(void) {
	log("System") % 0 << "Starting...";

	AsyncExecutor::start();

	testClock<chrono::steady_clock>("SteadyClock");
	testClock<chrono::high_resolution_clock>("HRClock");
	testClock<really_high_resolution_clock>("RHRClock");
	
	shaderman = new ShaderManager("./res/shader");

	Window *window = createWindow().size(1024, 768).title("Golden Eagle").visible(true);
	window->makeContextCurrent();

	Window *window2 = createWindow().share(window).title("LALALALALALALALALA");

	// give ownership of the second context to AsyncExecutor's 'fast' thread
	AsyncExecutor::enqueueFast([=] {
		window2->makeContextCurrent();
		// window2->visible(true); // but not really
	});

	// test background GL
	AsyncExecutor::enqueueFast([=] {

		// just check this works
		assert(Window::currentContext() == window2);

		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_FLOAT, nullptr);
		glDeleteTextures(1, &tex);
	});

	
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	initSceneFB(window->size());
	
	window->onResize.attach([](const window_size_event &e) {
		cout << "Resize: " << e.size.w << "x" << e.size.h << endl;
		if (e.size.w > 0 && e.size.h > 0) {
			// on windows, we seem to get a 0x0 resize event on minimise
			initSceneFB(e.size);
		}
		return false;
	});
	
	window->onKeyPress.attach([](const key_event &e) {
		if (e.key == GLFW_KEY_F5) reload_shaders();
		if (e.key == GLFW_KEY_R) sun_ori = quatd::axisangle(vec3d::i(), math::pi() / 180) * sun_ori;
		if (e.key == GLFW_KEY_F) sun_ori = quatd::axisangle(vec3d::i(), -math::pi() / 180) * sun_ori;
		if (e.key == GLFW_KEY_EQUAL) {
			exposure *= 1.2;
			cout << "exposure " << exposure << endl;
		}
		if (e.key == GLFW_KEY_MINUS) {
			exposure /= 1.2;
			cout << "exposure " << exposure << endl;
		}
		if (e.key == GLFW_KEY_P) {
			sun_moving = !sun_moving;
		}
		return false;
	});

	//SCENGRAPH START
	scene = new SceneGraph();

	SceneNode *root = new SceneNode(NullCore::create());
	scene->setRootNode(root);


	//CREATE AND SET CAMERA

	// vec3d pos(0, 0, 15);
	vec3d pos(0, 6361000, 0);

	cameraController = new OribitalFPSCamera(window, pos, 0, 0);

	//cameraController = new FPSCamera(window, pos, 0, 0);
	SceneNode *camN = new SceneNode(ControllerTransformCore::create(cameraController));
	root->addChild(camN);

	cam = new Camera();
	scene->setCamera(cam);
	cam->setCameraNode(camN);


	//CREATE PLANET

	GPUCacheManager::setMaxMemory(134217728);
	Planet *p = Planet::create(new PlanetPerlinTerrainGen(6360000, 1000, 32, 0.2, 70000));
	// Planet* p = Planet::create(new FlatTerrainGen(6360000, 16.0, 1.0, 14000));
	// Planet* p = Planet::create(new FlatTerrainGen(5.0, 16.0, 0.1, 14000));
	root->addChild(p->planetRoot());


	double lastTime = glfwGetTime();
	double lastFPSTime = glfwGetTime();
	int fps = 0;

	do {
		double now = glfwGetTime();
		glfwPollEvents();
		
		AsyncExecutor::execute(chrono::milliseconds(2));

		if (sun_moving) {
			sun_ori = quatd::axisangle(vec3d::i(), sun_speed * (now - lastTime)) * sun_ori;
		}

		display(window->width(), window->height());
		
		glFinish();
		window->swapBuffers();
		
		if (now - lastFPSTime > 1) {
			char fpsString[200];
			sprintf(fpsString, "Ambition [%d FPS @%dx%d, %d draw calls, %ld/%ld MB (%.1lf%%) GPU cache]",
				fps, window->width(), window->height(), draw_call_count,
				GPUCacheManager::getCurrentMemory() / 1024 / 1024,
				GPUCacheManager::getMaxMemory() / 1024 / 1024,
				100.0 * double(GPUCacheManager::getCurrentMemory()) / double(GPUCacheManager::getMaxMemory()));
			window->title(fpsString);
			fps = 0;
			lastFPSTime = now;
		}
		fps++;
		
		lastTime = now;
	} while (!window->shouldClose());

	delete window;

	log("System") % 0 << "Exiting normally";
	AsyncExecutor::stop();

	delete window2;

	glfwTerminate();

	std::exit(EXIT_SUCCESS);
}
