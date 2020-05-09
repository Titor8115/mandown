# mandown

A man-page inspired Markdown pager written in C.

Considering to change the name in later release.

## What is it

Need to lookup things from README? Or from manual page? Or perhaps just want to install something cool... (**Looking for work buddies**)

## Sample

😊 is rendered

- This is a bullet

This is <u>underline</u>

This is <em>italic</em>

This is <strong>bold</strong>

This is a <kbd>key</kbd>

Can you see this <s>strikethrough</s> or this <del>strikethrough</del>?

`This is a code block`

## Install

Current version is incomplete. However, it should work on Linux and OS X.

The Ncurses UI is still being developed.

```bash
$ git clone https://github.com/Titor8115/mandown.git
$ cd mandown
$ make
```

### Usage

```bash
$ cd mandown
$ ./mandown README.md
```

Mouse wheel scrolling is supported! (if your terminal emulator allows)

Scroll Up: <kbd>↑</kbd>, <kbd>BACKSPACE</kbd>

Scroll Down: <kbd>↓</kbd>, <kbd>ENTER</kbd>

Exit: <kbd>q</kbd>

if it compiles but doesn't run, try updating ncurses library.

```bash
$ apt-get install libncursesw5-dev
```

Mandown currently depends on libncursesw.
I haven't tested if it could also run under libncurses.

## Todo

- Format
  - [x] markdown
  - [ ] troff

- Command-line
  - [x] test version
  - [ ] document pathfinder

- Ncurses
  - [x] keyboard control
  - [x] coloring
