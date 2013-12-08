#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>

#include "GLee.h"
#include <GL/glu.h>
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "Simple example", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

	if (!GLEE_VERSION_1_2) {
		cout << "DO SOMETHING!" << endl;
	}

    // aa
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    std::vector<vec3d> vertices;
    std::vector<vec3d> uvs;
    std::vector<vec3d> normals;
    unsigned int nTriangles = 0;
    loadOBJ("res/models/cube.obj", vertices, uvs, normals, nTriangles);
    for(unsigned int i = 0; i < vertices.size(); i++) {
        std::cout << "Vert:" << vertices[i] << endl;
    }
    printf("# Vertices: %ld\n", vertices.size());

    //for(unsigned int i = 0; i < uvs.size(); i++) {
    //  printf("UV: %f %f\n", uvs[i].x, uvs[i].y);
    //}
    //printf("# uvs: %ld\n", uvs.size());

    // GLuint Tex = loadBMP("res/textures/tex.bmp");

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    struct simplevert {
        float x, y, z;
    };

    simplevert newverts[vertices.size()];
    for(int i = 0; i < vertices.size(); i++) {

        newverts[i].x = vertices[i].x();
        newverts[i].y = vertices[i].y();
        newverts[i].z = vertices[i].z();

        // if(nearzero(newverts[i])) newverts[i] = 0;
        // if(nearzero(newverts[i+1])) newverts[i+1] = 0;
        // if(nearzero(newverts[i+2])) newverts[i+2] = 0;
    }

    for(int i = 0; i < vertices.size(); i++)
        cout << i << ":" << newverts[i].x << " " << newverts[i].y << " " << newverts[i].z << endl;

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(simplevert), &newverts[0], GL_STATIC_DRAW);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec3d), &uvs[0], GL_STATIC_DRAW);

    GLuint programID = fgLoadShader("res/shaders/SimpleVertexShader.vert", "res/shaders/SimpleVertexShader.frag");

    vec3d pos = vec3d(20, 20, 20);
    vec3d look = vec3d(0, 0, 0);
    vec3d up = vec3d(0, 1, 0);

    mat4d Projection =  createPerspectiveFOV(45, 4.0f / 3.0f, 0.1f, 10000.0f);
    mat4d View = createLookAt(pos, look, up);
    mat4d Model = mat4d(3.0f);
    mat4<float> MVP = Projection * View * Model;

    cout << "Pos: " << pos << endl << "Look: " << look << endl << "Up: " << up << endl;
    cout << "Projection: " << endl << Projection << endl;
    cout << "View: " << endl << View << endl;
    cout << "Model: " << endl << Projection << endl;
    cout << "MVP:" << endl << MVP << endl;

        // cout << "View: " << endl << View << endl;
    

    double lastFPSTime = glfwGetTime();
    int fps = 0;

    // Vector3 vec();
    
    do {
        glfwPollEvents();

        double now = glfwGetTime();
        if (now - lastFPSTime > 1) {
            printf("FPS: %d\n", fps);
            fps = 0;
            lastFPSTime = glfwGetTime();
        }
        fps++;


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLuint MatrixID = glGetUniformLocation(programID, "MVP");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP(0, 0));
        glUseProgram(programID);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)0
        );

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)0
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
    } while(!glfwWindowShouldClose(window));

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

