#include <stdio.h>

int main() {
  char buffer[256];
  if (fgets(buffer, 256, stdin) == NULL) {
    return 1;
  }

  char *p = buffer;
  char *str = NULL;

  while (*p) {
    if (*p == '"' && str == NULL) {
      p++;
      str = p;
    }
    if (*p == '"' && str != NULL ) {
      *p = '\0';
      break;
    }
    p++;
  }

  if (str == NULL) {
    return 1;
  }

  puts("    .intel_syntax noprefix");
  puts(".LC0:");
  printf("    .string \"%s\"\n", str);
  puts(".globl main");
  puts("main:");
  puts("    push rbp");
  puts("    mov rbp, rsp");
  puts("    lea rdi, .LC0[rip]");
  puts("    call puts@PLT");
  puts("    mov eax, 0");
  puts("    pop rbp");
  puts("    ret");
}

