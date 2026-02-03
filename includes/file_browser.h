#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

#ifdef __APPLE__

static char* select_file(void) {
    const char *cmd =
        "osascript -e 'POSIX path of (choose file of type {\"ch8\", \"bin\"} "
        "with prompt \"Select a Chip-8 ROM\")' 2>/dev/null";

    char path[1024];
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;

    if (fgets(path, sizeof(path), fp)) {
        path[strcspn(path, "\n")] = 0;
        pclose(fp);
        return strdup(path);
    }

    pclose(fp);
    return NULL;
}

#else

static void set_raw_mode(int enable) {
    static struct termios oldt;
    struct termios newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

static char *get_key(void) {
    static char buffer[4];
    memset(buffer, 0, sizeof(buffer));
    read(STDIN_FILENO, buffer, 1);
    if (buffer[0] == '\x1b') {
        read(STDIN_FILENO, buffer + 1, 2);
    }
    return buffer;
}

static char* select_file(void) {
    char current_path[PATH_MAX];
    getcwd(current_path, sizeof(current_path));
    int selected = 0;
    int scroll_offset = 0;
    const int VIEWPORT_HEIGHT = 15;

    while (1) {
        DIR *dir = opendir(current_path);
        if (!dir) return NULL;

        struct dirent *entry;
        char items[1024][NAME_MAX]; // Increased buffer for local files
        int is_dir[1024];
        int count = 0;

        while ((entry = readdir(dir)) != NULL && count < 1024) {
            if (strcmp(entry->d_name, ".") == 0) continue;
            strcpy(items[count], entry->d_name);
            struct stat st;
            char fullpath[PATH_MAX];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", current_path, entry->d_name);
            stat(fullpath, &st);
            is_dir[count++] = S_ISDIR(st.st_mode);
        }
        closedir(dir);

        printf("\033[H\033[J");
        printf("📂 File Browser\nPath: %s\n", current_path);
        printf("Items %d-%d of %d | Enter = Open/Select\n", scroll_offset+1, 
                (scroll_offset + VIEWPORT_HEIGHT > count ? count : scroll_offset + VIEWPORT_HEIGHT), count);
        printf("----------------------------------------\n");

        for (int i = 0; i < VIEWPORT_HEIGHT; i++) {
            int idx = i + scroll_offset;
            if (idx >= count) break;
            if (idx == selected) {
                printf("\033[1;36m> %s %s\033[0m\n", (is_dir[idx] ? "📁" : "📄"), items[idx]);
            } else {
                printf("  %s %s\n", (is_dir[idx] ? "📁" : "📄"), items[idx]);
            }
        }
        printf("----------------------------------------\n");

        set_raw_mode(1);
        char *key = get_key();
        set_raw_mode(0);

        if (strcmp(key, "\x1b[A") == 0) { // UP
            if (selected > 0) {
                selected--;
                if (selected < scroll_offset) scroll_offset = selected;
            } else {
                selected = count - 1;
                scroll_offset = count - VIEWPORT_HEIGHT;
                if (scroll_offset < 0) scroll_offset = 0;
            }
        } else if (strcmp(key, "\x1b[B") == 0) { // DOWN
            if (selected < count - 1) {
                selected++;
                if (selected >= scroll_offset + VIEWPORT_HEIGHT) scroll_offset = selected - VIEWPORT_HEIGHT + 1;
            } else {
                selected = 0;
                scroll_offset = 0;
            }
        } else if (key[0] == '\n' || key[0] == '\r') {
            if (is_dir[selected]) {
                if (strcmp(items[selected], "..") == 0) {
                    char *slash = strrchr(current_path, '/');
                    if (slash && slash != current_path) *slash = '\0';
                    else strcpy(current_path, "/");
                } else {
                    if (strcmp(current_path, "/") != 0) strcat(current_path, "/");
                    strcat(current_path, items[selected]);
                }
                selected = 0;
                scroll_offset = 0;
            } else {
                char *result = malloc(PATH_MAX);
                snprintf(result, PATH_MAX, "%s/%s", current_path, items[selected]);
                return result;
            }
        }
    }
}

#endif
#endif
