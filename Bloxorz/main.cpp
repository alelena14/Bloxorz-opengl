#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "loadShaders.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <string>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


GLuint VaoId, VboId, ColorBufferId, ProgramId;
GLuint bgVao, bgVbo;
GLuint bgTexture;
GLuint bgProgramId;
GLuint NormalBufferId;
GLuint codColLocation;
GLuint TexCoordBufferId;
GLuint platformTexture;
GLuint platformProgram;
GLuint skyboxTexture;
GLuint skyboxProgram;
GLuint skyboxVAO, skyboxVBO;
GLuint depthMapFBO;
GLuint depthMap;
GLuint shadowProgram;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 lightProjection, lightView, lightSpaceMatrix;

glm::vec3 lightPos(-25.0f, 70.0f, 5.0f);

int moveCount = 0;

enum GameState
{
    PLAYING,        // control normal
    ROLLING,        // rostogolire
    EDGE_FALL,      // cadere pe margine
    FLOATING,       // win animation
    RESETTING       // reset / schimbare nivel
};

GameState gameState = PLAYING;

// ================= LEVEL LAYOUT =================

const float PLATFORM_HEIGHT = 0.2f;

// targetul final
int goalI, goalJ;
float targetThickness = 0.05f;
float targetY = PLATFORM_HEIGHT + targetThickness / 2.0f + 0.008f;

struct Level
{
    int rows;
    int cols;
    std::vector<std::vector<int>> map;

    int startI, startJ;
    int goalI, goalJ;
};


std::vector<Level> levels =
{
    // ================= LEVEL 1 =================
    {
        6, 10,
        {
            {1,1,1,0,0,0,0,0,0,0},
            {1,1,1,1,1,1,0,0,0,0},
            {1,1,1,1,1,1,1,1,1,0},
            {0,1,1,1,1,1,1,1,1,1},
            {0,0,0,0,0,1,1,1,1,1},
            {0,0,0,0,0,0,1,1,1,0}
        },
        1, 1,
        4, 7
    },

    // ================= LEVEL 2 =================
    {
        6, 5,
        {
            {0,1,1,1,1},
            {1,1,1,1,0},
            {1,1,0,1,0},
            {1,1,1,1,1},
            {0,0,1,1,1},
            {0,0,1,1,1}
        },
        0, 4,
        4, 3
    },

    // ================= LEVEL 3 =================
    {
        10, 15,
        {
            {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0},
            {0,0,0,0,0,1,0,0,1,1,1,0,0,0,0},
            {0,0,0,0,0,1,0,0,1,1,1,1,1,0,0},
            {1,1,1,1,1,1,0,0,0,0,0,1,1,1,1},
            {0,0,0,0,1,1,1,0,0,0,0,1,1,1,1},
            {0,0,0,0,1,1,1,0,0,0,0,0,1,1,1},
            {0,0,0,0,0,0,1,0,0,1,1,0,0,0,0},
            {0,0,0,0,0,0,1,1,1,1,1,0,0,0,0},
            {0,0,0,0,0,0,1,1,1,1,1,0,0,0,0},
            {0,0,0,0,0,0,0,1,1,1,0,0,0,0,0}
        },
        3, 0,
        4, 13
    }
};

int currentLevel = 0;
int LEVEL_COUNT = (int)levels.size();


// ================= BLOCK =================
enum BlockState {
    STANDING,   // vertical (ocupa 1 celula)
    LYING_X,    // culcat pe axa X (ocupa 2 celule pe coloana)
    LYING_Z     // culcat pe axa Z (ocupa 2 celule pe rand)
};

BlockState blockState = STANDING;
BlockState renderState;

const float BLOCK_HALF_STANDING = 1.0f;
const float BLOCK_HALF_LYING = 0.5f;

// target
int moveTargetI, moveTargetJ;
BlockState targetState;

// pozitia blocului pe grid
int blockI = 1;
int blockJ = 1;


// ============== ANIMATION ==============
glm::quat startRot;
glm::quat endRot;
float animT = 0.0f; // roll animation

bool fallAfterRoll = false;
glm::vec3 animAxis;

glm::mat4 view, projection;

// pentru plutirea de final
float floatY = 0.0f;
float floatSpeed = 0.08f;

// ============= CAMERA ===============
glm::vec3 cameraPos;        // pozitia reala a camerei
glm::vec3 cameraTarget;     // ce urmareste camera
glm::vec3 cameraDesiredPos; // pozitia dorita

float camYaw = glm::radians(60.0f);   // stanga-dreapta
float camPitch = glm::radians(60.0f); // sus-jos
float camDist = 15.0f;

float cameraSmooth = 0.05f;


// ================= SHADOW MAP =================
void InitShadowMap()
{
    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    float borderColor[] = { 1,1,1,1 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthMap, 0
    );

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// construire matrice umbra
glm::mat4 BuildShadowMatrix(glm::vec4 plane, glm::vec4 light)
{
    float dot = plane.x * light.x +
        plane.y * light.y +
        plane.z * light.z +
        plane.w * light.w;

    glm::mat4 m(0.0f);

    m[0][0] = dot - light.x * plane.x;
    m[1][0] = 0.0f - light.x * plane.y;
    m[2][0] = 0.0f - light.x * plane.z;
    m[3][0] = 0.0f - light.x * plane.w;

    m[0][1] = 0.0f - light.y * plane.x;
    m[1][1] = dot - light.y * plane.y;
    m[2][1] = 0.0f - light.y * plane.z;
    m[3][1] = 0.0f - light.y * plane.w;

    m[0][2] = 0.0f - light.z * plane.x;
    m[1][2] = 0.0f - light.z * plane.y;
    m[2][2] = dot - light.z * plane.z;
    m[3][2] = 0.0f - light.z * plane.w;

    m[0][3] = 0.0f - light.w * plane.x;
    m[1][3] = 0.0f - light.w * plane.y;
    m[2][3] = 0.0f - light.w * plane.z;
    m[3][3] = dot - light.w * plane.w;

    return m;
}

void startEdgeFall()
{
    gameState = EDGE_FALL;
}

glm::vec3 gridToWorld(int i, int j)
{
    const Level& lvl = levels[currentLevel];

    float centerX = lvl.cols * 0.5f;
    float centerZ = lvl.rows * 0.5f;

    return glm::vec3(
        j - centerX,
        0.0f,
        i - centerZ
    );
}

void resetLevel()
{
    moveCount = 0;

    const Level& lvl = levels[currentLevel];

    blockI = lvl.startI;
    blockJ = lvl.startJ;

    goalI = lvl.goalI;
    goalJ = lvl.goalJ;

    blockState = STANDING;
    renderState = STANDING;

    animT = 0.0f;
    floatY = 0.0f;

    cameraTarget = gridToWorld(blockI, blockJ);
    cameraTarget.y = 1.0f;

    gameState = PLAYING;
}

bool isOnTarget()
{
    return blockState == STANDING &&
        levels[currentLevel].goalI == blockI &&
        levels[currentLevel].goalJ == blockJ;
}

bool isInside(int i, int j)
{
    const Level& lvl = levels[currentLevel];
    return i >= 0 && i < lvl.rows &&
        j >= 0 && j < lvl.cols;
}

bool isValidPosition(int i, int j, BlockState state)
{
    const Level& lvl = levels[currentLevel];

    if (state == STANDING)
        return isInside(i, j) && lvl.map[i][j];

    if (state == LYING_X)
        return isInside(i, j) &&
        isInside(i, j + 1) &&
        lvl.map[i][j] &&
        lvl.map[i][j + 1];

    // LYING_Z
    return isInside(i, j) &&
        isInside(i + 1, j) &&
        lvl.map[i][j] &&
        lvl.map[i + 1][j];
}

void beginRoll()
{
    // rotatii
    startRot = glm::quat(1, 0, 0, 0);
    endRot = glm::angleAxis(
        glm::radians(90.0f),
        glm::normalize(animAxis)
    );

    renderState = blockState;
    moveCount++;
    animT = 0.0f;
    gameState = ROLLING;
}

void startMoveUp() {
    if (gameState != PLAYING) return;
    animAxis = glm::vec3(-1, 0, 0);

    if (blockState == STANDING) {
        moveTargetI = blockI - 2;
        moveTargetJ = blockJ;
        targetState = LYING_Z;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else if (blockState == LYING_Z) {
        // ridicare
        moveTargetI = blockI - 1;
        moveTargetJ = blockJ;
        targetState = STANDING;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }

    else if (blockState == LYING_X) {
        // mutare culcat
        moveTargetI = blockI - 1;
        moveTargetJ = blockJ;
        targetState = LYING_X;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else return;

    beginRoll();

}

void startMoveDown() {
    if (gameState != PLAYING) return;
    animAxis = glm::vec3(1, 0, 0);

    if (blockState == STANDING) {
        moveTargetI = blockI + 1;
        moveTargetJ = blockJ;
        targetState = LYING_Z;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else if (blockState == LYING_Z) {
        // ridicare
        moveTargetI = blockI + 2;
        moveTargetJ = blockJ;
        targetState = STANDING;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else if (blockState == LYING_X) {
        // mutare culcat
        moveTargetI = blockI + 1;
        moveTargetJ = blockJ;
        targetState = LYING_X;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else return;

    beginRoll();

}

void startMoveLeft() {
    if (gameState != PLAYING) return;
    animAxis = glm::vec3(0, 0, 1);

    if (blockState == STANDING) {
        moveTargetI = blockI;
        moveTargetJ = blockJ - 2;
        targetState = LYING_X;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else if (blockState == LYING_X) {
        // ridicare
        moveTargetI = blockI;
        moveTargetJ = blockJ - 1;
        targetState = STANDING;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else if (blockState == LYING_Z) {
        // mutare culcat
        moveTargetI = blockI;
        moveTargetJ = blockJ - 1;
        targetState = LYING_Z;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else return;

    beginRoll();

}

void startMoveRight() {
    if (gameState != PLAYING) return;
    animAxis = glm::vec3(0, 0, -1);

    if (blockState == STANDING) {
        moveTargetI = blockI;
        moveTargetJ = blockJ + 1;
        targetState = LYING_X;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else if (blockState == LYING_X) {
        // ridicare
        moveTargetI = blockI;
        moveTargetJ = blockJ + 2;
        targetState = STANDING;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else if (blockState == LYING_Z) {
        // mutare culcat
        moveTargetI = blockI;
        moveTargetJ = blockJ + 1;
        targetState = LYING_Z;

        if (!isValidPosition(moveTargetI, moveTargetJ, targetState))
            fallAfterRoll = true;
    }
    else return;

    beginRoll();

}

void animate(int)
{
    switch (gameState)
    {
    case ROLLING:
        animT += 0.05f;

        if (animT >= 1.0f)
        {
            animT = 1.0f;

            blockI = moveTargetI;
            blockJ = moveTargetJ;
            blockState = targetState;

            if (fallAfterRoll)
            {
                startEdgeFall(); 
                fallAfterRoll = false;
            }
            else if (isOnTarget())
            {
                gameState = FLOATING;
            }
            else
            {
                gameState = PLAYING;
            }
        }
        break;


    case EDGE_FALL:
        floatY -= floatSpeed;

        if (floatY < -6.0f)
            gameState = RESETTING;
        break;


    case FLOATING:
        floatY += floatSpeed;
        if (floatY > 6.0f)
        {
            currentLevel++;
            if (currentLevel >= LEVEL_COUNT)
                currentLevel = 0; // reincepe jocul
            gameState = RESETTING;
        }
        break;


    case RESETTING:
        resetLevel();
        break;

    default:
        break;
    }

    glutPostRedisplay();
    glutTimerFunc(10, animate, 0);
}

void processNormalKeys(unsigned char key, int x, int y)
{
    if (gameState != PLAYING) return;

    switch (key) {
    case 'w': startMoveUp();    break;
    case 's': startMoveDown();  break;
    case 'a': startMoveLeft();  break;
    case 'd': startMoveRight(); break;
    }

    glutPostRedisplay();
}

GLuint loadTexture(const char* filename)
{
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data =
        stbi_load(filename, &width, &height, &channels, 0);

    if (!data)
    {
        printf("Failed to load texture: %s\n", filename);
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);
    return texture;
}

void drawText(float x, float y, const char* text)
{
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

// ================= SKYBOX =================

float skyboxVertices[] = {
    -1, 1, -1, -1, -1, -1, 1, -1, -1,
    1, -1, -1, 1, 1, -1, -1, 1, -1,

    -1, -1, 1, -1, -1, -1, -1, 1, -1,
    -1, 1, -1, -1, 1, 1, -1, -1, 1,

    1, -1, -1, 1, -1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, -1, 1, -1, -1,

    -1, -1, 1, -1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, -1, 1, -1, -1, 1,

    -1, 1, -1, 1, 1, -1, 1, 1, 1,
    1, 1, 1, -1, 1, 1, -1, 1, -1,

    -1, -1, -1, -1, -1, 1, 1, -1, -1,
    1, -1, -1, -1, -1, 1, 1, -1, 1
};

// ================= CUBOID =================

GLfloat cubeVertices[] =
{
    // TOP (y = +0.5)
    -0.5f, 0.5f,-0.5f, 1,  0.5f, 0.5f,-0.5f, 1,  0.5f, 0.5f, 0.5f, 1,
     0.5f, 0.5f, 0.5f, 1, -0.5f, 0.5f, 0.5f, 1, -0.5f, 0.5f,-0.5f, 1,

     // BOTTOM (y = -0.5)
     -0.5f,-0.5f,-0.5f, 1,  0.5f,-0.5f,-0.5f, 1,  0.5f,-0.5f, 0.5f, 1,
      0.5f,-0.5f, 0.5f, 1, -0.5f,-0.5f, 0.5f, 1, -0.5f,-0.5f,-0.5f, 1,

      // FRONT (z = +0.5)
      -0.5f,-0.5f, 0.5f, 1,  0.5f,-0.5f, 0.5f, 1,  0.5f, 0.5f, 0.5f, 1,
       0.5f, 0.5f, 0.5f, 1, -0.5f, 0.5f, 0.5f, 1, -0.5f,-0.5f, 0.5f, 1,

       // BACK (z = -0.5)
       -0.5f,-0.5f,-0.5f, 1,  0.5f,-0.5f,-0.5f, 1,  0.5f, 0.5f,-0.5f, 1,
        0.5f, 0.5f,-0.5f, 1, -0.5f, 0.5f,-0.5f, 1, -0.5f,-0.5f,-0.5f, 1,

        // RIGHT (x = +0.5)
         0.5f,-0.5f,-0.5f, 1,  0.5f,-0.5f, 0.5f, 1,  0.5f, 0.5f, 0.5f, 1,
         0.5f, 0.5f, 0.5f, 1,  0.5f, 0.5f,-0.5f, 1,  0.5f,-0.5f,-0.5f, 1,

         // LEFT (x = -0.5)
         -0.5f,-0.5f,-0.5f, 1, -0.5f,-0.5f, 0.5f, 1, -0.5f, 0.5f, 0.5f, 1,
         -0.5f, 0.5f, 0.5f, 1, -0.5f, 0.5f,-0.5f, 1, -0.5f,-0.5f,-0.5f, 1
};

GLfloat cubeNormals[] =
{
    // TOP
    0,1,0,  0,1,0,  0,1,0,  0,1,0,  0,1,0,  0,1,0,
    // BOTTOM
    0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0,
    // FRONT
    0,0,1,  0,0,1,  0,0,1,  0,0,1,  0,0,1,  0,0,1,
    // BACK
    0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1,
    // RIGHT
    1,0,0,  1,0,0,  1,0,0,  1,0,0,  1,0,0,  1,0,0,
    // LEFT
   -1,0,0, -1,0,0, -1,0,0, -1,0,0, -1,0,0, -1,0,0
};

GLfloat cubeTexCoords[] =
{
    // TOP
    0,0, 1,0, 1,1,  1,1, 0,1, 0,0,
    // BOTTOM
    0,0, 1,0, 1,1,  1,1, 0,1, 0,0,
    // FRONT
    0,0, 1,0, 1,1,  1,1, 0,1, 0,0,
    // BACK
    0,0, 1,0, 1,1,  1,1, 0,1, 0,0,
    // RIGHT
    0,0, 1,0, 1,1,  1,1, 0,1, 0,0,
    // LEFT
    0,0, 1,0, 1,1,  1,1, 0,1, 0,0
};

GLfloat cubeColors[36 * 4];

// ================= OPENGL =================

void CreateVBO(void)
{
    for (int i = 0; i < 36; i++)
    {
        cubeColors[i * 4 + 0] = 1.0f;
        cubeColors[i * 4 + 1] = 0.0f;
        cubeColors[i * 4 + 2] = 0.4f;
        cubeColors[i * 4 + 3] = 1.0f;
    }

    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    // ===== POZITII =====
    glGenBuffers(1, &VboId);
    glBindBuffer(GL_ARRAY_BUFFER, VboId);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(cubeVertices),
        cubeVertices,
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // ===== CULORI =====
    glGenBuffers(1, &ColorBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(cubeColors),
        cubeColors,
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // ===== NORMALE =====
    glGenBuffers(1, &NormalBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, NormalBufferId);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(cubeNormals),
        cubeNormals,
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // ===== TEXCOORDS =====
    glGenBuffers(1, &TexCoordBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBufferId);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(cubeTexCoords),
        cubeTexCoords,
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

}

void DestroyVBO(void)
{
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &ColorBufferId);
    glDeleteBuffers(1, &VboId);
    glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
    ProgramId = LoadShaders("example.vert", "example.frag");
    glUseProgram(ProgramId);
}

GLuint loadCubemap(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(
            faces[i].c_str(),
            &width, &height, &channels, 0
        );

        if (data)
        {
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                format,
                width,
                height,
                0,
                format,
                GL_UNSIGNED_BYTE,
                data
            );
        }
        else
        {
            printf("Failed to load skybox texture: %s\n", faces[i].c_str());
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


void Initialize(void)
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    CreateVBO();
    CreateShaders();

    shadowProgram = LoadShaders("shadow.vert", "shadow.frag");
    InitShadowMap();

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    std::vector<std::string> faces = {
    "textures/right.png",
    "textures/left.png",
    "textures/top.png",
    "textures/bottom.png",
    "textures/front.png",
    "textures/back.png"
    };

    skyboxTexture = loadCubemap(faces);

    skyboxProgram = LoadShaders("skybox.vert", "skybox.frag");

    glUseProgram(skyboxProgram);
    glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);


    codColLocation = glGetUniformLocation(ProgramId, "codCol");
    cameraPos = glm::vec3(0.0f, 10.0f, 15.0f);
    cameraTarget = glm::vec3(-2.0f, 2.0f, 0.0f);


    platformProgram = LoadShaders("platform.vert", "platform.frag");
    platformTexture = loadTexture("textures/platform.jpg");
    glUseProgram(platformProgram);
    glUniform1i(glGetUniformLocation(platformProgram, "platformTexture"), 0);


    glUseProgram(ProgramId);
    glUniform1i(glGetUniformLocation(ProgramId, "texture1"), 0);
}

// ================= RENDER =================

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 100.0f);
    lightView = glm::lookAt(lightPos,
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;


    // =====================================================
    // 1. BACKGROUND SKYBOX
    // =====================================================

    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxProgram);

    glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(
        glGetUniformLocation(skyboxProgram, "view"),
        1, GL_FALSE, glm::value_ptr(viewNoTranslate)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(skyboxProgram, "projection"),
        1, GL_FALSE, glm::value_ptr(projection)
    );

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
    glDepthFunc(GL_LESS);



    // =====================================================
    // 2. CAMERA
    // =====================================================

    glm::vec3 blockWorldPos = gridToWorld(blockI, blockJ);
    blockWorldPos.y = 1.0f;

    cameraTarget = glm::mix(cameraTarget, blockWorldPos, cameraSmooth);

    float desiredDist = camDist;
    float desiredHeight = 8.0f;

    // reactie la stari
    if (gameState == FLOATING)
    {
        desiredDist = 10.0f;
        desiredHeight = 6.0f;
    }
    else if (gameState == EDGE_FALL)
    {
        desiredDist = 14.0f;
        desiredHeight = 10.0f;
    }

    cameraDesiredPos = blockWorldPos +
        glm::vec3(
            desiredDist * cos(camYaw),
            desiredHeight,
            desiredDist * sin(camYaw)
        );

    // interpolare
    cameraPos = glm::mix(cameraPos, cameraDesiredPos, cameraSmooth);

    view = glm::lookAt(
        cameraPos,
        cameraTarget,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    projection = glm::infinitePerspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f
    );



    // =====================================================
    // 3. PLATFORMA
    // =====================================================
    glUseProgram(platformProgram);
    glBindVertexArray(VaoId);

    glUniformMatrix4fv(glGetUniformLocation(ProgramId, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(ProgramId, "shadowMap"), 1);

    GLint modelLocP = glGetUniformLocation(platformProgram, "model");
    GLint viewLocP = glGetUniformLocation(platformProgram, "view");
    GLint projLocP = glGetUniformLocation(platformProgram, "projection");

    glUniformMatrix4fv(viewLocP, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocP, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(
        glGetUniformLocation(platformProgram, "lightPos"),
        1, glm::value_ptr(lightPos)
    );

    glUniform3fv(
        glGetUniformLocation(platformProgram, "viewPos"),
        1, glm::value_ptr(cameraPos)
    );

    glUniform3f(
        glGetUniformLocation(platformProgram, "lightColor"),
        1.0f, 1.0f, 1.0f
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, platformTexture);

    const Level& lvl = levels[currentLevel];

    for (int i = 0; i < lvl.rows; i++)
        for (int j = 0; j < lvl.cols; j++)
            if (lvl.map[i][j])
            {
                glm::mat4 model =
                    glm::translate(glm::mat4(1.0f),
                        glm::vec3(j - lvl.cols / 2.0f, 0.1f,
                            i - lvl.rows / 2.0f)) *
                    glm::scale(glm::mat4(1.0f),
                        glm::vec3(1.0f, 0.2f, 1.0f));

                glUniformMatrix4fv(modelLocP, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }




    // =====================================================
    // 4. BLOC + UMBRA
    // =====================================================
    glUseProgram(ProgramId);
    glBindVertexArray(VaoId);

    GLint modelLoc = glGetUniformLocation(ProgramId, "model");
    GLint viewLoc = glGetUniformLocation(ProgramId, "view");
    GLint projLoc = glGetUniformLocation(ProgramId, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(
        glGetUniformLocation(ProgramId, "lightPos"),
        1, glm::value_ptr(lightPos)
    );

    glUniform3fv(
        glGetUniformLocation(ProgramId, "viewPos"),
        1, glm::value_ptr(cameraPos)
    );

    glUniform3f(
        glGetUniformLocation(ProgramId, "lightColor"),
        1.0f, 1.0f, 1.0f
    );

    // ================= TARGET =================
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    glm::vec3 t = gridToWorld(goalI, goalJ);
    t.y = targetY;

    glm::mat4 targetModel =
        glm::translate(glm::mat4(1.0f), t) *
        glm::scale(glm::mat4(1.0f),
            glm::vec3(1.0f, 0.05f, 1.0f));


    glUniform1i(codColLocation, 2); // TARGET COLOR
    glUniformMatrix4fv(
        modelLoc, 1, GL_FALSE, glm::value_ptr(targetModel)
    );
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisable(GL_POLYGON_OFFSET_FILL);


    // ===== pozitie bloc =====
    BlockState stateToDraw = (gameState == ROLLING) ? renderState : blockState;

    float blockY =
        (stateToDraw == STANDING)
        ? PLATFORM_HEIGHT + BLOCK_HALF_STANDING
        : PLATFORM_HEIGHT + BLOCK_HALF_LYING;

    glm::vec3 pos = gridToWorld(blockI, blockJ);
    pos.y = blockY;

    if (stateToDraw == LYING_X) pos.x += 0.5f;
    if (stateToDraw == LYING_Z) pos.z += 0.5f;

    glm::vec3 scale(1.0f);
    if (stateToDraw == STANDING)
        scale = glm::vec3(1.0f, 2.0f, 1.0f);
    else if (stateToDraw == LYING_X)
        scale = glm::vec3(2.0f, 1.0f, 1.0f);
    else
        scale = glm::vec3(1.0f, 1.0f, 2.0f);

    glm::mat4 blockModel = glm::translate(glm::mat4(1.0f), pos);


    // ====== animatie rostogolire ======
    if (gameState == ROLLING)
    {
        float pivotY =
            (renderState == STANDING)
            ? -BLOCK_HALF_STANDING
            : -BLOCK_HALF_LYING;

        float edgeOffset = (renderState != STANDING && targetState == STANDING)
            ? 1.0f : 0.5f;

        glm::vec3 pivot;

        if (animAxis.x != 0)
            pivot = glm::vec3(0.0f, pivotY,
                animAxis.x > 0 ? edgeOffset : -edgeOffset);
        else
            pivot = glm::vec3(
                animAxis.z > 0 ? -edgeOffset : edgeOffset,
                pivotY, 0.0f);

        glm::quat q = glm::slerp(startRot, endRot, animT);

        blockModel =
            blockModel *
            glm::translate(glm::mat4(1.0f), pivot) *
            glm::toMat4(q) *
            glm::translate(glm::mat4(1.0f), -pivot);
    }

    // ===== plutire =====
    if (gameState == FLOATING)
    {

        blockModel =
            blockModel *
            glm::translate(glm::mat4(1.0f),
                glm::vec3(0.0f, floatY, 0.0f)) *
            glm::rotate(glm::mat4(1.0f),
                floatY,
                glm::vec3(0.0f, 1.0f, 0.0f));
    }


	// ===== cadere de pe platforma =====
    if (gameState == EDGE_FALL)
    {
        // directia ultimei mutari
        glm::vec3 rotAxis = glm::normalize(animAxis);

        // cade vertical
        blockModel =
            blockModel *
            glm::translate(glm::mat4(1.0f),
                glm::vec3(0.0f, floatY, 0.0f));

        // se roteste PE LOC
        blockModel =
            blockModel *
            glm::rotate(glm::mat4(1.0f),
                floatY,
                -rotAxis);
    }



    blockModel *= glm::scale(glm::mat4(1.0f), scale);


	// ================= SHADOW MAP =================
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(shadowProgram);
    GLint modelLocShadow = glGetUniformLocation(shadowProgram, "model");
    glUniformMatrix4fv(modelLocShadow, 1, GL_FALSE, glm::value_ptr(blockModel));
    glUniformMatrix4fv(glGetUniformLocation(shadowProgram, "lightSpaceMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glBindVertexArray(VaoId);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 800, 600);

    // ================= DESEN FINAL BLOC =================
    glUseProgram(ProgramId);
    glBindVertexArray(VaoId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(ProgramId, "shadowMap"), 0);

    glUniformMatrix4fv(glGetUniformLocation(ProgramId, "model"), 1, GL_FALSE, glm::value_ptr(blockModel));
    glUniformMatrix4fv(glGetUniformLocation(ProgramId, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(ProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(glGetUniformLocation(ProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(ProgramId, "viewPos"), 1, glm::value_ptr(cameraPos));
    glUniform3f(glGetUniformLocation(ProgramId, "lightColor"), 1.0f, 1.0f, 1.0f);

    glUniform1i(codColLocation, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // ================= GAME INFO =================
    glDisable(GL_DEPTH_TEST);
    glUseProgram(0);

    // proiectie 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // culoare text
    glColor3f(1.0f, 1.0f, 1.0f);

    // text dreapta sus
    char buffer[64];
    sprintf_s(buffer, "Level: %d  Moves: %d", currentLevel + 1, moveCount);
    drawText(620, 560, buffer);

    // restore
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);

    glFlush();
}

// ================= MAIN =================

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Bloxorz");
    glutTimerFunc(16, animate, 0);


    glewInit();

    Initialize();
	resetLevel();

    glutDisplayFunc(RenderFunction);
    glutKeyboardFunc(processNormalKeys);

    glutMainLoop();
}
