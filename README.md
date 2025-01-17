hardv
=====
(/*hɑ:rd'vi:*/)


`hardv` is a CLI flashcard app for UNIX-compatible systems,
conforming to the UNIX philosophy.

- The format of input files are
easy to parse by both human and other UNIX utilities
like `grep`/`cut`/`sed`/`awk`/`perl`/...

- Metadata is written back to the input file, no hidden secret.

- Except the review scheduling algorithm and the parser,
everything can be customized,
with any programming language you prefer.
E.g.:

	- You may configure it to a TUI program,
	or to view images in a GUI window.
	- `hardv` can open the editor;
	Send the content you wrote
	to an online judging system,
	and determine the next quiz time by the judging result.
	- It can be used to implement
	keyboard shortcut practice,
	cloze deletion,
	text-to-speech review,
	typing in the answer,
	and more.

- By default, `hardv` is an interactive UNIX filter
which makes it easy to be controlled by a master program.

- The scheduling algorithm is simple exponential time intervals,
easy to build a mental model.

Getting Started
---------------

First of all, let's see the standard quiz of `hardv`.

Suppose we have a file `input.fc` consists of flashcards.

	$ cat input.fc

	Q	hex(128) = ?
	A	0x80
	%%
	Q	hex(32768) = ?
	A	0x8000
	%%
	Q	the most famous C program from K&R, second edition
	A	#include <stdio.h>
		
		main()
		{
			printf("hello world\\n");
		}

Cards are separated by `%%` on a line by itself.
The key and value of a field in a card are separated by a tab character.
Subsequent lines in the value are indented by a tab character.
(See the [*Input Format*](#input-format) section)

We could use `hardv` to run a quiz.
(See the [*Installation*](#installation) section)
	
	$ hardv input.fc
	...

	Q:

		hex(128) = ?

	Press <ENTER> to check the answer.

	A:

		0x80

	Do you recall? (y/n/s)
	y

	...

After running a quiz,
two new fields `PREV` and `NEXT` may be inserted to the original file,
to indicate the previous and the next scheduled quiz time.

	NEXT    2001-08-21 15:31:06 -0500
	PREV    2001-08-23 15:31:06 -0500
	Q       hex(128) = ?
	A       0x80

You could manually modify these two fields to tune the quiz time.

The time interval is doubled if you are able to recall the card.
Otherwise the card will be scheduled at 24 hours later.
See the man page `hardv(1)` for detail.

To customize the quiz,
a `MOD` field is required to be added to the card.
See the [*Customize the Quiz*](#customize-the-quiz) section.

Customize the Quiz
------------------

If a card contains the `MOD` field,
the customized quiz is used.
The value of the `MOD` field is passed to `/bin/sh -c` by `hardv`,
and several environment variables are set,
for the mod can read information about the card.
The exit status of the mod determines the action `hardv` would take
for the card.

Let's write a simple mod called `recite`.
When a card with this mod is quizzed,
it prints the content of the `Q` field,
then the user is required to copy the `A` field exactly.
If the answer is correctly copied,
this mod notifies `hardv` that the user is able to recall the card.
If the answer is incorrectly copied,
this mod notifies `hardv` that the user is unable to recall the card.
The user can type `!` to skip the card,
and this mod notifies `hardv` to update nothing.

We create a shell script named `/usr/loca/bin/recite`
with the following content and make it executable.

~~~shell
#!/bin/sh

# Print a blank line before quizzing a card, except the first
if ! [ "$HARDV_FIRST" ]; then
	printf '\n'
fi
printf 'Q: %s\n' "$HARDV_Q"	# Print the question
printf '(recite) A: '		# Print the prompt
read a				# Read the user-input line to variable a
if [ "$a" = "!" ]; then
	exit 2;			# Skip this card
fi
if [ "$a" = "$HARDV_A" ]; then
	echo 'Correct!'
	exit 0;			# The user is able to recall
fi
printf 'Wrong! The answer is: %s\n' "$HARDV_A"
exit 1;				# The user is unable to recall
~~~

For the card we want to enable this mod,
we add the `MOD` field with the value `recite`.
(Ensure `/usr/local/bin` is in the `$PATH` environment variable)

	$ cat input.fc

	MOD	recite
	Q	hex(128) = ?
	A	0x80
	%%
	MOD	recite
	Q	hex(32768) = ?
	A	0x8000

When we run `hardv` on this card, `recite` is called.

	$ hardv input.fc
	Q: hex(128) = ?
	(recite) A: 0x80
	Correct!

	Q: hex(32768) = ?
	(recite) A: 0x800
	Wrong! The answer is: 0x8000

The mod can also be embedded in the input file,
demonstrated by the following.

	$ cat input.fc

	MOD
		! [ "$HARDV_FIRST" ] && printf '\n'
		printf 'Q: %s\n(recite) A: ' "$HARDV_Q"
		read a
		[ "$a" = "!" ] && exit 2;
		if [ "$a" = "$HARDV_A" ]; then
			echo 'Correct!'
			exit 0;
		fi
		printf 'Wrong! The answer is: %s\n' "$HARDV_A"
		exit 1;
	Q	hex(128) = ?
	A	0x80

This could be very handy if you want to wrap existed programs to a
`hardv` mod.

See the man page `hardv(1)` for detail about how to implement a mod.

Installation
------------

- Choose a version from
the [release page](https://github.com/dongyx/hardv/releases);
The latest is always recommended unless otherwise noted in the
release note.

- Extract the source.

- Execute:

	~~~
	$ make
	$ sudo make install
	~~~

`make install` copies files to `/usr/local` by default.
The man page `hardv(1)` is shipped with the installation.

Input Format
------------

A card is a key-value structure contains at least two fields `Q`
and `A`.

The key and the value are separated by a tab character or a newline
character.
Every line in the value which is not on the same line with the key
must start with a tab character, except for blank lines.
For blank lines in the value, the leading tab character is optional.

Cards are typically separated by `%%` or `%` on a line by itself.
In fact, any line starting with `%` is seen as a card separator,
but `%%` is recommended.

Empty cards are ignored.  Thus separators can be used as comments,
demonstrated by the following.

	%% This is a card set for hardv(1)
	%% Created at Feb 1, 1997

	Q       hex(256) = ?
	A       0x100
	%%
	Q       hex(128) = ?
	A       0x80

	%% Arithmetic cards begin

	Q       1 + 1 = ?
	A       2
	%%
	Q       8 - 5 = ?
	A       3

	%% Arithmetic cards end

See the man page `hardv(1)`
for the full description of the input format.

Usage
-----

Executing `hardv -h` prints a brief help.

Full description is documented in the man page `hardv(1)`.
Typing `man hardv` to read.

Migration from 1.x
------------------

`hardv` 2.0.0 is a breaking change from 1.x.
The file format is not compatible.

Thus `hardv` provides the `-1` option to convert old files.

	# Convert a.fc, b.fc, and c.fc to the new format, in-place
	$ hardv -1 a.fc b.fc c.fc

It in-place updates the old files to the new format.
**Backing up the old files before updating is strongly recommend.**

Following News
--------------

- <https://www.dyx.name>
- [@dongyx2](https://twitter.com/dongyx2)
