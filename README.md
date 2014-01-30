# ircbot

## Brief description

**ircbot** is a pretty simple IRC bot, designed to help keep order on a channel, as well as entertain the users ;) Its features include automatic kicking for bad language and flooding, but also random chatting and russian roulette (if you lose, you get banned).

Since it started as a bot for a Polish channel, much of the hard-coded text is currently in Polish, but I'm planning to add some multi-language support in the future.

## Compilation

Qt 4 is required to compile ircbot. The project file is included, so it should be enough to do:

```
qmake-qt4
make
```
