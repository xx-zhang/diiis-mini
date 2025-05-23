# Installation Guide

This guide will help you set up the D3Server on your system.

## System Requirements

- **CPU**: Intel Core i5 or AMD Ryzen 5 (minimum), Intel Core i7 or AMD Ryzen 7 (recommended)
- **Memory**: 4 GB RAM (minimum), 16 GB RAM (recommended)
- **Disk Space**: 1 GB

## Prerequisites

The following software needs to be installed on your system:

- C++ compiler with C++17 support (GCC 8+ or Clang 7+)
- CMake 3.12 or higher
- SQLite3 development libraries
- OpenSSL development libraries
- Google Test (for running tests)

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libsqlite3-dev libssl-dev libgtest-dev
```

### CentOS/RHEL

```bash
sudo yum install -y gcc-c++ cmake sqlite-devel openssl-devel gtest-devel
```

### macOS

```bash
brew install cmake sqlite openssl googletest
```

## Building from Source

1. Clone the repository:

```bash
git clone https://github.com/your-username/d3server.git
cd d3server
```

2. Create a build directory:

```bash
mkdir build
cd build
```

3. Configure with CMake:

```bash
cmake ..
```

4. Build the server:

```bash
make
```

5. Run tests (optional):

```bash
make test
```

6. Install (optional):

```bash
sudo make install
```

## Configuration

Copy the example configuration file to the default location:

```bash
cp ../configs/config.ini.example ./config.ini
```

Edit the configuration file according to your needs. See the [Configuration](configuration.md) documentation for details.

## Database Initialization

The server will automatically create the SQLite database on first run. However, you can initialize it manually:

```bash
./d3server --init-db
```

## Running the Server

```bash
./d3server
```

By default, the server will run in the foreground. Use the `--daemon` option to run it in the background.

## Creating an Account

Use the REST API to create an account:

```bash
curl -X POST "http://localhost:8080/api/accounts" \
  -H "Content-Type: application/json" \
  -d '{"login":"username@", "password":"YourPassword", "battleTag":"YourBattleTag"}'
```

Or to create an account with admin privileges:

```bash
curl -X POST "http://localhost:8080/api/accounts" \
  -H "Content-Type: application/json" \
  -d '{"login":"username@", "password":"YourPassword", "battleTag":"YourBattleTag", "role":"admin"}'
```

See [GM Commands](gm_commands.md) for more administrative operations.

## Client Configuration

1. Install the certificate from the `certs` directory: `bnetserver.p12` (password: `123`)

2. Modify hosts file or the game client to redirect to your server:

   ### Method 1: Hosts File
   
   Add to your hosts file (`C:\Windows\System32\drivers\etc\hosts` on Windows, `/etc/hosts` on Linux):
   
   ```
   127.0.0.1 us.actual.battle.net
   127.0.0.1 eu.actual.battle.net
   ```
   
   ### Method 2: Modify Game Executable
   
   Edit the game's executable file with a hex editor to replace the server addresses with your own.

3. Launch the client with:

   ```
   Diablo III64.exe -launch
   ```

## Troubleshooting

See the [Debugging](debugging.md) guide for troubleshooting information. 