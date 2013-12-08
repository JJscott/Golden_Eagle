#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "GLee.h"
#include <GLFW/glfw3.h>

#include "Initial3D.hpp"
#include "SceneRoot.hpp"
#include "TriangleEntity.hpp"

#include "common/Log.hpp"
#include "common/Config.hpp"
#include "SceneNode.hpp"

using namespace std;

using namespace ambition;

static void error_callback(int, const char* description) {
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(void) {

	auto logw = []() { return log("System"); };

	Log::getStandardErr()->setMinLevel(Log::information);
	logw() % Log::idgaf << "Starting...";
	log("Bar") % Log::warning << "There'll be a welcome in the hillside";
	log("Foo") % Log::error << "gaaaaaaaaaarrrrrrrrrrrhhhhhhhhh";
	log("SceneManager") % Log::critical << "GPU is dead";
	logw() % Log::nope << "NONONONONONONONOOoooooooooooooo";

	TriangleEntity myEntity;

    rootScene.addChildNode(&myEntity);

    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

	if (!GLEE_VERSION_1_2) {
		cout << "DO SOMETHING!" << endl;
	}

	glfwSetKeyCallback(window, key_callback);
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
        glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(-0.6f, -0.4f, 0.f);
        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(0.6f, -0.4f, 0.f);
        glColor3f(0.f, 0.f, 1.f);
        glVertex3f(0.f, 0.6f, 0.f);
        glEnd();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

