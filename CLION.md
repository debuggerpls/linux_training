### Using CLion in kernel sources
---

To use CLion with kernel sources, compilation database is needed. There are few tools that generate it but the simpliest way is to use the script:
```
scripts/gen_compile_commands.py
```

Then just open the file `compile_commands.json` as project and CLion will index kernel sources. For more information read [CLion documentation][1].



[1]: https://www.jetbrains.com/help/clion/compilation-database.html#compdb_reload
