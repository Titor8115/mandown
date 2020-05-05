# mandown

A man-page inspired Markdown reader written in C.

## What is mandown

Need to lookup things from README? Or from manual page? Or perhaps just want to install something cool...

## Install

Current version is incomplete. However, it should work on Linux and OS X.

The Ncurses UI is still being developed.

```shell
$ git clone https://github.com/Titor8115/mandown.git
$ cd mandown
$ make
```

### Usage

```
$ mandown ./README.md
```

While running, press <kbd>q</kbd> button to exit.

if it compiles but doesn't run, try updating ncurses library.

```
$ apt-get install libncursesw5-dev
```

Mandown currently depends on libncursesw.
I haven't tested if it could also run under libncurses.

## Looking for helps

## Todo

- Format
  - [x] markdown
  - [ ] troff

- Command-line
  - [x] test version
  - [ ] document pathfinder

- Ncurses
  - [x] keyboard control
  - [ ] coloring
