#include <demo.h>
#include <libC.h>
#include <user/syscalls.h>
#include <threads.h>

static void dots(void *p __attribute__((unused))) {
  for (;;) {
    sys_write(".", 1);
    sys_sleep(10);
  }
}

static void hello(void *p __attribute__((unused))) {
  sys_write("Hello, world !\n", 15);
  sys_exit(42);
}

static void crash(void *p __attribute__((unused))) {
  int *i = (int*)0xffffff00;
  *i = 42;
  sys_exit(0);
}

static void exec_command(char* str) {
  if (!strcmp(str, "dots")) {
    new_thread(dots, NULL);
  } else if (!strcmp(str, "crash")) {
    new_thread(crash, NULL);
  } else if (!strcmp(str, "hello")) {
    new_thread(hello, NULL);
  } else {
    sys_write("Unknown command", 15);
  }
}

void initsh(void *p __attribute__((unused))) {
  char cmd[10];
  int len = 0;

  memset(cmd, 0, 10);
  for (;;) {
    sys_write("\ninitsh> ", 9);
    while (!(len == 9 || cmd[len - 1] == '\n')) {
      if (!sys_read(cmd + len, 1))
	sys_sleep(1);
      else {
	sys_write(cmd + len, 1);
	len++;
      }
    }
    if (cmd[len - 1] == '\n')
      cmd[len - 1] = '\0';
    exec_command(cmd);
    memset(cmd, 0, 10);
    len = 0;
  }
}
