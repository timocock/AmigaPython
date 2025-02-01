# Python 2.7.18 AmigaOS Port Project

## Project Aims
- Provide a functional Python 2.7.18 implementation for AmigaOS systems
- Maintain compatibility with legacy Amiga hardware (68000 series) while supporting newer PowerPC architectures
- Implement essential POSIX functionality through Amiga-specific workarounds
- Preserve maximum compatibility with existing Python 2.x codebases
- Optimize for AmigaOS's unique features like message passing and shared libraries

## Amiga Port Limitations
⚠️ **POSIX Compatibility Challenges**:
- Limited thread support due to AmigaOS's process model
- Filesystem operations restricted to 31-character filename limits
- No support for symlinks on FAT filesystems
- Limited users/groups permissions implementation

⚠️ **System Constraints**:
- No native support for fork() system call emulation
- Reduced module availability (e.g., missing multiprocessing module)
- Memory handling optimized for Amiga's limited contiguous memory model
- Hardware-specific limitations on 68000 series (FPU requirements for certain math operations)

⚠️ **Known Issues**:
- Subprocess handling requires AmigaDOS workarounds
- Limited termios support for serial I/O
- No support for loadable kernel extensions
- Networking stack requires bsdsocket based TCP/IP stack and AmigaSSL

Current development focuses on maintaining core Python functionality while adapting to AmigaOS's unique architecture. Contributors are advised to test thoroughly on target hardware configurations.

## Known Limitations
- No thread support (AmigaOS lacks POSIX threads)
- Limited dynamic module loading
- No fork()/exec() POSIX semantics
- Case-sensitive path handling differs from Amiga norms

## Contribution Guidelines
- Test changes on real Amiga hardware or UAE-type of emulator
- Follow Amiga coding conventions for system calls
- Maintain compatibility with AmigaOS 3.1+
- Document all Amiga-specific API usage