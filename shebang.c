
#if HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_LINUX_BINFMTS_H
#include <linux/binfmts.h>
#else
#define BINPRM_BUF_SIZE 250
#endif


/* strlen */
#include <string.h>

/* realloc, exit */
#include <stdlib.h>

/* fdopen, fprintf */
#include <stdio.h>

/* for isspace() */
#include <ctype.h>

/* for read(), write() */
#include <unistd.h>

/* for open() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* wait */
#include <sys/wait.h>

/* for error() */
#include <error.h>
#include <errno.h>

#define DEBUG 1

/* Argv, argc are as follows (if shebang is called from a shebang file):
 *
 * argv[0] = path-to-the-shebang-program
 * argv[1] = path-to-the-shebang-file
 * argv[2] and following = arguments given to the execution of the shebang file
 *
 * E.g. for
 *  ./http-grab 'http://127.0.0.1:8081'
 *
 * argv[0] = path-to-the-shebang-program, e.g. /usr/local/bin/shebang
 * argv[1] = ./http-grab
 * argv[2] = http://127.0.0.1:8081
 */ 
int
main(int argc, char *argv[]) {
  /* --------------------------------------------------------------------------------------------
     Step 1: obtain the words of the first line of the shebang file 
     --------------------------------------------------------------------------------------------
   */

  /*
   * From: https://stackoverflow.com/questions/10813538/shebang-line-limit-in-bash-and-linux-kernel:
   * man execve: "A maximum line length of 127 characters is allowed for the first line in a #!
   * executable shell script." It's limited in the kernel by BINPRM_BUF_SIZE, set in
   * include/linux/binfmts.h.
   * 
   * Thus:  +2 for '\n' and '\0', beter be safe than sorry 
   */
  char line[BINPRM_BUF_SIZE+2];

  /* file descriptor for input */
  const char* input_filename = argv[1]; 
  int input = open(argv[1], O_RDONLY); 
  if (input < 0) 
    error(1, errno, "%s: cannot open for reading", input_filename); 
  FILE* finput = fdopen(input, "r");
  if (! finput)
    error(1, errno, "%s: cannot fdopen %d to stream", input_filename, input);

  if (fgets(line, sizeof(line), finput) != line) 
    error(1, errno, "%s: empty first line?", input_filename);

  const char* words[BINPRM_BUF_SIZE]; /* array of line 1 */
  int n_words = 0; /* size of the array */

  int in_whitespace = 1; /* mode while processing line */
  for (int i=0; line[i]; ++i) { 
    if (isspace(line[i])) {
      if (!in_whitespace)
        line[i] = '\0'; /* and remain in white_space mode */
      else
        ;
    }
    else /* line[i] is non-space character */
      if (in_whitespace) {
        /* first char of a new word, save it */
        words[n_words++] = line +i;
        in_whitespace = 0;
      }
      else 
        ; /* one more char of word */
  }
  words[n_words] = 0;

#ifdef DEBUG
  fprintf(stdout, "Step 1: after obtaining args\n");
  for (int i=0; (i<n_words); ++i) 
    fprintf(stdout, "word[%d] = %s\n", i, words[i]);
#endif

  /* --------------------------------------------------------------------------------------------
     Step 2: read the rest of the file and save it in a temporary file
     --------------------------------------------------------------------------------------------
   */
  char tmp_filename[20];
  if (sprintf(tmp_filename, "/tmp/shebang.%d", getpid()) < 0)
    error(1, 0, "%s: cannot create temporary file_name", tmp_filename);

  {
    int tmp_file = open( tmp_filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if (tmp_file < 0)
      error(1, errno, "%s: cannot create temporary file", tmp_filename);
	
    char buf[8192];
    int n;
    while ((n = read(input, buf, sizeof(buf)))>0) {
	    if ( write(tmp_file, buf, n) != n )
        error(1, errno, "write error on %s", tmp_filename);
    }
    if (n < 0) 
      error(1, errno, "read error from %s", input_filename);

    if (close(tmp_file) < 0)
      error(1, errno, "close error on %s", tmp_filename);
  }

#ifdef DEBUG
  fprintf(stdout, "Step 2: %s written", tmp_filename);
#endif

  /* --------------------------------------------------------------------------------------------
     Step 3: replace $1, .. $< in words[] array
     --------------------------------------------------------------------------------------------
   */

  for (int i=0; (words[i]); ++i) {
    if (words[i][0] == '$') {
      if (strlen(words[i]) != 2)
        error(1, 0, "illegal parameter: %s", words[i]);
      if (isdigit(words[i][1])) { 
        int pos = words[i][1] - '0';
        if (pos > (argc-2))
          error(1, 0, "no such parameter: %s", words[i]);
        words[i] = argv[pos+1];
      }
      if (words[i][1] == '<')
        words[i] = tmp_filename;
    }
  }

#ifdef DEBUG
  fprintf(stdout, "Step 3: after arg transformation\n");
  for (int i=0; (i<n_words); ++i) 
    fprintf(stdout, "word[%d] = %s\n", i, words[i]);
#endif

  /* --------------------------------------------------------------------------------------------
     Step 4: fork and wait for child or exec to command specified in shebang line
     --------------------------------------------------------------------------------------------
   */

  int child_pid = fork();
  if (child_pid<0) 
    error(1, errno, "cannot fork");

  if (child_pid == 0) {
    /* child */
    char* const* args = (char* const*) (words + 1);
    /* execvp should transfer PATH etc */
    if ( execvp(words[1], args) < 0)
      error(1, errno, "exec %s failed", words[1]);
    return 0;
  }
  else { /* parent, wait for child to die and then delete tmp_filename */
    int status;
    if ( waitpid(child_pid, &status, 0) < 0)
      error(1, errno, "waitpid on %d failed", child_pid);
    if (unlink(tmp_filename) != 0)
      error(1, errno, "%s: cannot delete", tmp_filename);
    return 0;
  }
}

