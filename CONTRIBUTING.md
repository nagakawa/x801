## Introduction

Thank you for contributing to Experiment801!

This document will explain how to contribute in a way that makes the development
team save time and want to work with you.

Experiment801 is an open-source project that *you* can contribute to!
We are looking for bug reports, feature requests, documentation changes and
code from the community.

For support questions, contact the #x801 channel on [our Discord](https://discord.gg/sDbNH5N)
instead.

## Ground rules

* Follow our code of conduct.
* Make sure that output files (e. g. executable and object files, compiled asset binaries)
  don't creep into the repository. Update the `.gitignore` file if necessary.
* Follow our [code style]
  (https://docs.google.com/document/d/1AskEPaRCH0A6xCgIYerogpheiXyx4UT886UwIGPR-vU/edit?usp=sharing).
* Consult [this document]
  (https://docs.google.com/document/d/1XcE7H5GtrZIlj6qKXPGBSr0KNMsfeWh0oBGjSW0jTNI/edit?usp=sharing) for specifications on file and network formats.
* Usually, you should have one class per file. Multiple classes in one file are okay if they're related to each other and one of them is small.

### Philosophy

The goals of Experiment801 are:

* to give people like *you* the ability to create content and game enhancements as easily as possible
  (even if this means that the game doesn't look like a high-budget game)
* to perform well, especially on servers, so more people can host their own
* to improve on what Wizard101 has failed

Keep these in mind when you request or develop new features.

## Getting started

First, fork the repository (make your own copy) by pressing the "Fork" button.

Clone the repository with the `--recursive` flag if you don't already have changes;
otherwise create a new remote to your repository (replacing the URL with that of
your own copy):

    git remote add forked-repo https://github.com/insert-username/x801.git

(Consult README.md for building instructions.)

If you'd like to see your change in our project, then please create a pull request.
Click [here](http://makeapullrequest.com/) to learn how!

## Reporting a bug

If you find a security issue*, **do NOT open an issue.** Email uruwi [at] protonmail [dot] com
or send a Discord PM to Uruwi. If you're not sure, contact Uruwi privately anyway.

\* These include (but are not limited to):

* any client action that can reliably cause the server to crash or become unresponsive
* anything that causes the client or server to write to or delete files outside of Experiment801
* anything that causes the server's save data to become corrupted
* anything that gives access to someone else's private information

Otherwise, you can file a bug report at our repo.

### Filing an issue

When filing an issue, make sure to answer the following questions:

1. What OS are you using? Include both the distribution and the kernel version. You can get the latter using `uname -a`, assuming that your operating system is POSIX-like.
2. What C++ compiler are you using? You might be able to get this info using `c++ --version`.
3. What version of Python are you using? You can get this info on `python3 --version`.
4. When does this bug occur? Does it affect compilation or linking, or the execution of a program?
5. If the project builds successfully, which mode is affected by this bug: the client or the server?
6. What did you do to reproduce this bug?
7. What did you expect to happen?
8. What actually happened?
9. If you encounter a crash, then try running the program under `gdb` and repeating the steps to reproduce the bug in question. After the program crashes, type in `bt` into the `gdb` prompt to get a stack trace, and attach this with the bug report.
10. If you encounter a graphical error and you have a GPU driver that supports OpenGL 4.3 or later, or that has the `ARB_debug_outupt` extension, run the client with the `-d` flag and look for any output that wasn't there before.

## Suggesting enhancements

If you want to see a feature implemented, open an issue on GitHub!
(Please consult the Philosophy section to find out whether a feature request is in the scope of this project.)
