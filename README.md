# mandown - mdn

A man-page inspired Markdown pager written in C.

**Name change:** executable's name changed to `mdn`

## What is it

Need to lookup things from README? Or from manual page? Or perhaps just want to install something cool...

## Update

- Mouse: other key are in progress
  - enable mouse_1 select href 
  - correct mouse wheel for scrolling.

## Sample

![screenshot](./screenshot.png)

## Installation

Current version is still being developed for some HTML tags. However, it should work on most Markdown documents.

<table>
  <tr>
    <th>Month</th>
    <th>Savings</th>
  </tr>
  <tr>
    <td>January</td>
    <td>$100</td>
  </tr>
</table>

The installed binary `mdn` would be at `/usr/local/bin/`

```shell
$ git clone https://github.com/Titor8115/mandown.git
$ cd mandown
$ make install
```

To remove the binary, you can run the commands below, or remove manually.

```shell
$ cd mandown
$ make uninstall
```

If you just want to compile and test it.

```shell
$ cd mandown
$ make
```

Feel free to create an issue.

## Usage

Check out the new sample

```shell
$ mdn sample.md
```

Mouse wheel scrolling is supported! (if your terminal emulator allows)

Scroll Up: <kbd>↑</kbd>, <kbd>k</kbd>

Scroll Down: <kbd>↓</kbd>, <kbd>j</kbd>

Page Up: <kbd>space bar</kbd>,  <kbd>pg up</kbd>, <kbd>fn + ↑</kbd>a, <kbd>b</kbd>

Page Down:  <kbd>back space</kbd>,  <kbd>pg down</kbd>, <kbd>fn + ↓</kbd>, <kbd>f</kbd>

Select & Get href: <kbd>tab</kbd> or double click <kbd>mouse 1</kbd> & <kbd>enter</kbd>

Exit: <kbd>q</kbd>

To read detailed usage, run `mdn -h`

## Library dependency

Mandown requires `libncurses(w)` and `libxml2` as compile-time dependencies.

Make sure you have them installed before compiling.

### Homebrew
**From source**

```shell
$ brew install ncurses
$ brew install libxml2-dev
```
**Precompiled binary**
```shell
$ brew install mandown
```


### Debian

```shell
$ apt-get install libncursesw5-dev
$ apt-get install libxml2-dev
```


## Todo

- [x] Line fold/wrap on white space
- [x] Optimized resizing
- [ ] Table and contents rendering
- [x] Makefile makeover
