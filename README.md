<a name="readme-top"></a>

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <h1 align="center">smallsh</h1>

  <p align="left">
    smallsh is a mini shell written in C. It implements a subset of features of well-known shells, such as bash. It handles variable expansion, comments, blank lines, input and output redirection, foreground and background processes, and more.
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#program-functionality">Program Functionality</a></li>
        <li><a href="#sample-execution">Sample Execution</a></li>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->

## About The Project

<!-- PROGRAM FUNCTIONALITY -->

### Program Functionality

1. The Command Prompt
    - Format:
        ```
        command [arg1 arg2 ...] [< input_file] [> output_file] [&]
        ```
    - Use `&` at the end of a command to execute it in the background
    - Use `>` and `<` for standard input or output redirection
    - Maximum line length is 2048 characters with a maximum of 512 arguments
2. Comments and Blank Lines
    - Lines that begin with # are treated as comments and are ignored
    - Blank lines (one without any commands) are ignored
    - The shell will re-prompt for another command if it receives a blank line or comment line
3. Expansion of the Variable `$$`
    - The variable `$$` will be expanded into the process ID of smallsh itself
4. Built-in Commands
    - Three built-in commands are supported: `exit`, `cd`, and `status`
    - All other commands are passed on to a member of the `exec()` family of functions
5. Executing Other Commands
    - Non-built-in commands will be forked off into a child process
    - Non-built-in commands must be located in the `PATH` variable in order for smallsh to execute them
6. Input and Output Redirection
    - Input and output redirection uses `dup2()`
    - Input files redirected via `stdin` are opened for reading only
    - Output files redirected via `stdout` are opened for writing only
    - `stdin` and `stdout` for a command can be redirected at the same time
7. Executing Commands in Foreground and Background
    - Commands without `&` at the end will run in the foreground
    - Commands with `&` at the end will run in the background (except built-in commands which will run in the foreground regardless of an `&` at the end of the command)
    - Background commands will print a process ID of the background process when it begins and a message with the process ID and exit status when the process terminates
8. Signals `SIGINT` and `SIGTSTP` - A Ctrl-C command from the keyboard will send a `SIGINT` signal to smallsh - A child process running in the foreground will terminate itself when it receives a `SIGINT` - smallsh itself will ignore `SIGINT` - A Ctrl-Z command from the keyboard will send a `SIGTSTP` signal to smallsh - Subsequent commands will no longer run in the background - `&` operator will be ignored - A subsequent `SIGTSTP` signal will return smallsh to its normal condition

 <p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- SAMPLE EXECUTION -->

### Sample Execution

```
$ smallsh
: ls
junk   smallsh    smallsh.c
: ls > junk
: status
exit value 0
: cat junk
junk
smallsh
smallsh.c
: wc < junk > junk2
: wc < junk
       3       3      23
: test -f badfile
: status
exit value 1
: wc < badfile
cannot open badfile for input
: status
exit value 1
: badfile
badfile: no such file or directory
: sleep 5
^Cterminated by signal 2
: status &
terminated by signal 2
: sleep 15 &
background pid is 4923
: ps
  PID TTY          TIME CMD
 4923 pts/0    00:00:00 sleep
 4564 pts/0    00:00:03 bash
 4867 pts/0    00:01:32 smallsh
 4927 pts/0    00:00:00 ps
:
: # that was a blank command line, this is a comment line
:
background pid 4923 is done: exit value 0
: # the background sleep finally finished
: sleep 30 &
background pid is 4941
: kill -15 4941
background pid 4941 is done: terminated by signal 15
: pwd
/nfs/stak/users/chaudhrn/CS344/prog3
: cd
: pwd
/nfs/stak/users/chaudhrn
: cd CS344
: pwd
/nfs/stak/users/chaudhrn/CS344
: echo 4867
4867
: echo $$
4867
: ^C^Z
Entering foreground-only mode (& is now ignored)
: date
 Mon Jan  2 11:24:33 PST 2017
: sleep 5 &
: date
 Mon Jan  2 11:24:38 PST 2017
: ^Z
Exiting foreground-only mode
: date
 Mon Jan  2 11:24:39 PST 2017
: sleep 5 &
background pid is 4963
: date
 Mon Jan 2 11:24:39 PST 2017
: exit
$
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- BUILT WITH -->

### Built With

[![C][c-shield]][c-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->

## Getting Started

### Prerequisites

-   GCC Compiler:
    [https://gcc.gnu.org/](https://gcc.gnu.org/)

### Installation

1. Clone the repo
    ```sh
    git clone https://github.com/cjsidler/smallsh.git
    ```
2. Build an executable
    ```sh
    gcc -std=gnu99 main.c -o smallsh
    ```
3. Run the executable.
    ```sh
    .\smallsh
    ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->

## License

[![MIT License][license-shield]][license-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTACT -->

## Contact

Contact me via email, LinkedIn, or GitHub:

[![Email][gmail-shield]][gmail-url]
[![LinkedIn][linkedin-shield]][linkedin-url]
[![GitHub][github-shield]][github-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->

## Acknowledgments

-   [README Template](https://github.com/othneildrew/Best-README-Template)
-   [Link Badges](https://shields.io/)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->

[license-shield]: https://img.shields.io/github/license/othneildrew/Best-README-Template.svg?style=for-the-badge
[license-url]: https://github.com/othneildrew/Best-README-Template/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://www.linkedin.com/in/collinsidler/
[c-shield]: https://img.shields.io/badge/C-blue?style=for-the-badge&logo=c&logoColor=white
[c-url]: https://en.wikipedia.org/wiki/C_(programming_language)
[gmail-shield]: https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white
[gmail-url]: mailto:cjsidler@gmail.com
[github-shield]: https://img.shields.io/badge/GitHub-black?style=for-the-badge&logo=github&logoColor=white
[github-url]: https://github.com/cjsidler/smallsh
