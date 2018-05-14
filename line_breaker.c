#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "rain.h"
#include "utils.h"

#define MAX_PARTICLES 2000
#define WCX 640
#define WCY 480
#define LINE_COUNT 64

int blinks = 0;

const float line_size = 0.002;
float line_slowdown = 0.0009f;
const float line_speed_incr = 0.000009f;
float left_bound = -0.314f;
float right_bound = 0.324f;
float rocket_move_speed = 0.001f;
bool move_left = false;
bool move_right = false;

float rtl = 0.04f; // 0.003f;
float ltr = -0.042f;

// scoring
int cur_score = 0;
int hi_score = -1;
int _score_update_frame = 0;

// flags
bool in_game = false;
bool paused = false;

typedef struct
{
    float trans_x;
    float trans_y;
    int light_alt;
    float speed;
    bool through;
} rocket;

typedef struct
{
    bool breakable;
    float start_y;
    float end_y;
    float color_r;
    float color_g;
    float color_b;
    float color_a;
} line;

rocket _rocket;
line lines[LINE_COUNT];
GLUquadric *qobj;
float slowdown = 2.0;
float velocity = 0.0;
float zoom = -13.0;
// float pan = 0.0;
// float tilt = 0.0;

int loop;
int fall;

// Paticle System
particles par_sys[MAX_PARTICLES];

float get_random_height(float LO, float HI)
{
    return LO + (float)(rand()) / ((float)(RAND_MAX / (HI - LO)));
}

void special_keys(int key, int x, int y)
{
    if (key == GLUT_KEY_UP)
    {
        // zoom += 1.0;
        //_rocket.trans_y += 0.001f;
    }
    if (key == GLUT_KEY_DOWN)
    {
        //_rocket.trans_y -= 0.001f;
        // zoom -= 1.0;
    }
    if (key == GLUT_KEY_RIGHT)
    {
        move_right = true;
        //_rocket.trans_x += 0.003f;
    }
    if (key == GLUT_KEY_LEFT)
    {
        move_left = true;
        //_rocket.trans_x -= 0.003f;
    }
}

void initLines()
{
    float trail = LINE_COUNT * 0.01f; // height to fill
    int alt = 0;
    for (int i = 0; i < LINE_COUNT; ++i)
    {
        lines[i].start_y = trail;
        if (alt == 0)
        {
            alt = 1;
            trail -= get_random_height(0.01f, 0.02f);
            lines[i].breakable = true;
            lines[i].color_a = 0.2f;
            lines[i].color_r = 1.0f;
            lines[i].color_g = 1.0f;
            lines[i].color_b = 1.0f;
        }
        else
        {
            alt = 0;
            trail -= get_random_height(0.02f, 0.035f);
            lines[i].breakable = false;
            lines[i].color_a = 1.0f;
            lines[i].color_r = 0.94f;
            lines[i].color_g = 0.2f;
            lines[i].color_b = 0.32f;
        }
        lines[i].end_y = trail;
    }
}

void initParticles(int i)
{
    par_sys[i].alive = true;
    par_sys[i].life = 1.0;
    par_sys[i].fade = (float)(rand() % 100) / 2000.0f + 0.003f;

    par_sys[i].xpos = (float)(rand() % 51) - 10;
    par_sys[i].ypos = 10.0;
    par_sys[i].zpos = (float)(rand() % 41) - 10;

    par_sys[i].red = 0.5;
    par_sys[i].green = 0.5;
    par_sys[i].blue = 1.0;

    par_sys[i].vel = velocity;
    par_sys[i].gravity = -0.8;
}

void gl_init()
{
    int x, z;

    glShadeModel(GL_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    // glClearColor(0.231, 0.192, 0.463, 0.6);
    glClearColor(0.12, 0.23, 0.32, 0.2);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    qobj = gluNewQuadric();
    gluQuadricNormals(qobj, GLU_SMOOTH);

    // Initialize particles
    for (loop = 0; loop < MAX_PARTICLES; loop++)
    {
        initParticles(loop);
    }
    initLines();
}

void draw_cover()
{
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.7f);

    glPushMatrix();
    glTranslatef(0.25f, 0.0f, 0.0f);

    glPushMatrix(); // L
    glBegin(GL_POLYGON);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.45, 0.09, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.45, 0.25, 0.0);
    glVertex3f(-0.42, 0.25, 0.0);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.42, 0.12, 0.0);
    glVertex3f(-0.38, 0.12, 0.0);
    glVertex3f(-0.38, 0.09, 0.0);
    glEnd();
    glPopMatrix();

    glPushMatrix(); // I
    glBegin(GL_POLYGON);
    glColor3f(0.9, 0.0, 0.0);
    glVertex3f(-0.37, 0.25, 0.0);
    glVertex3f(-0.34, 0.25, 0.0);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.34, 0.09, 0.0);
    glVertex3f(-0.37, 0.09, 0.0);
    glEnd();
    glPopMatrix();

    glPushMatrix(); // N
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.33, 0.25, 0.0);
    glVertex3f(-0.30, 0.25, 0.0);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.30, 0.09, 0.0);
    glVertex3f(-0.33, 0.09, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.27, 0.25, 0.0);
    glVertex3f(-0.24, 0.25, 0.0);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.24, 0.09, 0.0);
    glVertex3f(-0.27, 0.09, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.30, 0.25, 0.0);
    glVertex3f(-0.30, 0.20, 0.0);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.27, 0.09, 0.0);
    glVertex3f(-0.27, 0.15, 0.0);
    glEnd();
    glPopMatrix();

    glPushMatrix(); // E
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.23, 0.25, 0.0);
    glVertex3f(-0.20, 0.25, 0.0);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.20, 0.09, 0.0);
    glVertex3f(-0.23, 0.09, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.20, 0.25, 0.0);
    glVertex3f(-0.16, 0.25, 0.0);
    glVertex3f(-0.16, 0.22, 0.0);
    glVertex3f(-0.20, 0.22, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.20, 0.18, 0.0);
    glVertex3f(-0.18, 0.18, 0.0);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.18, 0.15, 0.0);
    glVertex3f(-0.23, 0.15, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.20, 0.12, 0.0);
    glVertex3f(-0.16, 0.12, 0.0);
    glVertex3f(-0.16, 0.09, 0.0);
    glVertex3f(-0.23, 0.09, 0.0);
    glEnd();
    glPopMatrix();

    glPushMatrix(); // S
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.15, 0.25, 0.0);
    glVertex3f(-0.06, 0.25, 0.0);
    glVertex3f(-0.06, 0.22, 0.0);
    glVertex3f(-0.15, 0.22, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.15, 0.22, 0.0);
    glVertex3f(-0.13, 0.22, 0.0);
    glVertex3f(-0.13, 0.19, 0.0);
    glVertex3f(-0.15, 0.19, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.15, 0.19, 0.0);
    glVertex3f(-0.06, 0.19, 0.0);
    glVertex3f(-0.06, 0.17, 0.0);
    glVertex3f(-0.15, 0.17, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(-0.06, 0.17, 0.0);
    glVertex3f(-0.08, 0.17, 0.0);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.08, 0.12, 0.0);
    glVertex3f(-0.06, 0.12, 0.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.6, 0.42, 0.6);
    glVertex3f(-0.15, 0.12, 0.0);
    glVertex3f(-0.06, 0.12, 0.0);
    glVertex3f(-0.06, 0.09, 0.0);
    glVertex3f(-0.15, 0.09, 0.0);
    glEnd();
    glPopMatrix();

    glPopMatrix();

    glPushMatrix();          // back yellow
    glColor3f(.7, 1.0, 0.0); //(R,G,B)
    glBegin(GL_POLYGON);
    glVertex3f(-0.5, 0.27, 0.0);
    glVertex3f(0.5, 0.27, 0.0);
    glVertex3f(0.5, 0.07, 0.0);
    glVertex3f(-0.5, 0.07, 0.0);
    glEnd();
    glPopMatrix();

    glPushMatrix(); // text
    int l, i;
    char *st = "***PRESS ENTER TO PLAY***";
    l = strlen(st); // see how many characters are in text string.

    glColor3f(.7, 1.0, 0.0);      //(R,G,B)
    glRasterPos2f(-0.42, -0.115); // location to start printing text
    for (i = 0; i < l; i++)       // loop until i is greater then l
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, st[i]); // Print a character on the screen
    }
    glPopMatrix();

    glPushMatrix();           // back red2
    glColor3f(0.0, 0.0, 1.0); //(R,G,B)
    glBegin(GL_POLYGON);
    glVertex3f(-0.44, -0.08, 0.0);
    glVertex3f(-0.12, -0.08, 0.0);
    glVertex3f(-0.12, -0.14, 0.0);
    glVertex3f(-0.44, -0.14, 0.0);
    glEnd();
    glPopMatrix();

    glPushMatrix();           // back red
    glColor3f(1.0, 0.0, 0.0); //(R,G,B)
    glBegin(GL_POLYGON);
    glVertex3f(-0.45, -0.07, 0.0);
    glVertex3f(-0.11, -0.07, 0.0);
    glVertex3f(-0.11, -0.15, 0.0);
    glVertex3f(-0.45, -0.15, 0.0);
    glEnd();
    glPopMatrix();

    glPushMatrix(); // text
    int m, n, o, k, j;
    char *st_ = "INSTRUCTIONS:";
    char *st1 = "Control the rocket with LEFT arrow & RIGHT arrow.";
    char *st2 = "Avoid collision with the RED parts. You can stay on the same";
    char *st3 = "side upto 4 pulse. Press Q to EXIT, R to restart & P to pause/unpause.";
    k = strlen(st_); // see how many characters are in text string.
    m = strlen(st1); // see how many characters are in text string.
    n = strlen(st2); // see how many characters are in text string.
    o = strlen(st3); // see how many characters are in text string.

    glColor3f(0.0, 0.0, 1.0);     //(R,G,B)
    glRasterPos2f(-0.015, -0.03); // location to start printing text
    for (j = 0; j < k; j++)       // loop until i is greater then j
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, st_[j]); // Print a character on the screen
    }
    glColor3f(0.0, 0.0, 1.0);     //(R,G,B)
    glRasterPos2f(-0.015, -0.05); // location to start printing text
    for (j = 0; j < m; j++)       // loop until i is greater then j
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, st1[j]); // Print a character on the screen
    }
    glColor3f(0.0, 0.0, 1.0);     //(R,G,B)
    glRasterPos2f(-0.015, -0.07); // location to start printing text
    for (j = 0; j < n; j++)       // loop until i is greater then j
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, st2[j]); // Print a character on the screen
    }
    glColor3f(0.0, 0.0, 1.0);     //(R,G,B)
    glRasterPos2f(-0.015, -0.09); // location to start printing text
    for (j = 0; j < o; j++)       // loop until i is greater then j
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, st3[j]); // Print a character on the screen
    }
    glPopMatrix();
    // glPopMatrix();

    glPushMatrix();           // back yellow2
    glColor3f(1.0, 1.0, 0.0); //(R,G,B)
    glBegin(GL_POLYGON);
    glVertex3f(-0.035, 0.0, 0.0);
    glVertex3f(0.455, 0.0, 0.0);
    glVertex3f(0.455, -0.24, 0.0);
    glVertex3f(-0.035, -0.24, 0.0);
    glEnd();
    glPopMatrix();

    glPushMatrix();           // back yellow
    glColor3f(0.0, 1.0, 1.0); //(R,G,B)
    glBegin(GL_POLYGON);
    glVertex3f(-0.045, 0.01, 0.0);
    glVertex3f(0.465, 0.01, 0.0);
    glVertex3f(0.465, -0.25, 0.0);
    glVertex3f(-0.045, -0.25, 0.0);
    glEnd();
    glPopMatrix();

    glPopMatrix();
}

void drawLines()
{
    glPushMatrix();
    for (int i = 0; i < LINE_COUNT; ++i)
    {
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.1f);
        glColor4f(lines[i].color_r, lines[i].color_g, lines[i].color_b, lines[i].color_a);
        glLineWidth(15);
        glBegin(GL_LINES);
        glVertex3f(0.0f, lines[i].start_y, 0.0f);
        glVertex3f(0.0f, lines[i].end_y, 0.0f);
        glEnd();
        glPopMatrix();
    }
    glPopMatrix();
}

// For Rain
void drawRain()
{
    glPushMatrix();
    glTranslatef(0, 0, 0.02);
    glRotatef(-3, 0.0f, 0.0f, 1.0f);
    glScalef(3.0f, 3.0f, 3.0f);

    float x, y, z;

    for (loop = 0; loop < MAX_PARTICLES; loop += 1)
    {
        if (par_sys[loop].alive == true)
        {
            x = par_sys[loop].xpos;
            y = par_sys[loop].ypos;
            z = par_sys[loop].zpos + zoom;

            // Draw particles
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glLineWidth(2.5f);
            glBegin(GL_LINES);

            glVertex3f(x, y, z);
            glVertex3f(x, y + 0.5, z);
            glEnd();

            // Update values
            // Move
            // Adjust slowdown for speed!
            par_sys[loop].ypos += par_sys[loop].vel / (slowdown * 1000);
            par_sys[loop].vel += par_sys[loop].gravity;
            // Decay
            par_sys[loop].life -= par_sys[loop].fade;

            if (par_sys[loop].ypos <= -10)
            {
                par_sys[loop].life = -1.0;
            }
            // Revive
            if (par_sys[loop].life < 0.0)
            {
                initParticles(loop);
            }
        }
    }
    glPopMatrix();
}

void drawRocket()
{
    glPushMatrix();
    glTranslatef(_rocket.trans_x, 0, -0.5);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 0.2f, _rocket.light_alt <= 35 ? 0.2f : 1.0f);
    gluCylinder(qobj, 0.005, 0.005, 0.038, 32, 32);
    glPopMatrix();
}

void drawBackground()
{
    glPushMatrix();
    glTranslatef(0, 0, -0.5);
    glScalef(1.0f, 5.0f, 1.0f);
    glBegin(GL_QUADS);
    // red color
    glColor4f(0.28f, 0.19f, 0.47f, 0.9);
    glVertex3f(-1.0, 0.043, 0);
    // glColor3f(0.19f, 0.09f, 0.38f);
    glColor4f(0.0f, 0.09f, 0.28f, 0.9);
    glVertex3f(1.0, 0.043, 0);
    // blue color
    glColor4f(0.60f, 0.42f, 0.60f, 0.9);
    glVertex3f(1.0, -0.043, 0);
    glVertex3f(-1.0, -0.043, 0);
    glEnd();
    glPopMatrix();
}

void drawScore()
{
    glPushMatrix();
    glTranslatef(-0.07, 0.038, -0.1);
    glColor4f(1, 1, 1, 1.0f);
    glRasterPos2i(0, 0); // location to start printing text
    char *score_txt = "SCORE: ";
    int i;
    for (i = 0; i < strlen(score_txt); i++) // loop until i is greater then l
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score_txt[i]); // Print a character on the screen
    }
    char score_str[100];
    itoa(cur_score, score_str);
    for (i = 0; i < strlen(score_str); i++) // loop until i is greater then l
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score_str[i]); // Print a character on the screen
    }
    glPopMatrix();
}

void drawHiScore()
{
    glPushMatrix();
    glTranslatef(-0.07, 0.035, -0.1);
    glColor4f(1, 1, 1, 1.0f);
    glRasterPos2i(0, 0); // location to start printing text
    char *score_txt = "HI SCORE: ";
    int i;
    for (i = 0; i < strlen(score_txt); i++) // loop until i is greater then l
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score_txt[i]); // Print a character on the screen
    }
    char score_str[100];
    if (hi_score > 0)
    {
        itoa(hi_score, score_str);
        for (i = 0; i < strlen(score_str); i++) // loop until i is greater then l
        {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score_str[i]); // Print a character on the screen
        }
    }
    else
    {
        score_str[0] = 'N';
        score_str[1] = 'I';
        score_str[2] = 'L';
        score_str[3] = '\0';
        for (i = 0; i < strlen(score_str); i++) // loop until i is greater then l
        {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score_str[i]); // Print a character on the screen
        }
    }
    glPopMatrix();
}

// Draw Particles
void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    if (!in_game)
    {
        draw_cover();
    }
    else
    {
        drawRain();
        drawBackground();
        drawLines();
        drawRocket();
        drawScore();
        drawHiScore();
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0)
        h = 1;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45, (float)w / (float)h, .1, 200);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void update(int value)
{
    if (!in_game)
    {
        glutTimerFunc(15, update, 0);
        return;
    }
    if (!paused)
    {
        _score_update_frame = ++_score_update_frame % 30;
        if (_score_update_frame == 15)
        {
            cur_score++;
        }
        bool _can_go = false;
        _rocket.light_alt = ++_rocket.light_alt % 70;
        if (_rocket.light_alt == 36)
        {
            blinks++;
        }
        if (blinks > 4)
        {
            return;
        }
        float _max_start_y = 0.0000001f;

        int change_index = -1;
        for (int i = 0; i < LINE_COUNT; ++i)
        {
            if (lines[i].breakable)
            {
                if ((-0.0008 >= lines[i].end_y) && (0.0008 <= lines[i].start_y))
                {
                    _can_go = true;
                }
            }
            lines[i].start_y -= line_slowdown;
            lines[i].end_y -= line_slowdown;
            if (lines[i].start_y <= -0.039f)
            {
                change_index = i;
            }
            if (_max_start_y < lines[i].start_y)
            {
                _max_start_y = lines[i].start_y;
            }
        }
        if (change_index >= 0)
        {
            float _height;
            lines[change_index].end_y = _max_start_y;
            if (lines[change_index].breakable)
            {
                _height = get_random_height(0.01f, 0.02f);
            }
            else
            {
                _height = get_random_height(0.02f, 0.035f);
            }
            lines[change_index].start_y = _max_start_y + _height;
        }
        bool _r = false;

        // rocket movement
        if (move_right)
        {
            if (_rocket.trans_x + _rocket.speed >= ltr && !_rocket.through)
            {
                if (_can_go)
                {
                    _rocket.through = true;
                }
                else
                {
                    _r = true;
                    goto _END;
                }
            }
            if (_rocket.trans_x + _rocket.speed <= right_bound)
            {
                _rocket.trans_x += _rocket.speed;
            }
            else
            {
                move_right = false;
                cur_score += 20;
                _rocket.through = false;
                line_slowdown += line_speed_incr;
                blinks = 0;
            }
        }
        else if (move_left)
        {
            if (_rocket.trans_x + _rocket.speed <= rtl && !_rocket.through)
            {
                if (_can_go)
                {
                    _rocket.through = true;
                }
                else
                {
                    _r = true;
                    goto _END;
                }
            }
            if (_rocket.trans_x + _rocket.speed >= left_bound)
            {
                _rocket.trans_x -= _rocket.speed;
            }
            else
            {
                move_left = false;
                cur_score += 20;
                _rocket.through = false;
                line_slowdown += line_speed_incr;
                blinks = 0;
            }
        }

    _END:
        glutPostRedisplay();
        if (cur_score > hi_score)
        {
            hi_score = cur_score;
        }
        if (_r)
        {
            // PlaySound("ce.wav", NULL, SND_ASYNC|SND_FILENAME);
            return;
        }
        glutTimerFunc(15, update, 0);
    }
    else
    {
        glutTimerFunc(15, update, 0);
    }
}

void restart()
{
    move_left = move_right = false;
    _rocket.trans_x = -0.363;
    _rocket.trans_y = 0.0f;
    _rocket.light_alt = 0;
    _rocket.through = false;
    _rocket.speed = 0.04f;
    line_slowdown = 0.0009f;
    blinks = 0;
    cur_score = 0;
    initLines();
    _score_update_frame = 0;
    glutTimerFunc(15, update, 0);
}

void normal_keys(unsigned char key, int x, int y)
{
    if (key == 'q')
    { // QUIT
        exit(0);
    }
    if (key == 'r')
    { // restart/play again
        restart();
    }
    if (key == 'p')
    {
        paused = paused ? false : true;
    }
    if (key == 13)
    {
        in_game = true;
    }
}

int main(int argc, char **argv)
{
    srand((unsigned)(time(0)));
    _rocket.trans_x = -0.363;
    _rocket.trans_y = 0.0f;
    _rocket.light_alt = 0;
    _rocket.through = false;
    _rocket.speed = 0.04f;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(WCX, WCY);
    glutCreateWindow("Line Breaker");
    gl_init();

    glutFullScreen(); // full screen :D
    glutDisplayFunc(drawScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(normal_keys);
    glutSpecialFunc(special_keys);
    glutTimerFunc(15, update, 0);
    glutMainLoop();

    return 0;
}
