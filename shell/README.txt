Our high level approach was to split up the command input into a 2D character
array by two functions – getargs and getword. We then executed that command
using fork, execvp, and waitpid in our execute function. We also kept track of
various errors that might occur along the process. The prompt simply printed out
the prompt when necessary. We used getchar to get the current command. As soon
as we found a \n we would stop, execute the command, and then keep reading from
stdin.

The biggest challenge we faced was fgets. It removes newlines when reading from
stdin which we did not realize. We had to change everything around to
accommodate for getchar -
the code got kind of messy real quick. We could of rewritten everything;
however, we saw it as an opportunity to learn more about gdb and debug all of
our failing test cases. Another test we had trouble was the one that made sure
our files were cleared when opening. We had forgotten the O_TRUNC flag, and it
took us a while to find the cause of the failing test.

I think the overall layout and structure of the code is good. The implementation
could have been better, especially in getword, but that is the price we had to
pay when deciding not to rewrite everything. I also think our dynamic allocation
could be implemented better.

Most of our major debugging was done through gdb, but we also provided error
messages for failing functions. We tested against the test file quite a bit as
well.

We did not implement any extra features.
