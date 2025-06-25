# Custom Shell

A feature-rich command-line shell implementation written in C++23, providing essential shell functionality with modern C++ features and GNU Readline integration.

## Features

### Core Shell Functionality
- **Interactive Command Line**: Full-featured readline integration with command history and tab completion
- **Command Parsing**: Robust input parsing with support for quoted strings and escape sequences
- **Pipeline Support**: Execute commands connected with pipes (`|`)
- **I/O Redirection**: Comprehensive redirection support:
  - Standard output redirection (`>`, `>>`, `1>`, `1>>`)
  - Standard error redirection (`2>`, `2>>`)
  - Append and truncate modes

### Built-in Commands
- `exit [0]` - Exit the shell
- `echo [args...]` - Display text
- `type <command>` - Show command type (builtin or external path)
- `pwd` - Print working directory
- `cd <directory>` - Change directory (supports `~` for home)
- `history` - Command history management with options:
  - `history` - Show all history
  - `history <n>` - Show last n commands
  - `history -r <file>` - Read history from file
  - `history -w <file>` - Write history to file
  - `history -a <file>` - Append new history to file

### Advanced Features
- **Tab Completion**: Auto-complete commands from both built-ins and PATH
- **History Management**: Persistent command history with file support
- **Path Resolution**: Automatic discovery and execution of system commands
- **Quote Handling**: Support for single quotes, double quotes, and escape sequences
- **Directory Creation**: Automatic directory creation for output redirection

## Building

### Prerequisites
- C++23 compatible compiler (GCC 13+ or Clang 16+)
- CMake 3.13 or higher
- GNU Readline library
- vcpkg (for dependency management)

### Build Instructions

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd shell
   ```

2. **Build using the provided script**
   ```bash
   ./shell.sh
   ```

   Or manually:
   ```bash
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
   cmake --build ./build
   ```

3. **Run the shell**
   ```bash
   ./build/shell
   ```

## Usage Examples

### Basic Commands
```bash
$ pwd
/home/user/current/directory

$ echo "Hello, World!"
Hello, World!

$ type ls
ls is /usr/bin/ls
```

### I/O Redirection
```bash
$ echo "Hello" > output.txt
$ echo "World" >> output.txt
$ cat output.txt
Hello
World

$ ls /nonexistent 2> error.log
$ cat error.log
ls: cannot access '/nonexistent': No such file or directory
```

### Pipelines
```bash
$ ls -la | grep ".cpp"
$ cat file.txt | grep "pattern" | wc -l
$ ps aux | grep "process" | awk '{print $2}'
```

### History Management
```bash
$ history 5
    1  pwd
    2  echo "test"
    3  ls -la
    4  cat file.txt
    5  history 5

$ history -w ~/.shell_history
$ history -r ~/.shell_history
```

### Directory Navigation
```bash
$ cd ~
$ pwd
/home/user

$ cd /tmp
$ pwd
/tmp
```

## Project Structure

```
├── src/
│   ├── main.cpp              # Entry point and main loop
│   └── shell/
│       ├── builtins.cpp      # Built-in command implementations
│       ├── builtins.hpp      # Built-in command declarations
│       ├── command.hpp       # Command structure definition
│       ├── completion.cpp    # Tab completion functionality
│       ├── completion.hpp    # Completion function declarations
│       ├── executor.cpp      # Command and pipeline execution
│       ├── executor.hpp      # Execution function declarations
│       ├── history.cpp       # History management
│       ├── history.hpp       # History function declarations
│       ├── parser.cpp        # Input parsing and tokenization
│       ├── parser.hpp        # Parser function declarations
│       ├── path.cpp          # PATH resolution and command discovery
│       ├── path.hpp          # Path function declarations
│       ├── utils.cpp         # Utility functions
│       └── utils.hpp         # Utility function declarations
├── CMakeLists.txt            # Build configuration
├── vcpkg.json               # Dependency configuration
├── shell.sh                 # Build and run script
└── README.md                # This file
```

## Architecture

### Command Processing Flow
1. **Input Reading**: Uses GNU Readline for input with history and completion
2. **Tokenization**: Parses input into tokens handling quotes and escapes
3. **Command Parsing**: Converts tokens into Command structures with redirection info
4. **Execution**: Handles built-ins directly or forks for external commands
5. **Pipeline Management**: Connects commands with pipes for multi-command pipelines

### Key Components
- **Parser**: Handles complex shell syntax including quotes and redirections
- **Executor**: Manages process creation, I/O redirection, and pipeline setup
- **Built-ins**: Implements essential shell commands directly in the shell process
- **Completion**: Provides intelligent tab completion for commands and files
- **History**: Manages command history with persistent storage

## Environment Variables

- `PATH` - Used for command resolution
- `HOME` - Used for `~` expansion in `cd` command
- `HISTFILE` - History file location (optional)

## Error Handling

The shell provides informative error messages for:
- Command not found
- File/directory access errors
- Invalid redirections
- Pipeline failures
- History file operations

## Contributing

1. Follow the existing code style and structure
2. Add appropriate error handling
3. Update tests for new functionality
4. Document new features in this README

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Technical Notes

- Uses C++23 features including `std::filesystem`
- Memory management handled automatically with RAII
- Signal handling for proper cleanup
- Fork/exec model for external command execution
- Pipe management for inter-process communication