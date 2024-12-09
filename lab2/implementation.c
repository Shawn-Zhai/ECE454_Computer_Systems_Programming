#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

#define CMD_W    1
#define CMD_A    2
#define CMD_S    3
#define CMD_D    4
#define CMD_CW   5
#define CMD_CCW  6
#define CMD_MX   7
#define CMD_MY   8

const int delta_x[4] = {0, -1, 0, 1};
const int delta_y[4] = {-1, 0, 1, 0};

uint16_t colored_pixels_row[2400000];
uint16_t colored_pixels_col[2400000];
uint8_t colored_pixels_R[2400000];
uint8_t colored_pixels_G[2400000];
uint8_t colored_pixels_B[2400000];


void apply_translation_up(int value, int rotation, bool x_reflect, int *x_shift, int *y_shift) {

    // Adjust for rotation
    int transformed_dir = rotation % 4;

    // Adjust for reflections
    if (x_reflect) transformed_dir = (transformed_dir + 2) % 4;

    *x_shift += delta_x[transformed_dir] * value;
    *y_shift += delta_y[transformed_dir] * value;
}

void apply_translation_down(int value, int rotation, bool x_reflect, int *x_shift, int *y_shift) {

    // Adjust for rotation
    int transformed_dir = (2 + rotation) % 4;

    // Adjust for reflections
    if (x_reflect) transformed_dir = (transformed_dir + 2) % 4;

    *x_shift += delta_x[transformed_dir] * value;
    *y_shift += delta_y[transformed_dir] * value;
}

void apply_translation_left(int value, int rotation, bool y_reflect, int *x_shift, int *y_shift) {

    // Adjust for rotation
    int transformed_dir = (1 + rotation) % 4;

    // Adjust for reflections
    if (y_reflect) transformed_dir = (transformed_dir + 2) % 4;

    *x_shift += delta_x[transformed_dir] * value;
    *y_shift += delta_y[transformed_dir] * value;
}

void apply_translation_right(int value, int rotation, bool y_reflect, int *x_shift, int *y_shift) {

    // Adjust for rotation
    int transformed_dir = (3 + rotation) % 4;

    // Adjust for reflections
    if (y_reflect) transformed_dir = (transformed_dir + 2) % 4;

    *x_shift += delta_x[transformed_dir] * value;
    *y_shift += delta_y[transformed_dir] * value;
}

// Function to apply rotation
void apply_rotation(int value, int *rotation, bool x_reflect, bool y_reflect) {
    
    if (x_reflect ^ y_reflect) {
        value *= -1;
    }

    *rotation += value;
    // Normalize rotation to be between -3 and 3
    *rotation = (*rotation % 4 + 4) % 4;
}

// Team Info Function (unchanged)
void print_team_info() {
    // Please modify this field with something interesting
    char team_name[] = "Aminuos";

    // Please fill in your information
    char student_first_name[] = "Shawn";
    char student_last_name[] = "Zhai";
    char student_student_number[] = "1006979389";

    // Printing out team information
    printf("*******************************************************************************************************\n");
    printf("Team Information:\n");
    printf("\tteam_name: %s\n", team_name);
    printf("\tstudent_first_name: %s\n", student_first_name);
    printf("\tstudent_last_name: %s\n", student_last_name);
    printf("\tstudent_student_number: %s\n", student_student_number);
}

int get_command_code(const char *key) {
    switch (key[0]) {
        case 'W': return CMD_W;
        case 'A': return CMD_A;
        case 'S': return CMD_S;
        case 'D': return CMD_D;
        case 'C': return (key[1] == 'W') ? CMD_CW : CMD_CCW;
        case 'M': return (key[1] == 'X') ? CMD_MX : CMD_MY;
    }
    return 0;
}

// Implementation Driver Function
void implementation_driver(struct kv *sensor_values, int sensor_values_count, unsigned char *frame_buffer,
                           unsigned int width, unsigned int height, bool grading_mode) {
    int processed_frames = 0;
    int x_shift = 0, y_shift = 0;
    int rotation = 0; // Cumulative rotation, from -3 to 3
    bool XR = false, YR = false;

    int width_minus_1 = width - 1;
    int height_minus_1 = height - 1;

    int colored_pixel_count = 0;
    int pixel_idx = 0;
    int total_pixels = width * height;

    // Find colored pixels
    for (int i = 0; i < total_pixels; i++) {
        if (frame_buffer[pixel_idx] != 255 || frame_buffer[pixel_idx + 1] != 255 || frame_buffer[pixel_idx + 2] != 255) {
            int row = i / width;
            int col = i % width;
            colored_pixels_row[colored_pixel_count] = row;
            colored_pixels_col[colored_pixel_count] = col;
            colored_pixels_R[colored_pixel_count] = frame_buffer[pixel_idx];
            colored_pixels_G[colored_pixel_count] = frame_buffer[pixel_idx + 1];
            colored_pixels_B[colored_pixel_count] = frame_buffer[pixel_idx + 2];
            colored_pixel_count++;
        }
        pixel_idx += 3;
    }

    int total = sensor_values_count - sensor_values_count % 25;
    for (int sensorValueIdx = 0; sensorValueIdx < total; sensorValueIdx++) {
    // for (int sensorValueIdx = 0; sensorValueIdx < sensor_values_count; sensorValueIdx++) {

        char *key = sensor_values[sensorValueIdx].key;
        int value = sensor_values[sensorValueIdx].value;

        int cmd_code = get_command_code(key);

        switch (cmd_code) {
            case CMD_W:
                apply_translation_up(value, rotation, XR, &x_shift, &y_shift);
                break;
            case CMD_A:
                apply_translation_left(value, rotation, YR, &x_shift, &y_shift);
                break;
            case CMD_S:
                apply_translation_down(value, rotation, XR, &x_shift, &y_shift);
                break;
            case CMD_D:
                apply_translation_right(value, rotation, YR, &x_shift, &y_shift);
                break;

            case CMD_CW:
                // Apply rotation clockwise
                apply_rotation(value, &rotation, XR, YR);
                break;

            case CMD_CCW:
                // Apply rotation counter-clockwise
                apply_rotation(-value, &rotation, XR, YR);
                break;

            case CMD_MX:
                // Apply reflection over X-axis
                XR = !XR;
                break;

            case CMD_MY:
                // Apply reflection over Y-axis
                YR = !YR;
                break;
        }

        processed_frames++;

        if (processed_frames % 25 == 0) {

            memset(frame_buffer, 255, width * height * 3);

            // Apply the cumulative transformations
            bool XT = x_shift != 0, YT = y_shift != 0;
            int transformation = (XT << 3) | (YT << 2) | (XR << 1) | YR;
            
            if (rotation == 1) {

                switch (transformation) {
                    case 15: // 1111
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 14: // 1110
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {
                                
                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 13: // 1101
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 12: // 1100
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 11: // 1011
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 10: // 1010
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 9: // 1001
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 8: // 1000
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 7: // 0111
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 6: // 0110
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 5: // 0101
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 4: // 0100
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 3: // 0011
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 2: // 0010
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 1: // 0001
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 0: // 0000
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 90 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_row;
                            colored_pixels_row[pixel] = orig_col;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                }
            }

            else if (rotation == 2) {

                switch (transformation) {
                    case 15: // 1111
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 14: // 1110
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {
                                
                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 13: // 1101
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 12: // 1100
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 11: // 1011
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 10: // 1010
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 9: // 1001
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 8: // 1000
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 7: // 0111
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 6: // 0110
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 5: // 0101
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 4: // 0100
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 3: // 0011
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 2: // 0010
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 1: // 0001
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 0: // 0000
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 180 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = width_minus_1 - orig_col;
                            colored_pixels_row[pixel] = height_minus_1 - orig_row;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                }
            }

            else if (rotation == 3) {

                switch (transformation) {
                    case 15: // 1111
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 14: // 1110
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {
                                
                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 13: // 1101
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 12: // 1100
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 11: // 1011
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 10: // 1010
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 9: // 1001
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 8: // 1000
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 7: // 0111
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 6: // 0110
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 5: // 0101
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 4: // 0100
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 3: // 0011
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 2: // 0010
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 1: // 0001
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 0: // 0000
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            // Rotate 270 degrees
                            int orig_row = colored_pixels_row[pixel];
                            int orig_col = colored_pixels_col[pixel];
                            colored_pixels_col[pixel] = orig_row;
                            colored_pixels_row[pixel] = height_minus_1 - orig_col;

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                }
            }

            else { // rotation == 0
                
                switch (transformation) {
                    case 15: // 1111
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 14: // 1110
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {
                                
                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 13: // 1101
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 12: // 1100
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT
                            colored_pixels_row[pixel] += y_shift; // YT

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 11: // 1011
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 10: // 1010
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 9: // 1001
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 8: // 1000
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] += x_shift; // XT

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 7: // 0111
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 6: // 0110
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            
                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 5: // 0101
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 4: // 0100
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] += y_shift; // YT

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 3: // 0011
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 2: // 0010
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_row[pixel] = (height_minus_1 - colored_pixels_row[pixel]); // XR
                        
                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                    case 1: // 0001
                    {
                        for (int pixel = 0; pixel < colored_pixel_count; pixel++) {

                            colored_pixels_col[pixel] = (width_minus_1 - colored_pixels_col[pixel]); // YR

                            // render frame
                            unsigned char *frame_ptr = &frame_buffer[(colored_pixels_row[pixel] * width + colored_pixels_col[pixel]) * 3];
                            *frame_ptr++ = colored_pixels_R[pixel];
                            *frame_ptr++ = colored_pixels_G[pixel];
                            *frame_ptr = colored_pixels_B[pixel];
                        }
                        break;
                    }
                }
            }

            // Verify the frame
            verifyFrame(frame_buffer, width, height, grading_mode);

            // Reset the cumulative transformations
            x_shift = 0;
            y_shift = 0;
            rotation = 0;
            XR = false;
            YR = false;
        }
    }
}