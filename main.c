#include <avr/io.h>
#include <stdio.h>
#include <string.h>

// ---------------- UART ----------------
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

int uart_getchar(void) {
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}

void read_line(char *buf, int max_len, uint8_t hide_input) {
    int i = 0;
    char c;
    while (i < max_len - 1) {
        c = uart_getchar();
        if (c == '\r' || c == '\n') break;
        buf[i++] = c;
        if (!hide_input) uart_putchar(c);
        else uart_putchar('*');
    }
    buf[i] = '\0';
    uart_puts("\r\n");
}

// ---------------- ANSI Colors ----------------
#define RED    "\x1b[31m"
#define GREEN  "\x1b[32m"
#define CYAN   "\x1b[36m"
#define RESET  "\x1b[0m"

// ---------------- DOS Logo ----------------
void print_dos_logo() {
    uart_puts(RED
"  ____   ____   _____ \r\n"
" |  _ \\ |  _ \\ | ____|\r\n"
" | | | || | | ||  _|  \r\n"
" | |_| || |_| || |___ \r\n"
" |____/ |____/ |_____|\r\n" RESET);
}

// ---------------- Neofetch ----------------
void neofetch() {
    const char *version = "1.0";
    const char *codename = "Chicken Nugget";

    print_dos_logo();

    uart_puts(GREEN "user@avr-dos\r\n" RESET);
    uart_puts(CYAN "------------------\r\n" RESET);

    // Use buffer and sprintf
    char buf[128];
    sprintf(buf, GREEN "OS: " RESET "Arduino DOS " GREEN "%s" RESET " (Codename: %s)\r\n", version, codename);
    uart_puts(buf);

    uart_puts(GREEN "Host: " RESET "Arduino Mega 2560 Rev3\r\n");
    uart_puts(GREEN "Terminal: " RESET "Serial Terminal\r\n");
    uart_puts(GREEN "CPU: " RESET "ATmega2560 @ 16MHz\r\n");
}
// ---------------- Simulated Filesystem ----------------
#define MAX_FILES 4
typedef struct {
    const char *name;
    const char *content;
} File;

typedef struct {
    const char *name;
    File files[MAX_FILES];
    int file_count;
} Directory;

Directory root = {
    "/",
    {
        {"readme.txt", "Welcome to Arduino DOS!\r\n"},
        {"info.txt", "Arduino DOS 1.0 Chicken Nugget\r\n"}
    },
    2
};

Directory *current_dir = &root;

// ---------------- Users ----------------
char current_user[16] = "user";
uint8_t is_root = 0;

// ---------------- Help ----------------
void help() {
    uart_puts(CYAN "Available commands:\r\n" RESET);
    uart_puts(GREEN "  echo <text>   " RESET "- Prints text\r\n");
    uart_puts(GREEN "  neofetch      " RESET "- System info with DOS logo\r\n");
    uart_puts(GREEN "  clear         " RESET "- Clears screen\r\n");
    uart_puts(GREEN "  pwd           " RESET "- Shows current directory\r\n");
    uart_puts(GREEN "  ls            " RESET "- Lists files\r\n");
    uart_puts(GREEN "  cd <dir>      " RESET "- Changes directory\r\n");
    uart_puts(GREEN "  cat <file>    " RESET "- Shows file contents\r\n");
    uart_puts(GREEN "  date          " RESET "- Shows system date/time\r\n");
    uart_puts(GREEN "  hostname      " RESET "- Shows hostname\r\n");
    uart_puts(GREEN "  sudo <cmd>    " RESET "- Executes command as root\r\n");
    uart_puts(GREEN "  exit          " RESET "- Logout to login prompt\r\n");
    uart_puts(GREEN "  help          " RESET "- Displays help\r\n");
}

// ---------------- Login ----------------
void login() {
    char username[16];
    char password[16];

    while (1) {
        uart_puts(GREEN "Arduino-DOS\r\n" RESET);
        uart_puts(GREEN "Version: " RESET "1.0 | Codename: Chicken Nugget\r\n\r\n");

        uart_puts("login: ");
        read_line(username, sizeof(username), 0);

        uart_puts("Password: ");
        read_line(password, sizeof(password), 1);

        // Root login
        if (strcmp(username, "root") == 0 && strlen(password) == 0) {
            strcpy(current_user, "root");
            is_root = 1;
            uart_puts(GREEN "\r\nLogin successful (root)!\r\n\r\n" RESET);
            break;
        }

        // User login
        if (strcmp(username, "user") == 0 && strcmp(password, "123456") == 0) {
            strcpy(current_user, "user");
            is_root = 0;
            uart_puts(GREEN "\r\nLogin successful!\r\n\r\n" RESET);
            break;
        }

        uart_puts(RED "Login incorrect\r\n\r\n" RESET);
    }
}

// ---------------- Main ----------------
int main(void) {
    uart_init();
    char cmd[64];
    const char *hostname = "avr-dos";

    while (1) {
        login();
        uart_puts(GREEN "Welcome to Arduino DOS 1.0 (Codename: Chicken Nugget)!\r\n" RESET);

        while (1) {
            uart_puts(GREEN "[");
            uart_puts(current_user);
            uart_puts("@");
            uart_puts(hostname);
            uart_puts(" /]$ " RESET);

            read_line(cmd, sizeof(cmd), 0);

            // ------------- Commands ----------------
            if (strncmp(cmd, "echo ", 5) == 0) {
                uart_puts(cmd + 5);
                uart_puts("\r\n");

            } else if (strcmp(cmd, "neofetch") == 0) {
                neofetch();

            } else if (strcmp(cmd, "help") == 0) {
                help();

            } else if (strcmp(cmd, "clear") == 0) {
                uart_puts("\x1b[2J\x1b[H"); // Clear screen

            } else if (strcmp(cmd, "pwd") == 0) {
                uart_puts(current_dir->name);
                uart_puts("\r\n");

            } else if (strcmp(cmd, "ls") == 0) {
                for (int i = 0; i < current_dir->file_count; i++) {
                    uart_puts(current_dir->files[i].name);
                    uart_puts("\r\n");
                }

            } else if (strncmp(cmd, "cd ", 3) == 0) {
                uart_puts("Directory changed (simulated)\r\n");

            } else if (strncmp(cmd, "cat ", 4) == 0) {
                char *filename = cmd + 4;
                int found = 0;
                for (int i = 0; i < current_dir->file_count; i++) {
                    if (strcmp(current_dir->files[i].name, filename) == 0) {
                        uart_puts(current_dir->files[i].content);
                        found = 1;
                        break;
                    }
                }
                if (!found) uart_puts("File not found\r\n");

            } else if (strcmp(cmd, "date") == 0) {
                uart_puts("2025-11-26 15:50:00\r\n"); // Simulated

            } else if (strcmp(cmd, "hostname") == 0) {
                uart_puts(hostname);
                uart_puts("\r\n");

            } else if (strcmp(cmd, "exit") == 0) {
                uart_puts(GREEN "Logging out...\r\n\r\n" RESET);
                break;

            } else if (strncmp(cmd, "sudo ", 5) == 0) {
                if (is_root) {
                    uart_puts("Already root.\r\n");
                    continue;
                }

                char password[16];
                uart_puts("[sudo] password for user: ");
                read_line(password, sizeof(password), 1);

                if (strcmp(password, "123456") != 0) {
                    uart_puts("sudo: incorrect password\r\n");
                    continue;
                }

                // Execute as root
                char *realcmd = cmd + 5;
                uart_puts("Executing as root: ");
                uart_puts(realcmd);
                uart_puts("\r\n");
                is_root = 1;
                strcpy(current_user, "root");
                strcpy(cmd, realcmd);
                continue;

            } else {
                uart_puts("Unknown command\r\n");
            }
        }
    }
}
