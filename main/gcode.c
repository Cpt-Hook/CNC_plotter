//
// Created by standa on 2/10/19.
//

#include <esp_log.h>
#include <string.h>
#include <ctype.h>

#include "gcode.h"
#include "servo.h"
#include "main.h"
#include "motors.h"
#include "fans.h"

#define COMMAD_LENGTH 20


static const char *TAG = "GCODE";


void process_commands() {
    for(;;){
        Command *command;
        xQueueReceive(queue, &command, portMAX_DELAY);
        print_command(command);

        for (int i = 0; i < command->size; i++) {
            switch (command->fields[i].letter) {
                case 'G':
                    g_command(command, (int) command->fields[i].num);
                    break;
                case 'S':
                    s_command(command, (int) command->fields[i].num);
                    break;
                case 'E':
                    e_command(command, (int) command->fields[i].num);
                    break;
                case 'F':
                    f_command(command, (int) command->fields[i].num);
                    break;
                case 'R':
                    restart();
                    break;
                default:
                    break;
            }
        };

        free(command->fields);
        free(command);
        ESP_LOGI(TAG, "Gcode succesfully executed");
        ESP_LOGI(TAG, "Free spaces in command queue: %d", (int)uxQueueSpacesAvailable(queue));
    }
}

void g_command(Command *command, int num) {
    bool move = false;
    if (num != 0 && num != 1) {
        ESP_LOGI(TAG, "Gcode command not implemented");
        return;
    }

    double feed = NORMAL_SPEED;
    if (num == 0) {
        feed = MAX_SPEED;
    } else if (num == 1) {
        feed = NORMAL_SPEED;
    }

    double newX = 0, newY = 0;
    for (int i = 0; i < command->size; i++) {
        if (command->fields[i].letter == 'X') {
            newX = command->fields[i].num;
            move = true;
        } else if (command->fields[i].letter == 'Y') {
            newY = command->fields[i].num;
            move = true;
        } else if (command->fields[i].letter == 'Z') {
            if (command->fields[i].num >= 0) {
                move_servo(SERVO_UP);
            } else {
                move_servo(servo_paper_pos);
            }
        } else if (command->fields[i].letter == 'F') {
            feed = command->fields[i].num;
        }
    }
    if (move) {
        plot_line(newX, newY, (int) feed);
    }
}

void s_command(Command *command, int num) {
    move_servo(num);
}

void e_command(Command *command, int num) {
    if (num) {
        motor_power(HIGH);
        servo_attach();
    } else {
        motor_power(LOW);
        servo_detach();
    }
}

void f_command(Command *command, int num) {
    if(num == 1) {
        set_fans(ON);
    }else if(num == 0){
        set_fans(OFF);
    }
}

Command* parse_buffer(const char* buffer, int bufferLength) {
    int size = 0;
    Command *command = (Command*) malloc(sizeof(Command));
    command->fields = (Field*) malloc(sizeof(Field) * COMMAD_LENGTH);

    for (int i = 0; i < bufferLength; i++) {
        if (size >= COMMAD_LENGTH) {
            break;
        }
        if (buffer[i] == ';') { //skip ; comment
            break;
        } else if (buffer[i] == '(') { //skip () comment
            while (++i < bufferLength) {
                if (buffer[i] == ')') {
                    i++;
                    if (i >= bufferLength)
                        command->size = size;
                    return command;
                }
            }
        } else if (buffer[i] == ' ') { //skip whitespaces
            continue;
        }
        if (isalpha((int)buffer[i])) {
            command->fields[size].letter = buffer[i];
            command->fields[size].num = atof(&buffer[++i]);
            while (i < bufferLength) {
                if (isdigit((int)buffer[i]) || buffer[i] == '.' || buffer[i] == ' ' || buffer[i] == '-' || buffer[i] == '+') {
                    i++;
                    continue;
                }
                else {
                    i--;
                    break;
                }
            }
            size++;
            if(size > COMMAD_LENGTH) {
                // TODO realloc more memory?
                ESP_LOGI(TAG, "Gcode with more fields than allocated buffer (%d) fields - restarting", COMMAD_LENGTH);
                esp_restart();
            }
        } else {
            ESP_LOGI(TAG, "WTF");
        }
    }
    command->size = size;
    command->fields = (Field*) realloc(command->fields, sizeof(Field) * size);
    return command;
}

Command* generate_R_command() {
    Command *command = (Command*) malloc(sizeof(Command));
    command->fields = (Field*) malloc(sizeof(Field) * 1);
    command->size = 1;
    command->fields[0].letter = 'R';

    return command;
}

void print_command(Command *command) {
    for (int i = 0; i < command->size; i++) {
        printf("%c%.4lf ", command->fields[i].letter, command->fields[i].num);
    }
    printf("\n");
}