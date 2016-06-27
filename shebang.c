#undef DEBUG

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_LINUX_BINFMTS_H
#include <linux/binfmts.h>
#else
#define BINPRM_BUF_SIZE 250
#endif


/* fprintf, sprintf */ 
#include <stdio.h>

/* memcpy */
#include <string.h>

/* exit */
#include <stdlib.h>

/* isspace */
#include <ctype.h>

/* read, write, close */
#include <unistd.h>

/* for open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* wait */
#include <sys/wait.h>

/* for error */
#include <error.h>
#include <errno.h>

#ifdef DEBUG
void
print_array(FILE* file, int lineno, const char* prompt,  const char* words[], int size) {
  fprintf(file, "%d: %s:", lineno, prompt);
  for (int i=0; (i < size); ++i) {
    if (i>0)
      fprintf(file, " ");
    fprintf(file, "%s", (words[i] ? words[i] : "NULL"));
  }
  fprintf(file, "\n");
}
#endif

/* Argv, argc are as follows (if shebang is called from a shebang file):
 *
 * argv[0] = path-to-the-shebang-program
 * argv[1] = shebang arguments, i.e. the first line of the shebang file minus #!/path/to/shebang
 * argv[2] = path to the shebang file
 * argv[3] and following = arguments given to the execution of the shebang file
 *
 * E.g. for
 *  ./http-grab 'http://127.0.0.1:8081':
 *
 * argv[0] = path-to-the-shebang-program, e.g. /usr/local/bin/shebang
 * argv[1] = phantomjs $<
 * argv[2] = ./http-grab
 * argv[3] = http://127.0.0.1:8081
 */ 
int
main(int argc, char *argv[]) {

  if (argc < 3)
    error(1, 0, "need at least 3 arguments but only got %d\n", argc);

  const char* words[BINPRM_BUF_SIZE]; /* array of line 1, last word will be 0 for execvp */
  int n_words = 0; /* size of the array */

  /* --------------------------------------------------------------------------------------------
     Step 1: split argv[1] into words, adding '\0' at end of each word in argv[1]
     --------------------------------------------------------------------------------------------
   */

  {
    char* line = argv[1];

    int in_whitespace = 1; /* mode while processing line */
    for (int i=0; line[i]; ++i) { 
      if (isspace(line[i])) {
        if (!in_whitespace) {
          line[i] = '\0';  /* end of word */
          in_whitespace = 1;
        }
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
    words[n_words] = 0; /* ensure 0 pointer at end, for execvp etc. */
  }

  /* --------------------------------------------------------------------------------------------
     Step 2: copy from line 2 to the end of the shebang file to a temporary file
     --------------------------------------------------------------------------------------------
   */

  char tmp_filename[30];
  if (sprintf(tmp_filename, "/tmp/%s-%s.%d", PACKAGE, PACKAGE_VERSION, getpid()) < 0)
    error(1, 0, "%s: cannot create temporary file_name", tmp_filename);

  {
    const char* input_filename = argv[2]; 

    int input = open(input_filename, O_RDONLY); 
    if (input < 0) 
      error(1, errno, "%s: cannot open for reading", input_filename); 

    int tmp_file = open( tmp_filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if (tmp_file < 0)
      error(1, errno, "%s: cannot create temporary file", tmp_filename);
	
    char buf[8192];
    int n; /* how many bytes were written/read */
    int first_line_read = 0; /* = 1 iff the first line was skipped */
    int offset = 0; /* read and write will be happen from buf + offset */

    while ((n = read(input, buf, sizeof(buf)))>0) {
      if (!first_line_read) {
        for (int j=0; ((j<n) && (!first_line_read)); ++j) {
          if (buf[j] == '\n') {
            offset = j+1;
            first_line_read = 1;
          }
        }
        /* Can happen with small reads or very long first line, but the latter is unlikely since its length
         * is limited to BINPRM_BUF_SIZE 
         */
        if (! first_line_read) 
          offset = n;
      }
      else
        offset = 0;

	    if ( write(tmp_file, buf + offset, n - offset) != n - offset )
        error(1, errno, "write error on %s", tmp_filename);
    }
    if (n < 0) 
      error(1, errno, "read error from %s", input_filename);

    if (close(input) < 0)
      error(1, errno, "close error on %s", input_filename);
    if (close(tmp_file) < 0)
      error(1, errno, "close error on %s", tmp_filename);
  }

  /* --------------------------------------------------------------------------------------------
     Step 3: replace $1, .. $< in words[] array and append any unused argv elements to words
     --------------------------------------------------------------------------------------------
   */

  {
    /* copy of argv to keep track of 'used' arguments from the command line, 0 means used */
    char** argv_unused = malloc(argc * sizeof(char*));
    memcpy(argv_unused, argv, argc * sizeof(char*));
    argv_unused[0] = argv_unused[1] = argv_unused[2] = 0;

    for (int i=0; (words[i]); ++i) {
      if (words[i][0] == '$') {
        /* check words[i] is of form '$X' */
        if ((words[i][1] == '\0') || (words[i][2] != '\0')) {
          free(argv_unused);
          error(1, 0, "illegal parameter: %s", words[i]);
        }
        if (isdigit(words[i][1])) { 
          int pos = (words[i][1] - '0') + 2;
          if (pos < argc) {
            words[i] = argv[pos]; 
            argv_unused[pos] = 0;
          }
          else
            /* 1st arg 0 means no exit, just message */
            error(0, 0, "no such parameter: %s", words[i]);
        }
        if (words[i][1] == '<')
          words[i] = tmp_filename;
      }
    }

    /* Append all unused argv[] elements to words */
    for (int i=0; (i<argc); ++i)
      if (argv_unused[i])
        words[n_words++] = argv[i];
    words[n_words] = 0; /* ensure 0 pointer at end, for execvp etc. */

    free(argv_unused);
  }

#ifdef DEBUG
  print_array(stdout, __LINE__,"words",  words, n_words);
#endif

  /* --------------------------------------------------------------------------------------------
     Step 4: fork and wait for child or exec to command specified in shebang line
     --------------------------------------------------------------------------------------------
   */

  int child_pid = fork();
  if (child_pid<0) 
    error(1, errno, "cannot fork");

  if (child_pid == 0) { /* child */
    /* execvp should transfer PATH etc */
    if ( execvp(words[0], (char * const*)words) < 0)
      error(1, errno, "exec %s failed", words[0]);
    return 1; /* execvp should not return, thus return error code 1 */
  }
  else { /* parent, wait for child to die and then delete tmp_filename */
    int status;
    if ( waitpid(child_pid, &status, 0) < 0)
      error(1, errno, "waitpid on %d failed", child_pid);
    
    if (unlink(tmp_filename) != 0)
      error(1, errno, "%s: cannot delete", tmp_filename);
    
    exit(WEXITSTATUS(status));
  }
}

