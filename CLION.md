### Using CLion in kernel sources
---

To use CLion with kernel sources, compilation database is needed. There are few tools that generate it but the simpliest way is to use the script:
```
scripts/gen_compile_commands.py
```

Then just open the file `compile_commands.json` as project and CLion will index kernel sources. For more information read [CLion documentation][1].

#### Formating
---

Coding style is documented in `Documentation/CodingStyle`. To check files for correct coding style:
```
sudo apt install python3-ply python3-git

<path-linux-src>/scripts/checkpatch.pl --file --no-tree <file>.c
```

#### Auto-format in CLion
---

Copy the `.clang-format` file from linux sources to the root of project. Then in the bottom right, set CLion to use ClangFormat. 

Shortcut to format file: `CTRL + ALT + L`

[CLion Clang format][2]

[1]: https://www.jetbrains.com/help/clion/compilation-database.html#compdb_reload
[2]: https://www.jetbrains.com/help/clion/clangformat-as-alternative-formatter.html
