// Cod sursa adaptat dupa OpenGLBook.com

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "loadShaders.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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


// matrice umbra
glm::mat4 shadowMatrix;
glm::vec3 lightPos(-25.0f, 70.0f, 5.0f);

// ================= BACKGROUND QUAD =================
float bgVertices[] = {
    // pos     // tex
    -1, -1,    0, 0,
     1, -1,    1, 0,
     1,  1,    1, 1,

     1,  1,    1, 1,
    -1,  1,    0, 1,
    -1, -1,    0, 0
};

float camYaw = glm::radians(120.0f);   // stanga-dreapta
float camPitch = glm::radians(35.0f); // sus-jos
float camDist = 12.0f;

// ================= PLATFORM LAYOUT =================
const int ROWS = 6;
const int COLS = 10;
const float PLATFORM_HEIGHT = 0.2f;
const float BLOCK_HALF_STANDING = 1.0f;
const float BLOCK_HALF_LYING = 0.5f;


int platform[ROWS][COLS] =
{
    {1,1,1,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1},
	{0,0,0,0,0,1,1,1,1,1},
	{0,0,0,0,0,0,1,1,1,0}
};

// ================= BLOCK =================
enum BlockState {
    STANDING,   // vertical (ocupa 1 celula)
    LYING_X,    // culcat pe axa X (ocupa 2 celule pe coloana)
    LYING_Z     // culcat pe axa Z (ocupa 2 celule pe rand)
};

BlockState blockState = STANDING;
BlockState renderState;
glm::vec3 startPos;
glm::vec3 endPos;


// ============== ANIMATION ==============
bool isAnimating = false;
float animAngle = 0.0f;
float animSpeed = 3.0f;   // grade / frame

glm::vec3 animAxis;       // axa de rotatie

float startY = 0.0f;
float endY = 0.0f;

// target
int targetI, targetJ;
BlockState targetState;


float PI = 3.141592f;
float Refx = 0, Refy = 0, Refz = 0;
float alpha = PI / 6.0f; 
float beta = PI / 4.0f;
float dist = 15.0f;    
// pozitia blocului pe grid
int blockI = 1;
int blockJ = 1;

float Obsx, Obsy, Obsz;
float Vx = 0, Vy = 0, Vz = 1;

glm::mat4 view, projection, myMatrix;


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


bool isInside(int i, int j) {
    return i >= 0 && i < ROWS && j >= 0 && j < COLS;
}

bool isValidPosition(int i, int j, BlockState state) {
    if (state == STANDING) {
        return isInside(i, j) && platform[i][j];
    }

    if (state == LYING_X) {
        return isInside(i, j) &&
            isInside(i, j + 1) &&
            platform[i][j] &&
            platform[i][j + 1];
    }

    // LYING_Z
    return isInside(i, j) &&
        isInside(i + 1, j) &&
        platform[i][j] &&
        platform[i + 1][j];
}


void checkGameOver() {
    if (!isValidPosition(blockI, blockJ, blockState)) {
        printf("GAME OVER\n");
        blockI = 0;
        blockJ = 0;
        blockState = STANDING;
    }
}

void startMoveUp() {
    if (isAnimating) return;
    animAxis = glm::vec3(-1, 0, 0);

    if (blockState == STANDING) {
        if (!isValidPosition(blockI - 2, blockJ, LYING_Z)) return;

        targetI = blockI - 2;
        targetJ = blockJ;
        targetState = LYING_Z;
    }
    else if (blockState == LYING_Z) {
        // ridicare
        if (!isValidPosition(blockI - 1, blockJ, STANDING)) return;

        targetI = blockI - 1;
        targetJ = blockJ;
        targetState = STANDING;
    }
    else if (blockState == LYING_X) {
        // mutare culcat
        if (!isValidPosition(blockI - 1, blockJ, LYING_X)) return;

        targetI = blockI - 1;
        targetJ = blockJ;
        targetState = LYING_X;
    }
    else return;

    // pozitia de start
    startPos = glm::vec3(blockJ - 5.0f, 0.0f, blockI - 5.0f);
    if (blockState == LYING_X) startPos.x += 0.5f;
    if (blockState == LYING_Z) startPos.z += 0.5f;

    // pozitia de final
    endPos = glm::vec3(targetJ - 5.0f, 0.0f, targetI - 5.0f);
    if (targetState == LYING_X) endPos.x += 0.5f;
    if (targetState == LYING_Z) endPos.z += 0.5f;

    animAngle = 0.0f;
    renderState = blockState;
    isAnimating = true;
}

void startMoveDown() {
    if (isAnimating) return;
    animAxis = glm::vec3(1, 0, 0);

    if (blockState == STANDING) {
        if (!isValidPosition(blockI + 1, blockJ, LYING_Z)) return;

        targetI = blockI + 1;
        targetJ = blockJ;
        targetState = LYING_Z;
    }
    else if (blockState == LYING_Z) {
        // ridicare
        if (!isValidPosition(blockI + 2, blockJ, STANDING)) return;

        targetI = blockI + 2;
        targetJ = blockJ;
        targetState = STANDING;
    }
    else if (blockState == LYING_X) {
        // mutare culcat
        if (!isValidPosition(blockI + 1, blockJ, LYING_X)) return;

        targetI = blockI + 1;
        targetJ = blockJ;
        targetState = LYING_X;
    }
    else return;

    // pozitia de start
    startPos = glm::vec3(blockJ - 5.0f, 0.0f, blockI - 5.0f);
    if (blockState == LYING_X) startPos.x += 0.5f;
    if (blockState == LYING_Z) startPos.z += 0.5f;

    // pozitia de final
    endPos = glm::vec3(targetJ - 5.0f, 0.0f, targetI - 5.0f);
    if (targetState == LYING_X) endPos.x += 0.5f;
    if (targetState == LYING_Z) endPos.z += 0.5f;

    animAngle = 0.0f;
    renderState = blockState;
    isAnimating = true;
}

void startMoveLeft() {
    if (isAnimating) return;
    animAxis = glm::vec3(0, 0, 1);

    if (blockState == STANDING) {
        if (!isValidPosition(blockI, blockJ - 2, LYING_X)) return;

        targetI = blockI;
        targetJ = blockJ - 2;
        targetState = LYING_X;
    }
    else if (blockState == LYING_X) {
        // ridicare
        if (!isValidPosition(blockI, blockJ - 1, STANDING)) return;

        targetI = blockI;
        targetJ = blockJ - 1;
        targetState = STANDING;
    }
    else if (blockState == LYING_Z) {
        // mutare culcat
        if (!isValidPosition(blockI, blockJ - 1, LYING_Z)) return;

        targetI = blockI;
        targetJ = blockJ - 1;
        targetState = LYING_Z;
    }
    else return;

    // pozitia de start
    startPos = glm::vec3(blockJ - 5.0f, 0.0f, blockI - 5.0f);
    if (blockState == LYING_X) startPos.x += 0.5f;
    if (blockState == LYING_Z) startPos.z += 0.5f;

    // pozitia de final
    endPos = glm::vec3(targetJ - 5.0f, 0.0f, targetI - 5.0f);
    if (targetState == LYING_X) endPos.x += 0.5f;
    if (targetState == LYING_Z) endPos.z += 0.5f;


    animAngle = 0.0f;
    renderState = blockState;
    isAnimating = true;
}

void startMoveRight() {
    if (isAnimating) return;
    animAxis = glm::vec3(0, 0, -1);

    if (blockState == STANDING) {
        if (!isValidPosition(blockI, blockJ + 1, LYING_X)) return;

        targetI = blockI;
        targetJ = blockJ + 1;
        targetState = LYING_X;
    }
    else if (blockState == LYING_X) {
        // ridicare
        if (!isValidPosition(blockI, blockJ + 2, STANDING)) return;

        targetI = blockI;
        targetJ = blockJ + 2;
        targetState = STANDING;
    }
    else if (blockState == LYING_Z) {
        // mutare culcat
        if (!isValidPosition(blockI, blockJ + 1, LYING_Z)) return;

        targetI = blockI;
        targetJ = blockJ + 1;
        targetState = LYING_Z;
    }
    else return;

    // pozitia de start
    startPos = glm::vec3(blockJ - 5.0f, 0.0f, blockI - 5.0f);
    if (blockState == LYING_X) startPos.x += 0.5f;
    if (blockState == LYING_Z) startPos.z += 0.5f;

    // pozitia de final
    endPos = glm::vec3(targetJ - 5.0f, 0.0f, targetI - 5.0f);
    if (targetState == LYING_X) endPos.x += 0.5f;
    if (targetState == LYING_Z) endPos.z += 0.5f;

    animAngle = 0.0f;
    renderState = blockState;
    isAnimating = true;
}

void animationTimer(int value) {
    if (isAnimating) {
        animAngle += animSpeed;

        if (animAngle >= 90.0f) {
            animAngle = 90.0f;
            isAnimating = false;

            blockI = targetI;
            blockJ = targetJ;
            blockState = targetState;
            renderState = blockState;

			checkGameOver();
        }
    }

    glutPostRedisplay();
    glutTimerFunc(10, animationTimer, 0);
}

void processNormalKeys(unsigned char key, int x, int y)
{
    if (key == 27) exit(0);

    if (isAnimating) return;

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

void CreateBackgroundQuad()
{
    glGenVertexArrays(1, &bgVao);
    glGenBuffers(1, &bgVbo);

    glBindVertexArray(bgVao);
    glBindBuffer(GL_ARRAY_BUFFER, bgVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices),
        bgVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float),
        (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}


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
        cubeColors[i * 4 + 0] = 0.45f;
        cubeColors[i * 4 + 1] = 0.25f;
        cubeColors[i * 4 + 2] = 0.10f;
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

void Initialize(void)
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    CreateVBO();
    CreateShaders();
    codColLocation = glGetUniformLocation(ProgramId, "codCol");

    bgProgramId = LoadShaders("bg.vert", "bg.frag");
    CreateBackgroundQuad();
    bgTexture = loadTexture("textures/bg.jpg");
    glUseProgram(bgProgramId);
    glUniform1i(glGetUniformLocation(bgProgramId, "bgTexture"), 0);

    platformProgram = LoadShaders("platform.vert", "platform.frag");
    platformTexture = loadTexture("textures/platform.png");
    glUseProgram(platformProgram);
    glUniform1i(glGetUniformLocation(platformProgram, "platformTexture"), 0);


    glUseProgram(ProgramId);
    glUniform1i(glGetUniformLocation(ProgramId, "texture1"), 0);
}

// ================= RENDER =================

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // =====================================================
    // 1. BACKGROUND
    // =====================================================
    glDisable(GL_DEPTH_TEST);
    glUseProgram(bgProgramId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bgTexture);

    glBindVertexArray(bgVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);



    // =====================================================
    // 2. CAMERA (o calculăm O SINGURĂ DATĂ)
    // =====================================================
    float cx = camDist * cos(camPitch) * cos(camYaw);
    float cy = camDist * sin(camPitch);
    float cz = camDist * cos(camPitch) * sin(camYaw);

    glm::vec3 cameraPos(cx, cy, cz);

    view = glm::lookAt(
        cameraPos,
        glm::vec3(-2.0f, 2.0f, 0.0f),   // centrul platformei
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    projection = glm::infinitePerspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f
    );



    // =====================================================
    // 3. PLATFORMA (SHADER SEPARAT)
    // =====================================================
    glUseProgram(platformProgram);
    glBindVertexArray(VaoId);

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

    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            if (platform[i][j])
            {
                glm::mat4 model =
                    glm::translate(glm::mat4(1.0f),
                        glm::vec3(j - 5.0f, 0.1f, i - 5.0f)) *
                    glm::scale(glm::mat4(1.0f),
                        glm::vec3(1.0f, 0.2f, 1.0f));

                glUniformMatrix4fv(
                    modelLocP, 1, GL_FALSE, glm::value_ptr(model)
                );

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }



    // =====================================================
    // 4. BLOC + UMBRĂ (SHADER VECHI)
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



    // ===== calcul matrice umbră =====
    glm::vec4 plane(0.0f, 1.0f, 0.0f, -PLATFORM_HEIGHT);
    glm::vec4 light(lightPos, 1.0f);
    shadowMatrix = BuildShadowMatrix(plane, light);



    // ===== poziție bloc =====
    BlockState stateToDraw = isAnimating ? renderState : blockState;

    float blockY =
        (stateToDraw == STANDING)
        ? PLATFORM_HEIGHT + BLOCK_HALF_STANDING
        : PLATFORM_HEIGHT + BLOCK_HALF_LYING;

    glm::vec3 pos(blockJ - 5.0f, blockY, blockI - 5.0f);

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



    // ===== animație =====
    if (isAnimating)
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

        blockModel =
            blockModel *
            glm::translate(glm::mat4(1.0f), pivot) *
            glm::rotate(glm::mat4(1.0f),
                glm::radians(animAngle),
                animAxis) *
            glm::translate(glm::mat4(1.0f), -pivot);
    }

    blockModel *= glm::scale(glm::mat4(1.0f), scale);



    // ===== BLOC =====
    glUseProgram(ProgramId);
    glBindVertexArray(VaoId);

    glUniform1i(codColLocation, 0);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(blockModel));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // ===== UMBRA BLOC =====
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    glUniform1i(codColLocation, 1);
    glUniformMatrix4fv(
        glGetUniformLocation(ProgramId, "shadowMatrix"),
        1, GL_FALSE, glm::value_ptr(shadowMatrix)
    );
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(blockModel));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisable(GL_POLYGON_OFFSET_FILL);


    glFlush();
}

// ================= MAIN =================

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Platforma 3D - Bloxorz");
    glutTimerFunc(16, animationTimer, 0);


    glewInit();

    Initialize();

    glutDisplayFunc(RenderFunction);
    glutKeyboardFunc(processNormalKeys);

    glutMainLoop();
}
