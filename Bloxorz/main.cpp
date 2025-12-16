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
int blockRow = 0;
int blockCol = 0;

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

void moveBlock(int dRow, int dCol)
{
    int newRow = blockRow + dRow;
    int newCol = blockCol + dCol;

    if (newRow < 0 || newRow >= ROWS ||
        newCol < 0 || newCol >= COLS)
        return;

    if (platform[newRow][newCol] == 0)
        return;

    blockRow = newRow;
    blockCol = newCol;
}

void processNormalKeys(unsigned char key, int x, int y)
{
    if (key == 27) exit(0);

    if (key == '+') dist += 1.0f;
    if (key == '-') dist -= 1.0f;

    if (key == 'w' && blockI > 0 && platform[blockI - 1][blockJ])
        blockI--;

    if (key == 's' && blockI < ROWS - 1 && platform[blockI + 1][blockJ])
        blockI++;

    if (key == 'a' && blockJ > 0 && platform[blockI][blockJ - 1])
        blockJ--;

    if (key == 'd' && blockJ < COLS - 1 && platform[blockI][blockJ + 1])
        blockJ++;

    glutPostRedisplay();
}




// ================= CUBOID =================

GLfloat cubeVertices[] =
{
    // TOP (y = +0.1)
    -0.5f, 0.1f,-0.5f, 1, 0.5f, 0.1f,-0.5f, 1, 0.5f, 0.1f, 0.5f, 1,
     0.5f, 0.1f, 0.5f, 1, -0.5f, 0.1f, 0.5f, 1, -0.5f, 0.1f,-0.5f, 1,

    // BOTTOM (y = -0.1)
    -0.5f,-0.1f,-0.5f, 1, 0.5f,-0.1f,-0.5f, 1, 0.5f,-0.1f, 0.5f, 1,
     0.5f,-0.1f, 0.5f, 1, -0.5f,-0.1f, 0.5f, 1, -0.5f,-0.1f,-0.5f, 1,

    // FRONT (z = +0.5)
    -0.5f,-0.1f, 0.5f, 1, 0.5f,-0.1f, 0.5f, 1, 0.5f, 0.1f, 0.5f, 1,
     0.5f, 0.1f, 0.5f, 1, -0.5f, 0.1f, 0.5f, 1, -0.5f,-0.1f, 0.5f, 1,

    // BACK (z = -0.5)
    -0.5f,-0.1f,-0.5f, 1, 0.5f,-0.1f,-0.5f, 1, 0.5f, 0.1f,-0.5f, 1,
     0.5f, 0.1f,-0.5f, 1, -0.5f, 0.1f,-0.5f, 1, -0.5f,-0.1f,-0.5f, 1,

    // RIGHT (x = +0.5)
     0.5f,-0.1f,-0.5f, 1, 0.5f,-0.1f, 0.5f, 1, 0.5f, 0.1f, 0.5f, 1,
     0.5f, 0.1f, 0.5f, 1, 0.5f, 0.1f,-0.5f, 1, 0.5f,-0.1f,-0.5f, 1,

     // LEFT (x = -0.5)
     -0.5f,-0.1f,-0.5f, 1, -0.5f,-0.1f, 0.5f, 1, -0.5f, 0.1f, 0.5f, 1,
     -0.5f, 0.1f, 0.5f, 1, -0.5f, 0.1f,-0.5f, 1, -0.5f,-0.1f,-0.5f, 1
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

    // camera fixa, vedere de sus + lateral
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

    for (int i = 0; i < 36; i++)
    {
        cubeColors[i * 4 + 0] = 0.4f;  // maro inchis
        cubeColors[i * 4 + 1] = 0.25f;
        cubeColors[i * 4 + 2] = 0.1f;
        cubeColors[i * 4 + 3] = 1.0f;
    }
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeColors), cubeColors);


    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            if (platform[i][j])
            {
                float x = j - 5.0f;
                float z = i - 5.0f;

                glm::mat4 model = glm::translate(
                    glm::mat4(1.0f),
                    glm::vec3(x, 0.0f, z)
                );

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

    // ================= BLOC =================
   
    // culoare diferita pentru bloc
    for (int i = 0; i < 36; i++)
    {
        cubeColors[i * 4 + 0] = 0.2f;
        cubeColors[i * 4 + 1] = 0.4f;
        cubeColors[i * 4 + 2] = 0.9f;
        cubeColors[i * 4 + 3] = 1.0f;
    }

    // trimitem noile culori la GPU
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeColors), cubeColors);

    float bx = blockJ - 5.0f;
    float bz = blockI - 5.0f;

    // scale corect pentru inaltime 2.0
    float scaleY = 10.0f;

    // ridicare corecta (jumatate din inaltimea finala)
    float liftY = (0.2f * scaleY) / 2.0f; // = 1.0

    glm::mat4 blockModel =
        glm::translate(glm::mat4(1.0f),
            glm::vec3(bx, liftY, bz)) *
        glm::scale(glm::mat4(1.0f),
            glm::vec3(1.0f, scaleY, 1.0f));

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

    glewInit();

    Initialize();

    glutDisplayFunc(RenderFunction);
    glutKeyboardFunc(processNormalKeys);

    glutMainLoop();
}
