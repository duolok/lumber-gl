#define _CRT_SECURE_NO_WARNINGS
#define M_PI 3.14159265358979323846
#define ELLIPSE_SEGMENTS 50

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

using namespace std;

// Constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const char* WINDOW_TITLE = "LumberGL";

float paintProgress = 0.0f; 

// Function declarations
unsigned int compileShader(GLenum shaderType, const char* source);
unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void updateTreeBaseColors(float* treeBase, unsigned int VBO, float paintProgress);
float clip(float n, float lower, float upper);

int main() {


    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return 3;
    }

    unsigned int shaderProgram = createShaderProgram("basic.vert", "basic.frag");

    float triangleVertices[] = {
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,
    };

    float houseBase[] = {
        -0.3f, -0.5f , 0.0f,      0.196f, 0.204f, 0.22f,
         0.3f, -0.5f , 0.0f,      0.196f, 0.204f, 0.22f,
         0.3f, -0.45f, 0.0f,      0.196f, 0.204f, 0.22f,

        -0.3f, -0.5f , 0.0f,     0.196f, 0.204f, 0.22f,
         0.3f, -0.45f, 0.0f,    0.196f, 0.204f, 0.22f,
        -0.3f, -0.45f, 0.0f,    0.196f, 0.204f, 0.22f,
    };

    float firstFloor[] = {
        -0.28f, -0.45f, 0.0f,     0.51f, 0.604f, 0.8f,
         0.28f, -0.45f, 0.0f,     0.51f, 0.604f, 0.8f,
         0.28f, -0.15f, 0.0f,     0.51f, 0.604f, 0.8f,

        -0.28f,  -0.45f, 0.0f,    0.51f, 0.604f, 0.8f,
         0.28f,  -0.15f, 0.0f,    0.51f, 0.604f, 0.8f,
        -0.28f,  -0.15f, 0.0f,    0.51f, 0.604f, 0.8f,
    };

    float door[] = {
        -0.03f, -0.45f, 0.0f,     1.0f, 1.0f, 1.0f,
         0.03f, -0.45f, 0.0f,     1.0f, 1.0f, 1.0f,
         0.03f, -0.3f, 0.0f,     1.0f, 1.0f, 1.0f,

        -0.03f,  -0.45f, 0.0f,    1.0f, 1.0f, 1.0f,
         0.03f,  -0.3f, 0.0f,    1.0f, 1.0f, 1.0f,
        -0.03f,  -0.3f, 0.0f,    1.0f, 1.0f, 1.0f,
    };

    float win1[] = {
        -0.25f, -0.38f, 0.0f,     1.0f, 1.0f, 1.0f,
        -0.19f, -0.38f, 0.0f,     1.0f, 1.0f, 1.0f,
        -0.19f, -0.28f, 0.0f,     1.0f, 1.0f, 1.0f,

        -0.25f,  -0.38f, 0.0f,    1.0f, 1.0f, 1.0f,
        -0.19f,  -0.28f, 0.0f,    1.0f, 1.0f, 1.0f,
        -0.25f,  -0.28f, 0.0f,    1.0f, 1.0f, 1.0f,
    };

    float win2[] = {
        -0.12f, -0.38f, 0.0f,     1.0f, 1.0f, 1.0f,
        -0.06f, -0.38f, 0.0f,     1.0f, 1.0f, 1.0f,
        -0.06f, -0.28f, 0.0f,     1.0f, 1.0f, 1.0f,

        -0.12f,  -0.38f, 0.0f,    1.0f, 1.0f, 1.0f,
        -0.06f,  -0.28f, 0.0f,    1.0f, 1.0f, 1.0f,
        -0.12f,  -0.28f, 0.0f,    1.0f, 1.0f, 1.0f,
    };

    float win3[] = {
         0.06f, -0.38f, 0.0f,     1.0f, 1.0f, 1.0f,
         0.12f, -0.38f, 0.0f,     1.0f, 1.0f, 1.0f,
         0.12f, -0.28f, 0.0f,     1.0f, 1.0f, 1.0f,

         0.06f,  -0.38f, 0.0f,    1.0f, 1.0f, 1.0f,
         0.12f,  -0.28f, 0.0f,    1.0f, 1.0f, 1.0f,
         0.06f,  -0.28f, 0.0f,    1.0f, 1.0f, 1.0f,
    };

    float win4[] = {
         0.19f, -0.38f, 0.0f,     1.0f, 1.0f, 1.0f,
         0.25f, -0.38f, 0.0f,     1.0f, 1.0f, 1.0f,
         0.25f, -0.28f, 0.0f,     1.0f, 1.0f, 1.0f,

         0.19f,  -0.38f, 0.0f,    1.0f, 1.0f, 1.0f,
         0.25f,  -0.28f, 0.0f,    1.0f, 1.0f, 1.0f,
         0.19f,  -0.28f, 0.0f,    1.0f, 1.0f, 1.0f,
    };

    float doorHandle[] = {
        0.02f, -0.36f, 0.0f,   0.196f, 0.204f, 0.22f,
        0.03f, -0.38f, 0.0f,     0.196f, 0.204f, 0.22f,
        0.03f, -0.36f, 0.0f,     0.196f, 0.204f, 0.22f,
   };


    float firstRoof[] = {
        -0.34f, -0.15f, 0.0f,      0.812f, 0.075f, 0.212f,
         0.34f, -0.15f, 0.0f,      0.812f, 0.075f, 0.212f,
         0.32f, -0.05f, 0.0f,      0.812f, 0.075f, 0.212f,

        -0.34f, -0.15f, 0.0f,    0.812f, 0.075f, 0.212f,
         0.32f, -0.05f, 0.0f,    0.812f, 0.075f, 0.212f,
        -0.32f, -0.05f, 0.0f,    0.812f, 0.075f, 0.212f,
    };

    float secondFloor[] = {
        -0.16f, -0.05f, 0.0f,     0.51f, 0.604f, 0.8f,
         0.16f, -0.05f, 0.0f,     0.51f, 0.604f, 0.8f,
         0.16f,  0.15f, 0.0f,      0.51f, 0.604f, 0.8f,

        -0.16f, -0.05f, 0.0f,    0.51f, 0.604f, 0.8f,
         0.16f,  0.15f, 0.0f,     0.51f, 0.604f, 0.8f,
        -0.16f,  0.15f, 0.0f,     0.51f, 0.604f, 0.8f,
    };

    float win5[] = {
         -0.14f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f,
         -0.08f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f,
         -0.14f, 0.1f, 0.0f,     1.0f, 1.0f, 1.0f,

         -0.08f,  0.0f, 0.0f,    1.0f, 1.0f, 1.0f,
         -0.14f,  0.1f, 0.0f,    1.0f, 1.0f, 1.0f,
         -0.08f,  0.1f, 0.0f,    1.0f, 1.0f, 1.0f,
    };

    float win6[] = {
         -0.03f,  0.0f, 0.0f,     1.0f, 1.0f, 1.0f,
          0.03f,  0.0f, 0.0f,     1.0f, 1.0f, 1.0f,
          0.03f,  0.1f, 0.0f,     1.0f, 1.0f, 1.0f,

         -0.03f,   0.0f, 0.0f,    1.0f, 1.0f, 1.0f,
          0.03f,   0.1f, 0.0f,    1.0f, 1.0f, 1.0f,
         -0.03f,   0.1f, 0.0f,    1.0f, 1.0f, 1.0f,
    };

    float win7[] = {
         0.08f,  0.0f, 0.0f,     1.0f, 1.0f, 1.0f,
         0.14f,  0.0f, 0.0f,     1.0f, 1.0f, 1.0f,
         0.14f,  0.1f, 0.0f,     1.0f, 1.0f, 1.0f,

         0.08f,  0.0, 0.0f,    1.0f, 1.0f, 1.0f,
         0.14f,  0.1f, 0.0f,    1.0f, 1.0f, 1.0f,
         0.08f,  0.1f, 0.0f,    1.0f, 1.0f, 1.0f,
    };

	float secondFloorExtension[] = {
		-0.16f,  0.15f, 0.0f,    0.51f, 0.604f, 0.8f,  
		 0.16f,  0.15f, 0.0f,    0.51f, 0.604f, 0.8f, 
		 0.0f,   0.3f,  0.0f,    0.51f, 0.604f, 0.8f,
	};

    float secondRoofLeft[] = {
        -0.20f,  0.20f, 0.0f,      0.812f, 0.075f, 0.212f,
		 0.0f,  0.3f,  0.0f,       0.812f, 0.075f, 0.212f, 
		 0.0f,  0.39f,  0.0f,      0.812f, 0.075f, 0.212f, 

		 0.0f,  0.3f,  0.0f,       0.812f, 0.075f, 0.212f, 
        -0.16f,  0.15f, 0.0f,      0.812f, 0.075f, 0.212f, 
        -0.20f,  0.20f, 0.0f,      0.812f, 0.075f, 0.212f,
    };

    float secondRoofRight[] = {
         0.20f,  0.20f, 0.0f,      0.812f, 0.075f, 0.212f,
		 0.0f,  0.3f,  0.0f,       0.812f, 0.075f, 0.212f, 
		 0.0f,  0.39f,  0.0f,      0.812f, 0.075f, 0.212f, 

		 0.0f,  0.3f,  0.0f,       0.812f, 0.075f, 0.212f, 
         0.16f,  0.15f, 0.0f,      0.812f, 0.075f, 0.212f, 
         0.20f,  0.20f, 0.0f,      0.812f, 0.075f, 0.212f,
    };

    float chimney[] = {
        0.15f,  0.33f,    0.0f,       0.812f, 0.075f, 0.212f,
        0.10f,  0.33f,   0.0f,       0.812f, 0.075f, 0.212f,
        0.10f,  0.2475f,  0.0f,       0.812f, 0.075f, 0.212f,

        0.10f,  0.295f,    0.0f,       0.812f, 0.075f, 0.212f,
        0.15f,  0.2475f,    0.0f,       0.812f, 0.075f, 0.212f,
        0.15f,  0.33f,   0.0f,       0.812f, 0.075f, 0.212f,
    };

    float dogHouseBase[] = {
       -0.95f, -0.75f , 0.0f,     0.278f, 0.204, 0.145f,
       -0.8f, -0.75f , 0.0f,      0.278f, 0.204, 0.145f,
       -0.8f, -0.55f, 0.0f,       0.278f, 0.204, 0.145f,
       -0.95f, -0.75f , 0.0f,     0.278f, 0.204, 0.145f,
       -0.8f, -0.55f, 0.0f,       0.278f, 0.204, 0.145f,
      -0.95f, -0.55f, 0.0f,       0.278f, 0.204, 0.145f,
};

	float treeBase[] = {
		0.95f, -0.75f, 0.0f,      0.22f, 0.169f, 0.1f,
		0.85f, -0.75f, 0.0f,      0.22f, 0.169f, 0.1f,
		0.87f, -0.45f, 0.0f,      0.22f, 0.169f, 0.1f,

		0.87f, -0.45f, 0.0f,      0.22f, 0.169f, 0.1f,
		0.93f, -0.45f, 0.0f,      0.22f, 0.169f, 0.1f,
		0.95f, -0.75f, 0.0f,      0.22f, 0.169f, 0.1f,
		};



    float dogHouseRoof[] = {
        -0.96f, -0.55f, 0.0f,      0.812f, 0.075f, 0.212f,
        -0.79f, -0.55f, 0.0f,      0.812f, 0.075f, 0.212f,
        -0.79f, -0.45f, 0.0f,      0.812f, 0.075f, 0.212f,

        -0.96f, -0.55f, 0.0f,    0.812f, 0.075f, 0.212f,
        -0.79f, -0.45f, 0.0f,    0.812f, 0.075f, 0.212f,
        -0.96f, -0.45f, 0.0f,    0.812f, 0.075f, 0.212f,
    };

    float ellipseVertices[(ELLIPSE_SEGMENTS + 2) * 6] = {}; 
    float centerX = 0.9f;
    float centerY = -0.1f;
    float radiusX = 0.09f;
    float radiusY = 0.4f;
    float r = 0.192f, g = 0.42f, b = 0.161f;

    // Add the center vertex first
    ellipseVertices[0] = centerX;
    ellipseVertices[1] = centerY;
    ellipseVertices[2] = 0.0f; 
    ellipseVertices[3] = r;
    ellipseVertices[4] = g;
    ellipseVertices[5] = b;

    for (int i = 0; i <= ELLIPSE_SEGMENTS; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(ELLIPSE_SEGMENTS);

        ellipseVertices[(i + 1) * 6] = centerX + radiusX * cos(theta);
        ellipseVertices[(i + 1) * 6 + 1] = centerY + radiusY * sin(theta);
        ellipseVertices[(i + 1) * 6 + 2] = 0.0f; 
        ellipseVertices[(i + 1) * 6 + 3] = r;
        ellipseVertices[(i + 1) * 6 + 4] = g;
        ellipseVertices[(i + 1) * 6 + 5] = b;
    }


    float dog[] = {
        // left leg
        -0.7f, -0.78f, 0.0f,      0.82f, 0.604f, 0.272f,
        -0.68f, -0.78f, 0.0f,     0.82f, 0.604f, 0.272f,
        -0.68f, -0.68f, 0.0f,     0.82f, 0.604f, 0.272f,

        -0.7f,  -0.78f, 0.0f,    0.82f, 0.604, 0.272f,
        -0.68f, -0.68f, 0.0f,    0.82f, 0.604f, 0.272f,
        -0.7f,  -0.68f, 0.0f,    0.82f, 0.604f, 0.272f,

        // right leg
        -0.62f, -0.78f, 0.0f,      0.82f, 0.604f, 0.272f,
        -0.60f, -0.78f, 0.0f,     0.82f, 0.604f, 0.272f,
        -0.60f, -0.68f, 0.0f,     0.82f, 0.604f, 0.272f,

        -0.62f,  -0.78f, 0.0f,    0.82f, 0.604, 0.272f,
        -0.60f, -0.68f, 0.0f,    0.82f, 0.604f, 0.272f,
        -0.62f,  -0.68f, 0.0f,    0.82f, 0.604f, 0.272f,

        // body
        -0.72f,  -0.68f, 0.0f,    0.82f, 0.604, 0.272f,
        -0.58f,  -0.68f,  0.0f,   0.82f, 0.604f, 0.272f,
        -0.58f,  -0.58f, 0.0f,    0.82f, 0.604f, 0.272f,

        -0.72f,  -0.68f, 0.0f,    0.82f, 0.604, 0.272f,
        -0.58f,  -0.58f,  0.0f,   0.82f, 0.604f, 0.272f,
        -0.72f,  -0.58f, 0.0f,    0.82f, 0.604f, 0.272f,

        // tail
        -0.72f,  -0.58f, 0.0f,    0.82f, 0.604f, 0.272f,
        -0.72f,  -0.61f, 0.0f,    0.82f, 0.604, 0.272f,
        -0.74f,  -0.70f, 0.0f,    0.82f, 0.604, 0.272f,

        -0.74f,  -0.70f, 0.0f,    0.82f, 0.604, 0.272f,
        -0.75f,  -0.69f, 0.0f,    0.82f, 0.604, 0.272f,
        -0.72f,  -0.58f, 0.0f,    0.82f, 0.604f, 0.272f,

        // head
        -0.58f,  -0.55f, 0.0f,    0.82f, 0.604f, 0.272f,
        -0.58f,  -0.60f,  0.0f,   0.82f, 0.604f, 0.272f,
        -0.54f,  -0.60f,  0.0f,   0.82f, 0.604f, 0.272f,

        -0.54f,  -0.60f,  0.0f,   0.82f, 0.604f, 0.272f,
        -0.54f,  -0.55f, 0.0f,    0.82f, 0.604f, 0.272f,
        -0.58f,  -0.55f,  0.0f,   0.82f, 0.604f, 0.272f,

        // ear
        -0.58f,  -0.55f,  0.0f,   0.82f, 0.604f, 0.272f,
        -0.58f,  -0.54f,  0.0f,   0.82f, 0.604f, 0.272f,
        -0.57f,  -0.55f,  0.0f,   0.82f, 0.604f, 0.272f,

        // eye
        -0.57f,  -0.57f,  0.0f,    0.0f, 0.0f, 0.0f,
        -0.57f,  -0.58f,  0.0f,   0.0f, 0.0f, 0.0f,
        -0.56f,  -0.58f,  0.0f,   0.0f, 0.0f, 0.0f,

        -0.57f,  -0.57f,  0.0f,    0.0f, 0.0f, 0.0f,
        -0.56f,  -0.58f,  0.0f,   0.0f, 0.0f, 0.0f,
        -0.56f,  -0.57f,  0.0f,   0.0f, 0.0f, 0.0f,

        //tongue
        -0.542f,  -0.595f,  0.0f,   0.949f, 0.749f, 0.941f,
        -0.538f,  -0.595f,  0.0f,   0.949f, 0.749f, 0.941f,
        -0.538f,  -0.618f,  0.0f,   0.949f, 0.749f, 0.941f,
    };




    float rectangleVertices[] = {
        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 
         1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,
         1.0f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f, 
         1.0f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,
        -1.0f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f  
    };

    unsigned int triangleVAO, triangleVBO;
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);

    glBindVertexArray(triangleVAO);

    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); 

    // Setup rectangle VAO and VBO
    unsigned int rectangleVAO, rectangleVBO;
    glGenVertexArrays(1, &rectangleVAO);
    glGenBuffers(1, &rectangleVBO);

    glBindVertexArray(rectangleVAO);

    glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); 

    // housebase
    unsigned int housebaseVAO, housebaseVBO;
    glGenVertexArrays(1, &housebaseVAO);
    glGenBuffers(1, &housebaseVBO);

    glBindVertexArray(housebaseVAO);

    glBindBuffer(GL_ARRAY_BUFFER, housebaseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(houseBase), houseBase, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); 

    // first Floor
    unsigned int firstfloorVAO, firstfloorVBO;
    glGenVertexArrays(1, &firstfloorVAO);
    glGenBuffers(1, &firstfloorVBO);

    glBindVertexArray(firstfloorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, firstfloorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(firstFloor), firstFloor, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); 

    // first roof
    unsigned int firstroofVAO, firstroofVBO;
    glGenVertexArrays(1, &firstroofVAO);
    glGenBuffers(1, &firstroofVBO);

    glBindVertexArray(firstroofVAO);

    glBindBuffer(GL_ARRAY_BUFFER, firstroofVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(firstRoof), firstRoof, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); 


    // second floor
    unsigned int secondfloorVAO, secondfloorVBO;
    glGenVertexArrays(1, &secondfloorVAO);
    glGenBuffers(1, &secondfloorVBO);

    glBindVertexArray(secondfloorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, secondfloorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondFloor), secondFloor, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); 


    // second floor extension
    unsigned int secondroofVAO, secondroofVBO;
    glGenVertexArrays(1, &secondroofVAO);
    glGenBuffers(1, &secondroofVBO);

    glBindVertexArray(secondroofVAO);

    glBindBuffer(GL_ARRAY_BUFFER, secondroofVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondFloorExtension), secondFloorExtension, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

	// Second Roof Left
    unsigned int secondroofleftVAO, secondroofleftVBO;
    glGenVertexArrays(1, &secondroofleftVAO);
    glGenBuffers(1, &secondroofleftVBO);

    glBindVertexArray(secondroofleftVAO);

    glBindBuffer(GL_ARRAY_BUFFER, secondroofleftVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondRoofLeft), secondRoofLeft, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Second Roof Right
    unsigned int secondroofrightVAO, secondroofrightVBO;
    glGenVertexArrays(1, &secondroofrightVAO);
    glGenBuffers(1, &secondroofrightVBO);

    glBindVertexArray(secondroofrightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, secondroofrightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondRoofRight), secondRoofRight, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Chimney
    unsigned int chimneyVAO, chimneyVBO;
    glGenVertexArrays(1, &chimneyVAO);
    glGenBuffers(1, &chimneyVBO);

    glBindVertexArray(chimneyVAO);

    glBindBuffer(GL_ARRAY_BUFFER, chimneyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(chimney), chimney, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); 


    // Door
    unsigned int doorVAO, doorVBO;
    glGenVertexArrays(1, &doorVAO);
    glGenBuffers(1, &doorVBO);

    glBindVertexArray(doorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, doorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(door), door, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); 


    // Door
    unsigned int handleVAO, handleVBO;
    glGenVertexArrays(1, &handleVAO);
    glGenBuffers(1, &handleVBO);

    glBindVertexArray(handleVAO);

    glBindBuffer(GL_ARRAY_BUFFER, handleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(doorHandle), doorHandle, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // windows
    unsigned int win1VAO, win1VBO;
    glGenVertexArrays(1, &win1VAO);
    glGenBuffers(1, &win1VBO);

    glBindVertexArray(win1VAO);

    glBindBuffer(GL_ARRAY_BUFFER, win1VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(win1), win1, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int win2VAO, win2VBO;
    glGenVertexArrays(1, &win2VAO);
    glGenBuffers(1, &win2VBO);

    glBindVertexArray(win2VAO);

    glBindBuffer(GL_ARRAY_BUFFER, win2VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(win2), win2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int win3VAO, win3VBO;
    glGenVertexArrays(1, &win3VAO);
    glGenBuffers(1, &win3VBO);

    glBindVertexArray(win3VAO);

    glBindBuffer(GL_ARRAY_BUFFER, win3VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(win3), win3, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int win4VAO, win4VBO;
    glGenVertexArrays(1, &win4VAO);
    glGenBuffers(1, &win4VBO);

    glBindVertexArray(win4VAO);

    glBindBuffer(GL_ARRAY_BUFFER, win4VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(win4), win4, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int win5VAO, win5VBO;
    glGenVertexArrays(1, &win5VAO);
    glGenBuffers(1, &win5VBO);

    glBindVertexArray(win5VAO);

    glBindBuffer(GL_ARRAY_BUFFER, win5VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(win5), win5, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int win6VAO, win6VBO;
    glGenVertexArrays(1, &win6VAO);
    glGenBuffers(1, &win6VBO);

    glBindVertexArray(win6VAO);

    glBindBuffer(GL_ARRAY_BUFFER, win6VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(win6), win6, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


    unsigned int win7VAO, win7VBO;
    glGenVertexArrays(1, &win7VAO);
    glGenBuffers(1, &win7VBO);

    glBindVertexArray(win7VAO);

    glBindBuffer(GL_ARRAY_BUFFER, win7VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(win7), win7, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);



    unsigned int doghousebaseVAO, doghousebaseVBO;
    glGenVertexArrays(1, &doghousebaseVAO);
    glGenBuffers(1, &doghousebaseVBO);

    glBindVertexArray(doghousebaseVAO);

    glBindBuffer(GL_ARRAY_BUFFER, doghousebaseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dogHouseBase), dogHouseBase, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


    unsigned int doghouseroofVAO, doghouseroofVBO;
    glGenVertexArrays(1, &doghouseroofVAO);
    glGenBuffers(1, &doghouseroofVBO);

    glBindVertexArray(doghouseroofVAO);

    glBindBuffer(GL_ARRAY_BUFFER, doghouseroofVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dogHouseRoof), dogHouseRoof, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int dogVAO, dogVBO;
    glGenVertexArrays(1, &dogVAO);
    glGenBuffers(1, &dogVBO);

    glBindVertexArray(dogVAO);

    glBindBuffer(GL_ARRAY_BUFFER, dogVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dog), dog, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int treebaseVAO, treebaseVBO;
    glGenVertexArrays(1, &treebaseVAO);
    glGenBuffers(1, &treebaseVBO);

    glBindVertexArray(treebaseVAO);

    glBindBuffer(GL_ARRAY_BUFFER, treebaseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(treeBase), treeBase, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int ellipseVAO, ellipseVBO;
    glGenVertexArrays(1, &ellipseVAO);
    glGenBuffers(1, &ellipseVBO);

    glBindVertexArray(ellipseVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ellipseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ellipseVertices), ellipseVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Color
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);



    int uH = SCR_HEIGHT;
    glUseProgram(shaderProgram);
    int uHLoc = glGetUniformLocation(shaderProgram, "uH");
    int isFenceLoc = glGetUniformLocation(shaderProgram, "isFence");
    glUniform1i(uHLoc, uH); 
    glUseProgram(shaderProgram);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        updateTreeBaseColors(treeBase, treebaseVBO, paintProgress);

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the rectangle
        glUniform1f(isFenceLoc, GL_TRUE);
        glBindVertexArray(rectangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw house base
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(housebaseVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw house first floor
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(firstfloorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw house first roof
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(firstroofVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw house first roof
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondfloorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw second floor extension (triangle)
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondroofVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Draw second roof left 
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondroofleftVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Draw second roof left
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondroofleftVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw second roof right
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondroofrightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw chimney
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(chimneyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw door
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(doorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw door handle
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(handleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Draw windows
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(win1VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(win2VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(win3VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(win4VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(win5VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(win6VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(win7VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(doghousebaseVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(doghouseroofVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(dogVAO);
        glDrawArrays(GL_TRIANGLES, 0, 42);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(treebaseVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(ellipseVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, ELLIPSE_SEGMENTS + 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &rectangleVAO);
    glDeleteBuffers(1, &rectangleVBO);
    glDeleteVertexArrays(1, &housebaseVAO);
    glDeleteBuffers(1, &housebaseVBO);
    glDeleteVertexArrays(1, &firstfloorVAO);
    glDeleteBuffers(1, &firstfloorVBO);
    glDeleteVertexArrays(1, &firstroofVAO);
    glDeleteBuffers(1, &firstroofVBO);
    glDeleteVertexArrays(1, &secondfloorVAO);
    glDeleteBuffers(1, &secondfloorVBO);
    glDeleteVertexArrays(1, &secondroofVAO);
    glDeleteBuffers(1, &secondroofVBO);
    glDeleteVertexArrays(1, &secondroofleftVAO);
    glDeleteBuffers(1, &secondroofleftVBO);
    glDeleteVertexArrays(1, &secondroofrightVAO);
    glDeleteBuffers(1, &secondroofrightVBO);
    glDeleteVertexArrays(1, &chimneyVAO);
    glDeleteBuffers(1, &chimneyVBO);
    glDeleteVertexArrays(1, &doorVAO);
    glDeleteBuffers(1, &doorVBO);
    glDeleteVertexArrays(1, &handleVAO);
    glDeleteBuffers(1, &handleVBO);
    glDeleteVertexArrays(1, &win1VAO);
    glDeleteBuffers(1, &win1VBO);
    glDeleteVertexArrays(1, &win2VAO);
    glDeleteBuffers(1, &win2VBO);
    glDeleteVertexArrays(1, &win3VAO);
    glDeleteBuffers(1, &win3VBO);
    glDeleteVertexArrays(1, &win4VAO);
    glDeleteBuffers(1, &win4VBO);
    glDeleteVertexArrays(1, &win5VAO);
    glDeleteBuffers(1, &win5VBO);
    glDeleteVertexArrays(1, &win6VAO);
    glDeleteBuffers(1, &win6VBO);
    glDeleteVertexArrays(1, &treebaseVBO);
    glDeleteBuffers(1, &treebaseVBO);
    glDeleteVertexArrays(1, &ellipseVBO);
    glDeleteBuffers(1, &ellipseVBO);
    glDeleteVertexArrays(1, &win7VAO);
    glDeleteBuffers(1, &win7VBO);
    glDeleteVertexArrays(1, &doghousebaseVAO);
    glDeleteBuffers(1, &doghousebaseVBO);
    glDeleteVertexArrays(1, &doghouseroofVAO);
    glDeleteBuffers(1, &doghouseroofVBO);
    glDeleteVertexArrays(1, &dogVAO);
    glDeleteBuffers(1, &dogVBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

unsigned int createShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderPath);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderPath);
    unsigned int program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader Program Validation Error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;

    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Successful read from a path: \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Error reading from a path: \"" << source << "\"!" << std::endl;
    }

    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);
    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf("Shader error! Error: \n");
        printf(infoLog);
    }

    return shader;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        paintProgress += 0.01f; // Increment progress
        paintProgress = clip(paintProgress, 0.0f, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        paintProgress -= 0.01f; // Decrement progress
        paintProgress = clip(paintProgress, 0.0f, 1.0f);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


void updateTreeBaseColors(float* treeBase, unsigned int VBO, float paintProgress) {
    float originalColor[3] = { 0.22f, 0.169f, 0.1f }; // Brown
    float whiteColor[3] = { 1.0f, 1.0f, 1.0f };       // White

    for (int i = 0; i < 6; ++i) { // Iterate through vertices in `treeBase`
        float yPos = treeBase[i * 6 + 1]; // Get the y-coordinate
        float progress = (yPos + 0.75f) / 0.3f; // Normalize y (-0.75 to -0.45 -> 0 to 1 range)

        // Adjust for bottom-to-top painting (remove inversion)
        // progress is directly proportional to height, no need for 1.0f - progress.

        // Calculate blendFactor that builds up over multiple presses
        float blendFactor = clip(paintProgress - progress, 0.0f, 1.0f);

        for (int j = 0; j < 3; ++j) {
            // Gradual accumulation of whiteness with more presses
            treeBase[i * 6 + 3 + j] = originalColor[j] * (1.0f - blendFactor) + whiteColor[j] * blendFactor;
        }
    }

    // Update the VBO with the new colors
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 6 * 6, treeBase);
}

float clip(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
}
