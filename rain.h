#ifndef RAIN_H
#define RAIN_H

typedef struct
{
    // Life
    bool alive; // is the particle alive?
    float life; // particle lifespan
    float fade; // decay
    // color
    float red;
    float green;
    float blue;
    // Position/direction
    float xpos;
    float ypos;
    float zpos;
    // Velocity/Direction, only goes down in y dir
    float vel;
    // Gravity
    float gravity;
} particles;

#endif
