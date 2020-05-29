# mandown

A man-page inspired Markdown pager written in C.

Considering to change the name in later release.

## What is it

Need to lookup things from README? Or from manual page? Or perhaps just want to install something cool... (**Looking for work buddies**)

## Update

**Line Wrap** won't break words apart.

**Bug:** resizing seems to break the rendering... Fix is on the way.

## Sample

![screenshot](./screenshot.png)

- This is nested list
  - 😊 is rendered
    - So is 🌚 🌕 🌖 🌗 🌘 🌑 🌒 🌓 🌔
      - It is 🔥
        - Me right now 💀

This is <ins>underline</ins>

This is <em>italic</em>

This is <strong>bold</strong>

This is a <kbd>key</kbd>

Can you see this <s>strikethrough</s> or this <del>strikethrough</del>?

`This is a code block`

## Installation

Current version is incomplete. However, it should work on simpler Markdown documents.

The Ncurses UI is still being developed for different HTML tags.

```shell
$ git clone https://github.com/Titor8115/mandown.git
$ cd mandown
$ make
```

## Library requirements

Mandown requires `libncurses(w)` and `libxml2` as compile-time dependencies.

Make sure you have them installed before compiling.

### Homebrew
```shell
$ brew install ncurses
$ brew install libxml2-dev
```

### Debian
```shell
$ apt-get install libncursesw5-dev
$ apt-get install  libxml2-dev
```

If headers are still missing or any other issues. Feel free to create an issue.

### Usage

```shell
$ cd mandown
$ ./mandown README.md
```

Mouse wheel scrolling is supported! (if your terminal emulator allows)

Scroll Up: <kbd>↑</kbd>, <kbd>k</kbd>, <kbd>BACKSPACE</kbd>

Scroll Down: <kbd>↓</kbd>, <kbd>j</kbd>, <kbd>ENTER</kbd>

Exit: <kbd>q</kbd>

## Todo

- [ ] Piping capability
- [x] Line fold/wrap on white space
- [ ] Optimized resizing
- [ ] Table and contents rendering
- [ ] Makefile makeover
