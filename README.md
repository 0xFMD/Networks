# Network

Collection of command-line network applications in C for learning POSIX sockets and basic networking.

## Features

- Multiple-client support
- Minimal dependencies, POSIX/Linux focused

## Prerequisites

- GCC
- make (optional)
- Linux / POSIX-compatible OS

## Project folders

| Folder | Description |
| --- | --- |
| **[chat/](./chat/)** | Simple multi-client chat. |
| **[webcam/](./webcam/)** | webcam streaming demo. |
| **[note/](./note/)** | Audio Note client-server example. |
| **[cloud/](./cloud/)** | Mock cloud storage (bucket). |

## Build (all apps)

If the project includes a Makefile:

```bash
make
```

Or compile manually:

```bash
gcc src/server.c -o server
gcc src/client.c -o client
```
