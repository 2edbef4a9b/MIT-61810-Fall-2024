#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void
sieve(int input_pipe[2]) __attribute__((noreturn));

void
sieve(int input_pipe[2])
{
  int pid, prime, number;
  int output_pipe[2];

  close(input_pipe[1]);
  if (read(input_pipe[0], &prime, sizeof(int)) == 0) {
    close(input_pipe[0]);
    exit(0);
  }
  printf("prime %d\n", prime);

  pipe(output_pipe);
  pid = fork();
  if (pid < 0) {
    fprintf(stderr, "primes: fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // Child process.
    close(input_pipe[0]);
    sieve(output_pipe);
  } else {
    // Parent process.
    close(output_pipe[0]);
    while (read(input_pipe[0], &number, sizeof(int)) > 0) {
      if (number % prime != 0) {
        write(output_pipe[1], &number, sizeof(int));
      }
    }
    close(input_pipe[0]);
    close(output_pipe[1]);
    wait(0);
    exit(0);
  }
}

int
main(int argc, char *argv[])
{
  int pid;
  int output_pipe[2];

  pipe(output_pipe);
  pid = fork();
  if (pid < 0) {
    fprintf(stderr, "primes: fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // Child process.
    sieve(output_pipe);
  } else {
    close(output_pipe[0]);
    // Parent process generator.
    int number;
    for (number = 2; number <= 1000000000; ++number) {
      write(output_pipe[1], &number, sizeof(int));
    }
    close(output_pipe[1]);
    wait(0);
    exit(0);
  }

  exit(0);
}
