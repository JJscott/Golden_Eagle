
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>

#include <iomanip>

#include <thread>
#include <chrono>


// ambition includes should be done like this, using <>
#include <ambition/Initial3D.hpp>
#include <ambition/Ambition.hpp>
#include <ambition/Log.hpp>
#include <ambition/Config.hpp>
#include <ambition/Concurrent.hpp>
#include <ambition/Window.hpp>
#include <ambition/Shader.hpp>
#include <ambition/SceneGraph.hpp>

#include "loadOBJ.hpp"
#include "loadBitmap.hpp"
#include "loadShader.hpp"

#include <ambition/ByteBuffer.hpp>

using namespace std;
using namespace ambition;

int main(void) {
	log("System") % 0 << "Starting...";

	AsyncExecutor::start();
	
	log().error() % 2 << "minor error";
	log().error() << "proper error";

	log().warning() << "minor warning";
	log().warning() % 0 << "proper warning";

	log() << std::endl;

	byte_buffer bb;
	bb << 0xCAFEBABA << "hello world" << -42 << "goodbyte world" << 0.0 << 0.0f << -9000.1 << -9001LL << "done";

	auto r = bb.read();

	log("bbtest") << std::hex << r.get<unsigned>();
	log("bbtest") << r.get<string>();
	log("bbtest") << r.get<int>();
	log("bbtest") << r.get<string>();
	log("bbtest") << r.get<double>();
	log("bbtest") << r.get<float>();
	log("bbtest") << r.get<double>();
	log("bbtest") << r.get<long long>();
	log("bbtest") << r.get<string>();

	{
		Event<int> event;

		thread a_thread([&] {
			log("test") << "waiting";
			try {
				while (!event.wait()) {
					log("test") << "wakey-wakey";
				}
				log("test") << "completed - shouldnt happen";
			} catch (interruption &e) {
				log("test") << "interrupted";
			}
		});

		a_thread.detach();

		this_thread::sleep_for(chrono::milliseconds(1000));
	}

	ShaderManager *shaderman = new ShaderManager("./res/shader");

	Window *window = createWindow().size(1024, 768).title("Golden Eagle").visible(true);
	window->makeContextCurrent();

	Window *window2 = createWindow().share(window).title("LALALALALALALALALA");

	// give ownership of the second context to AsyncExecutor's 'fast' thread
	AsyncExecutor::enqueueFast([=] {
		window2->makeContextCurrent();
		window2->visible(true); // but not really
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

	window->onScroll.attach([](const mouse_scroll_event &e) {
		cout << e.offset.h << endl;
		return false;
	});

	window->onChar.attach([](const char_event &e) {
		cout << e.codepoint << ": " << char(e.codepoint) << endl;
		return false;
	});

	std::cin.get();

	// aa
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	std::vector<vec3d> vertices;
	std::vector<vec3d> uvs;
	std::vector<vec3d> normals;
	unsigned int nTriangles = 0;

	loadOBJ("res/model/stump/stump.obj", vertices, uvs, normals, nTriangles);
	// for(unsigned int i = 0; i < vertices.size(); i++) {
		// std::cout << "Vert:" << vertices[i] << endl;
	// }
	// printf("# Vertices: %ld\n", vertices.size());

	//GLuint Tex = 
	loadBMP("res/model/stump/diff.bmp");

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);


	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3d), &vertices[0], GL_STATIC_DRAW);

	log() << "there are " << uvs.size() << " uvs";
	GLuint colorBuffer;
	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec3d), &uvs[0], GL_STATIC_DRAW);

	log() << "getting shaders";
	GLuint programID = shaderman->getProgram("test.glsl");
	log() << "got shaders";

	vec3d pos = vec3d(20, 20, 20);
	vec3d look = vec3d(0, 0, 0);
	vec3d up = vec3d(0, 1, 0);

	mat4d Projection =  createPerspectiveFOV(45, 4.0f / 3.0f, 0.1f, 10000.0f);
	mat4d View = createLookAt(pos, look, up);
	mat4d Model = mat4d(3.0f);

	//cout << "Pos: " << pos << endl << "Look: " << look << endl << "Up: " << up << endl;
	//cout << "Projection: " << endl << Projection << endl;
	//cout << "View: " << endl << View << endl;
	//cout << "Model: " << endl << Projection << endl;

	double longitude = 0;
	double elevation = 20;
	double zoom = 20;

	double lastFPSTime = glfwGetTime();
	int fps = 0;
	
	// Vector3 vec();
	
	do {
		glfwPollEvents();

		double now = glfwGetTime();
		if (now - lastFPSTime > 1) {
			char fpsString[100];
			sprintf(fpsString, "FPS: %d", fps);
			window->title(fpsString);
			fps = 0;
			lastFPSTime = glfwGetTime();
		}
		fps++;

		if (window->getKey(GLFW_KEY_A))
			longitude -= 0.05f;
		if (window->getKey(GLFW_KEY_D))
			longitude += 0.05f;

		if (window->getKey(GLFW_KEY_W))
			elevation += 0.1;
		if (window->getKey(GLFW_KEY_S))
			elevation -= 0.1;

		if (window->getKey(GLFW_KEY_E))
			zoom += 1;
		if (window->getKey(GLFW_KEY_Q))
			zoom -= 1;

		if (zoom < 1)
			zoom = 1;
		else if (zoom > 30)
			zoom = 30;

		if (longitude > initial3d::math::pi() * 2)
			longitude -= (initial3d::math::pi() * 2);

		if (elevation > 40)
			elevation = 40;
		if (elevation < 0)
			elevation = 0;

		pos = vec3d(cos(longitude) * zoom, elevation, sin(longitude) * zoom);
		// cout << "Pos: " << pos << endl;
		View = createLookAt(pos, look, up);
		// cout << "View: " << endl << View << endl;
		mat4d MVP = Projection * View * Model;
		// cout << "MVP: " << endl << MVP << endl;

		int width = window->width();
		int height = window->height();
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(MatrixID, 1, true, mat4f(MVP));

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(
			0,
			3,
			GL_DOUBLE,
			GL_FALSE,
			32,
			(void*)0
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glVertexAttribPointer(
			1,
			3,
			GL_DOUBLE,
			GL_FALSE,
			32,
			(void*)0
		);

		glDrawArrays(GL_TRIANGLES, 0, nTriangles*3);
		// glDisableVertexAttribArray(0);
		glFinish();
		window->swapBuffers();

	} while(!window->shouldClose());

	delete window;

	log("System") % 0 << "Exiting normally";
	AsyncExecutor::stop();

	delete window2;

	glfwTerminate();

	std::exit(EXIT_SUCCESS);
}
