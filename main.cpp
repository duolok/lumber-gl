#define _CRT_SECURE_NO_WARNINGS
#define M_PI 3.14159265358979323846
#define ELLIPSE_SEGMENTS 50

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <map>
#include <cmath>
#include FT_FREETYPE_H

using namespace std;

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const char* WINDOW_TITLE = "LumberGL";
const char* infoText = "Dusan Lecic SV80/2021";

const float transitionDuration = 5.0f;
bool isDay = true;
bool keyPressed = false;
bool transitionInProgress = false;
bool dogGoingLeft = false;
float paintProgress = 0.0f;
float timeOfDay = 0.3f;
float dimFactor = 1.0f;
float transitionStartTime = 0.0f;
float sunMoonProgress = 0.0f;
float skyColor[3] = { 0.412f, 0.737f, 0.851f };
float daySkyColor[3] = { 0.412f, 0.737f, 0.851f };
float nightSkyColor[3] = { 0.0f, 0.0f, 0.1f };
float objectDimFactor = 1.0f;
float dogSpeed = 0.003f;
float dogX = 0.0f;
float dogY = 0.0f;
float dogMinX = -0.1f;
float dogMaxX = 1.3f;
float chimneyX = 0.125f;
float chimneyY = 0.33f;

unsigned int compileShader(GLenum shaderType, const char* source);
unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void updateTreeBaseColors(float* treeBase, unsigned int VBO, float paintProgress);
void calculateSunMoonPosition(float progress, float& sunX, float& sunY, float& moonX, float& moonY);
void updateCircleVertices(float* vertices, float centerX, float centerY, float radius, float* color);
void updateDayNightCycle(float& timeOfDay, float* skyColor, float& objectDimFactor, bool& isDay);
void RenderTopRightText(unsigned int textShader, const std::string& text, float yOffset, float scale, glm::vec3 color);
void RenderText(unsigned int shader, std::string text, float x, float y, float scale, glm::vec3 color);
float CalculateTextWidth(const std::string& text, float scale);
float clip(float n, float lower, float upper);

struct Character {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

std::map<char, Character> Characters;

unsigned int textVAO, textVBO;

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

    // Initialize FreeType
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "fonts/ComicMono.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    FT_Set_Pixel_Sizes(face, 0, 20);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 200; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load Glyph" << std::endl;
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int shaderProgram = createShaderProgram("basic.vert", "basic.frag");
    unsigned int sunShader = createShaderProgram("sun.vert", "sun.frag");
    unsigned int dogShader = createShaderProgram("dog.vert", "dog.frag");
    unsigned int smokeShader = createShaderProgram("smoke.vert", "smoke.frag");
    unsigned int textShader = createShaderProgram("text.vert", "text.frag");


    float triangleVertices[] = {
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,
    };

    float sky[] = {
        -1.0f, 0.0f, 0.0f, 0.412f, 0.737f, 0.851f,
        1.0f, 0.0f, 0.0f,  0.412f, 0.737f, 0.851f,
        1.0f, 1.0f, 0.0f,  0.412f, 0.737f, 0.851f,

        1.0f, 1.0f, 0.0f,  0.412f, 0.737f, 0.851f,
        -1.0f, 1.0f, 0.0f, 0.412f, 0.737f, 0.851f,
        -1.0f, 0.0f, 0.0f, 0.412f, 0.737f, 0.851f,
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

    float chimneySmoke[] = {
        0.24f, 0.40f, 0.0f,          0.341f, 0.341f, 0.341f,
        0.24f, 0.70f, 0.0f,          0.341f, 0.341f, 0.341f,
        0.13f, 0.40f, 0.0f,          0.341f, 0.341f, 0.341f,

        0.13f, 0.40f, 0.0f,          0.341f, 0.341f, 0.341f,
        0.13f, 0.70f, 0.0f,          0.341f, 0.341f, 0.341f,
        0.24f, 0.70f, 0.0f,          0.341f, 0.341f, 0.341f,
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

    float rectangleVertices[] = {
        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,
         1.0f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 1.0f,
         1.0f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f,
        -1.0f, -0.8f, 0.0f,   1.0f, 1.0f, 1.0f
    };


    float sunVertices[(ELLIPSE_SEGMENTS + 2) * 6];
    float moonVertices[(ELLIPSE_SEGMENTS + 2) * 6];
    float sunColor[3] = { 1.0f, 1.0f, 0.0f };
    float moonColor[3] = { 0.8f, 0.8f, 0.8f };

    unsigned int sunVAO, sunVBO;
    unsigned int moonVAO, moonVBO;

    // Generate and bind buffers for sun
    glGenVertexArrays(1, &sunVAO);
    glGenBuffers(1, &sunVBO);
    glBindVertexArray(sunVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunVertices), sunVertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Generate and bind buffers for moon
    glGenVertexArrays(1, &moonVAO);
    glGenBuffers(1, &moonVBO);
    glBindVertexArray(moonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(moonVertices), moonVertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    unsigned int smokeVAO, smokeVBO;
    glGenVertexArrays(1, &smokeVAO);
    glGenBuffers(1, &smokeVBO);

    glBindVertexArray(smokeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, smokeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(chimneySmoke), chimneySmoke, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


    unsigned int skyVAO, skyVBO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);

    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sky), sky, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);


    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    float sunX, sunY, moonX, moonY;
    int uH = SCR_HEIGHT;
    int uHLoc = glGetUniformLocation(shaderProgram, "uH");
    int isFenceLoc = glGetUniformLocation(shaderProgram, "isFence");
    int dimLoc = glGetUniformLocation(shaderProgram, "dim");

    glUniform1i(uHLoc, uH);
    glUseProgram(shaderProgram);
    glUniform1f(dimLoc, dimFactor);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        updateTreeBaseColors(treeBase, treebaseVBO, paintProgress);

        if (transitionInProgress) {
            float currentTime = glfwGetTime();
            sunMoonProgress = (currentTime - transitionStartTime) / transitionDuration;
            if (sunMoonProgress >= 1.0f) {
                sunMoonProgress = 1.0f;
                transitionInProgress = false;
                isDay = !isDay;
            }
        }
        else {
            sunMoonProgress = 0.0f;
        }

        if (isDay) { dimFactor = 1.0f - 0.5f * sunMoonProgress; }
        else { dimFactor = 0.5f + 0.5f * sunMoonProgress; }


        calculateSunMoonPosition(sunMoonProgress, sunX, sunY, moonX, moonY);

        float sunAngleStart = isDay ? 0.0f : M_PI; // Start angle
        float sunAngleEnd = isDay ? M_PI : 0.0f;   // End angle
        float sunAngle = sunAngleStart + (sunAngleEnd - sunAngleStart) * sunMoonProgress;

        for (int i = 0; i < 3; ++i) {
            if (isDay) {
                skyColor[i] = daySkyColor[i] * (1.0f - sunMoonProgress) + nightSkyColor[i] * sunMoonProgress;
            }
            else {
                skyColor[i] = nightSkyColor[i] * (1.0f - sunMoonProgress) + daySkyColor[i] * sunMoonProgress;
            }
        }

        for (int i = 0; i < 6; ++i) {
            sky[i * 6 + 3] = skyColor[0];
            sky[i * 6 + 4] = skyColor[1];
            sky[i * 6 + 5] = skyColor[2];
        }

        glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sky), sky);


        glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sky), sky);

        glBindBuffer(GL_ARRAY_BUFFER, housebaseVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(houseBase), houseBase);

        updateCircleVertices(sunVertices, sunX, sunY, 0.1f, sunColor);
        updateCircleVertices(moonVertices, moonX, moonY, 0.1f, moonColor);

        glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sunVertices), sunVertices);

        glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(moonVertices), moonVertices);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_TRUE);
        glBindVertexArray(rectangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(housebaseVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(firstfloorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(firstroofVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondfloorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondroofVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondroofleftVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondroofleftVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(secondroofrightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(chimneyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(doorVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(handleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
        glBindVertexArray(treebaseVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(ellipseVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, ELLIPSE_SEGMENTS + 2);

        glUniform1f(isFenceLoc, GL_FALSE);
        glBindVertexArray(moonVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, ELLIPSE_SEGMENTS + 2);

        glUseProgram(sunShader);
        glUniform1f(glGetUniformLocation(sunShader, "time"), glfwGetTime());
        glBindVertexArray(sunVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, ELLIPSE_SEGMENTS + 2);

        glUseProgram(dogShader);
        glUniform2f(glGetUniformLocation(dogShader, "uPos"), dogX, dogY);
        glUniform1i(glGetUniformLocation(dogShader, "uFlip"), dogGoingLeft);
        glBindVertexArray(dogVAO);
        glDrawArrays(GL_TRIANGLES, 0, 42);
        glBindVertexArray(0);
        glUseProgram(0);

        glUseProgram(smokeShader);
        float currentTime = glfwGetTime();
        glUniform1f(glGetUniformLocation(smokeShader, "uTime"), currentTime);
        glUniform2f(glGetUniformLocation(smokeShader, "uOrigin"), 0.125f, 0.33f);
        glBindVertexArray(smokeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glUseProgram(textShader);
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
        glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		RenderTopRightText(textShader, infoText, 50.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &rectangleVAO);
    glDeleteBuffers(1, &rectangleVBO);
    glDeleteVertexArrays(1, &skyVAO);
    glDeleteBuffers(1, &skyVBO);
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
        paintProgress += 0.01f;
        paintProgress = clip(paintProgress, 0.0f, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        paintProgress -= 0.01f;
        paintProgress = clip(paintProgress, 0.0f, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !keyPressed) {
        keyPressed = true;
        transitionInProgress = true;
        transitionStartTime = glfwGetTime();
        cout << "Toggled day/night: " << (isDay ? "Day" : "Night") << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
        keyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        dogX -= dogSpeed;
        if (dogX < dogMinX) {
            dogX = dogMinX;
        }
        dogGoingLeft = true;

    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        dogX += dogSpeed;
        if (dogX > dogMaxX) {
            dogX = dogMaxX;
        }
        dogGoingLeft = false;
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


void updateTreeBaseColors(float* treeBase, unsigned int VBO, float paintProgress) {
    float originalColor[3] = { 0.22f, 0.169f, 0.1f };
    float whiteColor[3] = { 1.0f, 1.0f, 1.0f };

    for (int i = 0; i < 6; ++i) {
        float yPos = treeBase[i * 6 + 1];
        float progress = (yPos + 0.75f) / 0.3f; // normalize y (-0.75 to -0.45 -> 0 to 1 range)

        float blendFactor = clip(paintProgress - progress, 0.0f, 1.0f);

        for (int j = 0; j < 3; ++j) {
            treeBase[i * 6 + 3 + j] = originalColor[j] * (1.0f - blendFactor) + whiteColor[j] * blendFactor;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 6 * 6, treeBase);
}

void updateSunMoonPositionOnToggle(bool isDay, float& sunX, float& sunY, float& moonX, float& moonY) {
    float radius = 1.1f; // Circular orbit radius
    if (isDay) {
        sunX = 0.8f;
        sunY = radius;
        moonX = -0.8f;
        moonY = -radius;
    }
    else {
        // Moon at the top
        sunX = 0.0f;
        sunY = -radius;
        moonX = 0.0f;
        moonY = radius;
    }
}

void calculateSunMoonPosition(float progress, float& sunX, float& sunY, float& moonX, float& moonY) {
    float sunStartX, sunStartY, sunEndX, sunEndY;
    float moonStartX, moonStartY, moonEndX, moonEndY;
    if (isDay) {
        sunStartX = -0.8f;
        sunStartY = 0.8f;
        sunEndX = 1.2f;
        sunEndY = 0.6f;

        moonStartX = -1.2f;
        moonStartY = 0.6f;
        moonEndX = -0.8f;
        moonEndY = 0.8f;
    }
    else {
        moonStartX = -0.8f;
        moonStartY = 0.8f;
        moonEndX = 1.2f;
        moonEndY = 0.6f;

        sunStartX = -1.2f;
        sunStartY = 0.6f;
        sunEndX = -0.8f;
        sunEndY = 0.8f;
    }

    sunX = sunStartX + (sunEndX - sunStartX) * progress;
    sunY = sunStartY + (sunEndY - sunStartY) * progress;

    moonX = moonStartX + (moonEndX - moonStartX) * progress;
    moonY = moonStartY + (moonEndY - moonStartY) * progress;
}


void updateCircleVertices(float* vertices, float centerX, float centerY, float radius, float* color) {
    vertices[0] = centerX;
    vertices[1] = centerY;
    vertices[2] = 0.0f;
    vertices[3] = color[0];
    vertices[4] = color[1];
    vertices[5] = color[2];

    for (int i = 0; i <= ELLIPSE_SEGMENTS; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(ELLIPSE_SEGMENTS);
        vertices[(i + 1) * 6] = centerX + radius * cos(theta);
        vertices[(i + 1) * 6 + 1] = centerY + radius * sin(theta);
        vertices[(i + 1) * 6 + 2] = 0.0f;
        vertices[(i + 1) * 6 + 3] = color[0];
        vertices[(i + 1) * 6 + 4] = color[1];
        vertices[(i + 1) * 6 + 5] = color[2];
    }
}

void RenderText(unsigned int shader, std::string text, float x, float y, float scale, glm::vec3 color) {
    // Activate corresponding render state
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Iterate through all characters
    for (auto c : text) {
        Character ch = Characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance cursors for next glyph (advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}



float clip(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
}
// Function to calculate text width
float CalculateTextWidth(const std::string& text, float scale) {
    float width = 0.0f;
    for (const char& c : text) {
        Character ch = Characters[c];
        width += (ch.Advance >> 6) * scale; // Advance is in 1/64th pixels
    }
    return width;
}

void RenderTopRightText(unsigned int textShader, const std::string& text, float yOffset, float scale, glm::vec3 color) {
    // Activate the text shader and set the projection
    glUseProgram(textShader);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Calculate text width
    float textWidth = CalculateTextWidth(text, scale);

    // Position the text at the top-right corner
    float x = SCR_WIDTH - textWidth - 10.0f; // Add some padding
    float y = SCR_HEIGHT - yOffset;         // Adjust for y offset

    RenderText(textShader, text, x, y, scale, color);
}

// In your rendering loop


