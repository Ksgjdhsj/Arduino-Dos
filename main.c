// main.c - Arduino DOS 2.0 (Honey Cake)

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>   // Needed for sprintf

// ANSI Colors
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define CYAN    "\x1b[36m"

// UART Functions
void uart_init(void) {
    UBRR0 = 16; // 115200 baud @16MHz
    UCSR0B = (1<<TXEN0)|(1<<RXEN0);
}

void uart_putchar(char c) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = c;
}

int uart_puts(const char *s) {
    while (*s) uart_putchar(*s++);
    return 0;
}

char uart_getchar() {
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}

// Simple filesystem
typedef struct {
    char *name;
    char *content;
} File;

typedef struct {
    File files[10];
    int file_count;
    char *cwd;
} Directory;

Directory root = {
    .files = { {"readme.txt","Welcome to Arduino DOS!"}, {"info.txt","Arduino DOS 2.0 Honey Cake"} },
    .file_count = 2,
    .cwd = "/"
};

Directory *current_dir = &root;

// Utility Functions
void uart_readline(char *buf, int maxlen) {
    int i=0;
    while (1) {
        char c = uart_getchar();
        if (c == '\r' || c == '\n') {
            uart_puts("\r\n");
            buf[i] = 0;
            return;
        } else if (c == 8 || c == 127) { // Backspace
            if (i>0) { i--; uart_puts("\b \b"); }
        } else {
            if (i < maxlen-1) { buf[i++] = c; uart_putchar(c); }
        }
    }
}

// DOS Logo & Neofetch
void print_dos_logo() {
    uart_puts(RED "  ____   ____   _____\r\n" RESET);
    uart_puts(RED " |  _ \\ |  _ \\ | ____|\r\n" RESET);
    uart_puts(GREEN "| | | || | | ||  _|  \r\n" RESET);
    uart_puts(GREEN "| |_| || |_| || |___ \r\n" RESET);
    uart_puts(CYAN  "|____/ |____/ |_____|\r\n" RESET);
}

void neofetch() {
    char buf[128];
    const char *version = "2.0";
    const char *codename = "Honey Cake";

    print_dos_logo();
    uart_puts(GREEN "user@avr-dos\r\n" RESET);
    uart_puts(CYAN "------------------\r\n" RESET);

    sprintf(buf, GREEN "OS: " RESET "Arduino DOS %s (Codename: %s)\r\n", version, codename);
    uart_puts(buf);
    uart_puts(GREEN "Host: " RESET "Arduino Mega 2560 Rev3\r\n");
    uart_puts(GREEN "Terminal: " RESET "Serial Terminal\r\n");
    uart_puts(GREEN "CPU: " RESET "ATmega2560 @ 16MHz\r\n");
}

// Welcome Message
void print_welcome() {
    uart_puts("\r\n");
    uart_puts(CYAN "Welcome To Arduino DOS 2.0 (Codename: Honey Cake)\r\n" RESET);
    uart_puts(YELLOW "  * Documentation: https://github.com/Ksgjdhsj/Arduino-Dos\r\n" RESET);
    uart_puts("\r\n");
}

// Help
void help() {
    uart_puts(GREEN "Available commands:\r\n" RESET);
    uart_puts("  echo <text>   - Print text\r\n");
    uart_puts("  help          - Show this help\r\n");
    uart_puts("  clear         - Clear screen\r\n");
    uart_puts("  pwd           - Show current directory\r\n");
    uart_puts("  ls            - List files\r\n");
    uart_puts("  cd <dir>      - Change directory\r\n");
    uart_puts("  cat <file>    - Show file content\r\n");
    uart_puts("  date          - Show date/time\r\n");
    uart_puts("  hostname      - Show hostname\r\n");
    uart_puts("  neofetch      - Show system info\r\n");
    uart_puts("  rm <file>     - Remove a file\r\n");
    uart_puts("  sudo <cmd>    - Run command as root\r\n");
    uart_puts("  exit          - Logout\r\n");
}

// Main Shell
void shell(char *user) {
    char cmd[64];
    char buf[128];
    while (1) {
        sprintf(buf, "%s@avr-dos %s$ ", user, current_dir->cwd);
        uart_puts(buf);
        uart_readline(cmd, sizeof(cmd));

        // Commands
        if (strncmp(cmd, "echo ", 5) == 0) {
            uart_puts(cmd+5); uart_puts("\r\n");
        }
        else if (strcmp(cmd,"help")==0) help();
        else if (strcmp(cmd,"clear")==0) uart_puts("\x1b[2J\x1b[H");
        else if (strcmp(cmd,"pwd")==0) { uart_puts(current_dir->cwd); uart_puts("\r\n"); }
        else if (strcmp(cmd,"ls")==0) {
            for (int i=0;i<current_dir->file_count;i++) {
                uart_puts(current_dir->files[i].name); uart_puts("\r\n");
            }
        }
        else if (strncmp(cmd,"cat ",4)==0) {
            char *filename=cmd+4; int found=-1;
            for(int i=0;i<current_dir->file_count;i++) if(strcmp(current_dir->files[i].name,filename)==0) found=i;
            if(found!=-1) { uart_puts(current_dir->files[found].content); uart_puts("\r\n"); }
            else { sprintf(buf, RED "%s: command not found\x1b[0m\r\n", filename); uart_puts(buf); }
        }
        else if (strncmp(cmd,"rm ",3)==0) {
            char *filename=cmd+3; int found=-1;
            for(int i=0;i<current_dir->file_count;i++) if(strcmp(current_dir->files[i].name,filename)==0) found=i;
            if(found==-1) { uart_puts("rm: file not found\r\n"); }
            else { for(int j=found;j<current_dir->file_count-1;j++) current_dir->files[j]=current_dir->files[j+1]; current_dir->file_count--; uart_puts("File removed\r\n"); }
        }
        else if (strcmp(cmd,"hostname")==0) uart_puts("avr-dos\r\n");
        else if (strcmp(cmd,"neofetch")==0) neofetch();
        else if (strcmp(cmd,"exit")==0) break;
        else if (strncmp(cmd,"sudo ",5)==0) { shell("root"); }
        else { sprintf(buf, RED "%s: command not found\x1b[0m\r\n", cmd); uart_puts(buf); }
    }
}

// Login System
void login() {
    char username[16];
    char password[16];
    while(1) {
        uart_puts("login: ");
        uart_readline(username,sizeof(username));
        uart_puts("password: ");
        uart_readline(password,sizeof(password));

        if(strcmp(username,"user")==0 && strcmp(password,"123456")==0) {
            uart_puts("Login successful!\r\n");
            print_welcome();
            shell("user");
            break;
        }
        else if(strcmp(username,"root")==0 && strlen(password)==0) {
            uart_puts("Login successful!\r\n");
            print_welcome();
            shell("root");
            break;
        } else {
            uart_puts("Login failed!\r\n");
        }
    }
}

// Main
int main(void) {
    uart_init();
    uart_puts("\x1b[2J\x1b[H"); // Clear screen
    login();
    while(1); // Halt after logout
}
