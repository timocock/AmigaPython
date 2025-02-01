# AmigaOS Python 2.7 Port Status

## Remaining Core Tasks

### File System Integration
- [ ] Finalize path conversion functions (Amiga <-> Unix)
- [ ] Implement case-insensitive path handling
- [ ] Add support for Amiga device paths (DF0:, RAM: etc)
- [ ] Validate filesystem stats/attributes conversion

### Process Management
- [ ] Implement AmigaDOS `System()` replacement
- [ ] Add Amiga-specific process spawn methods
- [ ] Handle Amiga task IDs vs Unix PIDs

### I/O Subsystem
- [ ] Complete BPTR <-> file descriptor translation
- [ ] Implement Amiga message port async I/O
- [ ] Add console handler for Amiga shell

### Build System
- [ ] Verify cross-compilation with VBCC/m68k-amigaos
- [ ] Create Amiga-specific makefile targets
- [ ] Add AmigaOS version detection in configure
- [ ] Validate .library dependencies

### Testing
- [ ] Create Amiga test suite
- [ ] Verify on real Amiga 1200/4000 hardware
- [ ] Test under AmigaOS 3.x and 4.x
- [ ] Benchmark against Python 2.0 Amiga version

### Documentation
- [ ] Update README with Amiga build instructions
- [ ] Document Amiga-specific module limitations
- [ ] Create porting guide for Amiga developers

## External Dependencies

### Required Libraries
1. **AmigaSSL**  
   - Source: [AmiSSL](https://github.com/jens-maus/amissl)  
   - Version: 5.3+  
   - Install:  
     ```bash
     copy lib:amissl/ssl.lib to SDK:libs/
     copy include:amissl/ to SDK:include/
     ```

2. **zlib**  
   - Source: [Aminet](http://aminet.net/dev/lib/zlib.lha)  
   - Version: 1.2.11  
   - Install:  
     ```bash
     lha x zlib.lha SDK:libs/zlib/
     ```

3. **bzip2**  
   - Source: [AmigaBZ2](https://github.com/AmigaPorts/bzip2)  
   - Version: 1.0.8  
   - Install:  
     ```bash
     copy lib:bzip2.lib to SDK:libs/
     copy include:bzip2/ to SDK:include/
     ```

4. **expat** (for XML parsing)  
   - Source: [Amiga expat](https://github.com/AmigaPorts/expat)  
   - Version: 2.4.1  
   - Install:  
     ```bash
     copy lib:expat.lib to SDK:libs/
     ```

### Optional Components
1. **SQLite3**  
   - Source: [AmigaSQLite](https://github.com/AmigaSQLite/AmigaSQLite)  
   - Needed for `sqlite3` module

2. **libffi**  
   - Source: [AmiFFI](https://github.com/AmigaPorts/libffi)  
   - Required for ctypes support

3. **ncurses**  
   - Source: [AmigaNCurses](https://github.com/AmigaPorts/ncurses)  
   - Needed for console UI modules

## Build Validation Checklist

## Known Limitations
- No thread support (AmigaOS lacks POSIX threads)
- Limited dynamic module loading
- No fork()/exec() POSIX semantics
- Case-sensitive path handling differs from Amiga norms

## Contribution Guidelines
- Test changes on real Amiga hardware or FS-UAE emulator
- Follow Amiga coding conventions for system calls
- Maintain compatibility with AmigaOS 3.1+ and MorphOS
- Document all Amiga-specific API usage