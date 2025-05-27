cc64 limitations


If you pass a struct by value, it actually really does a reference.
So to work around this issue, you need to do your own assignment
to a scratch variable, before passing it to a function, to allow
it to be trashed.


"const" causes some things to go haywire. I think offsets are
wrong. Can't remember. Workaround is to remove all references
to const by using the preprocessor to define const to nothing.


You can't do some things after a return statement. Like some
sort of if/else block. Can't remember exactly, but workaround
is to add your own goto instead of a return, to go to the
end of the function where there are no more blocks.


You can't call a function pointer by just giving its name.
You need to do (*func)()
