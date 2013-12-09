#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>

#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Initial3D.hpp"
#include "SceneRoot.hpp"
#include "TriangleEntity.hpp"

#include "common/Log.hpp"
#include "common/Config.hpp"
#include "SceneNode.hpp"
#include "loadOBJ.hpp"
#include "loadBitmap.hpp"
#include "common/Ambition.hpp"
#include "loadShader.hpp"
#include "WindowManager.hpp"
#include "common/Event.hpp"
#include "Window.hpp"

#include "Shader.hpp"

using namespace std;

using namespace ambition;

// static void error_callback(int, const char* description) {
//     log("GLFW") % Log::error << description;
// }
// static void key_callback(GLFWwindow* window, int key, int, int action, int) {
//     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, GL_TRUE);
// }

int main(void) {
	Log::getStandardErr()->setMinLevel(Log::information);
	
	// int foo = 9001;

	// // use some kind of reference type if you need to modify the event arg
	// Event<int *> e;
	
	// unsigned k = e.attach([](int *arg) { log("Event") << *arg; });

	// thread t([&]() {
	// 	log("Thread") << "Begin...";
	// 	this_thread::sleep_for(chrono::milliseconds(3000));
	// 	e.notify(&foo);
	// });

	// // wait in a manner that suppresses unwanted wakeup
	// while (!e.wait()) {
	// 	cout << "spurious" << endl;
	// }

	// e.detach(k);
	// e.notify(&foo);

	ShaderManager *shaderman = new ShaderManager("./res/shaders");

	log("System") % Log::idgaf << "Starting...";
	
	TriangleEntity myEntity;

    rootScene.addChildNode(&myEntity);

    wm()->init();
    Window *window = wm()->addWindow(1024, 768, "Golden_Eagle");
    wm()->apply();



    glewExperimental = true;
	GLenum glew_err = glewInit();
	log("GLEW") << "Initialisation returned " << glew_err;
	if (glew_err != GLEW_OK) {
		log("GLEW") % Log::nope << "Initialisation failed: " << glewGetErrorString(glew_err) << endl;
		glfwTerminate();
		cin.get();
		std::exit(9001);
	}
	log("GLEW") << "Initialised";

	log("System") << "GL version string: " << glGetString(GL_VERSION);

	// aa
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    std::vector<vec3d> vertices;
    std::vector<vec3d> uvs;
    std::vector<vec3d> normals;
    unsigned int nTriangles = 0;

    loadOBJ("res/models/cube.obj", vertices, uvs, normals, nTriangles);
    // for(unsigned int i = 0; i < vertices.size(); i++) {
        // std::cout << "Vert:" << vertices[i] << endl;
    // }
    // printf("# Vertices: %ld\n", vertices.size());

    GLuint Tex = loadBMP("res/textures/tex.bmp");

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);


    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3d), &vertices[0], GL_STATIC_DRAW);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec3d), &uvs[0], GL_STATIC_DRAW);

    log() << "getting shaders";
    GLuint programID = shaderman->getProgram("SimpleVertexShader.vert;SimpleVertexShader.frag");
    log() << "got shaders";

    vec3d pos = vec3d(20, 20, 20);
    vec3d look = vec3d(0, 0, 0);
    vec3d up = vec3d(0, 1, 0);

    mat4d Projection =  createPerspectiveFOV(45, 4.0f / 3.0f, 0.1f, 10000.0f);
    mat4d View = createLookAt(pos, look, up);
    mat4d Model = mat4d(3.0f);

    cout << "Pos: " << pos << endl << "Look: " << look << endl << "Up: " << up << endl;
    cout << "Projection: " << endl << Projection << endl;
    cout << "View: " << endl << View << endl;
    cout << "Model: " << endl << Projection << endl;

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
            window->setTitle(fpsString);
            fps = 0;
            lastFPSTime = glfwGetTime();
        }
        fps++;

        if (glfwGetKey(window->getHandle(), GLFW_KEY_A))
            longitude -= 0.05f;
        if (glfwGetKey(window->getHandle(), GLFW_KEY_D))
            longitude += 0.05f;

        if (glfwGetKey(window->getHandle(), GLFW_KEY_W))
            elevation += 0.1;
        if (glfwGetKey(window->getHandle(), GLFW_KEY_S))
            elevation -= 0.1;

        if (glfwGetKey(window->getHandle(), GLFW_KEY_E))
            zoom += 1;
        if (glfwGetKey(window->getHandle(), GLFW_KEY_Q))
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

		int width, height;
		glfwGetWindowSize(window->getHandle(), &width, &height);
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

        glDrawArrays(GL_TRIANGLES, 0, 36);
        // glDisableVertexAttribArray(0);


		/*glBegin(GL_POLYGON);
		glVertex3f(-1, -1, 0.5);
		glVertex3f(1, -1, 0.5);
		glVertex3f(1, 1, 0.5);
		glVertex3f(-1, 1, 0.5);
		glEnd();*/

		glFinish();
        glfwSwapBuffers(window->getHandle());
    } while(!glfwWindowShouldClose(window->getHandle()));

    glfwDestroyWindow(window->getHandle());
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}

