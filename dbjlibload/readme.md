#DLL Dynamic Loading mechanism Encapsulated
#[Post and some explanations](http://dbj.org/how-to-load-windows-dll-at-runtime/)

### #DBJ4UNI -- [DBJ](http://dbj.org) for [MSLearning](https://twitter.com/MSLearning)

---------------------------------------

Why loading dynamicaly an dll and calling a function from it? 

Among other things, because you do not need a header file. You do not need to find it, use it and potentially "fight" with issues some other completely unrelated stuff in the same header, might do to your project.
