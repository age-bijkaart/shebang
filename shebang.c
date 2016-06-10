
#if HAVE_CONFIG_H
# include <config.h>
#endif

/* realloc, exit */
#include <stdlib.h>

/* for isspace() */
#include <ctype.h>

/* for read(), write() */
#include <unistd.h>

/* for open() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* for error() */
#include <error.h>
#include <errno.h>

/*
 * From: https://stackoverflow.com/questions/10813538/shebang-line-limit-in-bash-and-linux-kernel:
 * man execve: "A maximum line length of 127 characters is allowed for the first line in a #!
 * executable shell script." It's limited in the kernel by BINPRM_BUF_SIZE, set in
 * include/linux/binfmts.h.
 * 
 * Thus:  +2 for '\n' and '\0', beter be safe than sorry 
 * */
char bug[BINPRM_BUF_SIZE+2];


/* Argv, argc are as follows (if shebang is called from a shebang file):
 *
 * argv[0] = path-to-the-shebang-program
 * argv[1] = path-to-the-shebang-file
 * argv[2] and following = arguments given to the execution of the shebang file
 *
 * E.g. for
 *  ./http-grab 'http://127.0.0.1:8081'
 *
 * argv[0] = path-to-the-shebang-program
 * argv[1] = ./http-grab
 * argv[2] = http://127.0.0.1:8081
 */ 
int
main(int argc, char *argv[]) {

  /* file descriptor for input */
  const char* input_filename = argv[1]; 
  int input = open(argv[1], O_RDONLY); 
  if (input < 0) 
    error(1, errno, "%s: cannot open for reading", input_filename); 
  
  int stage = 1; /* 1 = line 1, 2 = line 2, 3 = rest */ 

  const char** words = 0; /* array of whitespace-separated words on line 2 */
  int n_words = 0; /* size of the array */


  /* file descriptor for output, default is stdout, in which case the program will behave exactly as
   * 'tail shebang-file -n +2'
   */
  int output = 1;
  const char* output_filename = "stdout";

  char buf[8192];
	int n;
	int i;

  char *word = 0;
  char word_size = 0;

	while ((n=read(input, buf, sizeof(buf)))>0) {
    for (i=0; (i<n); ++i) {
      switch (stage) {
        case 1:
            if (buf[i] == '\n')
              stage = 2;
            break;
          case 2:
            if (isspace(buf[i])) {
              if (word) {
                words = realloc(words, (n_words +1) * sizeof(const char*));
                words[n_words++] = word;
                word = 0;
                word_size = 0;
              }
              else /* no current word and white space input char: do nothing, e.g. multiple white space 
                      chars before/after a word */
                ;
            }
            else {
              if (word_size == 0)
                word_size = 1; /* Size if never 1, we need an extra place for '\0' */
              word = realloc(word, ++word_size);
              word[word_size -2] = buf[i];
              word[word_size -1] = '\0';
            }
            if (buf[i] == '\n') {
              stage = 3;
              if (n_words > 0) {
                output_filename = words[0];
                output = open( words[0], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
                if (output < 0)
                  error(1, errno, "%s: cannot create", output_filename);
              }
            }
            break;
        case 3:
		       if(write(output, &buf[i], 1)!=1) 
			       error(1, errno, "write error on %s", words[0]);
           break;
        default:
          error(1, errno,  "%d: illegal stage", stage);
      }
    }
  }

	if(n < 0)
		error(1, errno, "read error");

  /* Prepare exec parameters by appending the command lines pars to the words array */
  for (int j = 2; (j <argc); ++j) { 
    words = realloc(words, (n_words +1) * sizeof(char*)); 
    words[n_words++] = argv[j];
  }

  /* Add a null pointer at the end for exec */
  words = realloc(words, (n_words +1) * sizeof(char*)); 
  words[n_words++] = 0;

  if (close(input) < 0)
    error(1, errno, "close %s failed", input_filename);
  if (close(output) < 0)
    error(1, errno, "close %s failed", output_filename);

  char* const* args = (char* const*) (words + 1);
  /* execvp should transfer PATH etc */
  if ( execvp(words[1], args) < 0)
    error(1, errno, "exec %s failed", words[1]);

  return 0;
}

