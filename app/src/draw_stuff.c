#include "draw_stuff.h"

#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include <stdio.h>  //printf()
#include <stdlib.h> //exit()
#include <signal.h> //signal()
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#define M_PI 3.14159265358979323846

static UWORD *s_fb;
static bool isInitialized = false;

void DrawStuff_init()
{
    assert(!isInitialized);

    // Exception handling:ctrl + c
    // signal(SIGINT, Handler_1IN54_LCD);

    // Module Init
    if (DEV_ModuleInit() != 0)
    {
        DEV_ModuleExit();
        exit(0);
    }

    // LCD Init
    DEV_Delay_ms(2000);
    LCD_1IN54_Init(HORIZONTAL);
    LCD_1IN54_Clear(WHITE);
    LCD_SetBacklight(1023);

    UDOUBLE Imagesize = LCD_1IN54_HEIGHT * LCD_1IN54_WIDTH * 2;
    if ((s_fb = (UWORD *)malloc(Imagesize)) == NULL)
    {
        perror("Failed to apply for black memory");
        exit(0);
    }
    isInitialized = true;
    DrawStuff_updateScreen(true);
}
void DrawStuff_cleanup()
{
    assert(isInitialized);

    // Module Exit
    free(s_fb);
    s_fb = NULL;
    DEV_ModuleExit();
    isInitialized = false;
}

// void DrawStuff_updateScreen(char* message)

// maybe update later to take an int/ bool
void DrawStuff_updateScreen(bool isHappy)
{
    assert(isInitialized);
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    if (isHappy)
    {
        // ---------------------   HAPPY FACE  ----------------------------------------
        // Draw the face (big YELLOW circle)
        int happy_face_radius = 100; // Increase the radius for a bigger face
        Paint_DrawCircle(LCD_1IN54_WIDTH / 2, LCD_1IN54_HEIGHT / 2, happy_face_radius, YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);

        // Draw the eyes (larger black circles)
        int happy_eye_radius = 10; // Increase the radius for larger eyes
        Paint_DrawCircle(LCD_1IN54_WIDTH / 2 - 25, LCD_1IN54_HEIGHT / 2 - 20, happy_eye_radius, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(LCD_1IN54_WIDTH / 2 + 25, LCD_1IN54_HEIGHT / 2 - 20, happy_eye_radius, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

        // Draw the smile using small circles
        int smile_radius = 60; // Increase the smile radius for a bigger smile
        int smile_center_x = LCD_1IN54_WIDTH / 2 - 2;
        int smile_center_y = LCD_1IN54_HEIGHT / 2 - 10; // Adjust vertical position of smile

        // Use small circles to form the smile (half-arc shape)
        int num_smile_circles = 120; // Increase the number of circles for a smoother and bigger smile
        for (int i = 0; i < num_smile_circles; i++)
        {
            // Calculate angle for each circle position along the smile's path
            float angle = (20 + i * (140 / num_smile_circles)) * (M_PI / 180); // Adjust angle range for 180-degree smile

            // Calculate position of each circle along the arc
            int x = smile_center_x + smile_radius * cos(angle);
            int y = smile_center_y + smile_radius * sin(angle);

            // Draw each small circle to form the smile
            Paint_DrawCircle(x, y, 3, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL); // Increase circle size for better visibility
        }
    }
    else
    {
        // ---------------------   SAD FACE  ----------------------------------------
        // Draw the face (big RED circle)
        int sad_face_radius = 100; // Increase the radius for a bigger face
        Paint_DrawCircle(LCD_1IN54_WIDTH / 2, LCD_1IN54_HEIGHT / 2, sad_face_radius, RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);

        // Draw the eyes (larger black circles)
        int sad_eye_radius = 10; // Increase the radius for larger eyes
        Paint_DrawCircle(LCD_1IN54_WIDTH / 2 - 25, LCD_1IN54_HEIGHT / 2 - 20, sad_eye_radius, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(LCD_1IN54_WIDTH / 2 + 25, LCD_1IN54_HEIGHT / 2 - 20, sad_eye_radius, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

        // Draw the sad using small circles
        int sad_radius = 60;                          // Increase the sad radius for a bigger sad
        int sad_center_x = LCD_1IN54_WIDTH / 2 + 5;   // Adjust the horizontal position of the sad
        int sad_center_y = LCD_1IN54_HEIGHT / 2 + 80; // Adjust vertical position of sad

        // Use small circles to form the sad (half-arc shape)
        int num_sad_circles = 130; // Increase the number of circles for a smoother and bigger sad
        for (int i = 0; i < num_sad_circles; i++)
        {
            // Calculate angle for each circle position along the sad's path
            float angle = (200 + i * (140 / num_sad_circles)) * (M_PI / 180); // Convert to radians (adjusting angle range)

            // Calculate position of each circle along the arc
            int x = sad_center_x + sad_radius * cos(angle);
            int y = sad_center_y + sad_radius * sin(angle);

            // Draw each small circle to form the sad
            Paint_DrawCircle(x, y, 3, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL); // Increase circle size for better visibility
        }
    }

    // /* show bmp - Can't seem to find the file???*/
    // GUI_ReadBmp("./pic/LCD_1inch54.bmp");
    LCD_1IN54_Display(s_fb);
    DEV_Delay_ms(1000);
}
