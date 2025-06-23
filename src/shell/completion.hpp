#pragma once

char* command_generator(const char* text, int state);
char** command_completion(const char* text, int start, int end);