# What is it

This repository hosts a command line code generator for [the Nut ORM](https://github.com/HamedMasafi/Nut) library.

With this tool you can generated the ORM classes directly from your database schema. 

## Usage

```
mm@lapos:~/Projektek/qt/nut_codegen/bin$ ./nut_codegen --help
Usage: ./nut_codegen [options]
NUT ORM code generator

Options:
  -h, --help                   Displays this help.
  -v, --version                Displays version information.
  -d, --directory <directory>  Directory to generate files in
  --db, --database <database>  The name of the database
  --host <host>                Server to connect (by default it is localhost)
  --password <password>        Password to use when connecting to server
  --user <user>                MySQL user name to use when connecting to server
```

## Known limitations:

At the moment I am focusing on the MySQL support, thus only that is supported

## Kudos

[@HamedMasafi](https://github.com/HamedMasafi) for creating the [ Nut ORM](https://github.com/HamedMasafi/Nut) 

[@cornelius](https://github.com/cornelius) and the kode contributors for the [libkode](https://github.com/cornelius/kode) code generation codes
