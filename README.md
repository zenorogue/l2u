# l2u
commandline Latex to Unicode converter

Compilation should be straightforward: `make l2u`

Usage: Just run ./l2u and type LaTeX in the standard input. 

It covers the symbols I use most frequently on [Twitter](https://twitter.com/zenorogue/)
(where it is especially useful due to the space limit), such as:

* math italic `$n$`
* superscripts and subscript `$n^{2+3}$`
* blackboard board `$\bbR^3$`
* some basic symbols, like `\in`, `\neq`, `\leq`
* diacritics `Poincar\'e`, `G\"odel`

Abbreviations of LaTeX commands are included (`\bb` instead of `\mathbb`).
Contrary to LaTeX, you do not need to mark when the command ends,
so e.g. you can write `\bbR` instead of `\bb{R}`.

This is a very simple, incomplete program. Contributions are welcome.
