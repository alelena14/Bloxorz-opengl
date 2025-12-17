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

GLuint VaoId, VboId, ColorBufferId, ProgramId;

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

int blockRow = 0;
int blockCol = 0;

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
int blockI = 0;
int blockJ = 0;

float Obsx, Obsy, Obsz;
float Vx = 0, Vy = 0, Vz = 1;

glm::mat4 view, projection, myMatrix;


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
    glutTimerFunc(16, animationTimer, 0);
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



GLfloat cubeColors[36 * 4];

// ================= OPENGL =================

void CreateVBO(void)
{
    for (int i = 0; i < 36; i++)
    {
        cubeColors[i * 4 + 0] = 0.45f; // R
        cubeColors[i * 4 + 1] = 0.25f; // G
        cubeColors[i * 4 + 2] = 0.10f; // B

        cubeColors[i * 4 + 3] = 1.0f;
    }

    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &VboId);
    glBindBuffer(GL_ARRAY_BUFFER, VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &ColorBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
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
}

// ================= RENDER =================

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VaoId);

    // ===== CAMERA =====
    Obsx = 8.0f;
    Obsy = 8.0f;
    Obsz = 8.0f;

    view = glm::lookAt(
        glm::vec3(Obsx, Obsy, Obsz),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    projection = glm::infinitePerspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f
    );

    GLint modelLoc = glGetUniformLocation(ProgramId, "model");
    GLint viewLoc = glGetUniformLocation(ProgramId, "view");
    GLint projLoc = glGetUniformLocation(ProgramId, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // ===== PLATFORMA =====
    for (int i = 0; i < 36; i++) {
        cubeColors[i * 4 + 0] = 0.4f;
        cubeColors[i * 4 + 1] = 0.25f;
        cubeColors[i * 4 + 2] = 0.1f;
        cubeColors[i * 4 + 3] = 1.0f;
    }

    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeColors), cubeColors);

    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            if (platform[i][j]) {
                float x = j - 5.0f;
                float z = i - 5.0f;

                glm::mat4 model =
                    glm::translate(glm::mat4(1.0f),
                        glm::vec3(x, 0.1f, z)) *
                    glm::scale(glm::mat4(1.0f),
                        glm::vec3(1.0f, 0.2f, 1.0f));


                glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
                    glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

    // ===== BLOC =====
    for (int i = 0; i < 36; i++) {
        cubeColors[i * 4 + 0] = 0.2f;
        cubeColors[i * 4 + 1] = 0.4f;
        cubeColors[i * 4 + 2] = 0.9f;
        cubeColors[i * 4 + 3] = 1.0f;
    }

    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeColors), cubeColors);

    // stare de randare
    BlockState stateToDraw = isAnimating ? renderState : blockState;

    // ===== pozizie Y =====
    float blockY =
        (stateToDraw == STANDING)
        ? PLATFORM_HEIGHT + BLOCK_HALF_STANDING
        : PLATFORM_HEIGHT + BLOCK_HALF_LYING;

    // pozitie de baza
    glm::vec3 pos(blockJ - 5.0f, blockY, blockI - 5.0f);

    if (stateToDraw == LYING_X) pos.x += 0.5f;
    if (stateToDraw == LYING_Z) pos.z += 0.5f;

    // ===== scale =====
    glm::vec3 scale(1.0f);

    if (stateToDraw == STANDING)
        scale = glm::vec3(1.0f, 2.0f, 1.0f);
    else if (stateToDraw == LYING_X)
        scale = glm::vec3(2.0f, 1.0f, 1.0f);
    else if (stateToDraw == LYING_Z)
        scale = glm::vec3(1.0f, 1.0f, 2.0f);

    // ===== model matrix =====
    glm::mat4 blockModel = glm::translate(glm::mat4(1.0f), pos);

    // ===== rotatie cu pivot =====
    if (isAnimating)
    {
        glm::vec3 pivot(0.0f);

        // pivot pe muchia care atinge platforma
        float pivotY =
            (renderState == STANDING)
            ? -BLOCK_HALF_STANDING
            : -BLOCK_HALF_LYING;

        float edgeOffset = 0.5f;

        if (renderState != STANDING && targetState == STANDING)
            edgeOffset = 1.0f;

        if (animAxis.x != 0)   // W / S
            pivot = glm::vec3(0.0f, pivotY, animAxis.x > 0 ? edgeOffset : -edgeOffset);
        else                  // A / D
            pivot = glm::vec3( animAxis.z > 0 ? -edgeOffset : edgeOffset, pivotY, 0.0f);
       

        blockModel =
            blockModel *
            glm::translate(glm::mat4(1.0f), pivot) *
            glm::rotate(glm::mat4(1.0f),
                glm::radians(animAngle),
                animAxis) *
            glm::translate(glm::mat4(1.0f), -pivot);
    }

    blockModel *= glm::scale(glm::mat4(1.0f), scale);

    // ===== draw =====
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(blockModel));
    glDrawArrays(GL_TRIANGLES, 0, 36);


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
